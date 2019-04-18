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

OPENWL_API wl_Window CDECL
wl_WindowCreate(int width, int height, const char *title, void *userData, wl_WindowProperties *props) {
    auto window = new wl_WindowImpl(userData, props);

    if (title) {
        window->set_title(title);
    }
    //window->set_default_size(width, height);
    window->getDrawArea()->set_size_request(width, height); // derp! much cleaner than what we had been doing

    window->add_accel_group(globalAccelGroup); // does this need to be omitted for popup windows?

    return window;
}

OPENWL_API void CDECL wl_WindowDestroy(wl_Window window)
{
    delete window;
}

OPENWL_API void CDECL wl_WindowShow(wl_Window window)
{
    window->show_all();
}

OPENWL_API void CDECL wl_WindowShowRelative(wl_Window window, wl_Window relativeTo, int x, int y, int newWidth, int newHeight)
{
    int x2, y2;
    relativeTo->relativeToAbsolute(x, y, &x2, &y2);
    window->move(x2, y2);
    if (newWidth > 0 && newHeight > 0) {
        window->set_size_request(newWidth, newHeight);
    }
    window->show_all();
}

OPENWL_API void CDECL wl_WindowHide(wl_Window window)
{
    window->hide();
}

OPENWL_API void CDECL wl_WindowInvalidate(wl_Window window, int x, int y, int width, int height)
{
    window->invalidate(x, y, width, height);
}

OPENWL_API size_t CDECL wl_WindowGetOSHandle(wl_Window window)
{
    return window->getWindowHandle();
}

OPENWL_API void CDECL wl_WindowSetFocus(wl_Window window)
{
    window->setFocus();
}

OPENWL_API void CDECL wl_MouseGrab(wl_Window window)
{
    window->mouseGrab();
}

OPENWL_API void CDECL wl_MouseUngrab()
{
    gdk_pointer_ungrab(GDK_CURRENT_TIME);
}

/***************/
/** TIMER API **/
/***************/

OPENWL_API wl_Timer CDECL wl_TimerCreate(wl_Window window, int timerID, unsigned int msTimeout)
{
    auto timer = new wl_TimerImpl;
    timer->window = window;
    timer->timerID = timerID;
    clock_gettime(CLOCK_MONOTONIC, &timer->lastTime);

    auto slot = sigc::bind(sigc::mem_fun(*window, &wl_WindowImpl::on_timer_timeout), timer);
    timer->conn = Glib::signal_timeout().connect(slot, msTimeout);

    timer->connected = true;

    window->insertTimer(timer);

    return timer;
}

OPENWL_API void CDECL wl_TimerDestroy(wl_Timer timer)
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


OPENWL_API wl_Icon CDECL wl_IconLoadFromFile(const char *filename, int sizeToWidth)
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
    auto ret = new wl_IconImpl;
    ret->gtkImage = new Gtk::Image(pixbuf);
    return ret;
}

OPENWL_API wl_Accelerator CDECL wl_AccelCreate(wl_KeyEnum key, unsigned int modifiers)
{
    auto ret = new wl_AcceleratorImpl;
    ret->key = key;
    ret->modifiers = modifiers;
    return ret;
}

OPENWL_API wl_Action CDECL wl_ActionCreate(int id, const char *label, wl_Icon icon, wl_Accelerator accel)
{
    auto ret = new wl_ActionImpl;
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



OPENWL_API wl_Menu CDECL wl_MenuCreate()
{
    return new wl_MenuImpl;
}

OPENWL_API wl_MenuItem CDECL wl_MenuAddAction(wl_Menu menu, wl_Action action)
{
    std::string label_copy = action->label;
    replace_all(label_copy, "&", "_"); // menu mnemonics are prefixed with underscores, not &

    auto ret = new wl_MenuItemImpl;
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

    auto slot = sigc::bind(sigc::mem_fun(*menu, &wl_MenuImpl::on_item_activate), ret);
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

static wl_MenuItem addSubmenuCommon(wl_MenuShell *shell, const char *label, wl_Menu sub)
{
    std::string label_copy = label;
    replace_all(label_copy, "&", "_");

    auto item = new Gtk::MenuItem(label_copy, true);
    item->set_submenu(sub->gtkMenu);
    shell->getShell().append(*item);

    auto ret = new wl_MenuItemImpl;
    ret->parentShell = shell;
    ret->gtkItem = item;
    ret->action = nullptr;
    ret->sub = sub;

    sub->parentItem = ret; // refer back to parent, so we can find the associated window from children later

    return ret;
}

OPENWL_API wl_MenuItem CDECL wl_MenuAddSubmenu(wl_Menu menu, const char *label, wl_Menu sub)
{
    return addSubmenuCommon(menu, label, sub);
}

OPENWL_API void CDECL wl_MenuAddSeparator(wl_Menu menu)
{
    auto sep = new Gtk::SeparatorMenuItem();
    menu->gtkMenu.append(*sep);
}


OPENWL_API wl_MenuBar CDECL wl_MenuBarCreate()
{
    auto ret = new wl_MenuBarImpl;
    ret->attachedTo = nullptr;
    return ret;
}

OPENWL_API wl_MenuItem CDECL wl_MenuBarAddMenu(wl_MenuBar menuBar, const char *label, wl_Menu menu)
{
    return addSubmenuCommon(menuBar, label, menu);
}

OPENWL_API void CDECL wl_WindowSetMenuBar(wl_Window window, wl_MenuBar menuBar)
{
    window->setMenuBar(menuBar);
}

OPENWL_API void CDECL wl_WindowShowContextMenu(wl_Window window, int x, int y, wl_Menu menu, wl_Event *fromEvent)
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

OPENWL_API wl_DragData CDECL wl_DragDataCreate(wl_Window forWindow)
{
    auto ret = new wl_DragDataImpl;
    ret->forWindow = forWindow;
    return ret;
}

OPENWL_API void CDECL wl_DragDataRelease(wl_DragData *dragData)
{
    delete *dragData;
    *dragData = nullptr;
}

OPENWL_API void CDECL wl_DragAddFormat(wl_DragData dragData, const char *dragFormatMIME)
{
    dragData->formats.insert(dragFormatMIME);
}

// drop target stuff
OPENWL_API bool CDECL wl_DropHasFormat(wl_DropData dropData, const char *dropFormatMIME)
{
    auto result = dropData->hasTarget(dropFormatMIME);
    printf("drophasformat: %s\n", result ? "true" : "false");
    return result;
}

OPENWL_API bool CDECL wl_DropGetFormat(wl_DropData dropData, const char *dropFormatMIME, const void **data, size_t *dataSize)
{
    return dropData->getFormat(dropFormatMIME, data, dataSize);
}

OPENWL_API void CDECL wl_ClipboardRelease(wl_DropData dropData)
{
    delete dropData;
}


OPENWL_API bool CDECL wl_DropGetFiles(wl_DropData dropData, const struct wl_Files **files)
{
    return dropData->getFiles(files);
}

OPENWL_API void CDECL wl_DragRenderUTF8(wl_RenderPayload payload, const char *text)
{
    auto size = strlen(text) + 1;
    payload->data = malloc(size);
    memcpy(payload->data, text, size);
    payload->dataSize = size;
}

OPENWL_API void CDECL wl_DragRenderFiles(wl_RenderPayload payload, const struct wl_Files *files)
{
    // ??
}

OPENWL_API void CDECL wl_DragRenderFormat(wl_RenderPayload payload, const char *formatMIME, const void *data, size_t dataSize)
{
    payload->data = malloc(dataSize);
    memcpy(payload->data, data, dataSize);
    payload->dataSize = (size_t)dataSize;
}

OPENWL_API wl_DropEffect CDECL wl_DragExec(wl_DragData dragData, unsigned int dropActionsMask, wl_Event *fromEvent)
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

OPENWL_API void wl_WindowEnableDrops(wl_Window window, bool enabled)
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

OPENWL_API void CDECL wl_ClipboardSet(wl_DragData dragData)
{
    dragData->forWindow->setClipboard(dragData);
}

OPENWL_API wl_DropData CDECL wl_ClipboardGet()
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

OPENWL_API void CDECL wl_ExecuteOnMainThread(wl_Window window, wl_VoidCallback callback, void *data)
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


