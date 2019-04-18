#include "_wlWindow.h"
#include "private_defs.h"
#include "util.h"
#include "keystuff.h"

#include <gdk/gdkx.h>

struct EventFrame {
    _wlEventPrivate _priv;
    wl_Event wl_Event;
    EventFrame(GdkEvent *gdkEvent) {
        wl_Event._private = &_priv;
        _priv.gdkEvent = gdkEvent;
    }
};

void _wlWindow::dragClipRender(Gtk::SelectionData &selectionData, guint info) {
    EventFrame ef(nullptr);
    auto wl_Event = &ef.wl_Event;
    wl_Event->eventType = wl_kEventTypeDragRender;

    wl_Event->dragRenderEvent.dragFormat = selectionData.get_target().c_str(); //(WLDragFormatEnum)info

    auto payload = new _wlRenderPayload;
    wl_Event->dragRenderEvent.payload = payload;

    dispatchEvent(wl_Event);

    if (wl_Event->handled) {
        selectionData.set(8, (guint8 *)payload->data, (int)payload->dataSize);
        printf("selectiondata set\n");
    } else {
        printf("drag/clip render was unhandled, nothing to do/set\n");
    }
    delete payload;
}

void setGeometryHints(Gtk::Window *gtkWin, wl_WindowProperties *props, int menuHeight)
{
    Gdk::Geometry geom = {0};
    auto mask = (Gdk::WindowHints)0;
    if (props->usedFields & wl_kWindowPropMinWidth) {
        geom.min_width = props->minWidth;
        mask |= Gdk::HINT_MIN_SIZE;
    }
    if (props->usedFields & wl_kWindowPropMinHeight) {
        geom.min_height = props->minHeight + menuHeight;
        mask |= Gdk::HINT_MIN_SIZE;
    }
    if (props->usedFields & wl_kWindowPropMaxWidth) {
        geom.max_width = props->maxWidth;
        mask |= Gdk::HINT_MAX_SIZE;
    }
    if (props->usedFields & wl_kWindowPropMaxHeight) {
        geom.max_height = props->maxHeight + menuHeight;
        mask |= Gdk::HINT_MAX_SIZE;
    }
    gtkWin->set_geometry_hints(*gtkWin, geom, mask);
}

Gtk::WindowType getWindowType(wl_WindowProperties *props) {
    if (props && (props->usedFields & wl_kWindowPropStyle) && props->style == wl_kWindowStyleFrameless) {
        return Gtk::WINDOW_POPUP;
    } else {
        return Gtk::WINDOW_TOPLEVEL;
    }
}

_wlWindow::_wlWindow(void *userData, wl_WindowProperties *props)
        : Window(getWindowType(props)),
          userData(userData)
{
    // save props for later reapplication (when/if menubar added)
    if (props) {
        this->props = *props;
        setGeometryHints(this, props, 0);
    }

    // GTK stuff below
    signal_delete_event().connect(sigc::mem_fun1(*this, &_wlWindow::on_delete));

    // vertical box for menu bar + drawing area
    add(vbox);
    vbox.pack_end(drawArea, true, true, 0);

    drawArea.set_can_focus(true);

    drawArea.signal_expose_event().connect(sigc::mem_fun1(*this, &_wlWindow::on_drawArea_expose));
    drawArea.signal_size_allocate().connect(sigc::mem_fun1(*this, &_wlWindow::on_drawArea_allocate));

    drawArea.add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
                        Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK |
                        Gdk::BUTTON_MOTION_MASK |
                        Gdk::POINTER_MOTION_MASK | // do we not want this? only when explicitly asked for by API client?
                        Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK |
                        Gdk::FOCUS_CHANGE_MASK);
    //        drawArea.add_events(Gdk::ALL_EVENTS_MASK);

    drawArea.signal_button_press_event().connect(sigc::mem_fun1(*this, &_wlWindow::on_drawArea_buttonPress));
    drawArea.signal_button_release_event().connect(sigc::mem_fun1(*this, &_wlWindow::on_drawArea_buttonRelease));
    drawArea.signal_key_press_event().connect(sigc::mem_fun1(*this, &_wlWindow::on_drawArea_keyPress));
    drawArea.signal_key_release_event().connect(sigc::mem_fun1(*this, &_wlWindow::on_drawArea_keyRelease));
    drawArea.signal_motion_notify_event().connect(sigc::mem_fun1(*this, &_wlWindow::on_drawArea_mouseMotion));
    drawArea.signal_enter_notify_event().connect(sigc::mem_fun1(*this, &_wlWindow::on_drawArea_enterNotify));
    drawArea.signal_leave_notify_event().connect(sigc::mem_fun1(*this, &_wlWindow::on_drawArea_leaveNotify));

    // dnd, ughhh
    drawArea.signal_drag_begin().connect(sigc::mem_fun1(*this, &_wlWindow::on_drawArea_dragBegin));
    drawArea.signal_drag_motion().connect(sigc::mem_fun4(*this, &_wlWindow::on_drawArea_dragMotion));
    drawArea.signal_drag_data_get().connect(sigc::mem_fun4(*this, &_wlWindow::on_drawArea_dragDataGet));
    drawArea.signal_drag_drop().connect(sigc::mem_fun4(*this, &_wlWindow::on_drawArea_dragDrop));
    drawArea.signal_drag_end().connect(sigc::mem_fun1(*this, &_wlWindow::on_drawArea_dragEnd));
    drawArea.signal_drag_data_received().connect(sigc::mem_fun6(*this, &_wlWindow::on_drawArea_dragDataReceived));
    drawArea.signal_drag_data_delete().connect(sigc::mem_fun1(*this, &_wlWindow::on_drawArea_dragDataDelete));
    drawArea.signal_drag_leave().connect(sigc::mem_fun2(*this, &_wlWindow::on_drawArea_dragLeave));

    //        EXPOSURE_MASK = 1 << 1,
    //        POINTER_MOTION_MASK = 1 << 2,
    //        POINTER_MOTION_HINT_MASK = 1 << 3,
    //        BUTTON_MOTION_MASK = 1 << 4,
    //        BUTTON1_MOTION_MASK = 1 << 5,
    //        BUTTON2_MOTION_MASK = 1 << 6,
    //        BUTTON3_MOTION_MASK = 1 << 7,
    //        BUTTON_PRESS_MASK = 1 << 8,
    //        BUTTON_RELEASE_MASK = 1 << 9,
    //        KEY_PRESS_MASK = 1 << 10,
    //        KEY_RELEASE_MASK = 1 << 11,
    //        ENTER_NOTIFY_MASK = 1 << 12,
    //        LEAVE_NOTIFY_MASK = 1 << 13,
    //        FOCUS_CHANGE_MASK = 1 << 14,
    //        STRUCTURE_MASK = 1 << 15,
    //        PROPERTY_CHANGE_MASK = 1 << 16,
    //        VISIBILITY_NOTIFY_MASK = 1 << 17,
    //        PROXIMITY_IN_MASK = 1 << 18,
    //        PROXIMITY_OUT_MASK = 1 << 19,
    //        SUBSTRUCTURE_MASK = 1 << 20,
    //        SCROLL_MASK = 1 << 21,
    //        ALL_EVENTS_MASK = 0x3FFFFE
}

_wlWindow::~_wlWindow() {
    printf("wl_Window destruct\n");
    // stop all associated timers
    // copy set so removal doesn't mess with iterator
    auto set_copy = timers;
    for (auto timer : set_copy) {
        timer->disconnect();
        // but don't delete, in case the API client still has a handle to it
    }

    EventFrame ef(nullptr);
    auto wl_Event = &ef.wl_Event;
    wl_Event->eventType = wl_kEventTypeWindowDestroyed;
    wl_Event->destroyEvent.reserved = 0;
    dispatchEvent(wl_Event);
}

/**** callbacks ****/
bool _wlWindow::on_drawArea_expose(GdkEventExpose *gdkEvent) {
    EventFrame ef((GdkEvent *)gdkEvent);
    auto wl_Event = &ef.wl_Event;
    wl_Event->eventType = wl_kEventTypeWindowRepaint;
    auto context = drawArea.get_window()->create_cairo_context();
    wl_Event->repaintEvent.platformContext = context->cobj(); // cairo_t *
    wl_Event->repaintEvent.x = gdkEvent->area.x;
    wl_Event->repaintEvent.y = gdkEvent->area.y;
    wl_Event->repaintEvent.width = gdkEvent->area.width;
    wl_Event->repaintEvent.height = gdkEvent->area.height;
    dispatchEvent(wl_Event);
    return wl_Event->handled;
//        auto width = drawArea.get_allocation().get_width();
//        auto height = drawArea.get_allocation().get_height();
//        printf("alloc w/h: %d/%d\n", width, height);
}

void _wlWindow::on_drawArea_allocate(Gtk::Allocation &allocation) {
    if (allocation.get_width() != width || allocation.get_height() != height) {
        EventFrame ef(nullptr);
        auto wl_Event = &ef.wl_Event;
        wl_Event->eventType = wl_kEventTypeWindowResized;
        wl_Event->resizeEvent.newWidth = allocation.get_width();
        wl_Event->resizeEvent.newHeight = allocation.get_height();
        wl_Event->resizeEvent.oldWidth = width;
        wl_Event->resizeEvent.oldHeight = height;
        dispatchEvent(wl_Event);
        width = allocation.get_width();
        height = allocation.get_height();
    }
}

bool _wlWindow::on_drawArea_buttonPress(GdkEventButton *gdkEvent) {
    EventFrame ef((GdkEvent *)gdkEvent);
    auto wl_Event = &ef.wl_Event;
    wl_Event->eventType = wl_kEventTypeMouse;
    wl_Event->mouseEvent.eventType = wl_kMouseEventTypeMouseDown;
    wl_Event->mouseEvent.x = (int)gdkEvent->x;
    wl_Event->mouseEvent.y = (int)gdkEvent->y;
    wl_Event->mouseEvent.modifiers = gdkToWlModifiers(gdkEvent->state);
    wl_Event->mouseEvent.button = gdkToWlButton(gdkEvent->button);
    dispatchEvent(wl_Event);
    return wl_Event->handled;
}


bool _wlWindow::on_drawArea_buttonRelease(GdkEventButton *gdkEvent) {
    EventFrame ef((GdkEvent *)gdkEvent);
    auto wl_Event = &ef.wl_Event;
    wl_Event->eventType = wl_kEventTypeMouse;
    wl_Event->mouseEvent.eventType = wl_kMouseEventTypeMouseUp;
    wl_Event->mouseEvent.x = (int)gdkEvent->x;
    wl_Event->mouseEvent.y = (int)gdkEvent->y;
    wl_Event->mouseEvent.modifiers = gdkToWlModifiers(gdkEvent->state);
    wl_Event->mouseEvent.button = gdkToWlButton(gdkEvent->button);
    dispatchEvent(wl_Event);
    return wl_Event->handled;
}

bool _wlWindow::on_drawArea_keyPress(GdkEventKey *gdkEvent) {
    EventFrame ef((GdkEvent *)gdkEvent);
    auto wl_Event = &ef.wl_Event;
    wl_Event->eventType = wl_kEventTypeKey;
    wl_Event->keyEvent.key = wl_kKeyUnknown;
    wl_Event->keyEvent.modifiers = gdkToWlModifiers(gdkEvent->state);
    wl_Event->keyEvent.location = wl_kKeyLocationDefault;

    auto keyHandled = false;
    auto charHandled = false;
    bool suppressChar = false;

    // key event, if any
    auto found = keyMap.find(gdkEvent->keyval);
    if (found != keyMap.end()) {
        auto info = found->second;
        wl_Event->keyEvent.eventType = wl_kKeyEventTypeDown;
        wl_Event->keyEvent.key = info->key;
        wl_Event->keyEvent.string = info->stringRep;
        wl_Event->keyEvent.location = info->location;

        dispatchEvent(wl_Event);

        keyHandled = wl_Event->handled;
        suppressChar = info->suppressChar;
    }

    // character event, if any
    if (gdkEvent->string
        && strlen(gdkEvent->string) > 0
        && !suppressChar) // can be suppressed if it's something we don't want characters for (return, esc, etc)
    {
        wl_Event->keyEvent.eventType = wl_kKeyEventTypeChar;
        wl_Event->keyEvent.string = gdkEvent->string;

        dispatchEvent(wl_Event);

        charHandled = wl_Event->handled;
    }
    return keyHandled || charHandled;
}

bool _wlWindow::on_drawArea_keyRelease(GdkEventKey *gdkEvent) {
    auto found = keyMap.find(gdkEvent->keyval);
    if (found != keyMap.end()) {
        auto info = found->second;
        EventFrame ef((GdkEvent *)gdkEvent);
        auto wl_Event = &ef.wl_Event;
        wl_Event->eventType = wl_kEventTypeKey;
        wl_Event->keyEvent.eventType = wl_kKeyEventTypeUp;
        wl_Event->keyEvent.modifiers = gdkToWlModifiers(gdkEvent->state);
        wl_Event->keyEvent.location = info->location;
        wl_Event->keyEvent.key = info->key;
        wl_Event->keyEvent.string = info->stringRep;
        dispatchEvent(wl_Event);
        return wl_Event->handled;
    }
    return false; // definitely unhandled
}

bool _wlWindow::on_drawArea_mouseMotion(GdkEventMotion *gdkEvent) {
    EventFrame ef((GdkEvent *)gdkEvent);
    auto wl_Event = &ef.wl_Event;
    wl_Event->eventType = wl_kEventTypeMouse;
    wl_Event->mouseEvent.eventType = wl_kMouseEventTypeMouseMove;
    wl_Event->mouseEvent.button = wl_kMouseButtonNone;
    wl_Event->mouseEvent.modifiers = gdkToWlModifiers(gdkEvent->state);
    wl_Event->mouseEvent.x = (int)gdkEvent->x;
    wl_Event->mouseEvent.y = (int)gdkEvent->y;
    dispatchEvent(wl_Event);
    return wl_Event->handled;
}

bool _wlWindow::on_drawArea_enterNotify(GdkEventCrossing *gdkEvent) {
    EventFrame ef((GdkEvent *)gdkEvent);
    auto wl_Event = &ef.wl_Event;
    wl_Event->eventType = wl_kEventTypeMouse;
    wl_Event->mouseEvent.eventType = wl_kMouseEventTypeMouseEnter;
    wl_Event->mouseEvent.button = wl_kMouseButtonNone;
    wl_Event->mouseEvent.modifiers = gdkToWlModifiers(gdkEvent->state);
    wl_Event->mouseEvent.x = (int)gdkEvent->x;
    wl_Event->mouseEvent.y = (int)gdkEvent->y;
    dispatchEvent(wl_Event);
    return wl_Event->handled;
}

bool _wlWindow::on_drawArea_leaveNotify(GdkEventCrossing *gdkEvent) {
    EventFrame ef((GdkEvent *)gdkEvent);
    auto wl_Event = &ef.wl_Event;
    wl_Event->eventType = wl_kEventTypeMouse;
    wl_Event->mouseEvent.eventType = wl_kMouseEventTypeMouseLeave;
    wl_Event->mouseEvent.button = wl_kMouseButtonNone;
    wl_Event->mouseEvent.modifiers = gdkToWlModifiers(gdkEvent->state);
    wl_Event->mouseEvent.x = (int)gdkEvent->x;
    wl_Event->mouseEvent.y = (int)gdkEvent->y;
    dispatchEvent(wl_Event);
    return wl_Event->handled;
}


// drag source handlers
void _wlWindow::on_drawArea_dragBegin(const Glib::RefPtr<Gdk::DragContext>& context) {
    // on drag source
    printf("=== drag begin\n");
    dragActive = true;
}
void _wlWindow::on_drawArea_dragDataGet(const Glib::RefPtr<Gdk::DragContext>& dragContext, Gtk::SelectionData& selectionData, guint info, guint time) {
    // on drag source, set the data
    // Gtk::SelectionData::set(), _set_text(), etc ...
    printf("{{{*** drag data get }}}\n");

    dragClipRender(selectionData, info);
}
void _wlWindow::on_drawArea_dragEnd(const Glib::RefPtr<Gdk::DragContext>& context) {
    // emitted on source when drag is finished
    // undo whatever was done in drag_begin
    printf("=== drag end\n");
    dragActive = false;
}
void _wlWindow::on_drawArea_dragDataDelete(const Glib::RefPtr<Gdk::DragContext>& context) {
    printf("=== drag data delete (move complete)\n");
}

// drop site handlers
bool _wlWindow::on_drawArea_dragMotion(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time) {
    _wlDropData_Drop dropData(context, this);

    // emitted on drop site
    printf("=== drag motion\n");
    EventFrame ev(nullptr);
    auto wl_Event = &ev.wl_Event;
    wl_Event->eventType = wl_kEventTypeDrop;
    wl_Event->dropEvent.eventType = wl_kDropEventTypeFeedback;
    wl_Event->dropEvent.x = x;
    wl_Event->dropEvent.y = y;
    wl_Event->dropEvent.data = &dropData;

    //        printf("suggested: %d\n", context->get_suggested_action());
    //        printf("default action is: %d\n", wl_Event->dropEvent.defaultModifierAction);
    //        printf("actions: %d\n", context->get_actions());

    // set default modifier action to whatever's normal/suggested for this modifier combination (ctrl/alt/shift/etc)
    wl_Event->dropEvent.defaultModifierAction = gdkToWlDropEffectSingle(context->get_suggested_action());

    // set allowed effect mask to all the ones allowed/possible
    wl_Event->dropEvent.allowedEffectMask = gdkToWlDropEffectMulti(context->get_actions());

    dispatchEvent(wl_Event);

    // final verdict?
    auto dropAction = wlToGdkDropEffectMulti(wl_Event->dropEvent.allowedEffectMask);
    context->drag_status(dropAction, gtk_get_current_event_time());

    return (wl_Event->dropEvent.allowedEffectMask != wl_kDropEffectNone);
}

bool _wlWindow::on_drawArea_dragDrop(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time)
{
    _wlDropData_Drop dropData(context, this);

    printf("=== drag drop\n");
    // emitted on drop site
    // true if cursor position in a drop zone
    // must also call gtk_drag_finish to let source know drop is done (edit: that doesn't happen until dragDataReceived)

    EventFrame ef(nullptr);
    auto wl_Event = &ef.wl_Event;
    wl_Event->eventType = wl_kEventTypeDrop;
    wl_Event->dropEvent.eventType = wl_kDropEventTypeDrop;
    wl_Event->dropEvent.data = &dropData;
    printf("before dragDrop dispatchEvent\n");
    dispatchEvent(wl_Event);
    printf("back from dragDrop dispatchEvent\n");

    context->drop_finish(true, gtk_get_current_event_time());
    return true; // always return true because we wouldn't have gotten this far unless cursor was in a drop zone already, right?
}

void _wlWindow::on_drawArea_dragLeave(const Glib::RefPtr<Gdk::DragContext>& context, guint time) {
    printf("=== drag leave\n");
}

void _wlWindow::on_drawArea_dragDataReceived(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time) {
    // emitted on drop site once data received
    printf("=== drag data received\n");

    printf("selectionData format: %d / size: %d\n", selection_data.get_format(), selection_data.get_length());

    // tuck the data away for the API method that triggered the data fetch (wlDropGet...)
    *dragDataSize = (size_t)selection_data.get_length();
    *dragData = malloc(*dragDataSize);
    memcpy(*dragData, selection_data.get_data(), *dragDataSize);
    printf("data saved\n");

    auto should_del = context->get_selected_action() == Gdk::ACTION_MOVE;
    printf("should_del: %s\n", should_del ? "True" : "False");
    context->drag_finish(true, should_del, gtk_get_current_event_time());

    printf("dragDataReceived end\n");
    dragDataAwait = false;
}

bool _wlWindow::on_timer_timeout(wl_Timer timer) {
    EventFrame ef(nullptr);
    auto wl_Event = &ef.wl_Event;
    wl_Event->eventType = wl_kEventTypeTimer;
    wl_Event->timerEvent.timer = timer;
    wl_Event->timerEvent.timerID = timer->timerID;
    wl_Event->timerEvent.stopTimer = false;

    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    wl_Event->timerEvent.secondsSinceLast = timespecDiff(now, timer->lastTime);

    dispatchEvent(wl_Event);

    timer->lastTime = now;

    if (wl_Event->timerEvent.stopTimer) {
        return false; // disconnect (but don't delete)
    }
    return true; // continue by default
}

bool _wlWindow::on_delete(GdkEventAny *gdkEvent) {
    EventFrame ef((GdkEvent *)gdkEvent);
    auto wl_Event = &ef.wl_Event;
    wl_Event->eventType = wl_kEventTypeWindowCloseRequest;
    wl_Event->closeRequestEvent.cancelClose = false;
    dispatchEvent(wl_Event);
    if (!wl_Event->closeRequestEvent.cancelClose) {
        delete this; // ??
    }
    return wl_Event->closeRequestEvent.cancelClose;
}

void _wlWindow::on_clipboard_get(Gtk::SelectionData& selectionData, guint info)
{
    printf("on_clipboard_get (info %d)\n", info);
    dragClipRender(selectionData, info);
}

void _wlWindow::on_clipboard_clear()
{
    printf("on_clipboard_clear\n");
    EventFrame ef(nullptr);
    auto wl_Event = &ef.wl_Event;
    wl_Event->eventType = wl_kEventTypeClipboardClear;
    wl_Event->clipboardClearEvent.reserved = 0;
    dispatchEvent(wl_Event);
}

/**** public API *****/
//    void destroy() {
//        innerDestroy(nullptr, false);
//    }

void _wlWindow::invalidate(int x, int y, int width, int height) {
    if (width > 0 && height > 0) {
        drawArea.queue_draw_area(x, y, width, height);
    } else {
        drawArea.queue_draw();
    }
}

void _wlWindow::setFocus() {
    drawArea.grab_focus();
    activate_focus();
}

// helper method to save the packed menu height for us, since it seems difficult to acquire otherwise ...
void _wlWindow::on_menuBar_allocate(Gtk::Allocation &allocation)
{
    menuHeight = allocation.get_height();

    // re-set min/max size if those are in effect
    if (props.usedFields) {
        setGeometryHints(this, &props, menuHeight);
    }
}

void _wlWindow::setMenuBar(wl_MenuBar menuBar) {
    // only way to get the actual menubar height ...
    // which is used to fudge the min/max sizes (see on_menuBar_allocate())
    // as for the initial size, we just rely on setting the drawArea size before the window is visible
    menuBar->gtkMenuBar.signal_size_allocate().connect(sigc::mem_fun1(*this, &_wlWindow::on_menuBar_allocate));

    vbox.pack_start(menuBar->gtkMenuBar, false, false, 0);
    menuBar->attachedTo = this;
    attachedMenuBar = menuBar;
}

void _wlWindow::execAction(wl_Action action) {
    EventFrame ef(nullptr);
    auto wl_Event = &ef.wl_Event;
    wl_Event->eventType = wl_kEventTypeAction;
    wl_Event->actionEvent.id = action->id;
    wl_Event->actionEvent.action = action;
    dispatchEvent(wl_Event);
}

bool _wlWindow::getDragActive() {
    return dragActive;
}

void _wlWindow::getDropData(GdkDragContext *dragContext, GdkAtom formatAtom, void **data, size_t *size)
{
    dragData = data;
    dragDataSize = size;

    // set a flag to know if everything's been processed yet
    // seems like internal-only stuff happens synchronously, external drops are async
    dragDataAwait = true;

    // forces source to provide data
    // triggers on_drawArea_dragDataGet, on_drawArea_dragDataReceived
    gtk_drag_get_data(GTK_WIDGET(drawArea.gobj()), dragContext, formatAtom, gtk_get_current_event_time());

    // might need to block here until dragDataReceived fires ...
    while (dragDataAwait) {
        gtk_main_iteration_do(true);
    }
    printf("out of dragDataAwait loop\n");

    // erase local references -- caller owns data now
    dragData = nullptr;
    dragDataSize = nullptr;
}

//void _wlWindow::releaseDropData(void **dragData)
//{
//    free(*dragData);
//    *dragData = nullptr;
//}

void _wlWindow::setClipboard(wl_DragData dragData)
{
    std::list<Gtk::TargetEntry> listTargets;
    auto flags = Gtk::TARGET_SAME_APP | Gtk::TARGET_SAME_WIDGET | Gtk::TARGET_OTHER_APP | Gtk::TARGET_OTHER_WIDGET;
    for (const auto &format: dragData->formats) {
        Gtk::TargetEntry entry(const_cast<gchar *>(format.c_str()), flags, 0); //last param was: (guint)format
        listTargets.push_back(entry);
    }

    gtkClipboard->set(listTargets,
                      sigc::mem_fun(*this, &_wlWindow::on_clipboard_get),
                      sigc::mem_fun(*this, &_wlWindow::on_clipboard_clear));

    gtkClipboard->set_can_store(); // so it can be flushed on exit
}

size_t _wlWindow::getWindowHandle()
{
    return (size_t)gdk_x11_window_get_drawable_impl((GdkWindow *)gobj());
}

void _wlWindow::mouseGrab()
{
    Glib::RefPtr<Gdk::Window> gdkWindow = get_window();
    gdkWindow->pointer_grab(true, Gdk::BUTTON_MOTION_MASK, GDK_CURRENT_TIME);
//    gdk_pointer_grab((GdkWindow *)gobj(), true, GDK_BUTTON_MOTION_MASK, nullptr, nullptr, GDK_CURRENT_TIME);
}

void _wlWindow::relativeToAbsolute(int x, int y, int *absX, int *absY)
{
    int originX, originY;
    gdk_window_get_origin(get_window()->gobj(), &originX, &originY);
    if (attachedMenuBar) {
        // so this would only work for a visible window, whose menuHeight has been allocated ...
        g_assert(menuHeight > 0);
        originY += menuHeight;
    }
    *absX = originX + x;
    *absY = originY + y;
}

