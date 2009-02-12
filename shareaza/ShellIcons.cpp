//
// ShellIcons.cpp
//
// Copyright © Shareaza Development Team, 2002-2009.
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
#include "ShellIcons.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CShellIcons ShellIcons;

//////////////////////////////////////////////////////////////////////
// CShellIcons construction

CShellIcons::CShellIcons()
{
	// experimental values
	m_m16.InitHashTable( 31 );
	m_m32.InitHashTable( 31 );
	m_m48.InitHashTable( 31 );
}

CShellIcons::~CShellIcons()
{
}

//////////////////////////////////////////////////////////////////////
// CShellIcons clear

void CShellIcons::Clear()
{
	m_m16.RemoveAll();
	m_m32.RemoveAll();
	m_m48.RemoveAll();

	if ( m_i16.m_hImageList ) m_i16.DeleteImageList();
	if ( m_i32.m_hImageList ) m_i32.DeleteImageList();
	if ( m_i48.m_hImageList ) m_i48.DeleteImageList();

	m_i16.Create( 16, 16, ILC_COLOR32|ILC_MASK, 0, 16 );
	m_i32.Create( 32, 32, ILC_COLOR32|ILC_MASK, 0, 16 );
	m_i48.Create( 48, 48, ILC_COLOR32|ILC_MASK, 0, 16 );

	// SHI_FILE = 0
	VERIFY( AddIcon( CoolInterface.ExtractIcon( IDI_FILE, FALSE, LVSIL_SMALL) , m_i16 ) == SHI_FILE );
	VERIFY( AddIcon( CoolInterface.ExtractIcon( IDI_FILE, FALSE, LVSIL_NORMAL), m_i32 ) == SHI_FILE );
	VERIFY( AddIcon( CoolInterface.ExtractIcon( IDI_FILE, FALSE, LVSIL_BIG), m_i48 ) == SHI_FILE );

	// SHI_EXECUTABLE = 1
	VERIFY( AddIcon( CoolInterface.ExtractIcon( IDI_EXECUTABLE, FALSE, LVSIL_SMALL), m_i16 ) == SHI_EXECUTABLE );
	VERIFY( AddIcon( CoolInterface.ExtractIcon( IDI_EXECUTABLE, FALSE, LVSIL_NORMAL), m_i32 ) == SHI_EXECUTABLE );
	VERIFY( AddIcon( CoolInterface.ExtractIcon( IDI_EXECUTABLE, FALSE, LVSIL_BIG), m_i48 ) == SHI_EXECUTABLE );

	// SHI_COMPUTER = 2
	VERIFY( AddIcon( CoolInterface.ExtractIcon( IDI_COMPUTER, FALSE, LVSIL_SMALL), m_i16 ) == SHI_COMPUTER );
	VERIFY( AddIcon( CoolInterface.ExtractIcon( IDI_COMPUTER, FALSE, LVSIL_NORMAL), m_i32 ) == SHI_COMPUTER );
	VERIFY( AddIcon( CoolInterface.ExtractIcon( IDI_COMPUTER, FALSE, LVSIL_BIG), m_i48 ) == SHI_COMPUTER );

	// SHI_FOLDER_CLOSED = 3
	VERIFY( AddIcon( CoolInterface.ExtractIcon( IDI_FOLDER_CLOSED, FALSE, LVSIL_SMALL), m_i16 ) == SHI_FOLDER_CLOSED );
	VERIFY( AddIcon( CoolInterface.ExtractIcon( IDI_FOLDER_CLOSED, FALSE, LVSIL_NORMAL), m_i32 ) == SHI_FOLDER_CLOSED );
	VERIFY( AddIcon( CoolInterface.ExtractIcon( IDI_FOLDER_CLOSED, FALSE, LVSIL_BIG), m_i48 ) == SHI_FOLDER_CLOSED );

	// SHI_FOLDER_OPEN = 4
	VERIFY( AddIcon( CoolInterface.ExtractIcon( IDI_FOLDER_OPEN, FALSE, LVSIL_SMALL), m_i16 ) == SHI_FOLDER_OPEN );
	VERIFY( AddIcon( CoolInterface.ExtractIcon( IDI_FOLDER_OPEN, FALSE, LVSIL_NORMAL), m_i32 ) == SHI_FOLDER_OPEN );
	VERIFY( AddIcon( CoolInterface.ExtractIcon( IDI_FOLDER_OPEN, FALSE, LVSIL_BIG), m_i48 ) == SHI_FOLDER_OPEN );

	// SHI_LOCKED = 5 (overlay)
	VERIFY( AddIcon( CoolInterface.ExtractIcon( IDI_LOCKED, FALSE, LVSIL_SMALL), m_i16 ) == SHI_LOCKED );
//	VERIFY( AddIcon( CoolInterface.ExtractIcon( IDI_LOCKED, FALSE, LVSIL_NORMAL), m_i32 ) == SHI_LOCKED );
//	VERIFY( AddIcon( CoolInterface.ExtractIcon( IDI_LOCKED, FALSE, LVSIL_BIG), m_i48 ) == SHI_LOCKED );
	m_i16.SetOverlayImage( SHI_LOCKED, SHI_O_LOCKED );
//	m_i32.SetOverlayImage( SHI_LOCKED, SHI_O_LOCKED );
//	m_i48.SetOverlayImage( SHI_LOCKED, SHI_O_LOCKED );

	m_m16.SetAt(_T(".exe"), SHI_EXECUTABLE);
	m_m32.SetAt(_T(".exe"), SHI_EXECUTABLE);
	m_m48.SetAt(_T(".exe"), SHI_EXECUTABLE);

	m_m16.SetAt(_T(".com"), SHI_EXECUTABLE);
	m_m32.SetAt(_T(".com"), SHI_EXECUTABLE);
	m_m48.SetAt(_T(".com"), SHI_EXECUTABLE);
}

//////////////////////////////////////////////////////////////////////
// CShellIcons get

int CShellIcons::Get(LPCTSTR pszFile, int nSize)
{
	LPCTSTR pszType = _tcsrchr( pszFile, '.' );
	if ( pszType == NULL )
		return SHI_FILE;
	
	if ( m_i16.m_hImageList == NULL ) Clear();

	CString strType( pszType );
	ToLower( strType );

	HICON hIcon = NULL;
	int nIndex = SHI_FILE;

	SHFILEINFO sfi = { 0 };
	switch ( nSize )
	{
	case 16:
		if ( m_m16.Lookup( strType, nIndex ) ) return nIndex;
		Lookup( pszType, &hIcon, NULL, NULL, NULL );
		if ( ! hIcon && SHGetFileInfo( pszFile, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof( SHFILEINFO ),
			SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | SHGFI_SMALLICON ) )
		{
			hIcon = Settings.General.LanguageRTL ? CreateMirroredIcon( sfi.hIcon ) : sfi.hIcon;
		}
		nIndex = hIcon ? m_i16.Add( hIcon ) : 0;
		m_m16.SetAt( strType, nIndex );
		break;
	case 32:
		if ( m_m32.Lookup( strType, nIndex ) ) return nIndex;
		Lookup( pszType, NULL, &hIcon, NULL, NULL );
		if ( ! hIcon && SHGetFileInfo( pszFile, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof( SHFILEINFO ),
			SHGFI_USEFILEATTRIBUTES | SHGFI_ICON ) )
		{
			hIcon = Settings.General.LanguageRTL ? CreateMirroredIcon( sfi.hIcon ) : sfi.hIcon;
		}
		nIndex = hIcon ? m_i32.Add( hIcon ) : 0;
		m_m32.SetAt( strType, nIndex );
		break;
	case 48:
		if ( m_m48.Lookup( strType, nIndex ) ) return nIndex;
		Lookup( pszType, NULL, NULL, NULL, NULL, &hIcon );
		if ( ! hIcon && SHGetFileInfo( pszFile, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof( SHFILEINFO ),
			SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | SHGFI_LARGEICON ) )
		{
			hIcon = Settings.General.LanguageRTL ? CreateMirroredIcon( sfi.hIcon ) : sfi.hIcon;
		}
		nIndex = hIcon ? m_i48.Add( hIcon ) : 0;
		m_m48.SetAt( strType, nIndex );
		break;
	}
	
	if ( hIcon ) DestroyIcon( hIcon );

	return nIndex;
}

//////////////////////////////////////////////////////////////////////
// CShellIcons add icon

int CShellIcons::Add(HICON hIcon, int nSize)
{
	if ( m_i16.m_hImageList == NULL ) Clear();

	switch ( nSize )
	{
	case 16:
		return m_i16.Add( hIcon );
	case 32:
		return m_i32.Add( hIcon );
	case 48:
		return m_i48.Add( hIcon );
	default:
		return -1;
	}
}

//////////////////////////////////////////////////////////////////////
// CShellIcons icon extractor

HICON CShellIcons::ExtractIcon(int nIndex, int nSize)
{
	HICON hIcon;
	switch ( nSize )
	{
	case 16:
		hIcon = m_i16.ExtractIcon( nIndex );
		if ( Settings.General.LanguageRTL ) hIcon = CreateMirroredIcon( hIcon );
		return hIcon;
	case 32:
		hIcon = m_i32.ExtractIcon( nIndex );
		if ( Settings.General.LanguageRTL ) hIcon = CreateMirroredIcon( hIcon );
		return hIcon;
	case 48:
		hIcon = m_i48.ExtractIcon( nIndex );
		if ( Settings.General.LanguageRTL ) hIcon = CreateMirroredIcon( hIcon );
		return hIcon;
	default:
		return NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// CShellIcons common type string

CString	CShellIcons::GetTypeString(LPCTSTR pszFile)
{
	CString strOutput;

	LPCTSTR pszType = _tcsrchr( pszFile, '.' );
	if ( ! pszType ) return strOutput;

	CString strName, strMime;
	Lookup( pszType, NULL, NULL, &strName, &strMime );

	if ( strName.GetLength() )
	{
		strOutput = strName;
		if ( strMime.GetLength() ) strOutput += _T(" (") + strMime + _T(")");
	}
	else
	{
		strOutput = pszType + 1;
	}

	return strOutput;
}

//////////////////////////////////////////////////////////////////////
// CShellIcons lookup

BOOL CShellIcons::Lookup(LPCTSTR pszType, HICON* phSmallIcon, HICON* phLargeIcon, CString* psName, CString* psMIME, HICON* phHugeIcon)
{
	DWORD nType, nResult;
	TCHAR szResult[MAX_PATH];
	HKEY hKey, hSub;

	if ( phSmallIcon ) *phSmallIcon = NULL;
	if ( phLargeIcon ) *phLargeIcon = NULL;
	if ( psName ) *psName = pszType + 1;
	if ( psMIME ) psMIME->Empty();

	if ( pszType == NULL || *pszType == 0 ) return FALSE;

	if ( RegOpenKeyEx( HKEY_CLASSES_ROOT, pszType, 0, KEY_READ, &hKey ) != ERROR_SUCCESS ) return FALSE;

	if ( psMIME )
	{
		nResult = sizeof(TCHAR) * MAX_PATH; nType = REG_SZ;
		if ( RegQueryValueEx( hKey, _T("Content Type"), NULL, &nType, (LPBYTE)szResult, &nResult ) == ERROR_SUCCESS )
		{
			szResult[ nResult / sizeof(TCHAR) ] = 0;
			*psMIME = szResult;
		}
	}

	nResult = sizeof(TCHAR) * MAX_PATH; nType = REG_SZ;
	if ( RegQueryValueEx( hKey, _T(""), NULL, &nType, (LPBYTE)szResult, &nResult ) != ERROR_SUCCESS )
	{
		RegCloseKey( hKey );
		return FALSE;
	}

	RegCloseKey( hKey );
	szResult[ nResult / sizeof(TCHAR) ] = 0;

	if ( RegOpenKeyEx( HKEY_CLASSES_ROOT, szResult, 0, KEY_READ, &hKey ) != ERROR_SUCCESS ) return FALSE;

	if ( psName )
	{
		nResult = sizeof(TCHAR) * MAX_PATH;
		nType = REG_SZ;
		if ( RegQueryValueEx( hKey, _T(""), NULL, &nType, (LPBYTE)szResult, &nResult ) == ERROR_SUCCESS )
		{
			szResult[ nResult / sizeof(TCHAR) ] = 0;
			*psName = szResult;
		}
	}

	if ( RegOpenKeyEx( hKey, _T("DefaultIcon"), 0, KEY_READ, &hSub ) != ERROR_SUCCESS )
	{
		if ( RegOpenKeyEx( hKey, _T("CurVer"), 0, KEY_READ, &hSub ) != ERROR_SUCCESS )
		{
			RegCloseKey( hKey );
			return FALSE;
		}
		nResult = sizeof(TCHAR) * MAX_PATH;
		if ( RegQueryValueEx( hSub, _T(""), NULL, &nType, (LPBYTE)szResult, &nResult ) != ERROR_SUCCESS )
		{
			RegCloseKey( hSub );
			RegCloseKey( hKey );
			return FALSE;
		}
		RegCloseKey( hSub );
		RegCloseKey( hKey );
		szResult[ nResult / sizeof(TCHAR) ] = 0;

		if ( RegOpenKeyEx( HKEY_CLASSES_ROOT, szResult, 0, KEY_READ, &hKey ) != ERROR_SUCCESS ) return FALSE;

		if ( psName )
		{
			nResult = sizeof(TCHAR) * MAX_PATH; nType = REG_SZ;
			if ( RegQueryValueEx( hKey, _T(""), NULL, &nType, (LPBYTE)szResult, &nResult ) == ERROR_SUCCESS )
			{
				szResult[ nResult / sizeof(TCHAR) ] = 0;
				*psName = szResult;
			}
		}

		if ( RegOpenKeyEx( hKey, _T("DefaultIcon"), 0, KEY_READ, &hSub ) != ERROR_SUCCESS )
		{
			RegCloseKey( hSub );
			RegCloseKey( hKey );
			return FALSE;
		}
	}

	nResult = sizeof(TCHAR) * MAX_PATH;
	nType = REG_SZ;
	if ( RegQueryValueEx( hSub, _T(""), NULL, &nType, (LPBYTE)szResult, &nResult ) != ERROR_SUCCESS )
	{
		RegCloseKey( hSub );
		RegCloseKey( hKey );
		return FALSE;
	}
	
	RegCloseKey( hSub );
	RegCloseKey( hKey );
	szResult[ nResult / sizeof(TCHAR) ] = 0;

	if ( ! LoadIcon( szResult, phSmallIcon, phLargeIcon, phHugeIcon ) )
		return FALSE;

	if ( Settings.General.LanguageRTL )
	{
		if ( phSmallIcon && *phSmallIcon )
			*phSmallIcon = CreateMirroredIcon( *phSmallIcon );
		if ( phLargeIcon && *phLargeIcon )
			*phLargeIcon = CreateMirroredIcon( *phLargeIcon );
		if ( phHugeIcon && *phHugeIcon )
			*phHugeIcon = CreateMirroredIcon( *phHugeIcon );
	}

	return TRUE;
}
