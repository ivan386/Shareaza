//
// CtrlTaskPanel.cpp
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
#include "CtrlTaskPanel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CTaskPanel, CWnd)

BEGIN_MESSAGE_MAP(CTaskPanel, CWnd)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CTaskBox, CButton)

BEGIN_MESSAGE_MAP(CTaskBox, CButton)
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_NCHITTEST()
	ON_WM_NCACTIVATE()
	ON_WM_PAINT()
	ON_WM_SYSCOMMAND()
	ON_WM_SETCURSOR()
	ON_WM_NCLBUTTONUP()
	ON_WM_TIMER()
	ON_WM_NCLBUTTONDOWN()
END_MESSAGE_MAP()

#define CAPTION_HEIGHT	25


/////////////////////////////////////////////////////////////////////////////
// CTaskPanel construction

CTaskPanel::CTaskPanel()
	: m_pStretch	( NULL )
	, m_nMargin		( 12 )
	, m_nCurve		( 2 )
	, m_bLayout		( FALSE )
{
}

/////////////////////////////////////////////////////////////////////////////
// CTaskPanel create

BOOL CTaskPanel::Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CreateEx( WS_EX_CONTROLPARENT, NULL, lpszWindowName,
		dwStyle | WS_CHILD | WS_CLIPCHILDREN, rect, pParentWnd, nID, NULL );
}

/////////////////////////////////////////////////////////////////////////////
// CTaskPanel box list management

CTaskBox* CTaskPanel::AddBox(CTaskBox* pBox, POSITION posBefore)
{
	ASSERT ( pBox != NULL );
	if ( posBefore )
	{
		m_pBoxes.InsertBefore( posBefore, pBox );
	}
	else
	{
		m_pBoxes.AddTail( pBox );
	}

	OnChanged();

	return pBox;
}

POSITION CTaskPanel::GetBoxIterator() const
{
	return m_pBoxes.GetHeadPosition();
}

CTaskBox* CTaskPanel::GetNextBox(POSITION& pos) const
{
	return m_pBoxes.GetNext( pos );
}

INT_PTR CTaskPanel::GetBoxCount() const
{
	return m_pBoxes.GetCount();
}

void CTaskPanel::RemoveBox(CTaskBox* pBox)
{
	POSITION pos = m_pBoxes.Find( pBox );

	if ( pos )
	{
		m_pBoxes.RemoveAt( pos );
		if ( m_pStretch == pBox ) m_pStretch = NULL;
		OnChanged();
	}
}

void CTaskPanel::ClearBoxes(BOOL bDelete)
{
	if ( bDelete )
	{
		for ( POSITION pos = GetBoxIterator() ; pos ; ) delete GetNextBox( pos );
	}

	m_pBoxes.RemoveAll();
	m_pStretch = NULL;

	OnChanged();
}

void CTaskPanel::SetStretchBox(CTaskBox* pBox)
{
	m_pStretch = pBox;
}

void CTaskPanel::SetMargin(int nMargin, int nCurve)
{
	m_nMargin	= nMargin;
	m_nCurve	= nCurve;
}

void CTaskPanel::SetWatermark(LPCTSTR szWatermark)
{
	m_sWatermark = szWatermark;
}

void CTaskPanel::SetFooter(LPCTSTR szWatermark)
{
	m_sFooter = szWatermark;
}

/////////////////////////////////////////////////////////////////////////////
// CTaskPanel message handlers

int CTaskPanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	SetOwner( GetParent() );
	return 0;
}

void CTaskPanel::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize( nType, cx, cy );

	if ( m_pStretch != NULL && m_pStretch->GetOuterHeight() )
	{
		m_bLayout = TRUE;
		Invalidate();
	}
}

BOOL CTaskPanel::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CTaskPanel::OnPaint()
{
	CPaintDC dc( this );
	CRect rc;

	GetClientRect( &rc );

	if ( m_bLayout ) Layout( rc );

	HBITMAP hbmFooter = Skin.GetWatermark( m_sFooter, TRUE );
	if ( ! hbmFooter && CoolInterface.m_crTaskPanelBack == RGB( 122, 161, 230 ) )
	{
		hbmFooter = Skin.LoadBitmap( IDB_TASKPANEL_FOOTER, TRUE );
	}
	if ( hbmFooter )
	{
		BITMAP pInfo = {};
		if ( GetObject( hbmFooter, sizeof( BITMAP ), &pInfo ) )
		{
			CRect rcFooter( &rc );
			rc.bottom = rcFooter.top = rcFooter.bottom - pInfo.bmHeight;

			CoolInterface.DrawWatermark( &dc, &rcFooter, CBitmap::FromHandle( hbmFooter ) );
		}
	}

	HBITMAP hbmWatermark = Skin.GetWatermark( m_sWatermark, TRUE );
	if ( ! hbmWatermark || ! CoolInterface.DrawWatermark( &dc, &rc, CBitmap::FromHandle( hbmWatermark ) ) )
	{
		// No watermark

		dc.FillSolidRect( &rc, CoolInterface.m_crTaskPanelBack );
		CRect rcShadow( rc );
		rcShadow.bottom = rcShadow.top + 1;
		dc.FillSolidRect( &rcShadow, RGB( 92, 131, 200 ) ); // The top pixel row from BannerMark.bmp

		GetWindowRect( &rc );
		ScreenToClient( &rc );

		CRect rcVertLine( rc );
		rcVertLine.left = rcVertLine.right - 1;
		dc.FillSolidRect( &rcVertLine, RGB( 92, 131, 200 ) );
	}
}

void CTaskPanel::OnChanged()
{
	m_bLayout = TRUE;
	if ( m_hWnd ) Invalidate();
}

void CTaskPanel::Layout(CRect& rcClient)
{
	CRect rcBox( &rcClient );

	rcBox.DeflateRect( m_nMargin, m_nMargin );

	int nStretch = rcBox.Height();

	if ( m_pStretch && m_pStretch->GetOuterHeight() )
	{
		for ( POSITION pos = GetBoxIterator() ; pos ; )
		{
			CTaskBox* pBox = GetNextBox( pos );

			if ( pBox->m_bVisible && pBox->m_nHeight && pBox != m_pStretch )
			{
				nStretch -= pBox->GetOuterHeight() + m_nMargin;
			}
		}
	}

	// Prevent stretch boxes from having negative height
	nStretch = max( nStretch, CAPTION_HEIGHT * 2 );

	for ( POSITION pos = GetBoxIterator() ; pos ; )
	{
		CTaskBox* pBox = GetNextBox( pos );

		int nHeight = pBox->GetOuterHeight();

		if ( nHeight )
		{
			if ( pBox == m_pStretch && pBox->m_bOpen ) nHeight = nStretch;

			rcBox.bottom = rcBox.top + nHeight;

			pBox->SetWindowPos( NULL, rcBox.left, rcBox.top, rcBox.Width(), rcBox.Height(),
				SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOACTIVATE );

			for( CWnd* pChild = pBox->GetWindow( GW_CHILD ); pChild;
				pChild = pChild->GetWindow( GW_HWNDNEXT ) )
			{
				pChild->ShowWindow( pBox->m_bOpen ? SW_SHOW : SW_HIDE );
			}

			rcBox.OffsetRect( 0, nHeight + m_nMargin );
		}
		else if ( pBox->IsWindowVisible() )
		{
			pBox->ShowWindow( SW_HIDE );
		}
	}

	m_bLayout = FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CTaskBox construction

CTaskBox::CTaskBox()
	: m_pPanel			( NULL )
	, m_nHeight			( 0 )
	, m_bVisible		( TRUE )
	, m_bOpen			( TRUE )
	, m_bHover			( FALSE )
	, m_bPrimary		( FALSE )
	, m_hIcon			( NULL )
{
}

CTaskBox::~CTaskBox()
{
	if ( m_hIcon ) DestroyIcon( m_hIcon );
}

/////////////////////////////////////////////////////////////////////////////
// CTaskBox operations

BOOL CTaskBox::Create(CTaskPanel* pPanel, int nHeight, LPCTSTR pszCaption, UINT nIDIcon, UINT nID)
{
	CRect rect( 0, 0, 0, 0 );

	m_pPanel	= pPanel;
	m_nHeight	= nHeight;

	if ( pPanel->m_hWnd )
	{
		pPanel->GetClientRect( &rect );
		rect.DeflateRect( pPanel->m_nMargin, 0 );
		rect.bottom = 0;
	}

	if ( ! CreateEx( WS_EX_CONTROLPARENT, NULL, pszCaption,
		WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, rect, pPanel, nID ) ) return FALSE;

	if ( nIDIcon )
	{
		CoolInterface.SetIcon( nIDIcon, Settings.General.LanguageRTL, FALSE, this );
	}

	CString strKey;
	strKey.Format( _T("%s.Open"), (LPCTSTR)CString( GetRuntimeClass()->m_lpszClassName ) );
	m_bOpen = theApp.GetProfileInt( _T("Interface"), strKey, TRUE );

	return TRUE;
}

CTaskPanel* CTaskBox::GetPanel() const
{
	ASSERT( m_pPanel != NULL );
	ASSERT_KINDOF( CTaskPanel, m_pPanel );
	return m_pPanel;
}

void CTaskBox::SetCaption(LPCTSTR pszCaption)
{
	CString strOld;
	GetWindowText( strOld );

	if ( strOld != pszCaption )
	{
		SetWindowText( pszCaption );
		InvalidateNonclient();
	}
}

void CTaskBox::SetIcon(HICON hIcon)
{
	if ( m_hIcon && m_hIcon != hIcon )
		DestroyIcon( m_hIcon );

	m_hIcon = hIcon;

	CWnd::SetIcon( m_hIcon, FALSE );
	InvalidateNonclient();
}

void CTaskBox::SetSize(int nHeight)
{
	if ( m_nHeight == nHeight ) return;
	m_nHeight = nHeight;
	if ( m_pPanel ) m_pPanel->OnChanged();
}

void CTaskBox::SetPrimary(BOOL bPrimary)
{
	if ( m_bPrimary == bPrimary ) return;
	m_bPrimary = bPrimary;
	if ( m_pPanel ) m_pPanel->OnChanged();
}

void CTaskBox::SetWatermark(LPCTSTR szWatermark)
{
	m_sWatermark = szWatermark;
}

void CTaskBox::SetCaptionmark(LPCTSTR szWatermark)
{
	m_sCaptionmark = szWatermark;
}

void CTaskBox::Expand(BOOL bOpen)
{
	if ( m_bOpen == bOpen ) return;
	m_bOpen = bOpen;

	if ( m_pPanel != NULL )
	{
		m_pPanel->OnChanged();
		OnExpanded( m_bOpen );
	}

	CString strKey;
	strKey.Format( _T("%s.Open"), (LPCTSTR)CString( GetRuntimeClass()->m_lpszClassName ) );
	theApp.WriteProfileInt( _T("Interface"), strKey, m_bOpen );

	InvalidateNonclient();
}

void CTaskBox::OnExpanded(BOOL /*bOpen*/)
{
}

int CTaskBox::GetOuterHeight() const
{
	if ( ! m_bVisible || ! m_nHeight ) return 0;
	return CAPTION_HEIGHT + ( m_bOpen ? m_nHeight + 1 : 0 );
}

/////////////////////////////////////////////////////////////////////////////
// CTaskBox message handlers

void CTaskBox::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	NCCALCSIZE_PARAMS* pSize = (NCCALCSIZE_PARAMS*)lpncsp;

	pSize->rgrc[0].left ++;
	pSize->rgrc[0].top += CAPTION_HEIGHT;
	pSize->rgrc[0].right --;
	pSize->rgrc[0].bottom --;
}

LRESULT CTaskBox::OnNcHitTest(CPoint point)
{
	CRect rc;
	GetWindowRect( &rc );

	if ( rc.PtInRect( point ) )
	{
		if ( point.y < rc.top + CAPTION_HEIGHT ) return HTCAPTION;
		return HTCLIENT;
	}

	return HTNOWHERE;
}

BOOL CTaskBox::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if ( nHitTest == HTCAPTION )
	{
		if ( ! m_bHover )
		{
			m_bHover = TRUE;
			PaintBorders();
			SetTimer( 1, 50, NULL );
		}

		::SetCursor( theApp.LoadCursor( IDC_HAND ) );
		return TRUE;
	}

	return CButton::OnSetCursor(pWnd, nHitTest, message);
}

void CTaskBox::OnNcLButtonDown(UINT /*nHitTest*/, CPoint /*point*/)
{
}

void CTaskBox::OnNcLButtonUp(UINT nHitTest, CPoint /*point*/)
{
	if ( nHitTest == HTCAPTION ) Expand( ! m_bOpen );
}

void CTaskBox::OnTimer(UINT_PTR /*nIDEvent*/)
{
	CPoint point;
	GetCursorPos( &point );

	if ( OnNcHitTest( point ) != HTCAPTION )
	{
		KillTimer( 1 );
		m_bHover = FALSE;
		PaintBorders();
	}
}

BOOL CTaskBox::OnNcActivate(BOOL /*bActive*/)
{
	PaintBorders();
	return TRUE;
}

void CTaskBox::OnNcPaint()
{
	PaintBorders();
}

void CTaskBox::PaintBorders()
{
	CWindowDC dc( this );
	CString strCaption;
	CRect rc, rcc;

	GetWindowRect( &rc );
	rc.OffsetRect( -rc.left, -rc.top );

	BOOL bCaptionCurve = TRUE;
	HBITMAP hbmCaptionmark = Skin.GetWatermark( m_sCaptionmark, TRUE );
	if ( ! hbmCaptionmark && m_bPrimary && CoolInterface.m_crTaskBoxPrimaryBack == RGB( 30, 87, 199 ) )
	{
		hbmCaptionmark = Skin.LoadBitmap( IDB_TASKBOX_CAPTION, TRUE );
	}
	if ( hbmCaptionmark )
	{
		BITMAP pInfo = {};
		if ( GetObject( hbmCaptionmark, sizeof( BITMAP ), &pInfo ) )
		{
			bCaptionCurve = pInfo.bmWidth < 176;
		}
	}

	if ( m_pPanel->m_nCurve != 0 && bCaptionCurve )
	{
		dc.SetPixel( 0, 0, CoolInterface.m_crTaskPanelBack );
		dc.SetPixel( 1, 0, CoolInterface.m_crTaskPanelBack );
		dc.SetPixel( 0, 1, CoolInterface.m_crTaskPanelBack );
		dc.SetPixel( rc.right - 1, 0, CoolInterface.m_crTaskPanelBack );
		dc.SetPixel( rc.right - 2, 0, CoolInterface.m_crTaskPanelBack );
		dc.SetPixel( rc.right - 1, 1, CoolInterface.m_crTaskPanelBack );

		dc.ExcludeClipRect( 0, 0, 2, 1 );
		dc.ExcludeClipRect( 0, 1, 1, 2 );
		dc.ExcludeClipRect( rc.right - 2, 0, rc.right, 1 );
		dc.ExcludeClipRect( rc.right - 1, 1, rc.right, 2 );
	}

	rcc.SetRect( 0, 0, rc.right, CAPTION_HEIGHT );

	CSize size= rcc.Size();
	CDC* pBuffer = CoolInterface.GetBuffer( dc, size );

	if ( ! hbmCaptionmark || ! CoolInterface.DrawWatermark( pBuffer, &rcc, CBitmap::FromHandle( hbmCaptionmark ) ) )
	{
		// No watermark
		pBuffer->FillSolidRect( &rcc, m_bPrimary ? CoolInterface.m_crTaskBoxPrimaryBack : CoolInterface.m_crTaskBoxCaptionBack );
	}

	CPoint ptIcon( 6, rcc.Height() / 2 - 7 );

	DrawIconEx( pBuffer->GetSafeHdc(), ptIcon.x, ptIcon.y, CWnd::GetIcon( FALSE ),
		16, 16, 0, NULL, DI_NORMAL );

	GetWindowText( strCaption );

	CFont* pOldFont	= (CFont*)pBuffer->SelectObject( &CoolInterface.m_fntBold );
	CSize sz		= pBuffer->GetTextExtent( strCaption );

	pBuffer->SetBkMode( TRANSPARENT );
	pBuffer->SetTextColor( m_bHover ? CoolInterface.m_crTaskBoxCaptionHover :
		( m_bPrimary ? CoolInterface.m_crTaskBoxPrimaryText : CoolInterface.m_crTaskBoxCaptionText ) );

	pBuffer->ExtTextOut( ptIcon.x * 2 + 16 + 1, rcc.Height() / 2 - sz.cy / 2,
		ETO_CLIPPED, &rcc, strCaption, NULL );

	pBuffer->SelectObject( pOldFont );

	dc.BitBlt( rc.left, rc.top, rcc.Width(), rcc.Height(), pBuffer, 0, 0, SRCCOPY );

	dc.ExcludeClipRect( &rcc );

	if ( m_bOpen )
	{
		rc.top = rcc.bottom - 1;
		dc.Draw3dRect( &rc, CoolInterface.m_crTaskBoxCaptionBack,
			CoolInterface.m_crTaskBoxCaptionBack );
	}

	dc.SelectStockObject( SYSTEM_FONT ); // GDI font leak fix
}

void CTaskBox::InvalidateNonclient()
{
	if ( m_hWnd )
	{
		CRect rc;
		GetWindowRect( &rc );
		rc.bottom = rc.top + CAPTION_HEIGHT;
		ScreenToClient( &rc );
		RedrawWindow( &rc, NULL, RDW_FRAME|RDW_INVALIDATE );
	}
}

void CTaskBox::OnPaint()
{
	CPaintDC dc( this );
	CRect rc;

	GetClientRect( &rc );

	HBITMAP hbmWatermark = Skin.GetWatermark( m_sWatermark, TRUE );
	if ( ! hbmWatermark || ! CoolInterface.DrawWatermark( &dc, &rc, CBitmap::FromHandle( hbmWatermark ) ) )
	{
		// No watermark
		dc.FillSolidRect( &rc, CoolInterface.m_crTaskBoxClient );
	}
}

void CTaskBox::OnSysCommand(UINT /*nID*/, LPARAM /*lParam*/)
{
}

