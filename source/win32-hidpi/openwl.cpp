#include "../openwl.h"

#include "globals.h"
#include "win32util.h"

#include "wndproc.h" // topLevelWindowProc, appGlobalWindowProc

#include <stdio.h>

OPENWL_API int CDECL wl_Init(wl_EventCallback callback, struct wl_PlatformOptions* options)
{
	eventCallback = callback;
	registerWindowClass(topLevelWindowClass, topLevelWindowProc);
	registerWindowClass(appGlobalWindowClass, appGlobalWindowProc);

	// for receiving messages that don't belong to any window:
	// see: https://devblogs.microsoft.com/oldnewthing/20050426-18/?p=35783 "Thread messages are eaten by modal loops"
	// and: https://devblogs.microsoft.com/oldnewthing/?p=16553 "Why do messages posted by PostThreadMessage disappear?"
	// note 'HWND_MESSAGE' parent, for message-only windows
	appGlobalWindow = CreateWindow(appGlobalWindowClass, L"(app global)", WS_OVERLAPPED, 0, 0, 10, 10, HWND_MESSAGE, NULL, hInstance, NULL);

	return 0;
}

OPENWL_API void CDECL wl_Shutdown()
{
	//if (useDirect2D) {
	//	d2dFactory->Release();
	//}
	//OleUninitialize();
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
