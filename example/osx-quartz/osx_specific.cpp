#include "../../library/openwl.h"

#include "../main.h"

#include <CoreFoundation/CoreFoundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreText/CoreText.h>

#include <stdio.h>
#include <thread>
#include <vector>

static CGContextRef bitmapContext;
static CTFontRef font;
static CTLineRef dragSourceLine, dropTargetLine, hoverHereLine, helloLine, clickHereLine, modalLine;
static CGColorRef blackColor, whiteColor;

CTLineRef createLineWithFont(CFStringRef str, CTFontRef font, CGColorRef color) {
    const CFTypeRef keys[2] = { kCTFontAttributeName, kCTForegroundColorAttributeName };
    const CFTypeRef values[2] = { font, color };
    auto attrs = CFDictionaryCreate(kCFAllocatorDefault,
                                   (const void **)keys,
                                   (const void **)values,
                                   2,
                                   &kCFTypeDictionaryKeyCallBacks,
                                   &kCFTypeDictionaryValueCallBacks);
    auto attrString = CFAttributedStringCreate(kCFAllocatorDefault, str, attrs);
    auto line = CTLineCreateWithAttributedString(attrString);
    CFRelease(attrString);
    CFRelease(attrs);
    return line;
}

void platformInit() {
    auto colorspace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    bitmapContext = CGBitmapContextCreate(nullptr, MAX_WIDTH, MAX_HEIGHT, 8, 0, colorspace, kCGImageAlphaNoneSkipLast);
    auto data = (unsigned int *)CGBitmapContextGetData(bitmapContext);
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            unsigned char red = (j * 256) / MAX_WIDTH;
            unsigned char green = (i * 256) / MAX_HEIGHT;
            unsigned char blue = 255 - green;
            data[i * MAX_WIDTH + j] = ((blue << 16) + (green << 8) + red) | 0xff000000;
        }
    }
    CGColorSpaceRelease(colorspace);
    
    
    font = CTFontCreateWithName(CFSTR("ArialMT"), 30.0, NULL);
    
    blackColor = CGColorGetConstantColor(kCGColorBlack);
    whiteColor = CGColorGetConstantColor(kCGColorWhite);
    dragSourceLine = createLineWithFont(CFSTR("Drag Source"), font, blackColor);
    dropTargetLine = createLineWithFont(CFSTR("Drop Target"), font, blackColor);
    hoverHereLine = createLineWithFont(CFSTR("Hover Here"), font, blackColor);
    helloLine = createLineWithFont(CFSTR("Hello!"), font, blackColor);
    clickHereLine = createLineWithFont(CFSTR("Click Here"), font, blackColor);
    modalLine = createLineWithFont(CFSTR("MODAL"), font, blackColor);
}

static void drawLineAt(CGContextRef context, CTLineRef line, int x, int y) {
    CGFloat baseline;
    CTLineGetTypographicBounds(line, &baseline, 0, 0);
    
    CGContextSaveGState(context);
    CGContextTranslateCTM(context, 0, height);
    CGContextScaleCTM(context, 1.0, -1.0);
    
    CGContextSetTextPosition(context, x, height - y);
    CTLineDraw(line, context);
    
    CGContextRestoreGState(context);
}

static void drawTextRect(CGContextRef context, CTLineRef line, int x, int y, int width, int height, bool textOnly = false) {
    
    auto rect = CGRectMake(x, y, width, height);
    
    if (!textOnly) {
        CGContextSetLineWidth(context, 2);
        CGContextAddRect(context, rect);
        CGContextStrokePath(context);
        
        CGContextSetRGBFillColor(context, 0.5, 0.5, 0.5, 1);
        CGContextFillRect(context, rect);
    }
    CGContextSetRGBStrokeColor(context, 0, 0, 0, 1);

    CGFloat baseline;
    CGRect bounds;
    
    bounds = CTLineGetBoundsWithOptions(line, 0);
    CTLineGetTypographicBounds(line, &baseline, 0, 0);
    
    auto x1 = x + (width - bounds.size.width) / 2;
    auto y1 = y + (height - bounds.size.height) / 2 + baseline;
    
    CGContextSaveGState(context);
    CGContextTranslateCTM(context, 0, height);
    CGContextScaleCTM(context, 1.0, -1.0);
    
    CGContextSetTextPosition(context, x1, height - y1);
    CTLineDraw(line, context);
    
    CGContextRestoreGState(context);
}

void platformDraw(wl_PlatformContext *platformContext) {
    auto context = (CGContextRef)platformContext->context;
    
    CGContextSaveGState(context);
    
    CGContextSetTextMatrix(context, CGAffineTransformIdentity);

    auto image = CGBitmapContextCreateImage(bitmapContext);
    CGContextDrawImage(context, CGRectMake(0, 0, MAX_WIDTH, MAX_HEIGHT), image);
    CGImageRelease(image);

	// draw crossed lines
    CGContextSetLineWidth(context, 4.0);
    CGContextSetRGBStrokeColor(context, 0, 0, 0, 1);
    
    CGContextMoveToPoint(context, 10, 10);
    CGContextAddLineToPoint(context, width - 10, height - 10);
    CGContextStrokePath(context);
    
    CGContextMoveToPoint(context, 10, height - 10);
    CGContextAddLineToPoint(context, width - 10, 10);
    CGContextStrokePath(context);

	// thin outer rect
    CGContextSetLineWidth(context, 1.0);
    CGContextAddRect(context, CGRectMake(2.5, 2.5, width - 5, height - 5));
    CGContextStrokePath(context);
    
	drawTextRect(context, dragSourceLine, DRAG_SOURCE_X, DRAG_SOURCE_Y, DRAG_SOURCE_W, DRAG_SOURCE_H);
	drawTextRect(context, dropTargetLine, DROP_TARGET_X, DROP_TARGET_Y, DROP_TARGET_W, DROP_TARGET_H);
    drawTextRect(context, hoverHereLine, HOVER_HERE_X, HOVER_HERE_Y, HOVER_HERE_W, HOVER_HERE_H);
    drawTextRect(context, clickHereLine, MSG_CLICK_X, MSG_CLICK_Y, MSG_CLICK_W, MSG_CLICK_H);
    
    char buffer[1024];
    snprintf(buffer, 1024, "FRAME %d (%d)", lastFrame, totalFrames);
    auto cfstr = CFStringCreateWithCString(kCFAllocatorDefault, buffer, kCFStringEncodingUTF8);
    auto line = createLineWithFont(cfstr, font, whiteColor);
    CGContextSetRGBStrokeColor(context, 1, 1, 1, 1);
    drawLineAt(context, line, width - 250, height - 10);
    CFRelease(cfstr);
    CFRelease(line);
    
    lastFrame++;

    CGContextRestoreGState(context);
}

void platformDrawFrameless(wl_PlatformContext *platformContext)
{
    auto context = (CGContextRef)platformContext->context;
    CGContextSaveGState(context);
    // ====
    CGContextSetRGBFillColor(context, 1, 1, 1, 1);
    CGContextFillRect(context, CGRectMake(0, 0, POPUP_WIDTH, POPUP_HEIGHT));
    drawTextRect(context, helloLine, 0, 0, POPUP_WIDTH, POPUP_HEIGHT, true);
    // ====
    CGContextRestoreGState(context);
}

void platformDrawModal(wl_PlatformContext* platformContext)
{
    auto context = (CGContextRef)platformContext->context;
    CGContextSaveGState(context);
    // ====
    CGContextSetRGBFillColor(context, 0.3, 0.3, 1, 1);
    CGContextFillRect(context, CGRectMake(0, 0, modalWidth, modalHeight));
    drawTextRect(context, modalLine, 0, 0, modalWidth, modalHeight, true);
    // ====
    CGContextRestoreGState(context);
}

CGGradientRef easyGrad(CGFloat fromR, CGFloat fromG, CGFloat fromB, CGFloat fromA,
                       CGFloat toR, CGFloat toG, CGFloat toB, CGFloat toA)
{
    static auto colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    
    CGFloat components[] = {
        fromR, fromG, fromB, fromA,
        toR, toG, toB, toA
    };
    CGFloat locations[] = {
        0.0, 1.0
    };
    return CGGradientCreateWithColorComponents(colorSpace, components, locations, 2);
}


void platformDrawBox(RandomBox *box) {
    auto grad = easyGrad(box->r, box->g, box->b, 1, 0, 0, 0, 1);
    
    CGContextSaveGState(bitmapContext);
    
    CGContextClipToRect(bitmapContext, CGRectMake(box->x, box->y, box->width, box->height));
    auto start = CGPointMake(box->x, box->y);
    auto end = CGPointMake(box->x + box->width, box->y + box->height);
    CGContextDrawLinearGradient(bitmapContext, grad, start, end, 0);
    
    CGContextRestoreGState(bitmapContext); // pop clip
    
    CGGradientRelease(grad);
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
    // noop on mac
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


