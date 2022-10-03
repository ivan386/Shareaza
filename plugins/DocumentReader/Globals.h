//
// Globals.h
//
//	Created by:		Rolandas Rudomanskis
//
// Copyright (c) Shareaza Development Team, 2002-2014.
// This file is part of SHAREAZA (shareaza.sourceforge.net)
//
// Shareaza is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Shareaza is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#pragma once

#pragma warning(disable: 4100) // unreferenced formal parameter (in OLE this is common)
#pragma warning(disable: 4103) // pragma pack
#pragma warning(disable: 4127) // constant expression
#pragma warning(disable: 4146) // unary minus operator applied to unsigned type, result still unsigned
#pragma warning(disable: 4201) // nameless unions are part of C++
#pragma warning(disable: 4310) // cast truncates constant value
#pragma warning(disable: 4505) // unreferenced local function has been removed
#pragma warning(disable: 4710) // function couldn't be inlined
#pragma warning(disable: 4786) // identifier was truncated in the debug information

////////////////////////////////////////////////////////////////////
// Module Globals and Accessors...
//
extern HINSTANCE           v_hModule;
extern ULONG               v_cLocks;
extern CRITICAL_SECTION    v_csSynch;
extern HANDLE              v_hPrivateHeap;

#define DllModuleHandle()  ((HMODULE)v_hModule)
#define DllAddRef()  InterlockedIncrement((LPLONG)&v_cLocks)
#define DllRelease() InterlockedDecrement((LPLONG)&v_cLocks)
#define EnterCritical() EnterCriticalSection(&v_csSynch)
#define LeaveCritical() LeaveCriticalSection(&v_csSynch)

////////////////////////////////////////////////////////////////////
// StgOpenStorageEx (for Windows 2000/XP NTFS Non-OLE Files)
//
typedef HRESULT (CALLBACK* PFN_STGOPENSTGEX)(WCHAR*, DWORD, DWORD, DWORD, void*, void*, REFIID riid, void**);
extern PFN_STGOPENSTGEX v_pfnStgOpenStorageEx;

////////////////////////////////////////////////////////////////////////
// Fixed Win32 Errors as HRESULTs
//
#define E_WIN32_ACCESSVIOLATION   0x800701E7   //HRESULT_FROM_WIN32(ERROR_INVALID_ADDRESS)
#define E_WIN32_BUFFERTOOSMALL    0x8007007A   //HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)

////////////////////////////////////////////////////////////////////
// Custom Errors
//
#define E_ERR_BASE              0x80041100
#define E_DOCUMENTNOTOPEN       0x80041101   // "You must open a document to perform the action requested."
#define E_DOCUMENTOPENED        0x80041102   // "You must close the current document before opening a new one in the same object."
#define E_DOCUMENTLOCKED        0x80041103   // "The document is in use by another program and cannot be opened for read-write access."
#define E_NODOCUMENTPROPS       0x80041104   // "The document is not an OLE file, and does not support extended document properties."
#define E_DOCUMENTREADONLY      0x80041105   // "The command is not available because document was opened in read-only mode."
#define E_MUSTHAVESTORAGE       0x80041106   // "The command is available for OLE Structured Storage files only."
#define E_INVALIDOBJECT         0x80041107   // "The object is not connected to the document (it was removed or the document was closed)."
#define E_INVALIDPROPSET        0x80041108   // "Cannot access property because the set it belongs to does not exist."
#define E_INVALIDINDEX          0x80041109   // "The property requested does not exist in the collection."
#define E_ITEMEXISTS            0x8004110A   // "An item by that already exists in the collection."
#define E_ERR_MAX               0x8004110B

////////////////////////////////////////////////////////////////////
// Core PropertySet Functions (Central Part for Read/Write of Properties)
//
STDAPI OpenPropertyStorage(IPropertySetStorage* pPropSS, REFFMTID fmtid, BOOL fReadOnly, DWORD dwFlags, IPropertyStorage** ppPropStg);
STDAPI ReadProperty(IPropertyStorage* pPropStg, PROPSPEC spc, WORD wCodePage, VARIANT* pvtResult);
STDAPI WriteProperty(IPropertyStorage* pPropStg, PROPSPEC spc, WORD wCodePage, VARIANT* pvtValue);
STDAPI LoadPropertySetList(IPropertyStorage *pPropStg, WORD *pwCodePage, class CDocProperty** pplist, BOOL bOnlyThumb);
STDAPI SavePropertySetList(IPropertyStorage *pPropStg, WORD wCodePage, class CDocProperty* plist, ULONG *pcSavedItems);

////////////////////////////////////////////////////////////////////
// CDocProperty - Basic object for single property.
// 

typedef enum dsoFilePropertyType
{
	dsoPropertyTypeUnknown = 0,
    dsoPropertyTypeString = 1,
    dsoPropertyTypeLong,
    dsoPropertyTypeDouble,
    dsoPropertyTypeBool,
    dsoPropertyTypeDate
} dsoFilePropertyType;

typedef enum dsoFileOpenOptions
{
	dsoOptionDefault = 0,
    dsoOptionOnlyOpenOLEFiles = 1,
    dsoOptionOpenReadOnlyIfNoWriteAccess = 2,
    dsoOptionDontAutoCreate = 4,
    dsoOptionUseMBCStringsForNewSets = 8
} dsoFileOpenOptions;

// Document thumbnail data
typedef struct _ClipMetaHeader
{
	DWORD	MappingMode;	/* Units used to playback metafile */
	WORD	Unknown;		/* Unknown data; always = 8; MM_ANISOTROPIC? */
	WORD	Width;			/* Width of the metafile */
	WORD	Height;			/* Height of the metafile */
	WORD	Handle;			/* Handle to the metafile in memory */
} CLIPMETAHEADER;

class CDocProperty
{
public:
    CDocProperty();
    ~CDocProperty(void);

 // CustomProperty Implementation
	HRESULT get_Name(BSTR *pbstrName);
	HRESULT get_Type(dsoFilePropertyType *dsoType);
	HRESULT get_Value(VARIANT *pvValue);
	HRESULT put_Value(VARIANT *pvValue);
	HRESULT Remove();

 // Internal Functions
	HRESULT InitProperty(BSTR bstrName, PROPID propid, VARIANT* pvData, BOOL fNewItem, CDocProperty* pPreviousItem);
    CDocProperty* GetNextProperty(){return m_pNextItem;}
    CDocProperty* AppendLink(CDocProperty* pLinkItem);
    PROPID GetID(){return m_ulPropID;}
	VARIANT* GetDataPtr(){return &m_vValue;}
    BOOL IsRemoved(){return m_fRemovedItem;}
    BOOL IsDirty(){return (m_fModified && !(m_fDeadObj));}
    BOOL IsNewItem(){return m_fNewItem;}
    void Renew(){m_fRemovedItem = FALSE;}
    void Disconnect(){m_fDeadObj = TRUE; /*Release();*/}
    void OnSaveComplete(){m_fModified = FALSE; m_fNewItem = FALSE; m_fRemovedItem = FALSE;}
    void OnRemoveComplete(){m_fModified = FALSE; m_fNewItem = TRUE; m_fRemovedItem = TRUE;}

	static CDocProperty* CreateObject(BSTR bstrName, PROPID propid, VARIANT* pvData, BOOL fNewItem, CDocProperty* pPreviousItem);

private:
	BSTR		 m_bstrName;            // Property Name
    PROPID       m_ulPropID;            // Property ID
	VARIANT		 m_vValue;              // Property Value
    BOOL         m_fModified;           // Do we need to update item?
    BOOL         m_fExternal;           // Does object have external ref count?
	BOOL         m_fDeadObj;            // Is object still valid?
    BOOL         m_fNewItem;            // Is item added to list after load?
    BOOL         m_fRemovedItem;        // Is item marked for delete?
    CDocProperty* m_pNextItem;			// Items are linked in single link list (stores previous item)
};

////////////////////////////////////////////////////////////////////////
// Heap Allocation (Uses private Win32 Heap)
//
STDAPI_(LPVOID) MemAlloc(DWORD cbSize);
STDAPI_(void)   MemFree(LPVOID ptr);

// Override new/delete to use our task allocator
// (removing CRT dependency will improve code performance and size)...
void * _cdecl operator new(size_t size);
void  _cdecl operator delete(void *ptr);

////////////////////////////////////////////////////////////////////////
// String Manipulation Functions
//
STDAPI ConvertToUnicodeEx(LPCSTR pszMbcsString, DWORD cbMbcsLen, LPWSTR pwszUnicode, DWORD cbUniLen, WORD wCodePage);
STDAPI ConvertToMBCSEx(LPCWSTR pwszUnicodeString, DWORD cbUniLen, LPSTR pszMbcsString, DWORD cbMbcsLen, WORD wCodePage);
STDAPI_(LPWSTR) ConvertToCoTaskMemStr(BSTR bstrString);
STDAPI_(LPSTR)  ConvertToMBCS(LPCWSTR pwszUnicodeString, WORD wCodePage);
STDAPI_(BSTR)   ConvertAToBSTR(LPCSTR pszAnsiString, WORD wCodePage);
STDAPI_(BSTR)   ConvertWToBSTR(LPCWSTR pszAnsiString, WORD wCodePage);
STDAPI_(UINT)   CompareStrings(LPCWSTR pwsz1, LPCWSTR pwsz2);

#ifdef _UNICODE
	#define ConvertToBSTR ConvertWToBSTR
#else
	#define ConvertToBSTR ConvertAToBSTR
#endif

////////////////////////////////////////////////////////////////////////
// Unicode Win32 API wrappers (handles Unicode/ANSI convert for Win98/ME)
//
STDAPI_(BOOL) FFindQualifiedFileName(LPCWSTR pwszFile, LPWSTR pwszPath, ULONG *pcPathIdx);
STDAPI_(BOOL) FGetModuleFileName(HMODULE hModule, WCHAR** wzFileName);
STDAPI_(BOOL) FGetIconForFile(LPCWSTR pwszFile, HICON *pico);

////////////////////////////////////////////////////////////////////////
// Property Identifiers for Office Document Properties...
//
#ifndef PID_DICTIONARY // User Defined Properties (Dictionary based)
#define PID_DICTIONARY			0x00000000L
#define PID_CODEPAGE			0x00000001L
#endif
#ifndef PIDSI_TITLE    // Summary Information Properties (All OLE Files)
#define PIDSI_TITLE             0x00000002L  // VT_LPSTR
#define PIDSI_SUBJECT           0x00000003L  // VT_LPSTR
#define PIDSI_AUTHOR            0x00000004L  // VT_LPSTR
#define PIDSI_KEYWORDS          0x00000005L  // VT_LPSTR
#define PIDSI_COMMENTS          0x00000006L  // VT_LPSTR
#define PIDSI_TEMPLATE          0x00000007L  // VT_LPSTR
#define PIDSI_LASTAUTHOR        0x00000008L  // VT_LPSTR
#define PIDSI_REVNUMBER         0x00000009L  // VT_LPSTR
#define PIDSI_EDITTIME          0x0000000aL  // VT_FILETIME (UTC)
#define PIDSI_LASTPRINTED       0x0000000bL  // VT_FILETIME (UTC)
#define PIDSI_CREATE_DTM        0x0000000cL  // VT_FILETIME (UTC)
#define PIDSI_LASTSAVE_DTM      0x0000000dL  // VT_FILETIME (UTC)
#define PIDSI_PAGECOUNT         0x0000000eL  // VT_I4
#define PIDSI_WORDCOUNT         0x0000000fL  // VT_I4
#define PIDSI_CHARCOUNT         0x00000010L  // VT_I4
#define PIDSI_THUMBNAIL         0x00000011L  // VT_CF
#define PIDSI_APPNAME           0x00000012L  // VT_LPSTR
#define PIDSI_DOC_SECURITY      0x00000013L  // VT_I4
#endif
#ifndef PID_CATEGORY   // Document Summary Information Properties (Office 97+ Files)
#define PID_CATEGORY            0x00000002L	 // VT_LPSTR
#define PID_PRESFORMAT          0x00000003L	 // VT_LPSTR
#define PID_BYTECOUNT           0x00000004L	 // VT_I4
#define PID_LINECOUNT           0x00000005L	 // VT_I4
#define PID_PARACOUNT           0x00000006L	 // VT_I4
#define PID_SLIDECOUNT          0x00000007L	 // VT_I4
#define PID_NOTECOUNT           0x00000008L	 // VT_I4
#define PID_HIDDENCOUNT         0x00000009L	 // VT_I4
#define PID_MMCLIPCOUNT         0x0000000aL	 // VT_I4
#define PID_SCALE               0x0000000bL	 // VT_BOOL
#define PID_HEADINGPAIR         0x0000000cL	 // VT_VECTOR | VT_VARIANT
#define PID_DOCPARTS            0x0000000dL	 // VT_VECTOR | VT_LPSTR
#define PID_MANAGER             0x0000000eL	 // VT_LPSTR
#define PID_COMPANY             0x0000000fL	 // VT_LPSTR
#define PID_LINKSDIRTY          0x00000010L	 // VT_BOOL
#define PID_CCHWITHSPACES 		0x00000011L	 // VT_I4
#define PID_GUID				0x00000012L  // VT_LPSTR -- RESERVED, no longer used
#define PID_SHAREDDOC			0x00000013L	 // VT_BOOL
#define PID_LINKBASE			0x00000014L  // VT_LPSTR -- RESERVED, no longer used
#define PID_HLINKS              0x00000015L	 // VT_VECTOR | VT_VARIANT -- RESERVED, no longer used
#define PID_HLINKSCHANGED       0x00000016L	 // VT_BOOL
#define PID_VERSION				0x00000017L	 // VT_I4
#define PID_DIGSIG              0x00000018L  // VT_BLOB
#endif

////////////////////////////////////////////////////////////////////////
// Common macros -- Used to make code more readable.
//
#define SEH_TRY           __try {
#define SEH_EXCEPT(h)     } __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION){h = E_WIN32_ACCESSVIOLATION;}
#define SEH_EXCEPT_NULL   } __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION){}
#define SEH_START_FINALLY } __finally {
#define SEH_END_FINALLY   }

#define CHECK_NULL_RETURN(var, err)      if (NULL==(var)) return (err)
#define CHECK_FLAG_RETURN(flg, err)      if (flg) return (err)
#define RETURN_ON_FAILURE(hr)            if (FAILED(hr)) { return hr; }
#define FREE_MEMPOINTER(p)               if (p) {MemFree(p); p = NULL;}
#define FREE_COTASKMEM(c)                if (c) {CoTaskMemFree(c); c = NULL;}
#define FREE_BSTR(b)                     if (b) {SysFreeString(b); b = NULL;}

#define ADDREF_INTERFACE(x)              if (x) (x)->AddRef();
#define RELEASE_INTERFACE(x)             if (x) {/*(x)->Release(); */(x) = NULL;}
#define SAFE_RELEASE_INTERFACE(x)        SEH_TRY RELEASE_INTERFACE(x) SEH_EXCEPT_NULL
#define ZOMBIE_OBJECT(x)                 if (x) {(x)->Disconnect(); (x) = NULL;}

////////////////////////////////////////////////////////////////////////
// Debug macros (tracing and asserts)
//
#ifdef _DEBUG

#define ASSERT(x)  if(!(x)) DebugBreak()
#define ODS(x)	   OutputDebugString(x)

#define TRACE1(sz, arg1) { \
	TCHAR ach[MAX_PATH]; \
	wsprintf(ach, _T(sz), (arg1)); \
	ODS(ach); }

#define TRACE2(sz, arg1, arg2) { \
	TCHAR ach[MAX_PATH]; \
	wsprintf(ach, _T(sz), (arg1), (arg2)); \
	ODS(ach); }

#define TRACE3(sz, arg1, arg2, arg3) { \
	TCHAR ach[MAX_PATH]; \
	wsprintf(ach, _T(sz), (arg1), (arg2), (arg3)); \
	ODS(ach); }

#else // !defined(_DEBUG)

#define ASSERT(x)
#define ODS(x)
#define TRACE1(sz, arg1)
#define TRACE2(sz, arg1, arg2)
#define TRACE3(sz, arg1, arg2, arg3)
#define TRACE_LPRECT(sz, lprc)

#endif // (_DEBUG)