//
// RegExp.cpp
//
// Copyright (c) Shareaza Development Team, 2010.
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
#include "RegExp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" BOOL WINAPI DllMain(HINSTANCE /*hInstance*/, DWORD /*dwReason*/, LPVOID /*lpReserved*/)
{
    return TRUE; 
}

namespace RegExp
{

REGEXP_API BOOL Match(LPCTSTR szRegExp, LPCTSTR szContent)
{
	try
	{
		const wstring sRegExp( (LPCWSTR)CT2CW( szRegExp ) );
		const wregex regExpPattern( sRegExp,
			regex_constants::ECMAScript | regex_constants::icase );
		const wstring sContent( (LPCWSTR)CT2CW( szContent ) );
		if ( regex_search( sContent, regExpPattern ) )
			return TRUE;
	}
	catch (...)
	{
	}
	return FALSE;
}

REGEXP_API size_t Split(LPCTSTR szRegExp, LPCTSTR szContent, LPTSTR* pszResult)
{
	try
	{
		const wstring sRegExp( (LPCWSTR)CT2CW( szRegExp ) );
		const wregex regExpPattern( sRegExp,
			regex_constants::ECMAScript | regex_constants::icase );
		const wstring sContent( (LPCWSTR)CT2CW( szContent ) );
		wsmatch results;
		if ( regex_search( sContent, results, regExpPattern ) )
		{
			const size_t nCount = results.size();
			size_t len = 0;
			for ( size_t i = 0; i < nCount; ++i )
			{
				len += results.str( i ).size() + 1;
			}
			LPTSTR p = (LPTSTR)GlobalAlloc( GPTR, len * sizeof( wchar_t ) );

			*pszResult = p;
			for ( size_t i = 0; i < nCount; ++i )
			{
				wcscpy_s( p, len - ( p - *pszResult ), results.str( i ).c_str() );
				p += results.str( i ).size() + 1;
			}
			return nCount;
		}
	}
	catch (...)
	{
	}
	*pszResult = NULL;
	return 0;
}

};
