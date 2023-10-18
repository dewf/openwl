#include "../main.h"

#include <stdio.h>

#include <Bitmap.h>

BBitmap *background;
unsigned int *data;
BRect backgroundRect(0, 0, MAX_WIDTH-1, MAX_HEIGHT-1); // without the -1 the alignment is off ...

void platformInit()
{
    // generate the gradient bitmap to use
    data = new unsigned int[MAX_WIDTH * MAX_HEIGHT];
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            unsigned char red = (j * 256) / MAX_WIDTH;
            unsigned char green = (i * 256) / MAX_HEIGHT;
            unsigned char blue = 255 - green;
            data[i * MAX_WIDTH + j] = ((red << 16) + (green << 8) + blue) | 0xff000000;
        }
    }
    background = new BBitmap(backgroundRect, B_RGBA32, true, true);
    background->SetBits(data, MAX_WIDTH * MAX_HEIGHT * 4, 0, B_RGBA32);
}

void platformDraw(wl_PlatformContext *platformContext)
{
    platformContext->view->DrawBitmap(background, backgroundRect);
}

void platformDrawFrameless(wl_PlatformContext *platformContext)
{
}

void platformDrawModal(wl_PlatformContext* platformContext)
{
}

void platformDrawBox(RandomBox *box)
{
}

bool platformProvidesDragFormat(const char *formatMIME)
{
    return false;
}

void platformRenderDragFormat(wl_RenderPayloadRef payload, const char *formatMIME)
{
}


bool platformCheckDropFormats(wl_DropDataRef dropData)
{
    return false;
}

void platformHandleDrop(wl_DropDataRef dropData)
{
}

void platformCreateThreads(threadFunc_t threadFunc, int count)
{
}

void platformJoinThreads()
{
}

void platformShutdown()
{
}
