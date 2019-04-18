#pragma once

#include "../openwl.h"
#include "MyDataObject.h"

#include <string>
#include <vector>

struct wl_EventPrivateImpl {
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    wl_EventPrivateImpl(UINT message, WPARAM wParam, LPARAM lParam) :
        message(message), wParam(wParam), lParam(lParam)
    {
        //
    }
};

struct wl_WindowImpl {
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
    ~wl_WindowImpl() {
        if (d2dRenderTarget) {
            d2dRenderTarget->Release();
        }
    }
};
struct wl_IconImpl {
    HBITMAP hbitmap;
};
struct wl_TimerImpl {
    wl_Window window;
    int timerID;
    HANDLE timerQueue;
    HANDLE handle;
    //
    LARGE_INTEGER lastPerfCount; // to calculate time since last firing
};

struct wl_MenuBarImpl {
    HMENU hmenu;
};
struct wl_MenuImpl {
    HMENU hmenu;
};
struct wl_MenuItemImpl {
    wl_Action action;
    wl_Menu subMenu;
};

//static int nextActionID = 1001;
struct wl_ActionImpl {
    int id = -1;
    std::string label = "(none)";
    wl_Icon icon = nullptr;
    wl_Accelerator accel = nullptr;
    std::vector<wl_MenuItem> attachedItems; // to update any menu items when this label/icon/etc changes
};

struct wl_AcceleratorImpl {
    wl_KeyEnum key;
    unsigned int modifiers;
};

struct wl_RenderPayloadImpl {
    //char *text_utf8;
    void *data = nullptr;
    size_t size = 0;
    ~wl_RenderPayloadImpl() {
        if (data != nullptr) {
            free(data);
        }
    }
};

struct wl_DragDataImpl {
    //std::set<std::string> formats; // key is mime type
    MyDataObject *sendObject = 0;
    wl_DragDataImpl(wl_Window window) {
        sendObject = new MyDataObject(window);
        sendObject->AddRef();
    }
    ~wl_DragDataImpl() {
        printf("releasing sendobject ...\n");
        if (sendObject) sendObject->Release();
        printf("== wl_DragData destructor ==\n");
    }
};

struct wl_FilesInternal : public wl_Files
{
	wl_FilesInternal(int numFiles)
	{
		this->numFiles = numFiles;
		filenames = new const char *[numFiles];
		for (int i = 0; i < numFiles; i++) {
			filenames[i] = nullptr;
		}
	}
	~wl_FilesInternal() {
		for (int i = 0; i < numFiles; i++) {
			free(const_cast<char *>(filenames[i])); // created with strdup
		}
		delete[] filenames;
	}
};

struct wl_DropDataImpl {
    IDataObject *recvObject = 0;

	const void *data = nullptr;
	size_t dataSize = 0;
	wl_FilesInternal *files = nullptr;

// public methods =====================
    wl_DropDataImpl(IDataObject *dataObject) {
        //formats.clear();
        recvObject = dataObject;
        recvObject->AddRef();
    }
	~wl_DropDataImpl();

	bool hasFormat(const char *dragFormatMIME);
	bool getFormat(const char *dropFormatMIME, const void **outData, size_t *outSize);
	bool getFiles(const struct wl_Files **outFiles);
};
