// OpenWL.cpp : Defines the exported functions for the DLL application.
//

//#define OPENWL_EXPORTS // defined in project
#include "../openwl.h"

#define WIN32_LEAN_AND_MEAN // can't use with gdiplus
#include <Windows.h>
#include <shlobj.h> // for DROPFILES
#include <shellapi.h> // for DragQueryFile

#include <assert.h>
#include <cstring>

#include "resource.h"

// OLE D&D stuff, sigh
#include <Ole2.h>
#include "dragdrop/dropsource.h"

#include <boost/algorithm/string/join.hpp>

#include "globals.h"
#include "private_defs.h" // content for all the opaque API types

#include "pngloader.h"

#include "unicodestuff.h"
#include "keystuff.h"
#include "wndproc.h"

#include "MyDropTarget.h"

// fwd declarations

void RegisterDropWindow(wl_WindowRef window, IDropTarget **ppDropTarget);
void UnregisterDropWindow(wl_WindowRef window, IDropTarget *pDropTarget);

void registerWindowClass() {
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = NULL; // LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = NULL; // (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL; // MAKEINTRESOURCEW(IDR_MENU1);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = NULL; // LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);
}


OPENWL_API int CDECL wl_Init(wl_EventCallback callback, struct wl_PlatformOptions *options) {
	initKeyMap();

	eventCallback = callback;
	registerWindowClass();

	acceleratorList.clear();

	OleInitialize(nullptr);

    if (options) {
        if (options->useDirect2D) {
            HRESULT hr;

            useDirect2D = true;

            hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2dFactory);
            assert(SUCCEEDED(hr));

			// return to client so that it can be used elsewhere (by the drawing layer, etc)
			options->outParams.factory = d2dFactory;
        }
    }

    QueryPerformanceFrequency(&perfCounterTicksPerSecond);
    printf("perf counter freq: %lld\n", perfCounterTicksPerSecond.QuadPart);

	// any "module" inits here ====

	private_defs_init();

	return 0;
}

void wl_Shutdown() {
    if (useDirect2D) {
        d2dFactory->Release();
    }
	OleUninitialize();
}

void calcChromeExtra(int *extraWidth, int *extraHeight, DWORD dwStyle, BOOL hasMenu) {
    const int arbitraryExtent = 500;
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = arbitraryExtent; // just some arbitrary extents -- it's the difference we're interested in
    rect.bottom = arbitraryExtent;
    AdjustWindowRect(&rect, dwStyle, hasMenu);
    *extraWidth = (rect.right - rect.left) - arbitraryExtent;  // left and top will be negative, hence the subtraction (right - left) = outer width
    *extraHeight = (rect.bottom - rect.top) - arbitraryExtent; // bottom - top = outer height
}

long getWindowStyle(wl_WindowProperties *props, bool isPluginWindow) {
	long dwStyle = WS_OVERLAPPEDWINDOW;
	if (isPluginWindow) {
		dwStyle = WS_CHILD;
	} else if (props && (props->usedFields & wl_kWindowPropStyle)) {
		switch (props->style) {
		case wl_kWindowStyleDefault:
			dwStyle = WS_OVERLAPPEDWINDOW;
			break;
		case wl_kWindowStyleFrameless:
			dwStyle = WS_POPUP | WS_BORDER;
			break;
		default:
			printf("wl_WindowCreate: unknown window style\n");
			break;
		}
	}
	return dwStyle;
}

wl_WindowRef wl_WindowCreate(int width, int height, const char *title, void *userData, wl_WindowProperties *props)
{
	auto wideTitle = title ? utf8_to_wstring(title) : L"(UNTITLED)";

	int extraWidth = 0;
	int extraHeight = 0;

	bool isPluginWindow = (props &&
		(props->usedFields & wl_kWindowPropStyle) &&
		(props->usedFields & wl_kWindowPropNativeParent) &&
		(props->style == wl_kWindowStylePluginWindow));

	auto dwStyle = getWindowStyle(props, isPluginWindow);

	HWND hWnd = NULL;
	if (isPluginWindow)
	{
		hWnd = CreateWindowW(szWindowClass, wideTitle.c_str(), dwStyle,
			0, 0,
			width, height, props->nativeParent, nullptr, hInstance, nullptr);
	}
	else {
		// normal top-level window (or frameless)
		calcChromeExtra(&extraWidth, &extraHeight, dwStyle, FALSE); // FALSE = no menu for now ... will recalc when the time comes

		auto exStyle = (dwStyle & WS_POPUP) ? WS_EX_TOOLWINDOW : 0; // no taskbar button plz

		hWnd = CreateWindowExW(exStyle, szWindowClass, wideTitle.c_str(), dwStyle,
			CW_USEDEFAULT, CW_USEDEFAULT,
			width + extraWidth, height + extraHeight, nullptr, nullptr, hInstance, nullptr);
	}

	if (hWnd) {
		// associate data
		wl_WindowRef wlw = new wl_Window;
		wlw->hwnd = hWnd;
		wlw->dwStyle = dwStyle;
        wlw->clientWidth = width;
        wlw->clientHeight = height;
        wlw->extraWidth = extraWidth;
        wlw->extraHeight = extraHeight;
		wlw->userData = userData;
		wlw->dropTarget = nullptr;
		wlw->props.usedFields = 0;
		if (props != nullptr) {
			memcpy(&wlw->props, props, sizeof(wl_WindowProperties));
		}
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)wlw);

        if (useDirect2D) {
			d2dCreateTarget(wlw);
        }

        //ShowWindow(hWnd, SW_SHOWNORMAL);
        //UpdateWindow(hWnd);

		return wlw;
	}
	// else
	return nullptr;
}

void wl_WindowDestroy(wl_WindowRef window) {
	if (window->dropTarget) {
		UnregisterDropWindow(window, window->dropTarget);
		window->dropTarget = nullptr;
	}
	DestroyWindow(window->hwnd);
}

void wl_WindowShow(wl_WindowRef window)
{
	ShowWindow(window->hwnd, SW_SHOWNORMAL); // might need to use a different cmd based on whether first time or not
	UpdateWindow(window->hwnd);
}

OPENWL_API void CDECL wl_WindowShowRelative(wl_WindowRef window, wl_WindowRef relativeTo, int x, int y, int newWidth, int newHeight)
{
	POINT p{ x, y };
	ClientToScreen(relativeTo->hwnd, &p);
	auto doSize = (newWidth > 0 && newHeight > 0);
	auto flags = (doSize ? 0 : SWP_NOSIZE) | SWP_SHOWWINDOW | SWP_NOACTIVATE;
	SetWindowPos(window->hwnd, HWND_TOP, p.x, p.y, newWidth, newHeight, flags);
}

void wl_WindowHide(wl_WindowRef window)
{
	ShowWindow(window->hwnd, SW_HIDE);
}

int wl_Runloop() {
	//HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));
	printf("installing %d accelerators\n", (int)acceleratorList.size());
	HACCEL hAccelTable = CreateAcceleratorTable((ACCEL *)acceleratorList.data(), (int)acceleratorList.size());

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
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	return (int)msg.wParam;
}

void wl_ExitRunloop()
{
	PostQuitMessage(0);
}

void wl_WindowInvalidate(wl_WindowRef window, int x, int y, int width, int height)
{
	if (width > 0 && height > 0) {
		RECT r;
		r.left = x;
		r.top = y;
		r.right = x + width;
		r.bottom = y + height;
		InvalidateRect(window->hwnd, &r, FALSE);
	}
	else {
		InvalidateRect(window->hwnd, nullptr, FALSE); // entire window
	}
}

OPENWL_API size_t CDECL wl_WindowGetOSHandle(wl_WindowRef window)
{
	return (size_t)window->hwnd;
}


VOID CALLBACK timerCallback(_In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired) {
	wl_TimerRef timer = (wl_TimerRef)lpParameter;
	PostMessage(timer->window->hwnd, OPENWL_TIMER_MESSAGE, 0, (LPARAM)timer);
}

OPENWL_API wl_TimerRef CDECL wl_TimerCreate(wl_WindowRef window, int timerID, unsigned int msTimeout)
{
	// requires a window because it has userdat and other stuff  ...
	// which I guess we could put into a wl_TimerRef structure, but then we have multiple types of userdata ...
	wl_TimerRef timer = new wl_Timer;
	timer->timerID = timerID;
    QueryPerformanceCounter(&timer->lastPerfCount);
	timer->window = window;
	timer->timerQueue = NULL; // default timer queue ... not sure why we created one to begin with // CreateTimerQueue();

	if (!CreateTimerQueueTimer(&timer->handle, timer->timerQueue, timerCallback, timer, msTimeout, msTimeout, WT_EXECUTEDEFAULT)) {
		printf("failed to create TimerQueueTimer\n");
		return nullptr;
	}
	return timer;
}

OPENWL_API void CDECL wl_TimerDestroy(wl_TimerRef timer)
{
	auto timerDeleted = CreateEvent(NULL, TRUE, FALSE, NULL);
	DeleteTimerQueueTimer(timer->timerQueue, timer->handle, timerDeleted);
	WaitForSingleObject(timerDeleted, INFINITE);
	printf("wait complete, deleting timer\n");
	delete timer;
	CloseHandle(timerDeleted);
}


/************************************/
/*           MENU API               */
/************************************/

wl_MenuRef wl_MenuCreate()
{
	wl_MenuRef retMenu = new wl_Menu;
	retMenu->hmenu = CreatePopupMenu();
	return retMenu;
}

wl_MenuItemRef wl_MenuAddSubmenu(wl_MenuRef menu, const char *label, wl_MenuRef sub)
{
	auto wideLabel = utf8_to_wstring(label);

	wl_MenuItemRef retItem = new wl_MenuItem;
	memset(retItem, 0, sizeof(wl_MenuItem));
	AppendMenu(menu->hmenu, MF_STRING | MF_POPUP, (UINT_PTR)sub->hmenu, wideLabel.c_str());
	retItem->subMenu = sub;
	return retItem;
}

void wl_MenuAddSeparator(wl_MenuRef menu)
{
	AppendMenu(menu->hmenu, MF_SEPARATOR, 0, NULL);
}

wl_MenuBarRef wl_MenuBarCreate()
{
	wl_MenuBarRef retMenuBar = new wl_MenuBar;
	retMenuBar->hmenu = CreateMenu();
	return retMenuBar;
}

wl_MenuItemRef wl_MenuBarAddMenu(wl_MenuBarRef menuBar, const char *label, wl_MenuRef menu)
{
	auto wideLabel = utf8_to_wstring(label);

	AppendMenu(menuBar->hmenu, MF_STRING | MF_POPUP, (UINT_PTR)menu->hmenu, wideLabel.c_str());
	auto retMenuItem = new wl_MenuItem;
	retMenuItem->action = nullptr;
	retMenuItem->subMenu = menu;
	return retMenuItem;
}

void wl_WindowSetMenuBar(wl_WindowRef window, wl_MenuBarRef menuBar)
{
	window->hasMenu = TRUE; // use win32 bool constant ... but shouldn't be different from C++ true/false

    // recalc the "extra" (window chrome extents) so that we're dealing with inner client area
    // this is needed for enforcement of min/max sizes as well as just fixing up the client size below
    calcChromeExtra(&window->extraWidth, &window->extraHeight, window->dwStyle, window->hasMenu);

    // force resize to preserve client area
    SetWindowPos(window->hwnd, NULL, 0, 0, 
        window->clientWidth + window->extraWidth, 
        window->clientHeight + window->extraHeight, 
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
    
    // don't set the menu until the very end, otherwise it sends a WM_SIZE message that fucks our calculations up
    SetMenu(window->hwnd, menuBar->hmenu);
}

OPENWL_API wl_ActionRef CDECL wl_ActionCreate(int id, const char *label, wl_IconRef icon, wl_AcceleratorRef accel)
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
	parts.push_back(upperCased(stringRep));
	//
	auto joined = boost::algorithm::join(parts, "+");
	return joined;
}

wl_MenuItemRef wl_MenuAddAction(wl_MenuRef menu, wl_ActionRef action)
{
	wl_MenuItemRef retItem = new wl_MenuItem;
	memset(retItem, 0, sizeof(wl_MenuItem));
	
	// alter label if accelerator present
	std::string label = action->label;
	printf("wl_MenuAddAction label: %s\n", label.c_str());
	if (action->accel) {
		label += "\t";
		label += accelToString(action->accel);
		assert(label.compare(action->label) != 0); // err right?
	}

	auto wideLabel = utf8_to_wstring(label);
	AppendMenu(menu->hmenu, MF_STRING, action->id, wideLabel.c_str());
	if (action->icon) {
		// set the bitmap
		MENUITEMINFO info;
		info.cbSize = sizeof(MENUITEMINFO);
		info.fMask = MIIM_BITMAP;
		info.hbmpItem = action->icon->hbitmap;
		SetMenuItemInfo(menu->hmenu, action->id, FALSE, &info);
	}
	retItem->action = action;
	action->attachedItems.push_back(retItem);
return retItem;
}

//#define TEMP_BMP L"temp.bmp"

wl_IconRef wl_IconLoadFromFile(const char *filename, int sizeToWidth)
{
    auto pngBitmap = loadPngAndResize(filename, sizeToWidth, sizeToWidth);
    if (pngBitmap) {
        auto retIcon = new wl_Icon;
        retIcon->hbitmap = pngBitmap;
        return retIcon;
    }
    return nullptr;
}

OPENWL_API void CDECL wl_WindowShowContextMenu(wl_WindowRef window, int x, int y, wl_MenuRef menu, wl_Event *fromEvent)
{
	int needsRightAlign = GetSystemMetrics(SM_MENUDROPALIGNMENT);
	UINT alignFlags = needsRightAlign ? TPM_RIGHTALIGN : TPM_LEFTALIGN;
	// translate x,y into screen coords
	POINT point;
	point.x = x;
	point.y = y;
	ClientToScreen(window->hwnd, &point);
	TrackPopupMenu(menu->hmenu, alignFlags | TPM_TOPALIGN | TPM_LEFTBUTTON, point.x, point.y, 0, window->hwnd, NULL);
}

OPENWL_API wl_AcceleratorRef CDECL wl_AccelCreate(enum wl_KeyEnum key, unsigned int modifiers)
{
	auto retAccel = new wl_Accelerator;
	retAccel->key = key;
	retAccel->modifiers = modifiers;
	return retAccel;
}

/********* CLIPBOARD/DND API ***********/
HGLOBAL dataToHandle(void *data, int size) {
	auto handle = GlobalAlloc(GHND, size);
	auto ptr = GlobalLock(handle); // had a (wchar_t *) cast, not sure why
	memcpy(ptr, data, size);
	GlobalUnlock(ptr);
	return handle;
}

OPENWL_API const char *wl_kDragFormatUTF8 = "application/vnd.openwl-utf8"; // doesn't matter what these are on win32, we don't use them directly
OPENWL_API const char *wl_kDragFormatFiles = "application/vnd.openwl-files";

OPENWL_API wl_DragDataRef CDECL wl_DragDataCreate(wl_WindowRef window)
{
	return new wl_DragData(window);
}

OPENWL_API void CDECL wl_DragDataRelease(wl_DragDataRef *dragData)
{
	delete *dragData;
	*dragData = nullptr;
}

OPENWL_API void CDECL wl_DragAddFormat(wl_DragDataRef dragData, const char *dragFormatMIME)
{
	// we are capable of being a source of these formats
	// this should only be called one dragdata we're SENDING, because we're assuming a MyDataObject, not a base IDataObject given to us from outside
	dragData->sendObject->addDragFormat(dragFormatMIME);
}

OPENWL_API bool CDECL wl_DropHasFormat(wl_DropDataRef dropData, const char *dragFormatMIME)
{
	return dropData->hasFormat(dragFormatMIME);
}

OPENWL_API bool CDECL wl_DropGetFormat(wl_DropDataRef dropData, const char *dropFormatMIME, const void **data, size_t *dataSize)
{
	return dropData->getFormat(dropFormatMIME, data, dataSize);
}

//OPENWL_API bool CDECL wlDropGetText(wl_DropDataRef dropData, char *buffer, int maxLen)
//{
//	FORMATETC fmtetc = { CF_UNICODETEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
//	STGMEDIUM stgmed;
//	if (dropData->recvObject->GetData(&fmtetc, &stgmed) == S_OK) {
//		auto ptr = GlobalLock(stgmed.hGlobal);
//
//		auto utf8 = wstring_to_utf8((wchar_t *)ptr);
//		strcpy_s(buffer, maxLen, utf8.c_str());
//		//wcscpy_s(buffer, maxLen, (wchar_t *)ptr);
//
//		GlobalUnlock(stgmed.hGlobal);
//		ReleaseStgMedium(&stgmed);
//		
//		return true;
//	}
//	return false;
//}

OPENWL_API bool CDECL wl_DropGetFiles(wl_DropDataRef dropData, const struct wl_Files **files)
{
	return dropData->getFiles(files);
}

//OPENWL_API void CDECL wlDropFreeFiles(wl_Files *files)
//{
//	for (int i = 0; i < files->numFiles; i++) {
//		delete files->filenames[i];
//	}
//	delete files->filenames;
//}


OPENWL_API void CDECL wl_ClipboardSet(wl_DragDataRef dragData)
{
	OleSetClipboard(dragData->sendObject);
	//auto dataObject = mimeToDataObject(dragData);
	//if (dataObject) {
	//	OleSetClipboard(dataObject);
	//	OleFlushClipboard();
	//	dataObject->Release();
	//}
}

OPENWL_API wl_DropDataRef CDECL wl_ClipboardGet()
{
	IDataObject *obj;
	if (OleGetClipboard(&obj) == S_OK) {
		return new wl_DropData(obj);
	}
	return nullptr;
}

OPENWL_API void CDECL wl_ClipboardRelease(wl_DropDataRef dropData)
{
	delete dropData; // destructor releases contents
}

OPENWL_API void CDECL wl_ClipboardFlush()
{
	printf("flushing clipboard...\n");
	OleFlushClipboard();
}

OPENWL_API wl_DropEffect CDECL wl_DragExec(wl_DragDataRef dragData, unsigned int dropActionsMask, wl_Event *fromEvent)
{
	auto dataObject = dragData->sendObject;
	//auto dataObject = mimeToDataObject(dragData);
	IDropSource *dropSource;
	CreateDropSource(&dropSource);

	DWORD okEffects =
		((dropActionsMask & wl_kDropEffectCopy) ? DROPEFFECT_COPY : 0) |
		((dropActionsMask & wl_kDropEffectMove) ? DROPEFFECT_MOVE : 0) |
		((dropActionsMask & wl_kDropEffectLink) ? DROPEFFECT_LINK : 0);

	wl_DropEffect result;
	DWORD actualEffect;
	if (DoDragDrop(dataObject, dropSource, okEffects, &actualEffect) == DRAGDROP_S_DROP) {
		result = 
			(actualEffect == DROPEFFECT_COPY) ? wl_kDropEffectCopy :
			((actualEffect == DROPEFFECT_MOVE) ? wl_kDropEffectMove :
			((actualEffect == DROPEFFECT_LINK) ? wl_kDropEffectLink : wl_kDropEffectNone));
	}
	else {
		result = wl_kDropEffectNone;
	}
	// ownership based on effect??
	//dataObject->Release(); // let the wl_DragDataRef destructor take care of that, if need be
	dropSource->Release();
	return result;
}

void RegisterDropWindow(wl_WindowRef window, IDropTarget **ppDropTarget)
{
	auto pDropTarget = new MyDropTarget(window);

	// acquire a strong lock
	CoLockObjectExternal(pDropTarget, TRUE, FALSE);

	// tell OLE that the window is a drop target
	RegisterDragDrop(window->hwnd, pDropTarget);

	*ppDropTarget = pDropTarget;
}

void UnregisterDropWindow(wl_WindowRef window, IDropTarget *pDropTarget)
{
	// remove drag+drop
	RevokeDragDrop(window->hwnd);

	// remove the strong lock
	CoLockObjectExternal(pDropTarget, FALSE, TRUE);

	// release our own reference
	pDropTarget->Release();
}

OPENWL_API void wl_WindowEnableDrops(wl_WindowRef window, bool enabled)
{
	if (enabled) {
		RegisterDropWindow(window, &window->dropTarget);
	}
	else {
		UnregisterDropWindow(window, window->dropTarget);
		window->dropTarget = nullptr;
	}
}

// clip/drop data rendering
OPENWL_API void CDECL wl_DragRenderUTF8(wl_RenderPayloadRef payload, const char *text)
{
	// handle converting to the internal required clipboard format here (UTF-16)
	// that way the data is ready to go in CDataOboject::renderFormat without any special format checks
	auto wide = utf8_to_wstring(text);
	auto size = (wcslen(wide.c_str()) + 1) * sizeof(wchar_t);
	payload->data = malloc(size);
	memcpy(payload->data, wide.c_str(), size); // includes null at end
	payload->size = size;
}

OPENWL_API void CDECL wl_DragRenderFiles(wl_RenderPayloadRef payload, const struct wl_Files *files)
{
	// how ?
}

OPENWL_API void CDECL wl_DragRenderFormat(wl_RenderPayloadRef payload, const char *formatMIME, const void *data, size_t dataSize)
{
	payload->data = malloc(dataSize);
	memcpy(payload->data, data, dataSize);
	payload->size = dataSize;
}

/******* MISC STUFF ******/

static std::mutex execMutex;

void ExecuteMainItem(MainThreadExecItem *item) {
	std::lock_guard<std::mutex> lock(execMutex);
	//
	item->callback(item->data);
	item->execCond.notify_one();
}

OPENWL_API void CDECL wl_ExecuteOnMainThread(wl_WindowRef window, wl_VoidCallback callback, void *data)
{
	std::unique_lock<std::mutex> lock(execMutex);
	std::condition_variable cond;
	
	MainThreadExecItem item = { callback, data, cond };
	PostMessage(window->hwnd, WM_MainThreadExecMsg, 0, (LPARAM)&item); // safe to pass a pointer to item, because this function doesn't exit until it's done

	// block until it's done executing
	cond.wait(lock);
}

OPENWL_API void CDECL wl_Sleep(unsigned int millis)
{
	Sleep(millis);
}

OPENWL_API size_t CDECL wl_SystemMillis()
{
	// probably no need for the high res perf timer here
	return (size_t) GetTickCount64();
}

OPENWL_API void CDECL wl_MouseGrab(wl_WindowRef window)
{
	SetCapture(window->hwnd); // not saving the previous capture (return value) for now, maybe in the future
	lastGrabWindow = window;  // because ungrab / releasecapture are app-global, no specific handle given
}

OPENWL_API void CDECL wl_MouseUngrab()
{
	ReleaseCapture();
	lastGrabWindow->ignorePostGrabMove = true; // ignoring 1 unwanted move event, which can trigger unwanted redraws / state changes after a mouse up
}

OPENWL_API wl_CursorRef CDECL wl_CursorCreate(wl_CursorStyle style)
{
	auto found = cursorMap.find(style);
	if (found != cursorMap.end()) {
		return found->second;
	}
	else {
		LPWSTR name;
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

OPENWL_API void CDECL wl_WindowSetCursor(wl_WindowRef window, wl_CursorRef cursor)
{
	if (window->mouseInWindow) { // only makes sense to set when it's inside - gets reset when it leaves/re-enters anyway
		if (window->cursor != cursor) { // only set on change
			if (cursor) {
				// actually set
				SetCursor(cursor->handle);
				window->cursor = cursor;
			}
			else {
				// clear
				SetCursor(defaultCursor);
				window->cursor = nullptr;
			}
		}
	}
}
