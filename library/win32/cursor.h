#pragma once

#include "../openwl.h"

void cursor_init(); // cursor "module" init, place in wl_Init

struct wl_Cursor {
private:
	HCURSOR handle;
public:
	static wl_CursorRef create(wl_CursorStyle style);
	static wl_CursorRef defaultCursor;

	void set();
};
