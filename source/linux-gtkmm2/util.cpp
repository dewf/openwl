//
// Created by dang on 2/11/18.
//

#include "util.h"

unsigned int gdkToWlModifiers(unsigned int gdkState) {
    unsigned int ret = 0;
    ret |= (gdkState & GDK_SHIFT_MASK) ? WLModifier_Shift : 0;
    ret |= (gdkState & GDK_CONTROL_MASK) ? WLModifier_Control : 0;
    ret |= (gdkState & GDK_MOD1_MASK) ? WLModifier_Alt : 0;
//    ret |= (gdkState & GDK_MOD4_MASK) ? WLModifier_Meta : 0);
    return ret;
}

unsigned int wlToGdkModifiers(unsigned int modifiers) {
    unsigned int ret = 0;
    ret |= (modifiers & WLModifier_Shift) ? GDK_SHIFT_MASK : 0;
    ret |= (modifiers & WLModifier_Control) ? GDK_CONTROL_MASK : 0;
    ret |= (modifiers & WLModifier_Alt) ? GDK_MOD1_MASK : 0;
//    ret |= (modifiers & WLModifier_Meta) ? GDK_MOD4_MASK : 0;
    return ret;
}

WLMouseButton gdkToWlButton(uint button) {
    return (button == 1) ? WLMouseButton_Left :
           (button == 2) ? WLMouseButton_Middle :
           (button == 3) ? WLMouseButton_Right :
           WLMouseButton_Other;
}

uint wlToGdkButton(WLMouseButton wlButton) {
    return (wlButton == WLMouseButton_Left) ? 1 :
           (wlButton == WLMouseButton_Middle) ? 2 :
           (wlButton == WLMouseButton_Right) ? 3 :
           0;
}

// assumes only one is set
WLDropEffect  gdkToWlDropEffectSingle(Gdk::DragAction dragAction) {
    return (dragAction == Gdk::ACTION_COPY) ? WLDropEffect_Copy :
           (dragAction == Gdk::ACTION_MOVE) ? WLDropEffect_Move :
           (dragAction == Gdk::ACTION_LINK) ? WLDropEffect_Link :
           WLDropEffect_None;
}

// create a mask (allow multiple settings -- annoying that they use an enum for this)
unsigned int gdkToWlDropEffectMulti(Gdk::DragAction actions) {
    return (unsigned int)
                   ((actions & Gdk::ACTION_COPY) ? WLDropEffect_Copy : 0) |
           ((actions & Gdk::ACTION_MOVE) ? WLDropEffect_Move : 0) |
           ((actions & Gdk::ACTION_LINK) ? WLDropEffect_Link : 0);
}

Gdk::DragAction wlToGdkDropEffectMulti(unsigned int effectMask) {
    return (Gdk::DragAction)
                   ((effectMask & WLDropEffect_Copy) ? Gdk::ACTION_COPY : (Gdk::DragAction)0) |
           ((effectMask & WLDropEffect_Move) ? Gdk::ACTION_MOVE : (Gdk::DragAction)0) |
           ((effectMask & WLDropEffect_Link) ? Gdk::ACTION_LINK : (Gdk::DragAction)0);
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


