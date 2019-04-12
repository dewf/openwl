#include <stdio.h>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include <time.h>
#define NUM_THREADS 6

#include "main.h"

// file menu
#define FILE_ACTION_1 1
#define FILE_ACTION_2 2
#define EXIT_ACTION 3
wlAction fileAction1;
wlAction fileAction2;
wlAction exitAction;

// edit menu
#define COPY_ACTION 4
#define PASTE_ACTION 5
wlAction copyAction;
wlAction pasteAction;

// help menu
#define HELP_ACTION_1 6
wlAction helpAction1;

// context menu
#define CONTEXT_ACTION_1 7
#define CONTEXT_ACTION_2 8
#define CONTEXT_ACTION_3 9
#define CONTEXT_ACTION_4 10
wlAction contextAction1;
wlAction contextAction2;
wlAction contextAction3;
wlAction contextAction4;
wlMenu contextMenu;

int lastFrame = 0;
int totalFrames = 0;

int width, height;

#define FAST_TIMER 1
#define SLOW_TIMER 2
wlTimer fastTimer, slowTimer;

wlWindow mainWindow;
wlWindow framelessWindow;
bool framelessWindowVisible = false;

bool mouseDown = false;
int dragStartX, dragStartY;

bool pointInRect(int px, int py, int x, int y, int w, int h) {
	return (px >= x && px < (x + w) && py >= y && py < (y + h));
}
float pointDist(int x1, int y1, int x2, int y2) {
	return (float)sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

inline bool strEqual(const char *a, const char *b) {
	return !strcmp(a, b);
}

int CDECL eventCallback(wlWindow window, WLEvent *event, void *userData) {
	event->handled = true;
	switch (event->eventType) {
	case WLEventType_WindowCloseRequest:
		if (window == mainWindow) {
			printf("main window closing ... or is it?\n");
			//event->closeRequestEvent.cancelClose = true;
		}
		else {
			printf("closed something else, staying\n");
		}
		break;
	case WLEventType_WindowDestroyed:
		if (window == mainWindow) {
			printf("main window destroyed! exiting runloop\n");
			wlExitRunloop();
		}
		break;
	case WLEventType_Action:
		printf("action %zx chosen\n", (size_t)event->actionEvent.action);
		if (event->actionEvent.action == exitAction) {
			wlWindowDestroy(window); // app will close when destroy message received (see above)
		}
		else if (event->actionEvent.action == copyAction) {
			auto clipData = wlDragDataCreate(window);
			wlDragAddFormat(clipData, kWLDragFormatUTF8);
			wlClipboardSet(clipData);
			printf("clipboard copy done\n");
			wlDragDataRelease(&clipData);
		}
		else if (event->actionEvent.action == pasteAction) {
			auto clipData = wlClipboardGet();
			if (wlDropHasFormat(clipData, kWLDragFormatFiles)) {
				const WLFiles *files;
				wlDropGetFiles(clipData, &files);
				for (int i = 0; i < files->numFiles; i++) {
					printf("Got file: [%s]\n", files->filenames[i]);
				}
			}
			else if (wlDropHasFormat(clipData, kWLDragFormatUTF8)) {
				const char *text;
				size_t textSize;
				if (wlDropGetFormat(clipData, kWLDragFormatUTF8, (const void **)&text, &textSize)) {
					printf("got clipboard text: [%s]\n", text);
				}
			}
			wlClipboardRelease(clipData);
		}
		break;

	case WLEventType_WindowRepaint:
		if (window == mainWindow) {
			platformDraw(event->repaintEvent.platformContext);
		}
		else if (window == framelessWindow) {
			platformDrawFrameless(event->repaintEvent.platformContext);
		}
		break;

	case WLEventType_WindowResized:
		if (window == mainWindow) {
			width = event->resizeEvent.newWidth;
			height = event->resizeEvent.newHeight;

			printf("new size: %d, %d\n", width, height);

			printf("TODO: remove resize inval\n");
			wlWindowInvalidate(window, 0, 0, 0, 0);

			lastFrame = 0;
		}
		break;

	case WLEventType_Timer:
		//printf("window %zu timer %zu event\n", window, event->timerEvent.timer);
		if (event->timerEvent.timer == fastTimer) {
			wlWindowInvalidate(window, 0, 0, 0, 0); // entire window
		}
		else if (event->timerEvent.timer == slowTimer) {
			//printf("%d frames\n", numFrames);
			totalFrames = lastFrame;
			lastFrame = 0;
		}
		break;

	case WLEventType_Mouse:
	{
		if (event->mouseEvent.eventType == WLMouseEventType_MouseDown) {
			if (event->mouseEvent.button == WLMouseButton_Left) {
				if (pointInRect(event->mouseEvent.x, event->mouseEvent.y, DRAG_SOURCE_X, DRAG_SOURCE_Y, DRAG_SOURCE_W, DRAG_SOURCE_H)) {
					// start drag, save position
					mouseDown = true;
					dragStartX = event->mouseEvent.x;
					dragStartY = event->mouseEvent.y;
					printf("(potentially) starting drag...\n");
				}
			}
			else if (event->mouseEvent.button == WLMouseButton_Right) {
				printf("mouse down event, button %d @ %d,%d\n", event->mouseEvent.button, event->mouseEvent.x, event->mouseEvent.y);
				wlWindowShowContextMenu(window, event->mouseEvent.x, event->mouseEvent.y, contextMenu, event);
			}
		}
		else if (event->mouseEvent.eventType == WLMouseEventType_MouseUp) {
			mouseDown = false;
		}
		else if (event->mouseEvent.eventType == WLMouseEventType_MouseMove) {
			// if dragging ...
			if (mouseDown) {
				if (pointDist(dragStartX, dragStartY, event->mouseEvent.x, event->mouseEvent.y) > 4.0) {
					auto dragData = wlDragDataCreate(window);

					wlDragAddFormat(dragData, kWLDragFormatUTF8);
					//                    wlDragAddFormat(dragData, wlDragFormatFiles);

					printf("starting drag ...\n");
					auto whichAction = wlDragExec(dragData, WLDropEffect_Copy | WLDropEffect_Move | WLDropEffect_Link, event);
					printf("selected dragexec action: %d\n", whichAction);
					mouseDown = false;
					wlDragDataRelease(&dragData);
					printf("drag complete\n");
				}
			}

			// frameless hover updates
			if (pointInRect(event->mouseEvent.x, event->mouseEvent.y, HOVER_HERE_X, HOVER_HERE_Y, HOVER_HERE_W, HOVER_HERE_H)) {
				// show/move no matter what
				auto x2 = event->mouseEvent.x - POPUP_WIDTH / 2;
				auto y2 = event->mouseEvent.y - (POPUP_HEIGHT + 50);
				wlWindowShowRelative(framelessWindow, mainWindow, x2, y2, 0, 0);
				framelessWindowVisible = true;
			}
			else {
				if (framelessWindowVisible) {
					wlWindowHide(framelessWindow);
					framelessWindowVisible = false;
				}
			}

		}
		else if (event->mouseEvent.eventType == WLMouseEventType_MouseEnter) {
			printf("==> mouse entered window\n");
		}
		else if (event->mouseEvent.eventType == WLMouseEventType_MouseLeave) {
			printf("<== mouse left window\n");
		}

		break;
	}

	case WLEventType_Key:
		switch (event->keyEvent.eventType) {
		case WLKeyEventType_Down:
		{
			const char *loc =
				event->keyEvent.location == WLKeyLocation_Default ? "Default" :
				(event->keyEvent.location == WLKeyLocation_Left ? "Left" :
				(event->keyEvent.location == WLKeyLocation_Right ? "Right" :
					(event->keyEvent.location == WLKeyLocation_NumPad ? "Numpad" : "Unknown")));
			printf("Key down: %d [%s] (mods %02X) (loc %s)\n", event->keyEvent.key, event->keyEvent.string, event->keyEvent.modifiers, loc);
			break;
		}
		case WLKeyEventType_Up:
			//printf(" - keyup: %d [%s]\n", event->keyEvent.key, event->keyEvent.string);
			break;
		case WLKeyEventType_Char:
			printf("CHAR: '%s'\n", event->keyEvent.string);
			break;
		}
		break;

	case WLEventType_DragRender:
		if (platformProvidesDragFormat(event->dragRenderEvent.dragFormat)) {
			platformRenderDragFormat(event->dragRenderEvent.payload, event->dragRenderEvent.dragFormat);
		}
		else if (strEqual(event->dragRenderEvent.dragFormat, kWLDragFormatUTF8)) {
			wlDragRenderUTF8(event->dragRenderEvent.payload, u8"<<Here's your ad-hoc generated text, woooot!!>>");
		}
		else if (strEqual(event->dragRenderEvent.dragFormat, kWLDragFormatFiles)) {
			WLFiles files;
			files.numFiles = 3;
			files.filenames = new const char*[3];
			files.filenames[0] = "/boot/home/Desktop/cool_bitmap";
			files.filenames[1] = "/boot/home/Desktop/text_file";
			files.filenames[2] = "/boot/home/Desktop/dragme";
			//
			wlDragRenderFiles(event->dragRenderEvent.payload, &files);
			// safe to delete
			delete files.filenames;
		}
		else {
			printf("#### drag source render - unhandled mime type");
			event->handled = false;
		}
		break;

	case WLEventType_Drop:
		switch (event->dropEvent.eventType) {
		case WLDropEventType_Feedback:
			if (pointInRect(event->dropEvent.x, event->dropEvent.y, DROP_TARGET_X, DROP_TARGET_Y, DROP_TARGET_W, DROP_TARGET_H))
			{
				if (wlDropHasFormat(event->dropEvent.data, kWLDragFormatUTF8) ||
					wlDropHasFormat(event->dropEvent.data, kWLDragFormatFiles) ||
					platformCheckDropFormats(event->dropEvent.data))
				{
					event->dropEvent.allowedEffectMask &= event->dropEvent.defaultModifierAction; // use platform-specific suggestion based on mod keys
				}
			}
			else {
				event->dropEvent.allowedEffectMask = WLDropEffect_None;
			}
			break;
		case WLDropEventType_Drop:
			// check final modifiers?
			event->dropEvent.allowedEffectMask &= event->dropEvent.defaultModifierAction; // why is this here?

			if (platformCheckDropFormats(event->dropEvent.data)) {
				// check the platform-specific stuff first, otherwise sometimes the wlDragFormatUTF8 handler grabs it first
				// (eg, haiku's translation kit offers to convert all kinds of things to text, which is silly)
				platformHandleDrop(event->dropEvent.data);
			}
			else if (wlDropHasFormat(event->dropEvent.data, kWLDragFormatFiles)) {
				const WLFiles *files;
				if (wlDropGetFiles(event->dropEvent.data, &files)) {
					for (int i = 0; i < files->numFiles; i++) {
						printf("Got file: [%s]\n", files->filenames[i]);
					}
				}
			}
			else if (wlDropHasFormat(event->dropEvent.data, kWLDragFormatUTF8)) {
				const char *text;
				size_t textSize;
				if (wlDropGetFormat(event->dropEvent.data, kWLDragFormatUTF8, (const void **)&text, &textSize)) {
					printf("got text: [%s]\n", text);
				}
				else {
					printf("did NOT get text - wlDropGetText failed\n");
				}
			}
			else {
				printf("unhandled format in WLDropEventType_Drop\n");
			}

			break;
		}
		break; // case WLEventType_DragEvent

	default:
		printf("unhandled wl callback event type: %d\n", event->eventType);
		event->handled = false;
	}
	return 0;
}

void createActions() {
	auto icon1 = wlIconLoadFromFile("_icons/tall.png", 64);
	auto icon2 = wlIconLoadFromFile("_icons/wide.png", 64);
	//auto icon1 = wlIconLoadFromFile("_icons/horse.png", 16);
	//auto icon2 = wlIconLoadFromFile("_icons/laptop.png", 16);

	fileAction1 = wlActionCreate(FILE_ACTION_1, "Something", icon1, 0);

	// user uppercase key enums here because that's required for win32 virtual key codes,
	//  and doesn't require any special handling when generating labels (toupper etc)
	auto fa2Accel = wlAccelCreate(WLKey_K, WLModifier_Alt);
	fileAction2 = wlActionCreate(FILE_ACTION_2, "SubMenuItem", 0, fa2Accel);

	auto exitAccel = wlAccelCreate(WLKey_Q, WLModifier_Control);
	exitAction = wlActionCreate(EXIT_ACTION, "Quit c-client", 0, exitAccel);

	auto copyAccel = wlAccelCreate(WLKey_C, WLModifier_Control);
	copyAction = wlActionCreate(COPY_ACTION, "Copy", 0, copyAccel);

	auto pasteAccel = wlAccelCreate(WLKey_V, WLModifier_Control);
	pasteAction = wlActionCreate(PASTE_ACTION, "Paste", 0, pasteAccel);

	helpAction1 = wlActionCreate(HELP_ACTION_1, "Placeholder", icon2, 0);
	contextAction1 = wlActionCreate(CONTEXT_ACTION_1, "Context 01", 0, 0);
	contextAction2 = wlActionCreate(CONTEXT_ACTION_2, "Context 02", 0, 0);
	auto c3Accel = wlAccelCreate(WLKey_N, WLModifier_Shift);
	contextAction3 = wlActionCreate(CONTEXT_ACTION_3, "Context 03", 0, c3Accel);

	//auto c4Accel = wlAccelCreate(WLKey_OEM_4, WLModifier_Control);
	//contextAction4 = wlActionCreate(CONTEXT_ACTION_4, "Context WAT", 0, c4Accel);
}

void createMenu() {

	createActions();

#ifdef WL_PLATFORM_APPLE
	auto menuBar = wlMenuBarGetDefault();
	auto appMenu = wlGetApplicationMenu();
#else
	auto menuBar = wlMenuBarCreate();
#endif
	/* file menu */
	auto fileMenu = wlMenuCreate();
	wlMenuAddAction(fileMenu, fileAction1);

	auto fileSubMenu = wlMenuCreate();
	wlMenuAddAction(fileSubMenu, fileAction2);
	wlMenuAddSubmenu(fileMenu, "Su&bmenu", fileSubMenu);

#ifdef WL_PLATFORM_APPLE
	// app menu gets exit action on Apple platforms
	wlMenuAddAction(appMenu, exitAction);
#else
	// but it goes on the File menu elsewhere
	wlMenuAddSeparator(fileMenu);
	wlMenuAddAction(fileMenu, exitAction);
#endif
	wlMenuBarAddMenu(menuBar, "&File", fileMenu);

	/* edit menu */
	auto editMenu = wlMenuCreate();
	wlMenuAddAction(editMenu, copyAction);
	wlMenuAddAction(editMenu, pasteAction);
	wlMenuBarAddMenu(menuBar, "&Edit", editMenu);

	/* context menu (useful later) */
	contextMenu = wlMenuCreate();
	wlMenuAddAction(contextMenu, contextAction1);
	wlMenuAddAction(contextMenu, contextAction2);
	wlMenuAddSeparator(contextMenu);

	auto contextSubMenu = wlMenuCreate();
	wlMenuAddAction(contextSubMenu, contextAction3);
	wlMenuAddSubmenu(contextMenu, "Su&bmenu", contextSubMenu);
	/* end context menu*/

	wlMenuBarAddMenu(menuBar, "&Context", contextMenu);

	/* help menu */
	auto helpMenu = wlMenuCreate();
	wlMenuAddAction(helpMenu, helpAction1);

	wlMenuBarAddMenu(menuBar, "&Help", helpMenu);

	/* end */
#ifdef WL_PLATFORM_APPLE
	// nothing to do
#else
	wlWindowSetMenuBar(mainWindow, menuBar);
#endif
}

// this function is executed on the main thread,
// so no worries about locking the imageSurface or other shared things
void addNewBox(void *data) {
	auto threadID = (size_t)data;

	// generate a random square to draw
	int x = rand() % (width - BOXWIDTH);
	int y = rand() % (height - BOXWIDTH);
	float r, g, b;
	if ((threadID % 3) == 0) {
		r = 1.0;
		g = 0.0;
		b = 0;
	}
	else if ((threadID % 3) == 1) {
		r = 0.0;
		g = 1.0;
		b = 0.0;
	}
	else {
		r = 0.0;
		g = 0.0;
		b = 1.0;
	}
	RandomBox xbox = { r, g, b, x, y, BOXWIDTH, BOXWIDTH };
	auto box = &xbox;

	platformDrawBox(box);

	wlWindowInvalidate(mainWindow, box->x, box->y, box->width, box->height);
}

void bgThreadFunc(int threadID) {
	while (true) {
		size_t void_arg = threadID;
		wlExecuteOnMainThread(mainWindow, addNewBox, (void *)void_arg);
		wlSleep(200);
	}
}

int main(int argc, const char * argv[]) {
	srand((unsigned int)time(NULL));

	WLPlatformOptions opts = { 0 };
	wlInit(eventCallback, &opts);

	WLWindowProperties props = { 0 };
	props.usedFields = WLWindowProp_MinWidth | WLWindowProp_MinHeight | WLWindowProp_MaxWidth | WLWindowProp_MaxHeight;
	props.minWidth = 300;
	props.minHeight = 200;
	props.maxWidth = MAX_WIDTH;
	props.maxHeight = MAX_HEIGHT;

	width = 800;
	height = 600;

	mainWindow = wlWindowCreate(width, height, u8"hello there, cross-platform friend āǢʥϢ۩ใ ♥☺☼", nullptr, &props);

	props = { 0 };
	props.usedFields = WLWindowProp_Style;
	props.style = WLWindowStyle_Frameless;
	framelessWindow = wlWindowCreate(POPUP_WIDTH, POPUP_HEIGHT, nullptr, nullptr, &props);

	createMenu();

	platformInit();

	fastTimer = wlTimerCreate(mainWindow, FAST_TIMER, 16); // ~60fps
	slowTimer = wlTimerCreate(mainWindow, SLOW_TIMER, 1000);

	wlWindowEnableDrops(mainWindow, true);

	platformCreateThreads(bgThreadFunc, NUM_THREADS);

	wlWindowShow(mainWindow);

	wlRunloop();

	wlTimerDestroy(fastTimer);
	wlTimerDestroy(slowTimer);

	wlClipboardFlush();

	platformJoinThreads();
	platformShutdown();

	wlShutdown();
	return 0;
}
