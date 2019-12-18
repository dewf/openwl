#pragma once

//
//	ENUMFORMAT.CPP
//
//	By J Brown 2004 
//
//	www.catch22.net
//
//	Implementation of the IEnumFORMATETC interface
//
//	For Win2K and above look at the SHCreateStdEnumFmtEtc API call!!
//
//	Apparently the order of formats in an IEnumFORMATETC object must be
//  the same as those that were stored in the clipboard
//
//

//#define STRICT

#include <windows.h>

class CEnumFormatEtc : public IEnumFORMATETC
{
public:

	//
	// IUnknown members
	//
	HRESULT __stdcall  QueryInterface(REFIID iid, void ** ppvObject);
	ULONG	__stdcall  AddRef(void);
	ULONG	__stdcall  Release(void);

	//
	// IEnumFormatEtc members
	//
	HRESULT __stdcall  Next(ULONG celt, FORMATETC * rgelt, ULONG * pceltFetched);
	HRESULT __stdcall  Skip(ULONG celt);
	HRESULT __stdcall  Reset(void);
	HRESULT __stdcall  Clone(IEnumFORMATETC ** ppEnumFormatEtc);

	//
	// Construction / Destruction
	//
	CEnumFormatEtc(FORMATETC *pFormatEtc, int nNumFormats);
	~CEnumFormatEtc();

private:

	LONG		m_lRefCount;		// Reference count for this COM interface
	ULONG		m_nIndex;			// current enumerator index
	ULONG		m_nNumFormats;		// number of FORMATETC members
	FORMATETC * m_pFormatEtc;		// array of FORMATETC objects
};

//
//	"Drop-in" replacement for SHCreateStdEnumFmtEtc. Called by CDataObject::EnumFormatEtc
//
HRESULT CreateEnumFormatEtc(UINT nNumFormats, FORMATETC *pFormatEtc, IEnumFORMATETC **ppEnumFormatEtc);

//
//	Helper function to perform a "deep" copy of a FORMATETC
//
static void DeepCopyFormatEtc(FORMATETC *dest, FORMATETC *source);