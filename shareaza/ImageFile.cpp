//
// ImageFile.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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
#include "ImageServices.h"
#include "ImageFile.h"
#include "HttpRequest.h"
#include "Settings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CImageFile, CComObject)

/////////////////////////////////////////////////////////////////////////////
// CImageFile construction

CImageFile::CImageFile() :
	m_bScanned( FALSE ),
	m_nWidth( 0 ),
	m_nHeight( 0 ),
	m_nComponents( 0 ),
	m_bLoaded( FALSE ),
	m_pImage( NULL )
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
	m_bLoaded		= FALSE;
	m_pImage		= NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CImageFile load operations

BOOL CImageFile::LoadFromMemory(LPCTSTR pszType, LPCVOID pData, DWORD nLength, BOOL bScanOnly, BOOL bPartialOk)
{
	Clear();

	return m_bLoaded = m_ImageServices.LoadFromMemory( this, pszType, pData, nLength, bScanOnly, bPartialOk );
}

BOOL CImageFile::LoadFromFile(LPCTSTR pszFile, BOOL bScanOnly, BOOL bPartialOk)
{
	Clear();

	return m_bLoaded = m_ImageServices.LoadFromFile( this, pszFile, bScanOnly, bPartialOk );
}

BOOL CImageFile::LoadFromResource(HINSTANCE hInstance, UINT nResourceID, LPCTSTR pszType, BOOL bScanOnly, BOOL bPartialOk)
{
	Clear();

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

				CImageServices srv;
				m_bLoaded = srv.LoadFromMemory( this, pszType, pMemory, nSize, bScanOnly, bPartialOk );
			}
			FreeResource( hMemory );
		}
	}
	return m_bLoaded;
}

BOOL CImageFile::LoadFromURL(LPCTSTR pszURL)
{
	CHttpRequest pImageFetcher;

	pImageFetcher.SetURL( pszURL );
	pImageFetcher.LimitContentLength( Settings.Search.MaxPreviewLength * 100 );

	pImageFetcher.Execute( TRUE );
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
		if ( pBuffer == NULL ) return FALSE;

		strMIME.Replace( '/', '.' );
		return m_bLoaded = m_ImageServices.LoadFromMemory( this, strMIME, (LPVOID)pBuffer->m_pBuffer,
														   pBuffer->m_nLength );
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CImageFile save operations

BOOL CImageFile::SaveToMemory(LPCTSTR pszType, int nQuality, LPBYTE* ppBuffer, DWORD* pnLength)
{
	if ( ! m_bLoaded ) return FALSE;
	return m_ImageServices.SaveToMemory( this, pszType, nQuality, ppBuffer, pnLength );
}

/*BOOL CImageFile::SaveToFile(LPCTSTR pszType, int nQuality, HANDLE hFile, DWORD* pnLength)
{
	if ( ! m_bLoaded ) return FALSE;
	return m_ImageServices.SaveToFile( this, pszType, nQuality, hFile, pnLength );
}

BOOL CImageFile::SaveToFile(LPCTSTR pszFile, int nQuality)
{
	if ( ! m_bLoaded ) return FALSE;

	HANDLE hFile = CreateFile( pszFile, GENERIC_WRITE, 0,
		NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL );

	if ( hFile == INVALID_HANDLE_VALUE ) return FALSE;

	BOOL bResult = m_ImageServices.SaveToFile( this, pszFile, nQuality, hFile );

	CloseHandle( hFile );

	if ( ! bResult ) DeleteFile( pszFile );

	return bResult;
}*/

/////////////////////////////////////////////////////////////////////////////
// CImageFile serialization

DWORD CImageFile::GetSerialSize() const
{
	if ( ! m_bLoaded ) return 4;
	return 12 + ( ( m_nWidth * m_nComponents + 3 ) & ~3 ) * m_nHeight;
}

void CImageFile::Serialize(CArchive& ar)
{
	if ( ar.IsStoring() )
	{
		if ( ! m_bLoaded )
		{
			ar << m_bLoaded;
			return;
		}

		ar << m_nWidth;
		ar << m_nHeight;
		ar << m_nComponents;

		ar.Write( m_pImage, ( ( m_nWidth * m_nComponents + 3) & ~3 ) * m_nHeight );
	}
	else
	{
		Clear();

		ar >> m_nWidth;
		if ( ! m_nWidth ) return;
		ar >> m_nHeight;
		ar >> m_nComponents;

		int nPitch = ( ( m_nWidth * m_nComponents+ 3 ) & ~3 ) * m_nHeight;

		m_pImage = new BYTE[ nPitch  ];
		ReadArchive( ar, m_pImage, nPitch );

		m_bLoaded = TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CImageFile image access operations

HBITMAP CImageFile::CreateBitmap(HDC hUseDC)
{
	if ( ! m_bLoaded ) return NULL;
	if ( m_nComponents != 3 ) return NULL;

	BITMAPINFO pInfo = {};

	pInfo.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	pInfo.bmiHeader.biWidth			= (LONG)m_nWidth;
	pInfo.bmiHeader.biHeight		= (LONG)m_nHeight;
	pInfo.bmiHeader.biPlanes		= 1;
	pInfo.bmiHeader.biBitCount		= 24;
	pInfo.bmiHeader.biCompression	= BI_RGB;
	pInfo.bmiHeader.biSizeImage		= m_nWidth * m_nHeight * 3;

	HDC hDC = hUseDC ? hUseDC : GetDC( 0 );

	HBITMAP hBitmap = CreateDIBitmap( hDC, &pInfo.bmiHeader, 0, NULL, &pInfo, DIB_RGB_COLORS );

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

			SetDIBits( hDC, hBitmap, nY, 1, pLine, &pInfo, DIB_RGB_COLORS );

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

BOOL CImageFile::Resample(int nNewWidth, int nNewHeight)
{
	if ( m_nWidth <= 0 || m_nHeight <= 0 )
	{
		theApp.Message( MSG_DEBUG, _T("THUMBNAIL: Invalid width or height in CImageFile::Resample()") );
		return FALSE;
	}
	if ( nNewWidth <= 0 || nNewHeight <= 0 )
	{
		theApp.Message( MSG_DEBUG, _T("THUMBNAIL: Invalid new width or new height in CImageFile::Resample()") );
		return FALSE;
	}
	if ( ! m_bLoaded ) return FALSE;
	if ( m_nComponents != 3 ) return FALSE;
	if ( nNewWidth == m_nWidth && nNewHeight == m_nHeight ) return TRUE;

	DWORD nInPitch	= ( m_nWidth * 3 + 3 ) & ~3u;
	DWORD nOutPitch	= ( nNewWidth * 3 + 3 ) & ~3u;

	BYTE* pNew = new BYTE[ nOutPitch * nNewHeight ];
	BYTE* pOut = pNew;

	int* pColInfo = new int[ nNewWidth * 2 ];
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
	if ( ! m_bLoaded ) return FALSE;
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
	if ( ! m_bLoaded || m_nWidth <= 0 || m_nHeight <= 0 )
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
	if ( ! m_bLoaded ) return FALSE;
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
	if ( ! m_bLoaded ) return FALSE;
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
	if ( ! m_bLoaded ) return FALSE;
	if ( m_nComponents != 3 ) return FALSE;

	DWORD nPitch = ( m_nWidth * 3 + 3) & ~3u;

	BYTE* pImage = m_pImage;
	BYTE nTemp;

	for ( int nY = m_nHeight ; nY ; nY-- )
	{
		BYTE* pRow = pImage;
		pImage += nPitch;

		for ( int nX = m_nWidth ; nX ; nX-- )
		{
			nTemp = pRow[0];
			pRow[0] = pRow[2];
			pRow[2] = nTemp;
			pRow += 3;
		}
	}

	return TRUE;
}
