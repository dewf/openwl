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

struct _wl_EventPrivate {
    NSEvent *event;
};

struct _wl_Accelerator {
    enum wl_KeyEnum key;
    unsigned int modifiers;
};

struct _wl_Icon {
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

struct _wl_MenuBar {
    NSMenu *menuBar;
};

struct _wl_Menu {
    NSMenu *menu;
};

struct _wl_MenuItem {
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
struct _wl_DragData {
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

struct _wl_DropData {
    NSPasteboard *pboard;
    
    void *data = nullptr;
    size_t dataSize = 0;
    _wl_FilesInternal *files = nullptr;
    
    ~_wl_DropData() {
        if (data) free(data);
        if (files) delete files;
    }
};

struct _wl_RenderPayload {
    id data; // NSData, NSString, etc etc ... PList types?
};

#endif /* defined(__openwl__private_defs__) */
