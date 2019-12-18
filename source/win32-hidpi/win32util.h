#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void registerWindowClass(const WCHAR* windowClass, WNDPROC windowProc);
