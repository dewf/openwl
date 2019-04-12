//#   define bool _Bool
#include "../openwl.h"

#include <stdio.h>
#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#import <algorithm>
#import <set>

#include "miscutil.h"
#include "keystuff.h"
#include "globals.h"

#import "WLAppDelegate.h"
#import "WLWindowObject.h"
#import "WLContentView.h"
#import "MainThreadExecutor.h"

#include "private_defs.h"

/************** API begin ************/

OPENWL_API enum WLPlatform wlGetPlatform()
{
    return WLPlatform_Mac;
}

OPENWL_API int CDECL wlInit(wlEventCallback callback, struct WLPlatformOptions *options)
{
    initKeyMap();
    
    eventCallback = callback;

    pool = [[NSAutoreleasePool alloc] init];
    
    sharedApp = [NSApplication sharedApplication];
    
    myDelegate = [[WLAppDelegate alloc] init];
    [sharedApp setDelegate:myDelegate];

    // root menu
    
    rootMenu = [[NSMenu alloc] init];
    // create application menu that every app has
    auto appMenuItem = [rootMenu addItemWithTitle:@"APPMENU" action:nil keyEquivalent:@""];
    appMenu = [[NSMenu alloc] init];
    [appMenuItem setSubmenu:appMenu];
        
    [sharedApp setMainMenu:rootMenu];

    // end menu
    
    // get monotonic clock for timers
    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &systemClock);
    
    printf("Hello from wlInit\n");
    return 0;
}

OPENWL_API int CDECL wlRunloop()
{
    [sharedApp run];
    return 0;
}

OPENWL_API void CDECL wlExitRunloop()
{
    [sharedApp stop:nil];
}

OPENWL_API void CDECL wlShutdown()
{
    [myDelegate release];
    [pool release];
    
    // dealloc monotonic clock for timers
    mach_port_deallocate(mach_task_self(), systemClock);
    
    printf("wlShutdown complete\n");
}


/*** WINDOW API ***/
static void setProps(NSWindow *window, WLWindowProperties *props) {
    NSSize minSize = { FLT_MIN, FLT_MIN };
    NSSize maxSize = { FLT_MAX, FLT_MAX };
    if (props->usedFields & WLWindowProp_MinWidth)
        minSize.width = props->minWidth;
    if (props->usedFields & WLWindowProp_MinHeight)
        minSize.height = props->minHeight;
    if (props->usedFields & WLWindowProp_MaxWidth)
        maxSize.width = props->maxWidth;
    if (props->usedFields & WLWindowProp_MaxHeight)
        maxSize.height = props->maxHeight;
    [window setContentMinSize:minSize];
    [window setContentMaxSize:maxSize];
}

OPENWL_API wlWindow CDECL wlWindowCreate(int width, int height, const char *title, void *userData, struct WLWindowProperties *props)
{
    NSRect frame = NSMakeRect(300, 300, width, height);
    
    NSWindowStyleMask styleMask = NSTitledWindowMask|NSClosableWindowMask|NSMiniaturizableWindowMask|NSResizableWindowMask;
    if (props && (props->usedFields & WLWindowProp_Style) && (props->style == WLWindowStyle_Frameless)) {
        styleMask = NSWindowStyleMaskBorderless;
    }
    auto nsWindow = [[NSWindow alloc] initWithContentRect:frame
                                                     styleMask:styleMask
                                                       backing:NSBackingStoreBuffered
                                                        defer:NO];
    
    auto ret = [[WLWindowObject alloc] init];
    ret.nsWindow = nsWindow;
    ret.userData = userData;
    ret.width = width;
    ret.height = height;

    [nsWindow setDelegate:ret];
    if (title) {
        [nsWindow setTitle:[NSString stringWithUTF8String:title]];
    }
    //[nsWindow setBackgroundColor:[NSColor blueColor]];
    
    // create content view (handles all input / redraw events)
    auto contentViewObj = [[WLContentView alloc] init];
    ret.contentViewObj = contentViewObj;
    contentViewObj.parentWindowObj = ret;
    [nsWindow setContentView:contentViewObj];
    
    if (props != nullptr) {
        setProps(nsWindow, props);
    }
    
//    [nsWindow makeKeyAndOrderFront:NSApp];
    printf("nswindow is: %zX\n", (size_t)nsWindow);
    
    return (wlWindow)ret;
}

OPENWL_API void CDECL wlWindowDestroy(wlWindow window)
{
    WLWindowObject *obj = (WLWindowObject *)window;
    [obj.nsWindow close];
    // use orderOut instead? to set next key window?
}

OPENWL_API void CDECL wlWindowShow(wlWindow window)
{
    WLWindowObject *obj = (WLWindowObject *)window;
    [obj.nsWindow makeKeyAndOrderFront:NSApp];
//    [obj.nsWindow setIsVisible:YES];
}

OPENWL_API void CDECL wlWindowShowRelative(wlWindow window, wlWindow relativeTo, int x, int y, int newWidth, int newHeight)
{
    WLWindowObject *obj = (WLWindowObject *)window;
    WLWindowObject *relObj = (WLWindowObject *)relativeTo;
    
    auto newOrigin = [relObj pointToScreen:NSMakePoint(x, y)];
    [obj.nsWindow setFrameTopLeftPoint:newOrigin];
    
    if (newWidth > 0 && newHeight > 0) {
        [obj.nsWindow setContentSize:NSMakeSize(newWidth, newHeight)];
    }
    
    [obj.nsWindow orderFront:NULL];
    [obj.nsWindow setIsVisible:YES];
}

OPENWL_API void CDECL wlWindowHide(wlWindow window)
{
    WLWindowObject *obj = (WLWindowObject *)window;
    [obj.nsWindow setIsVisible:NO];
}

OPENWL_API void CDECL wlWindowInvalidate(wlWindow window, int x, int y, int width, int height)
{
    WLWindowObject *windowObj = (WLWindowObject *)window;
    if (width > 0 && height > 0) {
        [windowObj.contentViewObj setNeedsDisplayInRect:NSMakeRect(x, y, width, height)];
    } else {
        [windowObj.contentViewObj setNeedsDisplay:YES];
    }
}

OPENWL_API size_t CDECL wlWindowGetOSHandle(wlWindow window)
{
    WLWindowObject *windowObj = (WLWindowObject *)window;
    // you've already got it, buddy!
    return (size_t)windowObj;
}


OPENWL_API void CDECL wlMouseGrab(wlWindow window)
{
    // noop, seems to be the default behavior on OSX
}

OPENWL_API void CDECL wlMouseUngrab()
{
    // noop
}

static void sizeToFit(CGSize *size, int maxWidth, int maxHeight)
{
    double sourceAspect = size->width / size->height; // already floating point
    double targetAspect = (double)maxWidth / (double)maxHeight;
    if (sourceAspect <= targetAspect) {
        // clamp height, width will be OK
        size->height = maxHeight;
        size->width = int(sourceAspect * maxHeight);
        assert(size->width <= maxWidth);
    } else {
        // clamp width, height will be OK
        size->width = maxWidth;
        size->height = int(maxWidth / sourceAspect);
        assert(size->height <= maxHeight);
    }
}

OPENWL_API wlIcon CDECL wlIconLoadFromFile(const char *filename, int sizeToWidth)
{
    auto fname = [NSString stringWithUTF8String:filename];
    auto bundle = [NSBundle mainBundle];
    auto path = [bundle pathForResource:fname ofType:nil];
    NSImage *image = [[NSImage alloc] initWithContentsOfFile:path];
    
    auto newSize = [image size];
    sizeToFit(&newSize, sizeToWidth, sizeToWidth);
    
    auto scaled = resizeImage(image, newSize);
    [image release];
    
    auto ret = new _wlIcon;
    ret->image = scaled;
    return ret;
}

OPENWL_API wlAccelerator CDECL wlAccelCreate(enum WLKeyEnum key, unsigned int modifiers)
{
    auto ret = new _wlAccelerator;
    ret->key = key;
    ret->modifiers = modifiers;
    return ret;
}

OPENWL_API wlAction CDECL wlActionCreate(int id, const char *label, wlIcon icon, wlAccelerator accel)
{
    auto ret = [[WLActionObject alloc] init];
    ret._id = id;
    ret.label = [[NSString stringWithUTF8String:label]
                  stringByReplacingOccurrencesOfString:@"&" withString:@""]; // since we don't use mnemonics
    ret.icon = icon;
    ret.accel = accel;
    
    return (wlAction)ret;
}

OPENWL_API wlMenu CDECL wlMenuCreate()
{
    auto ret = new _wlMenu;
    ret->menu = [[NSMenu alloc] init];
    return ret;
}

OPENWL_API wlMenuItem CDECL wlMenuAddAction(wlMenu menu, wlAction action)
{
    WLActionObject *actionObj = (WLActionObject *)action;

    NSMenuItem *item = nil;
    if (actionObj.accel) {
        auto info = reverseKeymap[actionObj.accel->key];
        auto equiv = [[NSString stringWithUTF8String:info->stringRep] lowercaseString];
//        auto equiv = [[NSString stringWithFormat:@"%c", actionObj.accel->key] lowercaseString];
        item = [menu->menu addItemWithTitle:actionObj.label
                                          action:@selector(menuItemAction:)
                                   keyEquivalent:equiv];
        NSUInteger modMask =
        ((actionObj.accel->modifiers & WLModifier_Control) ? NSCommandKeyMask : 0) |
        ((actionObj.accel->modifiers & WLModifier_Alt) ? NSAlternateKeyMask : 0) |
        ((actionObj.accel->modifiers & WLModifier_Shift) ? NSShiftKeyMask : 0);

        [item setKeyEquivalentModifierMask:modMask];
    } else {
        item = [menu->menu addItemWithTitle:actionObj.label
                                     action:@selector(menuItemAction:)
                              keyEquivalent:@""];
    }

    [item setRepresentedObject:actionObj];
    
    if (actionObj.icon) {
        printf("setting icon %zX\n", (size_t)actionObj.icon->image);
        [item setImage:actionObj.icon->image];
    }
    
    auto ret = new _wlMenuItem;
    ret->menuItem = item;
    return ret;
}

static wlMenuItem addSubMenuCommon(NSMenu *parent, const char *label, NSMenu *child) {
    auto title = [[NSString stringWithUTF8String:label]
                  stringByReplacingOccurrencesOfString:@"&" withString:@""];
    auto item = [parent addItemWithTitle:title action:nil keyEquivalent:@""];

    [child setTitle:title]; // apparently necessary ...
    [item setSubmenu:child];
    
    auto ret = new _wlMenuItem;
    ret->menuItem = item;
    return ret;
}

OPENWL_API wlMenuItem CDECL wlMenuAddSubmenu(wlMenu menu, const char *label, wlMenu sub)
{
    return addSubMenuCommon(menu->menu, label, sub->menu);
}

OPENWL_API void CDECL wlMenuAddSeparator(wlMenu menu)
{
    [menu->menu addItem:[NSMenuItem separatorItem]];
}

OPENWL_API wlMenuItem CDECL wlMenuBarAddMenu(wlMenuBar menuBar, const char *label, wlMenu menu)
{
    return addSubMenuCommon(menuBar->menuBar, label, menu->menu);
}

OPENWL_API void CDECL wlWindowShowContextMenu(wlWindow window, int x, int y, wlMenu menu, struct WLEvent *fromEvent)
{
    WLWindowObject *windowObj = (WLWindowObject *)window;
    [NSMenu popUpContextMenu:menu->menu
                   withEvent:fromEvent->_private->event
                     forView:windowObj.contentViewObj];
}

// Mac-specific menu stuff (sigh)
OPENWL_API wlMenuBar CDECL wlMenuBarGetDefault()
{
    auto ret = new _wlMenuBar;
    ret->menuBar = rootMenu;
    return ret;
}

OPENWL_API wlMenu CDECL wlGetApplicationMenu()
{
    if (appMenu) {
        auto ret = new _wlMenu;
        ret->menu = appMenu;
        return ret;
    } else {
        return nullptr;
    }
}

/********* timer API ************/

OPENWL_API wlTimer CDECL wlTimerCreate(wlWindow window, int timerID, unsigned int msTimeout)
{
    double interval = msTimeout / 1000.0;
    auto ret = [[WLTimerObject alloc] init];
    ret._id = timerID;
    ret.forWindow = (WLWindowObject *)window;
    clock_get_time(systemClock, &ret->lastTime);
    
    ret.timer = [NSTimer scheduledTimerWithTimeInterval:interval target:ret selector:@selector(timerFired:) userInfo:nil repeats:YES];
    return (wlTimer)ret;
}

OPENWL_API void CDECL wlTimerDestroy(wlTimer timer)
{
    WLTimerObject *timerObj = (WLTimerObject *)timer;
    [timerObj stopTimer];
    [timerObj release];
}

/************ DND API ***************/

OPENWL_API const char *kWLDragFormatUTF8 = "text/plain;charset=UTF-8";
OPENWL_API const char *kWLDragFormatFiles = "text/uri-list";

OPENWL_API void wlWindowEnableDrops(wlWindow window, bool enabled)
{
    WLWindowObject *windowObj = (WLWindowObject *)window;
    if (enabled) {
        [windowObj.contentViewObj enableDrops];
    } else {
        [windowObj.contentViewObj disableDrops];
    }
}

OPENWL_API bool CDECL wlDropHasFormat(wlDropData dropData, const char *dropFormatMIME)
{
//    NSPasteboardTypeString,
//    NSFilenamesPboardType, // deprecated in 10.13...
//    NSURLPboardType,
    auto types = [dropData->pboard types];
    if ([types containsObject:(NSString *)wl_to_cocoa_dragFormat(dropFormatMIME)]) {
        return true;
    }
    return false;
}

OPENWL_API bool CDECL wlDropGetFormat(wlDropData dropData, const char *dropFormatMIME, const void **outData, size_t *outSize)
{
    auto cocoaDragFormat = (NSString *)wl_to_cocoa_dragFormat(dropFormatMIME);

    auto types = [dropData->pboard types];
    if ([types containsObject:cocoaDragFormat]) {
        auto data = [dropData->pboard dataForType:cocoaDragFormat];
        auto tempSize = [data length];
        auto tempData = [data bytes];
        dropData->data = malloc(tempSize);
        dropData->dataSize = tempSize;
        memcpy(dropData->data, tempData, tempSize);
        
        *outData = dropData->data;
        *outSize = dropData->dataSize;
        return true;
    }
    // else
    *outData = nullptr;
    *outSize = 0;
    return false;
}

OPENWL_API bool CDECL wlDropGetFiles(wlDropData dropData, const struct WLFiles **outFiles)
{
    auto types = [dropData->pboard types];
    if ([types containsObject:NSFilenamesPboardType]) {
        NSArray *pboard_files = [dropData->pboard propertyListForType:NSFilenamesPboardType];
        
        auto numFiles = (int)[pboard_files count];
        dropData->files = new _wlFilesInternal(numFiles);
        for (int i=0; i< numFiles; i++) {
            const char *utf8 = [(NSString*)(pboard_files[i]) UTF8String];
            dropData->files->filenames[i] = strdup(utf8);
        }
        
        *outFiles = dropData->files;
        return true;
    }
    *outFiles = nullptr;
    return false;
}

OPENWL_API wlDragData CDECL wlDragDataCreate(wlWindow forWindow)
{
    auto ret = new _wlDragData;
    ret->forWindow = forWindow;
    ret->formats.clear();
    return ret;
}

OPENWL_API void CDECL wlDragDataRelease(wlDragData *dragData)
{
    delete *dragData;
    *dragData = nullptr;
}

OPENWL_API void CDECL wlDragAddFormat(wlDragData dragData, const char *dragFormatMIME)
{
    dragData->formats.insert(dragFormatMIME);
}

OPENWL_API enum WLDropEffect CDECL wlDragExec(wlDragData dragData, unsigned int dropActionsMask, struct WLEvent *fromEvent)
{
    WLContentView *contentView = ((WLWindowObject *)dragData->forWindow).contentViewObj;
    
    // load a drag image, sigh
    // need to do this only once, elsewhere
    auto bundle = [NSBundle mainBundle];
    auto path = [bundle pathForResource:@"_icons/file.png" ofType:nil];
    NSImage *image = [[NSImage alloc] initWithContentsOfFile:path];

    auto x = fromEvent->mouseEvent.x;
    auto y = fromEvent->mouseEvent.y;

    // pre-set some things that will be asked for by the dragging protocols ...
    contentView.sourceData = dragData;
    contentView.sourceMask = wl_to_cocoa_dropEffect_multi(dropActionsMask);

//    auto pboard = [NSPasteboard pasteboardWithName:NSDragPboard];
//    printf("performing actual drag...\n");
//    [contentView dragImage:image
//                        at:NSMakePoint(x, y)
//                    offset:NSMakeSize(0, 0)
//                     event:fromEvent->_private->event
//                pasteboard:pboard
//                    source:contentView
//                 slideBack:YES];

//    new style below :(
    auto item = [[NSDraggingItem alloc] initWithPasteboardWriter:contentView];
    [item setDraggingFrame:NSMakeRect(x, y, 100, 100) contents:image];
    
    // below returns a session object, but we don't use it
    [contentView beginDraggingSessionWithItems:[NSArray arrayWithObjects:item, nil]
                                                        event:fromEvent->_private->event
                                                       source:contentView];
    [item release];
    [image release];
    
    // created a nested runloop, which will exit when the drag operation completes
    // we're just matching the behavior of other platforms, where a DragExec is blocking
    printf("going into subloop...\n");
    auto subloop = [[NSRunLoop alloc] init];
    [subloop run];
    printf("exited subloop ..\n");
    
    printf("returning from dragexec!\n");
    return WLDropEffect_None;
}

// rendering convenience
OPENWL_API void CDECL wlDragRenderUTF8(wlRenderPayload payload, const char *text)
{
    payload->data = [NSString stringWithUTF8String:text];
}

OPENWL_API void CDECL wlDragRenderFiles(wlRenderPayload payload, const struct WLFiles *files)
{
    // nothing yet
}

/********* clipboard API ****************/

OPENWL_API wlDropData CDECL wlClipboardGet()
{
    auto ret = new _wlDropData;
    ret->pboard = [[NSPasteboard pasteboardWithName:NSGeneralPboard] retain];
    return ret;
}

OPENWL_API void CDECL wlClipboardSet(wlDragData dragData)
{
    auto pboard = [NSPasteboard pasteboardWithName:NSGeneralPboard];
    [pboard clearContents];
    
    // wrap all available formats (and the possibility of rendering them)
    // rendering will be deferred until actual paste :D
    WLContentView *contentView = ((WLWindowObject *)dragData->forWindow).contentViewObj;
    contentView.sourceData = dragData;
    [pboard writeObjects:[NSArray arrayWithObject:contentView]];
}

OPENWL_API void CDECL wlClipboardRelease(wlDropData dropData)
{
    [dropData->pboard release];
    delete dropData;
}

OPENWL_API void CDECL wlClipboardFlush()
{
    printf("clipboard flushing not yet implemented on OSX\n");
}


/***** Misc Stuff ******/

OPENWL_API void CDECL wlExecuteOnMainThread(wlWindow window, wlVoidCallback callback, void *data)
{
    auto mte = [MainThreadExecutor withCallback:callback param:data];
    [mte performSelectorOnMainThread:@selector(executeCallback) withObject:nil waitUntilDone:YES];
}

OPENWL_API void CDECL wlSleep(unsigned int millis)
{
    auto interval = ((double)millis) / 1000.0;
    [NSThread sleepForTimeInterval:interval];
}

OPENWL_API size_t CDECL wlSystemMillis()
{
    mach_timespec_t currentTime;
    clock_get_time(systemClock, &currentTime);
    
    unsigned long millis = currentTime.tv_sec * 1000;
    millis += currentTime.tv_nsec / 1000000;
    return millis;
}

