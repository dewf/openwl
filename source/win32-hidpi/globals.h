#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "../openwl.h"

extern HINSTANCE hInstance; // set by DllMain
extern const WCHAR* topLevelWindowClass;
extern const WCHAR* appGlobalWindowClass;

extern DWORD mainThreadID;

extern HWND appGlobalWindow; // for receiving messages that don't belong to any window (see comments at point of creation for more)

// client-supplied callback
extern wl_EventCallback eventCallback;
