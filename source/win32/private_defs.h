#pragma once

#include "../openwl.h"
#include "MyDataObject.h"

#include <string>
#include <vector>

struct _wlEventPrivate {
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    _wlEventPrivate(UINT message, WPARAM wParam, LPARAM lParam) :
        message(message), wParam(wParam), lParam(lParam)
    {
        //
    }
};

struct _wlWindow {
    HWND hwnd = NULL;
    WLWindowProperties props;
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
    ~_wlWindow() {
        if (d2dRenderTarget) {
            d2dRenderTarget->Release();
        }
    }
};
struct _wlIcon {
    HBITMAP hbitmap;
};
struct _wlTimer {
    wlWindow window;
    int timerID;
    HANDLE timerQueue;
    HANDLE handle;
    //
    LARGE_INTEGER lastPerfCount; // to calculate time since last firing
};

struct _wlMenuBar {
    HMENU hmenu;
};
struct _wlMenu {
    HMENU hmenu;
};
struct _wlMenuItem {
    wlAction action;
    wlMenu subMenu;
};

//static int nextActionID = 1001;
struct _wlAction {
    int id = -1;
    std::string label = "(none)";
    wlIcon icon = nullptr;
    wlAccelerator accel = nullptr;
    std::vector<wlMenuItem> attachedItems; // to update any menu items when this label/icon/etc changes
};

struct _wlAccelerator {
    WLKeyEnum key;
    unsigned int modifiers;
};

struct _wlRenderPayload {
    //char *text_utf8;
    void *data = nullptr;
    size_t size = 0;
    ~_wlRenderPayload() {
        if (data != nullptr) {
            free(data);
        }
    }
};

struct _wlDragData {
    //std::set<std::string> formats; // key is mime type
    MyDataObject *sendObject = 0;
    _wlDragData(wlWindow window) {
        sendObject = new MyDataObject(window);
        sendObject->AddRef();
    }
    ~_wlDragData() {
        printf("releasing sendobject ...\n");
        if (sendObject) sendObject->Release();
        printf("== wlDragData destructor ==\n");
    }
};

struct _wlFilesInternal;

struct _wlDropData {
    IDataObject *recvObject = 0;

	const void *data = nullptr;
	size_t dataSize = 0;
	_wlFilesInternal *files = nullptr;

// public methods =====================
    _wlDropData(IDataObject *dataObject) {
        //formats.clear();
        recvObject = dataObject;
        recvObject->AddRef();
    }
	~_wlDropData();

	bool hasFormat(const char *dragFormatMIME);
	bool getFormat(const char *dropFormatMIME, const void **outData, size_t *outSize);
	bool getFiles(const struct WLFiles **outFiles);
};
