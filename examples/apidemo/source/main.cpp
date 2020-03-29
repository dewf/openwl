#include <stdio.h>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include <time.h>
#include <assert.h>
#define NUM_THREADS 6

#include "main.h"

enum IDsEnum {
    // action IDs
    ID_FileAction1,
    ID_FileAction2,
	ID_FileAction3, // Open
	ID_FileAction4, // Save
    ID_ExitAction,
    ID_CopyAction,
    ID_PasteAction,
    ID_HelpAction,
    ID_ContextAction1,
    ID_ContextAction2,
    ID_ContextAction3,
    ID_ContextAction4,
    // timer IDs
    ID_FastTimer,
    ID_SlowTimer
};

// file menu
wl_ActionRef fileAction1;
wl_ActionRef fileAction2;
wl_ActionRef openAction;
wl_ActionRef saveAsAction;
wl_ActionRef exitAction;

// edit menu
wl_ActionRef copyAction;
wl_ActionRef pasteAction;

// help menu
wl_ActionRef helpAction1;

// context menu
wl_ActionRef contextAction1;
wl_ActionRef contextAction2;
wl_ActionRef contextAction3;
wl_ActionRef contextAction4;
wl_MenuRef contextMenu;

int lastFrame = 0;
int totalFrames = 0;

int width, height;

wl_TimerRef fastTimer, slowTimer;

wl_WindowRef mainWindow;
wl_WindowRef framelessWindow;
bool framelessWindowVisible = false;

wl_CursorRef customCursor;

bool dragging = false;
int dragStartX, dragStartY;

bool grabbed = false;

bool pointInRect(int px, int py, int x, int y, int w, int h) {
	return (px >= x && px < (x + w) && py >= y && py < (y + h));
}
float pointDist(int x1, int y1, int x2, int y2) {
	return (float)sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

inline bool strEqual(const char *a, const char *b) {
	return !strcmp(a, b);
}

int CDECL eventCallback(wl_WindowRef window, wl_Event *event, void *userData) {
	event->handled = true;
	switch (event->eventType) {
	case wl_kEventTypeWindowCloseRequest:
		if (window == mainWindow) {
			printf("main window closing ... or is it?\n");
			//event->closeRequestEvent.cancelClose = true;
		}
		else {
			printf("closed something else, staying\n");
		}
		break;
	case wl_kEventTypeWindowDestroyed:
		if (window == mainWindow) {
			printf("main window destroyed! exiting runloop\n");
			wl_ExitRunloop();
		}
		break;
	case wl_kEventTypeAction:
		printf("action %p chosen\n", (void *)event->actionEvent.action);
		if (event->actionEvent.action == openAction) {
			wl_FileDialogOpts::FilterSpec specs[] = {
				{"Image Formats", "*.jpg;*.jpeg;*.png;*.gif"},
				{"JPEG images", "*.jpg;*.jpeg"},
				{"PNG images", "*.png"},
				{"GIF images", "*.gif"},
				{"All Files", "*.*"}
			};
			wl_FileDialogOpts opts = {};
			opts.mode = wl_FileDialogOpts::kModeMultiFile;
			        //wl_FileDialogOpts::kModeFolder;
			        //wl_FileDialogOpts::kModeFile;
			        //wl_FileDialogOpts::kModeMultiFile;
			opts.numFilters = sizeof(specs) / sizeof(wl_FileDialogOpts::FilterSpec);
			opts.filters = specs;

			wl_FileResults* results;
			if (wl_FileOpenDialog(&opts, &results)) {
				for (int i = 0; i < results->numResults; i++) {
					printf("opened file: [%s]\n", results->results[i]);
				}
				wl_FileResultsFree(&results);
			} else {
			    printf("(user canceled)\n");
			}
		}
		else if (event->actionEvent.action == saveAsAction) {
			wl_FileDialogOpts::FilterSpec specs[] = {
				{"Derp Werp", "*.derp;*.woot"},
				{"JPEG images", "*.jpg;*.jpeg"},
				{"PNG images", "*.png"},
				{"All Files", "*.*"}
			};
			wl_FileDialogOpts opts = {};
			opts.mode = wl_FileDialogOpts::kModeFile;
			opts.numFilters = sizeof(specs) / sizeof(wl_FileDialogOpts::FilterSpec);
			opts.filters = specs;
			opts.defaultExt = "png";
			
			wl_FileResults* results;
			if (wl_FileSaveDialog(&opts, &results)) {
				assert(results->numResults == 1);
				printf("saving to file: [%s]\n", results->results[0]);
				wl_FileResultsFree(&results);
			}
		}
		else if (event->actionEvent.action == exitAction) {
			wl_WindowDestroy(window); // app will close when destroy message received (see above)
		}
		else if (event->actionEvent.action == copyAction) {
			auto clipData = wl_DragDataCreate(window);
			wl_DragAddFormat(clipData, wl_kDragFormatUTF8);
			wl_ClipboardSet(clipData);
			printf("clipboard copy done\n");
			wl_DragDataRelease(&clipData);
		}
		else if (event->actionEvent.action == pasteAction) {
			auto clipData = wl_ClipboardGet();
			if (wl_DropHasFormat(clipData, wl_kDragFormatFiles)) {
				const wl_Files *files;
				wl_DropGetFiles(clipData, &files);
				for (int i = 0; i < files->numFiles; i++) {
					printf("Got file: [%s]\n", files->filenames[i]);
				}
			}
			else if (wl_DropHasFormat(clipData, wl_kDragFormatUTF8)) {
				const char *text;
				size_t textSize;
				if (wl_DropGetFormat(clipData, wl_kDragFormatUTF8, (const void **)&text, &textSize)) {
					printf("got clipboard text: [%s]\n", text);
				}
			}
			wl_ClipboardRelease(clipData);
		}
		break;

	case wl_kEventTypeWindowRepaint:
		if (window == mainWindow) {
			platformDraw(&event->repaintEvent.platformContext);
		}
		else if (window == framelessWindow) {
			platformDrawFrameless(&event->repaintEvent.platformContext);
		}
		break;

	case wl_kEventTypeWindowResized:
		if (window == mainWindow) {
			width = event->resizeEvent.newWidth;
			height = event->resizeEvent.newHeight;

			printf("new size: %d, %d\n", width, height);

			//printf("TODO: remove resize inval\n");
			//wl_WindowInvalidate(window, 0, 0, 0, 0);

			lastFrame = 0;
		}
		break;

	case wl_kEventTypeTimer:
		// NOTE! 'window' param will be null here, timer events are app-global, not related to any specific window
		switch ((size_t)event->timerEvent.userData) {
		case ID_FastTimer:
			wl_WindowInvalidate(mainWindow, 0, 0, 0, 0); // entire window
			break;
		case ID_SlowTimer:
			//printf("%d frames\n", numFrames);
			totalFrames = lastFrame;
			lastFrame = 0;
			break;
		}
		break;

	case wl_kEventTypeMouse:
	{
		if (window != mainWindow) break;

		if (event->mouseEvent.eventType == wl_kMouseEventTypeMouseDown) {
			if (event->mouseEvent.button == wl_kMouseButtonLeft) {
				if (pointInRect(event->mouseEvent.x, event->mouseEvent.y, DRAG_SOURCE_X, DRAG_SOURCE_Y, DRAG_SOURCE_W, DRAG_SOURCE_H)) {
					// start drag, save position
					dragging = true;
					dragStartX = event->mouseEvent.x;
					dragStartY = event->mouseEvent.y;
					printf("(potentially) starting drag...\n");
				}
				else if (pointInRect(event->mouseEvent.x, event->mouseEvent.y, MSG_CLICK_X, MSG_CLICK_Y, MSG_CLICK_W, MSG_CLICK_H)) {
					wl_MessageBoxParams params = {};
					params.title = "Message Box Title!";
					params.message = "(message content woot doot)";
					params.withHelpButton = true;
					params.icon = wl_MessageBoxParams::kIconInformation;
					params.buttons = wl_MessageBoxParams::kButtonsOk;
					auto result = wl_MessageBox(mainWindow, &params);
					printf("show message result: %d\n", result);
				}
				else {
				    // begin grab
				    wl_MouseGrab(mainWindow);
				    grabbed = true;
				}
			}
			else if (event->mouseEvent.button == wl_kMouseButtonRight) {
				printf("mouse down event, button %d @ %d,%d\n", event->mouseEvent.button, event->mouseEvent.x, event->mouseEvent.y);
				wl_WindowShowContextMenu(window, event->mouseEvent.x, event->mouseEvent.y, contextMenu, event);
			}
		}
		else if (event->mouseEvent.eventType == wl_kMouseEventTypeMouseUp) {
		    if (event->mouseEvent.button == wl_kMouseButtonLeft) {
                dragging = false;
                if (grabbed) {
                    printf("ending grab\n");
                    wl_MouseUngrab();
                    grabbed = false;
                }
		    }
		}
		else if (event->mouseEvent.eventType == wl_kMouseEventTypeMouseMove) {
			// if dragging ...
			if (dragging) {
				if (pointDist(dragStartX, dragStartY, event->mouseEvent.x, event->mouseEvent.y) > 4.0) {
					auto dragData = wl_DragDataCreate(window);

					wl_DragAddFormat(dragData, wl_kDragFormatUTF8);
					//                    wl_DragAddFormat(dragData, wlDragFormatFiles);

					printf("starting drag ...\n");
					auto whichAction = wl_DragExec(dragData, wl_kDropEffectCopy | wl_kDropEffectMove | wl_kDropEffectLink, event);
					printf("selected dragexec action: %d\n", whichAction);
                    dragging = false;
					wl_DragDataRelease(&dragData);
					printf("drag complete\n");
				}
			} else if (grabbed) {
			    // just print coords to verify grab
			    printf("(grabbed) move event: %d,%d\n", event->mouseEvent.x, event->mouseEvent.y);
			} else {
			    // neither dragging nor grabbing

                // frameless hover updates
                if (pointInRect(event->mouseEvent.x, event->mouseEvent.y, HOVER_HERE_X, HOVER_HERE_Y, HOVER_HERE_W, HOVER_HERE_H)) {
                    // show/move no matter what
                    auto x2 = event->mouseEvent.x - POPUP_WIDTH / 2;
                    auto y2 = event->mouseEvent.y - (POPUP_HEIGHT + 50);
                    wl_WindowShowRelative(framelessWindow, mainWindow, x2, y2, 0, 0);
                    framelessWindowVisible = true;

                    // set custom cursor
                    wl_WindowSetCursor(mainWindow, customCursor);
                }
                else {
                    if (framelessWindowVisible) {
                        wl_WindowHide(framelessWindow);
                        framelessWindowVisible = false;
                    }
                    // clear custom cursor
                    wl_WindowSetCursor(mainWindow, nullptr);
                }
			}
		}
		else if (event->mouseEvent.eventType == wl_kMouseEventTypeMouseEnter) {
			printf("==> mouse entered window\n");
		}
		else if (event->mouseEvent.eventType == wl_kMouseEventTypeMouseLeave) {
			printf("<== mouse left window\n");
		}
		else if (event->mouseEvent.eventType == wl_kMouseEventTypeMouseWheel) {
			printf("[mouse wheel: %s delta %d @ %d,%d]\n", 
				(event->mouseEvent.wheelAxis == wl_kMouseWheelAxisHorizontal) ? "HORIZ" : "VERT",
				event->mouseEvent.wheelDelta, event->mouseEvent.x, event->mouseEvent.y);
		}

		break;
	}

	case wl_kEventTypeKey:
		switch (event->keyEvent.eventType) {
		case wl_kKeyEventTypeDown:
		{
			const char *loc =
				event->keyEvent.location == wl_kKeyLocationDefault ? "Default" :
				(event->keyEvent.location == wl_kKeyLocationLeft ? "Left" :
				(event->keyEvent.location == wl_kKeyLocationRight ? "Right" :
					(event->keyEvent.location == wl_kKeyLocationNumPad ? "Numpad" : "Unknown")));
			printf("Key down: %d [%s] (mods %02X) (loc %s)\n", event->keyEvent.key, event->keyEvent.string, event->keyEvent.modifiers, loc);

			//if (event->keyEvent.key == wl_kKeyZ) {
			//	printf("fuckin Z!!\n");
			//	if (!framelessWindowVisible) {
			//		auto x2 = HOVER_HERE_X - POPUP_WIDTH / 2;
			//		auto y2 = HOVER_HERE_Y - 50;
			//		wl_WindowShowRelative(framelessWindow, mainWindow, x2, y2, 0, 0);
			//		framelessWindowVisible = true;
			//		printf("showing window!\n");
			//	}
			//	else {
			//		wl_WindowHide(framelessWindow);
			//		framelessWindowVisible = false;
			//		printf("hiding window!\n");
			//	}
			//}
			break;
		}
		case wl_kKeyEventTypeUp:
			//printf(" - keyup: %d [%s]\n", event->keyEvent.key, event->keyEvent.string);
			break;
		case wl_kKeyEventTypeChar:
			printf("CHAR: '%s'\n", event->keyEvent.string);
			break;
		}
		break;

	case wl_kEventTypeDragRender:
		if (platformProvidesDragFormat(event->dragRenderEvent.dragFormat)) {
			platformRenderDragFormat(event->dragRenderEvent.payload, event->dragRenderEvent.dragFormat);
		}
		else if (strEqual(event->dragRenderEvent.dragFormat, wl_kDragFormatUTF8)) {
			wl_DragRenderUTF8(event->dragRenderEvent.payload, u8"<<Here's your ad-hoc generated text, woooot!!>>");
		}
		else if (strEqual(event->dragRenderEvent.dragFormat, wl_kDragFormatFiles)) {
			wl_Files files;
			files.numFiles = 3;
			files.filenames = new const char*[3];
			files.filenames[0] = "/boot/home/Desktop/cool_bitmap";
			files.filenames[1] = "/boot/home/Desktop/text_file";
			files.filenames[2] = "/boot/home/Desktop/dragme";
			//
			wl_DragRenderFiles(event->dragRenderEvent.payload, &files);
			// safe to delete
			delete files.filenames;
		}
		else {
			printf("#### drag source render - unhandled mime type");
			event->handled = false;
		}
		break;

	case wl_kEventTypeDrop:
		switch (event->dropEvent.eventType) {
		case wl_kDropEventTypeFeedback:
			if (pointInRect(event->dropEvent.x, event->dropEvent.y, DROP_TARGET_X, DROP_TARGET_Y, DROP_TARGET_W, DROP_TARGET_H))
			{
				if (wl_DropHasFormat(event->dropEvent.data, wl_kDragFormatUTF8) ||
					wl_DropHasFormat(event->dropEvent.data, wl_kDragFormatFiles) ||
					platformCheckDropFormats(event->dropEvent.data))
				{
					event->dropEvent.allowedEffectMask &= event->dropEvent.defaultModifierAction; // use platform-specific suggestion based on mod keys
				}
			}
			else {
				event->dropEvent.allowedEffectMask = wl_kDropEffectNone;
			}
			break;
		case wl_kDropEventTypeLeave:
			printf("*** drop has left the window! ***\n");
			break;
		case wl_kDropEventTypeDrop:
			// check final modifiers?
			event->dropEvent.allowedEffectMask &= event->dropEvent.defaultModifierAction; // why is this here?

			if (platformCheckDropFormats(event->dropEvent.data)) {
				// check the platform-specific stuff first, otherwise sometimes the wlDragFormatUTF8 handler grabs it first
				// (eg, haiku's translation kit offers to convert all kinds of things to text, which is silly)
				platformHandleDrop(event->dropEvent.data);
			}
			else if (wl_DropHasFormat(event->dropEvent.data, wl_kDragFormatFiles)) {
				const wl_Files *files;
				if (wl_DropGetFiles(event->dropEvent.data, &files)) {
					for (int i = 0; i < files->numFiles; i++) {
						printf("Got file: [%s]\n", files->filenames[i]);
					}
				}
			}
			else if (wl_DropHasFormat(event->dropEvent.data, wl_kDragFormatUTF8)) {
				const char *text;
				size_t textSize;
				if (wl_DropGetFormat(event->dropEvent.data, wl_kDragFormatUTF8, (const void **)&text, &textSize)) {
					printf("got text: [%s]\n", text);
				}
				else {
					printf("did NOT get text - wlDropGetText failed\n");
				}
			}
			else {
				printf("unhandled format in wl_kDropEventTypeDrop\n");
			}

			break;
		}
		break; // case wl_kEventTypeDragEvent

	default:
		printf("unhandled wl callback event type: %d\n", event->eventType);
		event->handled = false;
	}
	return 0;
}

void createActions() {
	auto icon1 = wl_IconLoadFromFile("_icons/tall.png", 64);
	auto icon2 = wl_IconLoadFromFile("_icons/wide.png", 64);
	//auto icon1 = wl_IconLoadFromFile("_icons/horse.png", 16);
	//auto icon2 = wl_IconLoadFromFile("_icons/laptop.png", 16);

	fileAction1 = wl_ActionCreate(ID_FileAction1, "Something", icon1, 0);

	// user uppercase key enums here because that's required for win32 virtual key codes,
	//  and doesn't require any special handling when generating labels (toupper etc)
	auto fa2Accel = wl_AccelCreate(wl_kKeyK, wl_kModifierAlt);
	fileAction2 = wl_ActionCreate(ID_FileAction2, "SubMenuItem", 0, fa2Accel);

	// open
	auto openAccel = wl_AccelCreate(wl_kKeyO, wl_kModifierControl); // ctrl-o
	openAction = wl_ActionCreate(ID_FileAction3, "&Open", 0, openAccel);

	// save
	auto saveAsAccel = wl_AccelCreate(wl_kKeyS, wl_kModifierControl | wl_kModifierShift); // ctrl-shift-s
	saveAsAction = wl_ActionCreate(ID_FileAction4, "Save &As", 0, saveAsAccel);

	auto exitAccel = wl_AccelCreate(wl_kKeyQ, wl_kModifierControl);
	exitAction = wl_ActionCreate(ID_ExitAction, "Quit c-client", 0, exitAccel);

	auto copyAccel = wl_AccelCreate(wl_kKeyC, wl_kModifierControl);
	copyAction = wl_ActionCreate(ID_CopyAction, "Copy", 0, copyAccel);

	auto pasteAccel = wl_AccelCreate(wl_kKeyV, wl_kModifierControl);
	pasteAction = wl_ActionCreate(ID_PasteAction, "Paste", 0, pasteAccel);

	helpAction1 = wl_ActionCreate(ID_HelpAction, "Placeholder", icon2, 0);
	contextAction1 = wl_ActionCreate(ID_ContextAction1, "Context 01", 0, 0);
	contextAction2 = wl_ActionCreate(ID_ContextAction2, "Context 02", 0, 0);
	auto c3Accel = wl_AccelCreate(wl_kKeyN, wl_kModifierShift);
	contextAction3 = wl_ActionCreate(ID_ContextAction3, "Context 03", 0, c3Accel);

	//auto c4Accel = wl_AccelCreate(wl_kKeyOEM_4, wl_kModifierControl);
	//contextAction4 = wl_ActionCreate(CONTEXT_ACTION_4, "Context WAT", 0, c4Accel);
}

void createMenu() {

	createActions();

#ifdef WL_PLATFORM_MACOS
	auto menuBar = wl_MenuBarGetDefault();
	auto appMenu = wl_GetApplicationMenu();
#else
	auto menuBar = wl_MenuBarCreate();
#endif
	/* file menu */
	auto fileMenu = wl_MenuCreate();

	wl_MenuAddAction(fileMenu, openAction); // open
	wl_MenuAddAction(fileMenu, saveAsAction); // save

	wl_MenuAddAction(fileMenu, fileAction1);

	auto fileSubMenu = wl_MenuCreate();
	wl_MenuAddAction(fileSubMenu, fileAction2);
	wl_MenuAddSubmenu(fileMenu, "Su&bmenu", fileSubMenu);

#ifdef WL_PLATFORM_MACOS
	// app menu gets exit action on Apple platforms
	wl_MenuAddAction(appMenu, exitAction);
#else
	// but it goes on the File menu elsewhere
	wl_MenuAddSeparator(fileMenu);
	wl_MenuAddAction(fileMenu, exitAction);
#endif
	wl_MenuBarAddMenu(menuBar, "&File", fileMenu);

	/* edit menu */
	auto editMenu = wl_MenuCreate();
	wl_MenuAddAction(editMenu, copyAction);
	wl_MenuAddAction(editMenu, pasteAction);
	wl_MenuBarAddMenu(menuBar, "&Edit", editMenu);

	/* context menu (useful later) */
	contextMenu = wl_MenuCreate();
	wl_MenuAddAction(contextMenu, contextAction1);
	wl_MenuAddAction(contextMenu, contextAction2);
	wl_MenuAddSeparator(contextMenu);

	auto contextSubMenu = wl_MenuCreate();
	wl_MenuAddAction(contextSubMenu, contextAction3);
	wl_MenuAddSubmenu(contextMenu, "Su&bmenu", contextSubMenu);
	/* end context menu*/

	wl_MenuBarAddMenu(menuBar, "&Context", contextMenu);

	/* help menu */
	auto helpMenu = wl_MenuCreate();
	wl_MenuAddAction(helpMenu, helpAction1);

	wl_MenuBarAddMenu(menuBar, "&Help", helpMenu);

	/* end */
#ifdef WL_PLATFORM_MACOS
	// nothing to do
#else
	wl_WindowSetMenuBar(mainWindow, menuBar);
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

	wl_WindowInvalidate(mainWindow, box->x, box->y, box->width, box->height);
}

void bgThreadFunc(int threadID) {
	while (true) {
		size_t void_arg = threadID;
		wl_ExecuteOnMainThread(addNewBox, (void *)void_arg);
		wl_Sleep(200);
	}
}

int main(int argc, const char * argv[]) {
	srand((unsigned int)time(NULL));

	wl_PlatformOptions opts = { 0 };
	wl_Init(eventCallback, &opts);

	wl_WindowProperties props = { 0 };
	props.usedFields = wl_kWindowPropMinWidth | wl_kWindowPropMinHeight | wl_kWindowPropMaxWidth | wl_kWindowPropMaxHeight;
	props.minWidth = 300;
	props.minHeight = 200;
	props.maxWidth = MAX_WIDTH;
	props.maxHeight = MAX_HEIGHT;

	width = 800;
	height = 600;

	mainWindow = wl_WindowCreate(width, height, u8"hello there, cross-platform friend āǢʥϢ۩ใ ♥☺☼", nullptr, &props);

	props = { 0 };
	props.usedFields = wl_kWindowPropStyle;
	props.style = wl_kWindowStyleFrameless;
	framelessWindow = wl_WindowCreate(POPUP_WIDTH, POPUP_HEIGHT, nullptr, nullptr, &props);

	createMenu();

	platformInit();

	fastTimer = wl_TimerCreate(16, (void *)ID_FastTimer); // ~60fps
	slowTimer = wl_TimerCreate(1000, (void *)ID_SlowTimer);

	wl_WindowEnableDrops(mainWindow, true);

	platformCreateThreads(bgThreadFunc, NUM_THREADS);

	customCursor = wl_CursorCreate(wl_kCursorStyleResizeLeftRight);

	wl_WindowShow(mainWindow);

	wl_Runloop();

	wl_TimerDestroy(fastTimer);
	wl_TimerDestroy(slowTimer);

	wl_ClipboardFlush();

	platformJoinThreads();
	platformShutdown();

	wl_Shutdown();
	return 0;
}
