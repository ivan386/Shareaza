//
// dllmain.cpp : Implementation of DllMain.
//
// Copyright (c) Nikolay Raspopov, 2009.
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
#include "dllmain.h"

CPreviewModule _AtlModule;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	hInstance;
	return _AtlModule.DllMain(dwReason, lpReserved); 
}

void LoadData(CAtlMap< CString, CString >& oData)
{
	HUSKEY hKey = NULL;
	LSTATUS ret = SHRegOpenUSKey( REGISTRY_PATH, KEY_READ, NULL, &hKey, FALSE );
	if ( ERROR_SUCCESS == ret )
	{
		for ( DWORD i = 0; ; i++ )
		{
			TCHAR szExt[ MAX_PATH ] = {};
			DWORD nExtSize = _countof( szExt );
			DWORD nType = 0;
			TCHAR szCommand[ MAX_PATH ] = {};
			DWORD nCommandSize = _countof( szCommand );
			ret = SHRegEnumUSValue( hKey, i, szExt, &nExtSize, &nType,
				szCommand, &nCommandSize, SHREGENUM_HKCU );
			if ( ERROR_SUCCESS != ret )
				break;
			StrTrim( szExt, _T(" /t") );
			StrTrim( szCommand, _T(" /t") );
			if ( nType == REG_SZ && *szExt && *szCommand )
			{
				oData.SetAt( szExt, szCommand );
			}
		}

		SHRegCloseUSKey( hKey );
	}
}

void SaveData(const CAtlMap< CString, CString >& oData)
{
	// Get our CLSID
	LPWSTR szCLSID = NULL;
	StringFromCLSID( CLSID_PreviewPlugin, &szCLSID );
	const DWORD nCLSIDSize = lstrlenW( szCLSID ) * sizeof( WCHAR );

	// Find old settings in Shareaza
	CAtlList< CString > sExtList;
	HUSKEY hKey = NULL;
	LSTATUS ret = SHRegOpenUSKey( PLUGIN_PATH, KEY_READ, NULL, &hKey, FALSE );
	if ( ERROR_SUCCESS == ret )
	{
		for ( DWORD i = 0; ; i++ )
		{
			TCHAR szExt[ MAX_PATH ] = {};
			DWORD nExtSize = _countof( szExt );
			DWORD nType = 0;
			TCHAR szPlugin[ MAX_PATH ] = {};
			DWORD nPluginSize = _countof( szPlugin );
			ret = SHRegEnumUSValue( hKey, i, szExt, &nExtSize, &nType,
				szPlugin, &nPluginSize, SHREGENUM_HKCU );
			if ( ERROR_SUCCESS != ret )
				break;
			if ( nType == REG_SZ && lstrcmpi( szPlugin, szCLSID ) == 0 )
			{
				sExtList.AddTail( szExt );				
			}
		}
		SHRegCloseUSKey( hKey );
	}

	// Delete old plugin settings
	SHDeleteKey( HKEY_CURRENT_USER, REGISTRY_PATH );

	// Delete old plugin settings in Shareaza
	for ( POSITION pos = sExtList.GetHeadPosition(); pos; )
	{
		SHDeleteValue( HKEY_CURRENT_USER, PLUGIN_PATH, sExtList.GetNext( pos ) );
	}

	// Save to ...
	for ( POSITION pos = oData.GetStartPosition(); pos; )
	{
		CString sExt, sCommand;
		oData.GetNextAssoc( pos, sExt, sCommand );

		// ... plugin settings
		SHRegSetUSValue( REGISTRY_PATH, sExt, REG_SZ, (LPCTSTR)sCommand,
			sCommand.GetLength() * sizeof( TCHAR ), SHREGSET_FORCE_HKCU );

		// ... Shareaza settings
		TCHAR szKey[ MAX_PATH ] = { _T(".") };
		for ( LPCTSTR str = sExt.GetBuffer(); ; ++str )
		{
			LPTSTR space = StrChr( str, _T(' ') );
			if ( space )
				*space = _T('\0');
			lstrcpyn( szKey + 1, str, _countof( szKey ) - 1 );
			StrTrim( szKey + 1, _T(". /t") );

			if ( *szKey )
				SHRegSetUSValue( PLUGIN_PATH, szKey, REG_SZ, szCLSID,
					nCLSIDSize, SHREGSET_FORCE_HKCU );

			if ( ! space )
			{
				sExt.ReleaseBuffer();
				break;
			}
			str = space;
		}
	}

	CoTaskMemFree( szCLSID );
}
