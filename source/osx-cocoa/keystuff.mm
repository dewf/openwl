//
//  keystuff.cpp
//  openwl
//
//  Created by Daniel X on 2/12/18.
//  Copyright (c) 2018 OpenWL Developers. All rights reserved.
//

#include "keystuff.h"

#define CALCULATE_AT_RUNTIME "##calc_at_runtime##"
keymapInfo fixedEntries[] = {
    { 0xFFFF, wl_kKeyUnknown, "(unknown)", false },
    //
    { kVK_Escape, wl_kKeyEscape, "Escape", true },
    { kVK_Return, wl_kKeyReturn, "Return", true },
    { kVK_Tab, wl_kKeyTab, "Tab", true },
    { kVK_CapsLock, wl_kKeyCapsLock, "CapsLock", false },
    { kVK_Space, wl_kKeySpace, "Space", false },
    { kVK_Delete, wl_kKeyBackspace, "Backspace", true },
    //
    { kVK_F1, wl_kKeyF1, "F1", true },
    { kVK_F2, wl_kKeyF2, "F2", true },
    { kVK_F3, wl_kKeyF3, "F3", true },
    { kVK_F4, wl_kKeyF4, "F4", true },
    { kVK_F5, wl_kKeyF5, "F5", true },
    { kVK_F6, wl_kKeyF6, "F6", true },
    { kVK_F7, wl_kKeyF7, "F7", true },
    { kVK_F8, wl_kKeyF8, "F8", true },
    { kVK_F9, wl_kKeyF9, "F9", true },
    { kVK_F10, wl_kKeyF10, "F10", true },
    { kVK_F11, wl_kKeyF11, "F11", true },
    { kVK_F12, wl_kKeyF12, "F12", true },
    { kVK_F13, wl_kKeyF13, "F13", true },
    { kVK_F14, wl_kKeyF14, "F14", true },
    { kVK_F15, wl_kKeyF15, "F15", true },
    { kVK_F16, wl_kKeyF16, "F16", true },
    { kVK_F17, wl_kKeyF17, "F17", true },
    { kVK_F18, wl_kKeyF18, "F18", true },
    { kVK_F19, wl_kKeyF19, "F19", true },
    // home/end block
    { kVK_Function, wl_kKeyFn, "Fn", true },
    { kVK_ForwardDelete, wl_kKeyDelete, "Delete", true },
    { kVK_Home, wl_kKeyHome, "Home", true },
    { kVK_End, wl_kKeyEnd, "End", true },
    { kVK_PageUp, wl_kKeyPageUp, "PageUp", true },
    { kVK_PageDown, wl_kKeyPageDown, "PageDown", true },
    // modifiers
    { kVK_Command, wl_kKeyWinCommand, "Left Command", true, wl_kKeyLocationLeft },
    { 54, wl_kKeyWinCommand, "Right Command", true, wl_kKeyLocationRight },
    { kVK_Option, wl_kKeyAltOption, "Left Option", false, wl_kKeyLocationLeft },
    { kVK_RightOption, wl_kKeyAltOption, "Right Option", false, wl_kKeyLocationRight },
    { kVK_Shift, wl_kKeyShift, "Left Shift", false, wl_kKeyLocationLeft },
    { kVK_RightShift, wl_kKeyShift, "Right Shift", false, wl_kKeyLocationRight },
    { kVK_Control, wl_kKeyControl, "Left Control", false, wl_kKeyLocationLeft },
    { kVK_RightControl, wl_kKeyControl, "Right Control", false, wl_kKeyLocationRight },
    //
    { kVK_VolumeUp, wl_kKeyMediaVolumeUp, "VolumeUp", true },
    { kVK_VolumeDown, wl_kKeyMediaVolumeDown, "VolumeDown", true },
    { kVK_Mute, wl_kKeyMediaMute, "Mute", true },
    //
    { kVK_ANSI_Keypad0, wl_kKeyKP0, "KP 0", false, wl_kKeyLocationNumPad },
    { kVK_ANSI_Keypad1, wl_kKeyKP1, "KP 1", false, wl_kKeyLocationNumPad },
    { kVK_ANSI_Keypad2, wl_kKeyKP2, "KP 2", false, wl_kKeyLocationNumPad },
    { kVK_ANSI_Keypad3, wl_kKeyKP3, "KP 3", false, wl_kKeyLocationNumPad },
    { kVK_ANSI_Keypad4, wl_kKeyKP4, "KP 4", false, wl_kKeyLocationNumPad },
    { kVK_ANSI_Keypad5, wl_kKeyKP5, "KP 5", false, wl_kKeyLocationNumPad },
    { kVK_ANSI_Keypad6, wl_kKeyKP6, "KP 6", false, wl_kKeyLocationNumPad },
    { kVK_ANSI_Keypad7, wl_kKeyKP7, "KP 7", false, wl_kKeyLocationNumPad },
    { kVK_ANSI_Keypad8, wl_kKeyKP8, "KP 8", false, wl_kKeyLocationNumPad },
    { kVK_ANSI_Keypad9, wl_kKeyKP9, "KP 9", false, wl_kKeyLocationNumPad },
    //
    { kVK_ANSI_KeypadClear, wl_kKeyKPClear, "KP Clear", true, wl_kKeyLocationNumPad },
    { kVK_ANSI_KeypadPlus, wl_kKeyKPAdd, "KP Add", false, wl_kKeyLocationNumPad },
    { kVK_ANSI_KeypadMinus, wl_kKeyKPSubtract, "KP Subtract", false, wl_kKeyLocationNumPad },
    { kVK_ANSI_KeypadDivide, wl_kKeyKPDivide, "KP Divide", false, wl_kKeyLocationNumPad },
    { kVK_ANSI_KeypadMultiply, wl_kKeyKPMultiply, "KP Multiply", false, wl_kKeyLocationNumPad },
    { kVK_ANSI_KeypadEquals, wl_kKeyKPEquals, "KP Equals", false, wl_kKeyLocationNumPad },
    { kVK_ANSI_KeypadDecimal, wl_kKeyKPDecimal, "KP Decimal", false, wl_kKeyLocationNumPad },
    { kVK_ANSI_KeypadEnter, wl_kKeyKPEnter, "KP Enter", true, wl_kKeyLocationNumPad },
    //
    { kVK_LeftArrow, wl_kKeyLeftArrow, "LeftArrow", true },
    { kVK_RightArrow, wl_kKeyRightArrow, "RightArrow", true },
    { kVK_DownArrow, wl_kKeyDownArrow, "DownArrow", true },
    { kVK_UpArrow, wl_kKeyUpArrow, "UpArrow", true },
};
keymapInfo variableEntries[] = {
    { kVK_ANSI_A, wl_kKeyA, "A", false },
    { kVK_ANSI_B, wl_kKeyB, "B", false },
    { kVK_ANSI_C, wl_kKeyC, "C", false },
    { kVK_ANSI_D, wl_kKeyD, "D", false },
    { kVK_ANSI_E, wl_kKeyE, "E", false },
    { kVK_ANSI_F, wl_kKeyF, "F", false },
    { kVK_ANSI_G, wl_kKeyG, "G", false },
    { kVK_ANSI_H, wl_kKeyH, "H", false },
    { kVK_ANSI_I, wl_kKeyI, "I", false },
    { kVK_ANSI_J, wl_kKeyJ, "J", false },
    { kVK_ANSI_K, wl_kKeyK, "K", false },
    { kVK_ANSI_L, wl_kKeyL, "L", false },
    { kVK_ANSI_M, wl_kKeyM, "M", false },
    { kVK_ANSI_N, wl_kKeyN, "N", false },
    { kVK_ANSI_O, wl_kKeyO, "O", false },
    { kVK_ANSI_P, wl_kKeyP, "P", false },
    { kVK_ANSI_Q, wl_kKeyQ, "Q", false },
    { kVK_ANSI_R, wl_kKeyR, "R", false },
    { kVK_ANSI_S, wl_kKeyS, "S", false },
    { kVK_ANSI_T, wl_kKeyT, "T", false },
    { kVK_ANSI_U, wl_kKeyU, "U", false },
    { kVK_ANSI_V, wl_kKeyV, "V", false },
    { kVK_ANSI_W, wl_kKeyW, "W", false },
    { kVK_ANSI_X, wl_kKeyX, "X", false },
    { kVK_ANSI_Y, wl_kKeyY, "Y", false },
    { kVK_ANSI_Z, wl_kKeyZ, "Z", false },
    // number keys
    { kVK_ANSI_0, wl_kKey0, "0", false },
    { kVK_ANSI_1, wl_kKey1, "1", false },
    { kVK_ANSI_2, wl_kKey2, "2", false },
    { kVK_ANSI_3, wl_kKey3, "3", false },
    { kVK_ANSI_4, wl_kKey4, "4", false },
    { kVK_ANSI_5, wl_kKey5, "5", false },
    { kVK_ANSI_6, wl_kKey6, "6", false },
    { kVK_ANSI_7, wl_kKey7, "7", false },
    { kVK_ANSI_8, wl_kKey8, "8", false },
    { kVK_ANSI_9, wl_kKey9, "9", false },
};
std::map<int, keymapInfo *> codeToKeyInfo; // fixed non-alphanumeric keys
std::map<unichar, keymapInfo *> unicharToKeyInfo; // for non-fixed keys (foreign layouts etc)
std::map<wl_KeyEnum, keymapInfo *> reverseKeymap;

void initKeyMap() {
    int fixedCount = sizeof(fixedEntries) / sizeof(keymapInfo);
    for (int i=0; i< fixedCount; i++) {
        auto info = &fixedEntries[i];
        //        if (info->stringRep == CALCULATE_AT_RUNTIME) {
        //            // convert code to character in current keymap
        //            info->stringRep = "@";
        //        }
        codeToKeyInfo[info->code] = info;
        reverseKeymap[info->key] = info;
    }
    
    int variableCount = sizeof(variableEntries) / sizeof(keymapInfo);
    for (int i=0; i< variableCount; i++) {
        auto info = &variableEntries[i];
        auto uch = [[NSString stringWithUTF8String:info->stringRep] characterAtIndex:0];
        unicharToKeyInfo[uch] = info;
        reverseKeymap[info->key] = info;
    }
}
