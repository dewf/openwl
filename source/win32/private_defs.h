#pragma once

#include "../openwl.h"
#include "MyDataObject.h"

struct wl_EventPrivate {
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    wl_EventPrivate(UINT message, WPARAM wParam, LPARAM lParam) :
        message(message), wParam(wParam), lParam(lParam)
    {
        //
    }
};

struct wl_Icon {
    HBITMAP hbitmap;
};

struct wl_Accelerator {
    wl_KeyEnum key;
    unsigned int modifiers;
};

struct wl_MenuBar {
    HMENU hmenu;
};
struct wl_Menu {
    HMENU hmenu;
};
struct wl_MenuItem {
    wl_ActionRef action;
    wl_MenuRef subMenu;
};

struct wl_RenderPayload {
    //char *text_utf8;
    void* data = nullptr;
    size_t size = 0;
    ~wl_RenderPayload() {
        if (data != nullptr) {
            free(data);
        }
    }
};

struct wl_DragData {
    //std::set<std::string> formats; // key is mime type
    MyDataObject* sendObject = 0;
    wl_DragData(wl_WindowRef window) {
        sendObject = new MyDataObject(window);
        sendObject->AddRef();
    }
    ~wl_DragData() {
        printf("releasing sendobject ...\n");
        if (sendObject) sendObject->Release();
        printf("== wl_DragDataRef destructor ==\n");
    }
};

struct wl_FilesInternal : public wl_Files
{
    wl_FilesInternal(int numFiles)
    {
        this->numFiles = numFiles;
        filenames = new const char* [numFiles];
        for (int i = 0; i < numFiles; i++) {
            filenames[i] = nullptr;
        }
    }
    ~wl_FilesInternal() {
        for (int i = 0; i < numFiles; i++) {
            free(const_cast<char*>(filenames[i])); // created with strdup
        }
        delete[] filenames;
    }
};

struct wl_DropData {
    IDataObject* recvObject = 0;

    void* tempData = nullptr;
    size_t tempSize = 0;

    wl_FilesInternal* files = nullptr;

    // public methods =====================
    wl_DropData(IDataObject* dataObject) {
        //formats.clear();
        recvObject = dataObject;
        recvObject->AddRef();
    }
    ~wl_DropData();

    bool hasFormat(const char* dragFormatMIME);
    bool getFormat(const char* dropFormatMIME, const void** outData, size_t* outSize);
    bool getFiles(const struct wl_Files** outFiles);
};
