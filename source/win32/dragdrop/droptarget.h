#pragma once

//
//	DROPTARGET.H
//
//	By J Brown 2004 
//
//	www.catch22.net
//

//#define STRICT

#include <windows.h>


//
//	This is our definition of a class which implements
//  the IDropTarget interface
//
class CDropTarget : public IDropTarget
{
public:
	// IUnknown implementation
	HRESULT __stdcall QueryInterface(REFIID iid, void ** ppvObject);
	ULONG	__stdcall AddRef(void);
	ULONG	__stdcall Release(void);

	// IDropTarget implementation
	virtual HRESULT __stdcall DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) = 0;
	virtual HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) = 0;
	virtual HRESULT __stdcall DragLeave(void) = 0;
	virtual HRESULT __stdcall Drop(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) = 0;

	// Constructor
	CDropTarget(); //HWND hwnd
	~CDropTarget();

private:

	//// internal helper function
	//DWORD DropEffect(DWORD grfKeyState, POINTL pt, DWORD dwAllowed);
	//bool  QueryDataObject(IDataObject *pDataObject);

	//// Private member variables
	LONG	m_lRefCount;
	//HWND	m_hWnd;
	//bool    m_fAllowDrop;

	//IDataObject *m_pDataObject;
};

//void DropData(HWND hwnd, IDataObject *pDataObject);
//void RegisterDropWindow(HWND hwnd, IDropTarget **ppDropTarget);
//void UnregisterDropWindow(HWND hwnd, IDropTarget *pDropTarget);
