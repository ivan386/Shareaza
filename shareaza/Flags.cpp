//
// Flags.cpp
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
#include "Flags.h"
#include "ImageServices.h"
#include "ImageFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

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
	m_pImage.Create( 18, 18, ILC_COLOR32|ILC_MASK, 26 * 26, 8 ) || 
		m_pImage.Create( 18, 18, ILC_COLOR24|ILC_MASK, 26 * 26, 8 ) ||
		m_pImage.Create( 18, 18, ILC_COLOR16|ILC_MASK, 26 * 26, 8 );

	CString strFile = Settings.General.Path + _T("\\Data\\Flags.png");

	CImageFile pImage;

	if (	! pImage.LoadFromFile( strFile ) ||
			! pImage.EnsureRGB( GetSysColor( COLOR_WINDOW ) ) ||
			! pImage.SwapRGB() || 
			pImage.m_nWidth != (18 * 26) ||
			pImage.m_nHeight != (12 * 26) )
	{
		return FALSE;
	}

	COLORREF crBack = RGB( 0, 255, 0 );
	
	for ( int i = 0; i < 26; i++ )
	{
		for ( int j = 0; j < 26; j++ )
		{
			CRect rc( 0, 0, 0, 0 );
			rc.left		= i * 18;
			rc.right	= (i + 1) * 18;
			rc.top		= j * 12;
			rc.bottom	= (j + 1) * 12;

			AddFlag( &pImage, &rc, crBack );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CFlags add a flag

void CFlags::AddFlag(CImageFile* pImage, CRect* pRect, COLORREF crBack)
{
	ASSERT( pImage->m_bLoaded && pImage->m_nComponents == 3 );

	if ( pRect->left < 0 || pRect->left + 18 > pImage->m_nWidth ) return;
	if ( pRect->top < 0 || pRect->top > pImage->m_nHeight + 12 ) return;
	if ( pRect->right != pRect->left + 18 ) return;
	if ( pRect->bottom != pRect->top + 12 ) return;

	DWORD nPitch = pImage->m_nWidth * pImage->m_nComponents;
	while ( nPitch & 3 ) nPitch++;

	BYTE* pSource = pImage->m_pImage;
	pSource += pRect->top * nPitch + pRect->left * pImage->m_nComponents;

	HDC hDCMem1, hDCMem2;
	if ( HDC hDC = GetDC( NULL ) ) // Get screen DC
	{
		hDCMem1 = CreateCompatibleDC( hDC ); // Create memory DC for the source
		if ( !hDCMem1 ) 
		{
			ReleaseDC( NULL, hDC );
			return;
		}

		hDCMem2 = CreateCompatibleDC( hDC ); // Create memory DC for the destination
		if ( !hDCMem2 )
		{
			DeleteDC( hDCMem1 );
			ReleaseDC( NULL, hDC );
			return;
		}

		CBitmap bmOriginal, bmMoved;
		CDC* pDC = CDC::FromHandle( hDC );

		if ( !bmOriginal.CreateCompatibleBitmap( pDC, 18, 12 ) ) // Source bitmap
		{
			ReleaseDC( NULL, hDC );
			DeleteDC( hDCMem1 );
			DeleteDC( hDCMem2 );
			return;
		}

		if ( !bmMoved.CreateCompatibleBitmap( pDC, 18, 18 ) ) // Destination bitmap
		{
			ReleaseDC( NULL, hDC );
			DeleteDC( hDCMem1 );
			DeleteDC( hDCMem2 );
			bmOriginal.DeleteObject();
			return;
		}

		BITMAPINFOHEADER pInfo;
		pInfo.biSize		= sizeof(BITMAPINFOHEADER);
		pInfo.biWidth		= 18;
		pInfo.biHeight		= 12;
		pInfo.biPlanes		= 1;
		pInfo.biBitCount	= 24;
		pInfo.biCompression	= BI_RGB;
		pInfo.biSizeImage	= 18 * 12 * 3;

		for ( int nY = 11 ; nY >= 0 ; nY-- )
		{
			SetDIBits( hDCMem1, bmOriginal, nY, 1, pSource, (BITMAPINFO*)&pInfo, DIB_RGB_COLORS );
			pSource += nPitch;
		}

		HBITMAP hOld_bm1, hOld_bm2;
		hOld_bm1 = (HBITMAP)SelectObject( hDCMem1, bmOriginal.m_hObject );
		hOld_bm2 = (HBITMAP)SelectObject( hDCMem2, bmMoved.m_hObject );
		CDC* pDC2 = CDC::FromHandle( hDCMem2 );
		pDC2->SetBkMode( TRANSPARENT );
		pDC2->FillSolidRect( 0, 0, 18, 18, crBack );

		if ( theApp.m_bRTL )
			theApp.m_pfnSetLayout( hDCMem2, LAYOUT_RTL );
		StretchBlt( hDCMem2, 0, 3, 18, 12, hDCMem1, 0, 0, 18, 12, SRCCOPY );

		SelectObject( hDCMem1, hOld_bm1 );
		SelectObject( hDCMem2, hOld_bm2 );
		DeleteDC( hDCMem1 );
		DeleteDC( hDCMem2 );
		ReleaseDC( NULL, hDC );
		m_pImage.Add( &bmMoved, crBack );
		bmMoved.DeleteObject();
		bmOriginal.DeleteObject();
	}
}

//////////////////////////////////////////////////////////////////////
// CFlags clear

void CFlags::Clear()
{
	if ( m_pImage.m_hImageList != NULL ) m_pImage.DeleteImageList();

}

int CFlags::GetFlagIndex(CString sCountry)
{
	if ( sCountry.GetLength() == 2 )
	{
		char nFirstLetter = sCountry[0] - 65;
		char nSecondLetter = sCountry[1] - 65;
		return nFirstLetter * 26 + nSecondLetter;
	}
	return -1;
}
