//
//  globals.h
//  openwl
//
//  Created by Daniel X on 2/12/18.
//  Copyright (c) 2018 OpenWL Developers. All rights reserved.
//

#ifndef __openwl__globals__
#define __openwl__globals__

#include "../openwl.h"
#import <AppKit/AppKit.h>
#import "WLAppDelegate.h"

extern wlEventCallback eventCallback;
extern NSAutoreleasePool *pool;
extern NSApplication *sharedApp;
extern WLAppDelegate *myDelegate;
extern NSMenu *rootMenu;
extern NSMenu *appMenu;

#include <mach/clock.h>
#include <mach/mach.h>
extern clock_serv_t systemClock;   // for system clock

#endif /* defined(__openwl__globals__) */
