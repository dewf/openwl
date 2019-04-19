//
// Created by dang on 2/11/18.
//

#include "keystuff.h"

std::map<guint, keyInfo *> keyMap;
std::map<wl_KeyEnum, keyInfo *> reverseKeyMap;

keyInfo keyData[] = {
        { XK_VoidSymbol, XK_VoidSymbol, wl_kKeyUnknown, "(unknown)", false },
        //
        { XK_Escape, XK_VoidSymbol, wl_kKeyEscape, "Escape", true },
        { XK_Tab, XK_VoidSymbol, wl_kKeyTab, "Tab", true },
        { XK_BackSpace, XK_VoidSymbol, wl_kKeyBackspace, "Backspace", true },
        { XK_Return, XK_VoidSymbol, wl_kKeyReturn, "Return", true },
        { XK_space, XK_VoidSymbol, wl_kKeySpace, "Space", false },
        //
        { XK_F1, XK_VoidSymbol, wl_kKeyF1, "F1", false },
        { XK_F2, XK_VoidSymbol, wl_kKeyF2, "F2", false },
        { XK_F3, XK_VoidSymbol, wl_kKeyF3, "F3", false },
        { XK_F4, XK_VoidSymbol, wl_kKeyF4, "F4", false },
        { XK_F5, XK_VoidSymbol, wl_kKeyF5, "F5", false },
        { XK_F6, XK_VoidSymbol, wl_kKeyF6, "F6", false },
        { XK_F7, XK_VoidSymbol, wl_kKeyF7, "F7", false },
        { XK_F8, XK_VoidSymbol, wl_kKeyF8, "F8", false },
        { XK_F9, XK_VoidSymbol, wl_kKeyF9, "F9", false },
        { XK_F10, XK_VoidSymbol, wl_kKeyF10, "F10", false },
        { XK_F11, XK_VoidSymbol, wl_kKeyF11, "F11", false },
        { XK_F12, XK_VoidSymbol, wl_kKeyF12, "F12", false },
        { XK_F13, XK_VoidSymbol, wl_kKeyF13, "F13", false },
        { XK_F14, XK_VoidSymbol, wl_kKeyF14, "F14", false },
        { XK_F15, XK_VoidSymbol, wl_kKeyF15, "F15", false },
        { XK_F16, XK_VoidSymbol, wl_kKeyF16, "F16", false },
        { XK_F17, XK_VoidSymbol, wl_kKeyF17, "F17", false },
        { XK_F18, XK_VoidSymbol, wl_kKeyF18, "F18", false },
        { XK_F19, XK_VoidSymbol, wl_kKeyF19, "F19", false },
        //
        { XK_0, XK_VoidSymbol, wl_kKey0, "0", false },
        { XK_1, XK_VoidSymbol, wl_kKey1, "1", false },
        { XK_2, XK_VoidSymbol, wl_kKey2, "2", false },
        { XK_3, XK_VoidSymbol, wl_kKey3, "3", false },
        { XK_4, XK_VoidSymbol, wl_kKey4, "4", false },
        { XK_5, XK_VoidSymbol, wl_kKey5, "5", false },
        { XK_6, XK_VoidSymbol, wl_kKey6, "6", false },
        { XK_7, XK_VoidSymbol, wl_kKey7, "7", false },
        { XK_8, XK_VoidSymbol, wl_kKey8, "8", false },
        { XK_9, XK_VoidSymbol, wl_kKey9, "9", false },
        //
        { XK_a, XK_A, wl_kKeyA, "A", false },
        { XK_b, XK_B, wl_kKeyB, "B", false },
        { XK_c, XK_C, wl_kKeyC, "C", false },
        { XK_d, XK_D, wl_kKeyD, "D", false },
        { XK_e, XK_E, wl_kKeyE, "E", false },
        { XK_f, XK_F, wl_kKeyF, "F", false },
        { XK_g, XK_G, wl_kKeyG, "G", false },
        { XK_h, XK_H, wl_kKeyH, "H", false },
        { XK_i, XK_I, wl_kKeyI, "I", false },
        { XK_j, XK_J, wl_kKeyJ, "J", false },
        { XK_k, XK_K, wl_kKeyK, "K", false },
        { XK_l, XK_L, wl_kKeyL, "L", false },
        { XK_m, XK_M, wl_kKeyM, "M", false },
        { XK_n, XK_N, wl_kKeyN, "N", false },
        { XK_o, XK_O, wl_kKeyO, "O", false },
        { XK_p, XK_P, wl_kKeyP, "P", false },
        { XK_q, XK_Q, wl_kKeyQ, "Q", false },
        { XK_r, XK_R, wl_kKeyR, "R", false },
        { XK_s, XK_S, wl_kKeyS, "S", false },
        { XK_t, XK_T, wl_kKeyT, "T", false },
        { XK_u, XK_U, wl_kKeyU, "U", false },
        { XK_v, XK_V, wl_kKeyV, "V", false },
        { XK_w, XK_W, wl_kKeyW, "W", false },
        { XK_x, XK_X, wl_kKeyX, "X", false },
        { XK_y, XK_Y, wl_kKeyY, "Y", false },
        { XK_z, XK_Z, wl_kKeyZ, "Z", false },
        // modifiers
        { XK_Control_L, XK_VoidSymbol, wl_kKeyControl, "Control", false, wl_kKeyLocationLeft },
        { XK_Control_R, XK_VoidSymbol, wl_kKeyControl, "Control", false, wl_kKeyLocationRight },
        { XK_Shift_L, XK_VoidSymbol, wl_kKeyShift, "Shift", false, wl_kKeyLocationLeft },
        { XK_Shift_R, XK_VoidSymbol, wl_kKeyShift, "Shift", false, wl_kKeyLocationRight },
        { XK_Alt_L, XK_VoidSymbol, wl_kKeyAltOption, "Alt/Option", false, wl_kKeyLocationLeft },
        { XK_Alt_R, XK_VoidSymbol, wl_kKeyAltOption, "Alt/Option", false, wl_kKeyLocationRight },
        { XK_Super_L, XK_VoidSymbol, wl_kKeyWinCommand, "Win/Command", false, wl_kKeyLocationLeft },
        { XK_Super_R, XK_VoidSymbol, wl_kKeyWinCommand, "Win/Command", false, wl_kKeyLocationRight },
//        wl_kKeyFn,
        // home/end block
        { XK_Insert, XK_VoidSymbol, wl_kKeyInsert, "Insert", false },
        { XK_Delete, XK_VoidSymbol, wl_kKeyDelete, "Delete", false },
        { XK_Page_Up, XK_VoidSymbol, wl_kKeyPageUp, "PageUp", false },
        { XK_Page_Down, XK_VoidSymbol, wl_kKeyPageDown, "PageDown", false },
        { XK_Home, XK_VoidSymbol, wl_kKeyHome, "Home", false },
        { XK_End, XK_VoidSymbol, wl_kKeyEnd, "End", false },
        // arrow keys
        { XK_Left, XK_VoidSymbol, wl_kKeyLeftArrow, "LeftArrow", false },
        { XK_Up, XK_VoidSymbol, wl_kKeyUpArrow, "UpArrow", false },
        { XK_Right, XK_VoidSymbol, wl_kKeyRightArrow, "RightArrow", false },
        { XK_Down, XK_VoidSymbol, wl_kKeyDownArrow, "DownArrow", false },
        // keypad numbers
        { XK_KP_0, XK_VoidSymbol, wl_kKeyKP0, "KP-0", false, wl_kKeyLocationNumPad },
        { XK_KP_1, XK_VoidSymbol, wl_kKeyKP1, "KP-1", false, wl_kKeyLocationNumPad },
        { XK_KP_2, XK_VoidSymbol, wl_kKeyKP2, "KP-2", false, wl_kKeyLocationNumPad },
        { XK_KP_3, XK_VoidSymbol, wl_kKeyKP3, "KP-3", false, wl_kKeyLocationNumPad },
        { XK_KP_4, XK_VoidSymbol, wl_kKeyKP4, "KP-4", false, wl_kKeyLocationNumPad },
        { XK_KP_5, XK_VoidSymbol, wl_kKeyKP5, "KP-5", false, wl_kKeyLocationNumPad },
        { XK_KP_6, XK_VoidSymbol, wl_kKeyKP6, "KP-6", false, wl_kKeyLocationNumPad },
        { XK_KP_7, XK_VoidSymbol, wl_kKeyKP7, "KP-7", false, wl_kKeyLocationNumPad },
        { XK_KP_8, XK_VoidSymbol, wl_kKeyKP8, "KP-8", false, wl_kKeyLocationNumPad },
        { XK_KP_9, XK_VoidSymbol, wl_kKeyKP9, "KP-9", false, wl_kKeyLocationNumPad },
        // keypad ops
        { XK_Clear, XK_VoidSymbol, wl_kKeyKPClear, "KP Clear", false, wl_kKeyLocationNumPad },
        //
        { XK_KP_Equal, XK_VoidSymbol, wl_kKeyKPEquals, "KP Equals", false, wl_kKeyLocationNumPad },
        { XK_KP_Divide, XK_VoidSymbol, wl_kKeyKPDivide, "KP Divide", false, wl_kKeyLocationNumPad },
        { XK_KP_Multiply, XK_VoidSymbol, wl_kKeyKPMultiply, "KP Multiply", false, wl_kKeyLocationNumPad },
        { XK_KP_Subtract, XK_VoidSymbol, wl_kKeyKPSubtract, "KP Subtract", false, wl_kKeyLocationNumPad },
        { XK_KP_Add, XK_VoidSymbol, wl_kKeyKPAdd, "KP Add", false, wl_kKeyLocationNumPad },
        { XK_KP_Enter, XK_VoidSymbol, wl_kKeyKPEnter, "KP Enter", true, wl_kKeyLocationNumPad },
        { XK_KP_Decimal, XK_VoidSymbol, wl_kKeyKPDecimal, "KP Decimal", false, wl_kKeyLocationNumPad },
        // locks
        { XK_Caps_Lock, XK_VoidSymbol, wl_kKeyCapsLock, "CapsLock", false },
        { XK_Num_Lock, XK_VoidSymbol, wl_kKeyNumLock, "NumLock", false },
        { XK_Scroll_Lock, XK_VoidSymbol, wl_kKeyScrollLock, "ScrollLock", false },
        // misc
        { XK_Print, XK_VoidSymbol, wl_kKeyPrintScreen, "PrintScreen", false },
        { XK_Pause, XK_VoidSymbol, wl_kKeyPause, "Pause", false },
        { XK_Break, XK_VoidSymbol, wl_kKeyCancel, "Break (Ctrl-Pause)", false },
//        { XK_Cancel, XK_VoidSymbol, wl_kKeyCancel, "Cancel", false },
        // media
        { XF86XK_AudioMute, XK_VoidSymbol, wl_kKeyMediaMute, "MediaMute", false },
        { XF86XK_AudioLowerVolume, XK_VoidSymbol, wl_kKeyMediaVolumeDown, "MediaVolumeDown", false },
        { XF86XK_AudioRaiseVolume, XK_VoidSymbol, wl_kKeyMediaVolumeUp, "MediaVolumeUp", false },
        { XF86XK_AudioNext, XK_VoidSymbol, wl_kKeyMediaNext, "MediaNext", false },
        { XF86XK_AudioPrev, XK_VoidSymbol, wl_kKeyMediaPrev, "MediaPrev", false },
        { XF86XK_AudioStop, XK_VoidSymbol, wl_kKeyMediaStop, "MediaStop", false },
        { XF86XK_AudioPlay, XK_VoidSymbol, wl_kKeyMediaPlayPause, "MediaPlayPause", false }
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
