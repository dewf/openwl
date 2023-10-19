#include "../main.h"

#include <stdio.h>

#include <Bitmap.h>

static BBitmap *background;
static unsigned int *data;
static BRect backgroundRect(0, 0, MAX_WIDTH-1, MAX_HEIGHT-1); // without the -1 the alignment is off ...
static rgb_color black { 0, 0, 0, 255 }, white { 255, 255, 255, 255 };

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
    auto v = platformContext->view;
    auto visible = BRect(0, 0, width, height);
    v->DrawBitmap(background, visible);

    // draw crossed lines
    v->SetHighColor(black);
    v->SetPenSize(3.0f);
    v->StrokeLine(BPoint(10, 10), BPoint(width - 10, height - 10));
    v->StrokeLine(BPoint(10, height - 10), BPoint(width - 10, 10));

//    Gdiplus::Pen blackPen(Color::Black, DPIUP_F(4.0));
//    auto x1 = DPIUP(10);
//    auto y1 = DPIUP(10);
//    auto x2 = DPIUP(width - 10);
//    auto y2 = DPIUP(height - 10);
//    graphics.DrawLine(&blackPen, Point(x1, y1), Point(x2, y2));
//    graphics.DrawLine(&blackPen, Point(x1, y2), Point(x2, y1));

//    // thin outer rect
//    blackPen.SetWidth(DPIUP_F(1.0));
//    graphics.DrawRectangle(&blackPen, Rect(DPIUP(3), DPIUP(3), DPIUP(width - 6), DPIUP(height - 6)));

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
