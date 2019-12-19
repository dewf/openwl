#pragma once

#include <map>
#include "../openwl.h"

struct KeyInfo {
	int virtualCode;
	wl_KeyEnum key;
	const char *stringRep;
	bool suppressCharEvent;
	int knownLocation = wl_kKeyLocationDefault;
	KeyInfo(int virtualCode, wl_KeyEnum key, const char *stringRep, bool suppressCharEvent, int knownLocation = wl_kKeyLocationDefault)
	    :virtualCode(virtualCode),
	    key(key),
	    stringRep(stringRep),
	    suppressCharEvent(suppressCharEvent),
	    knownLocation(knownLocation)
	{}
};

extern std::map<WPARAM, KeyInfo *> keyMap; // win32 virtual keys to WL key enums
extern std::map<wl_KeyEnum, KeyInfo *> reverseKeyMap; // reverse

wl_KeyLocation locationForKey(wl_KeyEnum key, unsigned char scanCode, bool extended);

void keystuff_init();
