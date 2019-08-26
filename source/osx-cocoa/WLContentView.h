//
//  WLContentView.h
//  openwl
//
//  Created by Daniel X on 2/12/18.
//  Copyright (c) 2018 OpenWL Developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "../openwl.h"

//#import "WLWindowObject.h"
@class WLWindowObject; // fwd decl to avoid circular ref

@interface WLContentView : NSView <NSDraggingSource, NSPasteboardWriting> {
    NSMutableSet *keyDownSet;
    NSTrackingArea *trackingArea;
    NSCursor *cursor;
    bool containsCursor;
}
//@property (retain) NSTrackingArea *trackingArea;
@property (assign) WLWindowObject *parentWindowObj;
@property wl_DragDataRef sourceData;
@property NSDragOperation sourceMask;
@property wl_DropEffect dragResult;
- (id)init;
- (void)drawRect:(NSRect)dirtyRect;
- (void)enableDrops;
- (void)disableDrops;
- (void)setCursor:(NSCursor *)cursor;
@end
