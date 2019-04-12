//
//  WLAppDelegate.m
//  openwl
//
//  Created by Daniel X on 2/12/18.
//  Copyright (c) 2018 OpenWL Developers. All rights reserved.
//

#import "WLAppDelegate.h"

#import "WLWindowObject.h"
#include "globals.h"

@implementation WLAppDelegate
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    NSLog(@"woot, app did finish launching");
    [NSApp activateIgnoringOtherApps:YES];
}
- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}
- (void)menuItemAction:(id)sender {
    // just forward to active window
    auto activeWindow = [sharedApp mainWindow];
    [(WLWindowObject *)[activeWindow delegate] menuItemAction:sender];
}
@end
