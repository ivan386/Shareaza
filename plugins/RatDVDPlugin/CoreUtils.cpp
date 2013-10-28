//
// CoreUtils.cpp
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

#include "stdafx.h"

////////////////////////////////////////////////////////////////////////
// Heap Allocation (Uses CoTaskMemAlloc)
//
STDAPI_(LPVOID) MemAlloc(DWORD cbSize)
{
	CHECK_NULL_RETURN(v_hPrivateHeap, NULL);
	return HeapAlloc(v_hPrivateHeap, 0, cbSize);
}

STDAPI_(void) MemFree(LPVOID ptr)
{
	if ((v_hPrivateHeap) && (ptr))
		HeapFree(v_hPrivateHeap, 0, ptr);
}

////////////////////////////////////////////////////////////////////////
// String Manipulation Functions
//
////////////////////////////////////////////////////////////////////////
// ConvertToUnicodeEx
//
STDAPI ConvertToUnicodeEx(LPCSTR pszMbcsString, DWORD cbMbcsLen, LPWSTR pwszUnicode, DWORD cbUniLen, WORD wCodePage)
{
	DWORD cbRet;
	UINT iCode = CP_ACP;

	if (IsValidCodePage((UINT)wCodePage))
		iCode = (UINT)wCodePage;

	CHECK_NULL_RETURN(pwszUnicode,    E_POINTER);
	pwszUnicode[0] = L'\0';

	CHECK_NULL_RETURN(pszMbcsString,  E_POINTER);
	CHECK_NULL_RETURN(cbMbcsLen,      E_INVALIDARG);
	CHECK_NULL_RETURN(cbUniLen,       E_INVALIDARG);

	cbRet = MultiByteToWideChar(iCode, 0, pszMbcsString, cbMbcsLen, pwszUnicode, cbUniLen);
	if (cbRet == 0)	return HRESULT_FROM_WIN32(GetLastError());

	pwszUnicode[cbRet] = L'\0';
	return S_OK;
}

////////////////////////////////////////////////////////////////////////
// ConvertToMBCSEx
//
STDAPI ConvertToMBCSEx(LPCWSTR pwszUnicodeString, DWORD cbUniLen, LPSTR pszMbcsString, DWORD cbMbcsLen, WORD wCodePage)
{
	DWORD cbRet;
	UINT iCode = CP_ACP;

	if (IsValidCodePage((UINT)wCodePage))
		iCode = (UINT)wCodePage;

	CHECK_NULL_RETURN(pszMbcsString,     E_POINTER);
	pszMbcsString[0] = L'\0';

	CHECK_NULL_RETURN(pwszUnicodeString, E_POINTER);
	CHECK_NULL_RETURN(cbMbcsLen,         E_INVALIDARG);
	CHECK_NULL_RETURN(cbUniLen,          E_INVALIDARG);

	cbRet = WideCharToMultiByte(iCode, 0, pwszUnicodeString, -1, pszMbcsString, cbMbcsLen, NULL, NULL);
	if (cbRet == 0)	return HRESULT_FROM_WIN32(GetLastError());

	pszMbcsString[cbRet] = '\0';
	return S_OK;
}

////////////////////////////////////////////////////////////////////////
// ConvertToCoTaskMemStr
//
STDAPI_(LPWSTR) ConvertToCoTaskMemStr(BSTR bstrString)
{
	LPWSTR pwsz;
	ULONG cbLen;

	CHECK_NULL_RETURN(bstrString, NULL);

	cbLen = SysStringLen(bstrString);
	pwsz = (LPWSTR)CoTaskMemAlloc((cbLen * 2) + sizeof(WCHAR));
	if (pwsz)
	{
		memcpy(pwsz, bstrString, (cbLen * 2));
		pwsz[cbLen] = L'\0'; // Make sure it is NULL terminated.
	}

	return pwsz;
}

////////////////////////////////////////////////////////////////////////
// ConvertToMBCS -- NOTE: This returns CoTaskMemAlloc string!
//
STDAPI_(LPSTR) ConvertToMBCS(LPCWSTR pwszUnicodeString, WORD wCodePage)
{
	LPSTR psz = NULL;
	UINT cblen, cbnew;

	CHECK_NULL_RETURN(pwszUnicodeString, NULL);

	cblen = lstrlenW(pwszUnicodeString);
	cbnew = ((cblen + 1) * sizeof(WCHAR));
	psz = (LPSTR)CoTaskMemAlloc(cbnew);
	if ((psz) && FAILED(ConvertToMBCSEx(pwszUnicodeString, cblen, psz, cbnew, wCodePage)))
	{
		CoTaskMemFree(psz);
		psz = NULL;
	}

	return psz;
}

////////////////////////////////////////////////////////////////////////
// Unicode Win32 API wrappers (handles Unicode/ANSI convert for Win98/ME)
//
////////////////////////////////////////////////////////////////////////
// FFindQualifiedFileName
//
STDAPI_(BOOL) FFindQualifiedFileName(LPCWSTR pwszFile, LPWSTR pwszPath, ULONG *pcPathIdx)
{
	DWORD dwRet = 0;

	LPWSTR lpwszFilePart = NULL;
	SEH_TRY
	dwRet = SearchPathW( NULL, pwszFile, NULL, MAX_PATH, pwszPath, &lpwszFilePart );
	SEH_EXCEPT_NULL
	if ( ( 0 == dwRet || dwRet > MAX_PATH ) ) return FALSE;
	if ( pcPathIdx ) *pcPathIdx = (ULONG)( ( (ULONG_PTR)lpwszFilePart - (ULONG_PTR)pwszPath ) / 2 );

	return TRUE;
}

////////////////////////////////////////////////////////////////////////
// FGetModuleFileName
//
STDAPI_(BOOL) FGetModuleFileName(HMODULE hModule, WCHAR** wzFileName)
{
	CHECK_NULL_RETURN(wzFileName, FALSE);
	*wzFileName = NULL;

	LPWSTR pwsz = (LPWSTR)MemAlloc( MAX_PATH * 2 );
	CHECK_NULL_RETURN(pwsz, FALSE);

	DWORD dw = GetModuleFileNameW( hModule, pwsz, MAX_PATH );
	if (dw == 0)
	{
		MemFree(pwsz);
		return FALSE;
	}

	*wzFileName = pwsz;
	return TRUE;
}
