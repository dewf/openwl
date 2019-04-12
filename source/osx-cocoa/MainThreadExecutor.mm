//
//  MainThreadExecutor.m
//  openwl
//
//  Created by Daniel X on 2/12/18.
//  Copyright (c) 2018 OpenWL Developers. All rights reserved.
//

#import "MainThreadExecutor.h"

@implementation MainThreadExecutor
+ (MainThreadExecutor *) withCallback:(wlVoidCallback)cb param:(void *)p {
    auto ret = [[MainThreadExecutor alloc] autorelease];
    ret->callback = cb;
    ret->param = p;
    return ret;
}
- (void) executeCallback {
    self->callback(self->param);
}
@end
