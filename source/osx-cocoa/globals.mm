//
//  globals.cpp
//  openwl
//
//  Created by Daniel X on 2/12/18.
//  Copyright (c) 2018 OpenWL Developers. All rights reserved.
//

#include "globals.h"

#import <Foundation/Foundation.h>

wlEventCallback eventCallback = NULL;
NSAutoreleasePool *pool = nil;
NSApplication *sharedApp = nil;
WLAppDelegate *myDelegate = nil;
NSMenu *rootMenu = nil;
NSMenu *appMenu = nil;

clock_serv_t systemClock;   // for system clock

