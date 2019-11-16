//#   define bool _Bool
#include "../openwl.h"

#include <stdio.h>
#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#include <AvailabilityMacros.h>

#import <algorithm>
#import <set>

#include "miscutil.h"
#include "keystuff.h"
#include "globals.h"

#import "WLAppDelegate.h"
#import "WLWindowObject.h"
#import "WLContentView.h"
#import "MainThreadExecutor.h"
#import "NSCustomWindow.h"

#include "private_defs.h"

/************** API begin ************/

OPENWL_API enum wl_Platform wl_GetPlatform()
{
    return wl_kPlatformMac;
}

OPENWL_API int CDECL wl_Init(wl_EventCallback callback, struct wl_PlatformOptions *options)
{
    initKeyMap();
    
    eventCallback = callback;
    
    bool slaveMode = (options && options->pluginSlaveMode);
    if (!slaveMode) {
        // when creating UIs for plugins (eg. AudioUnits),
        //   we can't do any of this stuff or it breaks the host program

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
    }
    
    // get monotonic clock for timers
    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &systemClock);
    
    printf("Hello from wl_Init\n");
    return 0;
}

OPENWL_API int CDECL wl_Runloop()
{
    [sharedApp run];
    return 0;
}

OPENWL_API void CDECL wl_ExitRunloop()
{
    [sharedApp stop:nil];
}

OPENWL_API void CDECL wl_Shutdown()
{
    [myDelegate release];
    [pool release];
    
    // dealloc monotonic clock for timers
    mach_port_deallocate(mach_task_self(), systemClock);
    
    printf("wl_Shutdown complete\n");
}


/*** WINDOW API ***/
static void setProps(NSWindow *window, wl_WindowProperties *props) {
    NSSize minSize = { FLT_MIN, FLT_MIN };
    NSSize maxSize = { FLT_MAX, FLT_MAX };
    if (props->usedFields & wl_kWindowPropMinWidth)
        minSize.width = props->minWidth;
    if (props->usedFields & wl_kWindowPropMinHeight)
        minSize.height = props->minHeight;
    if (props->usedFields & wl_kWindowPropMaxWidth)
        maxSize.width = props->maxWidth;
    if (props->usedFields & wl_kWindowPropMaxHeight)
        maxSize.height = props->maxHeight;
    [window setContentMinSize:minSize];
    [window setContentMaxSize:maxSize];
}

static wl_WindowRef _createNormalWindow(int width, int height, const char *title, void *userData, wl_WindowProperties *props)
{
    NSRect frame = NSMakeRect(300, 300, width, height);
    
    bool isBorderless = props && (props->usedFields & wl_kWindowPropStyle) && (props->style == wl_kWindowStyleFrameless);
    
#if defined(MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12
    NSWindowStyleMask styleMask = NSTitledWindowMask|NSClosableWindowMask|NSMiniaturizableWindowMask|NSResizableWindowMask;
    if (isBorderless) {
        styleMask = NSWindowStyleMaskBorderless;
    }
#else
    NSUInteger styleMask = NSTitledWindowMask|NSClosableWindowMask|NSMiniaturizableWindowMask|NSResizableWindowMask;
    if (isBorderless) {
        styleMask = NSBorderlessWindowMask;
    }
#endif
    // we use a custom NSWindow subclass to get mouse events on borderless windows (otherwise they can't be set to 'key' window)
    auto nsWindow = [[NSCustomWindow alloc] initWithContentRect:frame
                                                styleMask:styleMask
                                                  backing:NSBackingStoreBuffered
                                                    defer:NO];
    
    if (isBorderless) {
        // other misc useful things to make these work properly ...
        [nsWindow setLevel:NSFloatingWindowLevel];
        [nsWindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenAuxiliary]; // so it can also work in fullscreen mode
    }

    // WL handle data allocation
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
    
    return (wl_WindowRef)ret;
}

static wl_WindowRef _createNSViewForPlugin(int width, int height, void *userData, wl_WindowProperties *props)
{
    // create a dummy WLWindowObject with no actual NSWindow associated with it
    //   really just a holder for UserData
    auto windowObj = [[WLWindowObject alloc] init];
    windowObj.nsWindow = NULL;
    windowObj.userData = userData;
    windowObj.width = width;
    windowObj.height = height;
    
    // create content view (what we actually care about here)
    auto contentViewObj = [[WLContentView alloc] init];
    windowObj.contentViewObj = contentViewObj;
    contentViewObj.parentWindowObj = windowObj;
    
    auto mask = NSViewWidthSizable | NSViewHeightSizable | NSViewMinXMargin | NSViewMaxXMargin | NSViewMinYMargin | NSViewMaxYMargin;
    [contentViewObj setAutoresizingMask:mask];
    [contentViewObj setFrame:NSMakeRect(0, 0, width, height)];
    
    // normally we don't get resize events for views, so ....
    [[NSNotificationCenter defaultCenter]
     addObserver: windowObj
     selector: @selector(windowDidResize:)
     name: NSViewFrameDidChangeNotification
     object: contentViewObj];
    [contentViewObj setPostsFrameChangedNotifications:YES];
    
    // TODO: need to control lifetime of dummy window properly (if the view gets destroyed, it needs to go, too)
    props->outParams.nsView = contentViewObj;
    return (wl_WindowRef)windowObj;
}

OPENWL_API wl_WindowRef CDECL wl_WindowCreate(int width, int height, const char *title, void *userData, struct wl_WindowProperties *props)
{
    if (props && (props->usedFields & wl_kWindowPropStyle) && (props->style == wl_kWindowStylePluginWindow))
    {
        // special NSView creation w/ a dummy window
        // it will return the NSView in the properties : props->attachInfo.outView
        return _createNSViewForPlugin(width, height, userData, props);
    } else {
        // normal
        return _createNormalWindow(width, height, title, userData, props);
    }
}

OPENWL_API void CDECL wl_WindowDestroy(wl_WindowRef window)
{
    WLWindowObject *obj = (WLWindowObject *)window;
    [obj.nsWindow close];
    // use orderOut instead? to set next key window?
}

OPENWL_API void CDECL wl_WindowShow(wl_WindowRef window)
{
    WLWindowObject *obj = (WLWindowObject *)window;
    
    // synthesize a resize event here, so that -- like other platforms, I think? --
    //   we can rely on handling that event to determine our window size before drawing
    [[NSNotificationCenter defaultCenter] postNotificationName:NSWindowDidResizeNotification object:obj.nsWindow];

    [obj.nsWindow makeKeyAndOrderFront:NSApp];
//    [obj.nsWindow setIsVisible:YES];
}

OPENWL_API void CDECL wl_WindowShowRelative(wl_WindowRef window, wl_WindowRef relativeTo, int x, int y, int newWidth, int newHeight)
{
    WLWindowObject *obj = (WLWindowObject *)window;
    WLWindowObject *relObj = (WLWindowObject *)relativeTo;

    // apparently need to size *before* moving, else it doesn't work right
    if (newWidth > 0 && newHeight > 0) {
        [obj.nsWindow setContentSize:NSMakeSize(newWidth, newHeight)];
    }

    auto newOrigin = [relObj pointToScreen:NSMakePoint(x, y)];
    [obj.nsWindow setFrameTopLeftPoint:newOrigin];
    
    [obj.nsWindow makeKeyAndOrderFront:NSApp];
}

OPENWL_API void CDECL wl_WindowHide(wl_WindowRef window)
{
    WLWindowObject *obj = (WLWindowObject *)window;
    [obj.nsWindow setIsVisible:NO];
}

OPENWL_API void CDECL wl_WindowInvalidate(wl_WindowRef window, int x, int y, int width, int height)
{
    WLWindowObject *windowObj = (WLWindowObject *)window;
    if (width > 0 && height > 0) {
        [windowObj.contentViewObj setNeedsDisplayInRect:NSMakeRect(x, y, width, height)];
    } else {
        [windowObj.contentViewObj setNeedsDisplay:YES];
    }
}

OPENWL_API size_t CDECL wl_WindowGetOSHandle(wl_WindowRef window)
{
    WLWindowObject *windowObj = (WLWindowObject *)window;
    // you've already got it, buddy!
    return (size_t)windowObj;
}


OPENWL_API void CDECL wl_MouseGrab(wl_WindowRef window)
{
    // noop, seems to be the default behavior on OSX
}

OPENWL_API void CDECL wl_MouseUngrab()
{
    // noop
}

/* cursor api */
OPENWL_API wl_CursorRef CDECL wl_CursorCreate(wl_CursorStyle style)
{
    auto found = cursorMap.find(style);
    if (found != cursorMap.end()) {
        return found->second;
    } else {
        // create anew
        NSCursor *cursor;
        switch (style) {
            case wl_kCursorStyleDefault:
                cursor = [NSCursor arrowCursor];
                break;
            case wl_kCursorStyleResizeLeftRight:
                cursor = [NSCursor resizeLeftRightCursor];
                break;
            case wl_kCursorStyleResizeUpDown:
                cursor = [NSCursor resizeUpDownCursor];
                break;
            default:
                printf("wl_CursorCreate: unknown cursor type\n");
                return nullptr;
        }
        auto ret = new wl_Cursor;
        ret->nsCursor = [cursor retain];
        // save for later
        cursorMap[style] = ret;
        // bye
        return ret;
    }
}

OPENWL_API void CDECL wl_WindowSetCursor(wl_WindowRef window, wl_CursorRef cursor)
{
    WLWindowObject *windowObj = (WLWindowObject *)window;
    NSCursor *nsCursor = cursor ? cursor->nsCursor : Nil;
    [windowObj.contentViewObj setCursor:nsCursor];
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

OPENWL_API wl_IconRef CDECL wl_IconLoadFromFile(const char *filename, int sizeToWidth)
{
    auto fname = [NSString stringWithUTF8String:filename];
    auto bundle = [NSBundle mainBundle];
    auto path = [bundle pathForResource:fname ofType:nil];
    NSImage *image = [[NSImage alloc] initWithContentsOfFile:path];
    
    auto newSize = [image size];
    sizeToFit(&newSize, sizeToWidth, sizeToWidth);
    
    auto scaled = resizeImage(image, newSize);
    [image release];
    
    auto ret = new wl_Icon;
    ret->image = scaled;
    return ret;
}

OPENWL_API wl_AcceleratorRef CDECL wl_AccelCreate(enum wl_KeyEnum key, unsigned int modifiers)
{
    auto ret = new wl_Accelerator;
    ret->key = key;
    ret->modifiers = modifiers;
    return ret;
}

OPENWL_API wl_ActionRef CDECL wl_ActionCreate(int id, const char *label, wl_IconRef icon, wl_AcceleratorRef accel)
{
    auto ret = [[WLActionObject alloc] init];
    ret._id = id;
    ret.label = [[NSString stringWithUTF8String:label]
                  stringByReplacingOccurrencesOfString:@"&" withString:@""]; // since we don't use mnemonics
    ret.icon = icon;
    ret.accel = accel;
    
    return (wl_ActionRef)ret;
}

OPENWL_API wl_MenuRef CDECL wl_MenuCreate()
{
    auto ret = new wl_Menu;
    ret->menu = [[NSMenu alloc] init];
    return ret;
}

OPENWL_API wl_MenuItemRef CDECL wl_MenuAddAction(wl_MenuRef menu, wl_ActionRef action)
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
        ((actionObj.accel->modifiers & wl_kModifierControl) ? NSCommandKeyMask : 0) |
        ((actionObj.accel->modifiers & wl_kModifierAlt) ? NSAlternateKeyMask : 0) |
        ((actionObj.accel->modifiers & wl_kModifierShift) ? NSShiftKeyMask : 0);

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
    
    auto ret = new wl_MenuItem;
    ret->menuItem = item;
    return ret;
}

static wl_MenuItemRef addSubMenuCommon(NSMenu *parent, const char *label, NSMenu *child) {
    auto title = [[NSString stringWithUTF8String:label]
                  stringByReplacingOccurrencesOfString:@"&" withString:@""];
    auto item = [parent addItemWithTitle:title action:nil keyEquivalent:@""];

    [child setTitle:title]; // apparently necessary ...
    [item setSubmenu:child];
    
    auto ret = new wl_MenuItem;
    ret->menuItem = item;
    return ret;
}

OPENWL_API wl_MenuItemRef CDECL wl_MenuAddSubmenu(wl_MenuRef menu, const char *label, wl_MenuRef sub)
{
    return addSubMenuCommon(menu->menu, label, sub->menu);
}

OPENWL_API void CDECL wl_MenuAddSeparator(wl_MenuRef menu)
{
    [menu->menu addItem:[NSMenuItem separatorItem]];
}

OPENWL_API wl_MenuItemRef CDECL wl_MenuBarAddMenu(wl_MenuBarRef menuBar, const char *label, wl_MenuRef menu)
{
    return addSubMenuCommon(menuBar->menuBar, label, menu->menu);
}

OPENWL_API void CDECL wl_WindowShowContextMenu(wl_WindowRef window, int x, int y, wl_MenuRef menu, struct wl_Event *fromEvent)
{
    WLWindowObject *windowObj = (WLWindowObject *)window;
    [NSMenu popUpContextMenu:menu->menu
                   withEvent:fromEvent->_private->event
                     forView:windowObj.contentViewObj];
}

// Mac-specific menu stuff (sigh)
OPENWL_API wl_MenuBarRef CDECL wl_MenuBarGetDefault()
{
    auto ret = new wl_MenuBar;
    ret->menuBar = rootMenu;
    return ret;
}

OPENWL_API wl_MenuRef CDECL wl_GetApplicationMenu()
{
    if (appMenu) {
        auto ret = new wl_Menu;
        ret->menu = appMenu;
        return ret;
    } else {
        return nullptr;
    }
}

/********* timer API ************/

OPENWL_API wl_TimerRef CDECL wl_TimerCreate(unsigned int msTimeout, void *userData)
{
    double interval = msTimeout / 1000.0;
    auto ret = [[WLTimerObject alloc] init];
    ret.userData = userData;
    clock_get_time(systemClock, &ret->lastTime);
    
    ret.nsTimer = [NSTimer scheduledTimerWithTimeInterval:interval target:ret selector:@selector(timerFired:) userInfo:nil repeats:YES];
    return (wl_TimerRef)ret;
}

OPENWL_API void CDECL wl_TimerDestroy(wl_TimerRef timer)
{
    WLTimerObject *timerObj = (WLTimerObject *)timer;
    [timerObj stopTimer];
    [timerObj release];
}

/************ DND API ***************/

OPENWL_API const char *wl_kDragFormatUTF8 = "text/plain;charset=UTF-8";
OPENWL_API const char *wl_kDragFormatFiles = "text/uri-list";

OPENWL_API void wl_WindowEnableDrops(wl_WindowRef window, bool enabled)
{
    WLWindowObject *windowObj = (WLWindowObject *)window;
    if (enabled) {
        [windowObj.contentViewObj enableDrops];
    } else {
        [windowObj.contentViewObj disableDrops];
    }
}

OPENWL_API bool CDECL wl_DropHasFormat(wl_DropDataRef dropData, const char *dropFormatMIME)
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

OPENWL_API bool CDECL wl_DropGetFormat(wl_DropDataRef dropData, const char *dropFormatMIME, const void **outData, size_t *outSize)
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

OPENWL_API bool CDECL wl_DropGetFiles(wl_DropDataRef dropData, const struct wl_Files **outFiles)
{
    auto types = [dropData->pboard types];
    if ([types containsObject:NSFilenamesPboardType]) {
        NSArray *pboard_files = [dropData->pboard propertyListForType:NSFilenamesPboardType];
        
        auto numFiles = (int)[pboard_files count];
        dropData->files = new wl_FilesInternal(numFiles);
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

OPENWL_API wl_DragDataRef CDECL wl_DragDataCreate(wl_WindowRef forWindow)
{
    auto ret = new wl_DragData;
    ret->forWindow = forWindow;
    ret->formats.clear();
    return ret;
}

OPENWL_API void CDECL wl_DragDataRelease(wl_DragDataRef *dragData)
{
    delete *dragData;
    *dragData = nullptr;
}

OPENWL_API void CDECL wl_DragAddFormat(wl_DragDataRef dragData, const char *dragFormatMIME)
{
    dragData->formats.insert(dragFormatMIME);
}

OPENWL_API enum wl_DropEffect CDECL wl_DragExec(wl_DragDataRef dragData, unsigned int dropActionsMask, struct wl_Event *fromEvent)
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
    
    // old nested runloop no longer works, but this seems to (in 10.12 anyway)
    [[NSRunLoop currentRunLoop] runMode:NSEventTrackingRunLoopMode beforeDate:[NSDate distantFuture]];
    
//    // created a nested runloop, which will exit when the drag operation completes
//    // we're just matching the behavior of other platforms, where a DragExec is blocking
//    printf("going into subloop...\n");
//    auto subloop = [[NSRunLoop alloc] init];
//    [subloop run];
//    printf("exited subloop ..\n");
    
    printf("returning from dragexec!\n");
    return contentView.dragResult;
}

// rendering convenience
OPENWL_API void CDECL wl_DragRenderUTF8(wl_RenderPayloadRef payload, const char *text)
{
    payload->data = [NSString stringWithUTF8String:text];
}

OPENWL_API void CDECL wl_DragRenderFiles(wl_RenderPayloadRef payload, const struct wl_Files *files)
{
    // nothing yet
}

/********* clipboard API ****************/

OPENWL_API wl_DropDataRef CDECL wl_ClipboardGet()
{
    auto ret = new wl_DropData;
    ret->pboard = [[NSPasteboard pasteboardWithName:NSGeneralPboard] retain];
    return ret;
}

OPENWL_API void CDECL wl_ClipboardSet(wl_DragDataRef dragData)
{
    auto pboard = [NSPasteboard pasteboardWithName:NSGeneralPboard];
    [pboard clearContents];
    
    // wrap all available formats (and the possibility of rendering them)
    // rendering will be deferred until actual paste :D
    WLContentView *contentView = ((WLWindowObject *)dragData->forWindow).contentViewObj;
    contentView.sourceData = dragData;
    [pboard writeObjects:[NSArray arrayWithObject:contentView]];
}

OPENWL_API void CDECL wl_ClipboardRelease(wl_DropDataRef dropData)
{
    [dropData->pboard release];
    delete dropData;
}

OPENWL_API void CDECL wl_ClipboardFlush()
{
    printf("clipboard flushing not yet implemented on OSX\n");
}


/***** Misc Stuff ******/

OPENWL_API void CDECL wl_ExecuteOnMainThread(wl_VoidCallback callback, void *data)
{
    auto mte = [MainThreadExecutor withCallback:callback param:data];
    [mte performSelectorOnMainThread:@selector(executeCallback) withObject:nil waitUntilDone:YES];
}

OPENWL_API void CDECL wl_Sleep(unsigned int millis)
{
    auto interval = ((double)millis) / 1000.0;
    [NSThread sleepForTimeInterval:interval];
}

OPENWL_API size_t CDECL wl_SystemMillis()
{
    mach_timespec_t currentTime;
    clock_get_time(systemClock, &currentTime);
    
    unsigned long millis = currentTime.tv_sec * 1000;
    millis += currentTime.tv_nsec / 1000000;
    return millis;
}

