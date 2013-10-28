//
// CtrlLibraryHeaderPanel.cpp
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
#include "Library.h"
#include "AlbumFolder.h"
#include "Schema.h"
#include "XML.h"

#include "ShellIcons.h"
#include "CoolInterface.h"
#include "Skin.h"

#include "CtrlLibraryFrame.h"
#include "CtrlLibraryHeaderPanel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CLibraryHeaderPanel, CWnd)
	//{{AFX_MSG_MAP(CLibraryHeaderPanel)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLibraryHeaderPanel construction

CLibraryHeaderPanel::CLibraryHeaderPanel()
{
}

CLibraryHeaderPanel::~CLibraryHeaderPanel()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryHeaderPanel operations

BOOL CLibraryHeaderPanel::Create(CWnd* pParentWnd)
{
	CRect rect( 0, 0, 0, 0 );
	return CWnd::CreateEx( WS_EX_CONTROLPARENT, NULL, _T("CLibraryHeaderPanel"), WS_CHILD,
		rect, pParentWnd, ID_LIBRARY_HEADER, NULL );
}

int CLibraryHeaderPanel::Update()
{
	CAlbumFolder* pFolder = GetSelectedAlbum();
	if ( pFolder == NULL || pFolder->m_pSchema == NULL ) return 0;

	m_nIcon32	= pFolder->m_pSchema->m_nIcon32;
	m_nIcon48	= pFolder->m_pSchema->m_nIcon48;

	m_sTitle	= pFolder->m_pSchema->m_sHeaderTitle;
	m_sSubtitle	= pFolder->m_pSchema->m_sHeaderSubtitle;

	if ( pFolder->GetParent() == NULL )
	{
		QWORD nTotalVolume;
		DWORD nTotalFiles;
		CString str;

		LibraryMaps.GetStatistics( &nTotalFiles, &nTotalVolume );
		str.Format( _T("%lu"), nTotalFiles );
		m_sSubtitle.Replace( _T("{totalFiles}"), str );
		str = Settings.SmartVolume( nTotalVolume, KiloBytes );
		m_sSubtitle.Replace( _T("{totalVolume}"), str );
	}

	pFolder->m_pSchema->ResolveTokens( m_sTitle, pFolder->m_pXML );
	pFolder->m_pSchema->ResolveTokens( m_sSubtitle, pFolder->m_pXML );

	if ( m_sTitle.IsEmpty() ) m_sTitle = pFolder->m_sName;

	m_pMetadata.Setup( pFolder->m_pSchema );
	m_pMetadata.Remove( pFolder->m_pSchema->GetFirstMemberName() );

	m_pMetadata.Combine( pFolder->m_pXML );

	m_pMetadata.CreateLinks();
	m_pMetadata.Clean( 54 );

	if ( m_pMetadata.GetCount() )
	{
		CClientDC dc( this );
		CFont* pFont = (CFont*)dc.SelectObject( &CoolInterface.m_fntNormal );
		m_nKeyWidth = m_nMetaWidth = 0;
		m_pMetadata.ComputeWidth( &dc, m_nKeyWidth, m_nMetaWidth );
		if ( m_nKeyWidth ) m_nKeyWidth += 8;
		m_nMetaWidth += m_nKeyWidth;
		dc.SelectObject( pFont );
	}

	if (m_hWnd) Invalidate();

	int nHeight = static_cast< int >( m_pMetadata.GetCount() * 12 + 8 );

	if ( pFolder->GetParent() )
	{
		nHeight = max( 64, nHeight );
	}
	else
	{
		nHeight = max( 56, nHeight );
	}

	return min( 80, nHeight );
}

void CLibraryHeaderPanel::OnSkinChange()
{
	if ( m_bmWatermark.m_hObject != NULL ) m_bmWatermark.DeleteObject();

	if ( HBITMAP hMark = Skin.GetWatermark( _T("CLibraryHeaderPanel") ) )
	{
		m_bmWatermark.Attach( hMark );
	}
	else if ( Skin.m_crBannerBack == RGB( 122, 161, 230 ) )
	{
		m_bmWatermark.LoadBitmap( IDB_BANNER_MARK );
	}
}

CAlbumFolder* CLibraryHeaderPanel::GetSelectedAlbum() const
{
	if (!m_hWnd) return Library.GetAlbumRoot();
	CLibraryFrame* pFrame = (CLibraryFrame*)GetOwner();
	ASSERT_KINDOF(CLibraryFrame, pFrame );

	CLibraryTreeItem* pItem	= pFrame->GetFolderSelection();
	if ( pItem == NULL ) return Library.GetAlbumRoot();
	if ( pItem->m_pSelNext != NULL ) return NULL;

	return pItem->m_pVirtual;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryHeaderPanel message handlers

int CLibraryHeaderPanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_szBuffer = CSize( 0, 0 );
	OnSkinChange();

	return 0;
}

void CLibraryHeaderPanel::OnDestroy()
{
	CWnd::OnDestroy();

	if ( m_bmBuffer.m_hObject != NULL )
	{
		m_dcBuffer.SelectObject( m_hBuffer );
		m_dcBuffer.DeleteDC();
		m_bmBuffer.DeleteObject();
	}
}

void CLibraryHeaderPanel::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize( nType, cx, cy );
	Invalidate();
}

void CLibraryHeaderPanel::OnPaint()
{
	CPaintDC dc( this );
	CRect rcClient;

	GetClientRect( &rcClient );
	if ( rcClient.IsRectEmpty() ) return;

	if ( rcClient.Width() > m_szBuffer.cx || rcClient.Height() > m_szBuffer.cy )
	{
		if ( m_bmBuffer.m_hObject != NULL )
		{
			m_dcBuffer.SelectObject( m_hBuffer );
			m_dcBuffer.DeleteDC();
			m_bmBuffer.DeleteObject();
		}

		m_szBuffer = rcClient.Size();
		m_bmBuffer.CreateCompatibleBitmap( &dc, m_szBuffer.cx, m_szBuffer.cy );
		m_dcBuffer.CreateCompatibleDC( &dc );
		m_hBuffer = (HBITMAP)m_dcBuffer.SelectObject( &m_bmBuffer )->m_hObject;
	}

	if ( ! CoolInterface.DrawWatermark( &m_dcBuffer, &rcClient, &m_bmWatermark, 0, 0 ) )
	{
		m_dcBuffer.FillSolidRect( &rcClient, Skin.m_crBannerBack );
	}

	DoPaint( &m_dcBuffer, rcClient );

	dc.BitBlt( rcClient.left, rcClient.top, rcClient.Width(),
		rcClient.Height(), &m_dcBuffer, 0, 0, SRCCOPY );
}

void CLibraryHeaderPanel::DoPaint(CDC* pDC, CRect& rcClient)
{
	CFont* pOldFont = pDC->GetCurrentFont();

	CPoint ptIcon( 8, ( rcClient.top + rcClient.bottom ) / 2 - 24 );

	if ( m_nIcon48 >= 0 )
	{
		ShellIcons.Draw( pDC, m_nIcon48, 48, ptIcon.x, ptIcon.y );
	}
	else if ( m_nIcon32 >= 0 )
	{
		ptIcon.x += 8; ptIcon.y += 8;
		ShellIcons.Draw( pDC, m_nIcon32, 32, ptIcon.x, ptIcon.y );
	}

	pDC->SetTextColor( Skin.m_crBannerText );
	pDC->SetBkMode( TRANSPARENT );

	CRect rcWork( &rcClient );
	rcWork.DeflateRect( 8, 4 );
	rcWork.left += 48 + 16;

	if ( m_pMetadata.GetCount() )
	{
		CRect rcMeta( &rcWork );
		rcMeta.left		= rcWork.right - m_nMetaWidth;
		rcWork.right	= rcMeta.left - 8;

		int nY = rcMeta.top;

		for ( POSITION pos = m_pMetadata.GetIterator() ; pos && nY + 12 < rcMeta.bottom ; nY += 12 )
		{
			CMetaItem* pItem = m_pMetadata.GetNext( pos );

			pDC->SelectObject( &CoolInterface.m_fntNormal );
			DrawText( pDC, rcMeta.left, nY, Settings.General.LanguageRTL ? ':' + pItem->m_sKey : pItem->m_sKey + ':' );

			if ( pItem->m_bLink ) pDC->SelectObject( &CoolInterface.m_fntUnder );
			DrawText( pDC, rcMeta.left + m_nKeyWidth, nY, pItem->m_sValue );

			pItem->SetRect( rcMeta.left + m_nKeyWidth, nY, rcMeta.right, nY + 12 );
		}
	}

	rcWork.DeflateRect( 0, 4 );

	pDC->SelectObject( &CoolInterface.m_fntCaption );
	Skin.DrawWrappedText( pDC, &rcWork, m_sTitle, NULL, FALSE );
	pDC->SelectObject( &CoolInterface.m_fntNormal );
	Skin.DrawWrappedText( pDC, &rcWork, m_sSubtitle, NULL, FALSE );

	pDC->SelectObject( pOldFont );
}

void CLibraryHeaderPanel::DrawText(CDC* pDC, int nX, int nY, LPCTSTR pszText)
{
	CSize sz = pDC->GetTextExtent( pszText, static_cast< int >( _tcslen( pszText ) ) );

	CRect rc( nX - 2, nY - 2, nX + sz.cx + 2, nY + sz.cy + 2 );

	UINT nOptions = ETO_CLIPPED | ( Settings.General.LanguageRTL ? ETO_RTLREADING : 0 );
	pDC->ExtTextOut( nX, nY, nOptions, &rc, pszText, static_cast< UINT >( _tcslen( pszText ) ), NULL );
}

BOOL CLibraryHeaderPanel::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if ( m_pMetadata.OnSetCursor( this ) ) return TRUE;

	return CWnd::OnSetCursor( pWnd, nHitTest, message );
}

void CLibraryHeaderPanel::OnLButtonUp(UINT nFlags, CPoint point)
{
	if ( CMetaItem* pItem = m_pMetadata.HitTest( point, TRUE ) )
	{
		CQuickLock oLock( Library.m_pSection );

		if ( CAlbumFolder* pFolder = pItem->GetLinkTarget() )
		{
			CLibraryFrame* pFrame = (CLibraryFrame*)GetOwner();
			ASSERT_KINDOF(CLibraryFrame, pFrame );
			pFrame->Display( pFolder );
		}
	}

	CWnd::OnLButtonUp( nFlags, point );
}
