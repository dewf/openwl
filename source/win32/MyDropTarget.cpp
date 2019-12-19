#include "MyDropTarget.h"

#include "private_defs.h"
#include "globals.h"

#include "window.h"

#include <assert.h>

void MyDropTarget::setEventFields(wl_Event &event, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) {
    event.dropEvent.modifiers =
        ((grfKeyState & MK_CONTROL) ? wl_kModifierControl : 0) |
        ((grfKeyState & MK_ALT) ? wl_kModifierAlt : 0) |
        ((grfKeyState & MK_SHIFT) ? wl_kModifierShift : 0);

    mWindow->screenToClient((LPPOINT)&pt);
    event.dropEvent.x = pt.x;
    event.dropEvent.y = pt.y;

    event.dropEvent.allowedEffectMask =
        ((*pdwEffect & DROPEFFECT_COPY) ? wl_kDropEffectCopy : 0) |
        ((*pdwEffect & DROPEFFECT_MOVE) ? wl_kDropEffectMove : 0) |
        ((*pdwEffect & DROPEFFECT_LINK) ? wl_kDropEffectLink : 0);
}

void MyDropTarget::updateEffect(wl_Event &event, DWORD *pdwEffect) {
    // post client handling
    auto mask = event.dropEvent.allowedEffectMask;
    if (mask != wl_kDropEffectNone) {
        *pdwEffect =
            ((mask & wl_kDropEffectCopy) ? DROPEFFECT_COPY : 0) |
            ((mask & wl_kDropEffectMove) ? DROPEFFECT_MOVE : 0) |
            ((mask & wl_kDropEffectLink) ? DROPEFFECT_LINK : 0);

        mWindow->setFocus();
    }
    else {
        *pdwEffect = DROPEFFECT_NONE;
    }
}

MyDropTarget::MyDropTarget(wl_WindowRef window) : CDropTarget() {
    mWindow = window;
}
MyDropTarget::~MyDropTarget() {
    printf("MyDropTarget destructor called\n");
}

void MyDropTarget::commonQueryClient(wl_Event &event, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) {
    setEventFields(event, grfKeyState, pt, pdwEffect); // sets the event fields from incoming params

    if ((event.dropEvent.modifiers & wl_kModifierControl) &&
        (event.dropEvent.modifiers & wl_kModifierShift)) {
        event.dropEvent.defaultModifierAction = wl_kDropEffectLink;
    }
    else if (event.dropEvent.modifiers & (wl_kModifierControl)) {
        event.dropEvent.defaultModifierAction = wl_kDropEffectCopy;
    }
    else {
        event.dropEvent.defaultModifierAction = wl_kDropEffectMove;
    }

    mWindow->sendEvent(event);
    updateEffect(event, pdwEffect);
}

HRESULT __stdcall MyDropTarget::DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) {
    printf("hello from DragEnter!\n");

    mDropData = new wl_DropData(pDataObject);
    //mDragData->items.clear();
    //mDragData->pDataObject = pDataObject;

    printf("enter pDataObject: %zX\n", (size_t)pDataObject);

    wl_Event event;
    event._private = nullptr;
    event.eventType = wl_kEventTypeDrop;
    event.dropEvent.eventType = wl_kDropEventTypeFeedback;
    event.dropEvent.data = mDropData;

    commonQueryClient(event, grfKeyState, pt, pdwEffect);

    return S_OK;
}

HRESULT __stdcall MyDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) {
    //printf("Hello from DragOver!\n");
    // event fields same as those in DragEnter
    wl_Event event;
    event._private = nullptr;
    event.eventType = wl_kEventTypeDrop;
    event.dropEvent.eventType = wl_kDropEventTypeFeedback;
    event.dropEvent.data = mDropData;
    commonQueryClient(event, grfKeyState, pt, pdwEffect);
    return S_OK;
}

HRESULT __stdcall MyDropTarget::DragLeave(void) {
    printf("Goodbye from DragLeave :( \n");
    //event.dropEvent.data = nullptr;
    // can we free mDragData without destroying the COM object it's wrapping? seems to crash things if that happens...
    mDropData->recvObject = nullptr; // let COM deal with freeing it...
    delete mDropData;
    mDropData = nullptr;
    return S_OK;
}

HRESULT __stdcall MyDropTarget::Drop(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) {

    wl_Event event;
    event._private = nullptr;
    event.eventType = wl_kEventTypeDrop;
    event.dropEvent.eventType = wl_kDropEventTypeDrop;
    event.dropEvent.data = mDropData;

    printf("Hello from Drop! data: %zX\n", (size_t)event.dropEvent.data);
    //event.dropEvent.eventType = WLDropEvent_Drop;

    assert(pDataObject == mDropData->recvObject);
    //mDragData->pDataObject = pDataObject; // could it be different?
    printf("drop pDataObject: %zX\n", (size_t)pDataObject);

    commonQueryClient(event, grfKeyState, pt, pdwEffect);

    //mDragData->pDataObject = nullptr;
    delete mDropData;

    return S_OK;
}
