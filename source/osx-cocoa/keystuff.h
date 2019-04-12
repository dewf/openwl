//
//  keystuff.h
//  openwl
//
//  Created by Daniel X on 2/12/18.
//  Copyright (c) 2018 OpenWL Developers. All rights reserved.
//

#ifndef __openwl__keystuff__
#define __openwl__keystuff__

#include <stdio.h>

#include "../openwl.h"
#include <map>
#import <Foundation/Foundation.h>
#import <Carbon/Carbon.h>

struct keymapInfo {
    int code;           // apple keycode
    WLKeyEnum key;
    const char *stringRep;
    bool suppressCharEvent;
    WLKeyLocation location;
};

extern keymapInfo fixedEntries[];
extern keymapInfo variableEntries[];
extern std::map<int, keymapInfo *> codeToKeyInfo; // fixed non-alphanumeric keys
extern std::map<unichar, keymapInfo *> unicharToKeyInfo; // for non-fixed keys (foreign layouts etc)
extern std::map<WLKeyEnum, keymapInfo *> reverseKeymap;

void initKeyMap();

#endif /* defined(__openwl__keystuff__) */
