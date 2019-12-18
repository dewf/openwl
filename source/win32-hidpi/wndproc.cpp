#include "wndproc.h"

#include "../openwl.h"
#include "window.h"
#include "private_defs.h"

#include <stdio.h>
#include "globals.h"

#include "timer.h"
//
//#include "unicodestuff.h"
//
//#include "keystuff.h" // locationForKey, keyInfo, etc
//
//#include <windowsx.h> // for some macros (GET_X_LPARAM etc)
//#include <assert.h>
//
// app-global window proc

void ExecuteMainItem(MainThreadExecItem* item) {
    std::lock_guard<std::mutex> lock(execMutex);
    item->callback(item->data);
    item->execCond.notify_one();
}

LRESULT CALLBACK appGlobalWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (hWnd == appGlobalWindow) {
        switch (message) {
        case WM_WLTimerMessage: {
            auto timer = (wl_TimerRef)lParam;
            timer->onTimerMessage(message, wParam, lParam);
            break;
        }
        case WM_WLMainThreadExecMsg:
            ExecuteMainItem((MainThreadExecItem*)lParam);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    else {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK topLevelWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    wl_WindowRef wlw = (wl_WindowRef)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    wl_EventPrivate eventPrivate(message, wParam, lParam);
    wl_Event event = {};
    event._private = &eventPrivate;
    event.eventType = wl_kEventTypeNone;
    event.handled = false;

    switch (message)
    {
    //case WM_COMMAND:
    //{
    //    int wmId = LOWORD(wParam);

    //    auto found = actionMap.find(wmId);
    //    if (found != actionMap.end()) {
    //        auto action = found->second;
    //        event.eventType = wl_kEventTypeAction;
    //        event.actionEvent.action = action; // aka value
    //        event.actionEvent.id = action->id;
    //        eventCallback(wlw, &event, wlw->userData);
    //    }
    //    if (!event.handled) {
    //        return DefWindowProc(hWnd, message, wParam, lParam);
    //    }
    //}
    //break;

    case WM_ERASEBKGND:
        //printf("nothx erase background!\n");
        return 1; // nonzero = we're handling background erasure -- keeps windows from doing it
        //break;

    case WM_SIZE:
    {
        if (wlw) {
            // apparently we receive this event with null handles sometimes?
            wlw->onSize(event);
        }
        if (!event.handled) {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }

    case WM_PAINT:
    {
        wlw->onPaint(event);
        if (!event.handled) {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_CLOSE:
        wlw->onClose(event);
        if (event.handled && event.closeRequestEvent.cancelClose) {
            // window not ready to close! -- do nothing, return 0
        }
        else {
            // onward to destruction ...
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

    case WM_DESTROY:
        wlw->onDestroy(event);
        if (!event.handled) {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

    case WM_GETMINMAXINFO:
    {
        if (wlw) {
            MINMAXINFO* mmi = (MINMAXINFO*)lParam;
            wlw->onGetMinMaxInfo(mmi);
        }
        else {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }

    //case WM_CHAR:
    //{
    //    unsigned char scanCode = (lParam >> 16) & 0xFF;
    //    bool found = (suppressedScanCodes.find(scanCode) != suppressedScanCodes.end());
    //    if (!found) {
    //        static wchar_t utf16_buffer[10];
    //        static int buf_len = 0;
    //        utf16_buffer[buf_len++] = (wchar_t)wParam;
    //        if (wParam >= 0xD800 && wParam <= 0xDFFF) {
    //            if (buf_len == 2) {
    //                utf16_buffer[2] = 0;
    //                auto utf8 = wstring_to_utf8(utf16_buffer);

    //                event.eventType = wl_kEventTypeKey;
    //                event.keyEvent.eventType = wl_kKeyEventTypeChar;
    //                event.keyEvent.key = wl_kKeyUnknown;
    //                event.keyEvent.modifiers = 0; // not used here

    //                event.keyEvent.string = utf8.c_str();
    //                eventCallback(wlw, &event, wlw->userData);

    //                buf_len = 0;
    //            }
    //        }
    //        else {
    //            // single char
    //            utf16_buffer[1] = 0;
    //            auto utf8 = wstring_to_utf8(utf16_buffer);

    //            event.eventType = wl_kEventTypeKey;
    //            event.keyEvent.eventType = wl_kKeyEventTypeChar;
    //            event.keyEvent.key = wl_kKeyUnknown;
    //            event.keyEvent.modifiers = 0; // not used here

    //            event.keyEvent.string = utf8.c_str();
    //            eventCallback(wlw, &event, wlw->userData);

    //            buf_len = 0;
    //        }
    //    }
    //    if (!event.handled) {
    //        return DefWindowProc(hWnd, message, wParam, lParam);
    //    }
    //    break;
    //}

    //case WM_KEYDOWN:
    //case WM_KEYUP:
    //{
    //    unsigned char scanCode = (lParam >> 16) & 0xFF;
    //    bool extended = ((lParam >> 24) & 0x1) ? true : false;

    //    KeyInfo* value = keyMap[0]; // wl_kKeyUnknown
    //    auto found = keyMap.find(wParam); // wParam = win32 virtual key
    //    if (found != keyMap.end()) {
    //        value = found->second;
    //    }
    //    if (value->key != wl_kKeyUnknown) {
    //        event.eventType = wl_kEventTypeKey;
    //        event.keyEvent.eventType = (message == WM_KEYDOWN ? wl_kKeyEventTypeDown : wl_kKeyEventTypeUp);
    //        event.keyEvent.key = value->key;
    //        event.keyEvent.modifiers = getKeyModifiers(); // should probably be tracking ctrl/alt/shift state as we go, instead of grabbing instantaneously here
    //        event.keyEvent.string = value->stringRep;

    //        // use scancode, extended value, etc to figure out location (left/right/numpad/etc)
    //        if (value->knownLocation >= 0) {
    //            event.keyEvent.location = (wl_KeyLocation)value->knownLocation;
    //        }
    //        else {
    //            event.keyEvent.location = locationForKey(value->key, scanCode, extended);
    //        }

    //        eventCallback(wlw, &event, wlw->userData);

    //        if (message == WM_KEYDOWN) { // is this needed for KEYUP too?

    //            // does the WM_CHAR need to be suppressed?
    //            // just add it to the blacklisted set (redundant but sufficient)
    //            // could/should use a queue instead, but not every WM_CHAR has a corresponding/prior WM_KEYDOWN event
    //            if (value->suppressCharEvent) {
    //                suppressedScanCodes.insert(scanCode);
    //            }
    //        }
    //    }
    //    else {
    //        printf("### unrecognized: VK %zd, scan %d [%s]\n", wParam, scanCode, extended ? "ext" : "--");
    //    }
    //    if (!event.handled) {
    //        return DefWindowProcW(hWnd, message, wParam, lParam);
    //    }
    //    break;
    //}

    //case WM_LBUTTONDOWN:
    //case WM_LBUTTONUP:
    //case WM_MBUTTONDOWN:
    //case WM_MBUTTONUP:
    //case WM_RBUTTONDOWN:
    //case WM_RBUTTONUP:
    //    event.eventType = wl_kEventTypeMouse;

    //    switch (message) {
    //    case WM_LBUTTONDOWN:
    //    case WM_MBUTTONDOWN:
    //    case WM_RBUTTONDOWN:
    //        event.mouseEvent.eventType = wl_kMouseEventTypeMouseDown;
    //        break;
    //    default:
    //        event.mouseEvent.eventType = wl_kMouseEventTypeMouseUp;
    //    }

    //    switch (message) {
    //    case WM_LBUTTONDOWN:
    //    case WM_LBUTTONUP:
    //        event.mouseEvent.button = wl_kMouseButtonLeft;
    //        break;
    //    case WM_MBUTTONDOWN:
    //    case WM_MBUTTONUP:
    //        event.mouseEvent.button = wl_kMouseButtonMiddle;
    //        break;
    //    case WM_RBUTTONDOWN:
    //    case WM_RBUTTONUP:
    //        event.mouseEvent.button = wl_kMouseButtonRight;
    //        break;
    //    }
    //    event.mouseEvent.x = GET_X_LPARAM(lParam);
    //    event.mouseEvent.y = GET_Y_LPARAM(lParam);

    //    event.mouseEvent.modifiers = getMouseModifiers(wParam);

    //    eventCallback(wlw, &event, wlw->userData);
    //    if (!event.handled) {
    //        return DefWindowProc(hWnd, message, wParam, lParam);
    //    }
    //    break;

    case WM_MOUSEMOVE: {
        bool ignored = false;
        wlw->onMouseMove(event, wParam, lParam, &ignored);
        if (ignored) {
            return 0; // no default handling -- pretend it didn't happen
        }
        if (!event.handled) {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }

    //case WM_MOUSEWHEEL:
    //case WM_MOUSEHWHEEL: // handle horizontal wheel as well
    //    event.eventType = wl_kEventTypeMouse;
    //    event.mouseEvent.eventType = wl_kMouseEventTypeMouseWheel;
    //    event.mouseEvent.wheelAxis = (message == WM_MOUSEWHEEL) ? wl_kMouseWheelAxisVertical : wl_kMouseWheelAxisHorizontal;
    //    event.mouseEvent.wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
    //    // x,y are screen coords, convert to window space
    //    POINT p;
    //    p.x = GET_X_LPARAM(lParam);
    //    p.y = GET_Y_LPARAM(lParam);
    //    ScreenToClient(hWnd, &p);
    //    event.mouseEvent.x = p.x;
    //    event.mouseEvent.y = p.y;
    //    event.mouseEvent.modifiers = getMouseModifiers(wParam);

    //    eventCallback(wlw, &event, wlw->userData);
    //    if (!event.handled) {
    //        return DefWindowProc(hWnd, message, wParam, lParam);
    //    }
    //    break;

    case WM_MOUSELEAVE:
        wlw->onMouseLeave(event);
        if (!event.handled) {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        // experimentally determined:
        //   don't bother resetting the cursor on its way out:
        //   1) windows seems to always handle it,
        //   2) if we have any overlapping (popup etc) windows of our own,
        //      because of the different timing of the event queues,
        //      there is no guarantee the "leave" event of one window will be processed
        //      before the (synthesized) "enter" of another --
        //      which was causing the explicitly set mouse pointer to be overridden incorrectly
        //      by the sometimes-late "leave" event
        break;

    //    //case WM_ENTERSIZEMOVE:
    //    //	printf("@@@ ENTER SIZE MOVE\n");
    //    //	break;

    //    //case WM_EXITSIZEMOVE:
    //    //	printf("@@@ EXIT SIZE MOVE\n");
    //    //	break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}
