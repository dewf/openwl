#include "../../../../source/openwl.h"

#include <stdio.h>

#include "../main.h"

#include <thread>
#include <vector>

#include <gdiplus.h>
using namespace Gdiplus;

GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR           gdiplusToken;

Gdiplus::Bitmap *bgBitmap;
Gdiplus::Graphics *bgGraphics;
unsigned int *data;

// DPI macros
#define DECLSF(dpi) double scaleFactor = dpi / 96.0;
#define INT(x) ((int)(x))
#define DPIUP(x) INT((x) * scaleFactor)                // from device-independent pixels to physical res
#define DPIDOWN(x) INT((x) / scaleFactor)              // from physical res to DIPs
#define DPIUP_INPLACE(x) x = DPIUP(x);
#define DPIDOWN_INPLACE(x) x = DPIDOWN(x);

void platformInit() {
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	data = new unsigned int[MAX_WIDTH * MAX_HEIGHT];
	for (int i = 0; i < MAX_HEIGHT; i++) {
		for (int j = 0; j < MAX_WIDTH; j++) {
			unsigned char red = (j * 256) / MAX_WIDTH;
			unsigned char green = (i * 256) / MAX_HEIGHT;
			unsigned char blue = 255 - green;
			data[i * MAX_WIDTH + j] = ((red << 16) + (green << 8) + blue) | 0xff000000;
		}
	}
	bgBitmap = new Gdiplus::Bitmap(MAX_WIDTH, MAX_HEIGHT, (MAX_WIDTH * 4), PixelFormat32bppARGB, (BYTE *)data);
	bgGraphics = new Graphics(bgBitmap);
}

static void drawTextRect(UINT dpi, Gdiplus::Graphics &g, Gdiplus::Font &font, const WCHAR *text, int x, int y, int width, int height, bool textOnly = false)
{
	DECLSF(dpi);

	RectF rect((float)DPIUP(x), (float)DPIUP(y), (float)DPIUP(width), (float)DPIUP(height));

	if (!textOnly) {
		Pen pen(Color::Black, DPIUP(2.0));
		g.DrawRectangle(&pen, rect);

		SolidBrush brush(Color::Gray);
		g.FillRectangle(&brush, rect);
	}
	RectF exts;
	g.MeasureString(text, -1, &font, rect, &exts);

	auto tx = DPIUP(x + (width - DPIDOWN(exts.Width)) / 2);     // inner DPIDOWN because exts already in physical space
	auto ty = DPIUP(y + (height - DPIDOWN(exts.Height)) / 2);
	SolidBrush black(Color::Black);
	g.DrawString(text, -1, &font, PointF(tx, ty), &black);
}

void platformDraw(wl_PlatformContext *platformContext) {
	Gdiplus::Graphics target(platformContext->gdi.hdc);

	DECLSF(platformContext->dpi);

	auto width2 = DPIUP(width);
	auto height2 = DPIUP(height);

	// double buffer ...
	Gdiplus::Bitmap offScreenBuffer(width2, height2, &target);
	Gdiplus::Graphics graphics(&offScreenBuffer);
	graphics.SetSmoothingMode(SmoothingModeAntiAlias);
	graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);

	graphics.DrawImage(bgBitmap, Rect(0, 0, DPIUP(MAX_WIDTH), DPIUP(MAX_HEIGHT)));

	// draw crossed lines
	Gdiplus::Pen blackPen(Color::Black, DPIUP(4.0));
	auto x1 = DPIUP(10);
	auto y1 = DPIUP(10);
	auto x2 = DPIUP(width - 10);
	auto y2 = DPIUP(height - 10);
	graphics.DrawLine(&blackPen, Point(x1, y1), Point(x2, y2));
	graphics.DrawLine(&blackPen, Point(x1, y2), Point(x2, y1));

	// thin outer rect
	blackPen.SetWidth(DPIUP(1.0));
	graphics.DrawRectangle(&blackPen, Rect(DPIUP(3), DPIUP(3), DPIUP(width - 6), DPIUP(height - 6)));

	// text stuff
	Gdiplus::Font font(L"Arial", 20.0); // why doesn't this have to be scaled? is it already dpi-aware?

	drawTextRect(platformContext->dpi, graphics, font, L"Drag Source", DRAG_SOURCE_X, DRAG_SOURCE_Y, DRAG_SOURCE_W, DRAG_SOURCE_H);
	drawTextRect(platformContext->dpi, graphics, font, L"Drop Target", DROP_TARGET_X, DROP_TARGET_Y, DROP_TARGET_W, DROP_TARGET_H);
	drawTextRect(platformContext->dpi, graphics, font, L"Hover Here", HOVER_HERE_X, HOVER_HERE_Y, HOVER_HERE_W, HOVER_HERE_H);

	//RectF frameRect((float)(width - 260), (float)(height - 50), (float)260, (float)50);
	//RectF exts;
	//graphics.MeasureString(L"FRAME 999 (999)", -1, &font, frameRect, &exts);
	//auto tx = frameRect.X + (frameRect.Width - exts.Width) / 2;
	//auto ty = frameRect.Y + (frameRect.Height - exts.Height) / 2;

	//WCHAR text[1024];
	//wsprintf(text, L"FRAME %d (%d)", lastFrame, totalFrames);
	//SolidBrush blackBrush(Color::White);
	//graphics.DrawString(text, -1, &font, PointF(tx, ty), &blackBrush);

	//lastFrame++;

	target.DrawImage(&offScreenBuffer, Rect(0, 0, width2, height2));
}

void platformDrawFrameless(wl_PlatformContext *platformContext)
{
	//Gdiplus::Graphics target(platformContext->gdi.hdc);

	//// double buffer ...
	//Gdiplus::Bitmap offScreenBuffer(POPUP_WIDTH, POPUP_HEIGHT, &target);
	//Gdiplus::Graphics graphics(&offScreenBuffer);
	//graphics.SetSmoothingMode(SmoothingModeAntiAlias);
	//graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);

	////// ==========================
	//RectF rect(0, 0, POPUP_WIDTH, POPUP_HEIGHT);

	//SolidBrush brush(Color::White);
	//graphics.FillRectangle(&brush, rect);

	//Gdiplus::Font font(L"Arial", 20.0);
	//drawTextRect(graphics, font, L"HELLO!", (int)rect.X, (int)rect.Y, (int)rect.Width, (int)rect.Height, true);

	//// =============
	//target.DrawImage(&offScreenBuffer, Rect(0, 0, POPUP_WIDTH, POPUP_HEIGHT));
}

void platformDrawBox(RandomBox *box) {
	//Gdiplus::LinearGradientBrush boxBrush(
	//	Point(box->x - 1, box->y - 1), // -1 to prevent little dot artifact ...
	//	Point(box->x + box->width, box->y + box->height),
	//	Color((BYTE)(box->r * 255), ((BYTE)box->g * 255), ((BYTE)box->b * 255)),
	//	Color::Black
	//);
	//bgGraphics->FillRectangle(&boxBrush, Rect(box->x, box->y, box->width, box->height));
}

bool platformProvidesDragFormat(const char *formatMIME)
{
	return false;
}
void platformRenderDragFormat(wl_RenderPayloadRef payload, const char *formatMIME)
{
	// nothing yet
}

bool platformCheckDropFormats(wl_DropDataRef dropData)
{
	return false;
}
void platformHandleDrop(wl_DropDataRef dropData)
{
	// nothing yet
}

void platformCreateThreads(threadFunc_t threadFunc, int count)
{
	std::vector<std::thread *> threads;
	for (int i = 0; i < count; i++) {
		auto th = new std::thread(threadFunc, i);
		th->detach();
		threads.push_back(th);
	}
}

void platformJoinThreads() {
	// don't bother joining the daemon threads, they'll exit when the app does
}

void platformShutdown() {
	delete bgGraphics;
	delete bgBitmap;
	delete[] data;
	GdiplusShutdown(gdiplusToken);
}
