//
// ThumbCache.cpp
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
#include "Settings.h"
#include "ThumbCache.h"
#include "ImageServices.h"
#include "ImageFile.h"
#include "Library.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define THUMB_SIGNATURE	"RAZATDB1"


//////////////////////////////////////////////////////////////////////
// CThumbCache construction

CThumbCache::CThumbCache()
{
	m_bOpen		= FALSE;
	m_nOffset	= 0;
	m_pIndex	= NULL;
	m_nIndex	= 0;
	m_nBuffer	= 0;
}

CThumbCache::~CThumbCache()
{
	Close();
}

//////////////////////////////////////////////////////////////////////
// CThumbCache prepare

BOOL CThumbCache::Prepare(LPCTSTR pszPath, CSize* pszThumb, BOOL bCreate)
{
	CString strPath( pszPath );

	int nSlash = strPath.ReverseFind( '\\' );
	if ( nSlash >= 0 ) strPath = strPath.Left( nSlash );
	strPath += _T("\\SThumbs.dat");

	if ( m_bOpen && strPath.CompareNoCase( m_sPath ) == 0 )
	{
		if ( m_szThumb != *pszThumb )
		{
			Close();
			DeleteFile( strPath );
		}

		return TRUE;
	}
	else if ( m_bOpen )
	{
		Close();
	}

	if ( m_pFile.Open( strPath, CFile::modeReadWrite ) )
	{
		CHAR szID[8];
		m_pFile.Read( szID, 8 );

		if ( memcmp( szID, THUMB_SIGNATURE, 8 ) != 0 )
		{
			m_pFile.Close();
			return DeleteFile( strPath ) && Prepare( pszPath, pszThumb, bCreate );
		}

		m_pFile.Read( &m_szThumb.cx, 4 );
		m_pFile.Read( &m_szThumb.cy, 4 );

		if ( pszThumb->cx == 0 && pszThumb->cy == 0 ) *pszThumb = m_szThumb;

		if ( m_szThumb == *pszThumb )
		{
			m_pFile.Read( &m_nOffset, 4 );
			m_pFile.Read( &m_nIndex, 4 );

			for ( m_nBuffer = m_nIndex ; m_nBuffer & 63 ; m_nBuffer++ );
			m_pIndex = new THUMB_INDEX[ m_nBuffer ];

			if ( m_pIndex == NULL )
			{
				theApp.Message( MSG_ERROR, _T("Memory allocation error in CThumbCache::Prepare") );
				return FALSE;
			}

			m_pFile.Seek( m_nOffset, 0 );
			m_pFile.Read( m_pIndex, sizeof(THUMB_INDEX) * m_nIndex );

			m_sPath = strPath;
			m_bOpen = TRUE;
		}
		else
		{
			m_pFile.Close();
			DeleteFile( strPath );
		}
	}

	if ( ! m_bOpen )
	{
		if ( ! bCreate ) return FALSE;

		if ( ! m_pFile.Open( strPath, CFile::modeReadWrite|CFile::modeCreate ) ) return FALSE;

		SetFileAttributes( strPath, FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM );

		if ( pszThumb->cx == 0 && pszThumb->cy == 0 ) *pszThumb = CSize( Settings.Library.ThumbSize, Settings.Library.ThumbSize );

		m_szThumb = *pszThumb;

		m_pFile.Write( THUMB_SIGNATURE, 8 );
		m_pFile.Write( &m_szThumb.cx, 4 );
		m_pFile.Write( &m_szThumb.cy, 4 );
		m_pFile.Write( &m_nOffset, 4 );
		m_pFile.Write( &m_nIndex, 4 );
		m_nOffset = 24;

		m_sPath = strPath;
		m_bOpen = TRUE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CThumbCache close

void CThumbCache::Close()
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( m_bOpen == FALSE ) return;

	m_sPath.Empty();
	m_pFile.Close();
	m_bOpen = FALSE;

	if ( m_pIndex != NULL ) delete [] m_pIndex;

	m_pIndex	= NULL;
	m_nIndex	= 0;
	m_nBuffer	= 0;
}

//////////////////////////////////////////////////////////////////////
// CThumbCache load

BOOL CThumbCache::Load(LPCTSTR pszPath, CSize* pszThumb, DWORD nIndex, CImageFile* pImage)
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( ! Prepare( pszPath, pszThumb, FALSE ) ) return FALSE;

	THUMB_INDEX* pIndex = m_pIndex;

    DWORD nCount = m_nIndex;
	for ( ; nCount ; nCount--, pIndex++ )
	{
		if ( pIndex->nIndex == nIndex ) break;
	}

	if ( nCount == 0 ) return FALSE;

	FILETIME pTime;
	GetFileTime( pszPath, &pTime );
	if ( CompareFileTime( &pIndex->pTime, &pTime ) != 0 ) return FALSE;

	m_pFile.Seek( pIndex->nOffset, 0 );

	try
	{
		CArchive ar( &m_pFile, CArchive::load );
		pImage->Serialize( ar );
	}
	catch ( CException* pException )
	{
		pException->Delete();
		return FALSE;
	}

	ASSERT( pImage );
	if ( pImage && pImage->m_nWidth > 0 && pImage->m_nHeight > 0 )
		return TRUE;
	else
	{
		theApp.Message( MSG_DEBUG, _T("THUMBNAIL: Invalid width or height in CThumbCache::Load()") );
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CThumbCache save

BOOL CThumbCache::Store(LPCTSTR pszPath, CSize* pszThumb, DWORD nIndex, CImageFile* pImage)
{
	CSingleLock pLock( &m_pSection, TRUE );

	ASSERT( pImage );
	if ( ! pImage || pImage->m_nWidth <= 0 || pImage->m_nHeight <= 0 )
	{
		theApp.Message( MSG_DEBUG, _T("THUMBNAIL: Invalid width or height in CThumbCache::Store()") );
		return FALSE;
	}
	if ( ! Prepare( pszPath, pszThumb, TRUE ) ) return FALSE;

	DWORD nBlock = pImage->GetSerialSize();

	THUMB_INDEX* pIndex = m_pIndex;

    DWORD nCount = m_nIndex;
	for ( ; nCount ; nCount--, pIndex++ )
	{
		if ( pIndex->nIndex == nIndex ) break;
	}

	if ( nCount != 0 && pIndex->nLength != nBlock )
	{
		pIndex->nIndex = 0;
		nCount = 0;
	}

	if ( nCount == 0 )
	{
		THUMB_INDEX* pBestIndex		= NULL;
		DWORD nBestOverhead			= 0xFFFFFFFF;

		for ( pIndex = m_pIndex, nCount = m_nIndex ; nCount ; nCount--, pIndex++ )
		{
			if ( pIndex->nLength >= nBlock &&
				( pIndex->nIndex == 0 || Library.LookupFile( pIndex->nIndex ) == NULL ) )
			{
				DWORD nOverhead = pIndex->nLength - nBlock;

				if ( nOverhead < nBestOverhead )
				{
					pBestIndex = pIndex;
					nBestOverhead = nOverhead;
					if ( nOverhead == 0 ) break;
				}
			}
		}

		if ( pBestIndex != NULL )
		{
			pIndex = pBestIndex;
		}
		else
		{
			if ( m_nIndex >= m_nBuffer )
			{
				m_nBuffer += 64;
				THUMB_INDEX* pNew = new THUMB_INDEX[ m_nBuffer ];
				if ( m_nIndex ) CopyMemory( pNew, m_pIndex, sizeof(THUMB_INDEX) * m_nIndex );
				if ( m_pIndex ) delete [] m_pIndex;
				m_pIndex = pNew;
			}

			pIndex = m_pIndex + m_nIndex++;
			pIndex->nOffset = m_nOffset;
			pIndex->nLength = nBlock;

			m_nOffset += nBlock;
		}

		pIndex->nIndex = nIndex;
	}

	GetFileTime( pszPath, &pIndex->pTime );

	m_pFile.Seek( pIndex->nOffset, 0 );

	try
	{
		CArchive ar( &m_pFile, CArchive::store );
		pImage->Serialize( ar );
		ar.Flush();
	}
	catch ( CException* pException )
	{
		pException->Delete();
	}

	m_pFile.SetLength( m_nOffset + sizeof(THUMB_INDEX) * m_nIndex );
	m_pFile.Seek( 16, 0 );
	m_pFile.Write( &m_nOffset, 4 );
	m_pFile.Write( &m_nIndex, 4 );
	m_pFile.Seek( m_nOffset, 0 );
	m_pFile.Write( m_pIndex, sizeof(THUMB_INDEX) * m_nIndex );

	m_pFile.Flush();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CThumbCache purge

BOOL CThumbCache::Purge(LPCTSTR pszPath)
{
	CString strPath( pszPath );

	int nSlash = strPath.ReverseFind( '\\' );
	if ( nSlash >= 0 ) strPath = strPath.Left( nSlash );
	strPath += _T("\\SThumbs.dat");

	if ( GetFileAttributes( strPath ) == 0xFFFFFFFF ) return FALSE;

	DeleteFile( strPath );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CThumbCache file time lookup

BOOL CThumbCache::GetFileTime(LPCTSTR pszPath, FILETIME* pTime)
{
	BOOL bSuccess = FALSE;

	if ( Library.m_pfnGetFileAttributesExW != NULL )
	{
		USES_CONVERSION;
		WIN32_FILE_ATTRIBUTE_DATA pInfo;
		bSuccess = (*Library.m_pfnGetFileAttributesExW)( T2CW(pszPath), GetFileExInfoStandard, &pInfo );
		*pTime = pInfo.ftLastWriteTime;
	}
	if ( !bSuccess && Library.m_pfnGetFileAttributesExA != NULL )
	{
		USES_CONVERSION;
		WIN32_FILE_ATTRIBUTE_DATA pInfo;
		bSuccess = (*Library.m_pfnGetFileAttributesExA)( T2CA(pszPath), GetFileExInfoStandard, &pInfo );
		*pTime = pInfo.ftLastWriteTime;
	}
	if ( !bSuccess )
	{
		HANDLE hFile = CreateFile( pszPath, GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE | ( theApp.m_bNT ? FILE_SHARE_DELETE : 0 ),
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		VERIFY_FILE_ACCESS( hFile, pszPath )

		if ( hFile != INVALID_HANDLE_VALUE )
		{
			bSuccess = TRUE;
			::GetFileTime( hFile, NULL, NULL, pTime );
			CloseHandle( hFile );
		}
	}

	return bSuccess;
}
