#pragma once

#include "../openwl.h"

#include <string>
#include <vector>
#include <map>

struct wl_Action {
private:
    int id = -1;
    std::string label = "(none)";
    wl_IconRef icon = nullptr;
    wl_AcceleratorRef accel = nullptr;
    std::vector<wl_MenuItemRef> attachedItems; // to update any menu items when this label/icon/etc changes
public:
    static wl_ActionRef create(int id, const char* label, wl_IconRef icon, wl_AcceleratorRef accel);
    static HACCEL createAccelTable();
    static wl_ActionRef findByID(int id);

    wl_MenuItemRef addToMenu(wl_MenuRef menu);
};
