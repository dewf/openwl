#include "wndproc.h"

#include "../openwl.h"
#include "private_defs.h"
#include "globals.h"

#include "unicodestuff.h"

#include "keystuff.h" // locationForKey, keyInfo, etc

#include <windowsx.h> // for some macros (GET_X_LPARAM etc)
#include <assert.h>

unsigned int getKeyModifiers() {
    unsigned int modifiers = 0;
    modifiers |= (GetKeyState(VK_CONTROL) & 0x8000) ? WLModifier_Control : 0;
    modifiers |= (GetKeyState(VK_MENU) & 0x8000) ? WLModifier_Alt : 0;
    modifiers |= (GetKeyState(VK_SHIFT) & 0x8000) ? WLModifier_Shift : 0;
    return modifiers;
}

unsigned int getMouseModifiers(WPARAM wParam) {
	unsigned int modifiers = 0;
	auto fwKeys = GET_KEYSTATE_WPARAM(wParam);
	modifiers |= (fwKeys & MK_CONTROL) ? WLModifier_Control : 0;
	modifiers |= (fwKeys & MK_SHIFT) ? WLModifier_Shift : 0;
	modifiers |= (fwKeys & MK_ALT) ? WLModifier_Alt : 0;
	return modifiers;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    wlWindow wlw = (wlWindow)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    _wlEventPrivate eventPrivate(message, wParam, lParam);
	WLEvent event = {};
    event._private = &eventPrivate;
    event.eventType = WLEventType_None;
    event.handled = false;

    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);

        auto found = actionMap.find(wmId);
        if (found != actionMap.end()) {
            auto action = found->second;
            event.eventType = WLEventType_Action;
            event.actionEvent.action = action; // aka value
            event.actionEvent.id = action->id;
            eventCallback(wlw, &event, wlw->userData);
        }
        if (!event.handled) {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_ERASEBKGND:
        //printf("nothx erase background!\n");
        return 1;
        //break;

    case WM_SIZE:
    {
        if (wlw) {
            RECT clientRect;
            GetClientRect(hWnd, &clientRect);
            int newWidth = clientRect.right - clientRect.left;
            int newHeight = clientRect.bottom - clientRect.top;

            event.eventType = WLEventType_WindowResized;
            event.resizeEvent.newWidth = newWidth;
            event.resizeEvent.newHeight = newHeight;
            event.resizeEvent.oldWidth = wlw->clientWidth; // hasn't updated yet
            event.resizeEvent.oldHeight = wlw->clientHeight;
            eventCallback(wlw, &event, wlw->userData);

            //printf("old w/h: %d,%d  --- new: %d, %d (extra %d,%d)\n", wlw->clientWidth, wlw->clientHeight, newWidth, newHeight,
            //    wlw->extraWidth, wlw->extraHeight);

            // update
            wlw->clientWidth = newWidth;
            wlw->clientHeight = newHeight;

            if (useDirect2D) {
                auto size = D2D1::SizeU(newWidth, newHeight);
                auto hr = wlw->d2dRenderTarget->Resize(size);
                assert(hr == S_OK);
            }
        }
        if (!event.handled) {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        event.eventType = WLEventType_WindowRepaint;
        event.repaintEvent.x = ps.rcPaint.left;
        event.repaintEvent.y = ps.rcPaint.top;
        event.repaintEvent.width = (ps.rcPaint.right - ps.rcPaint.left);
        event.repaintEvent.height = (ps.rcPaint.bottom - ps.rcPaint.top);

        if (useDirect2D) {
            WLPlatformContextD2D platformContext;
            platformContext.factory = d2dFactory;
            platformContext.target = wlw->d2dRenderTarget;
            //platformContext.writeFactory = writeFactory;
            event.repaintEvent.platformContext = &platformContext;

            wlw->d2dRenderTarget->BeginDraw();
            eventCallback(wlw, &event, wlw->userData);
            auto hr = wlw->d2dRenderTarget->EndDraw();
			if (hr == D2DERR_RECREATE_TARGET) {
				d2dCreateTarget(wlw);
			}
        }
        else {
            event.repaintEvent.platformContext = hdc;
            eventCallback(wlw, &event, wlw->userData);
        }

        EndPaint(hWnd, &ps);

        if (!event.handled) {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_CLOSE:
        event.eventType = WLEventType_WindowCloseRequest;
        event.closeRequestEvent.cancelClose = false;
        eventCallback(wlw, &event, wlw->userData);
        if (event.handled && event.closeRequestEvent.cancelClose) {
            // canceled -- do nothing / return 0
        }
        else {
            //DestroyWindow(hWnd);
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

    case WM_DESTROY:
        event.eventType = WLEventType_WindowDestroyed;
        event.destroyEvent.reserved = 0;
        eventCallback(wlw, &event, wlw->userData);

        // free the associated WindowData
        printf("freeing window handle\n");
        delete wlw;

        if (!event.handled) {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO *mmi = (MINMAXINFO *)lParam;

        if (wlw) {
            // min
            if (wlw->props.usedFields & WLWindowProp_MinWidth) {
                mmi->ptMinTrackSize.x = wlw->props.minWidth + wlw->extraWidth;
            }
            if (wlw->props.usedFields & WLWindowProp_MinHeight) {
                mmi->ptMinTrackSize.y = wlw->props.minHeight + wlw->extraHeight;
            }

            // max
            if (wlw->props.usedFields & WLWindowProp_MaxWidth) {
                mmi->ptMaxTrackSize.x = wlw->props.maxWidth + wlw->extraWidth;
            }
            if (wlw->props.usedFields & WLWindowProp_MaxHeight) {
                mmi->ptMaxTrackSize.y = wlw->props.maxHeight + wlw->extraHeight;
            }
        }
        else {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }

    case OPENWL_TIMER_MESSAGE:
    {
        auto timer = (wlTimer)lParam;

        event.eventType = WLEventType_Timer;
        event.timerEvent.timer = timer;
        event.timerEvent.timerID = timer->timerID;
        event.timerEvent.stopTimer = false;

        LARGE_INTEGER perfCount;
        QueryPerformanceCounter(&perfCount);

        auto sinceLast = (double)(perfCount.QuadPart - timer->lastPerfCount.QuadPart);
        sinceLast /= perfCounterTicksPerSecond.QuadPart;
        event.timerEvent.secondsSinceLast = sinceLast;

        eventCallback(wlw, &event, wlw->userData);

        timer->lastPerfCount = perfCount;

        // custom event so there's no defwindowproc handling
        if (event.handled && event.timerEvent.stopTimer) {
            // stop the timer immediately
            printf("win32 OPENWL_TIMER_MESSAGE: cancel not yet implemented\n");
        }
        break;
    }

    case WM_CHAR:
    {
        unsigned char scanCode = (lParam >> 16) & 0xFF;
        bool found = (suppressedScanCodes.find(scanCode) != suppressedScanCodes.end());
        if (!found) {
            static wchar_t utf16_buffer[10];
            static int buf_len = 0;
            utf16_buffer[buf_len++] = (wchar_t)wParam;
            if (wParam >= 0xD800 && wParam <= 0xDFFF) {
                if (buf_len == 2) {
                    utf16_buffer[2] = 0;
                    auto utf8 = wstring_to_utf8(utf16_buffer);

                    event.eventType = WLEventType_Key;
                    event.keyEvent.eventType = WLKeyEventType_Char;
                    event.keyEvent.key = WLKey_Unknown;
                    event.keyEvent.modifiers = 0; // not used here

                    event.keyEvent.string = utf8.c_str();
                    eventCallback(wlw, &event, wlw->userData);

                    buf_len = 0;
                }
            }
            else {
                // single char
                utf16_buffer[1] = 0;
                auto utf8 = wstring_to_utf8(utf16_buffer);

                event.eventType = WLEventType_Key;
                event.keyEvent.eventType = WLKeyEventType_Char;
                event.keyEvent.key = WLKey_Unknown;
                event.keyEvent.modifiers = 0; // not used here

                event.keyEvent.string = utf8.c_str();
                eventCallback(wlw, &event, wlw->userData);

                buf_len = 0;
            }
        }
        if (!event.handled) {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }

    case WM_KEYDOWN:
	case WM_KEYUP:
    {
        unsigned char scanCode = (lParam >> 16) & 0xFF;
        bool extended = ((lParam >> 24) & 0x1) ? true : false;

        KeyInfo *value = keyMap[0]; // WLKey_Unknown
        auto found = keyMap.find(wParam); // wParam = win32 virtual key
        if (found != keyMap.end()) {
            value = found->second;
        }
        if (value->key != WLKey_Unknown) {
            event.eventType = WLEventType_Key;
            event.keyEvent.eventType = (message == WM_KEYDOWN ? WLKeyEventType_Down : WLKeyEventType_Up);
            event.keyEvent.key = value->key;
            event.keyEvent.modifiers = getKeyModifiers(); // should probably be tracking ctrl/alt/shift state as we go, instead of grabbing instantaneously here
            event.keyEvent.string = value->stringRep;

            // use scancode, extended value, etc to figure out location (left/right/numpad/etc)
            if (value->knownLocation >= 0) {
                event.keyEvent.location = (WLKeyLocation)value->knownLocation;
            }
            else {
                event.keyEvent.location = locationForKey(value->key, scanCode, extended);
            }

            eventCallback(wlw, &event, wlw->userData);

			if (message == WM_KEYDOWN) { // is this needed for KEYUP too?

				// does the WM_CHAR need to be suppressed?
				// just add it to the blacklisted set (redundant but sufficient)
				// could/should use a queue instead, but not every WM_CHAR has a corresponding/prior WM_KEYDOWN event
				if (value->suppressCharEvent) {
					suppressedScanCodes.insert(scanCode);
				}
			}
        }
        else {
            printf("### unrecognized: VK %zd, scan %d [%s]\n", wParam, scanCode, extended ? "ext" : "--");
        }
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
        event.eventType = WLEventType_Mouse;

        switch (message) {
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
            event.mouseEvent.eventType = WLMouseEventType_MouseDown;
            break;
        default:
            event.mouseEvent.eventType = WLMouseEventType_MouseUp;
        }

        switch (message) {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
            event.mouseEvent.button = WLMouseButton_Left;
            break;
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
            event.mouseEvent.button = WLMouseButton_Middle;
            break;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
            event.mouseEvent.button = WLMouseButton_Right;
            break;
        }
        event.mouseEvent.x = GET_X_LPARAM(lParam);
        event.mouseEvent.y = GET_Y_LPARAM(lParam);

        eventCallback(wlw, &event, wlw->userData);
        if (!event.handled) {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

    case WM_MOUSEMOVE:
		event.eventType = WLEventType_Mouse;
		event.mouseEvent.x = GET_X_LPARAM(lParam);
		event.mouseEvent.y = GET_Y_LPARAM(lParam);
		//event.mouseEvent.modifiers = getMouseModifiers(wParam);

		if (!wlw->mouseInWindow) {
			// synthesize an "enter" event before the 1st move event sent
			event.mouseEvent.eventType = WLMouseEventType_MouseEnter;
			eventCallback(wlw, &event, wlw->userData);

			// indicate that we want a WM_MOUSELEAVE event to balance this
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.hwndTrack = hWnd;
			tme.dwFlags = TME_LEAVE;
			tme.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent(&tme);

			wlw->mouseInWindow = true;
			event.handled = false; // reset for next dispatch below
		}

        event.mouseEvent.eventType = WLMouseEventType_MouseMove;
        // button, modifiers etc not relevant

        eventCallback(wlw, &event, wlw->userData);
        if (!event.handled) {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

	case WM_MOUSEWHEEL:
		event.eventType = WLEventType_Mouse;
		event.mouseEvent.eventType = WLMouseEventType_MouseWheel;
		event.mouseEvent.wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		event.mouseEvent.x = GET_X_LPARAM(lParam);
		event.mouseEvent.y = GET_Y_LPARAM(lParam);
		event.mouseEvent.modifiers = getMouseModifiers(wParam);

		eventCallback(wlw, &event, wlw->userData);
		if (!event.handled) {
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_MOUSELEAVE:
		event.eventType = WLEventType_Mouse;
		event.mouseEvent.eventType = WLMouseEventType_MouseLeave;
		// no other values come with event

		wlw->mouseInWindow = false;
		
		eventCallback(wlw, &event, wlw->userData);
		if (!event.handled) {
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

    case WM_MainThreadExecMsg:
        ExecuteMainItem((MainThreadExecItem *)lParam);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}
