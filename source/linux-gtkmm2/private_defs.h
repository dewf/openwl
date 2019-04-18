//
// Created by dang on 2/11/18.
//

#ifndef C_CLIENT_PRIVATE_DEFS_H
#define C_CLIENT_PRIVATE_DEFS_H

#include "../openwl.h"
#include <gdkmm/event.h>
#include <gtkmm/image.h>
#include <gtkmm/menushell.h>
#include <gtkmm/menubar.h>
#include <gtkmm/menu.h>
#include <set>
#include "globals.h"

#include "_wlWindow.h"

struct _wl_EventPrivate {
    int eventCount = 0; // verify not reentrant
    GdkEvent *gdkEvent;
};

struct _wl_Icon {
    Gtk::Image *gtkImage;
};

struct _wl_Accelerator {
    wl_KeyEnum key;
    unsigned int modifiers;
};

struct _wl_Action {
    int id;
    std::string label;
    wl_Icon icon;
    wl_Accelerator accel;
    std::vector<wl_MenuItem> attachedItems; // for disabling multiple menu items at once, etc
};

struct _wl_Timer {
    wl_Window window;
    int timerID;
    sigc::connection conn;
    bool connected = false;
    timespec lastTime = { 0 }; // to provide secondsSinceLast for timer callbacks

    void disconnect() {
        if (connected) {
            conn.disconnect();
            connected = false;
            if (window) {
                window->removeTimer(this);
                window = nullptr;
            }
        }
    }

    virtual ~_wl_Timer() {
        disconnect();
    }
};

struct _wl_MenuShell {
    virtual wl_Window getAssociatedWindow() = 0;
    virtual Gtk::MenuShell &getShell() = 0;
};

struct _wl_MenuItem {
    _wl_MenuShell *parentShell;
    Gtk::MenuItem *gtkItem;
    wl_Action action;
    wl_Menu sub;
};

struct _wl_MenuBar : _wl_MenuShell {
    wl_Window attachedTo = nullptr;
    Gtk::MenuBar gtkMenuBar;

    Gtk::MenuShell &getShell() override { return gtkMenuBar; }

    wl_Window getAssociatedWindow() override {
        return attachedTo;
    }
};

struct _wl_Menu : _wl_MenuShell {
    Gtk::Menu gtkMenu;
    wl_MenuItem parentItem = nullptr;
    wl_Window contextFor = nullptr;

    Gtk::MenuShell &getShell() override { return gtkMenu; }

    wl_Window getAssociatedWindow() override {
        if (contextFor) {
            return contextFor;
        } else {
            if (parentItem && parentItem->parentShell) {
                return parentItem->parentShell->getAssociatedWindow();
            } else {
                return nullptr;
            }
        }
    }
    void on_item_activate(wl_MenuItem item) {
        auto window = getAssociatedWindow();
        if (window) {
            window->execAction(item->action);
        } else {
            printf("no associated window for menu\n");
        }
    }
};

struct _wl_DragData {
    wl_Window forWindow = nullptr;
    std::set<std::string> formats;
    bool dragActive = false;
};

struct _wl_FilesInternal : public wl_Files
{
    _wl_FilesInternal(int numFiles)
        :wl_Files()
    {
        this->numFiles = numFiles;
        filenames = new const char *[numFiles];
        for (int i=0; i< numFiles; i++) {
            filenames[i] = nullptr;
        }
    }
    ~_wl_FilesInternal() {
        for (int i=0; i< numFiles; i++) {
            free(const_cast<char *>(filenames[i])); // allocated w/ strdup
        }
        delete[] filenames;
    }
};

struct _wl_DropData {
    void *data = nullptr;
    size_t dataSize = 0;
    _wl_FilesInternal *files = nullptr;

    virtual ~_wl_DropData() {
        if (data) free(data);
        delete files; // apparently OK to delete null ptrs!
    }

    bool getFiles(const struct wl_Files **outFiles);

    virtual bool hasTarget(const char *target) = 0;
    virtual bool getFormat(const char *dropFormatMIME, const void **data, size_t *dataSize) = 0;
};

struct _wl_RenderPayload {
    void *data = nullptr;
    size_t dataSize = 0;

    ~_wl_RenderPayload() {
        if (data != nullptr) {
            free(data);
        }
    }
};

struct _wl_DropData_Drop : _wl_DropData {
    const Glib::RefPtr<Gdk::DragContext> &dragContext;
    wl_Window window;

// "public" ================================
    _wl_DropData_Drop(const Glib::RefPtr<Gdk::DragContext> &dragContext, wl_Window window)
            : dragContext(dragContext),
              window(window) {}

    //
    bool hasTarget(const char *target) override {
        auto targets = dragContext->get_targets();
        return std::find(targets.begin(), targets.end(), target) != targets.end();
    }

    bool getFormat(const char *dropFormatMIME, const void **outData, size_t *outSize) override {
        auto formatAtom = gdk_atom_intern(dropFormatMIME, false);
        window->getDropData(dragContext->gobj(), formatAtom, &data, &dataSize);
        // share data with caller - we still retain ownership
        *outData = data;
        *outSize = dataSize;
        return true;
    }
};


struct _wl_DropData_Clip : _wl_DropData {

// "public" =============
    bool hasTarget(const char *target) override {
        auto targets = gtkClipboard->wait_for_targets();
        return std::find(targets.begin(), targets.end(), std::string(target)) != targets.end();
    }

    bool getFormat(const char *dropFormatMIME, const void **outData, size_t *outSize) override {
        auto selection_data = gtkClipboard->wait_for_contents(dropFormatMIME);

        // tuck the data away for the API method that triggered the data fetch (wlDropGet...)
        dataSize = (size_t)selection_data.get_length();
        data = malloc(dataSize);
        memcpy(data, selection_data.get_data(), dataSize);

        // share with caller
        *outData = data;
        *outSize = dataSize;
        return true;
    }
};


#endif //C_CLIENT_PRIVATE_DEFS_H
