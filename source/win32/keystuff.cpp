#include <Windows.h>

#include "keystuff.h"

#include "unicodestuff.h"

std::map<WPARAM, KeyInfo *> keyMap; // win32 virtual keys to WL key enums
std::map<WLKeyEnum, KeyInfo *> reverseKeyMap; // reverse

#define CALCULATE_LOCATION -1
#define LOOKUP_AT_RUNTIME "##lookup at runtime##"

KeyInfo infoItems[] = {
	{ 0, WLKey_Unknown, "(Unknown)", false },
	//
	{ VK_ESCAPE, WLKey_Escape, "Escape", true },
	{ VK_BACK, WLKey_Backspace, "Backspace", true },
	{ VK_TAB, WLKey_Tab, "Tab", true },
	{ VK_CAPITAL, WLKey_CapsLock, "Caps Lock", false },
	{ VK_RETURN, WLKey_Return, "Return", true, CALCULATE_LOCATION },
	{ VK_SPACE, WLKey_Space, "Space", false },
	//
	{ VK_CONTROL, WLKey_Control, "Control", false, CALCULATE_LOCATION },
	{ VK_SHIFT, WLKey_Shift, "Shift", false, CALCULATE_LOCATION },
	{ VK_MENU, WLKey_AltOption, "Alt/Option", false, CALCULATE_LOCATION },
	{ VK_LWIN, WLKey_WinCommand, "Left Windows", false, WLKeyLocation_Left },
	{ VK_RWIN, WLKey_WinCommand, "Right Windows", false, WLKeyLocation_Right },
	//
	{ VK_F1, WLKey_F1, "F1", false },
	{ VK_F2, WLKey_F2, "F2", false },
	{ VK_F3, WLKey_F3, "F3", false },
	{ VK_F4, WLKey_F4, "F4", false },
	{ VK_F5, WLKey_F5, "F5", false },
	{ VK_F6, WLKey_F6, "F6", false },
	{ VK_F7, WLKey_F7, "F7", false },
	{ VK_F8, WLKey_F8, "F8", false },
	{ VK_F9, WLKey_F9, "F9", false },
	{ VK_F10, WLKey_F10, "F10", false },
	{ VK_F11, WLKey_F11, "F11", false },
	{ VK_F12, WLKey_F12, "F12", false },
	{ VK_F13, WLKey_F13, "F13", false },
	{ VK_F14, WLKey_F14, "F14", false },
	{ VK_F15, WLKey_F15, "F15", false },
	{ VK_F16, WLKey_F16, "F16", false },
	{ VK_F17, WLKey_F17, "F17", false },
	{ VK_F18, WLKey_F18, "F18", false },
	{ VK_F19, WLKey_F19, "F19", false },
	//
	{ '0', WLKey_0, "0", false },
	{ '1', WLKey_1, "1", false },
	{ '2', WLKey_2, "2", false },
	{ '3', WLKey_3, "3", false },
	{ '4', WLKey_4, "4", false },
	{ '5', WLKey_5, "5", false },
	{ '6', WLKey_6, "6", false },
	{ '7', WLKey_7, "7", false },
	{ '8', WLKey_8, "8", false },
	{ '9', WLKey_9, "9", false },
	//
	{ 'A', WLKey_A, "A", false },
	{ 'B', WLKey_B, "B", false },
	{ 'C', WLKey_C, "C", false },
	{ 'D', WLKey_D, "D", false },
	{ 'E', WLKey_E, "E", false },
	{ 'F', WLKey_F, "F", false },
	{ 'G', WLKey_G, "G", false },
	{ 'H', WLKey_H, "H", false },
	{ 'I', WLKey_I, "I", false },
	{ 'J', WLKey_J, "J", false },
	{ 'K', WLKey_K, "K", false },
	{ 'L', WLKey_L, "L", false },
	{ 'M', WLKey_M, "M", false },
	{ 'N', WLKey_N, "N", false },
	{ 'O', WLKey_O, "O", false },
	{ 'P', WLKey_P, "P", false },
	{ 'Q', WLKey_Q, "Q", false },
	{ 'R', WLKey_R, "R", false },
	{ 'S', WLKey_S, "S", false },
	{ 'T', WLKey_T, "T", false },
	{ 'U', WLKey_U, "U", false },
	{ 'V', WLKey_V, "V", false },
	{ 'W', WLKey_W, "W", false },
	{ 'X', WLKey_X, "X", false },
	{ 'Y', WLKey_Y, "Y", false },
	{ 'Z', WLKey_Z, "Z", false },
	// keypad
	{ VK_NUMPAD0, WLKey_KP_0, "Numpad 0", false, WLKeyLocation_NumPad },
	{ VK_NUMPAD1, WLKey_KP_1, "Numpad 1", false, WLKeyLocation_NumPad },
	{ VK_NUMPAD2, WLKey_KP_2, "Numpad 2", false, WLKeyLocation_NumPad },
	{ VK_NUMPAD3, WLKey_KP_3, "Numpad 3", false, WLKeyLocation_NumPad },
	{ VK_NUMPAD4, WLKey_KP_4, "Numpad 4", false, WLKeyLocation_NumPad },
	{ VK_NUMPAD5, WLKey_KP_5, "Numpad 5", false, WLKeyLocation_NumPad },
	{ VK_NUMPAD6, WLKey_KP_6, "Numpad 6", false, WLKeyLocation_NumPad },
	{ VK_NUMPAD7, WLKey_KP_7, "Numpad 7", false, WLKeyLocation_NumPad },
	{ VK_NUMPAD8, WLKey_KP_8, "Numpad 8", false, WLKeyLocation_NumPad },
	{ VK_NUMPAD9, WLKey_KP_9, "Numpad 9", false, WLKeyLocation_NumPad },
	//
	{ VK_NUMLOCK, WLKey_NumLock, "NumLock", false, WLKeyLocation_NumPad },
	{ VK_CLEAR, WLKey_KP_Clear, "KP-Clear", false, WLKeyLocation_NumPad },
	{ VK_DIVIDE, WLKey_KP_Divide, "KP-Divide", false, WLKeyLocation_NumPad },
	{ VK_MULTIPLY, WLKey_KP_Multiply, "KP-Multiply", false, WLKeyLocation_NumPad },
	{ VK_SUBTRACT, WLKey_KP_Subtract, "KP-Subtract", false, WLKeyLocation_NumPad },
	{ VK_ADD, WLKey_KP_Add, "KP-Add", false, WLKeyLocation_NumPad },
	//
	{ VK_INSERT, WLKey_Insert, "Insert", false, CALCULATE_LOCATION },
	{ VK_DELETE, WLKey_Delete, "Delete", false, CALCULATE_LOCATION },
	{ VK_HOME, WLKey_Home, "Home", false, CALCULATE_LOCATION },
	{ VK_END, WLKey_End, "End", false, CALCULATE_LOCATION },
	{ VK_PRIOR, WLKey_PageUp, "PageUp", false, CALCULATE_LOCATION },
	{ VK_NEXT, WLKey_PageDown, "PageDown", false, CALCULATE_LOCATION },
	//
	{ VK_UP, WLKey_UpArrow, "UpArrow", false, CALCULATE_LOCATION },
	{ VK_LEFT, WLKey_LeftArrow, "LeftArrow", false, CALCULATE_LOCATION },
	{ VK_RIGHT, WLKey_RightArrow, "RightArrow", false, CALCULATE_LOCATION },
	{ VK_DOWN, WLKey_DownArrow, "DownArrow", false, CALCULATE_LOCATION },
	//
	{ VK_SNAPSHOT, WLKey_PrintScreen, "PrintScrn", false },
	{ VK_SCROLL, WLKey_ScrollLock, "ScrollLock", false },
	{ VK_PAUSE, WLKey_Pause, "Pause", false },
	{ VK_CANCEL, WLKey_Cancel, "Cancel (Ctrl-Break)", true },
	//
	{ VK_VOLUME_MUTE, WLKey_MediaMute, "Media Mute", false },
	{ VK_VOLUME_DOWN, WLKey_MediaVolumeDown, "Media Volume Down", false },
	{ VK_VOLUME_UP, WLKey_MediaVolumeUp, "Media Volume Up", false },
	{ VK_MEDIA_NEXT_TRACK, WLKey_MediaNext, "Media Next", false },
	{ VK_MEDIA_PREV_TRACK, WLKey_MediaPrev, "Media Prev", false },
	{ VK_MEDIA_STOP, WLKey_MediaStop, "Media Stop", false },
	{ VK_MEDIA_PLAY_PAUSE, WLKey_MediaPlayPause, "Media Play/Pause", false },
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

WLKeyLocation locationForKey(WLKeyEnum key, unsigned char scanCode, bool extended)
{
	auto loc = WLKeyLocation_Default;
	switch (key) {
	case WLKey_Shift:
		//printf("locationForKey WLKeyShift: %d %d %d\n", key, scanCode, extended);
		loc = (scanCode == 54) ? WLKeyLocation_Right : WLKeyLocation_Left;
		break;
	case WLKey_Control:
	case WLKey_AltOption:
		loc = extended ? WLKeyLocation_Right : WLKeyLocation_Left;
		break;
	case WLKey_Return:
		loc = extended ? WLKeyLocation_NumPad : WLKeyLocation_Default;
		break;

	case WLKey_Insert:
	case WLKey_Delete:
	case WLKey_Home:
	case WLKey_End:
	case WLKey_PageUp:
	case WLKey_PageDown:
	case WLKey_LeftArrow:
	case WLKey_RightArrow:
	case WLKey_UpArrow:
	case WLKey_DownArrow:
		loc = extended ? WLKeyLocation_Default : WLKeyLocation_NumPad;
		break;
	}
	return loc;
}

