//
// CoolInterface.cpp
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
#include "Shareaza.h"
#include "CoolInterface.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CCoolInterface CoolInterface;


//////////////////////////////////////////////////////////////////////
// CCoolInterface construction

CCoolInterface::CCoolInterface()
{
	CreateFonts();
	CalculateColours();
	
	m_czBuffer = CSize( 0, 0 );
}

CCoolInterface::~CCoolInterface()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface clear

void CCoolInterface::Clear()
{
	m_pNameMap.RemoveAll();
	m_pImageMap.RemoveAll();
	
	if ( m_pImages.m_hImageList )
	{
		m_pImages.DeleteImageList();
	}
	
	if ( m_bmBuffer.m_hObject != NULL )
	{
		m_dcBuffer.SelectObject( CGdiObject::FromHandle( m_bmOldBuffer ) );
		m_dcBuffer.DeleteDC();
		m_bmBuffer.DeleteObject();
	}
	
	m_czBuffer = CSize( 0, 0 );
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface command management

void CCoolInterface::NameCommand(UINT nID, LPCTSTR pszName)
{
	m_pNameMap.SetAt( pszName, (LPVOID)nID );
}

UINT CCoolInterface::NameToID(LPCTSTR pszName)
{
	UINT nID = 0;
	if ( m_pNameMap.Lookup( pszName, (void*&)nID ) ) return nID;
	if ( _stscanf( pszName, _T("%lu"), &nID ) == 1 ) return nID;
	return 0;
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface image management

int CCoolInterface::ImageForID(UINT nID)
{
	int nImage;
	if ( m_pImageMap.Lookup( (LPVOID)nID, (void*&)nImage ) ) return nImage;
	return -1;
}

void CCoolInterface::AddIcon(UINT nID, HICON hIcon)
{
	ConfirmImageList();
	int nImage = m_pImages.Add( hIcon );
	m_pImageMap.SetAt( (LPVOID)nID, (LPVOID)nImage );
}

void CCoolInterface::CopyIcon(UINT nFromID, UINT nToID)
{
	int nImage;
	if ( m_pImageMap.Lookup( (LPVOID)nFromID, (void*&)nImage ) )
		m_pImageMap.SetAt( (LPVOID)nToID, (LPVOID)nImage );
}

HICON CCoolInterface::ExtractIcon(UINT nID)
{
	int nImage = ImageForID( nID );
	if ( nImage >= 0 ) return m_pImages.ExtractIcon( nImage );
	return NULL;
}

BOOL CCoolInterface::AddImagesFromToolbar(UINT nIDToolBar, COLORREF crBack)
{
	ConfirmImageList();
	
	CBitmap pBmp;
	if ( ! pBmp.LoadBitmap( nIDToolBar ) ) return FALSE;
	int nBase = m_pImages.Add( &pBmp, crBack );
	pBmp.DeleteObject();
	
	if ( nBase < 0 ) return FALSE;
	
	HRSRC hRsrc = FindResource( AfxGetInstanceHandle(), MAKEINTRESOURCE(nIDToolBar), RT_TOOLBAR );
	if ( hRsrc == NULL ) return FALSE;
	
	HGLOBAL hGlobal = LoadResource( AfxGetInstanceHandle(), hRsrc );
	if ( hGlobal == NULL ) return FALSE;
	
	TOOLBAR_RES* pData = (TOOLBAR_RES*)LockResource( hGlobal );
	
	if ( pData == NULL )
	{
		FreeResource( hGlobal );
		return FALSE;
	}
	
	for ( WORD nItem = 0 ; nItem < pData->wItemCount ; nItem++ )
	{
		if ( pData->items()[ nItem ] != ID_SEPARATOR )
		{
			m_pImageMap.SetAt( (LPVOID)(DWORD)pData->items()[ nItem ],
				(LPVOID)nBase );
			nBase++;
		}
	}
	
	UnlockResource( hGlobal );
	FreeResource( hGlobal );
	
	return TRUE;
}

BOOL CCoolInterface::ConfirmImageList()
{
	if ( m_pImages.m_hImageList != NULL ) return TRUE;
	
	if ( m_pImages.Create( 16, 16, ILC_COLOR16|ILC_MASK, 16, 4 ) ) return TRUE;
	else return m_pImages.Create( 16, 16, ILC_COLOR24|ILC_MASK, 16, 4 );
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface double buffer

CDC* CCoolInterface::GetBuffer(CDC& dcScreen, CSize& szItem)
{
	if ( szItem.cx <= m_czBuffer.cx && szItem.cy <= m_czBuffer.cy )
	{
		m_dcBuffer.SelectClipRgn( NULL );
		return &m_dcBuffer;
	}
	
	if ( m_bmBuffer.m_hObject )
	{
		m_dcBuffer.SelectObject( CGdiObject::FromHandle( m_bmOldBuffer ) );
		m_bmBuffer.DeleteObject();
	}
	
	m_czBuffer.cx = max( m_czBuffer.cx, szItem.cx );
	m_czBuffer.cy = max( m_czBuffer.cy, szItem.cy );
	
	if ( m_dcBuffer.m_hDC == NULL ) m_dcBuffer.CreateCompatibleDC( &dcScreen );
	m_bmBuffer.CreateCompatibleBitmap( &dcScreen, m_czBuffer.cx, m_czBuffer.cy );
	m_bmOldBuffer = (HBITMAP)m_dcBuffer.SelectObject( &m_bmBuffer )->GetSafeHandle();
	
	return &m_dcBuffer;
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface watermarking

BOOL CCoolInterface::DrawWatermark(CDC* pDC, CRect* pRect, CBitmap* pMark, int nOffX, int nOffY)
{
	BITMAP pWatermark;
	CBitmap* pOldMark;
	CDC dcMark;
	
	if ( pDC == NULL || pRect == NULL || pMark == NULL || pMark->m_hObject == NULL )
		return FALSE;
	
	dcMark.CreateCompatibleDC( pDC );
	pOldMark = (CBitmap*)dcMark.SelectObject( pMark );
	pMark->GetBitmap( &pWatermark );
	
	for ( int nY = pRect->top - nOffY ; nY < pRect->bottom ; nY += pWatermark.bmHeight )
	{
		if ( nY + pWatermark.bmHeight < pRect->top ) continue;
		
		for ( int nX = pRect->left - nOffX ; nX < pRect->right ; nX += pWatermark.bmWidth )
		{
			if ( nX + pWatermark.bmWidth < pRect->left ) continue;
			
			pDC->BitBlt( nX, nY, pWatermark.bmWidth, pWatermark.bmHeight, &dcMark, 0, 0, SRCCOPY );
		}
	}
	
	dcMark.SelectObject( pOldMark );
	dcMark.DeleteDC();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface fonts

void CCoolInterface::CreateFonts(LPCTSTR pszFace, int nSize)
{
	if ( ! pszFace ) pszFace = _T("Tahoma");
	if ( ! nSize ) nSize = 11;
	
	if ( m_fntNormal.m_hObject ) m_fntNormal.DeleteObject();
	if ( m_fntBold.m_hObject ) m_fntBold.DeleteObject();
	if ( m_fntUnder.m_hObject ) m_fntUnder.DeleteObject();
	if ( m_fntCaption.m_hObject ) m_fntCaption.DeleteObject();
	if ( m_fntItalic.m_hObject ) m_fntItalic.DeleteObject();
	
	m_fntNormal.CreateFont( -nSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );
	
	m_fntBold.CreateFont( -nSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );
	
	m_fntUnder.CreateFont( -nSize, 0, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );
	
	m_fntCaption.CreateFont( -nSize - 2, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );

	m_fntItalic.CreateFont( -nSize, 0, 0, 0, FW_NORMAL, TRUE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface colours

void CCoolInterface::CalculateColours(BOOL bCustom)
{
	if ( ! ( m_bCustom = bCustom ) )
	{
		m_crWindow		= GetSysColor( COLOR_WINDOW );
		m_crMidtone		= GetSysColor( COLOR_BTNFACE );
		m_crHighlight	= GetSysColor( COLOR_HIGHLIGHT );
		m_crText		= GetSysColor( COLOR_WINDOWTEXT );
		m_crHiText		= GetSysColor( COLOR_HIGHLIGHTTEXT );
	}
	
	m_crBackNormal			= CalculateColour( m_crMidtone, m_crWindow, 215 );
	m_crBackSel				= CalculateColour( m_crHighlight, m_crWindow, 178 );
	m_crBackCheck			= CalculateColour( m_crHighlight, m_crWindow, 200 );
	m_crBackCheckSel		= CalculateColour( m_crHighlight, m_crWindow, 127 );
	m_crMargin				= CalculateColour( m_crMidtone, m_crWindow, 39 );
	m_crShadow				= CalculateColour( m_crHighlight, GetSysColor( COLOR_3DSHADOW ), 200 );
	m_crBorder				= m_crHighlight;
	m_crCmdText				= GetSysColor( COLOR_MENUTEXT );
	m_crCmdTextSel			= GetSysColor( COLOR_MENUTEXT );
	m_crDisabled			= GetSysColor( COLOR_GRAYTEXT );
	
	m_crTipBack				= GetSysColor( COLOR_INFOBK );
	m_crTipText				= GetSysColor( COLOR_INFOTEXT );
	m_crTipBorder			= CalculateColour( m_crTipBack, (COLORREF)0, 100 );
	
	m_crTaskPanelBack		= RGB( 122, 161, 230 );
	m_crTaskBoxCaptionBack	= RGB( 250, 250, 255 );
	m_crTaskBoxPrimaryBack	= RGB( 30, 87, 199 );
	m_crTaskBoxCaptionText	= RGB( 34, 93, 217 );
	m_crTaskBoxPrimaryText	= RGB( 255, 255, 255 );
	m_crTaskBoxCaptionHover	= RGB( 84, 143, 255 );
	m_crTaskBoxClient		= RGB( 214, 223, 247 );
}

void CCoolInterface::OnSysColourChange()
{
	if ( ! m_bCustom ) CalculateColours();
}

COLORREF CCoolInterface::CalculateColour(COLORREF crFore, COLORREF crBack, int nAlpha)
{
	int nRed	= GetRValue( crFore ) * ( 255 - nAlpha ) / 255 + GetRValue( crBack ) * nAlpha / 255;
	int nGreen	= GetGValue( crFore ) * ( 255 - nAlpha ) / 255 + GetGValue( crBack ) * nAlpha / 255;
	int nBlue	= GetBValue( crFore ) * ( 255 - nAlpha ) / 255 + GetBValue( crBack ) * nAlpha / 255;

	return RGB( nRed, nGreen, nBlue );
}

COLORREF CCoolInterface::GetDialogBkColor()
{
	return CalculateColour( GetSysColor( COLOR_BTNFACE ), GetSysColor( COLOR_WINDOW ), 200 );
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface windows version check

BOOL CCoolInterface::IsNewWindows()
{
	OSVERSIONINFO pVersion;
	pVersion.dwOSVersionInfoSize = sizeof(pVersion);
	GetVersionEx( &pVersion );

	return pVersion.dwMajorVersion > 5 || ( pVersion.dwMajorVersion == 5 && pVersion.dwMinorVersion >= 1 );
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface windows XP+ themes

BOOL CCoolInterface::EnableTheme(CWnd* pWnd, BOOL bEnable)
{
	HINSTANCE hTheme = LoadLibrary( _T("UxTheme.dll") );
	if ( ! hTheme ) return FALSE;

	HRESULT (WINAPI *pfnSetWindowTheme)(HWND, LPCWSTR, LPCWSTR);

	(FARPROC&)pfnSetWindowTheme = GetProcAddress( hTheme, "SetWindowTheme" );

	BOOL bResult = FALSE;

	if ( pfnSetWindowTheme )
	{
		if ( bEnable )
		{
			bResult = SUCCEEDED( (*pfnSetWindowTheme)( pWnd->GetSafeHwnd(), NULL, NULL ) );
		}
		else
		{
			bResult = SUCCEEDED( (*pfnSetWindowTheme)( pWnd->GetSafeHwnd(), L" ", L" " ) );
		}
	}

	FreeLibrary( hTheme );

	return bResult;
}

