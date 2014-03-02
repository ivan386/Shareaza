//
// AntiVirus.cpp 
//
// Copyright (c) Shareaza Development Team, 2014.
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "AntiVirus.h"

#define AVVENDOR
#include <InitGuid.h>
#include <msoav.h>		

CAntiVirus AntiVirus;

void CAntiVirus::Enum(CComboBox& wndAntiVirus)
{
	if ( ! ::IsWindow( wndAntiVirus.GetSafeHwnd() ) )
		return;

	wndAntiVirus.ResetContent();

	// No anti-virus
	int nAntiVirus = wndAntiVirus.AddString( _T("") );
	wndAntiVirus.SetItemDataPtr( nAntiVirus, (LPVOID)new CString() );

	// Enum available anti-viruses
	CComPtr< ICatInformation > pInfo;
	HRESULT hr = pInfo.CoCreateInstance( CLSID_StdComponentCategoriesMgr );
	if ( SUCCEEDED( hr ) )
	{
		const CATID IDs[ 1 ] = { CATID_MSOfficeAntiVirus };
        CComPtr< IEnumCLSID > pEnum;
        hr = pInfo->EnumClassesOfCategories( 1, IDs, 0, NULL, &pEnum );
		if ( SUCCEEDED( hr ) )
		{
			CLSID clsid;
			while ( pEnum->Next( 1, &clsid, NULL ) == S_OK )
			{
				const CString sCLSID = Hashes::toGuid( clsid, true );
				HKEY hClass = NULL;
				if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_CLASSES_ROOT, _T("CLSID\\") + sCLSID, 0, KEY_READ, &hClass ) )
				{
					// Get it name
					TCHAR szValue[ MAX_PATH ] = {};
					DWORD nValue = MAX_PATH, nType = REG_SZ;
					if ( ERROR_SUCCESS == RegQueryValueEx( hClass, NULL, NULL, &nType, (LPBYTE)szValue, &nValue ) )
					{
						const int nIndex = wndAntiVirus.AddString( szValue );
						wndAntiVirus.SetItemDataPtr( nIndex, (LPVOID)new CString( sCLSID ) );
						if ( Settings.General.AntiVirus.CompareNoCase( sCLSID ) == 0 )
						{
							nAntiVirus = nIndex;
						}
					}
					RegCloseKey( hClass );
				}
			}
		}
	}

	wndAntiVirus.SetCurSel( nAntiVirus );
	wndAntiVirus.EnableWindow( wndAntiVirus.GetCount() > 1 );
}

void CAntiVirus::Free(CComboBox& wndAntiVirus)
{
	if ( ! ::IsWindow( wndAntiVirus.GetSafeHwnd() ) )
		return;

	const int nCount = wndAntiVirus.GetCount();
	for ( int i = 0; i < nCount; ++i )
	{
		delete (CString*)wndAntiVirus.GetItemDataPtr( i );
	}
}

void CAntiVirus::UpdateData(CComboBox& wndAntiVirus)
{
	if ( ! ::IsWindow( wndAntiVirus.GetSafeHwnd() ) )
		return;

	if ( wndAntiVirus.IsWindowEnabled() )
	{
		Settings.General.AntiVirus = *(CString*)wndAntiVirus.GetItemDataPtr( wndAntiVirus.GetCurSel() );
	}
}

bool CAntiVirus::Scan(LPCTSTR szPath)
{
	if ( Settings.General.AntiVirus.IsEmpty() )
		// No scan
		return true;

	CLSID clsid = {};
	if ( Hashes::fromGuid( Settings.General.AntiVirus, &clsid ) )
	{
		// This is CLSID of IOfficeAntiVirus anti-virus
		CComPtr< IOfficeAntiVirus > pAntivirus;
		HRESULT hr = ::CoCreateInstance( clsid, NULL, CLSCTX_ALL, IID_IOfficeAntiVirus, (void**)&pAntivirus );
		if ( SUCCEEDED( hr ) )
		{
			MSOAVINFO info = { sizeof( MSOAVINFO ) };
			info.fPath = true;
			info.hwnd = AfxGetMainWnd()->GetSafeHwnd();
			info.u.pwzFullPath = (LPTSTR)szPath;
			info.pwzHostName = CLIENT_NAME_T;
			hr = pAntivirus->Scan( &info );
			if ( hr == S_OK )
			{
				// OK
				theApp.Message( MSG_DEBUG, _T("Anti-virus check complete: %s"), (LPCTSTR)szPath );
			}
			else if ( hr == HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) )
			{
				// No file
				theApp.Message( MSG_WARNING, _T("Anti-virus failed to scan file: %s"), (LPCTSTR)szPath );
			}
			else
			{
				// Virus!
				theApp.Message( MSG_ERROR, _T("Anti-virus detected infected file: %s"), (LPCTSTR)szPath );
				return false;
			}
		}
		else
		{
			theApp.Message( MSG_ERROR, _T("Failed to initialize anti-virus scanner (error: 0x%08x)"), hr );
		}
	}

	return true;
}
