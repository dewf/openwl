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

struct _wlEventPrivate {
    int eventCount = 0; // verify not reentrant
    GdkEvent *gdkEvent;
};

struct _wlIcon {
    Gtk::Image *gtkImage;
};

struct _wlAccelerator {
    WLKeyEnum key;
    unsigned int modifiers;
};

struct _wlAction {
    int id;
    std::string label;
    wlIcon icon;
    wlAccelerator accel;
    std::vector<wlMenuItem> attachedItems; // for disabling multiple menu items at once, etc
};

struct _wlTimer {
    wlWindow window;
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

    virtual ~_wlTimer() {
        disconnect();
    }
};

struct _wlMenuShell {
    virtual wlWindow getAssociatedWindow() = 0;
    virtual Gtk::MenuShell &getShell() = 0;
};

struct _wlMenuItem {
    _wlMenuShell *parentShell;
    Gtk::MenuItem *gtkItem;
    wlAction action;
    wlMenu sub;
};

struct _wlMenuBar : _wlMenuShell {
    wlWindow attachedTo = nullptr;
    Gtk::MenuBar gtkMenuBar;

    Gtk::MenuShell &getShell() override { return gtkMenuBar; }

    wlWindow getAssociatedWindow() override {
        return attachedTo;
    }
};

struct _wlMenu : _wlMenuShell {
    Gtk::Menu gtkMenu;
    wlMenuItem parentItem = nullptr;
    wlWindow contextFor = nullptr;

    Gtk::MenuShell &getShell() override { return gtkMenu; }

    wlWindow getAssociatedWindow() override {
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
    void on_item_activate(wlMenuItem item) {
        auto window = getAssociatedWindow();
        if (window) {
            window->execAction(item->action);
        } else {
            printf("no associated window for menu\n");
        }
    }
};

struct _wlDragData {
    wlWindow forWindow = nullptr;
    std::set<std::string> formats;
    bool dragActive = false;
};

struct _wlFilesInternal : public WLFiles
{
    _wlFilesInternal(int numFiles)
        :WLFiles()
    {
        this->numFiles = numFiles;
        filenames = new const char *[numFiles];
        for (int i=0; i< numFiles; i++) {
            filenames[i] = nullptr;
        }
    }
    ~_wlFilesInternal() {
        for (int i=0; i< numFiles; i++) {
            free(const_cast<char *>(filenames[i])); // allocated w/ strdup
        }
        delete[] filenames;
    }
};

struct _wlDropData {
    void *data = nullptr;
    size_t dataSize = 0;
    _wlFilesInternal *files = nullptr;

    virtual ~_wlDropData() {
        if (data) free(data);
        delete files; // apparently OK to delete null ptrs!
    }

    bool getFiles(const struct WLFiles **outFiles);

    virtual bool hasTarget(const char *target) = 0;
    virtual bool getFormat(const char *dropFormatMIME, const void **data, size_t *dataSize) = 0;
};

struct _wlRenderPayload {
    void *data = nullptr;
    size_t dataSize = 0;

    ~_wlRenderPayload() {
        if (data != nullptr) {
            free(data);
        }
    }
};

struct _wlDropData_Drop : _wlDropData {
    const Glib::RefPtr<Gdk::DragContext> &dragContext;
    wlWindow window;

// "public" ================================
    _wlDropData_Drop(const Glib::RefPtr<Gdk::DragContext> &dragContext, wlWindow window)
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


struct _wlDropData_Clip : _wlDropData {

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
