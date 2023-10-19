#include "../main.h"

#include <stdio.h>

#include <Bitmap.h>

static BBitmap *background;
static unsigned int *data;
static BRect backgroundRect(0, 0, MAX_WIDTH-1, MAX_HEIGHT-1); // without the -1 the alignment is off ...
static rgb_color
    black { 0, 0, 0, 255 },
    white { 255, 255, 255, 255 },
    gray { 180, 180, 180, 255 };

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

static void drawTextRect(BView *view, BFont *font, const char *text, int x, int y, int width, int height, bool textOnly = false)
{
    BRect rect(x, y, x + width - 1, y + height - 1);

    if (!textOnly) {
        view->SetHighColor(gray);
        view->FillRect(rect);

        view->SetPenSize(2.0f);
        view->SetHighColor(black);
        view->StrokeRect(rect);
    }

    font_height heightInfo;
    view->SetFont(font);
    view->GetFontHeight(&heightInfo);
    auto fontWidth = view->StringWidth(text);
    auto fontHeight = heightInfo.ascent + heightInfo.descent;

    auto tx = x + ((width - fontWidth) / 2);
    auto ty = y + ((height - fontHeight) / 2) + heightInfo.ascent;
    view->SetHighColor(black);
    view->DrawString(text, BPoint(tx, ty));
//    auto tx = DPIUP_F(x + (width - DPIDOWN(exts.Width)) / 2);     // inner DPIDOWN because exts already in physical space
//    auto ty = DPIUP_F(y + (height - DPIDOWN(exts.Height)) / 2);
//    SolidBrush black(Color::Black);
//    g.DrawString(text, -1, &font, PointF(tx, ty), &black);
}

void platformDraw(wl_PlatformContext *platformContext)
{
    auto v = platformContext->view;
    auto visible = BRect(0, 0, width, height);
    v->DrawBitmap(background, visible, visible);

    // draw crossed lines
    v->SetHighColor(black);
    v->SetPenSize(3.0f);
    v->StrokeLine(BPoint(10, 10), BPoint(width - 10, height - 10));
    v->StrokeLine(BPoint(10, height - 10), BPoint(width - 10, 10));

    // thin outer rect
    v->SetPenSize(1.0f);
    v->StrokeRect(BRect(3, 3, width - 3, height - 3));

    // the various rects
    BFont font(be_plain_font);
    font.SetSize(20.0);
    drawTextRect(v, &font, "Drag Source", DRAG_SOURCE_X, DRAG_SOURCE_Y, DRAG_SOURCE_W, DRAG_SOURCE_H);
    drawTextRect(v, &font, "Drop Target", DROP_TARGET_X, DROP_TARGET_Y, DROP_TARGET_W, DROP_TARGET_H);
    drawTextRect(v, &font, "Hover Here", HOVER_HERE_X, HOVER_HERE_Y, HOVER_HERE_W, HOVER_HERE_H);
    drawTextRect(v, &font, "Click Here", MSG_CLICK_X, MSG_CLICK_Y, MSG_CLICK_W, MSG_CLICK_H);
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
