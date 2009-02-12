//
// CoreUtils.cpp
//
//	Date:			"$Date: $"
//	Revision:		"$Revision: 1.0 $"
//  Last change by:	"$Author: rolandas $"
//	Created by:		Rolandas Rudomanskis
//
// Copyright © Shareaza Development Team, 2002-2009.
// This file is part of SHAREAZA (www.shareaza.com)
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
#include "Globals.h"

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
// ConvertToBSTR 
//
STDAPI_(BSTR) ConvertToBSTR(LPCSTR pszAnsiString, WORD wCodePage)
{
	BSTR bstr = NULL;
	UINT cblen, cbnew;
    LPWSTR pwsz;

	CHECK_NULL_RETURN(pszAnsiString, NULL);

	cblen = lstrlen(pszAnsiString);
	if ((cblen > 0) && (*pszAnsiString != '\0'))
	{
		cbnew = ((cblen + 1) * sizeof(WCHAR));
		pwsz = (LPWSTR)MemAlloc(cbnew);
		if (pwsz) 
		{
			if (SUCCEEDED(ConvertToUnicodeEx(pszAnsiString, cblen, pwsz, cbnew, wCodePage)))
				bstr = SysAllocString(pwsz);

			MemFree(pwsz);
		}
	}

	return bstr;
}

///////////////////////////////////////////////////////////////////////////////////
// CompareStrings
//
//  Calls CompareString API using Unicode version (if available on OS). Otherwise,
//  we have to thunk strings down to MBCS to compare. This is fairly inefficient for
//  Win9x systems that don't handle Unicode, but hey...this is only a sample.
//
STDAPI_(UINT) CompareStrings(LPCWSTR pwsz1, LPCWSTR pwsz2)
{
	UINT iret;
	LCID lcid = GetThreadLocale();
	UINT cblen1, cblen2;

    typedef INT (WINAPI *PFN_CMPSTRINGW)(LCID, DWORD, LPCWSTR, INT, LPCWSTR, INT);
    static PFN_CMPSTRINGW s_pfnCompareStringW = NULL;

 // Check that valid parameters are passed and then contain somethimg...
	if ((pwsz1 == NULL) || ((cblen1 = lstrlenW(pwsz1)) == 0))
		return CSTR_LESS_THAN;

	if ((pwsz2 == NULL) || ((cblen2 = lstrlenW(pwsz2)) == 0))
		return CSTR_GREATER_THAN;

 // If the string is of the same size, then we do quick compare to test for
 // equality (this is slightly faster than calling the API, but only if we
 // expect the calls to find an equal match)...
	if (cblen1 == cblen2)
	{
		for (iret = 0; iret < cblen1; iret++)
		{
			if (pwsz1[iret] == pwsz2[iret])
				continue;

			if (((pwsz1[iret] >= 'A') && (pwsz1[iret] <= 'Z')) &&
				((pwsz1[iret] + ('a' - 'A')) == pwsz2[iret]))
				continue;

			if (((pwsz2[iret] >= 'A') && (pwsz2[iret] <= 'Z')) &&
				((pwsz2[iret] + ('a' - 'A')) == pwsz1[iret]))
				continue;

			break; // don't continue if we can't quickly match...
		}

		// If we made it all the way, then they are equal...
		if (iret == cblen1)
			return CSTR_EQUAL;
	}

 // Now ask the OS to check the strings and give us its read. (We prefer checking
 // in Unicode since this is faster and we may have strings that can't be thunked
 // down to the local ANSI code page)...
	if (v_fRunningOnNT) 
    {
		iret = CompareStringW(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, pwsz1, cblen1, pwsz2, cblen2);
	}
	else
	{
	 // If we are on Win9x, we don't have much of choice (thunk the call)...
		LPSTR psz1 = ConvertToMBCS(pwsz1, CP_ACP);
		LPSTR psz2 = ConvertToMBCS(pwsz2, CP_ACP);
		iret = CompareString(lcid, NORM_IGNORECASE,	psz1, -1, psz2, -1);
		CoTaskMemFree(psz2);
		CoTaskMemFree(psz1);
	}

	return iret;
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

	if ( v_fRunningOnNT )  // Windows NT/2000/XP
	{
		LPWSTR lpwszFilePart = NULL;
		SEH_TRY
		dwRet = SearchPathW( NULL, pwszFile, NULL, MAX_PATH, pwszPath, &lpwszFilePart );
		SEH_EXCEPT_NULL
        if ( ( 0 == dwRet || dwRet > MAX_PATH ) ) return FALSE;
        if ( pcPathIdx ) *pcPathIdx = (ULONG)( ( (ULONG_PTR)lpwszFilePart - (ULONG_PTR)pwszPath ) / 2 );
	}
	else
	{
        CHAR szBuffer[MAX_PATH];
		LPSTR lpszFilePart = NULL;

		LPSTR szFile = ConvertToMBCS(pwszFile, CP_ACP);
		CHECK_NULL_RETURN(szFile, E_OUTOFMEMORY);

        szBuffer[0] = '\0';
		dwRet = SearchPathA( NULL, szFile, NULL, MAX_PATH, szBuffer, &lpszFilePart );
        if ( ( 0 == dwRet || dwRet > MAX_PATH ) ) return FALSE;

        if ( pcPathIdx ) *pcPathIdx = (ULONG)( (ULONG_PTR)lpszFilePart - (ULONG_PTR)&szBuffer );
        if ( FAILED(ConvertToUnicodeEx( szBuffer, lstrlen(szBuffer), pwszPath, MAX_PATH, (WORD)GetACP() )) )
            return FALSE;
	}

    return TRUE;
}

////////////////////////////////////////////////////////////////////////
// FGetModuleFileName
//
STDAPI_(BOOL) FGetModuleFileName(HMODULE hModule, WCHAR** wzFileName)
{
    LPWSTR pwsz, pwsz2;
    DWORD dw;

    CHECK_NULL_RETURN(wzFileName, FALSE);
    *wzFileName = NULL;

    pwsz = (LPWSTR)MemAlloc( MAX_PATH * 2 );
    CHECK_NULL_RETURN(pwsz, FALSE);

 // Call GetModuleFileNameW on Win NT/2000/XP/2003 systems...
	if (v_fRunningOnNT)
    {
		dw = GetModuleFileNameW( hModule, pwsz, MAX_PATH );
        if (dw == 0)
        {
            MemFree(pwsz);
            return FALSE;
        }
	}
	else
	{
	 // If we are on Win9x, we don't have much of choice (thunk the call)...
        dw = GetModuleFileName( hModule, (LPSTR)pwsz, MAX_PATH );
        if (dw == 0)
        {
            MemFree(pwsz);
            return FALSE;
        }

        pwsz2 = (LPWSTR)MemAlloc( MAX_PATH * 2 );
        if ( pwsz2 == 0 )
        {
            MemFree(pwsz);
            return FALSE;
        }

        if ( FAILED(ConvertToUnicodeEx( (LPSTR)pwsz, dw, pwsz2, MAX_PATH, CP_ACP )) )
        {
            MemFree(pwsz2); pwsz2 = NULL;
        }

        MemFree(pwsz);
        pwsz = pwsz2;
    }

    *wzFileName = pwsz;
    return TRUE;
}

