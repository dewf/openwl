#pragma once

#include "../openwl.h"

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
