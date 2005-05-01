//
// ImageFile.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
#include "ImageServices.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// This detects ICL and makes necessary changes for proper compilation
#if __INTEL_COMPILER > 0
#define asm_m_nWidth CImageFile.m_nWidth
#else
#define asm_m_nWidth CImageFile::m_nWidth
#endif

IMPLEMENT_DYNAMIC(CImageFile, CComObject)


/////////////////////////////////////////////////////////////////////////////
// CImageFile construction

CImageFile::CImageFile(CImageServices* pService)
{
	m_pService		= pService;
	m_bScanned		= FALSE;
	m_nWidth		= 0;
	m_nHeight		= 0;
	m_nComponents	= 0;
	m_bLoaded		= FALSE;
	m_pImage		= NULL;
}

CImageFile::~CImageFile()
{
	Clear();
}

/////////////////////////////////////////////////////////////////////////////
// CImageFile clear operation

void CImageFile::Clear()
{
	if ( m_bLoaded ) delete [] m_pImage;

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
	return m_pService->LoadFromMemory( this, pszType, pData, nLength, bScanOnly, bPartialOk );
}

BOOL CImageFile::LoadFromFile(LPCTSTR pszType, HANDLE hFile, DWORD nLength, BOOL bScanOnly, BOOL bPartialOk)
{
	return m_pService->LoadFromFile( this, pszType, hFile, nLength, bScanOnly, bPartialOk );
}

BOOL CImageFile::LoadFromFile(LPCTSTR pszFile, BOOL bScanOnly, BOOL bPartialOk)
{
	HANDLE hFile = CreateFile( pszFile, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

	if ( hFile == INVALID_HANDLE_VALUE ) return FALSE;

	BOOL bResult = m_pService->LoadFromFile( this, pszFile, hFile, GetFileSize( hFile, NULL ), bScanOnly, bPartialOk );

	CloseHandle( hFile );

	return bResult;
}

BOOL CImageFile::LoadFromResource(HINSTANCE hInstance, UINT nResourceID, LPCTSTR pszType, BOOL bScanOnly, BOOL bPartialOk)
{
	HMODULE hModule = (HMODULE)hInstance;
	HRSRC hRes = FindResource( hModule, MAKEINTRESOURCE( nResourceID ), pszType );

	if ( hRes == NULL ) return FALSE;

	DWORD nSize			= SizeofResource( hModule, hRes );
	HGLOBAL hMemory		= ::LoadResource( hModule, hRes );
	LPCVOID pMemory		= (LPCVOID)LockResource( hMemory );
	CString strType;

	if ( pszType == RT_BITMAP || _tcscmp( pszType, _T("BMP") ) == 0 )
	{
		pszType = _T(".bmp");
	}
	else if ( _tcscmp( pszType, RT_JPEG ) == 0 )
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

	return m_pService->LoadFromMemory( this, pszType, pMemory, nSize, bScanOnly, bPartialOk );
}

/////////////////////////////////////////////////////////////////////////////
// CImageFile save operations

BOOL CImageFile::SaveToMemory(LPCTSTR pszType, int nQuality, LPBYTE* ppBuffer, DWORD* pnLength)
{
	return m_pService->SaveToMemory( this, pszType, nQuality, ppBuffer, pnLength );
}

BOOL CImageFile::SaveToFile(LPCTSTR pszType, int nQuality, HANDLE hFile, DWORD* pnLength)
{
	return m_pService->SaveToFile( this, pszType, nQuality, hFile, pnLength );
}

BOOL CImageFile::SaveToFile(LPCTSTR pszFile, int nQuality)
{
	HANDLE hFile = CreateFile( pszFile, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL );

	if ( hFile == INVALID_HANDLE_VALUE ) return FALSE;

	BOOL bResult = m_pService->SaveToFile( this, pszFile, nQuality, hFile );

	CloseHandle( hFile );

	if ( ! bResult ) DeleteFile( pszFile );

	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// CImageFile serialization

DWORD CImageFile::GetSerialSize() const
{
	/*if ( ! m_bLoaded ) return 4;
	int nPitch = m_nWidth * m_nComponents;
	while ( nPitch & 3 ) nPitch++;
	return 12 + nPitch * m_nHeight;*/

	if ( ! m_bLoaded ) return 4;
	return 12 + ( ( ( ( m_nWidth * m_nComponents ) + 3 ) & (-4) ) * m_nHeight );
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

		/*int nPitch = m_nWidth * m_nComponents;
		while ( nPitch & 3 ) nPitch++;

		ar.Write( m_pImage, nPitch * m_nHeight );*/

		ar.Write( m_pImage, ( ( ( m_nWidth * m_nComponents ) + 3) & (-4) ) * m_nHeight );
	}
	else
	{
		Clear();

		ar >> m_nWidth;
		if ( m_nWidth == 0 ) return;
		ar >> m_nHeight;
		ar >> m_nComponents;

		/*int nPitch = m_nWidth * m_nComponents;
		while ( nPitch & 3 ) nPitch++;

		m_pImage = new BYTE[ nPitch * m_nHeight ];
		ar.Read( m_pImage, nPitch * m_nHeight );*/

		int nPitch = ( ( ( m_nWidth * m_nComponents )+ 3 ) & (-4) ) * m_nHeight;

		m_pImage = new BYTE[ nPitch  ];
		ar.Read( m_pImage, nPitch );

		m_bLoaded = TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CImageFile image access operations

HBITMAP CImageFile::CreateBitmap(HDC hUseDC)
{
	if ( ! m_bLoaded ) return NULL;
	if ( m_nComponents != 3 ) return NULL;

	BITMAPINFO pInfo;
	ZeroMemory( &pInfo, sizeof(pInfo) );

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
		DWORD nPitch	= m_nWidth * 3;
		BYTE* pLine		= m_pImage;

		//while ( nPitch & 3 ) nPitch++;
		nPitch = ( nPitch + 3) & (-4);

		for ( int nY = m_nHeight ; nY ; nY-- )
		{
			/*
			BYTE bSwap, *pSwap = pLine;

			for ( int nX = m_nWidth ; nX ; nX--, pSwap += 3 )
			{
				bSwap = *pSwap;
				*pSwap = pSwap[2];
				pSwap[2] = bSwap;
			}*/

			__asm
			{
				mov edi, this
				mov esi, pLine
				mov ecx, [edi+asm_m_nWidth]
				loop1: mov eax, [esi]
				bswap eax
				ror eax, 8
				mov [esi], eax
				add esi, 3
				dec ecx
				jnz loop1
			}

			SetDIBits( hDC, hBitmap, nY - 1, 1, pLine, &pInfo, DIB_RGB_COLORS );

			/*
			pSwap = pLine;

			for ( nX = m_nWidth ; nX ; nX--, pSwap += 3 )
			{
				bSwap = *pSwap;
				*pSwap = pSwap[2];
				pSwap[2] = bSwap;
			}*/

			__asm
			{
				mov edi, this
				mov esi, pLine
				mov ecx, [edi+asm_m_nWidth]
				loop2: mov eax, [esi]
				bswap eax
				ror eax, 8
				mov [esi], eax
				add esi, 3
				dec ecx
				jnz loop2
			}

			pLine += nPitch;
		}
	}

	if ( hDC != hUseDC ) ReleaseDC( 0, hDC );

	return hBitmap;
}

/////////////////////////////////////////////////////////////////////////////
// CImageFile image modification operations

BOOL CImageFile::Resample(int nNewWidth, int nNewHeight)
{
	if ( ! m_bLoaded ) return FALSE;
	if ( m_nComponents != 3 ) return FALSE;
	if ( nNewWidth == m_nWidth && nNewHeight == m_nHeight ) return TRUE;

	DWORD nInPitch	= m_nWidth * 3;
	DWORD nOutPitch	= nNewWidth * 3;

	//while ( nOutPitch & 3 ) nOutPitch++;
	//while ( nInPitch & 3 ) nInPitch++;
	nOutPitch = ( nOutPitch + 3) & (-4);
	nInPitch =  ( nInPitch +  3) & (-4);

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
		int nCount = ( (nY+1) * m_nHeight / nNewHeight ) - nFirst + 1;

		if ( nFirst + nCount >= m_nHeight ) nCount = 1;

		BYTE* pRow = m_pImage + nInPitch * nFirst;
		pColPtr = pColInfo;

		for ( int nX = 0 ; nX < nNewWidth ; nX++, pColPtr++ )
		{
			BYTE* pIn = pRow + *pColPtr++;
/*
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
*/
			DWORD nPixels = *pColPtr * nCount;
			int nYY = nCount;
			__asm
			{
				mov esi, pIn
				xor eax, eax ;red
				xor ebx, ebx ;Green
				xor ecx, ecx
				xor edi, edi ;Blue
				loopYY: mov edx, pColPtr
				mov edx, [edx]
				loopXX: mov cl, [esi]
				add eax, ecx
				mov cl, [esi+1]
				add ebx, ecx
				mov cl, [esi+2]
				add edi, ecx
				add esi, 3
				dec edx
				jnz loopXX
				mov edx, pColPtr
				mov edx, [edx]
				lea edx, [edx+edx*2]
				add esi, nInPitch
				sub esi, edx
				dec nYY
				jnz loopYY
				xor edx, edx
				mov ecx, nPixels
				mov esi, pOut
				div ecx
				mov [esi], al
				xor edx, edx
				mov eax, ebx
				div ecx
				mov [esi+1], al
				xor edx, edx
				mov eax, edi
				div ecx
				mov [esi+2], al
				add esi, 3
				mov pOut, esi
			}
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

BOOL CImageFile::FastResample(int nNewWidth, int nNewHeight)
{
	if ( ! m_bLoaded ) return FALSE;
	if ( m_nComponents != 3 ) return FALSE;
	if ( nNewWidth == m_nWidth && nNewHeight == m_nHeight ) return TRUE;

	DWORD nInPitch	= m_nWidth * 3;
	DWORD nOutPitch	= nNewWidth * 3;

	//while ( nOutPitch & 3 ) nOutPitch++;
	//while ( nInPitch & 3 ) nInPitch++;
	nOutPitch = ( nOutPitch + 3) & (-4);
	nInPitch  = ( nInPitch  + 3) & (-4);

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
}

/////////////////////////////////////////////////////////////////////////////
// CImageFile image component modification

BOOL CImageFile::EnsureRGB(COLORREF crBack)
{
	if ( ! m_bLoaded || ! m_nWidth || ! m_nHeight )
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

	DWORD nInPitch	= m_nWidth * 1;
	DWORD nOutPitch	= m_nWidth * 3;

	//while ( nInPitch & 3 ) nInPitch++;
	//while ( nOutPitch & 3 ) nOutPitch++;
	nInPitch  = ( nInPitch  + 3) & (-4);
	nOutPitch = ( nOutPitch + 3) & (-4);

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

	DWORD nInPitch	= m_nWidth * 4;
	DWORD nOutPitch	= m_nWidth * 3;

	//while ( nInPitch & 3 ) nInPitch++;
	//while ( nOutPitch & 3 ) nOutPitch++;
	nInPitch =  ( nInPitch +  3) & (-4);
	nOutPitch = ( nOutPitch + 3) & (-4);

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

	//DWORD nPitch = m_nWidth * 3;
	//while ( nPitch & 3 ) nPitch++;
	DWORD nPitch = ( ( m_nWidth * 3 ) + 3) & (-4);

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
