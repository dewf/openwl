#ifndef OPENWL_LIBRARY_H
#define OPENWL_LIBRARY_H

#ifdef _WIN32
#   define WL_PLATFORM_WINDOWS
#elif defined __linux__
#   define WL_PLATFORM_LINUX
#elif defined __APPLE__ // could also use TargetConditionals.h ?
#   define WL_PLATFORM_MACOS
#endif

#ifdef WL_PLATFORM_WINDOWS
#	include <Windows.h>
#   include <d2d1_1.h>
#   include <dwrite.h>
#   ifdef WL_COMPILER_MINGW
#       define OPENWL_API __attribute__((visibility("default")))
#       define CDECL
#   else
#       ifdef OPENWL_EXPORTS
#           define OPENWL_API __declspec(dllexport)
#       else
#           define OPENWL_API __declspec(dllimport)
#       endif
#   endif
#elif defined WL_PLATFORM_MACOS
#   define OPENWL_API __attribute__((visibility("default")))
#   define CDECL
#   include <CoreGraphics/CoreGraphics.h> // for CGContextRef
#elif defined WL_PLATFORM_LINUX
#   define OPENWL_API __attribute__((visibility("default")))
#   define CDECL
#   include <cairo/cairo.h> // for cairo_t
#endif

#include <stddef.h>

#define WLHANDLE(x) struct wl_##x; typedef struct wl_##x* wl_##x##Ref
// e.g. struct wl_Window; typedef struct wl_Window* wl_WindowRef;

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

	WLHANDLE(Window);
	WLHANDLE(Cursor);
	WLHANDLE(Timer);
	WLHANDLE(MenuBar);
	WLHANDLE(Menu);
	WLHANDLE(MenuItem);
	WLHANDLE(Icon);
	WLHANDLE(Accelerator); // keyboard shortcuts
	WLHANDLE(Action); // keyboard shortcut or menu happening
	WLHANDLE(DragData); // dnd source
	WLHANDLE(DropData); // dnd dest
	WLHANDLE(EventPrivate); // implementation-specific private wl_Event fields
	WLHANDLE(RenderPayload); // drag source rendering payload -- platform-specific, use rendering functions to modify it

	enum wl_KeyEnum {
		wl_kKeyUnknown,
		//
		wl_kKeyEscape,
		wl_kKeyTab,
		wl_kKeyBackspace,
		wl_kKeyReturn,
		wl_kKeySpace,
		//
		wl_kKeyF1,
		wl_kKeyF2,
		wl_kKeyF3,
		wl_kKeyF4,
		wl_kKeyF5,
		wl_kKeyF6,
		wl_kKeyF7,
		wl_kKeyF8,
		wl_kKeyF9,
		wl_kKeyF10,
		wl_kKeyF11,
		wl_kKeyF12,
		wl_kKeyF13,
		wl_kKeyF14,
		wl_kKeyF15,
		wl_kKeyF16,
		wl_kKeyF17,
		wl_kKeyF18,
		wl_kKeyF19,
		//
		wl_kKey0,
		wl_kKey1,
		wl_kKey2,
		wl_kKey3,
		wl_kKey4,
		wl_kKey5,
		wl_kKey6,
		wl_kKey7,
		wl_kKey8,
		wl_kKey9,
		//
		wl_kKeyA,
		wl_kKeyB,
		wl_kKeyC,
		wl_kKeyD,
		wl_kKeyE,
		wl_kKeyF,
		wl_kKeyG,
		wl_kKeyH,
		wl_kKeyI,
		wl_kKeyJ,
		wl_kKeyK,
		wl_kKeyL,
		wl_kKeyM,
		wl_kKeyN,
		wl_kKeyO,
		wl_kKeyP,
		wl_kKeyQ,
		wl_kKeyR,
		wl_kKeyS,
		wl_kKeyT,
		wl_kKeyU,
		wl_kKeyV,
		wl_kKeyW,
		wl_kKeyX,
		wl_kKeyY,
		wl_kKeyZ,
		// modifiers
		wl_kKeyControl,
		wl_kKeyShift,
		wl_kKeyAltOption,
		wl_kKeyWinCommand,
		wl_kKeyFn,
		// home/end block
		wl_kKeyInsert,
		wl_kKeyDelete,
		wl_kKeyPageUp,
		wl_kKeyPageDown,
		wl_kKeyHome,
		wl_kKeyEnd,
		// arrow keys
		wl_kKeyLeftArrow,
		wl_kKeyUpArrow,
		wl_kKeyRightArrow,
		wl_kKeyDownArrow,
		// keypad numbers
		wl_kKeyKP0,
		wl_kKeyKP1,
		wl_kKeyKP2,
		wl_kKeyKP3,
		wl_kKeyKP4,
		wl_kKeyKP5,
		wl_kKeyKP6,
		wl_kKeyKP7,
		wl_kKeyKP8,
		wl_kKeyKP9,
		// keypad ops
		wl_kKeyKPClear,
		wl_kKeyKPEquals,
		wl_kKeyKPDivide,
		wl_kKeyKPMultiply,
		wl_kKeyKPSubtract,
		wl_kKeyKPAdd,
		wl_kKeyKPEnter,
		wl_kKeyKPDecimal,
		// locks
		wl_kKeyCapsLock,
		wl_kKeyNumLock,
		wl_kKeyScrollLock,
		// misc
		wl_kKeyPrintScreen,
		wl_kKeyPause,  // Pause/Break button minus Ctrl
		wl_kKeyCancel, // Ctrl-Break
		// media
		wl_kKeyMediaMute,
		wl_kKeyMediaVolumeDown,
		wl_kKeyMediaVolumeUp,
		wl_kKeyMediaNext,
		wl_kKeyMediaPrev,
		wl_kKeyMediaStop,
		wl_kKeyMediaPlayPause,
	};

	enum wl_EventType {
		wl_kEventTypeNone,
		wl_kEventTypeAction,
		wl_kEventTypeWindowCloseRequest,
		wl_kEventTypeWindowDestroyed,
		wl_kEventTypeWindowResized,
		wl_kEventTypeWindowRepaint,
		wl_kEventTypeTimer,
		wl_kEventTypeMouse,
		wl_kEventTypeKey,
		wl_kEventTypeDrop,
		wl_kEventTypeDragRender, // generate drag/clip data right when it's needed
		wl_kEventTypeClipboardClear, // let the app know it's safe to clear whatever it had copied
		//
		wl_kEventTypePlatformSpecific = 4999,
		// no #ifdef WL_PLATFORM_WINDOWS here because Dlang won't let us use version() in an enum,
		// so all the platform specific stuff will exist together
		wl_kEventTypeD2DTargetRecreated, // so the clients know to purge any device-dependent resources
		//
		wl_kEventTypeUserBegin = 9999
	};

	enum wl_MouseEventType {
		wl_kMouseEventTypeMouseDown,
		wl_kMouseEventTypeMouseUp,
		wl_kMouseEventTypeMouseMove,
		wl_kMouseEventTypeMouseEnter,
		wl_kMouseEventTypeMouseLeave,
		wl_kMouseEventTypeMouseWheel
	};

	enum wl_MouseButton {
		wl_kMouseButtonNone,
		wl_kMouseButtonLeft,
		wl_kMouseButtonMiddle,
		wl_kMouseButtonRight,
		wl_kMouseButtonOther
	};

	enum wl_MouseWheelAxis {
		wl_kMouseWheelAxisNone,
		wl_kMouseWheelAxisVertical,
		wl_kMouseWheelAxisHorizontal
	};

	enum wl_KeyEventType {
		wl_kKeyEventTypeUp,
		wl_kKeyEventTypeDown,
		wl_kKeyEventTypeChar
	};

	enum wl_Modifiers {
		wl_kModifierShift = 1 << 0,
		wl_kModifierControl = 1 << 1, // command on mac
		wl_kModifierAlt = 1 << 2,
		wl_kModifierMacControl = 1 << 3,
	};

	enum wl_KeyLocation {
		wl_kKeyLocationDefault,
		wl_kKeyLocationLeft,
		wl_kKeyLocationRight,
		wl_kKeyLocationNumPad
	};

	enum wl_DropEventType {
		wl_kDropEventTypeFeedback,
		wl_kDropEventTypeDrop
	};

	enum wl_DropEffect {
		wl_kDropEffectNone = 0,
		wl_kDropEffectCopy = 1 << 0,
		wl_kDropEffectMove = 1 << 1,
		wl_kDropEffectLink = 1 << 2,
		wl_kDropEffectOther = 1 << 3 // ask / private / etc
	};

	enum wl_CursorStyle {
		wl_kCursorStyleDefault,
		wl_kCursorStyleResizeLeftRight,
		wl_kCursorStyleResizeUpDown,
		wl_kCursorStyleIBeam,
	};
    
    // used in the repaint event - placed here to declutter the wl_RepaintEvent struct a bit
    struct wl_PlatformContext {
#ifdef WL_PLATFORM_WINDOWS
		union {
			struct {
				ID2D1Factory *factory;
				ID2D1RenderTarget *target;
			} d2d;
			struct {
				HDC hdc;
			} gdi;
		};
#elif defined WL_PLATFORM_MACOS
        CGContextRef context;
#elif defined WL_PLATFORM_LINUX
        cairo_t *cr;
#else
        int reserved;
#endif
    };

	struct wl_ActionEvent {
		wl_ActionRef action;
		int id;
	};
	struct wl_CloseRequestEvent {
		bool cancelClose;
	};
	struct wl_DestroyEvent {
		int reserved;
	};
	struct wl_ResizeEvent {
		int oldWidth, oldHeight;
		int newWidth, newHeight;
	};
	struct wl_RepaintEvent {
        struct wl_PlatformContext platformContext;
		int x, y, width, height; // affected area
	};
	struct wl_TimerEvent {
		wl_TimerRef timer;
		int timerID;
		bool stopTimer;
		double secondsSinceLast;
	};
	struct wl_MouseEvent {
		enum wl_MouseEventType eventType;
		int x, y;
		int wheelDelta;
		enum wl_MouseWheelAxis wheelAxis;
		enum wl_MouseButton button;
		unsigned int modifiers;
	};
	struct wl_KeyEvent {
		enum wl_KeyEventType eventType; // down, up, char
		enum wl_KeyEnum key; // only for down/up events
		const char *string; // only for char events
		unsigned int modifiers;
		enum wl_KeyLocation location; // left / right / etc
	};
	struct wl_DropEvent {
		enum wl_DropEventType eventType; // all have the same data (so no union below)
		wl_DropDataRef data;
		unsigned int modifiers;
		int x, y;
		enum wl_DropEffect defaultModifierAction; // platform-specific suggestion based on modifier combination
		unsigned int allowedEffectMask; // from wl_DropEffect enum
	};
	struct wl_DragRenderEvent {
		const char *dragFormat; // requested format
		wl_RenderPayloadRef payload; // target for rendering methods
	};
	struct wl_ClipboardClearEvent {
		int reserved;
	};
#ifdef WL_PLATFORM_WINDOWS
	struct wl_D2DTargetRecreatedEvent {
		ID2D1RenderTarget *newTarget;
		ID2D1RenderTarget *oldTarget; // optional
	};
#endif

	struct wl_Event {
		wl_EventPrivateRef _private;
		enum wl_EventType eventType;
		bool handled;
		union {
			struct wl_ActionEvent actionEvent;
			struct wl_CloseRequestEvent closeRequestEvent;
			struct wl_DestroyEvent destroyEvent;
			struct wl_ResizeEvent resizeEvent;
			struct wl_RepaintEvent repaintEvent;
			struct wl_TimerEvent timerEvent;
			struct wl_MouseEvent mouseEvent;
			struct wl_KeyEvent keyEvent;
			struct wl_DropEvent dropEvent;
			struct wl_DragRenderEvent dragRenderEvent;
			struct wl_ClipboardClearEvent clipboardClearEvent;
#ifdef WL_PLATFORM_WINDOWS
			struct wl_D2DTargetRecreatedEvent d2dTargetRecreatedEvent;
#endif
		};
	};

	enum wl_WindowPropertyEnum {
		wl_kWindowPropMinWidth = 1,
		wl_kWindowPropMinHeight = 1 << 1,
		wl_kWindowPropMaxWidth = 1 << 2,
		wl_kWindowPropMaxHeight = 1 << 3,
		wl_kWindowPropStyle = 1 << 4,
		wl_kWindowPropNativeParent = 1 << 5
	};

	enum wl_WindowStyleEnum {
		wl_kWindowStyleDefault,
		wl_kWindowStyleFrameless,
        wl_kWindowStylePluginWindow // for VST/AU/etc
	};

	struct wl_WindowProperties {
		unsigned int usedFields; // set of wl_WindowPropertyEnum
		// ======
		int minWidth, minHeight;
		int maxWidth, maxHeight;
        enum wl_WindowStyleEnum style;

        // stuff related to the AttachToNative mode (at this time, for audio plugin GUIs)
#ifdef WL_PLATFORM_WINDOWS
        HWND nativeParent; // only used when style = pluginWindow - wl_kWindowPropParent must also be set in used fields
#elif defined WL_PLATFORM_MACOS
        struct {
            void *nsView; // wl_WindowRef returned will be a dummy window, this is the good stuff
                          // void* for now, because pulling Objective-C headers into this file (for NSView) causes problems
        } outParams;
#endif
	};

	enum wl_Platform {
		wl_kPlatformWindows,
		wl_kPlatformLinux,
		wl_kPlatformMac
	};

	struct wl_Files {
		const char **filenames;
		int numFiles;
	};

	typedef int(CDECL *wl_EventCallback)(wl_WindowRef window, struct wl_Event *event, void *userData); // akin to win32 wndproc, handles everything
	typedef void (CDECL *wl_VoidCallback)(void *data);

	OPENWL_API enum wl_Platform wl_GetPlatform();

	/* application api */
	struct wl_PlatformOptions {
#ifdef WL_PLATFORM_WINDOWS
		bool useDirect2D; // instead of GDI
		struct {
			ID2D1Factory *factory; // filled by wl_Init()
		} outParams;
#elif defined WL_PLATFORM_MACOS
        bool pluginSlaveMode; // special wl_Init() mode, for when there's an existing runloop and we're only creating NSViews
#else
        int reserved;
#endif
	};
	OPENWL_API int CDECL wl_Init(wl_EventCallback callback, struct wl_PlatformOptions *options);
	OPENWL_API int CDECL wl_Runloop();
	OPENWL_API void CDECL wl_ExitRunloop(); // posts a message to exit runloop, returns immediately
	OPENWL_API void CDECL wl_Shutdown();

	/* window api */
	OPENWL_API wl_WindowRef CDECL wl_WindowCreate(int width, int height, const char *title, void *userData, struct wl_WindowProperties *props);
	OPENWL_API void CDECL wl_WindowDestroy(wl_WindowRef window);
	OPENWL_API void CDECL wl_WindowShow(wl_WindowRef window);
	OPENWL_API void CDECL wl_WindowShowRelative(wl_WindowRef window, wl_WindowRef relativeTo, int x, int y, int newWidth, int newHeight); // for pop-up windows
	OPENWL_API void CDECL wl_WindowHide(wl_WindowRef window);
	OPENWL_API void CDECL wl_WindowInvalidate(wl_WindowRef window, int x, int y, int width, int height);
	OPENWL_API size_t CDECL wl_WindowGetOSHandle(wl_WindowRef window);

	OPENWL_API void CDECL wl_WindowSetFocus(wl_WindowRef window);
	OPENWL_API void CDECL wl_MouseGrab(wl_WindowRef window);
	OPENWL_API void CDECL wl_MouseUngrab();

	/* cursor api */
	OPENWL_API wl_CursorRef CDECL wl_CursorCreate(wl_CursorStyle style);
	OPENWL_API void CDECL wl_WindowSetCursor(wl_WindowRef window, wl_CursorRef cursor); // null to clear

	/* timer API */
	OPENWL_API wl_TimerRef CDECL wl_TimerCreate(wl_WindowRef window, int timerID, unsigned int msTimeout);
	OPENWL_API void CDECL wl_TimerDestroy(wl_TimerRef timer);

	/* action API */
	OPENWL_API wl_IconRef CDECL wl_IconLoadFromFile(const char *filename, int sizeToWidth);
	OPENWL_API wl_AcceleratorRef CDECL wl_AccelCreate(enum wl_KeyEnum key, unsigned int modifiers);
	OPENWL_API wl_ActionRef CDECL wl_ActionCreate(int id, const char *label, wl_IconRef icon, wl_AcceleratorRef accel);

	/* menu API */
	OPENWL_API wl_MenuRef CDECL wl_MenuCreate(); // for menu bars or standalone popups
	OPENWL_API wl_MenuItemRef CDECL wl_MenuAddAction(wl_MenuRef menu, wl_ActionRef action);
	OPENWL_API wl_MenuItemRef CDECL wl_MenuAddSubmenu(wl_MenuRef menu, const char *label, wl_MenuRef sub);
	OPENWL_API void CDECL wl_MenuAddSeparator(wl_MenuRef menu);
	OPENWL_API wl_MenuItemRef CDECL wl_MenuBarAddMenu(wl_MenuBarRef menuBar, const char *label, wl_MenuRef menu);
	OPENWL_API void CDECL wl_WindowShowContextMenu(wl_WindowRef window, int x, int y, wl_MenuRef menu, struct wl_Event *fromEvent);
#ifdef WL_PLATFORM_MACOS
	// Mac-specific menu stuff (sigh)
	OPENWL_API wl_MenuBarRef CDECL wl_MenuBarGetDefault();
	OPENWL_API wl_MenuRef CDECL wl_GetApplicationMenu(); // special section of menu for Macs (prefs, quit, etc)
#else
	OPENWL_API wl_MenuBarRef CDECL wl_MenuBarCreate();
	OPENWL_API void CDECL wl_WindowSetMenuBar(wl_WindowRef window, wl_MenuBarRef menuBar);
#endif

	/* DND API */
	extern OPENWL_API const char *wl_kDragFormatUTF8;
	extern OPENWL_API const char *wl_kDragFormatFiles;
	// user can provide anything else with a custom mime type

	// drag source methods
	OPENWL_API wl_DragDataRef CDECL wl_DragDataCreate(wl_WindowRef forWindow);
	OPENWL_API void CDECL wl_DragDataRelease(wl_DragDataRef *dragData);
	OPENWL_API void CDECL wl_DragAddFormat(wl_DragDataRef dragData, const char *dragFormatMIME); // drag source: we're capable of generating this format
	OPENWL_API enum wl_DropEffect CDECL wl_DragExec(wl_DragDataRef dragData, unsigned int dropActionsMask, struct wl_Event *fromEvent); // modal / blocking

	// drop target methods
	OPENWL_API bool CDECL wl_DropHasFormat(wl_DropDataRef dropData, const char *dropFormatMIME); // drag target: testing for availability of this format
	// wl_DropGetFormat merely gets a pointer to data that is owned by the wl_DropDataRef - and it is only valid as long as the wl_DropDataRef is (typically for the duration of the callback)
	OPENWL_API bool CDECL wl_DropGetFormat(wl_DropDataRef dropData, const char *dropFormatMIME, const void **data, size_t *dataSize);
	// the wl_Files* is owned by the wl_DropDataRef (points to internal wl_DropDataRef structure) - onyl valid for life of dropData
	OPENWL_API bool CDECL wl_DropGetFiles(wl_DropDataRef dropData, const struct wl_Files **files);

	// clip/drop data rendering
	OPENWL_API void CDECL wl_DragRenderUTF8(wl_RenderPayloadRef payload, const char *text);
	OPENWL_API void CDECL wl_DragRenderFiles(wl_RenderPayloadRef payload, const struct wl_Files *files);
	OPENWL_API void CDECL wl_DragRenderFormat(wl_RenderPayloadRef payload, const char *formatMIME, const void *data, size_t dataSize);

	OPENWL_API void wl_WindowEnableDrops(wl_WindowRef window, bool enabled); // start/stop receiving drop events

	/* CLIPBOARD API */
	OPENWL_API void CDECL wl_ClipboardSet(wl_DragDataRef dragData);
	OPENWL_API wl_DropDataRef CDECL wl_ClipboardGet();
	OPENWL_API void CDECL wl_ClipboardRelease(wl_DropDataRef dropData);
	OPENWL_API void CDECL wl_ClipboardFlush();

	/* MISC */
	OPENWL_API void CDECL wl_ExecuteOnMainThread(wl_WindowRef window, wl_VoidCallback callback, void *data);
	OPENWL_API void CDECL wl_Sleep(unsigned int millis);
	OPENWL_API size_t CDECL wl_SystemMillis();

#ifdef __cplusplus
}
#endif


#endif // OPENWL_LIBRARY_H

