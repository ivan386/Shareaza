//
// Flags.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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
#include "Flags.h"
#include "ImageFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define IMAGELIST_FLAG_WIDTH	16
#define IMAGELIST_FLAG_HEIGHT	16

#define FLAG_WIDTH				18
#define FLAG_HEIGHT				18
#define REAL_FLAG_X				2
#define REAL_FLAG_Y				1
#define REAL_FLAG_WIDTH			16
#define REAL_FLAG_HEIGHT		11

CFlags Flags;

CFlags::CFlags()
{
}

CFlags::~CFlags()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CFlags load

BOOL CFlags::Load()
{
	Clear();

	m_pImage.Create( IMAGELIST_FLAG_WIDTH, IMAGELIST_FLAG_HEIGHT, ILC_COLOR32|ILC_MASK, 26 * 26, 8 ) ||
	m_pImage.Create( IMAGELIST_FLAG_WIDTH, IMAGELIST_FLAG_HEIGHT, ILC_COLOR24|ILC_MASK, 26 * 26, 8 ) ||
	m_pImage.Create( IMAGELIST_FLAG_WIDTH, IMAGELIST_FLAG_HEIGHT, ILC_COLOR16|ILC_MASK, 26 * 26, 8 );

	CImageFile pImage;
	if ( ! pImage.LoadFromFile( Settings.General.Path + _T("\\Data\\Flags.png") ) ||
		 ! pImage.EnsureRGB( GetSysColor( COLOR_WINDOW ) ) ||
		 ! pImage.SwapRGB() ||
		   pImage.m_nWidth != FLAG_WIDTH * 26 ||
		   pImage.m_nHeight != FLAG_HEIGHT * 26 )
	{
		return FALSE;
	}

	const COLORREF crBack = RGB( 0, 255, 0 );

	for ( int i = 0; i < 26; ++i )
	{
		for ( int j = 0; j < 26; ++j )
		{
			CRect rc( i * FLAG_WIDTH, j * FLAG_HEIGHT,
				( i + 1 ) * FLAG_WIDTH, ( j + 1 ) * FLAG_HEIGHT );
			AddFlag( &pImage, &rc, crBack );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CFlags add a flag

void CFlags::AddFlag(CImageFile* pImage, CRect* pRect, COLORREF crBack)
{
	DWORD nPitch = pImage->m_nWidth * pImage->m_nComponents;
	while ( nPitch & 3 ) nPitch++;

	BYTE* pSource = pImage->m_pImage;
	pSource += pRect->top * nPitch + pRect->left * pImage->m_nComponents;

	HDC hDC = GetDC( NULL );
	HDC hDCMem1 = CreateCompatibleDC( hDC ); // Create memory DC for the source
	HDC hDCMem2 = CreateCompatibleDC( hDC ); // Create memory DC for the destination
	CBitmap bmOriginal, bmMoved;
	CDC* pDC = CDC::FromHandle( hDC );
	bmOriginal.CreateCompatibleBitmap( pDC, FLAG_WIDTH, FLAG_HEIGHT );
	bmMoved.CreateCompatibleBitmap( pDC, IMAGELIST_FLAG_WIDTH, IMAGELIST_FLAG_HEIGHT );

	BITMAPINFOHEADER pInfo = {};
	pInfo.biSize		= sizeof(BITMAPINFOHEADER);
	pInfo.biWidth		= FLAG_WIDTH;
	pInfo.biHeight		= FLAG_HEIGHT;
	pInfo.biPlanes		= 1;
	pInfo.biBitCount	= 24;
	pInfo.biCompression	= BI_RGB;
	pInfo.biSizeImage	= FLAG_WIDTH * FLAG_HEIGHT * 3;

	for ( int nY = FLAG_HEIGHT - 1 ; nY >= 0 ; nY-- )
	{
		SetDIBits( hDCMem1, bmOriginal, nY, 1, pSource, (BITMAPINFO*)&pInfo, DIB_RGB_COLORS );
		pSource += nPitch;
	}

	HBITMAP hOld_bm1 = (HBITMAP)SelectObject( hDCMem1, bmOriginal.m_hObject );
	HBITMAP hOld_bm2 = (HBITMAP)SelectObject( hDCMem2, bmMoved.m_hObject );
	CDC* pDC2 = CDC::FromHandle( hDCMem2 );
	pDC2->SetBkMode( TRANSPARENT );
	pDC2->FillSolidRect( 0, 0, IMAGELIST_FLAG_WIDTH, IMAGELIST_FLAG_HEIGHT, crBack );

	if ( Settings.General.LanguageRTL )
		SetLayout( hDCMem2, LAYOUT_RTL );
	VERIFY( BitBlt( hDCMem2,
		( IMAGELIST_FLAG_WIDTH - REAL_FLAG_WIDTH ) / 2,
		( IMAGELIST_FLAG_HEIGHT - REAL_FLAG_HEIGHT ) / 2,
		REAL_FLAG_WIDTH,
		REAL_FLAG_HEIGHT,
		hDCMem1, REAL_FLAG_X, REAL_FLAG_Y, SRCCOPY ) );

	SelectObject( hDCMem1, hOld_bm1 );
	SelectObject( hDCMem2, hOld_bm2 );
	VERIFY( DeleteDC( hDCMem1 ) );
	VERIFY( DeleteDC( hDCMem2 ) );
	ReleaseDC( NULL, hDC );

	m_pImage.Add( &bmMoved, crBack );
	bmMoved.DeleteObject();
	bmOriginal.DeleteObject();
}

//////////////////////////////////////////////////////////////////////
// CFlags clear

void CFlags::Clear()
{
	if ( m_pImage.m_hImageList ) m_pImage.DeleteImageList();

}

int CFlags::GetCount() const
{
	return m_pImage.GetImageCount();
}

int CFlags::GetFlagIndex(const CString& sCountry) const
{
	if ( sCountry.GetLength() == 2 )
	{
		char nFirstLetter = (char)( sCountry[0] - 'A' );
		char nSecondLetter = (char)( sCountry[1] - 'A' );
		// Currently only the letters A-Z are in the flag matrix
		// but GeoIP can also return some combinations that aren't all letters (A1, A2, etc.)
		if ( nFirstLetter  >= 0 && nFirstLetter  <= 25 &&
			 nSecondLetter >= 0 && nSecondLetter <= 25 )
			return nFirstLetter * 26 + nSecondLetter;
	}
	return -1;
}

BOOL CFlags::Draw(int i, HDC hdcDst, int x, int y, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle)
{
	return ImageList_DrawEx( m_pImage, i, hdcDst, x, y,
		IMAGELIST_FLAG_WIDTH, IMAGELIST_FLAG_HEIGHT, rgbBk, rgbFg, fStyle );
}

HICON CFlags::ExtractIcon(int i)
{
	return m_pImage.ExtractIcon( i );
}
