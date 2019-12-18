#include "cursor.h"

#include "globals.h"
#include <map>

// loaded cursors map
static std::map<wl_CursorStyle, wl_CursorRef> cursorMap;

wl_CursorRef wl_Cursor::defaultCursor;

void cursor_init()
{
	wl_Cursor::defaultCursor = wl_Cursor::create(wl_kCursorStyleDefault);
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
		LPWSTR name = IDC_ARROW;
		switch (style) {
		case wl_kCursorStyleDefault:
			name = IDC_ARROW;
			break;
		case wl_kCursorStyleResizeLeftRight:
			name = IDC_SIZEWE;
			break;
		case wl_kCursorStyleResizeUpDown:
			name = IDC_SIZENS;
			break;
		case wl_kCursorStyleIBeam:
			name = IDC_IBEAM;
			break;
		}
		auto ret = new wl_Cursor();
		ret->handle = (HCURSOR)LoadImage(NULL, name, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
		cursorMap[style] = ret; // save for future reference
		return ret;
	}
}
