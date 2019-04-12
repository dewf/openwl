//
// Created by dang on 2/11/18.
//

#include "keystuff.h"

std::map<guint, keyInfo *> keyMap;
std::map<WLKeyEnum, keyInfo *> reverseKeyMap;

keyInfo keyData[] = {
        { XK_VoidSymbol, XK_VoidSymbol, WLKey_Unknown, "(unknown)", false },
        //
        { XK_Escape, XK_VoidSymbol, WLKey_Escape, "Escape", true },
        { XK_Tab, XK_VoidSymbol, WLKey_Tab, "Tab", true },
        { XK_BackSpace, XK_VoidSymbol, WLKey_Backspace, "Backspace", true },
        { XK_Return, XK_VoidSymbol, WLKey_Return, "Return", true },
        { XK_space, XK_VoidSymbol, WLKey_Space, "Space", false },
        //
        { XK_F1, XK_VoidSymbol, WLKey_F1, "F1", false },
        { XK_F2, XK_VoidSymbol, WLKey_F2, "F2", false },
        { XK_F3, XK_VoidSymbol, WLKey_F3, "F3", false },
        { XK_F4, XK_VoidSymbol, WLKey_F4, "F4", false },
        { XK_F5, XK_VoidSymbol, WLKey_F5, "F5", false },
        { XK_F6, XK_VoidSymbol, WLKey_F6, "F6", false },
        { XK_F7, XK_VoidSymbol, WLKey_F7, "F7", false },
        { XK_F8, XK_VoidSymbol, WLKey_F8, "F8", false },
        { XK_F9, XK_VoidSymbol, WLKey_F9, "F9", false },
        { XK_F10, XK_VoidSymbol, WLKey_F10, "F10", false },
        { XK_F11, XK_VoidSymbol, WLKey_F11, "F11", false },
        { XK_F12, XK_VoidSymbol, WLKey_F12, "F12", false },
        { XK_F13, XK_VoidSymbol, WLKey_F13, "F13", false },
        { XK_F14, XK_VoidSymbol, WLKey_F14, "F14", false },
        { XK_F15, XK_VoidSymbol, WLKey_F15, "F15", false },
        { XK_F16, XK_VoidSymbol, WLKey_F16, "F16", false },
        { XK_F17, XK_VoidSymbol, WLKey_F17, "F17", false },
        { XK_F18, XK_VoidSymbol, WLKey_F18, "F18", false },
        { XK_F19, XK_VoidSymbol, WLKey_F19, "F19", false },
        //
        { XK_0, XK_VoidSymbol, WLKey_0, "0", false },
        { XK_1, XK_VoidSymbol, WLKey_1, "1", false },
        { XK_2, XK_VoidSymbol, WLKey_2, "2", false },
        { XK_3, XK_VoidSymbol, WLKey_3, "3", false },
        { XK_4, XK_VoidSymbol, WLKey_4, "4", false },
        { XK_5, XK_VoidSymbol, WLKey_5, "5", false },
        { XK_6, XK_VoidSymbol, WLKey_6, "6", false },
        { XK_7, XK_VoidSymbol, WLKey_7, "7", false },
        { XK_8, XK_VoidSymbol, WLKey_8, "8", false },
        { XK_9, XK_VoidSymbol, WLKey_9, "9", false },
        //
        { XK_a, XK_A, WLKey_A, "A", false },
        { XK_b, XK_B, WLKey_B, "B", false },
        { XK_c, XK_C, WLKey_C, "C", false },
        { XK_d, XK_D, WLKey_D, "D", false },
        { XK_e, XK_E, WLKey_E, "E", false },
        { XK_f, XK_F, WLKey_F, "F", false },
        { XK_g, XK_G, WLKey_G, "G", false },
        { XK_h, XK_H, WLKey_H, "H", false },
        { XK_i, XK_I, WLKey_I, "I", false },
        { XK_j, XK_J, WLKey_J, "J", false },
        { XK_k, XK_K, WLKey_K, "K", false },
        { XK_l, XK_L, WLKey_L, "L", false },
        { XK_m, XK_M, WLKey_M, "M", false },
        { XK_n, XK_N, WLKey_N, "N", false },
        { XK_o, XK_O, WLKey_O, "O", false },
        { XK_p, XK_P, WLKey_P, "P", false },
        { XK_q, XK_Q, WLKey_Q, "Q", false },
        { XK_r, XK_R, WLKey_R, "R", false },
        { XK_s, XK_S, WLKey_S, "S", false },
        { XK_t, XK_T, WLKey_T, "T", false },
        { XK_u, XK_U, WLKey_U, "U", false },
        { XK_v, XK_V, WLKey_V, "V", false },
        { XK_w, XK_W, WLKey_W, "W", false },
        { XK_x, XK_X, WLKey_X, "X", false },
        { XK_y, XK_Y, WLKey_Y, "Y", false },
        { XK_z, XK_Z, WLKey_Z, "Z", false },
        // modifiers
        { XK_Control_L, XK_VoidSymbol, WLKey_Control, "Control", false, WLKeyLocation_Left },
        { XK_Control_R, XK_VoidSymbol, WLKey_Control, "Control", false, WLKeyLocation_Right },
        { XK_Shift_L, XK_VoidSymbol, WLKey_Shift, "Shift", false, WLKeyLocation_Left },
        { XK_Shift_R, XK_VoidSymbol, WLKey_Shift, "Shift", false, WLKeyLocation_Right },
        { XK_Alt_L, XK_VoidSymbol, WLKey_AltOption, "Alt/Option", false, WLKeyLocation_Left },
        { XK_Alt_R, XK_VoidSymbol, WLKey_AltOption, "Alt/Option", false, WLKeyLocation_Right },
        { XK_Super_L, XK_VoidSymbol, WLKey_WinCommand, "Win/Command", false, WLKeyLocation_Left },
        { XK_Super_R, XK_VoidSymbol, WLKey_WinCommand, "Win/Command", false, WLKeyLocation_Right },
//        WLKey_Fn,
        // home/end block
        { XK_Insert, XK_VoidSymbol, WLKey_Insert, "Insert", false },
        { XK_Delete, XK_VoidSymbol, WLKey_Delete, "Delete", false },
        { XK_Page_Up, XK_VoidSymbol, WLKey_PageUp, "PageUp", false },
        { XK_Page_Down, XK_VoidSymbol, WLKey_PageDown, "PageDown", false },
        { XK_Home, XK_VoidSymbol, WLKey_Home, "Home", false },
        { XK_End, XK_VoidSymbol, WLKey_End, "End", false },
        // arrow keys
        { XK_Left, XK_VoidSymbol, WLKey_LeftArrow, "LeftArrow", false },
        { XK_Up, XK_VoidSymbol, WLKey_UpArrow, "UpArrow", false },
        { XK_Right, XK_VoidSymbol, WLKey_RightArrow, "RightArrow", false },
        { XK_Down, XK_VoidSymbol, WLKey_DownArrow, "DownArrow", false },
        // keypad numbers
        { XK_KP_0, XK_VoidSymbol, WLKey_KP_0, "KP-0", false, WLKeyLocation_NumPad },
        { XK_KP_1, XK_VoidSymbol, WLKey_KP_1, "KP-1", false, WLKeyLocation_NumPad },
        { XK_KP_2, XK_VoidSymbol, WLKey_KP_2, "KP-2", false, WLKeyLocation_NumPad },
        { XK_KP_3, XK_VoidSymbol, WLKey_KP_3, "KP-3", false, WLKeyLocation_NumPad },
        { XK_KP_4, XK_VoidSymbol, WLKey_KP_4, "KP-4", false, WLKeyLocation_NumPad },
        { XK_KP_5, XK_VoidSymbol, WLKey_KP_5, "KP-5", false, WLKeyLocation_NumPad },
        { XK_KP_6, XK_VoidSymbol, WLKey_KP_6, "KP-6", false, WLKeyLocation_NumPad },
        { XK_KP_7, XK_VoidSymbol, WLKey_KP_7, "KP-7", false, WLKeyLocation_NumPad },
        { XK_KP_8, XK_VoidSymbol, WLKey_KP_8, "KP-8", false, WLKeyLocation_NumPad },
        { XK_KP_9, XK_VoidSymbol, WLKey_KP_9, "KP-9", false, WLKeyLocation_NumPad },
        // keypad ops
        { XK_Clear, XK_VoidSymbol, WLKey_KP_Clear, "KP Clear", false, WLKeyLocation_NumPad },
        //
        { XK_KP_Equal, XK_VoidSymbol, WLKey_KP_Equals, "KP Equals", false, WLKeyLocation_NumPad },
        { XK_KP_Divide, XK_VoidSymbol, WLKey_KP_Divide, "KP Divide", false, WLKeyLocation_NumPad },
        { XK_KP_Multiply, XK_VoidSymbol, WLKey_KP_Multiply, "KP Multiply", false, WLKeyLocation_NumPad },
        { XK_KP_Subtract, XK_VoidSymbol, WLKey_KP_Subtract, "KP Subtract", false, WLKeyLocation_NumPad },
        { XK_KP_Add, XK_VoidSymbol, WLKey_KP_Add, "KP Add", false, WLKeyLocation_NumPad },
        { XK_KP_Enter, XK_VoidSymbol, WLKey_KP_Enter, "KP Enter", true, WLKeyLocation_NumPad },
        { XK_KP_Decimal, XK_VoidSymbol, WLKey_KP_Decimal, "KP Decimal", false, WLKeyLocation_NumPad },
        // locks
        { XK_Caps_Lock, XK_VoidSymbol, WLKey_CapsLock, "CapsLock", false },
        { XK_Num_Lock, XK_VoidSymbol, WLKey_NumLock, "NumLock", false },
        { XK_Scroll_Lock, XK_VoidSymbol, WLKey_ScrollLock, "ScrollLock", false },
        // misc
        { XK_Print, XK_VoidSymbol, WLKey_PrintScreen, "PrintScreen", false },
        { XK_Pause, XK_VoidSymbol, WLKey_Pause, "Pause", false },
        { XK_Break, XK_VoidSymbol, WLKey_Cancel, "Break (Ctrl-Pause)", false },
//        { XK_Cancel, XK_VoidSymbol, WLKey_Cancel, "Cancel", false },
        // media
        { XF86XK_AudioMute, XK_VoidSymbol, WLKey_MediaMute, "MediaMute", false },
        { XF86XK_AudioLowerVolume, XK_VoidSymbol, WLKey_MediaVolumeDown, "MediaVolumeDown", false },
        { XF86XK_AudioRaiseVolume, XK_VoidSymbol, WLKey_MediaVolumeUp, "MediaVolumeUp", false },
        { XF86XK_AudioNext, XK_VoidSymbol, WLKey_MediaNext, "MediaNext", false },
        { XF86XK_AudioPrev, XK_VoidSymbol, WLKey_MediaPrev, "MediaPrev", false },
        { XF86XK_AudioStop, XK_VoidSymbol, WLKey_MediaStop, "MediaStop", false },
        { XF86XK_AudioPlay, XK_VoidSymbol, WLKey_MediaPlayPause, "MediaPlayPause", false }
};

void initKeymap() {
    int count = sizeof(keyData) / sizeof(keyInfo);
    for (int i=0; i< count; i++) {
        keyInfo *info = &keyData[i];
        keyMap[info->lowerSym] = info;
        if (info->upperSym != XK_VoidSymbol) {
            keyMap[info->upperSym] = info;
        }
        reverseKeyMap[info->key] = info;
    }
}
