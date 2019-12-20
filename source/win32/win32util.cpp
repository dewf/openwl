#include "win32util.h"

#include "globals.h"

#include <stdio.h>

#include "dpimacros.h"

#include <ShellScalingApi.h>

// types
typedef BOOL(__stdcall *TAdjustWindowRectExForDpi)(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi);
typedef UINT(__stdcall *TGetDpiForWindow)(HWND hwnd);

// public vars
const WCHAR* posProbeWindowClass = L"OpenWLPosProbe";

// static vars
static HMODULE user32 = NULL;
static TAdjustWindowRectExForDpi __adjustWindowRectExForDpi = NULL;
static TGetDpiForWindow __getDpiForWindow = NULL;

void win32util_init() {
	registerWindowClass(posProbeWindowClass, DefWindowProc);

	user32 = LoadLibrary(L"User32.dll");
	if (user32) {
		auto f = GetProcAddress(user32, "AdjustWindowRectExForDpi");
		if (f) {
			__adjustWindowRectExForDpi = (TAdjustWindowRectExForDpi)f;
		}
		auto f2 = GetProcAddress(user32, "GetDpiForWindow");
		if (f2) {
			__getDpiForWindow = (TGetDpiForWindow)f2;
		}
	}
}

void registerWindowClass(const WCHAR* windowClass, WNDPROC windowProc) {
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = windowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = NULL; // LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = NULL; // (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL; // MAKEINTRESOURCEW(IDR_MENU1);
	wcex.lpszClassName = windowClass;
	wcex.hIconSm = NULL; // LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);
}

void probeDefaultWindowPos(int *x, int *y, UINT *dpi) {
	auto hWnd = CreateWindow(posProbeWindowClass, L"none", WS_OVERLAPPED, CW_USEDEFAULT, SW_HIDE, 10, 10, NULL, NULL, hInstance, 0);
	*dpi = COMPAT_GetDpiForWindow(hWnd);
	RECT r;
	GetWindowRect(hWnd, &r);
	*x = r.left;
	*y = r.top;
	DestroyWindow(hWnd);
}

BOOL __stdcall COMPAT_AdjustWindowRectExForDpi(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi) {
	if (__adjustWindowRectExForDpi) {
		return __adjustWindowRectExForDpi(lpRect, dwStyle, bMenu, dwExStyle, dpi);
	}
	else {
		// for Win8.1, seems to work fine as-is with no scale multiplication ...
		return AdjustWindowRectEx(lpRect, dwStyle, bMenu, dwExStyle);
	}
}

UINT __stdcall COMPAT_GetDpiForWindow(HWND hwnd) {
	if (__getDpiForWindow) {
		return __getDpiForWindow(hwnd);
	}
	else {
		auto m = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		UINT dpiX, dpiY;
		GetDpiForMonitor(m, MDT_DEFAULT, &dpiX, &dpiY);
		return (dpiX + dpiY) / 2; // well, which should we use?
	}
}
