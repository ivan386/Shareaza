//
// Registry.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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


#include "StdAfx.h"
#include "Registry.h"


//////////////////////////////////////////////////////////////////////
// CRegistry construction

CRegistry::CRegistry()
{
}

CRegistry::~CRegistry()
{
}

//////////////////////////////////////////////////////////////////////
// CRegistry read a string value

CString CRegistry::GetString(LPCTSTR pszSection, LPCTSTR pszName, LPCTSTR pszDefault)
{
	CString strKey, strValue;
	DWORD nErrorCode;
	HKEY hKey;
	
	if ( pszDefault != NULL ) strValue = pszDefault;
	strKey.Format( _T("Software\\Shareaza\\Shareaza\\%s"), pszSection );
	
	nErrorCode = RegOpenKeyEx( HKEY_CURRENT_USER, strKey, 0, KEY_READ, &hKey );
	
	if ( nErrorCode == ERROR_SUCCESS )
	{
		DWORD nType = 0, nSize = 0;
		
		nErrorCode = RegQueryValueEx( hKey, pszName, 0, &nType, NULL, &nSize ); 
		
		if ( nErrorCode == ERROR_MORE_DATA && nType == REG_SZ && nSize >= sizeof(TCHAR) )
		{
			LPTSTR pszValue = strValue.GetBuffer( nSize / sizeof(TCHAR) - 1 );
			nErrorCode = RegQueryValueEx( hKey, pszName, 0, &nType, (PBYTE)pszValue, &nSize ); 
			strValue.ReleaseBuffer( nSize / sizeof(TCHAR) - 1 );
		}
		
		RegCloseKey( hKey );
	}
	
	if ( nErrorCode != ERROR_SUCCESS ) DisplayErrorMessageBox( nErrorCode );
	
	return strValue;
}

//////////////////////////////////////////////////////////////////////
// CRegistry read an integer value

int CRegistry::GetInt(LPCTSTR pszSection, LPCTSTR pszName, int nDefault)
{
	int nValue = nDefault;
	DWORD nErrorCode;
	CString strKey;
	HKEY hKey;
	
	strKey.Format( _T("Software\\Shareaza\\Shareaza\\%s"), pszSection );
	
	nErrorCode = RegOpenKeyEx( HKEY_CURRENT_USER, strKey, 0, KEY_READ, &hKey );
	
	if ( nErrorCode == ERROR_SUCCESS )
	{
		DWORD nType = 0, nSize = sizeof(nValue);
		
		nErrorCode = RegQueryValueEx( hKey, pszName, 0, &nType, (PBYTE)&nValue, &nSize ); 
		
		if ( nType != REG_DWORD || nSize != sizeof(nValue) )
		{
			nErrorCode = ERROR_MORE_DATA;
			nValue = nDefault;
		}
		
		RegCloseKey( hKey );
	}
	
	if ( nErrorCode != ERROR_SUCCESS ) DisplayErrorMessageBox( nErrorCode );
	
	return nValue;
}

DWORD CRegistry::GetDword(LPCTSTR pszSection, LPCTSTR pszName, DWORD dwDefault)
{
	return (int)GetInt( pszSection, pszName, (int)dwDefault );
}

//////////////////////////////////////////////////////////////////////
// CRegistry read a float value

double CRegistry::GetFloat(LPCTSTR pszSection, LPCTSTR pszName, double fDefault)
{
	double fValue = fDefault;
	_stscanf( GetString( pszSection, pszName ), _T("%lf"), &fValue );
	return fValue;
}

//////////////////////////////////////////////////////////////////////
// CRegistry write a string value

BOOL CRegistry::SetString(LPCTSTR pszSection, LPCTSTR pszName, LPCTSTR pszValue)
{
	DWORD nErrorCode;
	CString strKey;
	HKEY hKey;
	
	strKey.Format( _T("Software\\Shareaza\\Shareaza\\%s"), pszSection );
	
	nErrorCode = RegCreateKeyEx( HKEY_CURRENT_USER, strKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL );
	
	if ( nErrorCode == ERROR_SUCCESS )
	{
		nErrorCode = RegSetValueEx( hKey, pszName, 0, REG_SZ, (const BYTE *)pszValue,
								_tcslen(pszValue) * sizeof(TCHAR) + sizeof(TCHAR) );
		
		RegCloseKey( hKey );
	}
	
	if ( nErrorCode == ERROR_SUCCESS )
	{
		return TRUE;
	}
	else
	{
		DisplayErrorMessageBox( nErrorCode );
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CRegistry write an int value

BOOL CRegistry::SetInt(LPCTSTR pszSection, LPCTSTR pszName, int nValue)
{
	DWORD nErrorCode;
	CString strKey;
	HKEY hKey;
	
	strKey.Format( _T("Software\\Shareaza\\Shareaza\\%s"), pszSection );
	
	nErrorCode = RegCreateKeyEx( HKEY_CURRENT_USER, strKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL );
	
	if ( nErrorCode == ERROR_SUCCESS )
	{
		nErrorCode = RegSetValueEx( hKey, pszName, 0, REG_DWORD,
							(const BYTE *)&nValue, sizeof(nValue) );
		
		RegCloseKey( hKey );
	}
	
	if ( nErrorCode == ERROR_SUCCESS )
	{
		return TRUE;
	}
	else
	{
		DisplayErrorMessageBox( nErrorCode );
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// Helper function to display a message box holding an error code

void CRegistry::DisplayErrorMessageBox(DWORD nErrorCode)
{
#ifdef _DEBUG
	LPVOID lpMsgBuf;
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, nErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,	0, NULL );
	MessageBox( NULL, (LPCTSTR)lpMsgBuf, _T("Error"), MB_OK | MB_ICONINFORMATION );
	LocalFree( lpMsgBuf );
#endif
}
