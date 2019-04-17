#pragma once

#include <map>
#include "../openwl.h"

struct KeyInfo {
	int virtualCode;
	WLKeyEnum key;
	const char *stringRep;
	bool suppressCharEvent;
	int knownLocation = WLKeyLocation_Default;
	KeyInfo(int virtualCode, WLKeyEnum key, const char *stringRep, bool suppressCharEvent, int knownLocation = WLKeyLocation_Default)
	    :virtualCode(virtualCode),
	    key(key),
	    stringRep(stringRep),
	    suppressCharEvent(suppressCharEvent),
	    knownLocation(knownLocation)
	{}
};

extern std::map<WPARAM, KeyInfo *> keyMap; // win32 virtual keys to WL key enums
extern std::map<WLKeyEnum, KeyInfo *> reverseKeyMap; // reverse

void initKeyMap();
WLKeyLocation locationForKey(WLKeyEnum key, unsigned char scanCode, bool extended);
