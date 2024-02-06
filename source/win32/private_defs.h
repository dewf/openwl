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

    wl_DragData(wl_DragRenderDelegate renderDelegate) {
        sendObject = new MyDataObject(renderDelegate); // render delegate will be owned (and thus released) by the data object, because we don't know how long it will live (refcounted COM object)
        sendObject->AddRef();
    }

    ~wl_DragData() {
        printf("== wl_DragDataRef destructor ==\n");
        printf(" - releasing sendobject ...\n");
        if (sendObject) sendObject->Release();
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
