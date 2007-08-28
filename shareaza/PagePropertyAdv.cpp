//
// PagePropertyAdv.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2006.
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

#include "stdafx.h"
#include "Shareaza.h"
#include "PagePropertyAdv.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "Skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CPropertyPageAdv dialog

IMPLEMENT_DYNAMIC(CPropertyPageAdv, CPropertyPage)
CPropertyPageAdv::CPropertyPageAdv(UINT nIDD)
	: CPropertyPage(nIDD), m_nIcon(-1)
{
	m_psp.dwFlags |= PSP_USETITLE;
}

CPropertyPageAdv::~CPropertyPageAdv()
{
}

void CPropertyPageAdv::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPropertyPageAdv, CPropertyPage)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
END_MESSAGE_MAP()

// CPropertyPageAdv message handlers

BOOL CPropertyPageAdv::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_wndToolTip.Create( this );
	m_wndToolTip.Activate( TRUE );
	m_wndToolTip.SetMaxTipWidth( 200 );
	// Show the tooltip for 20 seconds
	m_wndToolTip.SetDelayTime( TTDT_AUTOPOP, 20 * 1000 );

	Skin.Apply( NULL, this, 0, &m_wndToolTip );

	return TRUE;
}

BOOL CPropertyPageAdv::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST )
	{
		MSG msg;
		CopyMemory( &msg, pMsg, sizeof(MSG) );
		HWND hWndParent = ::GetParent( msg.hwnd );

		while ( hWndParent && hWndParent != m_hWnd )
		{
			msg.hwnd = hWndParent;
			hWndParent = ::GetParent( hWndParent );
		}

		if ( msg.hwnd )
		{
			m_wndToolTip.RelayEvent( &msg );
		}
	}
	return CPropertyPage::PreTranslateMessage(pMsg);
}

void CPropertyPageAdv::OnPaint()
{
	CPaintDC dc( this );
	if ( theApp.m_bRTL ) theApp.m_pfnSetLayout( dc.m_hDC, LAYOUT_RTL );

	if ( m_nIcon >= 0 )
	{
		ShellIcons.Draw( &dc, m_nIcon, 48, 4, 4 );
	}

	for ( CWnd* pWnd = GetWindow( GW_CHILD ) ; pWnd ; pWnd = pWnd->GetNextWindow() )
	{
		if ( pWnd->GetStyle() & WS_VISIBLE ) continue;

		TCHAR szClass[16];
		GetClassName( pWnd->GetSafeHwnd(), szClass, 16 );
		if ( _tcsicmp( szClass, _T("STATIC") ) ) continue;

		CString str;
		CRect rc;

		pWnd->GetWindowText( str );
		pWnd->GetWindowRect( &rc );
		ScreenToClient( &rc );

		if ( str.IsEmpty() || str.GetAt( 0 ) != '-' )
			PaintStaticHeader( &dc, &rc, str );
	}

	dc.SetBkColor( CCoolInterface::GetDialogBkColor() );
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
