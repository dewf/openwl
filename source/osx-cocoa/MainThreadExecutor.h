//
//  MainThreadExecutor.h
//  openwl
//
//  Created by Daniel X on 2/12/18.
//  Copyright (c) 2018 OpenWL Developers. All rights reserved.
//

#import <Foundation/Foundation.h>

#include "../openwl.h"

// on-main-thread executor convenience object
@interface MainThreadExecutor : NSObject {
    wlVoidCallback callback;
    void *param;
}
+ (MainThreadExecutor *) withCallback:(wlVoidCallback)cb param:(void *)p;
- (void) executeCallback;
@end

