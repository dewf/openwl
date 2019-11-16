#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <map>
#include <vector>
#include <set>
#include <mutex>
#include <condition_variable>

#include "../openwl.h"

extern HINSTANCE hInstance; // set by DllMain
extern const WCHAR *szWindowClass;

extern DWORD mainThreadID;

enum Win32MessageEnum: UINT {
    Nothing = WM_USER,
	// custom window messages here
	AppGlobalMsgBegin = Nothing + 0x500,
	// custom app (main thread) messages here
	WM_WLTimerMessage,
    WM_WLMainThreadExecMsg,
};

// globals
extern std::map<int, wl_ActionRef> actionMap;
extern std::vector<ACCEL> acceleratorList;
extern std::set<unsigned char> suppressedScanCodes;

// timer stuff
extern LARGE_INTEGER perfCounterTicksPerSecond;

// direct2D stuff
extern bool useDirect2D;
extern ID2D1Factory1 *d2dFactory;
extern IDWriteFactory *writeFactory;
void d2dCreateTarget(wl_WindowRef wlw);

// fwd decls for wl_ExecuteOnMainThread
struct MainThreadExecItem {

    wl_VoidCallback callback;
    void *data;
    std::condition_variable& execCond;
};
void ExecuteMainItem(MainThreadExecItem *item);

// client-supplied callback
extern wl_EventCallback eventCallback;

