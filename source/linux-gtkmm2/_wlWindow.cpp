#include "_wlWindow.h"
#include "private_defs.h"
#include "util.h"
#include "keystuff.h"

#include <gdk/gdkx.h>

struct EventFrame {
    _wlEventPrivate _priv;
    WLEvent wlEvent;
    EventFrame(GdkEvent *gdkEvent) {
        wlEvent._private = &_priv;
        _priv.gdkEvent = gdkEvent;
    }
};

void _wlWindow::dragClipRender(Gtk::SelectionData &selectionData, guint info) {
    EventFrame ef(nullptr);
    auto wlEvent = &ef.wlEvent;
    wlEvent->eventType = WLEventType_DragRender;

    wlEvent->dragRenderEvent.dragFormat = selectionData.get_target().c_str(); //(WLDragFormatEnum)info

    auto payload = new _wlRenderPayload;
    wlEvent->dragRenderEvent.payload = payload;

    dispatchEvent(wlEvent);

    if (wlEvent->handled) {
        selectionData.set(8, (guint8 *)payload->data, (int)payload->dataSize);
        printf("selectiondata set\n");
    } else {
        printf("drag/clip render was unhandled, nothing to do/set\n");
    }
    delete payload;
}

void setGeometryHints(Gtk::Window *gtkWin, WLWindowProperties *props, int menuHeight)
{
    Gdk::Geometry geom = {0};
    auto mask = (Gdk::WindowHints)0;
    if (props->usedFields & WLWindowProp_MinWidth) {
        geom.min_width = props->minWidth;
        mask |= Gdk::HINT_MIN_SIZE;
    }
    if (props->usedFields & WLWindowProp_MinHeight) {
        geom.min_height = props->minHeight + menuHeight;
        mask |= Gdk::HINT_MIN_SIZE;
    }
    if (props->usedFields & WLWindowProp_MaxWidth) {
        geom.max_width = props->maxWidth;
        mask |= Gdk::HINT_MAX_SIZE;
    }
    if (props->usedFields & WLWindowProp_MaxHeight) {
        geom.max_height = props->maxHeight + menuHeight;
        mask |= Gdk::HINT_MAX_SIZE;
    }
    gtkWin->set_geometry_hints(*gtkWin, geom, mask);
}

Gtk::WindowType getWindowType(WLWindowProperties *props) {
    if (props && (props->usedFields & WLWindowProp_Style) && props->style == WLWindowStyle_Frameless) {
        return Gtk::WINDOW_POPUP;
    } else {
        return Gtk::WINDOW_TOPLEVEL;
    }
}

_wlWindow::_wlWindow(void *userData, WLWindowProperties *props)
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
    printf("wlwindow destruct\n");
    // stop all associated timers
    // copy set so removal doesn't mess with iterator
    auto set_copy = timers;
    for (auto timer : set_copy) {
        timer->disconnect();
        // but don't delete, in case the API client still has a handle to it
    }

    EventFrame ef(nullptr);
    auto wlEvent = &ef.wlEvent;
    wlEvent->eventType = WLEventType_WindowDestroyed;
    wlEvent->destroyEvent.reserved = 0;
    dispatchEvent(wlEvent);
}

/**** callbacks ****/
bool _wlWindow::on_drawArea_expose(GdkEventExpose *gdkEvent) {
    EventFrame ef((GdkEvent *)gdkEvent);
    auto wlEvent = &ef.wlEvent;
    wlEvent->eventType = WLEventType_WindowRepaint;
    auto context = drawArea.get_window()->create_cairo_context();
    wlEvent->repaintEvent.platformContext = context->cobj(); // cairo_t *
    wlEvent->repaintEvent.x = gdkEvent->area.x;
    wlEvent->repaintEvent.y = gdkEvent->area.y;
    wlEvent->repaintEvent.width = gdkEvent->area.width;
    wlEvent->repaintEvent.height = gdkEvent->area.height;
    dispatchEvent(wlEvent);
    return wlEvent->handled;
//        auto width = drawArea.get_allocation().get_width();
//        auto height = drawArea.get_allocation().get_height();
//        printf("alloc w/h: %d/%d\n", width, height);
}

void _wlWindow::on_drawArea_allocate(Gtk::Allocation &allocation) {
    if (allocation.get_width() != width || allocation.get_height() != height) {
        EventFrame ef(nullptr);
        auto wlEvent = &ef.wlEvent;
        wlEvent->eventType = WLEventType_WindowResized;
        wlEvent->resizeEvent.newWidth = allocation.get_width();
        wlEvent->resizeEvent.newHeight = allocation.get_height();
        wlEvent->resizeEvent.oldWidth = width;
        wlEvent->resizeEvent.oldHeight = height;
        dispatchEvent(wlEvent);
        width = allocation.get_width();
        height = allocation.get_height();
    }
}

bool _wlWindow::on_drawArea_buttonPress(GdkEventButton *gdkEvent) {
    EventFrame ef((GdkEvent *)gdkEvent);
    auto wlEvent = &ef.wlEvent;
    wlEvent->eventType = WLEventType_Mouse;
    wlEvent->mouseEvent.eventType = WLMouseEventType_MouseDown;
    wlEvent->mouseEvent.x = (int)gdkEvent->x;
    wlEvent->mouseEvent.y = (int)gdkEvent->y;
    wlEvent->mouseEvent.modifiers = gdkToWlModifiers(gdkEvent->state);
    wlEvent->mouseEvent.button = gdkToWlButton(gdkEvent->button);
    dispatchEvent(wlEvent);
    return wlEvent->handled;
}


bool _wlWindow::on_drawArea_buttonRelease(GdkEventButton *gdkEvent) {
    EventFrame ef((GdkEvent *)gdkEvent);
    auto wlEvent = &ef.wlEvent;
    wlEvent->eventType = WLEventType_Mouse;
    wlEvent->mouseEvent.eventType = WLMouseEventType_MouseUp;
    wlEvent->mouseEvent.x = (int)gdkEvent->x;
    wlEvent->mouseEvent.y = (int)gdkEvent->y;
    wlEvent->mouseEvent.modifiers = gdkToWlModifiers(gdkEvent->state);
    wlEvent->mouseEvent.button = gdkToWlButton(gdkEvent->button);
    dispatchEvent(wlEvent);
    return wlEvent->handled;
}

bool _wlWindow::on_drawArea_keyPress(GdkEventKey *gdkEvent) {
    EventFrame ef((GdkEvent *)gdkEvent);
    auto wlEvent = &ef.wlEvent;
    wlEvent->eventType = WLEventType_Key;
    wlEvent->keyEvent.key = WLKey_Unknown;
    wlEvent->keyEvent.modifiers = gdkToWlModifiers(gdkEvent->state);
    wlEvent->keyEvent.location = WLKeyLocation_Default;

    auto keyHandled = false;
    auto charHandled = false;
    bool suppressChar = false;

    // key event, if any
    auto found = keyMap.find(gdkEvent->keyval);
    if (found != keyMap.end()) {
        auto info = found->second;
        wlEvent->keyEvent.eventType = WLKeyEventType_Down;
        wlEvent->keyEvent.key = info->key;
        wlEvent->keyEvent.string = info->stringRep;
        wlEvent->keyEvent.location = info->location;

        dispatchEvent(wlEvent);

        keyHandled = wlEvent->handled;
        suppressChar = info->suppressChar;
    }

    // character event, if any
    if (gdkEvent->string
        && strlen(gdkEvent->string) > 0
        && !suppressChar) // can be suppressed if it's something we don't want characters for (return, esc, etc)
    {
        wlEvent->keyEvent.eventType = WLKeyEventType_Char;
        wlEvent->keyEvent.string = gdkEvent->string;

        dispatchEvent(wlEvent);

        charHandled = wlEvent->handled;
    }
    return keyHandled || charHandled;
}

bool _wlWindow::on_drawArea_keyRelease(GdkEventKey *gdkEvent) {
    auto found = keyMap.find(gdkEvent->keyval);
    if (found != keyMap.end()) {
        auto info = found->second;
        EventFrame ef((GdkEvent *)gdkEvent);
        auto wlEvent = &ef.wlEvent;
        wlEvent->eventType = WLEventType_Key;
        wlEvent->keyEvent.eventType = WLKeyEventType_Up;
        wlEvent->keyEvent.modifiers = gdkToWlModifiers(gdkEvent->state);
        wlEvent->keyEvent.location = info->location;
        wlEvent->keyEvent.key = info->key;
        wlEvent->keyEvent.string = info->stringRep;
        dispatchEvent(wlEvent);
        return wlEvent->handled;
    }
    return false; // definitely unhandled
}

bool _wlWindow::on_drawArea_mouseMotion(GdkEventMotion *gdkEvent) {
    EventFrame ef((GdkEvent *)gdkEvent);
    auto wlEvent = &ef.wlEvent;
    wlEvent->eventType = WLEventType_Mouse;
    wlEvent->mouseEvent.eventType = WLMouseEventType_MouseMove;
    wlEvent->mouseEvent.button = WLMouseButton_None;
    wlEvent->mouseEvent.modifiers = gdkToWlModifiers(gdkEvent->state);
    wlEvent->mouseEvent.x = (int)gdkEvent->x;
    wlEvent->mouseEvent.y = (int)gdkEvent->y;
    dispatchEvent(wlEvent);
    return wlEvent->handled;
}

bool _wlWindow::on_drawArea_enterNotify(GdkEventCrossing *gdkEvent) {
    EventFrame ef((GdkEvent *)gdkEvent);
    auto wlEvent = &ef.wlEvent;
    wlEvent->eventType = WLEventType_Mouse;
    wlEvent->mouseEvent.eventType = WLMouseEventType_MouseEnter;
    wlEvent->mouseEvent.button = WLMouseButton_None;
    wlEvent->mouseEvent.modifiers = gdkToWlModifiers(gdkEvent->state);
    wlEvent->mouseEvent.x = (int)gdkEvent->x;
    wlEvent->mouseEvent.y = (int)gdkEvent->y;
    dispatchEvent(wlEvent);
    return wlEvent->handled;
}

bool _wlWindow::on_drawArea_leaveNotify(GdkEventCrossing *gdkEvent) {
    EventFrame ef((GdkEvent *)gdkEvent);
    auto wlEvent = &ef.wlEvent;
    wlEvent->eventType = WLEventType_Mouse;
    wlEvent->mouseEvent.eventType = WLMouseEventType_MouseLeave;
    wlEvent->mouseEvent.button = WLMouseButton_None;
    wlEvent->mouseEvent.modifiers = gdkToWlModifiers(gdkEvent->state);
    wlEvent->mouseEvent.x = (int)gdkEvent->x;
    wlEvent->mouseEvent.y = (int)gdkEvent->y;
    dispatchEvent(wlEvent);
    return wlEvent->handled;
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
    auto wlEvent = &ev.wlEvent;
    wlEvent->eventType = WLEventType_Drop;
    wlEvent->dropEvent.eventType = WLDropEventType_Feedback;
    wlEvent->dropEvent.x = x;
    wlEvent->dropEvent.y = y;
    wlEvent->dropEvent.data = &dropData;

    //        printf("suggested: %d\n", context->get_suggested_action());
    //        printf("default action is: %d\n", wlEvent->dropEvent.defaultModifierAction);
    //        printf("actions: %d\n", context->get_actions());

    // set default modifier action to whatever's normal/suggested for this modifier combination (ctrl/alt/shift/etc)
    wlEvent->dropEvent.defaultModifierAction = gdkToWlDropEffectSingle(context->get_suggested_action());

    // set allowed effect mask to all the ones allowed/possible
    wlEvent->dropEvent.allowedEffectMask = gdkToWlDropEffectMulti(context->get_actions());

    dispatchEvent(wlEvent);

    // final verdict?
    auto dropAction = wlToGdkDropEffectMulti(wlEvent->dropEvent.allowedEffectMask);
    context->drag_status(dropAction, gtk_get_current_event_time());

    return (wlEvent->dropEvent.allowedEffectMask != WLDropEffect_None);
}

bool _wlWindow::on_drawArea_dragDrop(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time)
{
    _wlDropData_Drop dropData(context, this);

    printf("=== drag drop\n");
    // emitted on drop site
    // true if cursor position in a drop zone
    // must also call gtk_drag_finish to let source know drop is done (edit: that doesn't happen until dragDataReceived)

    EventFrame ef(nullptr);
    auto wlEvent = &ef.wlEvent;
    wlEvent->eventType = WLEventType_Drop;
    wlEvent->dropEvent.eventType = WLDropEventType_Drop;
    wlEvent->dropEvent.data = &dropData;
    printf("before dragDrop dispatchEvent\n");
    dispatchEvent(wlEvent);
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

bool _wlWindow::on_timer_timeout(wlTimer timer) {
    EventFrame ef(nullptr);
    auto wlEvent = &ef.wlEvent;
    wlEvent->eventType = WLEventType_Timer;
    wlEvent->timerEvent.timer = timer;
    wlEvent->timerEvent.timerID = timer->timerID;
    wlEvent->timerEvent.stopTimer = false;

    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    wlEvent->timerEvent.secondsSinceLast = timespecDiff(now, timer->lastTime);

    dispatchEvent(wlEvent);

    timer->lastTime = now;

    if (wlEvent->timerEvent.stopTimer) {
        return false; // disconnect (but don't delete)
    }
    return true; // continue by default
}

bool _wlWindow::on_delete(GdkEventAny *gdkEvent) {
    EventFrame ef((GdkEvent *)gdkEvent);
    auto wlEvent = &ef.wlEvent;
    wlEvent->eventType = WLEventType_WindowCloseRequest;
    wlEvent->closeRequestEvent.cancelClose = false;
    dispatchEvent(wlEvent);
    if (!wlEvent->closeRequestEvent.cancelClose) {
        delete this; // ??
    }
    return wlEvent->closeRequestEvent.cancelClose;
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
    auto wlEvent = &ef.wlEvent;
    wlEvent->eventType = WLEventType_ClipboardClear;
    wlEvent->clipboardClearEvent.reserved = 0;
    dispatchEvent(wlEvent);
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

void _wlWindow::setMenuBar(wlMenuBar menuBar) {
    // only way to get the actual menubar height ...
    // which is used to fudge the min/max sizes (see on_menuBar_allocate())
    // as for the initial size, we just rely on setting the drawArea size before the window is visible
    menuBar->gtkMenuBar.signal_size_allocate().connect(sigc::mem_fun1(*this, &_wlWindow::on_menuBar_allocate));

    vbox.pack_start(menuBar->gtkMenuBar, false, false, 0);
    menuBar->attachedTo = this;
    attachedMenuBar = menuBar;
}

void _wlWindow::execAction(wlAction action) {
    EventFrame ef(nullptr);
    auto wlEvent = &ef.wlEvent;
    wlEvent->eventType = WLEventType_Action;
    wlEvent->actionEvent.id = action->id;
    wlEvent->actionEvent.action = action;
    dispatchEvent(wlEvent);
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

void _wlWindow::setClipboard(wlDragData dragData)
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

