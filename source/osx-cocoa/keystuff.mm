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
    { 0xFFFF, WLKey_Unknown, "(unknown)", false },
    //
    { kVK_Escape, WLKey_Escape, "Escape", true },
    { kVK_Return, WLKey_Return, "Return", true },
    { kVK_Tab, WLKey_Tab, "Tab", true },
    { kVK_CapsLock, WLKey_CapsLock, "CapsLock", false },
    { kVK_Space, WLKey_Space, "Space", false },
    { kVK_Delete, WLKey_Backspace, "Backspace", true },
    //
    { kVK_F1, WLKey_F1, "F1", true },
    { kVK_F2, WLKey_F2, "F2", true },
    { kVK_F3, WLKey_F3, "F3", true },
    { kVK_F4, WLKey_F4, "F4", true },
    { kVK_F5, WLKey_F5, "F5", true },
    { kVK_F6, WLKey_F6, "F6", true },
    { kVK_F7, WLKey_F7, "F7", true },
    { kVK_F8, WLKey_F8, "F8", true },
    { kVK_F9, WLKey_F9, "F9", true },
    { kVK_F10, WLKey_F10, "F10", true },
    { kVK_F11, WLKey_F11, "F11", true },
    { kVK_F12, WLKey_F12, "F12", true },
    { kVK_F13, WLKey_F13, "F13", true },
    { kVK_F14, WLKey_F14, "F14", true },
    { kVK_F15, WLKey_F15, "F15", true },
    { kVK_F16, WLKey_F16, "F16", true },
    { kVK_F17, WLKey_F17, "F17", true },
    { kVK_F18, WLKey_F18, "F18", true },
    { kVK_F19, WLKey_F19, "F19", true },
    // home/end block
    { kVK_Function, WLKey_Fn, "Fn", true },
    { kVK_ForwardDelete, WLKey_Delete, "Delete", true },
    { kVK_Home, WLKey_Home, "Home", true },
    { kVK_End, WLKey_End, "End", true },
    { kVK_PageUp, WLKey_PageUp, "PageUp", true },
    { kVK_PageDown, WLKey_PageDown, "PageDown", true },
    // modifiers
    { kVK_Command, WLKey_WinCommand, "Left Command", true, WLKeyLocation_Left },
    { 54, WLKey_WinCommand, "Right Command", true, WLKeyLocation_Right },
    { kVK_Option, WLKey_AltOption, "Left Option", false, WLKeyLocation_Left },
    { kVK_RightOption, WLKey_AltOption, "Right Option", false, WLKeyLocation_Right },
    { kVK_Shift, WLKey_Shift, "Left Shift", false, WLKeyLocation_Left },
    { kVK_RightShift, WLKey_Shift, "Right Shift", false, WLKeyLocation_Right },
    { kVK_Control, WLKey_Control, "Left Control", false, WLKeyLocation_Left },
    { kVK_RightControl, WLKey_Control, "Right Control", false, WLKeyLocation_Right },
    //
    { kVK_VolumeUp, WLKey_MediaVolumeUp, "VolumeUp", true },
    { kVK_VolumeDown, WLKey_MediaVolumeDown, "VolumeDown", true },
    { kVK_Mute, WLKey_MediaMute, "Mute", true },
    //
    { kVK_ANSI_Keypad0, WLKey_KP_0, "KP 0", false, WLKeyLocation_NumPad },
    { kVK_ANSI_Keypad1, WLKey_KP_1, "KP 1", false, WLKeyLocation_NumPad },
    { kVK_ANSI_Keypad2, WLKey_KP_2, "KP 2", false, WLKeyLocation_NumPad },
    { kVK_ANSI_Keypad3, WLKey_KP_3, "KP 3", false, WLKeyLocation_NumPad },
    { kVK_ANSI_Keypad4, WLKey_KP_4, "KP 4", false, WLKeyLocation_NumPad },
    { kVK_ANSI_Keypad5, WLKey_KP_5, "KP 5", false, WLKeyLocation_NumPad },
    { kVK_ANSI_Keypad6, WLKey_KP_6, "KP 6", false, WLKeyLocation_NumPad },
    { kVK_ANSI_Keypad7, WLKey_KP_7, "KP 7", false, WLKeyLocation_NumPad },
    { kVK_ANSI_Keypad8, WLKey_KP_8, "KP 8", false, WLKeyLocation_NumPad },
    { kVK_ANSI_Keypad9, WLKey_KP_9, "KP 9", false, WLKeyLocation_NumPad },
    //
    { kVK_ANSI_KeypadClear, WLKey_KP_Clear, "KP Clear", true, WLKeyLocation_NumPad },
    { kVK_ANSI_KeypadPlus, WLKey_KP_Add, "KP Add", false, WLKeyLocation_NumPad },
    { kVK_ANSI_KeypadMinus, WLKey_KP_Subtract, "KP Subtract", false, WLKeyLocation_NumPad },
    { kVK_ANSI_KeypadDivide, WLKey_KP_Divide, "KP Divide", false, WLKeyLocation_NumPad },
    { kVK_ANSI_KeypadMultiply, WLKey_KP_Multiply, "KP Multiply", false, WLKeyLocation_NumPad },
    { kVK_ANSI_KeypadEquals, WLKey_KP_Equals, "KP Equals", false, WLKeyLocation_NumPad },
    { kVK_ANSI_KeypadDecimal, WLKey_KP_Decimal, "KP Decimal", false, WLKeyLocation_NumPad },
    { kVK_ANSI_KeypadEnter, WLKey_KP_Enter, "KP Enter", true, WLKeyLocation_NumPad },
    //
    { kVK_LeftArrow, WLKey_LeftArrow, "LeftArrow", true },
    { kVK_RightArrow, WLKey_RightArrow, "RightArrow", true },
    { kVK_DownArrow, WLKey_DownArrow, "DownArrow", true },
    { kVK_UpArrow, WLKey_UpArrow, "UpArrow", true },
};
keymapInfo variableEntries[] = {
    { kVK_ANSI_A, WLKey_A, "A", false },
    { kVK_ANSI_B, WLKey_B, "B", false },
    { kVK_ANSI_C, WLKey_C, "C", false },
    { kVK_ANSI_D, WLKey_D, "D", false },
    { kVK_ANSI_E, WLKey_E, "E", false },
    { kVK_ANSI_F, WLKey_F, "F", false },
    { kVK_ANSI_G, WLKey_G, "G", false },
    { kVK_ANSI_H, WLKey_H, "H", false },
    { kVK_ANSI_I, WLKey_I, "I", false },
    { kVK_ANSI_J, WLKey_J, "J", false },
    { kVK_ANSI_K, WLKey_K, "K", false },
    { kVK_ANSI_L, WLKey_L, "L", false },
    { kVK_ANSI_M, WLKey_M, "M", false },
    { kVK_ANSI_N, WLKey_N, "N", false },
    { kVK_ANSI_O, WLKey_O, "O", false },
    { kVK_ANSI_P, WLKey_P, "P", false },
    { kVK_ANSI_Q, WLKey_Q, "Q", false },
    { kVK_ANSI_R, WLKey_R, "R", false },
    { kVK_ANSI_S, WLKey_S, "S", false },
    { kVK_ANSI_T, WLKey_T, "T", false },
    { kVK_ANSI_U, WLKey_U, "U", false },
    { kVK_ANSI_V, WLKey_V, "V", false },
    { kVK_ANSI_W, WLKey_W, "W", false },
    { kVK_ANSI_X, WLKey_X, "X", false },
    { kVK_ANSI_Y, WLKey_Y, "Y", false },
    { kVK_ANSI_Z, WLKey_Z, "Z", false },
    // number keys
    { kVK_ANSI_0, WLKey_0, "0", false },
    { kVK_ANSI_1, WLKey_1, "1", false },
    { kVK_ANSI_2, WLKey_2, "2", false },
    { kVK_ANSI_3, WLKey_3, "3", false },
    { kVK_ANSI_4, WLKey_4, "4", false },
    { kVK_ANSI_5, WLKey_5, "5", false },
    { kVK_ANSI_6, WLKey_6, "6", false },
    { kVK_ANSI_7, WLKey_7, "7", false },
    { kVK_ANSI_8, WLKey_8, "8", false },
    { kVK_ANSI_9, WLKey_9, "9", false },
};
std::map<int, keymapInfo *> codeToKeyInfo; // fixed non-alphanumeric keys
std::map<unichar, keymapInfo *> unicharToKeyInfo; // for non-fixed keys (foreign layouts etc)
std::map<WLKeyEnum, keymapInfo *> reverseKeymap;

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
