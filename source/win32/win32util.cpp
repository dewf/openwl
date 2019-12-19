#include "win32util.h"

#include "globals.h"

#include <stdio.h>

const WCHAR* posProbeWindowClass = L"OpenWLPosProbe";

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
	*dpi = GetDpiForWindow(hWnd);
	RECT r;
	GetWindowRect(hWnd, &r);
	*x = r.left;
	*y = r.top;
	DestroyWindow(hWnd);
}

void win32util_init() {
	registerWindowClass(posProbeWindowClass, DefWindowProc);
}