//
//  NSCustomWindow.m
//  OpenWL
//
//  Created by Daniel G on 10/29/19.
//  Copyright © 2019 Daniel G. All rights reserved.
//

#import "NSCustomWindow.h"

@implementation NSCustomWindow

- (BOOL) canBecomeKeyWindow {
    return YES; // necessary for borderless windows to receive mouse events
}

@end
