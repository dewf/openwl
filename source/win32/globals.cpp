#include "globals.h"

#include "private_defs.h"
#include <cassert>

HINSTANCE hInstance = NULL; // set by DllMain
const WCHAR *szWindowClass = L"OpenWLTopLevel";

// globals
std::map<int, wl_ActionRef> actionMap;
std::vector<ACCEL> acceleratorList;
std::set<unsigned char> suppressedScanCodes;

// timer stuff
LARGE_INTEGER perfCounterTicksPerSecond = { 0 };

// direct2D stuff
bool useDirect2D = false;
ID2D1Factory1 *d2dFactory = nullptr;
// IDWriteFactory *writeFactory = nullptr; // really no need for this in OpenWL

// fwd decls for wl_ExecuteOnMainThread
//struct MainThreadExecItem;
//void ExecuteMainItem(MainThreadExecItem *item);

// client-supplied callback
wl_EventCallback eventCallback = nullptr;

void d2dCreateTarget(wl_WindowRef wlw) {
	ID2D1RenderTarget *oldTarget = wlw->d2dRenderTarget;
	if (wlw->d2dRenderTarget) {
		wlw->d2dRenderTarget->Release();
		wlw->d2dRenderTarget = nullptr;
	}
	// Create a Direct2D render target
	auto rtprops = D2D1::RenderTargetProperties();
	auto hrtprops = D2D1::HwndRenderTargetProperties(wlw->hwnd, D2D1::SizeU(wlw->clientWidth, wlw->clientHeight));
	auto hr = d2dFactory->CreateHwndRenderTarget(&rtprops, &hrtprops, &wlw->d2dRenderTarget);
	assert(hr == 0);

	// event
	wl_Event event;
	event.eventType = wl_kEventTypeD2DTargetRecreated;
	event.d2dTargetRecreatedEvent.newTarget = wlw->d2dRenderTarget;
	event.d2dTargetRecreatedEvent.oldTarget = oldTarget;
	event.handled = false;
	eventCallback(wlw, &event, wlw->userData);
	// don't care if it was handled or not
}
