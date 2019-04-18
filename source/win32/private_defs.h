#pragma once

#include "../openwl.h"
#include "MyDataObject.h"

#include <string>
#include <vector>

struct _wl_EventPrivate {
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    _wl_EventPrivate(UINT message, WPARAM wParam, LPARAM lParam) :
        message(message), wParam(wParam), lParam(lParam)
    {
        //
    }
};

struct _wl_Window {
    HWND hwnd = NULL;
    wl_WindowProperties props;
    void *userData = nullptr; // for client callback on window events
    IDropTarget *dropTarget = nullptr; // for dnd
                                       // useful stuff to know for various API calls
    DWORD dwStyle = 0;
    BOOL hasMenu = FALSE;

    int clientWidth = -1;
    int clientHeight = -1;
    int extraWidth = -1; // difference between client size and window size
    int extraHeight = -1;

	bool mouseInWindow = false;

    // for D2D only
    ID2D1HwndRenderTarget *d2dRenderTarget = nullptr;

    // destructor
    ~_wl_Window() {
        if (d2dRenderTarget) {
            d2dRenderTarget->Release();
        }
    }
};
struct _wl_Icon {
    HBITMAP hbitmap;
};
struct _wl_Timer {
    wl_Window window;
    int timerID;
    HANDLE timerQueue;
    HANDLE handle;
    //
    LARGE_INTEGER lastPerfCount; // to calculate time since last firing
};

struct _wl_MenuBar {
    HMENU hmenu;
};
struct _wl_Menu {
    HMENU hmenu;
};
struct _wl_MenuItem {
    wl_Action action;
    wl_Menu subMenu;
};

//static int nextActionID = 1001;
struct _wl_Action {
    int id = -1;
    std::string label = "(none)";
    wl_Icon icon = nullptr;
    wl_Accelerator accel = nullptr;
    std::vector<wl_MenuItem> attachedItems; // to update any menu items when this label/icon/etc changes
};

struct _wl_Accelerator {
    wl_KeyEnum key;
    unsigned int modifiers;
};

struct _wl_RenderPayload {
    //char *text_utf8;
    void *data = nullptr;
    size_t size = 0;
    ~_wl_RenderPayload() {
        if (data != nullptr) {
            free(data);
        }
    }
};

struct _wl_DragData {
    //std::set<std::string> formats; // key is mime type
    MyDataObject *sendObject = 0;
    _wl_DragData(wl_Window window) {
        sendObject = new MyDataObject(window);
        sendObject->AddRef();
    }
    ~_wl_DragData() {
        printf("releasing sendobject ...\n");
        if (sendObject) sendObject->Release();
        printf("== wl_DragData destructor ==\n");
    }
};

struct _wl_FilesInternal;

struct _wl_DropData {
    IDataObject *recvObject = 0;

	const void *data = nullptr;
	size_t dataSize = 0;
	_wl_FilesInternal *files = nullptr;

// public methods =====================
    _wl_DropData(IDataObject *dataObject) {
        //formats.clear();
        recvObject = dataObject;
        recvObject->AddRef();
    }
	~_wl_DropData();

	bool hasFormat(const char *dragFormatMIME);
	bool getFormat(const char *dropFormatMIME, const void **outData, size_t *outSize);
	bool getFiles(const struct wl_Files **outFiles);
};
