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
//#include "Shareaza.h"
//#include "Settings.h"
//#include "Schema.h"
//#include "Skin.h"
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
// Registry Helper functions

//Helper function to display a message box holding an error code
void CRegistry::DisplayErrorMessageBox(DWORD dwErrorCode)
{
	LPVOID lpMsgBuf;
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
	FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
	NULL, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
	(LPTSTR) &lpMsgBuf,	0, NULL );
	// Display the string.
	MessageBox( NULL, (LPCTSTR)lpMsgBuf, _T("Error"), MB_OK | MB_ICONINFORMATION );
	// Free the buffer.
	LocalFree( lpMsgBuf );
}

CString CRegistry::GetString(LPCTSTR pLocation, LPCTSTR pKeyName, LPCTSTR pDefault)
{
	char  sTemp[2048]; //Buffer to store value
	CString sReturnValue;
	HKEY  dmKey;
	DWORD  dwErrorCode; 

	DWORD  dwType;
	DWORD  len=2048; 

	sReturnValue.Format( _T("%s"), pDefault );
		
	dwErrorCode = RegOpenKeyEx( HKEY_CURRENT_USER,  pLocation,   0,   KEY_ALL_ACCESS,   &dmKey   ); 

	if ( dwErrorCode != ERROR_SUCCESS ) 
	{
		//DisplayErrorMessageBox(dwErrorCode);
		return( sReturnValue );
	} 

	dwErrorCode = RegQueryValueEx( dmKey, pKeyName , 0, &dwType, (LPBYTE)sTemp, &len ); 

	RegCloseKey( dmKey );

	if ( dwErrorCode != ERROR_SUCCESS ) 
	{
		//DisplayErrorMessageBox(dwErrorCode);
		return( sReturnValue ); 
	} 
		
	if( dwType == REG_SZ )
		sReturnValue.Format( _T("%s"), sTemp );
	//else AfxMessageBox( _T("Type error reading from registry") , MB_ICONQUESTION );

	return( sReturnValue );
}


int CRegistry::GetInt(LPCTSTR pLocation, LPCTSTR pKeyName, int iDefault)
{
	int iReturnValue = iDefault;
	HKEY  dmKey;
	DWORD dwErrorCode; 

	DWORD dwType;
	DWORD len = sizeof( int ); 
		
	dwErrorCode = RegOpenKeyEx( HKEY_CURRENT_USER,  pLocation,   0,   KEY_ALL_ACCESS,   &dmKey   ); 

	if ( dwErrorCode != ERROR_SUCCESS ) 
	{
		//DisplayErrorMessageBox(dwErrorCode);
		return( iDefault );
	} 

	dwErrorCode = RegQueryValueEx( dmKey, pKeyName , 0, &dwType, (LPBYTE)&iReturnValue, &len ); 

	RegCloseKey( dmKey );

	if ( dwErrorCode != ERROR_SUCCESS ) 
	{
		//DisplayErrorMessageBox(dwErrorCode);
		return( iDefault ); 
	} 
		
	if( dwType != REG_DWORD && dwType != REG_DWORD_BIG_ENDIAN )
	{
		//AfxMessageBox( _T("Type error reading from registry") , MB_ICONQUESTION );
		return( iDefault );
	}

	return( iReturnValue );
}

DWORD CRegistry::GetDword(LPCTSTR pLocation, LPCTSTR pKeyName, DWORD dwDefault)
{
	DWORD dwReturnValue = dwDefault;
	HKEY  dmKey;
	DWORD dwErrorCode; 

	DWORD dwType;
	DWORD len = sizeof( DWORD ); 
		
	dwErrorCode = RegOpenKeyEx( HKEY_CURRENT_USER,  pLocation,   0,   KEY_ALL_ACCESS,   &dmKey   ); 

	if ( dwErrorCode != ERROR_SUCCESS ) 
	{
		//DisplayErrorMessageBox(dwErrorCode);
		return( dwDefault );
	} 

	dwErrorCode = RegQueryValueEx( dmKey, pKeyName , 0, &dwType, (LPBYTE)&dwReturnValue, &len ); 

	RegCloseKey( dmKey );

	if ( dwErrorCode != ERROR_SUCCESS ) 
	{
		//DisplayErrorMessageBox(dwErrorCode);
		return( dwDefault ); 
	} 
		
	if( dwType != REG_DWORD && dwType != REG_DWORD_BIG_ENDIAN )
	{
		//AfxMessageBox( _T("Type error reading from registry") , MB_ICONQUESTION );
		return( dwDefault );
	}

	return( dwReturnValue );
}

double CRegistry::GetFloat(LPCTSTR pLocation, LPCTSTR pKeyName, double fDefault)
{
	double fReturnValue;

	CString sTemp = GetString( pLocation, pKeyName, _T("") );
	if ( sTemp.GetLength() ) 
	{
		_stscanf( sTemp, _T("%lf"), &fReturnValue );
		return( fReturnValue );
	}
	return( fDefault );
}


BOOL CRegistry::SetInt(LPCTSTR pLocation, LPCTSTR pKeyName, int iValue)
{
	HKEY  dmKey;
	DWORD dwDisposition, dwErrorCode; 

	dwErrorCode = RegCreateKeyEx( HKEY_CURRENT_USER,  pLocation, 0,  NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &dmKey, &dwDisposition ); 

	if ( dwErrorCode != ERROR_SUCCESS ) 
	{
		//DisplayErrorMessageBox(dwErrorCode);
		return( FALSE );
	} 

	dwErrorCode = RegSetValueEx( dmKey, pKeyName , 0, REG_DWORD, (LPBYTE)&iValue, sizeof(DWORD) ); 

	RegCloseKey( dmKey );

	if ( dwErrorCode != ERROR_SUCCESS ) 
	{
		//DisplayErrorMessageBox(dwErrorCode);
		return( FALSE ); 
	} 
		
	return( TRUE );
}


BOOL CRegistry::SetString(LPCTSTR pLocation, LPCTSTR pKeyName, LPCTSTR sValue)
{
	HKEY  dmKey;
	DWORD dwDisposition, dwErrorCode; 

	dwErrorCode = RegCreateKeyEx( HKEY_CURRENT_USER,  pLocation, 0,  NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &dmKey, &dwDisposition ); 

	if ( dwErrorCode != ERROR_SUCCESS ) 
	{
		//DisplayErrorMessageBox(dwErrorCode);
		return( FALSE );
	} 

	dwErrorCode = RegSetValueEx( dmKey, pKeyName , 0, REG_SZ, (LPBYTE)sValue, (_tcslen(sValue) + 1 ) * sizeof(TCHAR) ); 

	RegCloseKey( dmKey );

	if ( dwErrorCode != ERROR_SUCCESS ) 
	{
		//DisplayErrorMessageBox(dwErrorCode);
		return( FALSE ); 
	} 
		
	return( TRUE );
}