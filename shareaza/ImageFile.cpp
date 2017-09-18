//
// ImageFile.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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
#include "Buffer.h"
#include "ImageServices.h"
#include "ImageFile.h"
#include "HttpRequest.h"
#include "Settings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImageFile construction

CImageFile::CImageFile()
	: m_bScanned	( FALSE )
	, m_nWidth		( 0 )
	, m_nHeight		( 0 )
	, m_nComponents	( 0 )
	, m_pImage		( NULL )
	, m_nFlags		( 0 )
{
}

CImageFile::~CImageFile()
{
	Clear();
}

/////////////////////////////////////////////////////////////////////////////
// CImageFile clear operation

void CImageFile::Clear()
{
	delete [] m_pImage;

	m_bScanned		= FALSE;
	m_nWidth		= 0;
	m_nHeight		= 0;
	m_nComponents	= 0;
	m_pImage		= NULL;
	m_nFlags		= 0;
}

/////////////////////////////////////////////////////////////////////////////
// CImageFile load operations

BOOL CImageFile::LoadFromMemory(LPCTSTR pszType, LPCVOID pData, DWORD nLength, BOOL bScanOnly, BOOL bPartialOk)
{
	Clear();

	return ImageServices.LoadFromMemory( this, pszType, pData, nLength, bScanOnly, bPartialOk );
}

BOOL CImageFile::LoadFromFile(LPCTSTR pszFile, BOOL bScanOnly, BOOL bPartialOk)
{
	Clear();

	return ImageServices.LoadFromFile( this, pszFile, bScanOnly, bPartialOk );
}

BOOL CImageFile::LoadFromResource(HINSTANCE hInstance, UINT nResourceID, LPCTSTR pszType)
{
	Clear();

	BOOL bLoaded = FALSE;
	HMODULE hModule = (HMODULE)hInstance;
	HRSRC hRes = FindResource( hModule, MAKEINTRESOURCE( nResourceID ), pszType );
	if ( hRes  )
	{
		DWORD nSize			= SizeofResource( hModule, hRes );
		HGLOBAL hMemory		= LoadResource( hModule, hRes );
		if ( hMemory )
		{
			LPCVOID pMemory	= (LPCVOID)LockResource( hMemory );
			if ( pMemory )
			{
				CString strType;

				if ( _tcscmp( pszType, RT_JPEG ) == 0 )
				{
					pszType = _T(".jpg");
				}
				else if ( _tcscmp( pszType, RT_PNG ) == 0 )
				{
					pszType = _T(".png");
				}
				else
				{
					strType.Format( _T(".%s"), pszType );
					pszType = strType;
				}

				bLoaded = LoadFromMemory( pszType, pMemory, nSize );
			}
			FreeResource( hMemory );
		}
	}
	return bLoaded;
}

BOOL CImageFile::LoadFromURL(LPCTSTR pszURL)
{
	Clear();

	BOOL bLoaded = FALSE;
	CHttpRequest pImageFetcher;

	if ( !pImageFetcher.SetURL( pszURL ) )
		return FALSE;
	pImageFetcher.LimitContentLength( Settings.Search.MaxPreviewLength * 100 );

	if ( !pImageFetcher.Execute( TRUE ) )
		return FALSE;
	while ( pImageFetcher.IsPending() )
		Sleep( 50 );

	if ( pImageFetcher.GetStatusSuccess() )
	{
		CString strMIME = pImageFetcher.GetHeader( L"Content-Type" );

		if ( strMIME.CompareNoCase( L"image/jpeg" ) != 0 &&
			 strMIME.CompareNoCase( L"image/gif" ) != 0 &&
			 strMIME.CompareNoCase( L"image/bmp" ) != 0 &&
			 strMIME.CompareNoCase( L"image/png" ) != 0 )
		{
			theApp.Message( MSG_DEBUG, L"Preview failed: unacceptable content type." );
			return FALSE;
		}

		CBuffer* pBuffer = pImageFetcher.GetResponseBuffer();
		if ( pBuffer )
		{
			strMIME.Replace( '/', '.' );
			bLoaded = LoadFromMemory( strMIME, (LPVOID)pBuffer->m_pBuffer, pBuffer->m_nLength );
			if ( bLoaded )
				m_nFlags |= idRemote;
		}
	}

	return bLoaded;
}

BOOL CImageFile::LoadFromBitmap(HBITMAP hBitmap, BOOL bAlpha, BOOL bScanOnly)
{
	Clear();

	BITMAP bmInfo = {};
	if ( ! GetObject( hBitmap, sizeof( BITMAP ), &bmInfo ) )
		return FALSE;

	if ( bmInfo.bmType != 0 || bmInfo.bmPlanes != 1 || ! bmInfo.bmBits || bmInfo.bmWidth <= 0 || bmInfo.bmHeight <= 0 )
		// Unsupported format
		return FALSE;

	m_bScanned		= TRUE;
	m_nWidth		= bmInfo.bmWidth;
	m_nHeight		= bmInfo.bmHeight;
	m_nComponents	= ( bAlpha ? 4 : 3 );

	if ( bScanOnly )
		return TRUE;

	DWORD line_size = ( m_nWidth * m_nComponents + 3 ) & ~3;
	m_pImage = new BYTE[ line_size * m_nHeight ];
	if ( m_pImage )
	{
		HDC hDC = GetDC( NULL );
		BITMAPINFOHEADER bmi = { sizeof( BITMAPINFOHEADER ), bmInfo.bmWidth, -bmInfo.bmHeight, 1, ( ( m_nComponents == 3 ) ? 24u : 32u ), BI_RGB };
		int copied = GetDIBits( hDC, hBitmap, 0, bmInfo.bmHeight, m_pImage, (BITMAPINFO*)&bmi, DIB_RGB_COLORS );
		ReleaseDC( NULL, hDC );

		if ( copied == m_nHeight )
		{
			if ( SwapRGB() )
			{
				return TRUE;
			}
		}
	}

	delete [] m_pImage;
	m_pImage = NULL;

	return FALSE;
}

BOOL CImageFile::LoadFromService(const IMAGESERVICEDATA* pParams, SAFEARRAY* pArray)
{
	Clear();

	m_bScanned		= TRUE;
	m_nWidth		= pParams->nWidth;
	m_nHeight		= pParams->nHeight;
	m_nComponents	= pParams->nComponents;

	if ( pArray == NULL )
	{
		// Scanned only
		return TRUE;
	}

	BOOL bLoaded = FALSE;
	LONG nArray = 0;
	if ( SUCCEEDED( SafeArrayGetUBound( pArray, 1, &nArray ) ) )
	{
		nArray++;
		LONG nFullSize = ( ( pParams->nWidth * pParams->nComponents + 3 ) & ~3 ) * pParams->nHeight;
		if ( nArray == nFullSize )
		{
			LPBYTE pData = NULL;
			if ( SUCCEEDED( SafeArrayAccessData( pArray, (VOID**)&pData ) ) )
			{
				m_pImage = new BYTE[ nArray ];
				if ( m_pImage )
				{
					CopyMemory( m_pImage, pData, nArray );
					bLoaded = TRUE;
				}

				VERIFY( SUCCEEDED( SafeArrayUnaccessData( pArray ) ) );
			}
		}
	}

	return bLoaded;
}

/////////////////////////////////////////////////////////////////////////////
// CImageFile save operations

BOOL CImageFile::SaveToMemory(LPCTSTR pszType, int nQuality, LPBYTE* ppBuffer, DWORD* pnLength)
{
	if ( ! IsLoaded() )
		return FALSE;

	return ImageServices.SaveToMemory( this, pszType, nQuality, ppBuffer, pnLength );
}

BOOL CImageFile::SaveToFile(LPCTSTR pszFile, int nQuality, DWORD* pnLength)
{
	if ( ! IsLoaded() )
		return FALSE;

	return ImageServices.SaveToFile( this, pszFile, nQuality, pnLength );
}

/////////////////////////////////////////////////////////////////////////////
// CImageFile serialization

DWORD CImageFile::GetSerialSize() const
{
	if ( ! IsLoaded() ) return 4;
	return 12 + ( ( m_nWidth * m_nComponents + 3 ) & ~3 ) * m_nHeight;
}

void CImageFile::Serialize(CArchive& ar)
{
	if ( ar.IsStoring() )
	{
		if ( ! IsLoaded() )
		{
			ar << (int)0;
			return;
		}

		ar << m_nWidth;
		ar << m_nHeight;
		DWORD nCompositeValue = ( m_nFlags << 16 ) | ( m_nComponents );
		ar << nCompositeValue;

		ar.Write( m_pImage, ( ( m_nWidth * m_nComponents + 3 ) & ~3 ) * m_nHeight );
	}
	else
	{
		Clear();

		ar >> m_nWidth;
		if ( ! m_nWidth )
			return;
		ar >> m_nHeight;
		DWORD nCompositeValue;
		ar >> nCompositeValue;

		// Get higher bits for flags
		m_nFlags = nCompositeValue >> 16;
		// Clear high bits for components
		m_nComponents = nCompositeValue & 0x0000FFFF;

		int nPitch = ( ( m_nWidth * m_nComponents + 3 ) & ~3 ) * m_nHeight;

		m_pImage = new BYTE[ nPitch  ];
		ReadArchive( ar, m_pImage, nPitch );

		m_bScanned = TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CImageFile image access operations

HBITMAP CImageFile::CreateBitmap(HDC hUseDC)
{
	if ( ! IsLoaded() || m_nComponents != 3 )
		return NULL;

	BITMAPV5HEADER pV5Header = {};

	pV5Header.bV5Size			= sizeof(BITMAPV5HEADER);
	pV5Header.bV5Width			= (LONG)m_nWidth;
	pV5Header.bV5Height			= (LONG)m_nHeight;
	pV5Header.bV5Planes			= 1;
	pV5Header.bV5BitCount		= 24; // Not 32 bit :(
	pV5Header.bV5Compression	= BI_RGB;
	pV5Header.bV5SizeImage		= m_nWidth * m_nHeight * 3;

	// The following mask specification specifies a supported 32 BPP alpha format for Windows XP.
	// pV5Header.bV5RedMask   =  0x00FF0000;
	// pV5Header.bV5GreenMask =  0x0000FF00;
	// pV5Header.bV5BlueMask  =  0x000000FF;
	// pV5Header.bV5AlphaMask =  0xFF000000;

	HDC hDC = hUseDC ? hUseDC : GetDC( 0 );
	HBITMAP hBitmap;
	__try
	{
		void* pBits = NULL;
		hBitmap = CreateDIBSection( hDC, (BITMAPINFO*)&pV5Header, DIB_RGB_COLORS, (void**)&pBits, NULL, 0ul );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		hBitmap = NULL;
	}

	if ( hBitmap )
	{
		DWORD nPitch	= ( m_nWidth * 3 + 3 ) & ~3u;
		BYTE* pLine		= m_pImage;

		for ( int nY = m_nHeight; nY--; )
		{
			struct SwapRGB
			{
				void operator()(BYTE* pBegin, BYTE* pEnd)
				{
					for ( ; pBegin != pEnd; pBegin += 3 )
						std::swap( pBegin[ 0 ], pBegin[ 2 ] );
				}
			};

			SwapRGB()( pLine, pLine + m_nWidth * 3 );

			SetDIBits( hDC, hBitmap, nY, 1, pLine, (BITMAPINFO*)&pV5Header, DIB_RGB_COLORS );

			SwapRGB()( pLine, pLine + m_nWidth * 3 );

			pLine += nPitch;
		}
	}

	if ( hDC != hUseDC )
	{
		SelectObject( hDC, GetStockObject( ANSI_VAR_FONT ) ); // font leak fix
		ReleaseDC( 0, hDC );
	}

	return hBitmap;
}

/////////////////////////////////////////////////////////////////////////////
// CImageFile image modification operations

BOOL CImageFile::FitTo(int nNewWidth, int nNewHeight)
{
	int nSize = ( nNewHeight * m_nWidth ) / m_nHeight;
	if ( nSize > nNewWidth )
	{
		nSize = ( nNewWidth * m_nHeight ) / m_nWidth;
		return Resample( nNewWidth, nSize );
	}
	else
	{
		return Resample( nSize, nNewHeight );
	}
}

BOOL CImageFile::Resample(int nNewWidth, int nNewHeight)
{
	if ( m_nWidth <= 0 || m_nHeight <= 0 )
		return FALSE;
	if ( nNewWidth <= 0 || nNewHeight <= 0 )
		return FALSE;
	if ( ! IsLoaded() )
		return FALSE;
	if ( m_nComponents != 3 )
		return FALSE;
	if ( nNewWidth == m_nWidth && nNewHeight == m_nHeight )
		return TRUE;

	DWORD nInPitch	= ( m_nWidth * 3 + 3 ) & ~3u;
	DWORD nOutPitch	= ( nNewWidth * 3 + 3 ) & ~3u;

	BYTE* pNew = new BYTE[ nOutPitch * nNewHeight ];
	if ( ! pNew )
		return FALSE;
	BYTE* pOut = pNew;

	int* pColInfo = new int[ nNewWidth * 2 ];
	if ( ! pColInfo )
	{
		delete [] pNew;
		return FALSE;
	}
	int* pColPtr = pColInfo;

	for ( int nX = 0 ; nX < nNewWidth ; nX++ )
	{
		int nFirst = ( nX * m_nWidth / nNewWidth );
		int nCount = ( (nX+1) * m_nWidth / nNewWidth ) - nFirst + 1;
		if ( nFirst + nCount >= m_nWidth ) nCount = 1;
		*pColPtr++ = nFirst * 3;
		*pColPtr++ = nCount;
	}

	for ( int nY = 0 ; nY < nNewHeight ; nY++ )
	{
		int nFirst = ( nY * m_nHeight / nNewHeight );
		int nCount = ( ( nY + 1 ) * m_nHeight / nNewHeight ) - nFirst + 1;

		if ( nFirst + nCount >= m_nHeight ) nCount = 1;

		BYTE* pRow = m_pImage + nInPitch * nFirst;
		pColPtr = pColInfo;

		for ( int nX = 0 ; nX < nNewWidth ; nX++, pColPtr++ )
		{
			BYTE* pIn = pRow + *pColPtr++;

			DWORD nRed = 0, nGreen = 0, nBlue = 0, nPixels = 0;

			for ( int nYY = nCount ; nYY ; nYY-- )
			{
				for ( int nXX = *pColPtr ; nXX ; nXX-- )
				{
					nRed	+= *pIn++;
					nGreen	+= *pIn++;
					nBlue	+= *pIn++;
					nPixels++;
				}

				pIn += nInPitch - *pColPtr - *pColPtr - *pColPtr;
			}

			*pOut++ = (BYTE)( nRed / nPixels );
			*pOut++ = (BYTE)( nGreen / nPixels );
			*pOut++ = (BYTE)( nBlue / nPixels );

		}
		pOut += ( nOutPitch - nNewWidth * 3 );
	}

	delete [] pColInfo;
	delete [] m_pImage;

	m_pImage	= pNew;
	m_nWidth	= nNewWidth;
	m_nHeight	= nNewHeight;

	return TRUE;
}

/*BOOL CImageFile::FastResample(int nNewWidth, int nNewHeight)
{
	if ( ! IsLoaded() ) return FALSE;
	if ( m_nComponents != 3 ) return FALSE;
	if ( nNewWidth == m_nWidth && nNewHeight == m_nHeight ) return TRUE;

	DWORD nInPitch	= ( m_nWidth * 3 + 3 ) & ~3u;
	DWORD nOutPitch	= ( nNewWidth * 3 + 3 ) & ~3u;

	BYTE *pNew, *pRow, *pIn, *pOut;

	pOut = pNew = new BYTE[ nOutPitch * nNewHeight ];

	for ( int nY = 0 ; nY < nNewHeight ; nY++ )
	{
		pRow = m_pImage + nInPitch * ( nY * m_nHeight / nNewHeight );

		for ( int nX = 0 ; nX < nNewWidth ; nX++ )
		{
			pIn = pRow + 3 * ( nX * m_nWidth / nNewWidth );
			*pOut++ = *pIn++;
			*pOut++ = *pIn++;
			*pOut++ = *pIn++;
		}

		pOut += ( nOutPitch - nNewWidth * 3 );
	}

	delete [] m_pImage;

	m_pImage	= pNew;
	m_nWidth	= nNewWidth;
	m_nHeight	= nNewHeight;

	return TRUE;
}*/

/////////////////////////////////////////////////////////////////////////////
// CImageFile image component modification

BOOL CImageFile::EnsureRGB(COLORREF crBack)
{
	if ( ! IsLoaded() || m_nWidth <= 0 || m_nHeight <= 0 )
	{
		return FALSE;
	}
	else if ( m_nComponents == 3 )
	{
		return TRUE;
	}
	else if ( m_nComponents == 1 )
	{
		return MonoToRGB();
	}
	else if ( m_nComponents == 4 )
	{
		return AlphaToRGB( crBack );
	}
	else
	{
		return FALSE;
	}
}

BOOL CImageFile::MonoToRGB()
{
	if ( ! IsLoaded() ) return FALSE;
	if ( m_nComponents == 3 ) return TRUE;
	if ( m_nComponents != 1 ) return FALSE;

	DWORD nInPitch	= ( m_nWidth + 3 ) & ~3u;
	DWORD nOutPitch	= ( m_nWidth * 3 + 3 ) & ~3u;

	BYTE* pNew		= new BYTE[ nOutPitch * m_nHeight ];
	BYTE* pInRow	= m_pImage;
	BYTE* pOutRow	= pNew;

	for ( int nY = m_nHeight ; nY ; nY-- )
	{
		BYTE* pInCol	= pInRow;
		BYTE* pOutCol	= pOutRow;

		for ( int nX = m_nWidth ; nX ; nX-- )
		{
			*pOutCol++ = *pInCol;
			*pOutCol++ = *pInCol;
			*pOutCol++ = *pInCol++;
		}

		pInRow += nInPitch;
		pOutRow += nOutPitch;
	}

	delete [] m_pImage;

	m_pImage		= pNew;
	m_nComponents	= 3;

	return TRUE;
}

BOOL CImageFile::AlphaToRGB(COLORREF crBack)
{
	if ( ! IsLoaded() ) return FALSE;
	if ( m_nComponents == 3 ) return TRUE;
	if ( m_nComponents != 4 ) return FALSE;

	DWORD nInPitch	= ( m_nWidth * 4 + 3 ) & ~3u;
	DWORD nOutPitch	= ( m_nWidth * 3 + 3 ) & ~3u;

	BYTE* pNew		= new BYTE[ nOutPitch * m_nHeight ];
	BYTE* pInRow	= m_pImage;
	BYTE* pOutRow	= pNew;

	for ( int nY = m_nHeight ; nY ; nY-- )
	{
		BYTE* pInCol	= pInRow;
		BYTE* pOutCol	= pOutRow;

		for ( int nX = m_nWidth ; nX ; nX-- )
		{
			DWORD nAlpha = (DWORD)pInCol[3];

			if ( nAlpha == 255 )
			{
				*pOutCol++ = *pInCol++;
				*pOutCol++ = *pInCol++;
				*pOutCol++ = *pInCol++;
				pInCol++;
			}
			else if ( nAlpha == 0 )
			{
				*pOutCol++ = GetRValue( crBack );
				*pOutCol++ = GetGValue( crBack );
				*pOutCol++ = GetBValue( crBack );
				pInCol += 4;
			}
			else
			{
				*pOutCol++ = (BYTE)( ( (DWORD)(*pInCol++) * nAlpha + (DWORD)GetRValue( crBack ) * ( 255 - nAlpha ) ) / 255 );
				*pOutCol++ = (BYTE)( ( (DWORD)(*pInCol++) * nAlpha + (DWORD)GetGValue( crBack ) * ( 255 - nAlpha ) ) / 255 );
				*pOutCol++ = (BYTE)( ( (DWORD)(*pInCol++) * nAlpha + (DWORD)GetBValue( crBack ) * ( 255 - nAlpha ) ) / 255 );
				pInCol++;
			}
		}

		pInRow += nInPitch;
		pOutRow += nOutPitch;
	}

	delete [] m_pImage;

	m_pImage		= pNew;
	m_nComponents	= 3;

	return TRUE;
}

BOOL CImageFile::SwapRGB()
{
	if ( ! IsLoaded() ) return FALSE;
	if ( m_nComponents != 3 && m_nComponents != 4 ) return FALSE;

	const DWORD nPitch = ( m_nWidth * m_nComponents + 3 ) & ~3u;
	BYTE* pImage = m_pImage;

	for ( int nY = m_nHeight ; nY ; nY-- )
	{
		BYTE* pRow = pImage;
		pImage += nPitch;

		for ( int nX = m_nWidth ; nX ; nX-- )
		{
			const BYTE nTemp = pRow[0];
			pRow[0] = pRow[2];
			pRow[2] = nTemp;
			pRow += m_nComponents;
		}
	}

	return TRUE;
}

HBITMAP CImageFile::LoadBitmapFromFile(LPCTSTR pszFile)
{
	if ( HBITMAP hBitmap = (HBITMAP)LoadImage( NULL, pszFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE  ) )
		return hBitmap;

	CImageFile pFile;
	if ( pFile.LoadFromFile( pszFile, FALSE, FALSE ) && pFile.EnsureRGB() )
		return pFile.CreateBitmap();

	return NULL;
}

static const LPCTSTR pTypes[] = { RT_PNG, RT_JPEG };

HBITMAP CImageFile::LoadBitmapFromResource(UINT nResourceID, HINSTANCE hInstance)
{
	if ( HBITMAP hBitmap = (HBITMAP)LoadImage( hInstance, MAKEINTRESOURCE( nResourceID ), IMAGE_BITMAP, 0, 0, 0 ) )
		return hBitmap;

	for ( int i = 0; i < _countof( pTypes ); ++i )
	{
		CImageFile pFile;
		if ( pFile.LoadFromResource( hInstance, nResourceID, pTypes[ i ] ) && pFile.EnsureRGB() )
			return pFile.CreateBitmap();
	}

	return NULL;
}
