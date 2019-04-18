//
// Created by dang on 2/11/18.
//

#ifndef C_CLIENT_WLWINDOW_H
#define C_CLIENT_WLWINDOW_H

#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/drawingarea.h>
#include <openwl.h>
#include <set>
#include "globals.h"

struct _wl_Window : public Gtk::Window {
private:
    wl_WindowProperties props = {0}; // save these because they need to be reapplied when a menu is added

    int width = -1, height = -1;
    Gtk::VBox vbox;
    Gtk::DrawingArea drawArea;
    void *userData;
    //_wlEventPrivate _private;
    //wl_Event wl_Event;
    std::set<wl_Timer> timers;

    wl_MenuBar attachedMenuBar = nullptr;
    int menuHeight = -1;

    bool dragActive = false;

    // temporary storage for DnD data -- for use with getDragData/releaseDragData
    void **dragData = nullptr;
    size_t *dragDataSize = nullptr;

    bool dragDataAwait = false; // to make getDropData synchronous

    inline void dispatchEvent(wl_Event *wl_Event) {
        wl_Event->handled = false;
        eventCallback(this, wl_Event, userData);
    }

    // common for drag source / clipboard set
    void dragClipRender(Gtk::SelectionData &selectionData, guint info);

public:
    Gtk::DrawingArea *getDrawArea() { return &drawArea; }

    void insertTimer(wl_Timer timer) {
        timers.insert(timer);
    }
    void removeTimer(wl_Timer timer) {
        timers.erase(timer);
    }

    _wl_Window(void *userData, wl_WindowProperties *props);
    virtual ~_wl_Window();

    /**** callbacks ****/
    bool on_drawArea_expose(GdkEventExpose *gdkEvent);
    void on_drawArea_allocate(Gtk::Allocation &allocation);
    bool on_drawArea_buttonPress(GdkEventButton *gdkEvent);
    bool on_drawArea_buttonRelease(GdkEventButton *gdkEvent);
    bool on_drawArea_keyPress(GdkEventKey *gdkEvent);
    bool on_drawArea_keyRelease(GdkEventKey *gdkEvent);
    bool on_drawArea_mouseMotion(GdkEventMotion *gdkEvent);
    bool on_drawArea_enterNotify(GdkEventCrossing *gdkEvent);
    bool on_drawArea_leaveNotify(GdkEventCrossing *gdkEvent);

    // to get menu size (to adjust client area to requested size [and min/max sizes])
    void on_menuBar_allocate(Gtk::Allocation &allocation);

    // drag source handlers
    void on_drawArea_dragBegin(const Glib::RefPtr<Gdk::DragContext>& context);
    void on_drawArea_dragDataGet(const Glib::RefPtr<Gdk::DragContext>& dragContext, Gtk::SelectionData& selectionData, guint info, guint time);
    void on_drawArea_dragEnd(const Glib::RefPtr<Gdk::DragContext>& context);
    void on_drawArea_dragDataDelete(const Glib::RefPtr<Gdk::DragContext>& context);
    bool on_drawArea_dragMotion(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time);
    bool on_drawArea_dragDrop(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time);
    void on_drawArea_dragLeave(const Glib::RefPtr<Gdk::DragContext>& context, guint time);
    void on_drawArea_dragDataReceived(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time);
    bool on_timer_timeout(wl_Timer timer);
    bool on_delete(GdkEventAny *gdkEvent);
    void on_clipboard_get(Gtk::SelectionData& selectionData, guint info);
    void on_clipboard_clear();

    /**** public API *****/
    void invalidate(int x, int y, int width, int height);
    void setFocus();
    void setMenuBar(wl_MenuBar menuBar);
    void execAction(wl_Action action);

    bool getDragActive();
    void getDropData(GdkDragContext *dragContext, GdkAtom formatAtom, void **data, size_t *size);
    void releaseDropData(void **dragData);
    void setClipboard(wl_DragData dragData);

    size_t getWindowHandle();
    void mouseGrab();

    void relativeToAbsolute(int x, int y, int *absX, int *absY);
};

#endif //C_CLIENT_WLWINDOW_H
