//
// stdafx.cpp : source file that includes just the standard includes
//
// Copyright (c) Nikolay Raspopov, 2014.
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

CString LoadString( UINT nID )
{
	CString str;
	str.LoadString( nID );
	return str;
}

CString GetURLs()
{
	CString sURLs;
	DWORD nType = 0, nSize = MAX_PATH * sizeof( TCHAR ) * 100;
	LPTSTR szData = sURLs.GetBuffer( nSize );
	if ( SHRegGetUSValue( LoadString( IDS_KEY ), _T( "URLs" ), &nType, szData, &nSize, FALSE, NULL, 0 ) != ERROR_SUCCESS || nType != REG_SZ )
		nSize = 0;
	szData[ nSize / sizeof( TCHAR ) ] = _T( '\0' );
	sURLs.ReleaseBuffer();
	sURLs.Trim();
	return ( sURLs.IsEmpty() ? LoadString( IDS_URL ) : sURLs );
}

BOOL SaveURLs( const CString& sURLs )
{
	return ( SHRegSetUSValue( LoadString( IDS_KEY ), _T( "URLs" ), REG_SZ, (LPCTSTR)sURLs, sURLs.GetLength() * sizeof( TCHAR ), SHREGSET_FORCE_HKCU ) == ERROR_SUCCESS );
}
