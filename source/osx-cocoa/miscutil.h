//
//  miscutil.h
//  openwl
//
//  Created by Daniel X on 11/29/17.
//  Copyright (c) 2017 OpenWL Developers. All rights reserved.
//

#ifndef openwl_miscutil_h
#define openwl_miscutil_h

#include "../openwl.h"
#import <Foundation/Foundation.h>

enum WLKeyEnum;

NSImage *resizeImage(NSImage *sourceImage, NSSize newSize);
WLKeyEnum keyCodeToKeySym(unsigned short keyCode, NSUInteger cocoa_modifiers);

// misc utility functions
unsigned int cocoa_to_wl_modifiers_multi(NSUInteger flags);
unsigned int cocoa_to_wl_dropEffect_multi(NSUInteger mask);
WLDropEffect cocoa_to_wl_dropEffect_single(NSDragOperation operation);
NSDragOperation wl_to_cocoa_dropEffect_single(unsigned int dropEffects);
NSDragOperation wl_to_cocoa_dropEffect_multi(unsigned int dropEffects);
CFStringRef wl_to_cocoa_dragFormat(const char *dragFormatMIME);
const char * cocoa_to_wl_dragFormat(CFStringRef pboardFormat);

#endif
