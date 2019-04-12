#pragma once

#include "dragdrop/droptarget.h"
#include "../openwl.h"

class MyDropTarget: public CDropTarget
{
private:
    wlWindow mWindow;
    wlDropData mDropData;
    void setEventFields(WLEvent &event, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
    void updateEffect(WLEvent &event, DWORD *pdwEffect);

public:
    MyDropTarget(wlWindow window);
    ~MyDropTarget();

    void commonQueryClient(WLEvent &event, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    HRESULT __stdcall DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) override;
    HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) override;
    HRESULT __stdcall DragLeave(void) override;
    HRESULT __stdcall Drop(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) override;
};

