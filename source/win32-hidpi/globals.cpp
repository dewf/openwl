#include "globals.h"

HINSTANCE hInstance = NULL; // set by DllMain - need to find a way to set it for static library compilation as well
const WCHAR* topLevelWindowClass = L"OpenWLTopLevel";
const WCHAR* appGlobalWindowClass = L"OpenWLAppGlobal";

DWORD mainThreadID;

HWND appGlobalWindow;

// client-supplied callback
wl_EventCallback eventCallback = nullptr;

// timer stuff
LARGE_INTEGER perfCounterTicksPerSecond = { 0 };

// direct2D stuff
bool useDirect2D = false;
ID2D1Factory1* d2dFactory = nullptr;
