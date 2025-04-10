#include <Windows.h>

#include "keystuff.h"

#include "unicodestuff.h"

std::map<WPARAM, KeyInfo *> keyMap; // win32 virtual keys to WL key enums
std::map<wl_KeyEnum, KeyInfo *> reverseKeyMap; // reverse

#define CALCULATE_LOCATION -1
#define LOOKUP_AT_RUNTIME "##lookup at runtime##"

KeyInfo infoItems[] = {
	{ 0, wl_kKeyUnknown, "(Unknown)", false },
	//
	{ VK_ESCAPE, wl_kKeyEscape, "Escape", true },
	{ VK_BACK, wl_kKeyBackspace, "Backspace", true },
	{ VK_TAB, wl_kKeyTab, "Tab", true },
	{ VK_CAPITAL, wl_kKeyCapsLock, "Caps Lock", false },
	{ VK_RETURN, wl_kKeyReturn, "Return", true, CALCULATE_LOCATION },
	{ VK_SPACE, wl_kKeySpace, "Space", false },
	//
	{ VK_CONTROL, wl_kKeyControl, "Control", false, CALCULATE_LOCATION },
	{ VK_SHIFT, wl_kKeyShift, "Shift", false, CALCULATE_LOCATION },
	{ VK_MENU, wl_kKeyAltOption, "Alt/Option", false, CALCULATE_LOCATION },
	{ VK_LWIN, wl_kKeyWinCommand, "Left Windows", false, wl_kKeyLocationLeft },
	{ VK_RWIN, wl_kKeyWinCommand, "Right Windows", false, wl_kKeyLocationRight },
	//
	{ VK_F1, wl_kKeyF1, "F1", false },
	{ VK_F2, wl_kKeyF2, "F2", false },
	{ VK_F3, wl_kKeyF3, "F3", false },
	{ VK_F4, wl_kKeyF4, "F4", false },
	{ VK_F5, wl_kKeyF5, "F5", false },
	{ VK_F6, wl_kKeyF6, "F6", false },
	{ VK_F7, wl_kKeyF7, "F7", false },
	{ VK_F8, wl_kKeyF8, "F8", false },
	{ VK_F9, wl_kKeyF9, "F9", false },
	{ VK_F10, wl_kKeyF10, "F10", false },
	{ VK_F11, wl_kKeyF11, "F11", false },
	{ VK_F12, wl_kKeyF12, "F12", false },
	{ VK_F13, wl_kKeyF13, "F13", false },
	{ VK_F14, wl_kKeyF14, "F14", false },
	{ VK_F15, wl_kKeyF15, "F15", false },
	{ VK_F16, wl_kKeyF16, "F16", false },
	{ VK_F17, wl_kKeyF17, "F17", false },
	{ VK_F18, wl_kKeyF18, "F18", false },
	{ VK_F19, wl_kKeyF19, "F19", false },
	//
	{ '0', wl_kKey0, "0", false },
	{ '1', wl_kKey1, "1", false },
	{ '2', wl_kKey2, "2", false },
	{ '3', wl_kKey3, "3", false },
	{ '4', wl_kKey4, "4", false },
	{ '5', wl_kKey5, "5", false },
	{ '6', wl_kKey6, "6", false },
	{ '7', wl_kKey7, "7", false },
	{ '8', wl_kKey8, "8", false },
	{ '9', wl_kKey9, "9", false },
	//
	{ 'A', wl_kKeyA, "A", false },
	{ 'B', wl_kKeyB, "B", false },
	{ 'C', wl_kKeyC, "C", false },
	{ 'D', wl_kKeyD, "D", false },
	{ 'E', wl_kKeyE, "E", false },
	{ 'F', wl_kKeyF, "F", false },
	{ 'G', wl_kKeyG, "G", false },
	{ 'H', wl_kKeyH, "H", false },
	{ 'I', wl_kKeyI, "I", false },
	{ 'J', wl_kKeyJ, "J", false },
	{ 'K', wl_kKeyK, "K", false },
	{ 'L', wl_kKeyL, "L", false },
	{ 'M', wl_kKeyM, "M", false },
	{ 'N', wl_kKeyN, "N", false },
	{ 'O', wl_kKeyO, "O", false },
	{ 'P', wl_kKeyP, "P", false },
	{ 'Q', wl_kKeyQ, "Q", false },
	{ 'R', wl_kKeyR, "R", false },
	{ 'S', wl_kKeyS, "S", false },
	{ 'T', wl_kKeyT, "T", false },
	{ 'U', wl_kKeyU, "U", false },
	{ 'V', wl_kKeyV, "V", false },
	{ 'W', wl_kKeyW, "W", false },
	{ 'X', wl_kKeyX, "X", false },
	{ 'Y', wl_kKeyY, "Y", false },
	{ 'Z', wl_kKeyZ, "Z", false },
	// keypad
	{ VK_NUMPAD0, wl_kKeyKP0, "Numpad 0", false, wl_kKeyLocationNumPad },
	{ VK_NUMPAD1, wl_kKeyKP1, "Numpad 1", false, wl_kKeyLocationNumPad },
	{ VK_NUMPAD2, wl_kKeyKP2, "Numpad 2", false, wl_kKeyLocationNumPad },
	{ VK_NUMPAD3, wl_kKeyKP3, "Numpad 3", false, wl_kKeyLocationNumPad },
	{ VK_NUMPAD4, wl_kKeyKP4, "Numpad 4", false, wl_kKeyLocationNumPad },
	{ VK_NUMPAD5, wl_kKeyKP5, "Numpad 5", false, wl_kKeyLocationNumPad },
	{ VK_NUMPAD6, wl_kKeyKP6, "Numpad 6", false, wl_kKeyLocationNumPad },
	{ VK_NUMPAD7, wl_kKeyKP7, "Numpad 7", false, wl_kKeyLocationNumPad },
	{ VK_NUMPAD8, wl_kKeyKP8, "Numpad 8", false, wl_kKeyLocationNumPad },
	{ VK_NUMPAD9, wl_kKeyKP9, "Numpad 9", false, wl_kKeyLocationNumPad },
	//
	{ VK_NUMLOCK, wl_kKeyNumLock, "NumLock", false, wl_kKeyLocationNumPad },
	{ VK_CLEAR, wl_kKeyKPClear, "KP-Clear", false, wl_kKeyLocationNumPad },
	{ VK_DIVIDE, wl_kKeyKPDivide, "KP-Divide", false, wl_kKeyLocationNumPad },
	{ VK_MULTIPLY, wl_kKeyKPMultiply, "KP-Multiply", false, wl_kKeyLocationNumPad },
	{ VK_SUBTRACT, wl_kKeyKPSubtract, "KP-Subtract", false, wl_kKeyLocationNumPad },
	{ VK_ADD, wl_kKeyKPAdd, "KP-Add", false, wl_kKeyLocationNumPad },
	//
	{ VK_INSERT, wl_kKeyInsert, "Insert", false, CALCULATE_LOCATION },
	{ VK_DELETE, wl_kKeyDelete, "Delete", false, CALCULATE_LOCATION },
	{ VK_HOME, wl_kKeyHome, "Home", false, CALCULATE_LOCATION },
	{ VK_END, wl_kKeyEnd, "End", false, CALCULATE_LOCATION },
	{ VK_PRIOR, wl_kKeyPageUp, "PageUp", false, CALCULATE_LOCATION },
	{ VK_NEXT, wl_kKeyPageDown, "PageDown", false, CALCULATE_LOCATION },
	//
	{ VK_UP, wl_kKeyUpArrow, "UpArrow", false, CALCULATE_LOCATION },
	{ VK_LEFT, wl_kKeyLeftArrow, "LeftArrow", false, CALCULATE_LOCATION },
	{ VK_RIGHT, wl_kKeyRightArrow, "RightArrow", false, CALCULATE_LOCATION },
	{ VK_DOWN, wl_kKeyDownArrow, "DownArrow", false, CALCULATE_LOCATION },
	//
	{ VK_SNAPSHOT, wl_kKeyPrintScreen, "PrintScrn", false },
	{ VK_SCROLL, wl_kKeyScrollLock, "ScrollLock", false },
	{ VK_PAUSE, wl_kKeyPause, "Pause", false },
	{ VK_CANCEL, wl_kKeyCancel, "Cancel (Ctrl-Break)", true },
	//
	{ VK_VOLUME_MUTE, wl_kKeyMediaMute, "Media Mute", false },
	{ VK_VOLUME_DOWN, wl_kKeyMediaVolumeDown, "Media Volume Down", false },
	{ VK_VOLUME_UP, wl_kKeyMediaVolumeUp, "Media Volume Up", false },
	{ VK_MEDIA_NEXT_TRACK, wl_kKeyMediaNext, "Media Next", false },
	{ VK_MEDIA_PREV_TRACK, wl_kKeyMediaPrev, "Media Prev", false },
	{ VK_MEDIA_STOP, wl_kKeyMediaStop, "Media Stop", false },
	{ VK_MEDIA_PLAY_PAUSE, wl_kKeyMediaPlayPause, "Media Play/Pause", false },
};

void initKeyMap() {
	int count = sizeof(infoItems) / sizeof(KeyInfo);
	for (int i = 0; i < count; i++) {
		KeyInfo *info = &infoItems[i];

		keyMap[info->virtualCode] = info;
		reverseKeyMap[info->key] = info;
		
		// generate labels for layout-specific keys
		if (info->stringRep == LOOKUP_AT_RUNTIME) {
			auto res = MapVirtualKey(info->virtualCode, MAPVK_VK_TO_CHAR);
			if (res & 0x80000000) {
				printf("OEM vk %d is dead key\n", info->virtualCode);
				res &= 0xFFFF; // lower 16 only
			}
			wchar_t buffer[2] = { (wchar_t)res, 0 };
			std::wstring wide(buffer);
			auto utf8 = wstring_to_utf8(wide);

			info->stringRep = _strdup(utf8.c_str());
		}
	}
}

wl_KeyLocation locationForKey(wl_KeyEnum key, unsigned char scanCode, bool extended)
{
	auto loc = wl_kKeyLocationDefault;
	switch (key) {
	case wl_kKeyShift:
		//printf("locationForKey WLKeyShift: %d %d %d\n", key, scanCode, extended);
		loc = (scanCode == 54) ? wl_kKeyLocationRight : wl_kKeyLocationLeft;
		break;
	case wl_kKeyControl:
	case wl_kKeyAltOption:
		loc = extended ? wl_kKeyLocationRight : wl_kKeyLocationLeft;
		break;
	case wl_kKeyReturn:
		loc = extended ? wl_kKeyLocationNumPad : wl_kKeyLocationDefault;
		break;

	case wl_kKeyInsert:
	case wl_kKeyDelete:
	case wl_kKeyHome:
	case wl_kKeyEnd:
	case wl_kKeyPageUp:
	case wl_kKeyPageDown:
	case wl_kKeyLeftArrow:
	case wl_kKeyRightArrow:
	case wl_kKeyUpArrow:
	case wl_kKeyDownArrow:
		loc = extended ? wl_kKeyLocationDefault : wl_kKeyLocationNumPad;
		break;
	}
	return loc;
}

void keystuff_init()
{
	initKeyMap();
}
