#include "MyDropTarget.h"

#include "private_defs.h"
#include "globals.h"

#include <assert.h>

void MyDropTarget::setEventFields(WLEvent &event, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) {
    event.dropEvent.modifiers =
        ((grfKeyState & MK_CONTROL) ? WLModifier_Control : 0) |
        ((grfKeyState & MK_ALT) ? WLModifier_Alt : 0) |
        ((grfKeyState & MK_SHIFT) ? WLModifier_Shift : 0);

    ScreenToClient(mWindow->hwnd, (LPPOINT)&pt);
    event.dropEvent.x = pt.x;
    event.dropEvent.y = pt.y;

    event.dropEvent.allowedEffectMask =
        ((*pdwEffect & DROPEFFECT_COPY) ? WLDropEffect_Copy : 0) |
        ((*pdwEffect & DROPEFFECT_MOVE) ? WLDropEffect_Move : 0) |
        ((*pdwEffect & DROPEFFECT_LINK) ? WLDropEffect_Link : 0);
}

void MyDropTarget::updateEffect(WLEvent &event, DWORD *pdwEffect) {
    // post client handling
    auto mask = event.dropEvent.allowedEffectMask;
    if (mask != WLDropEffect_None) {
        *pdwEffect =
            ((mask & WLDropEffect_Copy) ? DROPEFFECT_COPY : 0) |
            ((mask & WLDropEffect_Move) ? DROPEFFECT_MOVE : 0) |
            ((mask & WLDropEffect_Link) ? DROPEFFECT_LINK : 0);

        SetFocus(mWindow->hwnd);
    }
    else {
        *pdwEffect = DROPEFFECT_NONE;
    }
}

MyDropTarget::MyDropTarget(wlWindow window) : CDropTarget() {
    mWindow = window;
}
MyDropTarget::~MyDropTarget() {
    printf("MyDropTarget destructor called\n");
}

void MyDropTarget::commonQueryClient(WLEvent &event, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) {
    setEventFields(event, grfKeyState, pt, pdwEffect); // sets the event fields from incoming params

    if ((event.dropEvent.modifiers & WLModifier_Control) &&
        (event.dropEvent.modifiers & WLModifier_Shift)) {
        event.dropEvent.defaultModifierAction = WLDropEffect_Link;
    }
    else if (event.dropEvent.modifiers & (WLModifier_Control)) {
        event.dropEvent.defaultModifierAction = WLDropEffect_Copy;
    }
    else {
        event.dropEvent.defaultModifierAction = WLDropEffect_Move;
    }

    eventCallback(mWindow, &event, mWindow->userData);
    updateEffect(event, pdwEffect);
}

HRESULT __stdcall MyDropTarget::DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) {
    printf("hello from DragEnter!\n");

    mDropData = new _wlDropData(pDataObject);
    //mDragData->items.clear();
    //mDragData->pDataObject = pDataObject;

    printf("enter pDataObject: %zX\n", (size_t)pDataObject);

    WLEvent event;
    event._private = nullptr;
    event.eventType = WLEventType_Drop;
    event.dropEvent.eventType = WLDropEventType_Feedback;
    event.dropEvent.data = mDropData;

    commonQueryClient(event, grfKeyState, pt, pdwEffect);

    return S_OK;
}

HRESULT __stdcall MyDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) {
    //printf("Hello from DragOver!\n");
    // event fields same as those in DragEnter
    WLEvent event;
    event._private = nullptr;
    event.eventType = WLEventType_Drop;
    event.dropEvent.eventType = WLDropEventType_Feedback;
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

    WLEvent event;
    event._private = nullptr;
    event.eventType = WLEventType_Drop;
    event.dropEvent.eventType = WLDropEventType_Drop;
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
