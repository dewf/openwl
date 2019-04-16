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

- (BOOL)windowShouldClose:(NSWindow *)sender {
    WLEvent event;
    event.handled = false;
    event.eventType = WLEventType_WindowCloseRequest;
    event.closeRequestEvent.cancelClose = false;
    eventCallback((wlWindow)self, &event, userData);
    if (event.handled && event.closeRequestEvent.cancelClose) {
        return NO;
    } else {
        return YES;
    }
}

- (void)windowWillClose:(NSNotification *)notification {
    WLEvent event;
    event.handled = false;
    event.eventType = WLEventType_WindowDestroyed;
    event.destroyEvent.reserved = 0;
    eventCallback((wlWindow)self, &event, userData);
    // doesn't matter if handled
}

- (void)windowDidResize:(NSNotification *)notification {
    auto size = [contentViewObj frame].size;
    int newWidth = size.width;
    int newHeight = size.height;
    
    WLEvent event;
    event.handled = false;
    event.eventType = WLEventType_WindowResized;
    event.resizeEvent.oldWidth = width;
    event.resizeEvent.oldHeight = height;
    event.resizeEvent.newWidth = newWidth;
    event.resizeEvent.newHeight = newHeight;
    eventCallback((wlWindow)self, &event, userData);
    // doesn't matter if handled
    
    width = newWidth;
    height = newHeight;
}

// forwarded from app delegate
- (void)menuItemAction:(id)sender {
    NSMenuItem *menuItem = sender;
    WLActionObject *actionObj = [menuItem representedObject];
    
    WLEvent event;
    event.handled = false;
    event.eventType = WLEventType_Action;
    event.actionEvent.id = actionObj._id;
    event.actionEvent.action = (wlAction)actionObj;
    eventCallback((wlWindow)self, &event, userData);
}

- (NSPoint) pointToScreen: (NSPoint)p {
    auto contentFrame = [contentViewObj frame];
    auto flippedY = contentFrame.size.height - p.y;
    auto screenRect = [nsWindow convertRectToScreen:NSMakeRect(p.x + contentFrame.origin.x, flippedY + contentFrame.origin.y, 0, 0)];
    return screenRect.origin;
}

@end //WLWindowObject











