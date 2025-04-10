//
// Created by dang on 2/11/18.
//

#ifndef C_CLIENT_KEYSTUFF_H
#define C_CLIENT_KEYSTUFF_H

#include "../openwl.h"
#include <map>
#include <glib.h>

#define XK_MISCELLANY
#define XK_LATIN1
#include "X11/keysymdef.h"
#include "X11/XF86keysym.h"

struct keyInfo {
    guint lowerSym;
    guint upperSym;
    wl_KeyEnum key;
    const char *stringRep;
    bool suppressChar; // suppress generation of "char" keydown events (for things like Escape, Tab, etc)
    wl_KeyLocation location;
};

extern std::map<guint, keyInfo *> keyMap;
extern std::map<wl_KeyEnum, keyInfo *> reverseKeyMap;
extern keyInfo keyData[];

void initKeymap();

#endif //C_CLIENT_KEYSTUFF_H
