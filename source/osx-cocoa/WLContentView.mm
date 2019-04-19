//
//  WLContentView.m
//  openwl
//
//  Created by Daniel X on 2/12/18.
//  Copyright (c) 2018 OpenWL Developers. All rights reserved.
//

#import "WLContentView.h"
#import "WLWindowObject.h"

#include "miscutil.h"
#include "keystuff.h"
#include "globals.h"

#include "private_defs.h"

@implementation WLContentView
@synthesize parentWindowObj;
//@synthesize trackingArea;
@synthesize sourceData;
@synthesize sourceMask;
@synthesize dragResult;

- (id)init {
    self = [super init];
    if (self) {
        keyDownSet = [NSMutableSet set];
        
        auto options =
        // when active?
        NSTrackingActiveWhenFirstResponder |
        // auto-size to visible area (ignore rect)
        NSTrackingInVisibleRect |
        // events we want:
        NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved |
        // other options
        NSTrackingEnabledDuringMouseDrag;
        
        trackingArea = [[[NSTrackingArea alloc]
                         initWithRect:self.bounds options:options owner:self userInfo:nil] autorelease];
        [self addTrackingArea:trackingArea];
    }
    return self;
}

- (void)dealloc {
    if (parentWindowObj.nsWindow == NULL) {
        // this is an orphan NSView used for things like audio plugin views
        // make sure the client gets its destroy message to properly clean up
        auto notification = [[NSNotification alloc] initWithName:NSWindowWillCloseNotification object:NULL userInfo:NULL];
        [parentWindowObj windowWillClose:notification];
        [notification release];
        
        // also need to destroy the dummy parent window ourselves, since nobody else knows anything about it

        // first unsub from frame changes ...
        [[NSNotificationCenter defaultCenter]
            removeObserver:parentWindowObj
                      name:NSViewFrameDidChangeNotification
                    object:self];
        
        // bye
        [parentWindowObj dealloc];
    }
    [super dealloc];
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent {
    // first click doesn't just grant focus -- actually does whatever
    return YES;
}

- (BOOL)isFlipped {
    return YES;
}

// key events
- (void)flagsChanged:(NSEvent *)theEvent {
    wl_Event event;
    event.handled = false;
    event.eventType = wl_kEventTypeKey;
    event.keyEvent.eventType = wl_kKeyEventTypeDown;
    event.keyEvent.modifiers = cocoa_to_wl_modifiers_multi(theEvent.modifierFlags);
    event.keyEvent.location = wl_kKeyLocationDefault;
    
    auto found = codeToKeyInfo.find(theEvent.keyCode);
    keymapInfo *info = found != codeToKeyInfo.end() ? found->second : nullptr;
    if (info) {
        event.keyEvent.key = info->key;
        event.keyEvent.string = info->stringRep;
        event.keyEvent.location = info->location;
        eventCallback((wl_WindowRef)parentWindowObj, &event, parentWindowObj.userData);
        
        if (!event.handled) {
            [super flagsChanged:theEvent];
        }
    } else {
        printf("unhandled code: %d\n", theEvent.keyCode);
        [super flagsChanged:theEvent];
    }
}

- (void)keyDown:(NSEvent *)theEvent {
    if (![rootMenu performKeyEquivalent:theEvent]) {
        // menu didn't have any shortcuts for it ...
        wl_Event event;
        event.handled = false;
        event.eventType = wl_kEventTypeKey;
        event.keyEvent.eventType = wl_kKeyEventTypeDown;
        event.keyEvent.modifiers = cocoa_to_wl_modifiers_multi(theEvent.modifierFlags);
        event.keyEvent.location = wl_kKeyLocationDefault;
        event.keyEvent.key = wl_kKeyUnknown;
        
        /**** keydown event *****/
        
        auto found = codeToKeyInfo.find(theEvent.keyCode);
        keymapInfo *info = found != codeToKeyInfo.end() ? found->second : nullptr;
        if (!info) {
            // try again with alphanumeric stuff, based on first-and-only character of string
            if ([theEvent.charactersIgnoringModifiers length] == 1) {
                auto ch = [[theEvent.charactersIgnoringModifiers uppercaseString] characterAtIndex:0];
                auto found2 = unicharToKeyInfo.find(ch);
                info = found2 != unicharToKeyInfo.end() ? found2->second : nullptr;
            }
        }
        if (info) {
            event.keyEvent.key = info->key;
            event.keyEvent.string = info->stringRep;
            event.keyEvent.location = info->location;
            eventCallback((wl_WindowRef)parentWindowObj, &event, parentWindowObj.userData);
        }
        auto keyDown_handled = event.handled;
        
        /**** char event *****/
        
        if (([theEvent.characters length] > 0) &&
            !(info && info->suppressCharEvent))
        {
            event.handled = false;
            event.keyEvent.eventType = wl_kKeyEventTypeChar;
            // .key, .modifiers, .location still set from earlier
            event.keyEvent.string = [theEvent.characters UTF8String];
            eventCallback((wl_WindowRef)parentWindowObj, &event, parentWindowObj.userData);
        }
        auto char_handled = event.handled;
        
        if (!(keyDown_handled || char_handled)) {
            printf("unhandled code: %d\n", theEvent.keyCode);
            [super keyDown:theEvent];
        }
        
        // add to currently-down set, until we hear back from it on the other end
        // we only send keyup events for ones in the set, to avoid sending up events
        // for things that get short-circuited into a menu action
        [keyDownSet addObject:theEvent];
    }
}

- (void)keyUp:(NSEvent *)theEvent {
    auto matching = [[keyDownSet allObjects] filteredArrayUsingPredicate:[NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary *bindings) {
        return ((NSEvent *)evaluatedObject).keyCode == theEvent.keyCode;
    }]];
    if (matching.count > 0) {
        [keyDownSet removeObject:matching[0]];
        
        //        wl_Event event;
        //        event.handled = false;
        //        event.eventType = wl_kEventTypeKey;
        //        event.keyEvent.eventType = wl_kKeyEventTypeUp;
        //        event.keyEvent.modifiers = cocoa_to_wl_modifiers_multi(theEvent.modifierFlags);
        //        event.keyEvent.string = [theEvent.characters UTF8String];
        //        event.keyEvent.keysym = keyCodeToKeySym(theEvent.keyCode, theEvent.modifierFlags);
        //
        //        eventCallback((wl_WindowRef)parentWindowObj, &event, parentWindowObj.userData);
        //        if (!event.handled) {
        //            [super keyDown:theEvent];
        //        }
    } else {
        // ignore, because it was never a valid keydown to begin with
        // (ie, was intercepted by a menu action)
    }
}

// mouse button common
- (BOOL)mouseButtonCommon:(wl_MouseEventType)mouseEventType
              whichButton:(wl_MouseButton)whichButton
                fromEvent:(NSEvent *)theEvent
{
    wl_EventPrivate priv;
    priv.event = theEvent;
    wl_Event event;
    event._private = &priv;
    event.handled = false;
    event.eventType = wl_kEventTypeMouse;
    event.mouseEvent.eventType = mouseEventType;
    event.mouseEvent.button = whichButton;
    
    auto point = [self convertPoint:theEvent.locationInWindow fromView:nil]; // utilize isFlipped property
    event.mouseEvent.x = point.x;
    event.mouseEvent.y = point.y;
    
    event.mouseEvent.modifiers = cocoa_to_wl_modifiers_multi(theEvent.modifierFlags);
    eventCallback((wl_WindowRef)parentWindowObj, &event, parentWindowObj.userData);
    return event.handled;
}

- (void)mouseDown:(NSEvent *)theEvent {
    //    printf("mouse down -- left button\n");
    if (![self mouseButtonCommon:wl_kMouseEventTypeMouseDown
                     whichButton:wl_kMouseButtonLeft
                       fromEvent:theEvent]) {
        [super mouseDown:theEvent];
    }
}

- (void)rightMouseDown:(NSEvent *)theEvent {
    //    printf("mouse down -- right button\n");
    if (![self mouseButtonCommon:wl_kMouseEventTypeMouseDown
                     whichButton:wl_kMouseButtonRight
                       fromEvent:theEvent]) {
        [super rightMouseDown:theEvent];
    }
}

- (void)otherMouseDown:(NSEvent *)theEvent {
    //    printf("mouse down -- MIDDLE button\n");
    if (![self mouseButtonCommon:wl_kMouseEventTypeMouseDown
                     whichButton:(theEvent.buttonNumber == 2 ? wl_kMouseButtonMiddle : wl_kMouseButtonOther)
                       fromEvent:theEvent]) {
        [super otherMouseDown:theEvent];
    }
}

- (void)mouseUp:(NSEvent *)theEvent {
    //    printf("mouse up\n");
    if (![self mouseButtonCommon:wl_kMouseEventTypeMouseUp
                     whichButton:wl_kMouseButtonLeft
                       fromEvent:theEvent]) {
        [super mouseUp:theEvent];
    }
}

- (void)rightMouseUp:(NSEvent *)theEvent {
    //    printf("right mouse up\n");
    if (![self mouseButtonCommon:wl_kMouseEventTypeMouseUp
                     whichButton:wl_kMouseButtonRight
                       fromEvent:theEvent]) {
        [super rightMouseUp:theEvent];
    }
}

- (void)otherMouseUp:(NSEvent *)theEvent {
    //    printf("other mouse up\n");
    if (![self mouseButtonCommon:wl_kMouseEventTypeMouseUp
                     whichButton:(theEvent.buttonNumber == 2 ? wl_kMouseButtonMiddle : wl_kMouseButtonOther)
                       fromEvent:theEvent]) {
        [super otherMouseUp:theEvent];
    }
}

- (void)mouseEntered:(NSEvent *)theEvent {
    //    printf("mouse entered\n");
    if (![self mouseButtonCommon:wl_kMouseEventTypeMouseEnter
                     whichButton:wl_kMouseButtonNone
                       fromEvent:theEvent]) {
        [super mouseEntered:theEvent];
    }
}

- (void)mouseExited:(NSEvent *)theEvent {
    //    printf("mouse exited\n");
    if (![self mouseButtonCommon:wl_kMouseEventTypeMouseLeave
                     whichButton:wl_kMouseButtonNone
                       fromEvent:theEvent]) {
        [super mouseExited:theEvent];
    }
}

- (void)mouseMoved:(NSEvent *)theEvent {
    //    printf("mouse moved!\n");
    if (![self mouseButtonCommon:wl_kMouseEventTypeMouseMove
                     whichButton:wl_kMouseButtonNone
                       fromEvent:theEvent]) {
        [super mouseMoved:theEvent];
    }
}

- (void)mouseDragged:(NSEvent *)theEvent {
    //    printf("mouse dragged!\n");
    if (![self mouseButtonCommon:wl_kMouseEventTypeMouseMove
                     whichButton:wl_kMouseButtonNone
                       fromEvent:theEvent]) {
        [super mouseDragged:theEvent];
    }
}

- (void)drawRect:(NSRect)dirtyRect {
    // no need to call super, since direct descendant of NSView
    //    auto size = self.bounds.size;
    
    wl_Event event;
    event.handled = false;
    event.eventType = wl_kEventTypeWindowRepaint;
    event.repaintEvent.x = dirtyRect.origin.x;
    event.repaintEvent.y = dirtyRect.origin.y;
    event.repaintEvent.width = dirtyRect.size.width;
    event.repaintEvent.height = dirtyRect.size.height;
    
    auto context = [NSGraphicsContext currentContext];
    CGContextRef cgc = (CGContextRef)[context graphicsPort];
    //    // invert xform (using isFlipped instead)
    //    CGContextTranslateCTM (cgc, 0.0, size.height);
    //    CGContextScaleCTM (cgc, 1.0, -1.0);
    event.repaintEvent.platformContext = cgc;
    
    eventCallback((wl_WindowRef)parentWindowObj, &event, parentWindowObj.userData);
    // nothing to do if not handled (no super to forward to ...)
}

// dnd-related stuff
- (void)enableDrops {
    NSArray *types = [NSArray arrayWithObjects:
                      NSPasteboardTypeString,
                      NSFilenamesPboardType, // deprecated in 10.13...
                      NSURLPboardType,
                      nil];
    [self registerForDraggedTypes:types];
}

- (void)disableDrops {
    [self unregisterDraggedTypes];
}

- (NSDragOperation) dragCommonEvent:(id<NSDraggingInfo>)sender dropEventType:(wl_DropEventType)eventType {
    auto sourceDragMask = [sender draggingSourceOperationMask];
    printf("source drag mask: %08lX\n", sourceDragMask);
    
    auto allowedEffectMask = cocoa_to_wl_dropEffect_multi(sourceDragMask);
    printf("sending allowed: %d\n", allowedEffectMask);
    // cocoa doesn't have a default drop operation per se (because it either hands us
    //   the raw source mask, or applies its own masking based on modifier keys before we get this,
    //   so just pick a first reasonable default
    auto defaultDragOperation =
    ((allowedEffectMask & wl_kDropEffectCopy) ? wl_kDropEffectCopy :
     ((allowedEffectMask & wl_kDropEffectLink) ? wl_kDropEffectLink :
      ((allowedEffectMask & wl_kDropEffectMove) ? wl_kDropEffectMove :
       ((allowedEffectMask & wl_kDropEffectOther) ? wl_kDropEffectOther :
        wl_kDropEffectNone))));
    printf("sending default: %d\n", defaultDragOperation);
    
    auto data = new wl_DropData;
    data->pboard = [sender draggingPasteboard]; // need to retain?
    
    // view-converted (y-inverted) location
    auto location = [self convertPoint:[sender draggingLocation] fromView:nil];
    
    wl_Event event;
    event.handled = false;
    event.eventType = wl_kEventTypeDrop;
    event.dropEvent.eventType = eventType;
    event.dropEvent.x = location.x;
    event.dropEvent.y = location.y;
    event.dropEvent.allowedEffectMask = allowedEffectMask;
    event.dropEvent.defaultModifierAction = defaultDragOperation;
    event.dropEvent.data = data;
    
    eventCallback((wl_WindowRef)parentWindowObj, &event, parentWindowObj.userData);
    
    delete data; // not needed anymore
    
    if (event.handled) {
        printf("post: allowed: %d\n", event.dropEvent.allowedEffectMask);
        auto chosen = wl_to_cocoa_dropEffect_single(event.dropEvent.allowedEffectMask);
        printf("post: chosen cocoa drop effect: %ld\n", chosen);
        return chosen;
    } else {
        return NSDragOperationNone;
    }
    return NSDragOperationNone;
}

// drop destination protocol methods
- (NSDragOperation) draggingEntered:(id<NSDraggingInfo>)sender {
    printf("dragging entered!\n");
    return [self dragCommonEvent:sender dropEventType:wl_kDropEventTypeFeedback];
}

- (NSDragOperation) draggingUpdated:(id<NSDraggingInfo>)sender {
    printf("dragging updated!\n");
    return [self dragCommonEvent:sender dropEventType:wl_kDropEventTypeFeedback];
}

- (BOOL) performDragOperation:(id<NSDraggingInfo>)sender {
    printf("performDragOperation...\n");
    [self dragCommonEvent:sender dropEventType:wl_kDropEventTypeDrop];
    return YES;
}

// drag source protocol methods
- (void)draggedImage:(NSImage *)image endedAt:(NSPoint)screenPoint operation:(NSDragOperation)operation {
    // delete / move etc
    self.dragResult = cocoa_to_wl_dropEffect_single(operation);
    printf("drag ended (operation %ld) -- exiting nested runloop...\n", operation);
    CFRunLoopStop([[NSRunLoop currentRunLoop] getCFRunLoop]);
}

- (NSArray *)writableTypesForPasteboard:(NSPasteboard *)pasteboard {
    printf("writableTypesForPasteboard\n");
    auto ret = [NSMutableArray array];
    for (auto fmt=self.sourceData->formats.begin(); fmt!=self.sourceData->formats.end(); fmt++) {
        [ret addObject:(NSString *)wl_to_cocoa_dragFormat(fmt->c_str())];
    }
    return ret;
}

- (NSPasteboardWritingOptions)writingOptionsForType:(NSString *)type pasteboard:(NSPasteboard *)pasteboard {
    printf("writingOptionsForType\n");
    return NSPasteboardWritingPromised; // woot, defer until actually dropped
}

- (id)pasteboardPropertyListForType:(NSString *)type {
    // produce the data
    wl_Event event;
    event.handled = false;
    event.eventType = wl_kEventTypeDragRender;
    event.dragRenderEvent.dragFormat = cocoa_to_wl_dragFormat((CFStringRef)type);
    
    auto payload = new wl_RenderPayload;
    event.dragRenderEvent.payload = payload;
    
    eventCallback((wl_WindowRef)parentWindowObj, &event, parentWindowObj.userData);
    
    id ret = nil;
    if (event.handled) {
        ret = payload->data;
        //        NSData *retData = [NSData dataWithBytes:event.dragRenderEvent.data length:event.dragRenderEvent.dataSize];
        //        free(event.dragRenderEvent.data);
        //        return retData;
    }
    delete payload; // the inner data will go out via ret .. not sure about retain/releases here ...
    return ret;
}

- (NSDragOperation)draggingSession:(NSDraggingSession *)session sourceOperationMaskForDraggingContext:(NSDraggingContext)context {
    printf("providing drag mask: %08lX\n", self.sourceMask);
    switch (context) {
        case NSDraggingContextOutsideApplication:
            return self.sourceMask;
            
        case NSDraggingContextWithinApplication:
            return self.sourceMask;
            
        default:
            break;
    }
    return self.sourceMask;
}

@end
