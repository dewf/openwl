#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

LRESULT CALLBACK appGlobalWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK topLevelWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
