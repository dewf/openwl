//
//  WLWindowObject.h
//  openwl
//
//  Created by Daniel X on 2/12/18.
//  Copyright (c) 2018 OpenWL Developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

//#import "WLContentView.h"
@class WLContentView; // fwd decl to avoid circular ref

// no struct _wlWindow, just a cast of a WLWindowObject id
@interface WLWindowObject : NSObject <NSWindowDelegate>
@property (assign) NSWindow *nsWindow;
@property (assign) WLContentView *contentViewObj;
@property void *userData;
@property int width;
@property int height;
- (void)menuItemAction:(id)sender;
- (NSPoint) pointToScreen: (NSPoint)p;
@end
