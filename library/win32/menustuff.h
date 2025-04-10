#pragma once

#include "../openwl.h"

#include <string>
#include <vector>
#include <map>

struct wl_Icon {
    HBITMAP hbitmap;
};

struct wl_Accelerator {
    wl_KeyEnum key;
    unsigned int modifiers;
};

struct wl_Action {
    int id = -1;
    std::string label = "(none)";
    wl_IconRef icon = nullptr;
    wl_AcceleratorRef accel = nullptr;
    //std::vector<wl_MenuItemRef> attachedItems; // to update any menu items when this label/icon/etc changes

    static wl_ActionRef create(int id, const char* label, wl_IconRef icon, wl_AcceleratorRef accel);
    static wl_ActionRef findByID(int id);
};

struct wl_MenuItem {
    wl_ActionRef action = nullptr;
    wl_MenuRef subMenu = nullptr;
};

struct wl_Menu {
    HMENU hmenu;
    std::vector<wl_MenuItemRef> items;
    wl_MenuItemRef addSubMenu(std::string label, wl_MenuRef subMenu);
    wl_MenuItemRef addAction(wl_ActionRef action);
};

struct wl_MenuBar {
    HMENU hmenu;
    std::vector<wl_MenuRef> menus;
    void addMenu(std::string label, wl_MenuRef menu);
    HACCEL generateAcceleratorTable();

    static wl_MenuBarRef create();
};


