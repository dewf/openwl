//
//  miscutil.m
//  openwl
//
//  Created by Daniel X on 11/29/17.
//  Copyright (c) 2017 OpenWL Developers. All rights reserved.
//

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#import "../openwl.h"

NSImage *resizeImage(NSImage *sourceImage, NSSize newSize) {
    if (! sourceImage.isValid) return nil;
    
    NSBitmapImageRep *rep = [[NSBitmapImageRep alloc]
                             initWithBitmapDataPlanes:NULL
                             pixelsWide:newSize.width
                             pixelsHigh:newSize.height
                             bitsPerSample:8
                             samplesPerPixel:4
                             hasAlpha:YES
                             isPlanar:NO
                             colorSpaceName:NSCalibratedRGBColorSpace
                             bytesPerRow:0
                             bitsPerPixel:0];
    rep.size = newSize;
    
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:rep]];
    [sourceImage drawInRect:NSMakeRect(0, 0, newSize.width, newSize.height) fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
    [NSGraphicsContext restoreGraphicsState];
    
    NSImage *newImage = [[NSImage alloc] initWithSize:newSize];
    [newImage addRepresentation:rep];
    return newImage;
}




//#include "../../openwl.h"

typedef struct {
    int keycode;
    WLKeyEnum ks1;
    WLKeyEnum ks2;
    WLKeyEnum ks3;
    WLKeyEnum ks4;
} KeyInfo;

#define NOTHING WLKey_VoidSymbol

//// defining these up here (instead of replacing them in the table below)
////   to match how it came out of xmodmap -pke ... easier to compare/fix anything that way
//#define _U25CA WLKey_lozenge // LOZENGE
//#define _U2211 WLKey_narysummation // N-ARY SUMMATION
//#define _U2030 WLKey_permille // PER MILLE SIGN
//#define _U2044 WLKey_fractionslash // FRACTION SLASH
//#define _U2039 WLKey_singleleftanglequot // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
//#define _U203A WLKey_singlerightanglequot // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
//#define _UFB02 WLKey_latinsmallligaturefl // LATIN SMALL LIGATURE FL
//#define _UFB01 WLKey_latinsmallligaturefi // LATIN SMALL LIGATURE FI
//#define _U02C6 WLKey_modlettercircumflex // MODIFIER LETTER CIRCUMFLEX ACCENT
//#define _U220F WLKey_naryproduct // N-ARY PRODUCT
//#define _U2206 WLKey_increment // INCREMENT
//#define _U02DA WLKey_ringabove // RING ABOVE
//#define _UF8FF WLKey_privateuse // <Private Use, Last>
//#define _U02DC WLKey_smalltilde // SMALL TILDE
//#define _0x1000003 WLKey_endoftext_maybe // ?? unicode 0x03: end-of-text / Ctrl-C ??

// normal, +shift, +alt, +alt-shift
KeyInfo codeToSymMap[] = {
//    { 8, WLKey_a, WLKey_A, WLKey_aring, WLKey_Aring },
//    { 9, WLKey_s, WLKey_S, WLKey_ssharp, WLKey_Iacute },
//    { 10, WLKey_d, WLKey_D, WLKey_partialderivative, WLKey_Icircumflex },
//    { 11, WLKey_f, WLKey_F, WLKey_function, WLKey_Idiaeresis },
//    { 12, WLKey_h, WLKey_H, WLKey_abovedot, WLKey_Oacute },
//    { 13, WLKey_g, WLKey_G, WLKey_copyright, WLKey_doubleacute },
//    { 14, WLKey_z, WLKey_Z, WLKey_Greek_OMEGA, WLKey_cedilla },
//    { 15, WLKey_x, WLKey_X, WLKey_approxeq, WLKey_ogonek },
//    { 16, WLKey_c, WLKey_C, WLKey_ccedilla, WLKey_Ccedilla },
//    { 17, WLKey_v, WLKey_V, WLKey_radical, _U25CA },
//    { 18, WLKey_section, WLKey_plusminus, WLKey_section, WLKey_plusminus },
//    { 19, WLKey_b, WLKey_B, WLKey_integral, WLKey_idotless },
//    { 20, WLKey_q, WLKey_Q, WLKey_oe, WLKey_OE },
//    { 21, WLKey_w, WLKey_W, _U2211, WLKey_doublelowquotemark },
//    { 22, WLKey_e, WLKey_E, WLKey_dead_acute, WLKey_acute },
//    { 23, WLKey_r, WLKey_R, WLKey_registered, _U2030 },
//    { 24, WLKey_y, WLKey_Y, WLKey_yen, WLKey_Aacute },
//    { 25, WLKey_t, WLKey_T, WLKey_dagger, WLKey_caron },
//    { 26, WLKey_1, WLKey_exclam, WLKey_exclamdown, _U2044 },
//    { 27, WLKey_2, WLKey_at, WLKey_trademark, WLKey_EuroSign },
//    { 28, WLKey_3, WLKey_numbersign, WLKey_sterling, _U2039 },
//    { 29, WLKey_4, WLKey_dollar, WLKey_cent, _U203A },
//    { 30, WLKey_6, WLKey_asciicircum, WLKey_section, _UFB02 },
//    { 31, WLKey_5, WLKey_percent, WLKey_infinity, _UFB01 },
//    { 32, WLKey_equal, WLKey_plus, WLKey_notequal, WLKey_plusminus },
//    { 33, WLKey_9, WLKey_parenleft, WLKey_ordfeminine, WLKey_periodcentered },
//    { 34, WLKey_7, WLKey_ampersand, WLKey_paragraph, WLKey_doubledagger },
//    { 35, WLKey_minus, WLKey_underscore, WLKey_endash, WLKey_emdash },
//    { 36, WLKey_8, WLKey_asterisk, WLKey_enfilledcircbullet, WLKey_degree },
//    { 37, WLKey_0, WLKey_parenright, WLKey_masculine, WLKey_singlelowquotemark },
//    { 38, WLKey_bracketright, WLKey_braceright, WLKey_leftsinglequotemark, WLKey_rightsinglequotemark },
//    { 39, WLKey_o, WLKey_O, WLKey_oslash, WLKey_Oslash },
//    { 40, WLKey_u, WLKey_U, WLKey_dead_diaeresis, WLKey_diaeresis },
//    { 41, WLKey_bracketleft, WLKey_braceleft, WLKey_leftdoublequotemark, WLKey_rightdoublequotemark },
//    { 42, WLKey_i, WLKey_I, WLKey_dead_circumflex, _U02C6 },
//    { 43, WLKey_p, WLKey_P, WLKey_Greek_pi, _U220F },
//    { 44, WLKey_Return, NOTHING, WLKey_Return, NOTHING },
//    { 45, WLKey_l, WLKey_L, WLKey_notsign, WLKey_Ograve },
//    { 46, WLKey_j, WLKey_J, _U2206, WLKey_Ocircumflex },
//    { 47, WLKey_apostrophe, WLKey_quotedbl, WLKey_ae, WLKey_AE },
//    { 48, WLKey_k, WLKey_K, _U02DA, _UF8FF },
//    { 49, WLKey_semicolon, WLKey_colon, WLKey_ellipsis, WLKey_Uacute },
//    { 50, WLKey_backslash, WLKey_bar, WLKey_guillemotleft, WLKey_guillemotright },
//    { 51, WLKey_comma, WLKey_less, WLKey_lessthanequal, WLKey_macron },
//    { 52, WLKey_slash, WLKey_question, WLKey_division, WLKey_questiondown },
//    { 53, WLKey_n, WLKey_N, WLKey_dead_tilde, _U02DC },
//    { 54, WLKey_m, WLKey_M, WLKey_mu, WLKey_Acircumflex },
//    { 55, WLKey_period, WLKey_greater, WLKey_greaterthanequal, WLKey_breve },
//    { 56, WLKey_Tab, NOTHING, WLKey_Tab, NOTHING },
//    { 57, WLKey_space, NOTHING, WLKey_nobreakspace, NOTHING },
//    { 58, WLKey_grave, WLKey_asciitilde, WLKey_dead_grave, WLKey_grave },
//    { 59, WLKey_BackSpace, NOTHING, WLKey_BackSpace, NOTHING },
//    { 60, _0x1000003, NOTHING, _0x1000003, NOTHING },
//    { 61, WLKey_Escape, NOTHING, WLKey_Escape, NOTHING },
//    { 62, NOTHING, NOTHING, NOTHING },
//    { 63, WLKey_Meta_L, NOTHING, WLKey_Meta_L, NOTHING },
//    { 64, WLKey_Shift_L, NOTHING, WLKey_Shift_L, NOTHING },
//    { 65, WLKey_Caps_Lock, NOTHING, WLKey_Caps_Lock, NOTHING },
//    { 66, WLKey_Mode_switch, NOTHING, WLKey_Mode_switch, NOTHING },
//    { 67, WLKey_Control_L, NOTHING, WLKey_Control_L, NOTHING },
//    { 68, WLKey_Shift_R, NOTHING, WLKey_Shift_R, NOTHING },
//    { 69, WLKey_Mode_switch, NOTHING, WLKey_Mode_switch, NOTHING },
//    { 70, WLKey_Control_R, NOTHING, WLKey_Control_R, NOTHING },
//    { 71, WLKey_Meta_R, NOTHING, WLKey_Meta_R, NOTHING },
//    { 72, NOTHING, NOTHING, NOTHING },
//    { 73, WLKey_KP_Decimal, NOTHING, WLKey_KP_Decimal, NOTHING },
//    { 74, NOTHING, NOTHING, NOTHING },
//    { 75, WLKey_KP_Multiply, NOTHING, WLKey_KP_Multiply, NOTHING },
//    { 76, NOTHING, NOTHING, NOTHING },
//    { 77, WLKey_KP_Add, NOTHING, WLKey_KP_Add, NOTHING },
//    { 78, NOTHING, NOTHING, NOTHING },
//    { 79, WLKey_Escape, NOTHING, WLKey_Escape, NOTHING },
//    { 80, NOTHING, NOTHING, NOTHING },
//    { 81, NOTHING, NOTHING, NOTHING },
//    { 82, NOTHING, NOTHING, NOTHING },
//    { 83, WLKey_KP_Divide, NOTHING, WLKey_KP_Divide, NOTHING },
//    { 84, WLKey_KP_Enter, NOTHING, WLKey_KP_Enter, NOTHING },
//    { 85, NOTHING, NOTHING, NOTHING },
//    { 86, WLKey_KP_Subtract, NOTHING, WLKey_KP_Subtract, NOTHING },
//    { 87, NOTHING, NOTHING, NOTHING },
//    { 88, NOTHING, NOTHING, NOTHING },
//    { 89, WLKey_KP_Equal, NOTHING, WLKey_KP_Equal, NOTHING },
//    { 90, WLKey_KP_0, NOTHING, WLKey_KP_0, NOTHING },
//    { 91, WLKey_KP_1, NOTHING, WLKey_KP_1, NOTHING },
//    { 92, WLKey_KP_2, NOTHING, WLKey_KP_2, NOTHING },
//    { 93, WLKey_KP_3, NOTHING, WLKey_KP_3, NOTHING },
//    { 94, WLKey_KP_4, NOTHING, WLKey_KP_4, NOTHING },
//    { 95, WLKey_KP_5, NOTHING, WLKey_KP_5, NOTHING },
//    { 96, WLKey_KP_6, NOTHING, WLKey_KP_6, NOTHING },
//    { 97, WLKey_KP_7, NOTHING, WLKey_KP_7, NOTHING },
//    { 98, NOTHING, NOTHING, NOTHING },
//    { 99, WLKey_KP_8, NOTHING, WLKey_KP_8, NOTHING },
//    { 100, WLKey_KP_9, NOTHING, WLKey_KP_9, NOTHING },
//    { 101, NOTHING, NOTHING, NOTHING },
//    { 102, NOTHING, NOTHING, NOTHING },
//    { 103, NOTHING, NOTHING, NOTHING },
//    { 104, WLKey_F5, NOTHING, WLKey_F5, NOTHING },
//    { 105, WLKey_F6, NOTHING, WLKey_F6, NOTHING },
//    { 106, WLKey_F7, NOTHING, WLKey_F7, NOTHING },
//    { 107, WLKey_F3, NOTHING, WLKey_F3, NOTHING },
//    { 108, WLKey_F8, NOTHING, WLKey_F8, NOTHING },
//    { 109, WLKey_F9, NOTHING, WLKey_F9, NOTHING },
//    { 110, NOTHING, NOTHING, NOTHING },
//    { 111, WLKey_F11, NOTHING, WLKey_F11, NOTHING },
//    { 112, NOTHING, NOTHING, NOTHING },
//    { 113, WLKey_F13, NOTHING, WLKey_F13, NOTHING },
//    { 114, NOTHING, NOTHING, NOTHING },
//    { 115, WLKey_F14, NOTHING, WLKey_F14, NOTHING },
//    { 116, NOTHING, NOTHING, NOTHING },
//    { 117, WLKey_F10, NOTHING, WLKey_F10, NOTHING },
//    { 118, NOTHING, NOTHING, NOTHING },
//    { 119, WLKey_F12, NOTHING, WLKey_F12, NOTHING },
//    { 120, NOTHING, NOTHING, NOTHING },
//    { 121, WLKey_F15, NOTHING, WLKey_F15, NOTHING },
//    { 122, WLKey_Help, NOTHING, WLKey_Help, NOTHING },
//    { 123, WLKey_Home, NOTHING, WLKey_Home, NOTHING },
//    { 124, WLKey_Prior, NOTHING, WLKey_Prior, NOTHING },
//    { 125, WLKey_Delete, NOTHING, WLKey_Delete, NOTHING },
//    { 126, WLKey_F4, NOTHING, WLKey_F4, NOTHING },
//    { 127, WLKey_End, NOTHING, WLKey_End, NOTHING },
//    { 128, WLKey_F2, NOTHING, WLKey_F2, NOTHING },
//    { 129, WLKey_Next, NOTHING, WLKey_Next, NOTHING },
//    { 130, WLKey_F1, NOTHING, WLKey_F1, NOTHING },
//    { 131, WLKey_Left, NOTHING, WLKey_Left, NOTHING },
//    { 132, WLKey_Right, NOTHING, WLKey_Right, NOTHING },
//    { 133, WLKey_Down, NOTHING, WLKey_Down, NOTHING },
//    { 134, WLKey_Up, NOTHING, WLKey_Up, NOTHING },
};

WLKeyEnum keyCodeToKeySym(unsigned short keyCode, NSUInteger cocoa_modifiers)
{
    KeyInfo which = codeToSymMap[keyCode];
    assert((which.keycode-8) == keyCode);
    auto masked = cocoa_modifiers & (NSAlternateKeyMask | NSShiftKeyMask);
    if (masked == 0) {
        return which.ks1;
    } else if (masked == NSShiftKeyMask) {
        return which.ks2;
    } else if (masked == NSAlternateKeyMask) {
        return which.ks3;
    } else if (masked == (NSAlternateKeyMask | NSShiftKeyMask)) {
        return which.ks4;
    }
    //return WLKey_VoidSymbol;

    return WLKey_Unknown;
}


// misc utility functions
unsigned int cocoa_to_wl_modifiers_multi(NSUInteger flags) {
    unsigned int ret = 0;
    ret |= flags & NSCommandKeyMask ? WLModifier_Control : 0;
    ret |= flags & NSAlternateKeyMask ? WLModifier_Alt : 0;
    ret |= flags & NSShiftKeyMask ? WLModifier_Shift : 0;
    ret |= flags & NSControlKeyMask ? WLModifier_MacControl : 0;
    return ret;
}

unsigned int cocoa_to_wl_dropEffect_multi(NSUInteger mask) {
    unsigned int ret = 0;
    ret |= mask & NSDragOperationCopy ? WLDropEffect_Copy : 0;
    ret |= mask & NSDragOperationLink ? WLDropEffect_Link : 0;
    ret |= mask & NSDragOperationMove ? WLDropEffect_Move : 0;
    ret |= mask & NSDragOperationDelete ? WLDropEffect_Move : 0;
    ret |= mask & NSDragOperationGeneric ? WLDropEffect_Copy : 0;
    ret |= mask & NSDragOperationPrivate ? WLDropEffect_Other : 0;
    return ret;
}

WLDropEffect cocoa_to_wl_dropEffect_single(NSDragOperation operation) {
    return (operation & NSDragOperationCopy ? WLDropEffect_Copy :
            (operation & NSDragOperationLink ? WLDropEffect_Link :
             (operation & NSDragOperationMove ? WLDropEffect_Move :
              (operation & NSDragOperationDelete ? WLDropEffect_Move :
               (operation & NSDragOperationGeneric ? WLDropEffect_Copy :
                (operation & NSDragOperationPrivate ? WLDropEffect_Other : WLDropEffect_None))))));
}

NSDragOperation wl_to_cocoa_dropEffect_single(unsigned int dropEffects) {
    // we allow a mask as input, but only accept return a single one
    return (dropEffects & WLDropEffect_Copy ? NSDragOperationCopy :
            (dropEffects & WLDropEffect_Move ? NSDragOperationMove :
             (dropEffects & WLDropEffect_Link ? NSDragOperationLink :
              (dropEffects & WLDropEffect_Other ? NSDragOperationGeneric : NSDragOperationNone))));
}

NSDragOperation wl_to_cocoa_dropEffect_multi(unsigned int dropEffects) {
    // we allow a mask as input, but only accept return a single one
    return (dropEffects & WLDropEffect_Copy ? NSDragOperationCopy : 0) |
    (dropEffects & WLDropEffect_Move ? NSDragOperationMove : 0) |
    (dropEffects & WLDropEffect_Link ? NSDragOperationLink : 0) |
    (dropEffects & WLDropEffect_Other ? NSDragOperationGeneric : 0);
}

CFStringRef wl_to_cocoa_dragFormat(const char *dragFormatMIME) {
    if (!strcmp(dragFormatMIME, kWLDragFormatUTF8)) {
        return (CFStringRef)NSPasteboardTypeString;
    } else if (!strcmp(dragFormatMIME, kWLDragFormatFiles)) {
        return (CFStringRef)NSFilenamesPboardType;
    } else {
        auto cfstr = CFStringCreateWithCString(kCFAllocatorDefault, dragFormatMIME, kCFStringEncodingUTF8);
        auto result = UTTypeCreatePreferredIdentifierForTag(kUTTagClassMIMEType, cfstr, NULL);
        CFRelease(cfstr);
        return result;
    }
}

char dragFormatBuffer[4096];

const char * cocoa_to_wl_dragFormat(CFStringRef pboardFormat) {
    if (UTTypeConformsTo(pboardFormat, (CFStringRef)NSPasteboardTypeString)) {
        return kWLDragFormatUTF8;
    } else if (UTTypeConformsTo(pboardFormat, (CFStringRef)NSFilenamesPboardType)) {
        return kWLDragFormatFiles;
    } else {
        auto cfstr = UTTypeCopyPreferredTagWithClass(pboardFormat, kUTTagClassMIMEType);
        if (CFStringGetCString(cfstr, dragFormatBuffer, 4096, kCFStringEncodingUTF8)) {
            return dragFormatBuffer;
        }
    }
    return nullptr;
}

