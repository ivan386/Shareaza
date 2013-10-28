//
// Globals.h
//
//	Created by:		Rolandas Rudomanskis
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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
#include <windows.h>
#include <ole2.h>

//#pragma warning(disable: 4100) // unreferenced formal parameter (in OLE this is common)
//#pragma warning(disable: 4103) // pragma pack
//#pragma warning(disable: 4127) // constant expression
//#pragma warning(disable: 4146) // unary minus operator applied to unsigned type, result still unsigned
//#pragma warning(disable: 4201) // nameless unions are part of C++
//#pragma warning(disable: 4310) // cast truncates constant value
//#pragma warning(disable: 4505) // unreferenced local function has been removed
//#pragma warning(disable: 4710) // function couldn't be inlined
//#pragma warning(disable: 4786) // identifier was truncated in the debug information

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
// CRatDVDClassFactory - Class Factory
//
class CRatDVDClassFactory : public IClassFactory
{
public:
	CRatDVDClassFactory(): m_cRef(0){}
	~CRatDVDClassFactory(void){}

 // IUnknown Implementation
	STDMETHODIMP         QueryInterface(REFIID riid, void ** ppv);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

 // IClassFactory Implementation
	STDMETHODIMP  CreateInstance(LPUNKNOWN punk, REFIID riid, void** ppv);
	STDMETHODIMP  LockServer(BOOL fLock);

private:
	ULONG          m_cRef;          // Reference count
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

////////////////////////////////////////////////////////////////////////
// Unicode Win32 API wrappers (handles Unicode/ANSI convert for Win98/ME)
//
STDAPI_(BOOL) FFindQualifiedFileName(LPCWSTR pwszFile, LPWSTR pwszPath, ULONG *pcPathIdx);
STDAPI_(BOOL) FGetModuleFileName(HMODULE hModule, WCHAR** wzFileName);

////////////////////////////////////////////////////////////////////////
// Common macros -- Used to make code more readable.
//
#define SEH_TRY           __try {
#define SEH_EXCEPT(h)     } __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION){h = E_WIN32_ACCESSVIOLATION;}
#define SEH_EXCEPT_NULL   } __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION){}
#define SEH_START_FINALLY } __finally {
#define SEH_END_FINALLY   }
#define CHECK_NULL_RETURN(var, err)      if (NULL==(var)) return (err)

////////////////////////////////////////////////////////////////////////
// Debug macros (tracing and asserts)
//
#ifdef _DEBUG

#define ASSERT(x)  if(!(x)) DebugBreak()
#define ODS(x)	   OutputDebugString(x)

#define TRACE1(sz, arg1) { \
	CHAR ach[MAX_PATH]; \
	wsprintf(ach, (sz), (arg1)); \
	ODS(ach); }

#define TRACE2(sz, arg1, arg2) { \
	CHAR ach[MAX_PATH]; \
	wsprintf(ach, (sz), (arg1), (arg2)); \
	ODS(ach); }

#define TRACE3(sz, arg1, arg2, arg3) { \
	CHAR ach[MAX_PATH]; \
	wsprintf(ach, (sz), (arg1), (arg2), (arg3)); \
	ODS(ach); }

#else // !defined(_DEBUG)

#define ASSERT(x)
#define ODS(x)
#define TRACE1(sz, arg1)
#define TRACE2(sz, arg1, arg2)
#define TRACE3(sz, arg1, arg2, arg3)
#define TRACE_LPRECT(sz, lprc)

#endif // (_DEBUG)
