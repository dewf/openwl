//
//  private_defs.cpp
//  openwl
//
//  Created by Daniel X on 2/12/18.
//  Copyright (c) 2018 OpenWL Developers. All rights reserved.
//

#include "private_defs.h"
#include "globals.h"

#import "WLWindowObject.h"


std::map<wl_CursorStyle, wl_CursorRef> cursorMap;



// wl_ActionRef
@implementation WLActionObject
@synthesize _id;
@synthesize label;
@synthesize icon;
@synthesize accel;
@end


// wl_TimerRef
#define ONE_BILLION 1000000000
inline double timespecDiff(mach_timespec_t newer, mach_timespec_t older) {
    if (newer.tv_nsec < older.tv_nsec) {
        newer.tv_nsec += ONE_BILLION;
        newer.tv_sec -= 1;
    }
    auto diff = (newer.tv_sec - older.tv_sec) * ONE_BILLION + (newer.tv_nsec - older.tv_nsec);
    return (double) diff / ONE_BILLION;
}

@implementation WLTimerObject
@synthesize timer;
@synthesize forWindow;
@synthesize _id;
- (void)timerFired:(id)sender {
    wl_Event event;
    event.handled = false;
    event.eventType = wl_kEventTypeTimer;
    event.timerEvent.timerID = _id;
    event.timerEvent.timer = (wl_TimerRef)self;
    event.timerEvent.stopTimer = false;
    
    mach_timespec_t now;
    clock_get_time(systemClock, &now);
    event.timerEvent.secondsSinceLast = timespecDiff(now, self->lastTime);
    
    eventCallback((wl_WindowRef)forWindow, &event, forWindow.userData);
    
    self->lastTime = now;
    
    if (event.handled && event.timerEvent.stopTimer) {
        [self stopTimer];
    }
}
- (void)stopTimer {
    printf("stopping timer %d\n", _id);
    [timer invalidate];
}
@end

