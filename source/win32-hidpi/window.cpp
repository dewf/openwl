#include "window.h"

#include "unicodestuff.h"
#include "globals.h"
#include "comstuff.h"

#include "cursor.h"

#include <ShellScalingApi.h>

#include <windowsx.h> // for some macros (GET_X_LPARAM etc)

#include <stdio.h>

// DPI macros
#define DECLSF(dpi) double scaleFactor = dpi / 96.0;
#define INT(x) ((int)(x))
#define DPIUP(x) INT(x * scaleFactor)                // from device-independent pixels to physical res
#define DPIDOWN(x) INT(x / scaleFactor)              // from physical res to DIPs
#define DPIUP_INPLACE(x) x = DPIUP(x);
#define DPIDOWN_INPLACE(x) x = DPIDOWN(x);

// fwd decls
long getWindowStyle(wl_WindowProperties* props, bool isPluginWindow);
void calcChromeExtra(int* extraWidth, int* extraHeight, DWORD dwStyle, BOOL hasMenu, UINT dpi);
unsigned int getKeyModifiers();
unsigned int getMouseModifiers(WPARAM wParam);

wl_Window::wl_Window()
{
	// nothing yet, all the work done in static ::create
}

wl_Window::~wl_Window()
{
	if (d2dRenderTarget) {
		d2dRenderTarget->Release();
	}
}

wl_WindowRef wl_Window::create(int dipWidth, int dipHeight, const char* title, void* userData, wl_WindowProperties* props)
{
	auto wideTitle = title ? utf8_to_wstring(title) : L"(UNTITLED)";

	int extraWidth = 0;
	int extraHeight = 0;

	bool isPluginWindow = (props &&
		(props->usedFields & wl_kWindowPropStyle) &&
		(props->usedFields & wl_kWindowPropNativeParent) &&
		(props->style == wl_kWindowStylePluginWindow));

	auto dwStyle = getWindowStyle(props, isPluginWindow);

	// DPI awareness
	auto defaultMonitor = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
	UINT dpiX, dpiY;
	GetDpiForMonitor(defaultMonitor, MDT_DEFAULT, &dpiX, &dpiY);
	UINT dpi = (dpiX + dpiY) / 2; // uhh, I guess?

	DECLSF(dpi);
	auto width = DPIUP(dipWidth);
	auto height = DPIUP(dipHeight);

	// fix props
	DPIUP_INPLACE(props->minWidth);
	DPIUP_INPLACE(props->minHeight);
	DPIUP_INPLACE(props->maxWidth);
	DPIUP_INPLACE(props->maxHeight);

	// create actual win32 window
	HWND hWnd = NULL;
	if (isPluginWindow)
	{
		hWnd = CreateWindowW(topLevelWindowClass, wideTitle.c_str(), dwStyle,
			0, 0,
			width, height, props->nativeParent, nullptr, hInstance, nullptr);
	}
	else {
		// normal top-level window (or frameless)
		calcChromeExtra(&extraWidth, &extraHeight, dwStyle, FALSE, dpi); // FALSE = no menu for now ... will recalc when the time comes

		auto exStyle = (dwStyle & WS_POPUP) ? WS_EX_TOOLWINDOW : 0; // no taskbar button plz

		hWnd = CreateWindowExW(exStyle, topLevelWindowClass, wideTitle.c_str(), dwStyle,
			CW_USEDEFAULT, CW_USEDEFAULT,
			width + extraWidth, height + extraHeight, nullptr, nullptr, hInstance, nullptr);
	}

	if (hWnd) {
		// associate data
		wl_WindowRef wlw = new wl_Window;
		wlw->hWnd = hWnd;
		wlw->dwStyle = dwStyle;
		wlw->dpi = dpi;
		wlw->clientWidth = width;
		wlw->clientHeight = height;
		wlw->extraWidth = extraWidth;
		wlw->extraHeight = extraHeight;
		wlw->userData = userData;
		wlw->dropTarget = nullptr;
		wlw->props.usedFields = 0;
		if (props != nullptr) {
			memcpy(&wlw->props, props, sizeof(wl_WindowProperties));
		}
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)wlw);

		if (useDirect2D) {
			wlw->direct2DCreateTarget();
		}

		return wlw;
	}
	// else
	return nullptr;
}

void wl_Window::unregisterDropWindow()
{
	if (dropTarget) {
		// remove drag+drop
		RevokeDragDrop(hWnd);

		// remove the strong lock
		CoLockObjectExternal(dropTarget, FALSE, TRUE);

		// release our own reference
		dropTarget->Release();

		dropTarget = nullptr;
	}
}

void wl_Window::direct2DCreateTarget()
{
	ID2D1RenderTarget* oldTarget = d2dRenderTarget;
	if (d2dRenderTarget) {
		d2dRenderTarget->Release();
		d2dRenderTarget = nullptr;
	}
	// Create a Direct2D render target
	auto rtprops = D2D1::RenderTargetProperties();
	auto hrtprops = D2D1::HwndRenderTargetProperties(hWnd, D2D1::SizeU(clientWidth, clientHeight));
	HR(d2dFactory->CreateHwndRenderTarget(&rtprops, &hrtprops, &d2dRenderTarget));

	// event
	wl_Event event;
	event.eventType = wl_kEventTypeD2DTargetRecreated;
	event.d2dTargetRecreatedEvent.newTarget = d2dRenderTarget;
	event.d2dTargetRecreatedEvent.oldTarget = oldTarget;
	event.handled = false;
	eventCallback(this, &event, userData);
	// don't care if it was handled or not
}

void wl_Window::wlDestroy()
{
	unregisterDropWindow();
	DestroyWindow(hWnd);
}

void wl_Window::show()
{
	ShowWindow(hWnd, SW_SHOWNORMAL); // might need to use a different cmd based on whether first time or not
	UpdateWindow(hWnd);
}

void wl_Window::showRelative(wl_WindowRef relativeTo, int x, int y, int newWidth, int newHeight)
{
	DECLSF(dpi);
	DPIUP_INPLACE(x);
	DPIUP_INPLACE(y);
	DPIUP_INPLACE(newWidth);
	DPIUP_INPLACE(newHeight);

	POINT p{ x, y };
	ClientToScreen(relativeTo->hWnd, &p);
	auto doSize = (newWidth > 0 && newHeight > 0);
	auto flags = (doSize ? 0 : SWP_NOSIZE) | SWP_SHOWWINDOW | SWP_NOACTIVATE;
	SetWindowPos(hWnd, HWND_TOP, p.x, p.y, newWidth, newHeight, flags);
}

void wl_Window::hide()
{
	ShowWindow(hWnd, SW_HIDE);
}

void wl_Window::invalidate(int x, int y, int width, int height)
{
	DECLSF(dpi);
	if (width > 0 && height > 0) {
		RECT r;
		r.left = DPIUP(x);
		r.top = DPIUP(y);
		r.right = DPIUP(x + width);
		r.bottom = DPIUP(y + height);
		InvalidateRect(hWnd, &r, FALSE);
	}
	else {
		InvalidateRect(hWnd, nullptr, FALSE); // entire window
	}
}

void wl_Window::setCursor(wl_CursorRef cursor)
{
	if (mouseInWindow) { // only makes sense to set when it's inside - gets reset when it leaves/re-enters anyway
		if (this->cursor != cursor) { // only set on change
			if (cursor) {
				// actually set
				cursor->set();
				this->cursor = cursor;
			}
			else {
				// clear
				wl_Cursor::defaultCursor->set();
				this->cursor = nullptr;
			}
		}
	}
}

// win32 wndproc handling ==========================================================

void wl_Window::onClose(wl_Event& event)
{
	event.eventType = wl_kEventTypeWindowCloseRequest;
	event.closeRequestEvent.cancelClose = false;
	eventCallback(this, &event, userData);
}

void wl_Window::onDestroy(wl_Event& event)
{
	event.eventType = wl_kEventTypeWindowDestroyed;
	event.destroyEvent.reserved = 0;
	eventCallback(this, &event, userData);

	printf("deleting wl_Window\n");
	delete this;
}

void wl_Window::onSize(wl_Event& event) {
	RECT clientRect;
	GetClientRect(hWnd, &clientRect);

	int newWidth = clientRect.right - clientRect.left;
	int newHeight = clientRect.bottom - clientRect.top;

	DECLSF(dpi);

	event.eventType = wl_kEventTypeWindowResized;
	event.resizeEvent.newWidth = DPIDOWN(newWidth);
	event.resizeEvent.newHeight = DPIDOWN(newHeight);
	event.resizeEvent.oldWidth = DPIDOWN(clientWidth); // hasn't updated yet
	event.resizeEvent.oldHeight = DPIDOWN(clientHeight);
	eventCallback(this, &event, userData);

	// update saved/old
	clientWidth = newWidth;
	clientHeight = newHeight;

	if (useDirect2D) {
		auto size = D2D1::SizeU(newWidth, newHeight);
		HR(d2dRenderTarget->Resize(size));
	}
}

void wl_Window::onGetMinMaxInfo(MINMAXINFO* mmi)
{
	// min
	if (props.usedFields & wl_kWindowPropMinWidth) {
		mmi->ptMinTrackSize.x = props.minWidth + extraWidth;
	}
	if (props.usedFields & wl_kWindowPropMinHeight) {
		mmi->ptMinTrackSize.y = props.minHeight + extraHeight;
	}

	// max
	if (props.usedFields & wl_kWindowPropMaxWidth) {
		mmi->ptMaxTrackSize.x = props.maxWidth + extraWidth;
	}
	if (props.usedFields & wl_kWindowPropMaxHeight) {
		mmi->ptMaxTrackSize.y = props.maxHeight + extraHeight;
	}
}

void wl_Window::onPaint(wl_Event& event)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);

	DECLSF(dpi);

	event.eventType = wl_kEventTypeWindowRepaint;
	event.repaintEvent.x = DPIDOWN(ps.rcPaint.left);   // is this going to give us off-by-1 pixel errors repainting? due to rounding?
	event.repaintEvent.y = DPIDOWN(ps.rcPaint.top);
	event.repaintEvent.width = DPIDOWN(ps.rcPaint.right) - event.repaintEvent.x;
	event.repaintEvent.height = DPIDOWN(ps.rcPaint.bottom) - event.repaintEvent.y;

	// pass through DPI in either case
	event.repaintEvent.platformContext.dpi = dpi;

	if (useDirect2D) {
		event.repaintEvent.platformContext.d2d.factory = d2dFactory;
		event.repaintEvent.platformContext.d2d.target = d2dRenderTarget;

		d2dRenderTarget->BeginDraw();
		eventCallback(this, &event, userData);
		auto hr = d2dRenderTarget->EndDraw();
		if (hr == D2DERR_RECREATE_TARGET) {
			direct2DCreateTarget();
		}
	}
	else {
		event.repaintEvent.platformContext.gdi.hdc = hdc;
		eventCallback(this, &event, userData);
	}

	EndPaint(hWnd, &ps);
}

void wl_Window::onMouseMove(wl_Event& event, WPARAM wParam, LPARAM lParam, bool *ignored)
{
	DECLSF(dpi);

	if (ignorePostGrabMove) {
		// we just ungrabbed this window, so ignore 1 move message
		ignorePostGrabMove = false;
		*ignored = true;
		return;
	}
	event.eventType = wl_kEventTypeMouse;
	event.mouseEvent.x = DPIDOWN(GET_X_LPARAM(lParam));
	event.mouseEvent.y = DPIDOWN(GET_Y_LPARAM(lParam));
	event.mouseEvent.modifiers = getMouseModifiers(wParam);

	if (!mouseInWindow) {
		// we must set a cursor since our WNDCLASS isn't allowed to have a default
		//    (a class default prevents ad-hoc cursors entirely)
		// (otherwise we'll get random junk coming in from outside)
		// note this also MUST happen before sending the synthetic enter event,
		//  because said event might very well set the mouse cursor, and we'd just be negating it
		setCursor(wl_Cursor::defaultCursor);
		//wl_Cursor::setDefault();
		//cursor = nullptr;
		//SetCursor(defaultCursor);
		//wlw->cursor = nullptr;

		// set the mouse-in-window flag now because some API calls (eg wl_WindowSetCursor) called by event handlers may require it ASAP
		mouseInWindow = true;

		// synthesize an "enter" event before the 1st move event sent
		event.mouseEvent.eventType = wl_kMouseEventTypeMouseEnter;
		eventCallback(this, &event, userData);

		// indicate that we want a WM_MOUSELEAVE event to balance this
		// TODO: can't recall, does this need to be canceled elsewhere?
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.hwndTrack = hWnd;
		tme.dwFlags = TME_LEAVE;
		tme.dwHoverTime = HOVER_DEFAULT;
		TrackMouseEvent(&tme);

		event.handled = false; // reset for next dispatch below
	}

	// set event type down here because might have to overwrite after branch above
	event.mouseEvent.eventType = wl_kMouseEventTypeMouseMove;

	eventCallback(this, &event, userData);
}

void wl_Window::onMouseLeave(wl_Event& event)
{
	event.eventType = wl_kEventTypeMouse;
	event.mouseEvent.eventType = wl_kMouseEventTypeMouseLeave;
	// no other values come with event

	mouseInWindow = false;

	eventCallback(this, &event, userData);
}

// misc util funcs =================================================================

long getWindowStyle(wl_WindowProperties* props, bool isPluginWindow) {
	long dwStyle = WS_OVERLAPPEDWINDOW;
	if (isPluginWindow) {
		dwStyle = WS_CHILD;
	}
	else if (props && (props->usedFields & wl_kWindowPropStyle)) {
		switch (props->style) {
		case wl_kWindowStyleDefault:
			dwStyle = WS_OVERLAPPEDWINDOW;
			break;
		case wl_kWindowStyleFrameless:
			dwStyle = WS_POPUP | WS_BORDER;
			break;
		default:
			printf("wl_WindowCreate: unknown window style\n");
			break;
		}
	}
	return dwStyle;
}

void calcChromeExtra(int* extraWidth, int* extraHeight, DWORD dwStyle, BOOL hasMenu, UINT dpi) {
	const int arbitraryExtent = 500;
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = arbitraryExtent; // just some arbitrary extents -- it's the difference we're interested in
	rect.bottom = arbitraryExtent;

	AdjustWindowRectExForDpi(&rect, dwStyle, hasMenu, 0, dpi);

	*extraWidth = (rect.right - rect.left) - arbitraryExtent;  // left and top will be negative, hence the subtraction (right - left) = outer width
	*extraHeight = (rect.bottom - rect.top) - arbitraryExtent; // bottom - top = outer height
}

unsigned int getKeyModifiers() {
	unsigned int modifiers = 0;
	modifiers |= (GetKeyState(VK_CONTROL) & 0x8000) ? wl_kModifierControl : 0;
	modifiers |= (GetKeyState(VK_MENU) & 0x8000) ? wl_kModifierAlt : 0;
	modifiers |= (GetKeyState(VK_SHIFT) & 0x8000) ? wl_kModifierShift : 0;
	return modifiers;
}

unsigned int getMouseModifiers(WPARAM wParam) {
	unsigned int modifiers = 0;
	auto fwKeys = GET_KEYSTATE_WPARAM(wParam);
	modifiers |= (fwKeys & MK_CONTROL) ? wl_kModifierControl : 0;
	modifiers |= (fwKeys & MK_SHIFT) ? wl_kModifierShift : 0;
	modifiers |= (fwKeys & MK_ALT) ? wl_kModifierAlt : 0;
	return modifiers;
}
