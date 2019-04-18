//
//  private_defs.h
//  openwl
//
//  Created by Daniel X on 2/12/18.
//  Copyright (c) 2018 OpenWL Developers. All rights reserved.
//

#ifndef __openwl__private_defs__
#define __openwl__private_defs__

#include "../openwl.h"
#import <AppKit/AppKit.h>

@class WLWindowObject;

#include <set>
#include <string>

struct wl_EventPrivateImpl {
    NSEvent *event;
};

struct wl_AcceleratorImpl {
    enum wl_KeyEnum key;
    unsigned int modifiers;
};

struct wl_IconImpl {
    NSImage *image;
};

// wl_Action
@interface WLActionObject : NSObject
@property int _id;
@property (copy) NSString *label;
@property wl_Icon icon;
@property wl_Accelerator accel;
// maintain list of NSMenuItems that it's connected to, so that it can be updated?
@end

struct wl_MenuBarImpl {
    NSMenu *menuBar;
};

struct wl_MenuImpl {
    NSMenu *menu;
};

struct wl_MenuItemImpl {
    NSMenuItem *menuItem;
};

// wl_Timer
// can't seem to properly subclass NSTimer (factory methods don't return (id), but instead (NSTimer), etc)
// http://www.cocoabuilder.com/archive/cocoa/174162-creating-an-nstimer-subclass.html
// so just inherit from NSObject, and pass this as userInfo * to an NSTimer
@interface WLTimerObject : NSObject {
    @public mach_timespec_t lastTime;
}
@property (retain) NSTimer *timer;
@property int _id;
@property (assign) WLWindowObject *forWindow;
- (void)timerFired:(id)sender;
- (void)stopTimer;
@end

// drag data: drag source / clipboard source
struct wl_DragDataImpl {
    wl_Window forWindow;
    std::set<std::string> formats;
};

// drop data: for drag dest / clipboard paste
struct _wl_FilesInternal : public wl_Files
{
    _wl_FilesInternal(int numFiles) {
        this->numFiles = numFiles;
        filenames = new const char *[numFiles];
    }
    ~_wl_FilesInternal() {
        for (int i=0; i< numFiles; i++) {
            free(const_cast<char *>(filenames[i])); // created w/ strdup
        }
        delete filenames;
    }
};

struct wl_DropDataImpl {
    NSPasteboard *pboard;
    
    void *data = nullptr;
    size_t dataSize = 0;
    _wl_FilesInternal *files = nullptr;
    
    ~wl_DropDataImpl() {
        if (data) free(data);
        if (files) delete files;
    }
};

struct wl_RenderPayloadImpl {
    id data; // NSData, NSString, etc etc ... PList types?
};

#endif /* defined(__openwl__private_defs__) */
