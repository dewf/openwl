//
//  WLWindowObject.m
//  openwl
//
//  Created by Daniel X on 2/12/18.
//  Copyright (c) 2018 OpenWL Developers. All rights reserved.
//

#import "WLWindowObject.h"
#import "WLContentView.h"

#include "globals.h"
#include "private_defs.h"

@implementation WLWindowObject

@synthesize nsWindow;
@synthesize contentViewObj;
@synthesize userData;
@synthesize width;
@synthesize height;

- (void)windowDidBecomeKey:(NSNotification *)notification {
    wl_Event event;
    event.handled = false;
    event.eventType = wl_kEventTypeFocusChange;
    event.focusChangeEvent.state = true;
    eventCallback((wl_WindowRef)self, &event, userData);
    // .handled not used
}

- (void)windowDidResignKey:(NSNotification *)notification {
    wl_Event event;
    event.handled = false;
    event.eventType = wl_kEventTypeFocusChange;
    event.focusChangeEvent.state = false;
    eventCallback((wl_WindowRef)self, &event, userData);
    // .handled not used
}

- (BOOL)windowShouldClose:(NSWindow *)sender {
    wl_Event event;
    event.handled = false;
    event.eventType = wl_kEventTypeWindowCloseRequest;
    event.closeRequestEvent.cancelClose = false;
    eventCallback((wl_WindowRef)self, &event, userData);
    if (event.handled && event.closeRequestEvent.cancelClose) {
        return NO;
    } else {
        return YES;
    }
}

- (void)windowWillClose:(NSNotification *)notification {
    wl_Event event;
    event.handled = false;
    event.eventType = wl_kEventTypeWindowDestroyed;
    event.destroyEvent.reserved = 0;
    eventCallback((wl_WindowRef)self, &event, userData);
    // doesn't matter if handled
}

- (void)windowDidResize:(NSNotification *)notification {
    auto size = [contentViewObj frame].size;
    int newWidth = size.width;
    int newHeight = size.height;
    
    wl_Event event;
    event.handled = false;
    event.eventType = wl_kEventTypeWindowResized;
    event.resizeEvent.oldWidth = width;
    event.resizeEvent.oldHeight = height;
    event.resizeEvent.newWidth = newWidth;
    event.resizeEvent.newHeight = newHeight;
    eventCallback((wl_WindowRef)self, &event, userData);
    // doesn't matter if handled
    
    width = newWidth;
    height = newHeight;
}

// forwarded from app delegate
- (void)menuItemAction:(id)sender {
    NSMenuItem *menuItem = sender;
    WLActionObject *actionObj = [menuItem representedObject];
    
    wl_Event event;
    event.handled = false;
    event.eventType = wl_kEventTypeAction;
    event.actionEvent.id = actionObj._id;
    event.actionEvent.action = (wl_ActionRef)actionObj;
    eventCallback((wl_WindowRef)self, &event, userData);
}

- (NSPoint) pointToScreen: (NSPoint)p {
    auto contentFrame = [contentViewObj frame];
    auto flippedY = contentFrame.size.height - p.y;
    auto screenRect = [nsWindow convertRectToScreen:NSMakeRect(p.x + contentFrame.origin.x, flippedY + contentFrame.origin.y, 0, 0)];
    return screenRect.origin;
}

@end //WLWindowObject











