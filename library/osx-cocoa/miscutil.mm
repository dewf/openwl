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
    wl_KeyEnum ks1;
    wl_KeyEnum ks2;
    wl_KeyEnum ks3;
    wl_KeyEnum ks4;
} KeyInfo;

#define NOTHING wl_kKeyVoidSymbol

//// defining these up here (instead of replacing them in the table below)
////   to match how it came out of xmodmap -pke ... easier to compare/fix anything that way
//#define _U25CA wl_kKeylozenge // LOZENGE
//#define _U2211 wl_kKeynarysummation // N-ARY SUMMATION
//#define _U2030 wl_kKeypermille // PER MILLE SIGN
//#define _U2044 wl_kKeyfractionslash // FRACTION SLASH
//#define _U2039 wl_kKeysingleleftanglequot // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
//#define _U203A wl_kKeysinglerightanglequot // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
//#define _UFB02 wl_kKeylatinsmallligaturefl // LATIN SMALL LIGATURE FL
//#define _UFB01 wl_kKeylatinsmallligaturefi // LATIN SMALL LIGATURE FI
//#define _U02C6 wl_kKeymodlettercircumflex // MODIFIER LETTER CIRCUMFLEX ACCENT
//#define _U220F wl_kKeynaryproduct // N-ARY PRODUCT
//#define _U2206 wl_kKeyincrement // INCREMENT
//#define _U02DA wl_kKeyringabove // RING ABOVE
//#define _UF8FF wl_kKeyprivateuse // <Private Use, Last>
//#define _U02DC wl_kKeysmalltilde // SMALL TILDE
//#define _0x1000003 wl_kKeyendoftext_maybe // ?? unicode 0x03: end-of-text / Ctrl-C ??

// normal, +shift, +alt, +alt-shift
KeyInfo codeToSymMap[] = {
//    { 8, wl_kKeya, wl_kKeyA, wl_kKeyaring, wl_kKeyAring },
//    { 9, wl_kKeys, wl_kKeyS, wl_kKeyssharp, wl_kKeyIacute },
//    { 10, wl_kKeyd, wl_kKeyD, wl_kKeypartialderivative, wl_kKeyIcircumflex },
//    { 11, wl_kKeyf, wl_kKeyF, wl_kKeyfunction, wl_kKeyIdiaeresis },
//    { 12, wl_kKeyh, wl_kKeyH, wl_kKeyabovedot, wl_kKeyOacute },
//    { 13, wl_kKeyg, wl_kKeyG, wl_kKeycopyright, wl_kKeydoubleacute },
//    { 14, wl_kKeyz, wl_kKeyZ, wl_kKeyGreek_OMEGA, wl_kKeycedilla },
//    { 15, wl_kKeyx, wl_kKeyX, wl_kKeyapproxeq, wl_kKeyogonek },
//    { 16, wl_kKeyc, wl_kKeyC, wl_kKeyccedilla, wl_kKeyCcedilla },
//    { 17, wl_kKeyv, wl_kKeyV, wl_kKeyradical, _U25CA },
//    { 18, wl_kKeysection, wl_kKeyplusminus, wl_kKeysection, wl_kKeyplusminus },
//    { 19, wl_kKeyb, wl_kKeyB, wl_kKeyintegral, wl_kKeyidotless },
//    { 20, wl_kKeyq, wl_kKeyQ, wl_kKeyoe, wl_kKeyOE },
//    { 21, wl_kKeyw, wl_kKeyW, _U2211, wl_kKeydoublelowquotemark },
//    { 22, wl_kKeye, wl_kKeyE, wl_kKeydead_acute, wl_kKeyacute },
//    { 23, wl_kKeyr, wl_kKeyR, wl_kKeyregistered, _U2030 },
//    { 24, wl_kKeyy, wl_kKeyY, wl_kKeyyen, wl_kKeyAacute },
//    { 25, wl_kKeyt, wl_kKeyT, wl_kKeydagger, wl_kKeycaron },
//    { 26, wl_kKey1, wl_kKeyexclam, wl_kKeyexclamdown, _U2044 },
//    { 27, wl_kKey2, wl_kKeyat, wl_kKeytrademark, wl_kKeyEuroSign },
//    { 28, wl_kKey3, wl_kKeynumbersign, wl_kKeysterling, _U2039 },
//    { 29, wl_kKey4, wl_kKeydollar, wl_kKeycent, _U203A },
//    { 30, wl_kKey6, wl_kKeyasciicircum, wl_kKeysection, _UFB02 },
//    { 31, wl_kKey5, wl_kKeypercent, wl_kKeyinfinity, _UFB01 },
//    { 32, wl_kKeyequal, wl_kKeyplus, wl_kKeynotequal, wl_kKeyplusminus },
//    { 33, wl_kKey9, wl_kKeyparenleft, wl_kKeyordfeminine, wl_kKeyperiodcentered },
//    { 34, wl_kKey7, wl_kKeyampersand, wl_kKeyparagraph, wl_kKeydoubledagger },
//    { 35, wl_kKeyminus, wl_kKeyunderscore, wl_kKeyendash, wl_kKeyemdash },
//    { 36, wl_kKey8, wl_kKeyasterisk, wl_kKeyenfilledcircbullet, wl_kKeydegree },
//    { 37, wl_kKey0, wl_kKeyparenright, wl_kKeymasculine, wl_kKeysinglelowquotemark },
//    { 38, wl_kKeybracketright, wl_kKeybraceright, wl_kKeyleftsinglequotemark, wl_kKeyrightsinglequotemark },
//    { 39, wl_kKeyo, wl_kKeyO, wl_kKeyoslash, wl_kKeyOslash },
//    { 40, wl_kKeyu, wl_kKeyU, wl_kKeydead_diaeresis, wl_kKeydiaeresis },
//    { 41, wl_kKeybracketleft, wl_kKeybraceleft, wl_kKeyleftdoublequotemark, wl_kKeyrightdoublequotemark },
//    { 42, wl_kKeyi, wl_kKeyI, wl_kKeydead_circumflex, _U02C6 },
//    { 43, wl_kKeyp, wl_kKeyP, wl_kKeyGreek_pi, _U220F },
//    { 44, wl_kKeyReturn, NOTHING, wl_kKeyReturn, NOTHING },
//    { 45, wl_kKeyl, wl_kKeyL, wl_kKeynotsign, wl_kKeyOgrave },
//    { 46, wl_kKeyj, wl_kKeyJ, _U2206, wl_kKeyOcircumflex },
//    { 47, wl_kKeyapostrophe, wl_kKeyquotedbl, wl_kKeyae, wl_kKeyAE },
//    { 48, wl_kKeyk, wl_kKeyK, _U02DA, _UF8FF },
//    { 49, wl_kKeysemicolon, wl_kKeycolon, wl_kKeyellipsis, wl_kKeyUacute },
//    { 50, wl_kKeybackslash, wl_kKeybar, wl_kKeyguillemotleft, wl_kKeyguillemotright },
//    { 51, wl_kKeycomma, wl_kKeyless, wl_kKeylessthanequal, wl_kKeymacron },
//    { 52, wl_kKeyslash, wl_kKeyquestion, wl_kKeydivision, wl_kKeyquestiondown },
//    { 53, wl_kKeyn, wl_kKeyN, wl_kKeydead_tilde, _U02DC },
//    { 54, wl_kKeym, wl_kKeyM, wl_kKeymu, wl_kKeyAcircumflex },
//    { 55, wl_kKeyperiod, wl_kKeygreater, wl_kKeygreaterthanequal, wl_kKeybreve },
//    { 56, wl_kKeyTab, NOTHING, wl_kKeyTab, NOTHING },
//    { 57, wl_kKeyspace, NOTHING, wl_kKeynobreakspace, NOTHING },
//    { 58, wl_kKeygrave, wl_kKeyasciitilde, wl_kKeydead_grave, wl_kKeygrave },
//    { 59, wl_kKeyBackSpace, NOTHING, wl_kKeyBackSpace, NOTHING },
//    { 60, _0x1000003, NOTHING, _0x1000003, NOTHING },
//    { 61, wl_kKeyEscape, NOTHING, wl_kKeyEscape, NOTHING },
//    { 62, NOTHING, NOTHING, NOTHING },
//    { 63, wl_kKeyMeta_L, NOTHING, wl_kKeyMeta_L, NOTHING },
//    { 64, wl_kKeyShift_L, NOTHING, wl_kKeyShift_L, NOTHING },
//    { 65, wl_kKeyCaps_Lock, NOTHING, wl_kKeyCaps_Lock, NOTHING },
//    { 66, wl_kKeyMode_switch, NOTHING, wl_kKeyMode_switch, NOTHING },
//    { 67, wl_kKeyControl_L, NOTHING, wl_kKeyControl_L, NOTHING },
//    { 68, wl_kKeyShift_R, NOTHING, wl_kKeyShift_R, NOTHING },
//    { 69, wl_kKeyMode_switch, NOTHING, wl_kKeyMode_switch, NOTHING },
//    { 70, wl_kKeyControl_R, NOTHING, wl_kKeyControl_R, NOTHING },
//    { 71, wl_kKeyMeta_R, NOTHING, wl_kKeyMeta_R, NOTHING },
//    { 72, NOTHING, NOTHING, NOTHING },
//    { 73, wl_kKeyKPDecimal, NOTHING, wl_kKeyKPDecimal, NOTHING },
//    { 74, NOTHING, NOTHING, NOTHING },
//    { 75, wl_kKeyKPMultiply, NOTHING, wl_kKeyKPMultiply, NOTHING },
//    { 76, NOTHING, NOTHING, NOTHING },
//    { 77, wl_kKeyKPAdd, NOTHING, wl_kKeyKPAdd, NOTHING },
//    { 78, NOTHING, NOTHING, NOTHING },
//    { 79, wl_kKeyEscape, NOTHING, wl_kKeyEscape, NOTHING },
//    { 80, NOTHING, NOTHING, NOTHING },
//    { 81, NOTHING, NOTHING, NOTHING },
//    { 82, NOTHING, NOTHING, NOTHING },
//    { 83, wl_kKeyKPDivide, NOTHING, wl_kKeyKPDivide, NOTHING },
//    { 84, wl_kKeyKPEnter, NOTHING, wl_kKeyKPEnter, NOTHING },
//    { 85, NOTHING, NOTHING, NOTHING },
//    { 86, wl_kKeyKPSubtract, NOTHING, wl_kKeyKPSubtract, NOTHING },
//    { 87, NOTHING, NOTHING, NOTHING },
//    { 88, NOTHING, NOTHING, NOTHING },
//    { 89, wl_kKeyKP_Equal, NOTHING, wl_kKeyKP_Equal, NOTHING },
//    { 90, wl_kKeyKP0, NOTHING, wl_kKeyKP0, NOTHING },
//    { 91, wl_kKeyKP1, NOTHING, wl_kKeyKP1, NOTHING },
//    { 92, wl_kKeyKP2, NOTHING, wl_kKeyKP2, NOTHING },
//    { 93, wl_kKeyKP3, NOTHING, wl_kKeyKP3, NOTHING },
//    { 94, wl_kKeyKP4, NOTHING, wl_kKeyKP4, NOTHING },
//    { 95, wl_kKeyKP5, NOTHING, wl_kKeyKP5, NOTHING },
//    { 96, wl_kKeyKP6, NOTHING, wl_kKeyKP6, NOTHING },
//    { 97, wl_kKeyKP7, NOTHING, wl_kKeyKP7, NOTHING },
//    { 98, NOTHING, NOTHING, NOTHING },
//    { 99, wl_kKeyKP8, NOTHING, wl_kKeyKP8, NOTHING },
//    { 100, wl_kKeyKP9, NOTHING, wl_kKeyKP9, NOTHING },
//    { 101, NOTHING, NOTHING, NOTHING },
//    { 102, NOTHING, NOTHING, NOTHING },
//    { 103, NOTHING, NOTHING, NOTHING },
//    { 104, wl_kKeyF5, NOTHING, wl_kKeyF5, NOTHING },
//    { 105, wl_kKeyF6, NOTHING, wl_kKeyF6, NOTHING },
//    { 106, wl_kKeyF7, NOTHING, wl_kKeyF7, NOTHING },
//    { 107, wl_kKeyF3, NOTHING, wl_kKeyF3, NOTHING },
//    { 108, wl_kKeyF8, NOTHING, wl_kKeyF8, NOTHING },
//    { 109, wl_kKeyF9, NOTHING, wl_kKeyF9, NOTHING },
//    { 110, NOTHING, NOTHING, NOTHING },
//    { 111, wl_kKeyF11, NOTHING, wl_kKeyF11, NOTHING },
//    { 112, NOTHING, NOTHING, NOTHING },
//    { 113, wl_kKeyF13, NOTHING, wl_kKeyF13, NOTHING },
//    { 114, NOTHING, NOTHING, NOTHING },
//    { 115, wl_kKeyF14, NOTHING, wl_kKeyF14, NOTHING },
//    { 116, NOTHING, NOTHING, NOTHING },
//    { 117, wl_kKeyF10, NOTHING, wl_kKeyF10, NOTHING },
//    { 118, NOTHING, NOTHING, NOTHING },
//    { 119, wl_kKeyF12, NOTHING, wl_kKeyF12, NOTHING },
//    { 120, NOTHING, NOTHING, NOTHING },
//    { 121, wl_kKeyF15, NOTHING, wl_kKeyF15, NOTHING },
//    { 122, wl_kKeyHelp, NOTHING, wl_kKeyHelp, NOTHING },
//    { 123, wl_kKeyHome, NOTHING, wl_kKeyHome, NOTHING },
//    { 124, wl_kKeyPrior, NOTHING, wl_kKeyPrior, NOTHING },
//    { 125, wl_kKeyDelete, NOTHING, wl_kKeyDelete, NOTHING },
//    { 126, wl_kKeyF4, NOTHING, wl_kKeyF4, NOTHING },
//    { 127, wl_kKeyEnd, NOTHING, wl_kKeyEnd, NOTHING },
//    { 128, wl_kKeyF2, NOTHING, wl_kKeyF2, NOTHING },
//    { 129, wl_kKeyNext, NOTHING, wl_kKeyNext, NOTHING },
//    { 130, wl_kKeyF1, NOTHING, wl_kKeyF1, NOTHING },
//    { 131, wl_kKeyLeft, NOTHING, wl_kKeyLeft, NOTHING },
//    { 132, wl_kKeyRight, NOTHING, wl_kKeyRight, NOTHING },
//    { 133, wl_kKeyDown, NOTHING, wl_kKeyDown, NOTHING },
//    { 134, wl_kKeyUp, NOTHING, wl_kKeyUp, NOTHING },
};

wl_KeyEnum keyCodeToKeySym(unsigned short keyCode, NSUInteger cocoa_modifiers)
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
    //return wl_kKeyVoidSymbol;

    return wl_kKeyUnknown;
}


// misc utility functions
unsigned int cocoa_to_wl_modifiers_multi(NSUInteger flags) {
    unsigned int ret = 0;
    ret |= flags & NSCommandKeyMask ? wl_kModifierControl : 0;
    ret |= flags & NSAlternateKeyMask ? wl_kModifierAlt : 0;
    ret |= flags & NSShiftKeyMask ? wl_kModifierShift : 0;
    ret |= flags & NSControlKeyMask ? wl_kModifierMacControl : 0;
    return ret;
}

unsigned int cocoa_to_wl_dropEffect_multi(NSUInteger mask) {
    unsigned int ret = 0;
    ret |= mask & NSDragOperationCopy ? wl_kDropEffectCopy : 0;
    ret |= mask & NSDragOperationLink ? wl_kDropEffectLink : 0;
    ret |= mask & NSDragOperationMove ? wl_kDropEffectMove : 0;
    ret |= mask & NSDragOperationDelete ? wl_kDropEffectMove : 0;
    ret |= mask & NSDragOperationGeneric ? wl_kDropEffectCopy : 0;
    ret |= mask & NSDragOperationPrivate ? wl_kDropEffectOther : 0;
    return ret;
}

wl_DropEffect cocoa_to_wl_dropEffect_single(NSDragOperation operation) {
    return (operation & NSDragOperationCopy ? wl_kDropEffectCopy :
            (operation & NSDragOperationLink ? wl_kDropEffectLink :
             (operation & NSDragOperationMove ? wl_kDropEffectMove :
              (operation & NSDragOperationDelete ? wl_kDropEffectMove :
               (operation & NSDragOperationGeneric ? wl_kDropEffectCopy :
                (operation & NSDragOperationPrivate ? wl_kDropEffectOther : wl_kDropEffectNone))))));
}

NSDragOperation wl_to_cocoa_dropEffect_single(unsigned int dropEffects) {
    // we allow a mask as input, but only accept return a single one
    return (dropEffects & wl_kDropEffectCopy ? NSDragOperationCopy :
            (dropEffects & wl_kDropEffectMove ? NSDragOperationMove :
             (dropEffects & wl_kDropEffectLink ? NSDragOperationLink :
              (dropEffects & wl_kDropEffectOther ? NSDragOperationGeneric : NSDragOperationNone))));
}

NSDragOperation wl_to_cocoa_dropEffect_multi(unsigned int dropEffects) {
    // we allow a mask as input, but only accept return a single one
    return (dropEffects & wl_kDropEffectCopy ? NSDragOperationCopy : 0) |
    (dropEffects & wl_kDropEffectMove ? NSDragOperationMove : 0) |
    (dropEffects & wl_kDropEffectLink ? NSDragOperationLink : 0) |
    (dropEffects & wl_kDropEffectOther ? NSDragOperationGeneric : 0);
}

CFStringRef wl_to_cocoa_dragFormat(const char *dragFormatMIME) {
    if (!strcmp(dragFormatMIME, wl_kDragFormatUTF8)) {
        return (CFStringRef)NSPasteboardTypeString;
    } else if (!strcmp(dragFormatMIME, wl_kDragFormatFiles)) {
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
        return wl_kDragFormatUTF8;
    } else if (UTTypeConformsTo(pboardFormat, (CFStringRef)NSFilenamesPboardType)) {
        return wl_kDragFormatFiles;
    } else {
        auto cfstr = UTTypeCopyPreferredTagWithClass(pboardFormat, kUTTagClassMIMEType);
        if (CFStringGetCString(cfstr, dragFormatBuffer, 4096, kCFStringEncodingUTF8)) {
            return dragFormatBuffer;
        }
    }
    return nullptr;
}

