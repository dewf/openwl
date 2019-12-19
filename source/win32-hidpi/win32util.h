#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void registerWindowClass(const WCHAR* windowClass, WNDPROC windowProc);
void probeDefaultWindowPos(int *x, int *y, UINT *dpi);

void win32util_init();