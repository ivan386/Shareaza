//
// PagePropertyAdv.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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
#include "PagePropertyAdv.h"
#include "CoolInterface.h"
#include "ShellIcons.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CPropertyPageAdv

IMPLEMENT_DYNAMIC(CPropertyPageAdv, CPropertyPage)

CPropertyPageAdv::CPropertyPageAdv(UINT nIDD) :
	CPropertyPage( nIDD ),
	m_nIcon( -1 )
{
	m_psp.dwFlags |= PSP_USETITLE;
}

void CPropertyPageAdv::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPropertyPageAdv, CPropertyPage)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CPropertyPageAdv message handlers

BOOL CPropertyPageAdv::OnInitDialog()
{
	if ( ! CPropertyPage::OnInitDialog() )
		return FALSE;

	Skin.Apply( NULL, this );

	return TRUE;
}

void CPropertyPageAdv::OnPaint()
{
	CPaintDC dc( this );
	if ( Settings.General.LanguageRTL )
		SetLayout( dc.m_hDC, LAYOUT_RTL );

	if ( m_nIcon >= 0 )
	{
		ShellIcons.Draw( &dc, m_nIcon, 48, 4, 4 );
	}

	for ( CWnd* pWnd = GetWindow( GW_CHILD ) ; pWnd ; pWnd = pWnd->GetNextWindow() )
	{
		if ( pWnd->GetStyle() & WS_VISIBLE ) continue;

		TCHAR szClass[16];
		GetClassName( pWnd->GetSafeHwnd(), szClass, 16 );
		if ( _tcsicmp( szClass, _T("STATIC") ) != 0 ) continue;

		CString str;
		CRect rc;

		pWnd->GetWindowText( str );
		pWnd->GetWindowRect( &rc );
		ScreenToClient( &rc );

		if ( str.IsEmpty() || str.GetAt( 0 ) != '-' )
			PaintStaticHeader( &dc, &rc, str );
	}

	dc.SetBkColor( Skin.m_crDialog );
}

void CPropertyPageAdv::PaintStaticHeader(CDC* pDC, CRect* prc, LPCTSTR psz)
{
	CFont* pOldFont = (CFont*)pDC->SelectObject( GetFont() );
	CSize sz = pDC->GetTextExtent( psz );

	pDC->SetBkMode( OPAQUE );
	pDC->SetBkColor( Skin.m_crBannerBack );
	pDC->SetTextColor( Skin.m_crBannerText );

	CRect rc( prc );
	rc.bottom	= rc.top + min( rc.Height(), 16 );
	rc.right	= rc.left + sz.cx + 10;

	pDC->ExtTextOut( rc.left + 4, rc.top + 1, ETO_CLIPPED|ETO_OPAQUE,
		&rc, psz, static_cast< UINT >( _tcslen( psz ) ), NULL );

	rc.SetRect( rc.right, rc.top, prc->right, rc.top + 1 );
	pDC->ExtTextOut( rc.left, rc.top, ETO_OPAQUE, &rc, NULL, 0, NULL );

	pDC->SelectObject( pOldFont );
}

BOOL CPropertyPageAdv::OnEraseBkgnd(CDC* pDC)
{
	CPropertySheetAdv* pSheet = (CPropertySheetAdv*)GetParent();
	if ( pSheet->m_pSkin && pSheet->m_pSkin->OnEraseBkgnd( this, pDC ) ) return TRUE;

	CRect rc;
	GetClientRect( &rc );
	pDC->FillSolidRect( &rc, Skin.m_crDialog );

	return TRUE;
}

HBRUSH CPropertyPageAdv::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

	if ( nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC )
	{
		pDC->SetBkColor( Skin.m_crDialog );
		hbr = Skin.m_brDialog;
	}

	return hbr;
}

// CPropertySheetAdv

IMPLEMENT_DYNAMIC(CPropertySheetAdv, CPropertySheet)

BEGIN_MESSAGE_MAP(CPropertySheetAdv, CPropertySheet)
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCACTIVATE()
	ON_WM_NCPAINT()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCLBUTTONUP()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_NCMOUSEMOVE()
	ON_WM_SIZE()
//	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_SETTEXT, &CPropertySheetAdv::OnSetText)
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPropertySheetAdv::CPropertySheetAdv() : 
	CPropertySheet( _T("") ),
	m_pSkin( NULL )
{
	m_psh.dwFlags &= ~PSP_HASHELP;
}

BOOL CPropertySheetAdv::OnInitDialog()
{
	BOOL bResult = CPropertySheet::OnInitDialog();

	m_pSkin = Skin.GetWindowSkin( _T("CTorrentSheet") );
	if ( m_pSkin == NULL ) m_pSkin = Skin.GetWindowSkin( this );
	if ( m_pSkin == NULL ) m_pSkin = Skin.GetWindowSkin( _T("CDialog") );

	if ( m_pSkin != NULL )
	{
		CRect rc;
		GetClientRect( &rc );
		m_pSkin->CalcWindowRect( &rc );
		m_brDialog.CreateSolidBrush( Skin.m_crDialog );
		SetWindowPos( NULL, 0, 0, rc.Width(), rc.Height(), SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED );
		OnSize( 1982, 0, 0 );
	}

	return bResult;
}

void CPropertySheetAdv::SetTabTitle(CPropertyPage* pPage, CString& strTitle)
{
	CString strClass( pPage->GetRuntimeClass()->m_lpszClassName );
	CString strTabLabel = Skin.GetDialogCaption( strClass );
	if ( ! strTabLabel.IsEmpty() )
		strTitle = strTabLabel;
	pPage->m_psp.pszTitle = strTitle.GetBuffer();
}

void CPropertySheetAdv::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	if ( m_pSkin )
		m_pSkin->OnNcCalcSize( this, bCalcValidRects, lpncsp );
	else
		CPropertySheet::OnNcCalcSize( bCalcValidRects, lpncsp );
}

LRESULT CPropertySheetAdv::OnNcHitTest(CPoint point)
{
	if ( m_pSkin )
		return m_pSkin->OnNcHitTest( this, point, ( GetStyle() & WS_THICKFRAME ) ? TRUE : FALSE );
	else
		return CPropertySheet::OnNcHitTest( point );
}

BOOL CPropertySheetAdv::OnNcActivate(BOOL bActive)
{
	if ( m_pSkin )
	{
		BOOL bVisible = IsWindowVisible();
		if ( bVisible ) ModifyStyle( WS_VISIBLE, 0 );
		BOOL bResult = CPropertySheet::OnNcActivate( bActive );
		if ( bVisible ) ModifyStyle( 0, WS_VISIBLE );
		m_pSkin->OnNcActivate( this, bActive || ( m_nFlags & WF_STAYACTIVE ) );
		return bResult;
	}
	else
		return CPropertySheet::OnNcActivate( bActive );
}

void CPropertySheetAdv::OnNcPaint()
{
	if ( m_pSkin )
		m_pSkin->OnNcPaint( this );
	else
		CPropertySheet::OnNcPaint();
}

void CPropertySheetAdv::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	if ( m_pSkin && m_pSkin->OnNcLButtonDown( this, nHitTest, point ) ) return;
	CPropertySheet::OnNcLButtonDown(nHitTest, point);
}

void CPropertySheetAdv::OnNcLButtonUp(UINT nHitTest, CPoint point)
{
	if ( m_pSkin && m_pSkin->OnNcLButtonUp( this, nHitTest, point ) ) return;
	CPropertySheet::OnNcLButtonUp( nHitTest, point );
}

void CPropertySheetAdv::OnNcLButtonDblClk(UINT nHitTest, CPoint point)
{
	if ( m_pSkin && m_pSkin->OnNcLButtonDblClk( this, nHitTest, point ) ) return;
	CPropertySheet::OnNcLButtonDblClk( nHitTest, point );
}

void CPropertySheetAdv::OnNcMouseMove(UINT nHitTest, CPoint point)
{
	if ( m_pSkin ) m_pSkin->OnNcMouseMove( this, nHitTest, point );
	CPropertySheet::OnNcMouseMove( nHitTest, point );
}

void CPropertySheetAdv::OnSize(UINT nType, int cx, int cy)
{
	if ( m_pSkin ) m_pSkin->OnSize( this );

	if ( nType != 1982 ) CPropertySheet::OnSize( nType, cx, cy );
}

LRESULT CPropertySheetAdv::OnSetText(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if ( m_pSkin )
	{
		BOOL bVisible = IsWindowVisible();
		if ( bVisible ) ModifyStyle( WS_VISIBLE, 0 );
		LRESULT lResult = Default();
		if ( bVisible ) ModifyStyle( 0, WS_VISIBLE );
		if ( m_pSkin ) m_pSkin->OnSetText( this );
		return lResult;
	}
	else
		return Default();
}

//BOOL CPropertySheetAdv::OnEraseBkgnd(CDC* pDC)
//{
//	if ( m_pSkin && m_pSkin->OnEraseBkgnd( this, pDC ) ) return TRUE;
//	
//	CRect rc;
//	GetClientRect( &rc );
//	pDC->FillSolidRect( &rc, Skin.m_crDialog );
//
//	return TRUE;
//}

BOOL CPropertySheetAdv::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	return FALSE;
}
