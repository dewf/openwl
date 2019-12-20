#include "window.h"

#include "unicodestuff.h"
#include "globals.h"
#include "comstuff.h"

#include "cursor.h"
#include "private_defs.h"
#include "keystuff.h"
#include "action.h"

#include "win32util.h"

#include "MyDropTarget.h"

#include <ShellScalingApi.h>
#include <windowsx.h> // for some macros (GET_X_LPARAM etc)
#include <stdio.h>

#include <set>

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

// static variables
wl_WindowRef wl_Window::lastGrabWindow = nullptr;
std::set<unsigned char> suppressedScanCodes;

// methods ============================================================

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

	// these need to be set by either branch below
	UINT dpi;
	int width, height;

	// create actual win32 window
	HWND hWnd = NULL;
	if (isPluginWindow)
	{
		dpi = GetDpiForWindow(props->nativeParent);
		// for now just assume what was passed in was the correct pixel width (ie, not DIPs)
		width = dipWidth;
		height = dipHeight;

		hWnd = CreateWindowW(topLevelWindowClass, wideTitle.c_str(), dwStyle,
			0, 0,
			width, height, props->nativeParent, nullptr, hInstance, nullptr);
	}
	else {
		// normal top-level window (or frameless)

		// get DPI and default position on whichever monitor Windows wants us on
		int defaultX, defaultY;
		probeDefaultWindowPos(&defaultX, &defaultY, &dpi);

		DECLSF(dpi);
		width = DPIUP(dipWidth);
		height = DPIUP(dipHeight);
		// note: we don't fix props -- the min/max/width/height all stay in DIPs, 
		//   because window might get dragged between monitors of different DPI
		// so just recalc (DPIUP) from props each time (wl_Window::onGetMinMaxInfo)

		calcChromeExtra(&extraWidth, &extraHeight, dwStyle, FALSE, dpi); // FALSE = no menu for now ... will recalc when the time comes

		auto exStyle = (dwStyle & WS_POPUP) ? WS_EX_TOOLWINDOW : 0; // no taskbar button plz

		hWnd = CreateWindowExW(exStyle, topLevelWindowClass, wideTitle.c_str(), dwStyle,
			defaultX, defaultY,
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
	rtprops.dpiX = (FLOAT)dpi;
	rtprops.dpiY = (FLOAT)dpi;
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

void wl_Window::RegisterDropWindow(wl_WindowRef window, IDropTarget** ppDropTarget)
{
	auto pDropTarget = new MyDropTarget(window);

	// acquire a strong lock
	CoLockObjectExternal(pDropTarget, TRUE, FALSE);

	// tell OLE that the window is a drop target
	RegisterDragDrop(window->hWnd, pDropTarget);

	*ppDropTarget = pDropTarget;
}

void wl_Window::UnregisterDropWindow(wl_WindowRef window, IDropTarget* pDropTarget)
{
	// remove drag+drop
	RevokeDragDrop(window->hWnd);

	// remove the strong lock
	CoLockObjectExternal(pDropTarget, FALSE, TRUE);

	// release our own reference
	pDropTarget->Release();
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

void wl_Window::grab()
{
	SetCapture(hWnd); // not saving the previous capture (return value) for now, maybe in the future
	lastGrabWindow = this;  // because ungrab / releasecapture are app-global, no specific handle given
}

// note this is a static method
void wl_Window::ungrab()
{
	ReleaseCapture();
	lastGrabWindow->ignorePostGrabMove = true; // ignoring 1 unwanted move event, which can trigger unwanted redraws / state changes after a mouse up
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

void wl_Window::showContextMenu(int x, int y, wl_MenuRef menu, wl_Event* fromEvent)
{
	DECLSF(dpi);

	int needsRightAlign = GetSystemMetrics(SM_MENUDROPALIGNMENT);
	UINT alignFlags = needsRightAlign ? TPM_RIGHTALIGN : TPM_LEFTALIGN;
	// translate x,y into screen coords
	POINT point;
	point.x = DPIUP(x); // from DIPs to real pixels
	point.y = DPIUP(y);
	ClientToScreen(hWnd, &point);

	// menus are still dumb data structures so we'll leave this here for now (as opposed to a method on wl_MenuRef)
	TrackPopupMenu(menu->hmenu, alignFlags | TPM_TOPALIGN | TPM_LEFTBUTTON, point.x, point.y, 0, hWnd, NULL);
}

void wl_Window::setMenuBar(wl_MenuBarRef menuBar)
{
	hasMenu = TRUE; // use win32 bool constant ... but shouldn't be different from C++ true/false

	// recalc the "extra" (window chrome extents) so that we're dealing with inner client area
	// this is needed for enforcement of min/max sizes as well as just fixing up the client size below
	calcChromeExtra(&extraWidth, &extraHeight, dwStyle, hasMenu, dpi);

	// force resize to preserve client area
	SetWindowPos(hWnd, NULL, 0, 0,
		clientWidth + extraWidth,
		clientHeight + extraHeight,
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);

	// don't set the menu until the very end, otherwise it sends a WM_SIZE message that fucks our calculations up
	SetMenu(hWnd, menuBar->hmenu);
}

void wl_Window::enableDrops(bool enabled)
{
	if (enabled) {
		RegisterDropWindow(this, &dropTarget);
	}
	else {
		UnregisterDropWindow(this, dropTarget);
		dropTarget = nullptr;
	}
}

void wl_Window::sendEvent(wl_Event& event)
{
	eventCallback(this, &event, userData);
}

void wl_Window::setFocus()
{
	SetFocus(hWnd);
}

void wl_Window::screenToClient(LPPOINT p)
{
	DECLSF(dpi);
	ScreenToClient(hWnd, p); // screen is in actual pixels, but we're going to return DIPs
	DPIDOWN_INPLACE(p->x);
	DPIDOWN_INPLACE(p->y);
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
	DECLSF(dpi);

	// min
	if (props.usedFields & wl_kWindowPropMinWidth) {
		mmi->ptMinTrackSize.x = DPIUP(props.minWidth) + extraWidth;
	}
	if (props.usedFields & wl_kWindowPropMinHeight) {
		mmi->ptMinTrackSize.y = DPIUP(props.minHeight) + extraHeight;
	}

	// max
	if (props.usedFields & wl_kWindowPropMaxWidth) {
		mmi->ptMaxTrackSize.x = DPIUP(props.maxWidth) + extraWidth;
	}
	if (props.usedFields & wl_kWindowPropMaxHeight) {
		mmi->ptMaxTrackSize.y = DPIUP(props.maxHeight) + extraHeight;
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
		wl_Cursor::defaultCursor->set();
		cursor = nullptr;

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

void wl_Window::onMouseButton(wl_Event& event, UINT message, WPARAM wParam, LPARAM lParam)
{
	event.eventType = wl_kEventTypeMouse;

	DECLSF(dpi);

	switch (message) {
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		event.mouseEvent.eventType = wl_kMouseEventTypeMouseDown;
		break;
	default:
		event.mouseEvent.eventType = wl_kMouseEventTypeMouseUp;
	}

	switch (message) {
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		event.mouseEvent.button = wl_kMouseButtonLeft;
		break;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		event.mouseEvent.button = wl_kMouseButtonMiddle;
		break;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		event.mouseEvent.button = wl_kMouseButtonRight;
		break;
	}
	event.mouseEvent.x = DPIDOWN(GET_X_LPARAM(lParam));
	event.mouseEvent.y = DPIDOWN(GET_Y_LPARAM(lParam));

	event.mouseEvent.modifiers = getMouseModifiers(wParam);

	eventCallback(this, &event, userData);
}

void wl_Window::onChar(wl_Event& event, WPARAM wParam, LPARAM lParam)
{
	unsigned char scanCode = (lParam >> 16) & 0xFF;
	bool found = (suppressedScanCodes.find(scanCode) != suppressedScanCodes.end());
	if (!found) {
		static wchar_t utf16_buffer[10];
		static int buf_len = 0;
		utf16_buffer[buf_len++] = (wchar_t)wParam;
		if (wParam >= 0xD800 && wParam <= 0xDFFF) {
			if (buf_len == 2) {
				utf16_buffer[2] = 0;
				auto utf8 = wstring_to_utf8(utf16_buffer);

				event.eventType = wl_kEventTypeKey;
				event.keyEvent.eventType = wl_kKeyEventTypeChar;
				event.keyEvent.key = wl_kKeyUnknown;
				event.keyEvent.modifiers = 0; // not used here

				event.keyEvent.string = utf8.c_str();
				eventCallback(this, &event, userData);

				buf_len = 0;
			}
		}
		else {
			// single char
			utf16_buffer[1] = 0;
			auto utf8 = wstring_to_utf8(utf16_buffer);

			event.eventType = wl_kEventTypeKey;
			event.keyEvent.eventType = wl_kKeyEventTypeChar;
			event.keyEvent.key = wl_kKeyUnknown;
			event.keyEvent.modifiers = 0; // not used here

			event.keyEvent.string = utf8.c_str();
			eventCallback(this, &event, userData);

			buf_len = 0;
		}
	}
}

void wl_Window::onKey(wl_Event& event, UINT message, WPARAM wParam, LPARAM lParam)
{
	unsigned char scanCode = (lParam >> 16) & 0xFF;
	bool extended = ((lParam >> 24) & 0x1) ? true : false;

	KeyInfo* value = keyMap[0]; // wl_kKeyUnknown
	auto found = keyMap.find(wParam); // wParam = win32 virtual key
	if (found != keyMap.end()) {
		value = found->second;
	}
	if (value->key != wl_kKeyUnknown) {
		event.eventType = wl_kEventTypeKey;
		event.keyEvent.eventType = (message == WM_KEYDOWN ? wl_kKeyEventTypeDown : wl_kKeyEventTypeUp);
		event.keyEvent.key = value->key;
		event.keyEvent.modifiers = getKeyModifiers(); // should probably be tracking ctrl/alt/shift state as we go, instead of grabbing instantaneously here
		event.keyEvent.string = value->stringRep;

		// use scancode, extended value, etc to figure out location (left/right/numpad/etc)
		if (value->knownLocation >= 0) {
			event.keyEvent.location = (wl_KeyLocation)value->knownLocation;
		}
		else {
			event.keyEvent.location = locationForKey(value->key, scanCode, extended);
		}

		eventCallback(this, &event, userData);

		if (message == WM_KEYDOWN) { // is this needed for KEYUP too?
			// does the WM_CHAR need to be suppressed?
			// just add it to the blacklisted set (redundant but sufficient)
			// could/should use a queue instead, but not every WM_CHAR has a corresponding/prior WM_KEYDOWN event
			if (value->suppressCharEvent) {
				suppressedScanCodes.insert(scanCode);
			}
		}
	}
	else {
		printf("### unrecognized: VK %zd, scan %d [%s]\n", wParam, scanCode, extended ? "ext" : "--");
	}
}

void wl_Window::onAction(wl_Event& event, int actionID)
{
	auto action = wl_Action::findByID(actionID);
	if (action) {
		event.eventType = wl_kEventTypeAction;
		event.actionEvent.action = action; // aka value
		event.actionEvent.id = actionID;
		eventCallback(this, &event, userData);
	}
}

void wl_Window::onDPIChanged(UINT newDPI, RECT *suggestedRect)
{
	dpi = newDPI;

	// recalc extraWidth/extraHeight for resize constraints
	calcChromeExtra(&extraWidth, &extraHeight, dwStyle, hasMenu, dpi);

	auto x = suggestedRect->left;
	auto y = suggestedRect->top;
	auto width = suggestedRect->right - suggestedRect->left;
	auto height = suggestedRect->bottom - suggestedRect->top;
	SetWindowPos(hWnd, HWND_TOP, x, y, width, height, 0);

	// recreate target with new DPI
	direct2DCreateTarget();
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
