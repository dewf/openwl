#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void registerWindowClass(const WCHAR* windowClass, WNDPROC windowProc);
void probeDefaultWindowPos(int *x, int *y, UINT *dpi);

BOOL __stdcall COMPAT_AdjustWindowRectExForDpi(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi);
UINT __stdcall COMPAT_GetDpiForWindow(HWND hwnd);

void win32util_init();
