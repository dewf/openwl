#include "../../../../source/openwl.h"

#include "../main.h"

#include <cairo/cairo.h>

#include <stdio.h>
#include <thread>
#include <vector>

static cairo_surface_t *imageSurface;

void platformInit() {
	imageSurface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, MAX_WIDTH, MAX_HEIGHT);
	auto data = (unsigned int *)cairo_image_surface_get_data(imageSurface);
	for (int i = 0; i < MAX_HEIGHT; i++) {
		for (int j = 0; j < MAX_WIDTH; j++) {
			unsigned char red = (j * 256) / MAX_WIDTH;
			unsigned char green = (i * 256) / MAX_HEIGHT;
			unsigned char blue = 255 - green;
			data[i * MAX_WIDTH + j] = ((red << 16) + (green << 8) + blue) | 0xff000000;
		}
	}
}

static void moveForText(cairo_t *cr, const char *text, int x, int y, int w, int h) {
	cairo_font_extents_t fe;
	cairo_font_extents(cr, &fe);

	cairo_text_extents_t te;
	cairo_text_extents(cr, text, &te);
	cairo_move_to(cr, x + (w - te.width) / 2, y + fe.ascent + (h - te.height) / 2);
}

static void drawTextRect(cairo_t *cr, const char *text, int x, int y, int w, int h, bool textOnly = false) {
	if (!textOnly) {
		cairo_set_line_width(cr, 2);
		cairo_rectangle(cr, x, y, w, h);
		cairo_stroke(cr);
		cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
		cairo_rectangle(cr, x, y, w, h);
		cairo_fill(cr);
	}
	cairo_set_source_rgb(cr, 0, 0, 0);
	moveForText(cr, text, x, y, w, h);
	cairo_show_text(cr, text);
}

void platformDraw(void *platformContext) {
    auto cr = (cairo_t *)platformContext;

	cairo_set_source_surface(cr, imageSurface, 0, 0);
	cairo_paint(cr);

	// draw crossed lines
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_set_line_width(cr, 4);

	cairo_move_to(cr, 10, 10);
	cairo_line_to(cr, width - 10, height - 10);
	cairo_stroke(cr);

	cairo_move_to(cr, 10, height - 10);
	cairo_line_to(cr, width - 10, 10);
	cairo_stroke(cr);

	// thin outer rect
	cairo_set_line_width(cr, 1);
	cairo_rectangle(cr, 2.5, 2.5, width - 5, height - 5);
	cairo_stroke(cr);

	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, 20);
	drawTextRect(cr, "Drag Source", DRAG_SOURCE_X, DRAG_SOURCE_Y, DRAG_SOURCE_W, DRAG_SOURCE_H);
	drawTextRect(cr, "Drop Target", DROP_TARGET_X, DROP_TARGET_Y, DROP_TARGET_W, DROP_TARGET_H);
	drawTextRect(cr, "Hover Here", HOVER_HERE_X, HOVER_HERE_Y, HOVER_HERE_W, HOVER_HERE_H);

	// large text
	cairo_set_source_rgb(cr, 1, 1, 1);
	//cairo_set_font_size(cr, 50);
	moveForText(cr, "FRAME 999 (999)", width - 300, height - 50, 300, 50);
	char text[1024];
	sprintf(text, "FRAME %d (%d)", lastFrame, totalFrames);
	cairo_show_text(cr, text);

	lastFrame++;
	//    cairo_surface_flush(cairo_get_target(cr));
}

void platformDrawFrameless(void *platformContext)
{
	auto cr = (cairo_t *)platformContext;

	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_rectangle(cr, 0, 0, POPUP_WIDTH, POPUP_HEIGHT);
	cairo_fill(cr);

	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, 20);
	drawTextRect(cr, "Hello!", 0, 0, POPUP_WIDTH, POPUP_HEIGHT, true);
}

void platformDrawBox(RandomBox *box) {
	// draw new box on background image
	auto cr = cairo_create(imageSurface);
	//
	auto gradient = cairo_pattern_create_linear(box->x, box->y, box->x + box->width, box->y + box->height);
	cairo_pattern_add_color_stop_rgb(gradient, 0, box->r, box->g, box->b);
	cairo_pattern_add_color_stop_rgb(gradient, 1, 0.0, 0.0, 0.0);
	cairo_rectangle(cr, box->x, box->y, box->width, box->height);
	cairo_set_source(cr, gradient);
	cairo_fill(cr);
	//
	cairo_destroy(cr);
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

void platformShutdown()
{
    // nothing on this platform
}

bool platformProvidesDragFormat(const char *formatMIME)
{
	return false;
}

void platformRenderDragFormat(wlRenderPayload payload, const char *formatMIME)
{
	// nothing yet
}

bool platformCheckDropFormats(wlDropData dropData)
{
	return false;
}

void platformHandleDrop(wlDropData dropData)
{
	// nothing yet
}
