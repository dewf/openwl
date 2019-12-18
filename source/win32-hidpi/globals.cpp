#include "globals.h"

HINSTANCE hInstance = NULL; // set by DllMain - need to find a way to set it for static library compilation as well
const WCHAR* topLevelWindowClass = L"OpenWLTopLevel";
const WCHAR* appGlobalWindowClass = L"OpenWLAppGlobal";

DWORD mainThreadID;

HWND appGlobalWindow;

// client-supplied callback
wl_EventCallback eventCallback = nullptr;
