#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

LRESULT CALLBACK topLevelWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
