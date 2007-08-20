//
// ShellIcons.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "Shareaza.h"
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
}

CShellIcons::~CShellIcons()
{
}

//////////////////////////////////////////////////////////////////////
// CShellIcons clear

void CShellIcons::Clear()
{
	if ( m_i16.m_hImageList ) m_i16.DeleteImageList();
	if ( m_i32.m_hImageList ) m_i32.DeleteImageList();
	if ( m_i48.m_hImageList ) m_i48.DeleteImageList();
	
	m_i16.Create( 16, 16, ILC_COLOR32|ILC_MASK, SHI_MAX, 4 );
	m_i32.Create( 32, 32, ILC_COLOR32|ILC_MASK, 1, 4 );
	m_i48.Create( 48, 48, ILC_COLOR32|ILC_MASK, 1, 4 );
	
	CBitmap bmBase;
	HICON hTemp;
	
	if ( theApp.m_bRTL ) 
		bmBase.LoadBitmap( IDB_SHELL_BASE_RTL );
	else
		bmBase.LoadBitmap( IDB_SHELL_BASE );
	m_i16.Add( &bmBase, RGB( 0, 255, 0 ) );
	m_i16.SetOverlayImage( SHI_LOCKED, SHI_O_LOCKED );
	m_i16.SetOverlayImage( SHI_PARTIAL, SHI_O_PARTIAL );
	m_i16.SetOverlayImage( SHI_COLLECTION, SHI_O_COLLECTION );
	m_i16.SetOverlayImage( SHI_COMMERCIAL, SHI_O_COMMERCIAL );
	m_i16.SetOverlayImage( SHI_RATING_FAKE, SHI_O_RATING_FAKE );
	m_i16.SetOverlayImage( SHI_RATING_AVERAGE, SHI_O_RATING_AVERAGE );
	m_i16.SetOverlayImage( SHI_RATING_GOOD, SHI_O_RATING_GOOD );
	
	hTemp = (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_FILE), IMAGE_ICON, 32, 32, 0 );
	AddIcon( hTemp, m_i32 );
	hTemp = (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_FILE), IMAGE_ICON, 48, 48, 0 );
	AddIcon( hTemp, m_i48 );
	
	hTemp = (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_EXECUTABLE), IMAGE_ICON, 32, 32, 0 );
	AddIcon( hTemp, m_i32 );
	hTemp = (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_EXECUTABLE), IMAGE_ICON, 48, 48, 0 );
	AddIcon( hTemp, m_i48 );
	
	hTemp = (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_COLLECTION_MASK), IMAGE_ICON, 32, 32, 0 );
	// not needed?
	m_i32.SetOverlayImage( AddIcon( hTemp, m_i32 ), SHI_O_COLLECTION );
	hTemp = (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_COLLECTION_MASK), IMAGE_ICON, 48, 48, 0 );
	// not needed?
	m_i48.SetOverlayImage( AddIcon( hTemp, m_i48 ), SHI_O_COLLECTION );
	
	m_m16.RemoveAll();
	m_m32.RemoveAll();
	m_m48.RemoveAll();
	
	m_m16.SetAt( _T(".exe"), SHI_EXECUTABLE );
	m_m32.SetAt( _T(".exe"), 1 );
	m_m48.SetAt( _T(".exe"), 1 );
}

//////////////////////////////////////////////////////////////////////
// CShellIcons get

int CShellIcons::Get(LPCTSTR pszFile, int nSize)
{
	LPCTSTR pszType = _tcsrchr( pszFile, '.' );
	if ( pszType == NULL ) return 0;
	
	if ( m_i16.m_hImageList == NULL ) Clear();

	CString strType( pszType );
	ToLower( strType );

	HICON hIcon = NULL;
	int nIndex = 0;

	SHFILEINFO sfi = { 0 };
	switch ( nSize )
	{
	case 16:
		if ( m_m16.Lookup( strType, nIndex ) ) return nIndex;
		Lookup( pszType, &hIcon, NULL, NULL, NULL );
		if ( ! hIcon && SHGetFileInfo( pszFile, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof( SHFILEINFO ),
			SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | SHGFI_SMALLICON ) )
		{
			hIcon = theApp.m_bRTL ? CreateMirroredIcon( sfi.hIcon ) : sfi.hIcon;
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
			hIcon = theApp.m_bRTL ? CreateMirroredIcon( sfi.hIcon ) : sfi.hIcon;
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
			hIcon = theApp.m_bRTL ? CreateMirroredIcon( sfi.hIcon ) : sfi.hIcon;
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
		if ( theApp.m_bRTL ) hIcon = CreateMirroredIcon( hIcon );
		return hIcon;
	case 32:
		hIcon = m_i32.ExtractIcon( nIndex );
		if ( theApp.m_bRTL ) hIcon = CreateMirroredIcon( hIcon );
		return hIcon;
	case 48:
		hIcon = m_i48.ExtractIcon( nIndex );
		if ( theApp.m_bRTL ) hIcon = CreateMirroredIcon( hIcon );
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
	TCHAR szResult[128];
	HKEY hKey, hSub;

	if ( phSmallIcon ) *phSmallIcon = NULL;
	if ( phLargeIcon ) *phLargeIcon = NULL;
	if ( psName ) *psName = pszType + 1;
	if ( psMIME ) psMIME->Empty();

	if ( pszType == NULL || *pszType == 0 ) return FALSE;

	if ( RegOpenKeyEx( HKEY_CLASSES_ROOT, pszType, 0, KEY_READ, &hKey ) != ERROR_SUCCESS ) return FALSE;

	if ( psMIME )
	{
		nResult = sizeof(TCHAR) * 128; nType = REG_SZ;
		if ( RegQueryValueEx( hKey, _T("Content Type"), NULL, &nType, (LPBYTE)szResult, &nResult ) == ERROR_SUCCESS )
		{
			szResult[ nResult / sizeof(TCHAR) ] = 0;
			*psMIME = szResult;
		}
	}

	nResult = sizeof(TCHAR) * 128; nType = REG_SZ;
	if ( RegQueryValueEx( hKey, _T(""), NULL, &nType, (LPBYTE)szResult, &nResult ) != ERROR_SUCCESS )
	{
		RegCloseKey( hKey );
		return FALSE;
	}

	RegCloseKey( hKey );
	szResult[ nResult / sizeof(TCHAR) ] = 0;

	if ( RegOpenKeyEx( HKEY_CLASSES_ROOT, szResult, 0, KEY_READ, &hKey ) != ERROR_SUCCESS ) return 0;

	if ( psName )
	{
		nResult = sizeof(TCHAR) * 128; nType = REG_SZ;
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
		nResult = sizeof(TCHAR) * 128;
		if ( RegQueryValueEx( hSub, _T(""), NULL, &nType, (LPBYTE)szResult, &nResult ) != ERROR_SUCCESS )
		{
			RegCloseKey( hSub );
			RegCloseKey( hKey );
			return FALSE;
		}
		RegCloseKey( hKey );
		szResult[ nResult / sizeof(TCHAR) ] = 0;

		if ( RegOpenKeyEx( HKEY_CLASSES_ROOT, szResult, 0, KEY_READ, &hKey ) != ERROR_SUCCESS ) return 0;
		if ( psName )
		{
			nResult = sizeof(TCHAR) * 128; nType = REG_SZ;
			if ( RegQueryValueEx( hKey, _T(""), NULL, &nType, (LPBYTE)szResult, &nResult ) == ERROR_SUCCESS )
			{
				szResult[ nResult / sizeof(TCHAR) ] = 0;
				*psName = szResult;
			}
		}

		if ( RegOpenKeyEx( hKey, _T("DefaultIcon"), 0, KEY_READ, &hSub ) != ERROR_SUCCESS )
		{
			RegCloseKey( hKey );
			return FALSE;
		}
	}

	nResult = sizeof(TCHAR) * 128; nType = REG_SZ;
	if ( RegQueryValueEx( hSub, _T(""), NULL, &nType, (LPBYTE)szResult, &nResult ) != ERROR_SUCCESS )
	{
		RegCloseKey( hSub );
		RegCloseKey( hKey );
		return FALSE;
	}
	
	RegCloseKey( hSub );
	RegCloseKey( hKey );
	szResult[ nResult / sizeof(TCHAR) ] = 0;

	CString strIcon( szResult );

	int nIcon, nIndex = strIcon.ReverseFind( ',' );
	if ( nIndex < 0 && strIcon.Right(3).MakeLower() != _T("ico") ) return 0;

	if ( nIndex != -1 )
	{
		if ( _stscanf( strIcon.Mid( nIndex + 1 ), _T("%i"), &nIcon ) != 1 ) return FALSE;
		strIcon = strIcon.Left( nIndex );
	}
	else nIndex = nIcon = 0;

	if ( strIcon.GetLength() < 3 ) return FALSE;

	if ( strIcon.GetAt( 0 ) == '\"' && strIcon.GetAt( strIcon.GetLength() - 1 ) == '\"' )
		strIcon = strIcon.Mid( 1, strIcon.GetLength() - 2 );

	BOOL bSuccess = FALSE;

	if ( phLargeIcon || phSmallIcon )
	{
		if ( ExtractIconEx( strIcon, nIcon, phLargeIcon, phSmallIcon, 1 ) )
		{
			bSuccess |= ( phLargeIcon && *phLargeIcon ) || ( phSmallIcon && *phSmallIcon );
			if ( theApp.m_bRTL ) 
			{
				if ( phLargeIcon && *phLargeIcon ) 
					*phLargeIcon = CreateMirroredIcon( *phLargeIcon );
				if ( phSmallIcon && *phSmallIcon ) 
					*phSmallIcon = CreateMirroredIcon( *phSmallIcon );
			}
		}
	}

	if ( theApp.m_pfnPrivateExtractIconsW && phHugeIcon )
	{
		UINT nLoadedID;

		if ( theApp.m_pfnPrivateExtractIconsW( strIcon, nIcon, 48, 48, phHugeIcon, &nLoadedID, 1, 0 ) )
		{
			bSuccess = TRUE;
			if ( phHugeIcon && *phHugeIcon && theApp.m_bRTL )
				*phHugeIcon = CreateMirroredIcon( *phHugeIcon );
		}
	}

	return bSuccess != 0;
}

//////////////////////////////////////////////////////////////////////
// CShellIcons drawing

void CShellIcons::Draw(CDC* pDC, int nIcon, int nSize, int nX, int nY, COLORREF crBack, BOOL bSelected)
{
	ImageList_DrawEx( ShellIcons.GetHandle( nSize ), nIcon, pDC->GetSafeHdc(),
		nX, nY, nSize, nSize, crBack, CLR_DEFAULT, bSelected ? ILD_SELECTED : ILD_NORMAL );
	
	if ( crBack != CLR_NONE ) pDC->ExcludeClipRect( nX, nY, nX + nSize, nY + nSize );
}
