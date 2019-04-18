#pragma once

#include "dragdrop/droptarget.h"
#include "../openwl.h"

class MyDropTarget: public CDropTarget
{
private:
    wl_Window mWindow;
    wl_DropData mDropData;
    void setEventFields(wl_Event &event, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
    void updateEffect(wl_Event &event, DWORD *pdwEffect);

public:
    MyDropTarget(wl_Window window);
    ~MyDropTarget();

    void commonQueryClient(wl_Event &event, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    HRESULT __stdcall DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) override;
    HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) override;
    HRESULT __stdcall DragLeave(void) override;
    HRESULT __stdcall Drop(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) override;
};

