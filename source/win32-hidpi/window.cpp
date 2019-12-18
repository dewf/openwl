#include "window.h"

#include "unicodestuff.h"
#include "globals.h"
#include "comstuff.h"

// fwd decls
long getWindowStyle(wl_WindowProperties* props, bool isPluginWindow);
void calcChromeExtra(int* extraWidth, int* extraHeight, DWORD dwStyle, BOOL hasMenu);

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

wl_WindowRef wl_Window::create(int width, int height, const char* title, void* userData, wl_WindowProperties* props)
{
	auto wideTitle = title ? utf8_to_wstring(title) : L"(UNTITLED)";

	int extraWidth = 0;
	int extraHeight = 0;

	bool isPluginWindow = (props &&
		(props->usedFields & wl_kWindowPropStyle) &&
		(props->usedFields & wl_kWindowPropNativeParent) &&
		(props->style == wl_kWindowStylePluginWindow));

	auto dwStyle = getWindowStyle(props, isPluginWindow);

	HWND hWnd = NULL;
	if (isPluginWindow)
	{
		hWnd = CreateWindowW(topLevelWindowClass, wideTitle.c_str(), dwStyle,
			0, 0,
			width, height, props->nativeParent, nullptr, hInstance, nullptr);
	}
	else {
		// normal top-level window (or frameless)
		calcChromeExtra(&extraWidth, &extraHeight, dwStyle, FALSE); // FALSE = no menu for now ... will recalc when the time comes

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

void wl_Window::destroy()
{
	unregisterDropWindow();
	DestroyWindow(hWnd);
}

void wl_Window::show()
{
	ShowWindow(hWnd, SW_SHOWNORMAL); // might need to use a different cmd based on whether first time or not
	UpdateWindow(hWnd);
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

void calcChromeExtra(int* extraWidth, int* extraHeight, DWORD dwStyle, BOOL hasMenu) {
	const int arbitraryExtent = 500;
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = arbitraryExtent; // just some arbitrary extents -- it's the difference we're interested in
	rect.bottom = arbitraryExtent;
	AdjustWindowRect(&rect, dwStyle, hasMenu);
	*extraWidth = (rect.right - rect.left) - arbitraryExtent;  // left and top will be negative, hence the subtraction (right - left) = outer width
	*extraHeight = (rect.bottom - rect.top) - arbitraryExtent; // bottom - top = outer height
}
