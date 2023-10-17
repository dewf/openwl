#include "../openwl.h"

#include <InterfaceKit.h>
#include <Application.h>
#include <Locker.h>
#include <stdio.h>

class MyApp : public BApplication {
public:
    MyApp() : BApplication("application/x-vnd.nobody-MyApp1234") {} // TODO: custom app name?
};

static MyApp *app = nullptr;

static wl_EventCallback __appCallback = nullptr;
static BLocker callbackMutex("callback mutex"); // should be recursive-friendly!

class Window; //fwd decl

static int safeCallback(Window *win, wl_Event *event, void *data) {
    callbackMutex.Lock();
    auto ret = __appCallback((wl_WindowRef)win, event, data);
    callbackMutex.Unlock();
    return ret;
}

class Window : public BWindow {
private:
    void *userData = nullptr;
public:
    Window(const char *title, void *userData, int width, int height, wl_WindowProperties *props) :
        BWindow(BRect(0, 0, width, height), title, B_TITLED_WINDOW, 0 /*B_QUIT_ON_WINDOW_CLOSE*/)
    {
        this->userData = userData;
        CenterOnScreen();
        if (props != nullptr) {
            int minWidth = (props->usedFields & wl_kWindowPropMinWidth) != 0 ? props->minWidth : 0.0f;
            int maxWidth = (props->usedFields & wl_kWindowPropMaxWidth) != 0 ? props->maxWidth : 65535.0f;
            int minHeight = (props->usedFields & wl_kWindowPropMinHeight) != 0 ? props->minHeight : 0.0f;
            int maxHeight = (props->usedFields & wl_kWindowPropMaxHeight) != 0 ? props->maxHeight : 65535.0f;
            SetSizeLimits(minWidth, maxWidth, minHeight, maxHeight);
        }
    }

    ~Window() override {
        wl_Event event;
        event.eventType = wl_kEventTypeWindowDestroyed;
        safeCallback(this, &event, userData);
    }

    // events =======

    // really a "close requested" event
    bool QuitRequested() override {
        wl_Event event;
        event.eventType = wl_kEventTypeWindowCloseRequest;
        event.closeRequestEvent.cancelClose = false;
        safeCallback(this, &event, userData);
        return !event.closeRequestEvent.cancelClose;
    }
};


OPENWL_API int CDECL wl_Init(wl_EventCallback callback, struct wl_PlatformOptions *options)
{
    __appCallback = callback;
    app = new MyApp();
    return 0;
}

OPENWL_API int CDECL wl_Runloop()
{
    app->Run();
    return 0;
}

OPENWL_API void CDECL wl_ExitRunloop()
{
    app->PostMessage(B_QUIT_REQUESTED);
    return;
}

OPENWL_API void CDECL wl_Shutdown()
{
    delete app;
    app = nullptr;
    return;
}

/* window api */
OPENWL_API wl_WindowRef CDECL wl_WindowCreate(int width, int height, const char *title, void *userData, struct wl_WindowProperties *props)
{
    auto window = new Window(title, userData, width, height, props);
    return (wl_WindowRef)window;
}

OPENWL_API void CDECL wl_WindowDestroy(wl_WindowRef window)
{
    return;
}

OPENWL_API void CDECL wl_WindowShow(wl_WindowRef window)
{
    ((Window *)window)->Show();
    return;
}

OPENWL_API void CDECL wl_WindowShowRelative(wl_WindowRef window, wl_WindowRef relativeTo, int x, int y, int newWidth, int newHeight)
{
    return;
}

OPENWL_API void CDECL wl_WindowShowModal(wl_WindowRef window, wl_WindowRef parent)
{
    return;
}

OPENWL_API void CDECL wl_WindowEndModal(wl_WindowRef window)
{
    return;
}

OPENWL_API void CDECL wl_WindowHide(wl_WindowRef window)
{
    return;
}

OPENWL_API void CDECL wl_WindowInvalidate(wl_WindowRef window, int x, int y, int width, int height)
{
    return;
}

OPENWL_API size_t CDECL wl_WindowGetOSHandle(wl_WindowRef window)
{
    return 0;
}

OPENWL_API void CDECL wl_WindowSetFocus(wl_WindowRef window)
{
    return;
}

OPENWL_API void CDECL wl_MouseGrab(wl_WindowRef window)
{
    return;
}

OPENWL_API void CDECL wl_MouseUngrab()
{
    return;
}

/* cursor api */
OPENWL_API wl_CursorRef CDECL wl_CursorCreate(wl_CursorStyle style)
{
    return nullptr;
}

OPENWL_API void CDECL wl_WindowSetCursor(wl_WindowRef window, wl_CursorRef cursor)
{
    return;
}

/* timer API */
OPENWL_API wl_TimerRef CDECL wl_TimerCreate(unsigned int msTimeout, void *userData)
{
    return nullptr;
}

OPENWL_API void CDECL wl_TimerDestroy(wl_TimerRef timer)
{
    return;
}

/* action API */
OPENWL_API wl_IconRef CDECL wl_IconLoadFromFile(const char *filename, int sizeToWidth)
{
    return nullptr;
}

OPENWL_API wl_AcceleratorRef CDECL wl_AccelCreate(enum wl_KeyEnum key, unsigned int modifiers)
{
    return nullptr;
}

OPENWL_API wl_ActionRef CDECL wl_ActionCreate(int id, const char *label, wl_IconRef icon, wl_AcceleratorRef accel)
{
    return nullptr;
}

/* menu API */
OPENWL_API wl_MenuRef CDECL wl_MenuCreate()
{
    return nullptr;
}

OPENWL_API wl_MenuItemRef CDECL wl_MenuAddAction(wl_MenuRef menu, wl_ActionRef action)
{
    return nullptr;
}

OPENWL_API wl_MenuItemRef CDECL wl_MenuAddSubmenu(wl_MenuRef menu, const char *label, wl_MenuRef sub)
{
    return nullptr;
}

OPENWL_API void CDECL wl_MenuAddSeparator(wl_MenuRef menu)
{
    return;
}

OPENWL_API wl_MenuItemRef CDECL wl_MenuBarAddMenu(wl_MenuBarRef menuBar, const char *label, wl_MenuRef menu)
{
    return nullptr;
}

OPENWL_API void CDECL wl_WindowShowContextMenu(wl_WindowRef window, int x, int y, wl_MenuRef menu, struct wl_Event *fromEvent)
{
    return;
}

OPENWL_API wl_MenuBarRef CDECL wl_MenuBarCreate()
{
    return nullptr;
}

OPENWL_API void CDECL wl_WindowSetMenuBar(wl_WindowRef window, wl_MenuBarRef menuBar)
{
    return;
}

/* DND API */
OPENWL_API const char *wl_kDragFormatUTF8 = "dragfmt__UTF8";
OPENWL_API const char *wl_kDragFormatFiles = "dragfmt__FILES";

// drag source methods
OPENWL_API wl_DragDataRef CDECL wl_DragDataCreate(wl_WindowRef forWindow)
{
    return nullptr;
}

OPENWL_API void CDECL wl_DragDataRelease(wl_DragDataRef *dragData)
{
    return;
}

OPENWL_API void CDECL wl_DragAddFormat(wl_DragDataRef dragData, const char *dragFormatMIME)
{
    return;
}

OPENWL_API enum wl_DropEffect CDECL wl_DragExec(wl_DragDataRef dragData, unsigned int dropActionsMask, struct wl_Event *fromEvent)
{
    return wl_kDropEffectNone;
}

// drop target methods
OPENWL_API bool CDECL wl_DropHasFormat(wl_DropDataRef dropData, const char *dropFormatMIME)
{
    return false;
}
// wl_DropGetFormat merely gets a pointer to data that is owned by the wl_DropDataRef - and it is only valid as long as the wl_DropDataRef is (typically for the duration of the callback)
OPENWL_API bool CDECL wl_DropGetFormat(wl_DropDataRef dropData, const char *dropFormatMIME, const void **data, size_t *dataSize)
{
    return false;
}
// the wl_Files* is owned by the wl_DropDataRef (points to internal wl_DropDataRef structure) - onyl valid for life of dropData
OPENWL_API bool CDECL wl_DropGetFiles(wl_DropDataRef dropData, const struct wl_Files **files)
{
    return false;
}

// clip/drop data rendering
OPENWL_API void CDECL wl_DragRenderUTF8(wl_RenderPayloadRef payload, const char *text)
{
    return;
}

OPENWL_API void CDECL wl_DragRenderFiles(wl_RenderPayloadRef payload, const struct wl_Files *files)
{
    return;
}

OPENWL_API void CDECL wl_DragRenderFormat(wl_RenderPayloadRef payload, const char *formatMIME, const void *data, size_t dataSize)
{
    return;
}

OPENWL_API void wl_WindowEnableDrops(wl_WindowRef window, bool enabled)
{
    return;
}

/* CLIPBOARD API */
OPENWL_API void CDECL wl_ClipboardSet(wl_DragDataRef dragData)
{
    return;
}

OPENWL_API wl_DropDataRef CDECL wl_ClipboardGet()
{
    return nullptr;
}

OPENWL_API void CDECL wl_ClipboardRelease(wl_DropDataRef dropData)
{
    return;
}

OPENWL_API void CDECL wl_ClipboardFlush()
{
    return;
}

/* FILE OPEN / SAVE */
OPENWL_API bool CDECL wl_FileOpenDialog(struct wl_FileDialogOpts* opts, struct wl_FileResults** results)
{
    return false;
}

OPENWL_API bool CDECL wl_FileSaveDialog(struct wl_FileDialogOpts* opts, struct wl_FileResults** results)
{
    return false;
}

OPENWL_API void CDECL wl_FileResultsFree(struct wl_FileResults** results)
{
    return;
}

/* MESSAGEBOX / ALERT */
OPENWL_API wl_MessageBoxParams::Result CDECL wl_MessageBox(wl_WindowRef window, struct wl_MessageBoxParams* params)
{
    return wl_MessageBoxParams::kResultCancel;
}

/* MISC */
OPENWL_API void CDECL wl_ExecuteOnMainThread(wl_VoidCallback callback, void *data)
{
    return;
}

OPENWL_API void CDECL wl_Sleep(unsigned int millis)
{
    return;
}

OPENWL_API size_t CDECL wl_SystemMillis()
{
    return 0;
}
