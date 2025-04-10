#include "cursor.h"

#include "globals.h"
#include <map>

// loaded cursors map
static std::map<wl_CursorStyle, wl_CursorRef> cursorMap;
static std::map<wl_CursorStyle, LPWSTR> w32CursorMap;

wl_CursorRef wl_Cursor::defaultCursor;

void cursor_init()
{
	wl_Cursor::defaultCursor = wl_Cursor::create(wl_kCursorStyleDefault);

	w32CursorMap[wl_kCursorStyleDefault] = IDC_ARROW;
	w32CursorMap[wl_kCursorStyleTextSelect] = IDC_IBEAM;
	w32CursorMap[wl_kCursorStyleBusyWait] = IDC_WAIT;
	w32CursorMap[wl_kCursorStyleCross] = IDC_CROSS;
	w32CursorMap[wl_kCursorStyleUpArrow] = IDC_UPARROW;
	w32CursorMap[wl_kCursorStyleResizeTopLeftBottomRight] = IDC_SIZENWSE;
	w32CursorMap[wl_kCursorStyleResizeTopRightBottomLeft] = IDC_SIZENESW;
	w32CursorMap[wl_kCursorStyleResizeLeftRight] = IDC_SIZEWE;
	w32CursorMap[wl_kCursorStyleResizeUpDown] = IDC_SIZENS;
	w32CursorMap[wl_kCursorStyleMove] = IDC_SIZEALL;
	w32CursorMap[wl_kCursorStyleUnavailable] = IDC_NO;
	w32CursorMap[wl_kCursorStyleHandSelect] = IDC_HAND;
	w32CursorMap[wl_kCursorStylePointerWorking] = IDC_APPSTARTING;
	w32CursorMap[wl_kCursorStyleHelpSelect] = IDC_HELP;
	w32CursorMap[wl_kCursorStyleLocationSelect] = IDC_PIN;
	w32CursorMap[wl_kCursorStylePersonSelect] = IDC_PERSON;
	w32CursorMap[wl_kCursorStyleHandwriting] = MAKEINTRESOURCE(32631);
}

void wl_Cursor::set()
{
	SetCursor(handle);
}

wl_CursorRef wl_Cursor::create(wl_CursorStyle style)
{
	auto found = cursorMap.find(style);
	if (found != cursorMap.end()) {
		return found->second;
	}
	else {
		auto name = IDC_ARROW;

		// convert to win32 style
		auto found = w32CursorMap.find(style);
		if (found != w32CursorMap.end()) {
			name = found->second;
		}

		auto ret = new wl_Cursor();
		ret->handle = (HCURSOR)LoadImage(NULL, name, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
		cursorMap[style] = ret; // save for future reference
		return ret;
	}
}
