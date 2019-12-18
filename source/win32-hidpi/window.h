#pragma once

#include "../openwl.h"

struct wl_Window {
private:
    HWND hWnd = NULL;
    wl_WindowProperties props;
    void* userData = nullptr;           // for client callback on window events
    IDropTarget* dropTarget = nullptr;  // for dnd
                                        // useful stuff to know for various API calls
    DWORD dwStyle = 0;
    BOOL hasMenu = FALSE;

    int clientWidth = -1;
    int clientHeight = -1;
    int extraWidth = -1;    // difference between client size and window size
    int extraHeight = -1;

    bool mouseInWindow = false;

    wl_CursorRef cursor = nullptr;

    // for D2D only
    ID2D1HwndRenderTarget* d2dRenderTarget = nullptr;

    // filter spurious post-grab-release move event :(
    bool ignorePostGrabMove = false;

    // private methods ============================
	wl_Window();
    void unregisterDropWindow();
public:
    ~wl_Window();
	static wl_WindowRef create(int width, int height, const char* title, void* userData, wl_WindowProperties* props);
    void destroy(); // win32 destruction - the wndproc will be responsible for actual C++ deletion
    void show();
    void direct2DCreateTarget();
};
