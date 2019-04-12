#include <Windows.h>
#include <Ole2.h>

HRESULT CreateDropSource(IDropSource **ppDropSource);
HRESULT CreateDataObject(FORMATETC *fmtetc, STGMEDIUM *stgmeds, UINT count, IDataObject **ppDataObject);

void dragDropInit()
{
	OleInitialize(nullptr);
}

void dragDropShutdown()
{
	OleUninitialize();
}

void dragDropRegisterTarget(HWND hwnd)
{
	// RegisterDragDrop(hwnd, target)
}

void dragDropUnregisterTarget(HWND hwnd)
{
	RevokeDragDrop(hwnd);
}
