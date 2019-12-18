#include "action.h"

#include "private_defs.h"

#include "keystuff.h"
#include "window.h"

#include <assert.h>
#include "unicodestuff.h"

static std::map<int, wl_ActionRef> actionMap;
static std::vector<ACCEL> acceleratorList;

// fwd decls
std::string joinParts(std::vector<std::string> parts, const char* delimiter);
std::string accelToString(wl_AcceleratorRef accel);

// methods

wl_ActionRef wl_Action::create(int id, const char* label, wl_IconRef icon, wl_AcceleratorRef accel)
{
	wl_ActionRef retAction = new wl_Action;
	retAction->label = label;
	printf("retAction label: %s (original %s)\n", retAction->label.c_str(), label);
	retAction->id = id; // nextActionID++;
	retAction->icon = icon;
	retAction->attachedItems.clear();
	if (accel) {
		retAction->accel = accel;
		ACCEL acc;
		acc.fVirt = FVIRTKEY |
			((accel->modifiers & wl_kModifierShift) ? FSHIFT : 0) |
			((accel->modifiers & wl_kModifierControl) ? FCONTROL : 0) |
			((accel->modifiers & wl_kModifierAlt) ? FALT : 0);
		acc.key = reverseKeyMap[accel->key]->virtualCode; // key enum -> win32 virtual code
		acc.cmd = retAction->id;
		acceleratorList.push_back(acc);
		printf("fVirt is: %d | key is: %c\n", acc.fVirt, acc.key);
	}
	// add to map to fetch during WndProc (since not so easy to add menu item data)
	actionMap[retAction->id] = retAction;

	return retAction;
}

HACCEL wl_Action::createAccelTable()
{
	return CreateAcceleratorTable((ACCEL*)acceleratorList.data(), (int)acceleratorList.size());
}

void wl_Action::onActionID(wl_Event& event, int id, wl_WindowRef window)
{
	auto found = actionMap.find(id);
	if (found != actionMap.end()) {
		auto action = found->second;
		event.eventType = wl_kEventTypeAction;
		event.actionEvent.action = action; // aka value
		event.actionEvent.id = action->id;
		window->sendEvent(event);
	}
}

wl_MenuItemRef wl_Action::addToMenu(wl_MenuRef menu)
{
	wl_MenuItemRef retItem = new wl_MenuItem;
	memset(retItem, 0, sizeof(wl_MenuItem));

	// alter label if accelerator present
	std::string label = this->label;
	printf("wl_MenuAddAction label: %s\n", label.c_str());
	if (accel) {
		label += "\t";
		label += accelToString(accel);
		assert(label.compare(this->label) != 0); // err right?
	}

	auto wideLabel = utf8_to_wstring(label);
	AppendMenu(menu->hmenu, MF_STRING, id, wideLabel.c_str());
	if (icon) {
		// set the bitmap
		MENUITEMINFO info;
		info.cbSize = sizeof(MENUITEMINFO);
		info.fMask = MIIM_BITMAP;
		info.hbmpItem = icon->hbitmap;
		SetMenuItemInfo(menu->hmenu, id, FALSE, &info);
	}
	retItem->action = this;
	attachedItems.push_back(retItem);
	return retItem;
}

// util funcs

std::string joinParts(std::vector<std::string> parts, const char* delimiter) {
	std::string res;

	size_t numParts = parts.size();
	size_t i = 0;
	for (auto p = parts.begin(); p != parts.end(); p++, i++) {
		res.append(*p);
		if (i < numParts - 1) { // final element doesn't receive delimiter
			res.append(delimiter);
		}
	}

	return res;
}

std::string accelToString(wl_AcceleratorRef accel) {
	std::vector<std::string> parts;
	if (accel->modifiers & wl_kModifierControl)
		parts.push_back("Ctrl");
	if (accel->modifiers & wl_kModifierAlt)
		parts.push_back("Alt");
	if (accel->modifiers & wl_kModifierShift)
		parts.push_back("Shift");
	//
	auto stringRep = reverseKeyMap[accel->key]->stringRep;

	//parts.push_back(upperCased(stringRep)); // for the time being, no need to do this - we're only allowing known A-Za-z0-9 keys as accelerators
											  // might have something to do with the "lookup at runtime" thing for getting the string representation of a key? been a long time

	parts.push_back(stringRep);
	//
	return joinParts(parts, "+");
}
