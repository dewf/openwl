#ifndef __MAIN_H__
#define __MAIN_H__

#include "../../../source/openwl.h"

#define MAX_WIDTH 1600
#define MAX_HEIGHT 900

#define DRAG_SOURCE_X 100
#define DRAG_SOURCE_Y 100
#define DRAG_SOURCE_W 200
#define DRAG_SOURCE_H 200

#define DROP_TARGET_X 500
#define DROP_TARGET_Y 100
#define DROP_TARGET_W 200
#define DROP_TARGET_H 200

#define HOVER_HERE_X 320
#define HOVER_HERE_Y 320
#define HOVER_HERE_W 160
#define HOVER_HERE_H 160

#define POPUP_WIDTH 200
#define POPUP_HEIGHT 200

extern int width, height;
extern int lastFrame, totalFrames;

void platformInit();
void platformDraw(wl_PlatformContext *platformContext);
void platformDrawFrameless(wl_PlatformContext *platformContext);

#define BOXWIDTH 40
struct RandomBox {
	float r, g, b;
	int x, y, width, height;
};
void platformDrawBox(RandomBox *box);

bool platformProvidesDragFormat(const char *formatMIME);
void platformRenderDragFormat(wl_RenderPayloadRef payload, const char *formatMIME);

bool platformCheckDropFormats(wl_DropDataRef dropData);
void platformHandleDrop(wl_DropDataRef dropData);

typedef void(*threadFunc_t)(int threadID);
void platformCreateThreads(threadFunc_t threadFunc, int count);
void platformJoinThreads(); // only if necessary
void platformShutdown();

#endif // __MAIN_H__

