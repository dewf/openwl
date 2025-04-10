#include "menustuff.h"

#include "keystuff.h"
#include "window.h"

#include <assert.h>
#include "unicodestuff.h"

static std::map<int, wl_ActionRef> actionMap;

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

std::string accelToString(wl_AcceleratorRef accel)
{
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

// methods

wl_ActionRef wl_Action::create(int id, const char* label, wl_IconRef icon, wl_AcceleratorRef accel)
{
	wl_ActionRef retAction = new wl_Action;
	retAction->label = label;
	printf("retAction label: %s (original %s)\n", retAction->label.c_str(), label);
	retAction->id = id; // nextActionID++;
	retAction->icon = icon;
	//retAction->attachedItems.clear();
	retAction->accel = accel;

	actionMap[id] = retAction;

	return retAction;
}

wl_ActionRef wl_Action::findByID(int id)
{
	auto found = actionMap.find(id);
	if (found != actionMap.end()) {
		return found->second;
	}
	else {
		return nullptr;
	}
}

wl_MenuItemRef wl_Menu::addSubMenu(std::string label, wl_MenuRef subMenu)
{
	auto wideLabel = utf8_to_wstring(label);

	auto retItem = new wl_MenuItem;
	//memset(retItem, 0, sizeof(wl_MenuItem));
	AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)subMenu->hmenu, wideLabel.c_str());
	retItem->subMenu = subMenu;

	// so we can iterate later
	items.push_back(retItem);

	return retItem;
}

wl_MenuItemRef wl_Menu::addAction(wl_ActionRef action)
{
	wl_MenuItemRef retItem = new wl_MenuItem();
	////memset(retItem, 0, sizeof(wl_MenuItem));

	// make sure we can iterate later!
	items.push_back(retItem);

	// alter label if accelerator present
	std::string label = action->label;
	printf("wl_MenuAddAction label: %s\n", label.c_str());
	if (action->accel) {
		label += "\t";
		label += accelToString(action->accel);
		assert(label.compare(action->label) != 0); // err right?
	}

	auto wideLabel = utf8_to_wstring(label);
	AppendMenu(hmenu, MF_STRING, action->id, wideLabel.c_str());
	if (action->icon) {
		// set the bitmap
		MENUITEMINFO info;
		info.cbSize = sizeof(MENUITEMINFO);
		info.fMask = MIIM_BITMAP;
		info.hbmpItem = action->icon->hbitmap;
		SetMenuItemInfo(hmenu, action->id, FALSE, &info);
	}
	retItem->action = action;
	//attachedItems.push_back(retItem);
	return retItem;
}

void wl_MenuBar::addMenu(std::string label, wl_MenuRef menu)
{
	auto wideLabel = utf8_to_wstring(label);
	AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)menu->hmenu, wideLabel.c_str());
	menus.push_back(menu);
}

static void addMenuAccels(wl_MenuRef menu, std::vector<ACCEL>& accum)
{
	for (auto i = menu->items.begin(); i != menu->items.end(); i++) {
		auto item = *i;
		if (item->subMenu != nullptr) {
			addMenuAccels(item->subMenu, accum);
		}
		else if (item->action != nullptr) {
			if (item->action->accel) {
				auto accel = item->action->accel;
				ACCEL acc;
				acc.fVirt = FVIRTKEY |
					((accel->modifiers & wl_kModifierShift) ? FSHIFT : 0) |
					((accel->modifiers & wl_kModifierControl) ? FCONTROL : 0) |
					((accel->modifiers & wl_kModifierAlt) ? FALT : 0);
				acc.key = reverseKeyMap[accel->key]->virtualCode; // key enum -> win32 virtual code
				acc.cmd = item->action->id;
				printf("fVirt is: %d | key is: %c\n", acc.fVirt, acc.key);
				accum.push_back(acc);
			}
		}
		else {
			// uhhh
			printf("addMenuAccels: neither subMenu nor action were filled??\n");
		}
	}
}

HACCEL wl_MenuBar::generateAcceleratorTable()
{
	std::vector<ACCEL> accum;
	for (auto i = menus.begin(); i != menus.end(); i++) {
		auto menu = *i;
		addMenuAccels(menu, accum);
	}
	//printf("accumulated %d ACCELs\n", (int)accum.size());
	return CreateAcceleratorTable((ACCEL*)accum.data(), (int)accum.size());
}

wl_MenuBarRef wl_MenuBar::create()
{
	wl_MenuBarRef retMenuBar = new wl_MenuBar;
	retMenuBar->hmenu = CreateMenu();
	return retMenuBar;
}
