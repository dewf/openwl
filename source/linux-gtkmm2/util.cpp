//
// Created by dang on 2/11/18.
//

#include "util.h"

unsigned int gdkToWlModifiers(unsigned int gdkState) {
    unsigned int ret = 0;
    ret |= (gdkState & GDK_SHIFT_MASK) ? wl_kModifierShift : 0;
    ret |= (gdkState & GDK_CONTROL_MASK) ? wl_kModifierControl : 0;
    ret |= (gdkState & GDK_MOD1_MASK) ? wl_kModifierAlt : 0;
//    ret |= (gdkState & GDK_MOD4_MASK) ? wl_kModifierMeta : 0);
    return ret;
}

unsigned int wlToGdkModifiers(unsigned int modifiers) {
    unsigned int ret = 0;
    ret |= (modifiers & wl_kModifierShift) ? GDK_SHIFT_MASK : 0;
    ret |= (modifiers & wl_kModifierControl) ? GDK_CONTROL_MASK : 0;
    ret |= (modifiers & wl_kModifierAlt) ? GDK_MOD1_MASK : 0;
//    ret |= (modifiers & wl_kModifierMeta) ? GDK_MOD4_MASK : 0;
    return ret;
}

wl_MouseButton gdkToWlButton(uint button) {
    return (button == 1) ? wl_kMouseButtonLeft :
           (button == 2) ? wl_kMouseButtonMiddle :
           (button == 3) ? wl_kMouseButtonRight :
           wl_kMouseButtonOther;
}

uint wlToGdkButton(wl_MouseButton wlButton) {
    return (wlButton == wl_kMouseButtonLeft) ? 1 :
           (wlButton == wl_kMouseButtonMiddle) ? 2 :
           (wlButton == wl_kMouseButtonRight) ? 3 :
           0;
}

// assumes only one is set
wl_DropEffect  gdkToWlDropEffectSingle(Gdk::DragAction dragAction) {
    return (dragAction == Gdk::ACTION_COPY) ? wl_kDropEffectCopy :
           (dragAction == Gdk::ACTION_MOVE) ? wl_kDropEffectMove :
           (dragAction == Gdk::ACTION_LINK) ? wl_kDropEffectLink :
           wl_kDropEffectNone;
}

// create a mask (allow multiple settings -- annoying that they use an enum for this)
unsigned int gdkToWlDropEffectMulti(Gdk::DragAction actions) {
    return (unsigned int)
                   ((actions & Gdk::ACTION_COPY) ? wl_kDropEffectCopy : 0) |
           ((actions & Gdk::ACTION_MOVE) ? wl_kDropEffectMove : 0) |
           ((actions & Gdk::ACTION_LINK) ? wl_kDropEffectLink : 0);
}

Gdk::DragAction wlToGdkDropEffectMulti(unsigned int effectMask) {
    return (Gdk::DragAction)
                   ((effectMask & wl_kDropEffectCopy) ? Gdk::ACTION_COPY : (Gdk::DragAction)0) |
           ((effectMask & wl_kDropEffectMove) ? Gdk::ACTION_MOVE : (Gdk::DragAction)0) |
           ((effectMask & wl_kDropEffectLink) ? Gdk::ACTION_LINK : (Gdk::DragAction)0);
}

bool endswith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

//const char *targetFromFormat(WLDragFormatEnum format) {
//    switch(format) {
//        case WLDragFormat_TextUTF8:
//            return "UTF8_STRING"; //"text/plain;charset=utf-8";
//        case WLDragFormat_Files:
//            return "text/uri-list";
//        default: // or _None:
//            return "(none)";
//    }
//}


