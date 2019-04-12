#ifndef OPENWL_LIBRARY_H
#define OPENWL_LIBRARY_H

#ifdef _WIN32
#   define WL_PLATFORM_WINDOWS
#elif defined __linux__
#   define WL_PLATFORM_LINUX
#elif defined __APPLE__ // could also use TargetConditionals.h ?
#   define WL_PLATFORM_APPLE
#elif defined __HAIKU__
#   define WL_PLATFORM_HAIKU
#endif

#ifdef WL_PLATFORM_WINDOWS
#	include <Windows.h>
#   include <d2d1_1.h>
#   include <dwrite.h>
#   ifdef OPENWL_EXPORTS
#       define OPENWL_API __declspec(dllexport)
#   else
#       define OPENWL_API __declspec(dllimport)
#   endif
//#   define CDECL __cdecl
#elif defined WL_PLATFORM_LINUX || defined WL_PLATFORM_APPLE || defined WL_PLATFORM_HAIKU
#   define OPENWL_API __attribute__((visibility("default")))
#   define CDECL
#endif

#include <stddef.h>

#define WLHANDLE(x) struct _##x; typedef struct _##x* x
// e.g. struct _wlWindow; typedef struct _wlWindow* wlWindow;

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

	WLHANDLE(wlWindow);
	WLHANDLE(wlTimer);
	WLHANDLE(wlMenuBar);
	WLHANDLE(wlMenu);
	WLHANDLE(wlMenuItem);
	WLHANDLE(wlIcon);
	WLHANDLE(wlAccelerator); // keyboard shortcuts
	WLHANDLE(wlAction); // keyboard shortcut or menu happening
	WLHANDLE(wlDragData); // dnd source
	WLHANDLE(wlDropData); // dnd dest
	WLHANDLE(wlEventPrivate); // implementation-specific private WLEvent fields
	WLHANDLE(wlRenderPayload); // drag source rendering payload -- platform-specific, use rendering functions to modify it

	enum WLKeyEnum {
		WLKey_Unknown,
		//
		WLKey_Escape,
		WLKey_Tab,
		WLKey_Backspace,
		WLKey_Return,
		WLKey_Space,
		//
		WLKey_F1,
		WLKey_F2,
		WLKey_F3,
		WLKey_F4,
		WLKey_F5,
		WLKey_F6,
		WLKey_F7,
		WLKey_F8,
		WLKey_F9,
		WLKey_F10,
		WLKey_F11,
		WLKey_F12,
		WLKey_F13,
		WLKey_F14,
		WLKey_F15,
		WLKey_F16,
		WLKey_F17,
		WLKey_F18,
		WLKey_F19,
		//
		WLKey_0,
		WLKey_1,
		WLKey_2,
		WLKey_3,
		WLKey_4,
		WLKey_5,
		WLKey_6,
		WLKey_7,
		WLKey_8,
		WLKey_9,
		//
		WLKey_A,
		WLKey_B,
		WLKey_C,
		WLKey_D,
		WLKey_E,
		WLKey_F,
		WLKey_G,
		WLKey_H,
		WLKey_I,
		WLKey_J,
		WLKey_K,
		WLKey_L,
		WLKey_M,
		WLKey_N,
		WLKey_O,
		WLKey_P,
		WLKey_Q,
		WLKey_R,
		WLKey_S,
		WLKey_T,
		WLKey_U,
		WLKey_V,
		WLKey_W,
		WLKey_X,
		WLKey_Y,
		WLKey_Z,
		// modifiers
		WLKey_Control,
		WLKey_Shift,
		WLKey_AltOption,
		WLKey_WinCommand,
		WLKey_Fn,
		// home/end block
		WLKey_Insert,
		WLKey_Delete,
		WLKey_PageUp,
		WLKey_PageDown,
		WLKey_Home,
		WLKey_End,
		// arrow keys
		WLKey_LeftArrow,
		WLKey_UpArrow,
		WLKey_RightArrow,
		WLKey_DownArrow,
		// keypad numbers
		WLKey_KP_0,
		WLKey_KP_1,
		WLKey_KP_2,
		WLKey_KP_3,
		WLKey_KP_4,
		WLKey_KP_5,
		WLKey_KP_6,
		WLKey_KP_7,
		WLKey_KP_8,
		WLKey_KP_9,
		// keypad ops
		WLKey_KP_Clear,
		WLKey_KP_Equals,
		WLKey_KP_Divide,
		WLKey_KP_Multiply,
		WLKey_KP_Subtract,
		WLKey_KP_Add,
		WLKey_KP_Enter,
		WLKey_KP_Decimal,
		// locks
		WLKey_CapsLock,
		WLKey_NumLock,
		WLKey_ScrollLock,
		// misc
		WLKey_PrintScreen,
		WLKey_Pause,  // Pause/Break button minus Ctrl
		WLKey_Cancel, // Ctrl-Break
		// media
		WLKey_MediaMute,
		WLKey_MediaVolumeDown,
		WLKey_MediaVolumeUp,
		WLKey_MediaNext,
		WLKey_MediaPrev,
		WLKey_MediaStop,
		WLKey_MediaPlayPause,
	};

	enum WLEventType {
		WLEventType_None,
		WLEventType_Action,
		WLEventType_WindowCloseRequest,
		WLEventType_WindowDestroyed,
		WLEventType_WindowResized,
		WLEventType_WindowRepaint,
		WLEventType_Timer,
		WLEventType_Mouse,
		WLEventType_Key,
		WLEventType_Drop,
		WLEventType_DragRender, // generate drag/clip data right when it's needed
		WLEventType_ClipboardClear, // let the app know it's safe to clear whatever it had copied
		//
		WLEventType_PlatformSpecific = 4999,
		// no #ifdef WL_PLATFORM_WINDOWS here because Dlang won't let us use version() in an enum,
		// so all the platform specific stuff will exist together
		WLEventType_D2DTargetRecreated, // so the clients know to purge any device-dependent resources
		//
		WLEventType_UserBegin = 9999
	};

	enum WLMouseEventType {
		WLMouseEventType_MouseDown,
		WLMouseEventType_MouseUp,
		WLMouseEventType_MouseMove,
		WLMouseEventType_MouseEnter,
		WLMouseEventType_MouseLeave,
		WLMouseEventType_MouseClick, // down then up within a certain spatial / time range
		WLMouseEventType_MouseWheel
	};

	enum WLMouseButton {
		WLMouseButton_None,
		WLMouseButton_Left,
		WLMouseButton_Middle,
		WLMouseButton_Right,
		WLMouseButton_Other
	};

	enum WLKeyEventType {
		WLKeyEventType_Up,
		WLKeyEventType_Down,
		WLKeyEventType_Char
	};

	enum WLModifiers {
		WLModifier_Shift = 1 << 0,
		WLModifier_Control = 1 << 1, // command on mac
		WLModifier_Alt = 1 << 2,
		WLModifier_MacControl = 1 << 3,
	};

	enum WLKeyLocation {
		WLKeyLocation_Default,
		WLKeyLocation_Left,
		WLKeyLocation_Right,
		WLKeyLocation_NumPad
	};

	enum WLDropEventType {
		WLDropEventType_Feedback,
		WLDropEventType_Drop
	};

	enum WLDropEffect {
		WLDropEffect_None = 0,
		WLDropEffect_Copy = 1 << 0,
		WLDropEffect_Move = 1 << 1,
		WLDropEffect_Link = 1 << 2,
		WLDropEffect_Other = 1 << 3 // ask / private / etc
	};

#ifdef WL_PLATFORM_WINDOWS
	struct WLPlatformContextD2D { // special case for the platformContext void*
		ID2D1Factory *factory;
		ID2D1RenderTarget *target;
	};
#endif

	struct WLActionEvent {
		wlAction action;
		int id;
	};
	struct WLCloseRequestEvent {
		bool cancelClose;
	};
	struct WLDestroyEvent {
		int reserved;
	};
	struct WLResizeEvent {
		int oldWidth, oldHeight;
		int newWidth, newHeight;
	};
	struct WLRepaintEvent {
		void *platformContext; // HDC, CGContextRef, cairo_t, WLPlatformContextD2D *, etc
		int x, y, width, height; // affected area
	};
	struct WLTimerEvent {
		wlTimer timer;
		int timerID;
		bool stopTimer;
		double secondsSinceLast;
	};
	struct WLMouseEvent {
		enum WLMouseEventType eventType;
		int x, y;
		int wheelDelta;
		enum WLMouseButton button;
		unsigned int modifiers;
	};
	struct WLKeyEvent {
		enum WLKeyEventType eventType; // down, up, char
		enum WLKeyEnum key; // only for down/up events
		const char *string; // only for char events
		unsigned int modifiers;
		enum WLKeyLocation location; // left / right / etc
	};
	struct WLDropEvent {
		enum WLDropEventType eventType; // all have the same data (so no union below)
		wlDropData data;
		unsigned int modifiers;
		int x, y;
		enum WLDropEffect defaultModifierAction; // platform-specific suggestion based on modifier combination
		unsigned int allowedEffectMask; // from WLDropEffect enum
	};
	struct WLDragRenderEvent {
		const char *dragFormat; // requested format
		wlRenderPayload payload; // target for rendering methods
	};
	struct WLClipboardClearEvent {
		int reserved;
	};
#ifdef WL_PLATFORM_WINDOWS
	struct WLD2DTargetRecreatedEvent {
		ID2D1RenderTarget *newTarget;
		ID2D1RenderTarget *oldTarget; // optional
	};
#endif

	struct WLEvent {
		wlEventPrivate _private;
		enum WLEventType eventType;
		bool handled;
		union {
			struct WLActionEvent actionEvent;
			struct WLCloseRequestEvent closeRequestEvent;
			struct WLDestroyEvent destroyEvent;
			struct WLResizeEvent resizeEvent;
			struct WLRepaintEvent repaintEvent;
			struct WLTimerEvent timerEvent;
			struct WLMouseEvent mouseEvent;
			struct WLKeyEvent keyEvent;
			struct WLDropEvent dropEvent;
			struct WLDragRenderEvent dragRenderEvent;
			struct WLClipboardClearEvent clipboardClearEvent;
#ifdef WL_PLATFORM_WINDOWS
			struct WLD2DTargetRecreatedEvent d2dTargetRecreatedEvent;
#endif
		};
	};

	enum WLWindowPropertyEnum {
		WLWindowProp_MinWidth = 1,
		WLWindowProp_MinHeight = 1 << 1,
		WLWindowProp_MaxWidth = 1 << 2,
		WLWindowProp_MaxHeight = 1 << 3,
		WLWindowProp_Style = 1 << 4,
	};

	enum WLWindowStyleEnum {
		WLWindowStyle_Default,
		WLWindowStyle_Frameless
	};

	struct WLWindowProperties {
		unsigned int usedFields; // set of WLWindowPropertyEnum
		// ======
		int minWidth, minHeight;
		int maxWidth, maxHeight;
		enum WLWindowStyleEnum style;
	};

	enum WLPlatform {
		WLPlatform_Windows,
		WLPlatform_Linux,
		WLPlatform_Mac
	};

	struct WLFiles {
		const char **filenames;
		int numFiles;
	};

	typedef int(CDECL *wlEventCallback)(wlWindow window, struct WLEvent *event, void *userData); // akin to win32 wndproc, handles everything
	typedef void (CDECL *wlVoidCallback)(void *data);

	OPENWL_API enum WLPlatform wlGetPlatform();

	/* application api */
	struct WLPlatformOptions {
		int reserved;
#ifdef WL_PLATFORM_WINDOWS
		bool useDirect2D; // instead of GDI
		struct {
			ID2D1Factory *factory; // filled by wlInit()
		} outParams;
#endif
	};
	OPENWL_API int CDECL wlInit(wlEventCallback callback, struct WLPlatformOptions *options);
	OPENWL_API int CDECL wlRunloop();
	OPENWL_API void CDECL wlExitRunloop(); // posts a message to exit runloop, returns immediately
	OPENWL_API void CDECL wlShutdown();

	/* window api */
	OPENWL_API wlWindow CDECL wlWindowCreate(int width, int height, const char *title, void *userData, struct WLWindowProperties *props);
	OPENWL_API void CDECL wlWindowDestroy(wlWindow window);
	OPENWL_API void CDECL wlWindowShow(wlWindow window);
	OPENWL_API void CDECL wlWindowShowRelative(wlWindow window, wlWindow relativeTo, int x, int y, int newWidth, int newHeight); // for pop-up windows
	OPENWL_API void CDECL wlWindowHide(wlWindow window);
	OPENWL_API void CDECL wlWindowInvalidate(wlWindow window, int x, int y, int width, int height);
	OPENWL_API size_t CDECL wlWindowGetOSHandle(wlWindow window);

	OPENWL_API void CDECL wlWindowSetFocus(wlWindow window);
	OPENWL_API void CDECL wlMouseGrab(wlWindow window);
	OPENWL_API void CDECL wlMouseUngrab();

	/* timer API */
	OPENWL_API wlTimer CDECL wlTimerCreate(wlWindow window, int timerID, unsigned int msTimeout);
	OPENWL_API void CDECL wlTimerDestroy(wlTimer timer);

	/* action API */
	OPENWL_API wlIcon CDECL wlIconLoadFromFile(const char *filename, int sizeToWidth);
	OPENWL_API wlAccelerator CDECL wlAccelCreate(enum WLKeyEnum key, unsigned int modifiers);
	OPENWL_API wlAction CDECL wlActionCreate(int id, const char *label, wlIcon icon, wlAccelerator accel);

	/* menu API */
	OPENWL_API wlMenu CDECL wlMenuCreate(); // for menu bars or standalone popups
	OPENWL_API wlMenuItem CDECL wlMenuAddAction(wlMenu menu, wlAction action);
	OPENWL_API wlMenuItem CDECL wlMenuAddSubmenu(wlMenu menu, const char *label, wlMenu sub);
	OPENWL_API void CDECL wlMenuAddSeparator(wlMenu menu);
	OPENWL_API wlMenuItem CDECL wlMenuBarAddMenu(wlMenuBar menuBar, const char *label, wlMenu menu);
	OPENWL_API void CDECL wlWindowShowContextMenu(wlWindow window, int x, int y, wlMenu menu, struct WLEvent *fromEvent);
#ifdef WL_PLATFORM_APPLE
	// Mac-specific menu stuff (sigh)
	OPENWL_API wlMenuBar CDECL wlMenuBarGetDefault();
	OPENWL_API wlMenu CDECL wlGetApplicationMenu(); // special section of menu for Macs (prefs, quit, etc)
#else
	OPENWL_API wlMenuBar CDECL wlMenuBarCreate();
	OPENWL_API void CDECL wlWindowSetMenuBar(wlWindow window, wlMenuBar menuBar);
#endif

	/* DND API */
	extern OPENWL_API const char *kWLDragFormatUTF8;
	extern OPENWL_API const char *kWLDragFormatFiles;
	// user can provide anything else with a custom mime type

	// drag source methods
	OPENWL_API wlDragData CDECL wlDragDataCreate(wlWindow forWindow);
	OPENWL_API void CDECL wlDragDataRelease(wlDragData *dragData);
	OPENWL_API void CDECL wlDragAddFormat(wlDragData dragData, const char *dragFormatMIME); // drag source: we're capable of generating this format
	OPENWL_API enum WLDropEffect CDECL wlDragExec(wlDragData dragData, unsigned int dropActionsMask, struct WLEvent *fromEvent); // modal / blocking

	// drop target methods
	OPENWL_API bool CDECL wlDropHasFormat(wlDropData dropData, const char *dropFormatMIME); // drag target: testing for availability of this format
	// wlDropGetFormat merely gets a pointer to data that is owned by the wlDropData - and it is only valid as long as the wlDropData is (typically for the duration of the callback)
	OPENWL_API bool CDECL wlDropGetFormat(wlDropData dropData, const char *dropFormatMIME, const void **data, size_t *dataSize);
	// the WLFiles* is owned by the wlDropData (points to internal wlDropData structure) - onyl valid for life of dropData
	OPENWL_API bool CDECL wlDropGetFiles(wlDropData dropData, const struct WLFiles **files);

	// clip/drop data rendering
	OPENWL_API void CDECL wlDragRenderUTF8(wlRenderPayload payload, const char *text);
	OPENWL_API void CDECL wlDragRenderFiles(wlRenderPayload payload, const struct WLFiles *files);
	OPENWL_API void CDECL wlDragRenderFormat(wlRenderPayload payload, const char *formatMIME, const void *data, size_t dataSize);

	OPENWL_API void wlWindowEnableDrops(wlWindow window, bool enabled); // start/stop receiving drop events

	/* CLIPBOARD API */
	OPENWL_API void CDECL wlClipboardSet(wlDragData dragData);
	OPENWL_API wlDropData CDECL wlClipboardGet();
	OPENWL_API void CDECL wlClipboardRelease(wlDropData dropData);
	OPENWL_API void CDECL wlClipboardFlush();

	/* MISC */
	OPENWL_API void CDECL wlExecuteOnMainThread(wlWindow window, wlVoidCallback callback, void *data);
	OPENWL_API void CDECL wlSleep(unsigned int millis);
	OPENWL_API size_t CDECL wlSystemMillis();

#ifdef __cplusplus
}
#endif


#endif // OPENWL_LIBRARY_H

