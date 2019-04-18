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

struct _wlEventPrivate {
    NSEvent *event;
};

struct _wlAccelerator {
    enum wl_KeyEnum key;
    unsigned int modifiers;
};

struct _wlIcon {
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

struct _wlMenuBar {
    NSMenu *menuBar;
};

struct _wlMenu {
    NSMenu *menu;
};

struct _wlMenuItem {
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
struct _wlDragData {
    wl_Window forWindow;
    std::set<std::string> formats;
};

// drop data: for drag dest / clipboard paste
struct _wlFilesInternal : public wl_Files
{
    _wlFilesInternal(int numFiles) {
        this->numFiles = numFiles;
        filenames = new const char *[numFiles];
    }
    ~_wlFilesInternal() {
        for (int i=0; i< numFiles; i++) {
            free(const_cast<char *>(filenames[i])); // created w/ strdup
        }
        delete filenames;
    }
};

struct _wlDropData {
    NSPasteboard *pboard;
    
    void *data = nullptr;
    size_t dataSize = 0;
    _wlFilesInternal *files = nullptr;
    
    ~_wlDropData() {
        if (data) free(data);
        if (files) delete files;
    }
};

struct _wlRenderPayload {
    id data; // NSData, NSString, etc etc ... PList types?
};

#endif /* defined(__openwl__private_defs__) */
