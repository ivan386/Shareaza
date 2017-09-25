//
// CoolInterface.cpp
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
#include "Settings.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "XML.h"

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
	m_czBuffer = CSize( 0, 0 );

	// experimental values
	m_pNameMap.InitHashTable( 631 );
	m_pImageMap16.InitHashTable( 347 );
	m_pImageMap32.InitHashTable( 61 );
	m_pImageMap48.InitHashTable( 61 );
	m_pWindowIcons.InitHashTable( 61 );
}

CCoolInterface::~CCoolInterface()
{
	Clear();

	HICON hIcon;
	HWND hWnd;
	for( POSITION pos = m_pWindowIcons.GetStartPosition(); pos; )
	{
		m_pWindowIcons.GetNextAssoc( pos, hIcon, hWnd );
		VERIFY( DestroyIcon( hIcon ) );
	}
	m_pWindowIcons.RemoveAll();
}

void CCoolInterface::Load()
{
	CQuickLock oLock( m_pSection );

	CreateFonts();
	CalculateColours();
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface clear

void CCoolInterface::Clear()
{
	CQuickLock oLock( m_pSection );

	m_pNameMap.RemoveAll();

	m_pImageMap16.RemoveAll();
	if ( m_pImages16.m_hImageList )
	{
		m_pImages16.DeleteImageList();
	}

	m_pImageMap32.RemoveAll();
	if ( m_pImages32.m_hImageList )
	{
		m_pImages32.DeleteImageList();
	}

	m_pImageMap48.RemoveAll();
	if ( m_pImages48.m_hImageList )
	{
		m_pImages48.DeleteImageList();
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
	CQuickLock oLock( m_pSection );

	m_pNameMap.SetAt( pszName, nID );
}

UINT CCoolInterface::NameToID(LPCTSTR pszName) const
{
	if ( ! pszName || ! *pszName )
		return 0;

	CQuickLock oLock( m_pSection );

	UINT nID = 0;
	if ( m_pNameMap.Lookup( pszName, nID ) )
		return nID;

	return _tcstoul( pszName, NULL, 10 );
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface image management

int CCoolInterface::ImageForID(UINT nID, int nImageListType) const
{
	CQuickLock oLock( m_pSection );

	int nImage = -1;
	switch ( nImageListType )
	{
	case LVSIL_SMALL:
		return m_pImageMap16.Lookup( nID, nImage ) ? nImage : -1;
	case LVSIL_NORMAL:
		return m_pImageMap32.Lookup( nID, nImage ) ? nImage : -1;
	case LVSIL_BIG:
		return m_pImageMap48.Lookup( nID, nImage ) ? nImage : -1;
	}
	return -1;
}

void CCoolInterface::AddIcon(UINT nID, HICON hIcon, int nImageListType)
{
	CQuickLock oLock( m_pSection );

	VERIFY( ConfirmImageList() );

	switch ( nImageListType )
	{
	case LVSIL_SMALL:
		m_pImageMap16.SetAt( nID, m_pImages16.Add( hIcon ) );
		break;
	case LVSIL_NORMAL:
		m_pImageMap32.SetAt( nID, m_pImages32.Add( hIcon ) );
		break;
	case LVSIL_BIG:
		m_pImageMap48.SetAt( nID, m_pImages48.Add( hIcon ) );
		break;
	}
}

void CCoolInterface::CopyIcon(UINT nFromID, UINT nToID, int nImageListType)
{
	CQuickLock oLock( m_pSection );

	int nImage;
	switch ( nImageListType )
	{
	case LVSIL_SMALL:
		if ( m_pImageMap16.Lookup( nFromID, nImage ) )
			m_pImageMap16.SetAt( nToID, nImage );
		break;
	case LVSIL_NORMAL:
		if ( m_pImageMap32.Lookup( nFromID, nImage ) )
			m_pImageMap32.SetAt( nToID, nImage );
		break;
	case LVSIL_BIG:
		if ( m_pImageMap48.Lookup( nFromID, nImage ) )
			m_pImageMap48.SetAt( nToID, nImage );
		break;
	}
}

HICON CCoolInterface::ExtractIcon(UINT nID, BOOL bMirrored, int nImageListType)
{
	CQuickLock oLock( m_pSection );

	HICON hIcon = NULL;
	int nImage = ImageForID( nID, nImageListType );
	if ( nImage >= 0 )
	{
		switch ( nImageListType )
		{
		case LVSIL_SMALL:
			hIcon = m_pImages16.ExtractIcon( nImage );
			break;
		case LVSIL_NORMAL:
			hIcon = m_pImages32.ExtractIcon( nImage );
			break;
		case LVSIL_BIG:
			hIcon = m_pImages48.ExtractIcon( nImage );
			break;
		}
	}
	if ( hIcon == NULL )
	{
		int cx = 0;
		switch ( nImageListType )
		{
		case LVSIL_SMALL:
			cx = 16;
			break;
		case LVSIL_NORMAL:
			cx = 32;
			break;
		case LVSIL_BIG:
			cx = 48;
			break;
		}
		hIcon = (HICON)LoadImage( AfxGetResourceHandle(),
			MAKEINTRESOURCE( nID ), IMAGE_ICON, cx, cx, 0 );
		if ( hIcon )
			AddIcon( nID, hIcon, nImageListType );
#ifdef _DEBUG
		else
			theApp.Message( MSG_ERROR, _T("Failed to load icon %d (%dx%d)."), nID, cx, cx );
#endif // _DEBUG
	}
	if ( hIcon )
	{
		if ( bMirrored && nID != ID_HELP_ABOUT )
		{
			hIcon = CreateMirroredIcon( hIcon );
			ASSERT( hIcon != NULL );
		}
	}
	return hIcon;
}

int CCoolInterface::ExtractIconID(UINT nID, BOOL bMirrored, int nImageListType)
{
	CQuickLock oLock( m_pSection );

	DestroyIcon( ExtractIcon( nID, bMirrored, nImageListType ) );
	return ImageForID( nID, nImageListType );
}

void CCoolInterface::SetIcon(UINT nID, BOOL bMirrored, BOOL bBigIcon, CWnd* pWnd)
{
	CQuickLock oLock( m_pSection );

	HICON hIcon = ExtractIcon( nID, bMirrored, bBigIcon ? LVSIL_NORMAL : LVSIL_SMALL );
	if ( hIcon )
	{
		m_pWindowIcons.SetAt( hIcon, pWnd->GetSafeHwnd() );
		hIcon = pWnd->SetIcon( hIcon, bBigIcon );
		if ( hIcon )
		{
			VERIFY( m_pWindowIcons.RemoveKey( hIcon ) );
			VERIFY( DestroyIcon( hIcon ) );
		}
	}
}

void CCoolInterface::SetIcon(HICON hIcon, BOOL bMirrored, BOOL bBigIcon, CWnd* pWnd)
{
	CQuickLock oLock( m_pSection );

	if ( hIcon )
	{
		if ( bMirrored ) hIcon = CreateMirroredIcon( hIcon );
		m_pWindowIcons.SetAt( hIcon, pWnd->GetSafeHwnd() );
		hIcon = pWnd->SetIcon( hIcon, bBigIcon );
		if ( hIcon )
		{
			VERIFY( m_pWindowIcons.RemoveKey( hIcon ) );
			VERIFY( DestroyIcon( hIcon ) );
		}
	}
}

/*BOOL CCoolInterface::AddImagesFromToolbar(UINT nIDToolBar, COLORREF crBack)
{
	CQuickLock oLock( m_pSection );

	VERIFY( ConfirmImageList() );

	CBitmap pBmp;
	if ( ! pBmp.LoadBitmap( nIDToolBar ) ) return FALSE;
	int nBase = m_pImages.Add( &pBmp, crBack );
	pBmp.DeleteObject();

	if ( nBase < 0 ) return FALSE;

	BOOL bRet = FALSE;
	HRSRC hRsrc = FindResource( AfxGetResourceHandle(), MAKEINTRESOURCE(nIDToolBar), RT_TOOLBAR );
	if ( hRsrc )
	{
		HGLOBAL hGlobal = LoadResource( AfxGetResourceHandle(), hRsrc );
		if ( hGlobal )
		{
			TOOLBAR_RES* pData = (TOOLBAR_RES*)LockResource( hGlobal );
			if ( pData )
			{
				for ( WORD nItem = 0 ; nItem < pData->wItemCount ; nItem++ )
				{
					if ( pData->items()[ nItem ] != ID_SEPARATOR )
					{
						m_pImageMap.SetAt( pData->items()[ nItem ], nBase );
						nBase++;
					}
				}
				bRet = TRUE;
			}
			FreeResource( hGlobal );
		}
	}
	return bRet;
}*/

BOOL CCoolInterface::ConfirmImageList()
{
	return
		( m_pImages16.GetSafeHandle() ||
		  m_pImages16.Create( 16, 16, ILC_COLOR32|ILC_MASK, 16, 4 ) ||
		  m_pImages16.Create( 16, 16, ILC_COLOR24|ILC_MASK, 16, 4 ) ||
		  m_pImages16.Create( 16, 16, ILC_COLOR16|ILC_MASK, 16, 4 ) ) &&
		( m_pImages32.GetSafeHandle() ||
		  m_pImages32.Create( 32, 32, ILC_COLOR32|ILC_MASK, 16, 4 ) ||
		  m_pImages32.Create( 32, 32, ILC_COLOR24|ILC_MASK, 16, 4 ) ||
		  m_pImages32.Create( 32, 32, ILC_COLOR16|ILC_MASK, 16, 4 ) ) &&
		( m_pImages48.GetSafeHandle() ||
			m_pImages48.Create( 48, 48, ILC_COLOR32|ILC_MASK, 16, 4 ) ||
			m_pImages48.Create( 48, 48, ILC_COLOR24|ILC_MASK, 16, 4 ) ||
			m_pImages48.Create( 48, 48, ILC_COLOR16|ILC_MASK, 16, 4 ) );
}

void CCoolInterface::LoadIconsTo(CImageList& pImageList, const UINT nID[], BOOL bMirror, int nImageListType)
{
	int nCount = 0;
	for ( ; nID[ nCount ]; ++nCount );
	ASSERT( nCount != 0 );

	int cx = 16;
	switch ( nImageListType )
	{
	case LVSIL_NORMAL:
		cx = 32;
		break;
	case LVSIL_BIG:
		cx = 48;
		break;
	}

	if ( pImageList.GetSafeHandle() )
		VERIFY( pImageList.DeleteImageList() );

	VERIFY( pImageList.Create( cx, cx, ILC_COLOR32|ILC_MASK, nCount, 0 ) ||
		pImageList.Create( cx, cx, ILC_COLOR24|ILC_MASK, nCount, 0 ) ||
		pImageList.Create( cx, cx, ILC_COLOR16|ILC_MASK, nCount, 0 ) );

	for ( int i = 0; nID[ i ]; ++i )
	{
		HICON hIcon = CoolInterface.ExtractIcon( nID[ i ], bMirror, nImageListType );
		ASSERT( hIcon != NULL );
		VERIFY( pImageList.Add( hIcon ) != -1 );
		VERIFY( DestroyIcon( hIcon ) );
	}
}

void CCoolInterface::LoadProtocolIconsTo(CImageList& pImageList, BOOL bMirror, int nImageListType)
{
	LoadIconsTo( pImageList, protocolIDs, bMirror, nImageListType );
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface double buffer

CDC* CCoolInterface::GetBuffer(CDC& dcScreen, const CSize& szItem)
{
	CQuickLock oLock( m_pSection );

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
	if ( pDC == NULL || pRect == NULL || pMark == NULL || pMark->m_hObject == NULL )
		return FALSE;

	BITMAP pWatermark = {};
	if ( ! pMark->GetBitmap( &pWatermark ) ||
		! pWatermark.bmWidth ||
		! pWatermark.bmHeight ||
		! pWatermark.bmPlanes ||
		! pWatermark.bmBitsPixel )
		return FALSE;

	CDC dcMark;
	if ( ! dcMark.CreateCompatibleDC( pDC ) )
		return FALSE;

	if ( Settings.General.LanguageRTL )
		SetLayout( dcMark.m_hDC, LAYOUT_BITMAPORIENTATIONPRESERVED );

	CBitmap* pOldMark = (CBitmap*)dcMark.SelectObject( pMark );

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

/////////////////////////////////////////////////////////////////////////////
// CCoolInterface thumbnail painting

void CCoolInterface::DrawThumbnail(CDC* pDC, const CRect& rcThumb,
	BOOL bWaiting, BOOL bSelected, CBitmap& bmThumb, int nIcon48, int nIcon32,
	const CString& strLabel)
{
	CRect rcFrame( &rcThumb );
	rcFrame.DeflateRect( 1, 1, 1, 1 );

	if ( bmThumb.m_hObject )
	{
		BITMAP info = {};
		bmThumb.GetBitmap( &info );

		CDC dcMem;
		dcMem.CreateCompatibleDC( pDC );

		CBitmap* pOld = dcMem.SelectObject( &bmThumb );

		int cx;
		int cy;
		if ( rcFrame.Width() < info.bmWidth || rcFrame.Height() < info.bmHeight )
		{
			cx = rcFrame.Width();
			cy = rcFrame.Height();
		}
		else
		{
			cx = info.bmWidth;
			cy = info.bmHeight;
		}
		if ( info.bmWidth > info.bmHeight )
			cy = ( cx * info.bmHeight ) / info.bmWidth;
		else
			cx = ( cy * info.bmWidth ) / info.bmHeight;

		int x = rcFrame.left + ( rcFrame.Width() - cx ) / 2;
		int y = rcFrame.top + ( rcFrame.Height() - cy ) / 2;

		pDC->SetStretchBltMode( HALFTONE );
		pDC->StretchBlt( x, y, cx, cy, &dcMem, 0, 0, info.bmWidth, info.bmHeight, SRCCOPY );

		dcMem.SelectObject( pOld );

		pDC->ExcludeClipRect( x, y, x + cx, y + cy );
	}
	else
	{
		if ( int size = ( ( nIcon48 >= 0 ) ? 48 : ( ( nIcon32 >= 0 ) ? 32 : 0 ) ) )
		{
			int x = rcFrame.left + ( rcFrame.Width() - 2 - size ) / 2;
			int y = rcFrame.top + ( rcFrame.Height() - 2 - size ) / 2;

			ShellIcons.Draw( pDC, ( ( nIcon48 >= 0 ) ? nIcon48 : nIcon32 ),
				size, x, y, ( bWaiting ? m_crWindow : m_crBackNormal ), bSelected );

			pDC->ExcludeClipRect( x, y, x + size, y + size );
		}

		if ( ! strLabel.IsEmpty() )
		{
			CRect rcLabel( rcFrame.left + 2, rcFrame.top + ( rcFrame.Height() + 48 + 4 ) / 2,
				rcFrame.right - 2, rcFrame.bottom - 2 );

			CFont* pOldFont = pDC->SelectObject( bWaiting ? &m_fntBold : &m_fntUnder );
			pDC->SetBkColor( bWaiting ? m_crWindow : m_crBackNormal );
			pDC->SetTextColor( bWaiting ? m_crTextAlert : m_crTextLink );
			pDC->FillSolidRect( &rcLabel, ( bWaiting ? m_crWindow : m_crBackNormal ) );
			pDC->DrawText( strLabel, &rcLabel, DT_CENTER | DT_VCENTER | DT_WORDBREAK |
				DT_EDITCONTROL | DT_NOPREFIX | DT_END_ELLIPSIS );
			pDC->ExcludeClipRect( &rcLabel );
			pDC->SelectObject( pOldFont );
		}
	}

	pDC->FillSolidRect( &rcFrame, ( bWaiting ? m_crWindow : m_crBackNormal ) );
	pDC->Draw3dRect( &rcThumb, m_crMargin, m_crMargin );
	pDC->ExcludeClipRect( &rcThumb );

	if ( bSelected )
	{
		rcFrame.InflateRect( 2, 2 );
		pDC->Draw3dRect( &rcFrame, m_crBackCheck, m_crBackCheck );
		rcFrame.InflateRect( 1, 1 );
		pDC->Draw3dRect( &rcFrame, m_crHighlight, m_crHighlight );
		rcFrame.InflateRect( 1, 1 );
		pDC->Draw3dRect( &rcFrame, m_crBackCheck, m_crBackCheck );
		pDC->ExcludeClipRect( &rcFrame );
	}
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface fonts

void CCoolInterface::CreateFonts(LPCTSTR pszFace, int nSize)
{
	CQuickLock oLock( m_pSection );

	if ( ! pszFace ) pszFace = Settings.Fonts.DefaultFont;
	if ( ! nSize ) nSize = Settings.Fonts.FontSize;

	if ( m_fntNormal.m_hObject ) m_fntNormal.DeleteObject();
	if ( m_fntBold.m_hObject ) m_fntBold.DeleteObject();
	if ( m_fntUnder.m_hObject ) m_fntUnder.DeleteObject();
	if ( m_fntCaption.m_hObject ) m_fntCaption.DeleteObject();
	if ( m_fntItalic.m_hObject ) m_fntItalic.DeleteObject();
	if ( m_fntBoldItalic.m_hObject ) m_fntBoldItalic.DeleteObject();
	if ( m_fntNavBar.m_hObject ) m_fntNavBar.DeleteObject();
	if ( m_fntRichDefault.m_hObject ) m_fntRichDefault.DeleteObject();
	if ( m_fntRichHeading.m_hObject ) m_fntRichHeading.DeleteObject();

	m_fntNormal.CreateFont( -nSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, theApp.m_nFontQuality,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );

	m_fntBold.CreateFont( -nSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, theApp.m_nFontQuality,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );

	m_fntUnder.CreateFont( -nSize, 0, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, theApp.m_nFontQuality,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );

	m_fntCaption.CreateFont( -nSize - 2, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, theApp.m_nFontQuality,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );

	m_fntItalic.CreateFont( -nSize, 0, 0, 0, FW_NORMAL, TRUE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, theApp.m_nFontQuality,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );

	m_fntBoldItalic.CreateFont( -nSize, 0, 0, 0, FW_BOLD, TRUE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, theApp.m_nFontQuality,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );

	m_fntNavBar.CreateFont( -nSize - 2, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, theApp.m_nFontQuality,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );

	m_fntRichDefault.CreateFont( -nSize - 1, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, theApp.m_nFontQuality,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );

	m_fntRichHeading.CreateFont( -nSize - 6, 0, 0, 0, FW_EXTRABOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, theApp.m_nFontQuality,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );
}

//////////////////////////////////////////////////////////////////////
// CCoolInterface colours

void CCoolInterface::CalculateColours(BOOL bCustom)
{
	CQuickLock oLock( m_pSection );

	if ( ( m_bCustom = bCustom ) == FALSE )
	{
		m_crWindow		= GetSysColor( COLOR_WINDOW );
		m_crMidtone		= GetSysColor( COLOR_BTNFACE );
		m_crText		= GetSysColor( COLOR_WINDOWTEXT );
		m_crHiText		= GetSysColor( COLOR_HIGHLIGHTTEXT );
		m_crHiBorder	= GetSysColor( COLOR_HIGHLIGHT );
		m_crHighlight	= GetSysColor( COLOR_HIGHLIGHT );
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
	m_crTipWarnings			= RGB( 64, 64, 64 ); // Colour of warning messages (Grey)

	m_crTaskPanelBack		= RGB( 122, 161, 230 );
	m_crTaskBoxCaptionBack	= RGB( 250, 250, 255 );
	m_crTaskBoxPrimaryBack	= RGB( 30, 87, 199 );
	m_crTaskBoxCaptionText	= RGB( 34, 93, 217 );
	m_crTaskBoxPrimaryText	= RGB( 255, 255, 255 );
	m_crTaskBoxCaptionHover	= RGB( 84, 143, 255 );
	m_crTaskBoxClient		= RGB( 214, 223, 247 );

	m_crMediaWindow			= RGB( 0, 0, 0 );
	m_crMediaWindowText		= RGB( 200, 200, 255 );
	m_crMediaStatus			= RGB( 0, 0, 0x60 );
	m_crMediaStatusText		= RGB( 0xF0, 0xF0, 0xFF );
	m_crMediaPanel  		= RGB( 0, 0, 0x30 );
	m_crMediaPanelText  	= RGB( 255, 255, 255 );
	m_crMediaPanelActive  	= RGB( 128, 0, 0 );
	m_crMediaPanelActiveText = RGB( 255, 255, 255 );
	m_crMediaPanelCaption	= RGB( 0x00, 0x00, 0x80 );
	m_crMediaPanelCaptionText = RGB( 0xFF, 0xFF, 0 );

	m_crTrafficWindowBack	= RGB( 0, 0, 0 );
	m_crTrafficWindowText	= RGB( 193, 196, 255 );
	m_crTrafficWindowGrid	= RGB( 0, 0, 128 );

	m_crMonitorHistoryBack	= RGB( 0, 0, 0 );
	m_crMonitorHistoryBackMax = RGB( 80, 0, 0 );
	m_crMonitorHistoryText	= RGB( 255, 0, 0 );
	m_crMonitorDownloadLine	= RGB( 0, 0xFF, 0 );
	m_crMonitorUploadLine	= RGB( 0xFF, 0xFF, 0 );
	m_crMonitorDownloadBar	= RGB( 0, 0xBB, 0 );
	m_crMonitorUploadBar	= RGB( 0xBB, 0xBB, 0 );

	m_crRatingNull			= RGB( 0, 0, 0 );
	m_crRating0				= RGB( 255, 0, 0 );
	m_crRating1	 			= RGB( 128, 128, 128 );
	m_crRating2				= RGB( 80, 80, 80 );
	m_crRating3				= RGB( 0, 0, 0 );
	m_crRating4				= RGB( 0, 128, 0 );
	m_crRating5				= RGB( 0, 0, 255 );

	m_crRichdocBack			= RGB( 255, 255, 255 );
	m_crRichdocText  		= RGB( 0, 0, 0 );
	m_crRichdocHeading 		= RGB( 0x80, 0, 0 );
	m_crTextAlert 			= RGB( 255, 0, 0 );
	m_crTextStatus 			= RGB( 0, 128, 0 );
	m_crTextLink  			= RGB( 0, 0, 255 );
	m_crTextLinkHot			= RGB( 255, 0, 0 );

	m_crChatIn				= RGB( 0, 0, 255 );
	m_crChatOut				= RGB( 255, 0, 0 );
	m_crChatNull	  		= RGB( 128, 128, 128 );
	m_crSearchExists  		= RGB( 0, 127, 0 );
	m_crSearchExistsHit		= RGB( 0, 64, 0 );
	m_crSearchExistsSelected	= RGB( 0, 255, 0 );
	m_crSearchQueued 		= RGB( 0, 0, 160 );
	m_crSearchQueuedHit		= RGB( 0, 0, 100 );
	m_crSearchQueuedSelected	= GetSysColor( COLOR_HIGHLIGHTTEXT );
	m_crSearchGhostrated	= RGB( 200, 90, 0 );
	m_crSearchNull  		= GetSysColor( COLOR_3DSHADOW );
	m_crTransferSource		= RGB( 30, 30, 30 );
	m_crTransferRanges		= RGB( 220, 240, 220 );
	m_crTransferCompleted	= RGB( 0, 127, 0 );
	m_crTransferVerifyPass	= RGB( 0, 0, 127 );
	m_crTransferVerifyFail	= RGB( 255, 0, 0 );
	m_crTransferCompletedSelected	= RGB( 0, 255, 0 );
	m_crTransferVerifyPassSelected	= RGB( 0, 255, 0 );
	m_crTransferVerifyFailSelected	= RGB( 255, 0, 0 );
	m_crNetworkNull  		= RGB( 192, 192, 192 );
	m_crNetworkG1  			= RGB( 80, 80, 80 );
	m_crNetworkG2  			= RGB( 100, 100, 255 );
	m_crNetworkED2K  		= RGB( 128, 128, 0 );
	m_crNetworkDC  			= RGB( 0, 128, 0 );
	m_crNetworkUp			= RGB( 127, 0, 0 );
	m_crNetworkDown			= RGB( 0, 0, 127 );
	m_crSecurityAllow		= RGB( 0, 127, 0 );
	m_crSecurityDeny		= RGB( 255, 0, 0 );

	m_crDropdownText		= GetSysColor( COLOR_MENUTEXT );
	m_crDropdownBox			= GetSysColor( COLOR_WINDOW );
	m_crResizebarEdge		= GetSysColor( COLOR_BTNFACE );
	m_crResizebarFace		= GetSysColor( COLOR_BTNFACE );
	m_crResizebarShadow		= GetSysColor( COLOR_3DSHADOW );
	m_crResizebarHighlight	= GetSysColor( COLOR_3DHIGHLIGHT );
	m_crFragmentShaded		= GetSysColor( COLOR_BTNFACE );
	m_crFragmentComplete	= GetSysColor( COLOR_ACTIVECAPTION );
	m_crFragmentPass		= RGB( 0, 220, 0 );
	m_crFragmentFail		= RGB( 220, 0, 0 );
	m_crFragmentRequest		= RGB( 255, 255, 0 );
	m_crFragmentBorder		= RGB( 50, 50, 50 );
	m_crFragmentBorderSelected	= RGB( 50, 50, 50 );
	m_crFragmentBorderSimpleBar	= RGB( 50, 50, 50 );
	m_crFragmentBorderSimpleBarSelected	= RGB( 255, 255, 255 );

	m_crSysWindow			= GetSysColor( COLOR_WINDOW );
	m_crSysBtnFace			= GetSysColor( COLOR_BTNFACE );
	m_crSysBorders			= GetSysColor( COLOR_GRAYTEXT );
	m_crSys3DShadow 		= GetSysColor( COLOR_3DSHADOW );
	m_crSys3DHighlight		= GetSysColor( COLOR_3DHIGHLIGHT );
	m_crSysActiveCaption	= GetSysColor( COLOR_ACTIVECAPTION );
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


//////////////////////////////////////////////////////////////////////
// CCoolInterface windows XP+ themes

BOOL CCoolInterface::EnableTheme(CWnd* pWnd, BOOL bEnable)
{
	BOOL bResult = FALSE;

	if ( theApp.m_pfnSetWindowTheme )
	{
		if ( bEnable )
		{
			bResult = SUCCEEDED( theApp.m_pfnSetWindowTheme( pWnd->GetSafeHwnd(), NULL, NULL ) );
		}
		else
		{
			bResult = SUCCEEDED( theApp.m_pfnSetWindowTheme( pWnd->GetSafeHwnd(), L" ", L" " ) );
		}
	}

	return bResult;
}

int CCoolInterface::GetImageCount(int nImageListType)
{
	CQuickLock oLock( m_pSection );

	switch ( nImageListType )
	{
	case LVSIL_SMALL:
		return m_pImages16.GetImageCount();
	case LVSIL_NORMAL:
		return m_pImages32.GetImageCount();
	case LVSIL_BIG:
		return m_pImages48.GetImageCount();
	}
	return 0;
}

BOOL CCoolInterface::Add(CXMLElement* pBase, HBITMAP hbmImage, COLORREF crMask, int nImageListType)
{
	CQuickLock oLock( m_pSection );

	VERIFY( ConfirmImageList() );

	int nBase = 0;
	switch ( nImageListType )
	{
	case LVSIL_SMALL:
		nBase = m_pImages16.Add( CBitmap::FromHandle( hbmImage ), crMask );
		break;
	case LVSIL_NORMAL:
		nBase = m_pImages32.Add( CBitmap::FromHandle( hbmImage ), crMask );
		break;
	case LVSIL_BIG:
		nBase = m_pImages48.Add( CBitmap::FromHandle( hbmImage ), crMask );
		break;
	}
	if ( nBase < 0 )
	{
		return FALSE;
	}

	const static LPCTSTR pszNames[] = {
		_T("id"),  _T("id1"), _T("id2"), _T("id3"), _T("id4"), _T("id5"),
		_T("id6"), _T("id7"), _T("id8"), _T("id9"), NULL };
	int nIndex = 0;
	int nIndexRev = GetImageCount( nImageListType ) - 1;	// Total number of images
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );
		if ( ! pXML->IsNamed( _T("image") ) )
		{
			TRACE( "Unknown tag \"%s\" inside \"%s:%s\" in CCoolInterface::Add\r\n",
				(LPCSTR)CT2A( pXML->GetName() ), (LPCSTR)CT2A( pBase->GetName() ), (LPCSTR)CT2A( pBase->GetAttributeValue( _T("id") ) ) );
			continue;
		}

		CString strValue = pXML->GetAttributeValue( _T("index") );
		if ( strValue.GetLength() )
		{
			if ( _stscanf( strValue, _T("%i"), &nIndex ) != 1 )
			{
				TRACE( "Image \"%s\" has invalid index \"%s\" in CCoolInterface::Add\r\n",
					(LPCSTR)CT2A( pBase->GetAttributeValue( _T("id") ) ), (LPCSTR)CT2A( strValue ) );
				continue;
			}
		}

		nIndex += nBase;
		for ( int nName = 0 ; pszNames[ nName ] ; nName++ )
		{
			UINT nID = NameToID( pXML->GetAttributeValue( pszNames[ nName ] ) );
			if ( nID )
			{
				switch ( nImageListType )
				{
				case LVSIL_SMALL:
					m_pImageMap16.SetAt( nID, Settings.General.LanguageRTL ? nIndexRev : nIndex );
					break;
				case LVSIL_NORMAL:
					m_pImageMap32.SetAt( nID, Settings.General.LanguageRTL ? nIndexRev : nIndex );
					break;
				case LVSIL_BIG:
					m_pImageMap48.SetAt( nID, Settings.General.LanguageRTL ? nIndexRev : nIndex );
					break;
				}
			}
			if ( nName && ! nID ) break;
		}
		nIndexRev--;
		nIndex -= nBase;
		nIndex ++;
	}
	return TRUE;
}

CImageList* CCoolInterface::SetImageListTo(CListCtrl& pWnd, int nImageListType)
{
	CQuickLock oLock( m_pSection );

	switch ( nImageListType )
	{
	case LVSIL_SMALL:
		return pWnd.SetImageList( &m_pImages16, nImageListType );
	case LVSIL_NORMAL:
		return pWnd.SetImageList( &m_pImages32, nImageListType );
	case LVSIL_BIG:
		return pWnd.SetImageList( &m_pImages48, nImageListType );
	}
	return NULL;
}

BOOL CCoolInterface::Draw(CDC* pDC, int nImage, POINT pt, UINT nStyle, int nImageListType) const
{
	CQuickLock oLock( m_pSection );

	HIMAGELIST hList = NULL;
	switch ( nImageListType )
	{
	case LVSIL_SMALL:
		hList = m_pImages16.GetSafeHandle();
		break;
	case LVSIL_NORMAL:
		hList = m_pImages32.GetSafeHandle();
		break;
	case LVSIL_BIG:
		hList = m_pImages48.GetSafeHandle();
		break;
	}
	return ImageList_Draw( hList, nImage, pDC->GetSafeHdc(), pt.x, pt.y, nStyle );
}

BOOL CCoolInterface::DrawEx(CDC* pDC, int nImage, POINT pt, SIZE sz, COLORREF clrBk, COLORREF clrFg, UINT nStyle, int nImageListType) const
{
	CQuickLock oLock( m_pSection );

	HIMAGELIST hList = NULL;
	switch ( nImageListType )
	{
	case LVSIL_SMALL:
		hList = m_pImages16.GetSafeHandle();
		break;
	case LVSIL_NORMAL:
		hList = m_pImages32.GetSafeHandle();
		break;
	case LVSIL_BIG:
		hList = m_pImages48.GetSafeHandle();
		break;
	}
	return ImageList_DrawEx( hList, nImage, pDC->GetSafeHdc(),
		pt.x, pt.y, sz.cx, sz.cy, clrBk, clrFg, nStyle );
}

BOOL CCoolInterface::Draw(CDC* pDC, UINT nID, int nSize, int nX, int nY, COLORREF crBack, BOOL bSelected, BOOL bExclude) const
{
	CQuickLock oLock( m_pSection );

	HIMAGELIST hList;
	int nType;
	switch ( nSize )
	{
	case 16:
		hList = m_pImages16.GetSafeHandle();
		nType = LVSIL_SMALL;
		break;
	case 32:
		hList = m_pImages32.GetSafeHandle();
		nType = LVSIL_NORMAL;
		break;
	case 48:
		hList = m_pImages48.GetSafeHandle();
		nType = LVSIL_BIG;
		break;
	default:
		ASSERT( FALSE );
		return FALSE;
	}
	int nImage = ImageForID( nID, nType );
	if ( nImage == -1 )
		return FALSE;
	BOOL bRet = ImageList_DrawEx( hList, nImage, pDC->GetSafeHdc(),
		nX, nY, nSize, nSize, crBack, CLR_DEFAULT, bSelected ? ILD_SELECTED : ILD_NORMAL );
	if ( bExclude && crBack == CLR_NONE )
		pDC->ExcludeClipRect( nX, nY, nX + nSize, nY + nSize );
	return bRet;
}
