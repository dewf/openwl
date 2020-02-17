#include "wndproc.h"

#include "../openwl.h"
#include "window.h"
#include "private_defs.h"

#include <stdio.h>
#include "globals.h"

#include "action.h"
#include "timer.h"

// app-global window proc stuff ==================================================

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

// top-level window proc ==================================================================

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
    case WM_COMMAND:
    {
        int id = LOWORD(wParam);
        wlw->onAction(event, id);
        if (!event.handled) {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }

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

    case WM_CHAR:
    {
        wlw->onChar(event, wParam, lParam);
        if (!event.handled) {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }

    case WM_KEYDOWN:
    case WM_KEYUP:
    {
        wlw->onKey(event, message, wParam, lParam);
        if (!event.handled) {
            return DefWindowProcW(hWnd, message, wParam, lParam);
        }
        break;
    }

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
        wlw->onMouseButton(event, message, wParam, lParam);
        if (!event.handled) {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

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

    case WM_MOUSEWHEEL:
    case WM_MOUSEHWHEEL: // handle horizontal wheel as well
        wlw->onMouseWheel(event, message, wParam, lParam);
        if (!event.handled) {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

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

	case WM_DPICHANGED: {
		wlw->onDPIChanged(LOWORD(wParam), (RECT*)lParam);
		break;
	}

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}
