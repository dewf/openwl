#include "window.h"

#include "unicodestuff.h"
#include "globals.h"
#include "comstuff.h"

#include <ShellScalingApi.h>

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
