#include "../openwl.h"

#include "globals.h"
#include "win32util.h"

#include "wndproc.h" // topLevelWindowProc, appGlobalWindowProc
#include "unicodestuff.h"
#include "comstuff.h"

#include "private_defs.h"
#include "pngloader.h"

// misc "modules"
#include "window.h"
#include "timer.h"
#include "cursor.h"
#include "action.h"
#include "keystuff.h"

#include <stdio.h>
#include <assert.h>

OPENWL_API int CDECL wl_Init(wl_EventCallback callback, struct wl_PlatformOptions* options)
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	eventCallback = callback;
	registerWindowClass(topLevelWindowClass, topLevelWindowProc);
	registerWindowClass(appGlobalWindowClass, appGlobalWindowProc);

	OleInitialize(nullptr);

	if (options) {
		if (options->useDirect2D) {
			useDirect2D = true;

			HR(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2dFactory));

			// return to client so that it can be used elsewhere (by the drawing layer, etc)
			options->outParams.factory = d2dFactory;
		}
	}

	// essential info for timers
	QueryPerformanceFrequency(&perfCounterTicksPerSecond);
	printf("perf counter freq: %lld\n", perfCounterTicksPerSecond.QuadPart);

	// for receiving messages that don't belong to any window:
	// see: https://devblogs.microsoft.com/oldnewthing/20050426-18/?p=35783 "Thread messages are eaten by modal loops"
	// and: https://devblogs.microsoft.com/oldnewthing/?p=16553 "Why do messages posted by PostThreadMessage disappear?"
	// note 'HWND_MESSAGE' parent, for message-only windows
	appGlobalWindow = CreateWindow(appGlobalWindowClass, L"(app global)", WS_OVERLAPPED, 0, 0, 10, 10, HWND_MESSAGE, NULL, hInstance, NULL);

	// various "module" inits ======
	cursor_init();
	keystuff_init();

	return 0;
}

OPENWL_API void CDECL wl_Shutdown()
{
	SafeRelease(&d2dFactory);
	OleUninitialize();
}

OPENWL_API int CDECL wl_Runloop()
{
	HACCEL hAccelTable = wl_Action::createAccelTable();

	// Main message loop:
	MSG msg;
	BOOL bRet;
	while ((bRet = GetMessage(&msg, nullptr, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			// handle the error and possibly exit
			printf("win32 message loop error, exiting\n");
			break;
		}
		else
		{
			// note that thread-only (windowless) messages such as WM_WLTimerMessage / WM_WLMainThreadExecMsg
			//   are now being processed by an invisible / message-only window
			//   (because otherwise they won't be processed during modal events, like dragging the window, or DnD)
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	return (int)msg.wParam;
}

OPENWL_API void CDECL wl_ExitRunloop()
{
	PostQuitMessage(0);
}

OPENWL_API wl_WindowRef CDECL wl_WindowCreate(int width, int height, const char* title, void* userData, struct wl_WindowProperties* props)
{
	return wl_Window::create(width, height, title, userData, props);
}

OPENWL_API void CDECL wl_WindowDestroy(wl_WindowRef window)
{
	window->wlDestroy();
}

OPENWL_API void CDECL wl_WindowShow(wl_WindowRef window)
{
	window->show();
}

OPENWL_API void CDECL wl_WindowShowRelative(wl_WindowRef window, wl_WindowRef relativeTo, int x, int y, int newWidth, int newHeight)
{
	window->showRelative(relativeTo, x, y, newWidth, newHeight);
}

OPENWL_API void CDECL wl_WindowHide(wl_WindowRef window)
{
	window->hide();
}

OPENWL_API void CDECL wl_WindowInvalidate(wl_WindowRef window, int x, int y, int width, int height)
{
	window->invalidate(x, y, width, height);
}

/* timer API */

OPENWL_API wl_TimerRef CDECL wl_TimerCreate(unsigned int msTimeout, void* userData)
{
	return wl_Timer::create(msTimeout, userData);
}

OPENWL_API void CDECL wl_TimerDestroy(wl_TimerRef timer)
{
	delete timer;
}

// menus =============================================

/* action API */
OPENWL_API wl_IconRef CDECL wl_IconLoadFromFile(const char* filename, int sizeToWidth)
{
	auto pngBitmap = loadPngAndResize(filename, sizeToWidth, sizeToWidth);
	if (pngBitmap) {
		auto retIcon = new wl_Icon;
		retIcon->hbitmap = pngBitmap;
		return retIcon;
	}
	return nullptr;
}

OPENWL_API wl_AcceleratorRef CDECL wl_AccelCreate(enum wl_KeyEnum key, unsigned int modifiers)
{
	auto retAccel = new wl_Accelerator;
	retAccel->key = key;
	retAccel->modifiers = modifiers;
	return retAccel;
}

OPENWL_API wl_ActionRef CDECL wl_ActionCreate(int id, const char* label, wl_IconRef icon, wl_AcceleratorRef accel)
{
	return wl_Action::create(id, label, icon, accel);
}

/* menu API */
OPENWL_API wl_MenuRef CDECL wl_MenuCreate()
{
	wl_MenuRef retMenu = new wl_Menu;
	retMenu->hmenu = CreatePopupMenu();
	return retMenu;
}

OPENWL_API wl_MenuItemRef CDECL wl_MenuAddAction(wl_MenuRef menu, wl_ActionRef action)
{
	// wl_ActionRef is currently more complicated, with private data, so we let it add itself
	// (whereas wl_MenuRefs are still dumb structs)
	return action->addToMenu(menu);
}

OPENWL_API wl_MenuItemRef CDECL wl_MenuAddSubmenu(wl_MenuRef menu, const char* label, wl_MenuRef sub)
{
	auto wideLabel = utf8_to_wstring(label);

	wl_MenuItemRef retItem = new wl_MenuItem;
	memset(retItem, 0, sizeof(wl_MenuItem));
	AppendMenu(menu->hmenu, MF_STRING | MF_POPUP, (UINT_PTR)sub->hmenu, wideLabel.c_str());
	retItem->subMenu = sub;
	return retItem;
}

OPENWL_API void CDECL wl_MenuAddSeparator(wl_MenuRef menu)
{
	AppendMenu(menu->hmenu, MF_SEPARATOR, 0, NULL);
}

OPENWL_API wl_MenuItemRef CDECL wl_MenuBarAddMenu(wl_MenuBarRef menuBar, const char* label, wl_MenuRef menu)
{
	auto wideLabel = utf8_to_wstring(label);

	AppendMenu(menuBar->hmenu, MF_STRING | MF_POPUP, (UINT_PTR)menu->hmenu, wideLabel.c_str());
	auto retMenuItem = new wl_MenuItem;
	retMenuItem->action = nullptr;
	retMenuItem->subMenu = menu;
	return retMenuItem;
}

OPENWL_API void CDECL wl_WindowShowContextMenu(wl_WindowRef window, int x, int y, wl_MenuRef menu, struct wl_Event* fromEvent)
{
	window->showContextMenu(x, y, menu, fromEvent);
}

OPENWL_API wl_MenuBarRef CDECL wl_MenuBarCreate()
{
	wl_MenuBarRef retMenuBar = new wl_MenuBar;
	retMenuBar->hmenu = CreateMenu();
	return retMenuBar;
}

OPENWL_API void CDECL wl_WindowSetMenuBar(wl_WindowRef window, wl_MenuBarRef menuBar)
{
	window->setMenuBar(menuBar);
}

// misc ===============================================

OPENWL_API void CDECL wl_ExecuteOnMainThread(wl_VoidCallback callback, void* data)
{
	std::unique_lock<std::mutex> lock(execMutex);
	std::condition_variable cond;

	MainThreadExecItem item = { callback, data, cond };
	PostMessage(appGlobalWindow, WM_WLMainThreadExecMsg, 0, (LPARAM)&item); // safe to pass a pointer to item, because this function doesn't exit until it's done

	// block until it's done executing
	cond.wait(lock);
}

OPENWL_API void CDECL wl_Sleep(unsigned int millis)
{
	Sleep(millis);
}

OPENWL_API void CDECL wl_MouseGrab(wl_WindowRef window)
{
	window->grab();
}

OPENWL_API void CDECL wl_MouseUngrab()
{
	wl_Window::ungrab();
}

OPENWL_API wl_CursorRef CDECL wl_CursorCreate(wl_CursorStyle style)
{
	return wl_Cursor::create(style);
}

OPENWL_API void CDECL wl_WindowSetCursor(wl_WindowRef window, wl_CursorRef cursor)
{
	window->setCursor(cursor);
}
