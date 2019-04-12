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
#include "_wlWindow.h"
#include "keystuff.h"

/**************************/
/** MAIN/APPLICATION API **/
/**************************/

OPENWL_API int CDECL wlInit(wlEventCallback callback, struct WLPlatformOptions *options) {
    initKeymap();

    appMain = new Gtk::Main(nullptr, nullptr);
    ::eventCallback = callback;
    globalAccelGroup = Gtk::AccelGroup::create();
    gtkClipboard = Gtk::Clipboard::get();
}

OPENWL_API int CDECL wlRunloop() {
    appMain->run();
}

OPENWL_API void CDECL wlExitRunloop() {
    appMain->quit();
}

OPENWL_API void CDECL wlShutdown() {
    delete appMain;
}


/****************/
/** WINDOW API **/
/****************/

OPENWL_API wlWindow CDECL
wlWindowCreate(int width, int height, const char *title, void *userData, WLWindowProperties *props) {
    auto window = new _wlWindow(userData, props);

    if (title) {
        window->set_title(title);
    }
    //window->set_default_size(width, height);
    window->getDrawArea()->set_size_request(width, height); // derp! much cleaner than what we had been doing

    window->add_accel_group(globalAccelGroup); // does this need to be omitted for popup windows?

    return window;
}

OPENWL_API void CDECL wlWindowDestroy(wlWindow window)
{
    delete window;
}

OPENWL_API void CDECL wlWindowShow(wlWindow window)
{
    window->show_all();
}

OPENWL_API void CDECL wlWindowShowRelative(wlWindow window, wlWindow relativeTo, int x, int y, int newWidth, int newHeight)
{
    int x2, y2;
    relativeTo->relativeToAbsolute(x, y, &x2, &y2);
    window->move(x2, y2);
    if (newWidth > 0 && newHeight > 0) {
        window->set_size_request(newWidth, newHeight);
    }
    window->show_all();
}

OPENWL_API void CDECL wlWindowHide(wlWindow window)
{
    window->hide();
}

OPENWL_API void CDECL wlWindowInvalidate(wlWindow window, int x, int y, int width, int height)
{
    window->invalidate(x, y, width, height);
}

OPENWL_API size_t CDECL wlWindowGetOSHandle(wlWindow window)
{
    return window->getWindowHandle();
}

OPENWL_API void CDECL wlWindowSetFocus(wlWindow window)
{
    window->setFocus();
}

OPENWL_API void CDECL wlMouseGrab(wlWindow window)
{
    window->mouseGrab();
}

OPENWL_API void CDECL wlMouseUngrab()
{
    gdk_pointer_ungrab(GDK_CURRENT_TIME);
}

/***************/
/** TIMER API **/
/***************/

OPENWL_API wlTimer CDECL wlTimerCreate(wlWindow window, int timerID, unsigned int msTimeout)
{
    auto timer = new _wlTimer;
    timer->window = window;
    timer->timerID = timerID;
    clock_gettime(CLOCK_MONOTONIC, &timer->lastTime);

    auto slot = sigc::bind(sigc::mem_fun(*window, &_wlWindow::on_timer_timeout), timer);
    timer->conn = Glib::signal_timeout().connect(slot, msTimeout);

    timer->connected = true;

    window->insertTimer(timer);

    return timer;
}

OPENWL_API void CDECL wlTimerDestroy(wlTimer timer)
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


OPENWL_API wlIcon CDECL wlIconLoadFromFile(const char *filename, int sizeToWidth)
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
    auto ret = new _wlIcon;
    ret->gtkImage = new Gtk::Image(pixbuf);
    return ret;
}

OPENWL_API wlAccelerator CDECL wlAccelCreate(WLKeyEnum key, unsigned int modifiers)
{
    auto ret = new _wlAccelerator;
    ret->key = key;
    ret->modifiers = modifiers;
    return ret;
}

OPENWL_API wlAction CDECL wlActionCreate(int id, const char *label, wlIcon icon, wlAccelerator accel)
{
    auto ret = new _wlAction;
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



OPENWL_API wlMenu CDECL wlMenuCreate()
{
    return new _wlMenu;
}

OPENWL_API wlMenuItem CDECL wlMenuAddAction(wlMenu menu, wlAction action)
{
    std::string label_copy = action->label;
    replace_all(label_copy, "&", "_"); // menu mnemonics are prefixed with underscores, not &

    auto ret = new _wlMenuItem;
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

    auto slot = sigc::bind(sigc::mem_fun(*menu, &_wlMenu::on_item_activate), ret);
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

static wlMenuItem addSubmenuCommon(_wlMenuShell *shell, const char *label, wlMenu sub)
{
    std::string label_copy = label;
    replace_all(label_copy, "&", "_");

    auto item = new Gtk::MenuItem(label_copy, true);
    item->set_submenu(sub->gtkMenu);
    shell->getShell().append(*item);

    auto ret = new _wlMenuItem;
    ret->parentShell = shell;
    ret->gtkItem = item;
    ret->action = nullptr;
    ret->sub = sub;

    sub->parentItem = ret; // refer back to parent, so we can find the associated window from children later

    return ret;
}

OPENWL_API wlMenuItem CDECL wlMenuAddSubmenu(wlMenu menu, const char *label, wlMenu sub)
{
    return addSubmenuCommon(menu, label, sub);
}

OPENWL_API void CDECL wlMenuAddSeparator(wlMenu menu)
{
    auto sep = new Gtk::SeparatorMenuItem();
    menu->gtkMenu.append(*sep);
}


OPENWL_API wlMenuBar CDECL wlMenuBarCreate()
{
    auto ret = new _wlMenuBar;
    ret->attachedTo = nullptr;
    return ret;
}

OPENWL_API wlMenuItem CDECL wlMenuBarAddMenu(wlMenuBar menuBar, const char *label, wlMenu menu)
{
    return addSubmenuCommon(menuBar, label, menu);
}

OPENWL_API void CDECL wlWindowSetMenuBar(wlWindow window, wlMenuBar menuBar)
{
    window->setMenuBar(menuBar);
}

OPENWL_API void CDECL wlWindowShowContextMenu(wlWindow window, int x, int y, wlMenu menu, WLEvent *fromEvent)
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

OPENWL_API const char *kWLDragFormatUTF8 = "text/plain"; //charset=utf-8";
OPENWL_API const char *kWLDragFormatFiles = "text/uri-list";

OPENWL_API wlDragData CDECL wlDragDataCreate(wlWindow forWindow)
{
    auto ret = new _wlDragData;
    ret->forWindow = forWindow;
    return ret;
}

OPENWL_API void CDECL wlDragDataRelease(wlDragData *dragData)
{
    delete *dragData;
    *dragData = nullptr;
}

OPENWL_API void CDECL wlDragAddFormat(wlDragData dragData, const char *dragFormatMIME)
{
    dragData->formats.insert(dragFormatMIME);
}

// drop target stuff
OPENWL_API bool CDECL wlDropHasFormat(wlDropData dropData, const char *dropFormatMIME)
{
    auto result = dropData->hasTarget(dropFormatMIME);
    printf("drophasformat: %s\n", result ? "true" : "false");
    return result;
}

OPENWL_API bool CDECL wlDropGetFormat(wlDropData dropData, const char *dropFormatMIME, const void **data, size_t *dataSize)
{
    return dropData->getFormat(dropFormatMIME, data, dataSize);
}

OPENWL_API void CDECL wlClipboardRelease(wlDropData dropData)
{
    delete dropData;
}


OPENWL_API bool CDECL wlDropGetFiles(wlDropData dropData, const struct WLFiles **files)
{
    return dropData->getFiles(files);
}

OPENWL_API void CDECL wlDragRenderUTF8(wlRenderPayload payload, const char *text)
{
    auto size = strlen(text) + 1;
    payload->data = malloc(size);
    memcpy(payload->data, text, size);
    payload->dataSize = size;
}

OPENWL_API void CDECL wlDragRenderFiles(wlRenderPayload payload, const struct WLFiles *files)
{
    // ??
}

OPENWL_API void CDECL wlDragRenderFormat(wlRenderPayload payload, const char *formatMIME, const void *data, size_t dataSize)
{
    payload->data = malloc(dataSize);
    memcpy(payload->data, data, dataSize);
    payload->dataSize = (size_t)dataSize;
}

OPENWL_API WLDropEffect CDECL wlDragExec(wlDragData dragData, unsigned int dropActionsMask, WLEvent *fromEvent)
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
            gtk_drag_begin(GTK_WIDGET(drawArea->gobj()), list, allowedActions, wlToGdkButton(WLMouseButton_Left), fromEvent->_private->gdkEvent);
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

OPENWL_API void wlWindowEnableDrops(wlWindow window, bool enabled)
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

OPENWL_API void CDECL wlClipboardSet(wlDragData dragData)
{
    dragData->forWindow->setClipboard(dragData);
}

OPENWL_API wlDropData CDECL wlClipboardGet()
{
    return new _wlDropData_Clip();
}

OPENWL_API void CDECL wlClipboardFlush()
{
    gtkClipboard->store();
}

/*******************/
/**** MISC API *****/
/*******************/

#include <mutex>
#include <condition_variable>

struct IdleData {
    wlVoidCallback callback;
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

OPENWL_API void CDECL wlExecuteOnMainThread(wlWindow window, wlVoidCallback callback, void *data)
{
    std::unique_lock<std::mutex> lock(execMutex);
    std::condition_variable cond;

    IdleData id = { callback, data, cond };
    g_idle_add(idleFunc, &id);

    cond.wait(lock);
}

OPENWL_API void CDECL wlSleep(unsigned int millis)
{
    timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = millis * 1000 * 1000;
    nanosleep(&ts, NULL);
}

OPENWL_API size_t CDECL wlSystemMillis()
{
    auto usec = g_get_monotonic_time();
    return (size_t)(usec / 1000);
}


