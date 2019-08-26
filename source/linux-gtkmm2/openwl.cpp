#include "../openwl.h"
#include <gtkmm.h>
#include <cstdio>
#include <vector>
#include <cassert>
#include <set>
#include <openwl.h>
#include "unicodestuff.h"
#include "boost_stuff.h"

#include <time.h> // nanosleep, clock_gettime

#include "globals.h"
#include "util.h"
#include "private_defs.h"
#include "wlWindow.h"
#include "keystuff.h"

/**************************/
/** MAIN/APPLICATION API **/
/**************************/

OPENWL_API int CDECL wl_Init(wl_EventCallback callback, struct wl_PlatformOptions *options) {
    initKeymap();

    appMain = new Gtk::Main(nullptr, nullptr);
    ::eventCallback = callback;
    globalAccelGroup = Gtk::AccelGroup::create();
    gtkClipboard = Gtk::Clipboard::get();
}

OPENWL_API int CDECL wl_Runloop() {
    appMain->run();
}

OPENWL_API void CDECL wl_ExitRunloop() {
    appMain->quit();
}

OPENWL_API void CDECL wl_Shutdown() {
    delete appMain;
}


/****************/
/** WINDOW API **/
/****************/

OPENWL_API wl_WindowRef CDECL
wl_WindowCreate(int width, int height, const char *title, void *userData, wl_WindowProperties *props) {
    auto window = new wl_Window(userData, props);

    if (title) {
        window->set_title(title);
    }
    //window->set_default_size(width, height);
    window->getDrawArea()->set_size_request(width, height); // derp! much cleaner than what we had been doing

    window->add_accel_group(globalAccelGroup); // does this need to be omitted for popup windows?

    return window;
}

OPENWL_API void CDECL wl_WindowDestroy(wl_WindowRef window)
{
    delete window;
}

OPENWL_API void CDECL wl_WindowShow(wl_WindowRef window)
{
    window->show_all();
}

OPENWL_API void CDECL wl_WindowShowRelative(wl_WindowRef window, wl_WindowRef relativeTo, int x, int y, int newWidth, int newHeight)
{
    int x2, y2;
    relativeTo->relativeToAbsolute(x, y, &x2, &y2);
    window->move(x2, y2);
    if (newWidth > 0 && newHeight > 0) {
        window->set_size_request(newWidth, newHeight);
    }
    window->show_all();
}

OPENWL_API void CDECL wl_WindowHide(wl_WindowRef window)
{
    window->hide();
}

OPENWL_API void CDECL wl_WindowInvalidate(wl_WindowRef window, int x, int y, int width, int height)
{
    window->invalidate(x, y, width, height);
}

OPENWL_API size_t CDECL wl_WindowGetOSHandle(wl_WindowRef window)
{
    return window->getWindowHandle();
}

OPENWL_API void CDECL wl_WindowSetFocus(wl_WindowRef window)
{
    window->setFocus();
}

OPENWL_API void CDECL wl_MouseGrab(wl_WindowRef window)
{
    // automatic grabs seem to be in effect, and what's more,
    //  if we do this manually, they stop working (eg when dragging outside the window)
//    window->mouseGrab();
}

OPENWL_API void CDECL wl_MouseUngrab()
{
    // see comment above re: automatic grabs
//    gdk_pointer_ungrab(GDK_CURRENT_TIME);
}

/**************/
/* CURSOR API */
/**************/

OPENWL_API wl_CursorRef CDECL wl_CursorCreate(wl_CursorStyle style)
{
    // does it exist in the map already?
    auto found = cursorMap.find(style);
    if (found != cursorMap.end()) {
        return found->second;
    } else {
        // create anew
        GdkCursorType gdkType;
        switch (style) {
            case wl_kCursorStyleDefault:
                gdkType = GDK_ARROW;
                break;
            case wl_kCursorStyleResizeLeftRight:
                gdkType = GDK_SB_H_DOUBLE_ARROW;
                break;
            case wl_kCursorStyleResizeUpDown:
                gdkType = GDK_SB_V_DOUBLE_ARROW;
                break;
            default:
                printf("wl_CursorCreate: unknown style %d\n", style);
                return nullptr;
        }
        auto ret = new wl_Cursor;
        ret->gdkCursor = gdk_cursor_new(gdkType);
        // store in map
        cursorMap[style] = ret;
        return ret;
    }
}

OPENWL_API void CDECL wl_WindowSetCursor(wl_WindowRef window, wl_CursorRef cursor)
{
    window->setCursor(cursor);
}

/***************/
/** TIMER API **/
/***************/

OPENWL_API wl_TimerRef CDECL wl_TimerCreate(wl_WindowRef window, int timerID, unsigned int msTimeout)
{
    auto timer = new wl_Timer;
    timer->window = window;
    timer->timerID = timerID;
    clock_gettime(CLOCK_MONOTONIC, &timer->lastTime);

    auto slot = sigc::bind(sigc::mem_fun(*window, &wl_Window::on_timer_timeout), timer);
    timer->conn = Glib::signal_timeout().connect(slot, msTimeout);

    timer->connected = true;

    window->insertTimer(timer);

    return timer;
}

OPENWL_API void CDECL wl_TimerDestroy(wl_TimerRef timer)
{
    delete timer;
    printf("deleted timer %p - %d\n", timer, timer->timerID);
}


/********************/
/**** action API ****/
/********************/

static void sizeToFit(int *width, int *height, int maxWidth, int maxHeight)
{
    double sourceAspect = (double)*width / (double)*height;
    double targetAspect = (double)maxWidth / (double)maxHeight;
    if (sourceAspect <= targetAspect) {
        // clamp height, width will be OK
        *height = maxHeight;
        *width = int(sourceAspect * maxHeight);
        assert(*width <= maxWidth);
    }
    else {
        // clamp width, height will be OK
        *width = maxWidth;
        *height = int(maxWidth / sourceAspect);
        assert(*height <= maxHeight);
    }
}


OPENWL_API wl_IconRef CDECL wl_IconLoadFromFile(const char *filename, int sizeToWidth)
{
    auto pixbuf = Gdk::Pixbuf::create_from_file(filename);
    if (sizeToWidth > 0) {
        int width = pixbuf->get_width();
        int height = pixbuf->get_height();
        sizeToFit(&width, &height, sizeToWidth, sizeToWidth);
        auto scaled = pixbuf->scale_simple(width, height, Gdk::INTERP_BILINEAR);
        // no need to delete pixbuf before reassignment, it's reference-counted
        pixbuf = scaled;
    }
    auto ret = new wl_Icon;
    ret->gtkImage = new Gtk::Image(pixbuf);
    return ret;
}

OPENWL_API wl_AcceleratorRef CDECL wl_AccelCreate(wl_KeyEnum key, unsigned int modifiers)
{
    auto ret = new wl_Accelerator;
    ret->key = key;
    ret->modifiers = modifiers;
    return ret;
}

OPENWL_API wl_ActionRef CDECL wl_ActionCreate(int id, const char *label, wl_IconRef icon, wl_AcceleratorRef accel)
{
    auto ret = new wl_Action;
    ret->id = id;
    ret->label = label;
    ret->icon = icon;
    ret->accel = accel;
    ret->attachedItems.clear();
    return ret;
}

/*****************/
/*** MENU API ****/
/*****************/



OPENWL_API wl_MenuRef CDECL wl_MenuCreate()
{
    return new wl_Menu;
}

OPENWL_API wl_MenuItemRef CDECL wl_MenuAddAction(wl_MenuRef menu, wl_ActionRef action)
{
    std::string label_copy = action->label;
    replace_all(label_copy, "&", "_"); // menu mnemonics are prefixed with underscores, not &

    auto ret = new wl_MenuItem;
    if (action->icon) {
        ret->gtkItem = new Gtk::ImageMenuItem(*action->icon->gtkImage, label_copy, true);
    } else {
        ret->gtkItem = new Gtk::MenuItem(label_copy, true);
    }
    ret->parentShell = menu;
    ret->action = action;
    ret->sub = nullptr;

    menu->gtkMenu.append(*ret->gtkItem);

//    auto slot = sigc::bind(sigc::mem_fun(*window, &_wlWindow::on_timer_timeout), timer);
//    timer->conn = Glib::signal_timeout().connect(slot, msTimeout);

    auto slot = sigc::bind(sigc::mem_fun(*menu, &wl_Menu::on_item_activate), ret);
    ret->gtkItem->signal_activate().connect(slot);

    // add accelerator if any
    if (action->accel) {
        auto found = reverseKeyMap.find(action->accel->key);
        if (found != reverseKeyMap.end()) {
            auto info = found->second;
            auto mods = wlToGdkModifiers(action->accel->modifiers);
            ret->gtkItem->add_accelerator("activate", globalAccelGroup, info->lowerSym, (Gdk::ModifierType)mods, Gtk::ACCEL_VISIBLE);
        }
    }
    return ret;

}

static wl_MenuItemRef addSubmenuCommon(wl_MenuShell *shell, const char *label, wl_MenuRef sub)
{
    std::string label_copy = label;
    replace_all(label_copy, "&", "_");

    auto item = new Gtk::MenuItem(label_copy, true);
    item->set_submenu(sub->gtkMenu);
    shell->getShell().append(*item);

    auto ret = new wl_MenuItem;
    ret->parentShell = shell;
    ret->gtkItem = item;
    ret->action = nullptr;
    ret->sub = sub;

    sub->parentItem = ret; // refer back to parent, so we can find the associated window from children later

    return ret;
}

OPENWL_API wl_MenuItemRef CDECL wl_MenuAddSubmenu(wl_MenuRef menu, const char *label, wl_MenuRef sub)
{
    return addSubmenuCommon(menu, label, sub);
}

OPENWL_API void CDECL wl_MenuAddSeparator(wl_MenuRef menu)
{
    auto sep = new Gtk::SeparatorMenuItem();
    menu->gtkMenu.append(*sep);
}


OPENWL_API wl_MenuBarRef CDECL wl_MenuBarCreate()
{
    auto ret = new wl_MenuBar;
    ret->attachedTo = nullptr;
    return ret;
}

OPENWL_API wl_MenuItemRef CDECL wl_MenuBarAddMenu(wl_MenuBarRef menuBar, const char *label, wl_MenuRef menu)
{
    return addSubmenuCommon(menuBar, label, menu);
}

OPENWL_API void CDECL wl_WindowSetMenuBar(wl_WindowRef window, wl_MenuBarRef menuBar)
{
    window->setMenuBar(menuBar);
}

OPENWL_API void CDECL wl_WindowShowContextMenu(wl_WindowRef window, int x, int y, wl_MenuRef menu, wl_Event *fromEvent)
{
    menu->contextFor = window;
    if (fromEvent
        && fromEvent->_private->gdkEvent
        && fromEvent->_private->gdkEvent->type == GDK_BUTTON_PRESS)
    {
        auto buttonEvent = (GdkEventButton *)fromEvent->_private->gdkEvent;
        menu->gtkMenu.popup(buttonEvent->button, buttonEvent->time);
    } else {
        menu->gtkMenu.popup(0, gtk_get_current_event_time());
    }
    menu->contextFor = nullptr;
}

/***************************/
/*** DnD / CLIPBOARD API ***/
/***************************/

OPENWL_API const char *wl_kDragFormatUTF8 = "text/plain"; //charset=utf-8";
OPENWL_API const char *wl_kDragFormatFiles = "text/uri-list";

OPENWL_API wl_DragDataRef CDECL wl_DragDataCreate(wl_WindowRef forWindow)
{
    auto ret = new wl_DragData;
    ret->forWindow = forWindow;
    return ret;
}

OPENWL_API void CDECL wl_DragDataRelease(wl_DragDataRef *dragData)
{
    delete *dragData;
    *dragData = nullptr;
}

OPENWL_API void CDECL wl_DragAddFormat(wl_DragDataRef dragData, const char *dragFormatMIME)
{
    dragData->formats.insert(dragFormatMIME);
}

// drop target stuff
OPENWL_API bool CDECL wl_DropHasFormat(wl_DropDataRef dropData, const char *dropFormatMIME)
{
    auto result = dropData->hasTarget(dropFormatMIME);
    printf("drophasformat: %s\n", result ? "true" : "false");
    return result;
}

OPENWL_API bool CDECL wl_DropGetFormat(wl_DropDataRef dropData, const char *dropFormatMIME, const void **data, size_t *dataSize)
{
    return dropData->getFormat(dropFormatMIME, data, dataSize);
}

OPENWL_API void CDECL wl_ClipboardRelease(wl_DropDataRef dropData)
{
    delete dropData;
}


OPENWL_API bool CDECL wl_DropGetFiles(wl_DropDataRef dropData, const struct wl_Files **files)
{
    return dropData->getFiles(files);
}

OPENWL_API void CDECL wl_DragRenderUTF8(wl_RenderPayloadRef payload, const char *text)
{
    auto size = strlen(text) + 1;
    payload->data = malloc(size);
    memcpy(payload->data, text, size);
    payload->dataSize = size;
}

OPENWL_API void CDECL wl_DragRenderFiles(wl_RenderPayloadRef payload, const struct wl_Files *files)
{
    // ??
}

OPENWL_API void CDECL wl_DragRenderFormat(wl_RenderPayloadRef payload, const char *formatMIME, const void *data, size_t dataSize)
{
    payload->data = malloc(dataSize);
    memcpy(payload->data, data, dataSize);
    payload->dataSize = (size_t)dataSize;
}

OPENWL_API wl_DropEffect CDECL wl_DragExec(wl_DragDataRef dragData, unsigned int dropActionsMask, wl_Event *fromEvent)
{
    auto count = dragData->formats.size();
    auto entries = new GtkTargetEntry[count];
    int i = 0;
    for (const auto &format: dragData->formats) {
        auto target = const_cast<gchar *>(format.c_str());;
        printf("dragexec target: [%s]\n", target);
        entries[i].target = target;
                //const_cast<gchar *>(targetFromFormat(format));
        entries[i].info = 0; //format.c_str();
        entries[i].flags = Gtk::TARGET_SAME_APP | Gtk::TARGET_SAME_WIDGET | Gtk::TARGET_OTHER_APP | Gtk::TARGET_OTHER_WIDGET;
        i++;
    }
    auto list = gtk_target_list_new(entries, (guint)count);

    auto drawArea = dragData->forWindow->getDrawArea();

    auto allowedActions = (GdkDragAction)wlToGdkDropEffectMulti(dropActionsMask);

    auto dragContext =
            gtk_drag_begin(GTK_WIDGET(drawArea->gobj()), list, allowedActions, wlToGdkButton(wl_kMouseButtonLeft), fromEvent->_private->gdkEvent);
    printf("drag begun...\n");

    // enter modal loop here, wait for drag result
    while (dragData->forWindow->getDragActive()) {
        gtk_main_iteration_do(true);
    }
    printf("iterations done\n");

    gtk_target_list_unref(list);
    delete[] entries;

    auto result = gdkToWlDropEffectSingle((Gdk::DragAction)dragContext->action);
    return result;
}

OPENWL_API void wl_WindowEnableDrops(wl_WindowRef window, bool enabled)
{
    auto drawArea = window->getDrawArea();
    if (enabled) {
        printf("drag dest set on window\n");
        drawArea->drag_dest_set(); //Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_DEFAULT); // or default?
    } else {
        drawArea->drag_dest_unset();
    }
}

/*********************/
/*** CLIPBOARD API ***/
/*********************/

OPENWL_API void CDECL wl_ClipboardSet(wl_DragDataRef dragData)
{
    dragData->forWindow->setClipboard(dragData);
}

OPENWL_API wl_DropDataRef CDECL wl_ClipboardGet()
{
    return new wl_DropData_Clip();
}

OPENWL_API void CDECL wl_ClipboardFlush()
{
    gtkClipboard->store();
}

/*******************/
/**** MISC API *****/
/*******************/

#include <mutex>
#include <condition_variable>

struct IdleData {
    wl_VoidCallback callback;
    void *data;
    std::condition_variable& execCond;
};
static std::mutex execMutex;

static gboolean idleFunc(gpointer user_data) {
    std::lock_guard<std::mutex> lock(execMutex);
    //
    auto idleData = (IdleData *)user_data;
    idleData->callback(idleData->data);
    idleData->execCond.notify_one();
    //
    return G_SOURCE_REMOVE; // one shot
}

OPENWL_API void CDECL wl_ExecuteOnMainThread(wl_WindowRef window, wl_VoidCallback callback, void *data)
{
    std::unique_lock<std::mutex> lock(execMutex);
    std::condition_variable cond;

    IdleData id = { callback, data, cond };
    g_idle_add(idleFunc, &id);

    cond.wait(lock);
}

OPENWL_API void CDECL wl_Sleep(unsigned int millis)
{
    timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = millis * 1000 * 1000;
    nanosleep(&ts, NULL);
}

OPENWL_API size_t CDECL wl_SystemMillis()
{
    auto usec = g_get_monotonic_time();
    return (size_t)(usec / 1000);
}


