#include "../openwl.h"

#include "globals.h"
#include "win32util.h"

#include "wndproc.h" // topLevelWindowProc, appGlobalWindowProc
#include "unicodestuff.h"

#include "window.h"
#include "comstuff.h"

#include <stdio.h>
#include <assert.h>

OPENWL_API int CDECL wl_Init(wl_EventCallback callback, struct wl_PlatformOptions* options)
{
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

	// for receiving messages that don't belong to any window:
	// see: https://devblogs.microsoft.com/oldnewthing/20050426-18/?p=35783 "Thread messages are eaten by modal loops"
	// and: https://devblogs.microsoft.com/oldnewthing/?p=16553 "Why do messages posted by PostThreadMessage disappear?"
	// note 'HWND_MESSAGE' parent, for message-only windows
	appGlobalWindow = CreateWindow(appGlobalWindowClass, L"(app global)", WS_OVERLAPPED, 0, 0, 10, 10, HWND_MESSAGE, NULL, hInstance, NULL);

	return 0;
}

OPENWL_API void CDECL wl_Shutdown()
{
	SafeRelease(&d2dFactory);
	OleUninitialize();
}

OPENWL_API int CDECL wl_Runloop()
{
	//HACCEL hAccelTable = CreateAcceleratorTable((ACCEL*)acceleratorList.data(), (int)acceleratorList.size());

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
			//// note that thread-only (windowless) messages such as WM_WLTimerMessage / WM_WLMainThreadExecMsg
			////   are now being processed in a message hook (see wl_Init for that).
			////   (because otherwise they won't be processed during modal events, like dragging the window, or DnD
			//// normal window message processing
			//if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			//{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			//}
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
	window->destroy();
}

OPENWL_API void CDECL wl_WindowShow(wl_WindowRef window)
{
	window->show();
}
