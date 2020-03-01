#pragma once

#include "dragdrop/droptarget.h"
#include "../openwl.h"

class MyDropTarget: public CDropTarget
{
private:
    wl_WindowRef mWindow;
    wl_DropDataRef mDropData;
    void setEventFields(wl_Event &event, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
    void updateEffect(wl_Event &event, DWORD *pdwEffect);

	// save the parameters provided for drop feedback,
	//   to see if it's even a new event - don't know why win32 keeps sending duplicate events
	//   when nothing else has changed. perhaps a lazy way of checking for changing modifier keys?
	struct FeedbackState {
		DWORD grfKeyState;
		POINTL pt;
		DWORD initPdwEffect;
		bool operator==(const FeedbackState &other) {
			return (
				grfKeyState == other.grfKeyState &&
				pt.x == other.pt.x &&
				pt.y == other.pt.y &&
				initPdwEffect == other.initPdwEffect);
		}
		// not included in comparison:
		DWORD postPdwEffect;
	} lastDropState;

public:
    MyDropTarget(wl_WindowRef window);
    ~MyDropTarget();

    void commonQueryClient(wl_Event &event, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    HRESULT __stdcall DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) override;
    HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) override;
    HRESULT __stdcall DragLeave(void) override;
    HRESULT __stdcall Drop(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) override;
};

