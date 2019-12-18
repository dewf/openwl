#pragma once

#include "../openwl.h"

//#include <vector>

struct wl_EventPrivate {
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    wl_EventPrivate(UINT message, WPARAM wParam, LPARAM lParam) :
        message(message), wParam(wParam), lParam(lParam)
    {
        //
    }
};

struct wl_Icon {
    HBITMAP hbitmap;
};

struct wl_Accelerator {
    wl_KeyEnum key;
    unsigned int modifiers;
};

struct wl_MenuBar {
    HMENU hmenu;
};
struct wl_Menu {
    HMENU hmenu;
};
struct wl_MenuItem {
    wl_ActionRef action;
    wl_MenuRef subMenu;
};
