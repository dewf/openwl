//
// Created by dang on 2/11/18.
//

#ifndef C_CLIENT_UTIL_H
#define C_CLIENT_UTIL_H

#include "../openwl.h"
#include <gdk/gdktypes.h>
#include <gdkmm/dragcontext.h>

unsigned int gdkToWlModifiers(unsigned int gdkState);

unsigned int wlToGdkModifiers(unsigned int modifiers);

wl_MouseButton gdkToWlButton(uint button);

uint wlToGdkButton(wl_MouseButton wlButton);

// assumes only one is set
wl_DropEffect  gdkToWlDropEffectSingle(Gdk::DragAction dragAction);

// create a mask (allow multiple settings -- annoying that they use an enum for this)
unsigned int gdkToWlDropEffectMulti(Gdk::DragAction actions);

Gdk::DragAction wlToGdkDropEffectMulti(unsigned int effectMask);

//const char *targetFromFormat(WLDragFormatEnum format);

#define ONE_BILLION 1000000000
inline double timespecDiff(timespec newer, timespec older) {
    if (newer.tv_nsec < older.tv_nsec) {
        newer.tv_nsec += ONE_BILLION;
        newer.tv_sec -= 1;
    }
    auto diff = (newer.tv_sec - older.tv_sec) * ONE_BILLION + (newer.tv_nsec - older.tv_nsec);
    return (double) diff / ONE_BILLION;
}

#endif //C_CLIENT_UTIL_H
