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
    wl_KeyEnum key;
    const char *stringRep;
    bool suppressCharEvent;
    wl_KeyLocation location;
};

extern keymapInfo fixedEntries[];
extern keymapInfo variableEntries[];
extern std::map<int, keymapInfo *> codeToKeyInfo; // fixed non-alphanumeric keys
extern std::map<unichar, keymapInfo *> unicharToKeyInfo; // for non-fixed keys (foreign layouts etc)
extern std::map<wl_KeyEnum, keymapInfo *> reverseKeymap;

void initKeyMap();

#endif /* defined(__openwl__keystuff__) */
