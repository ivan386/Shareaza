//
// DlgTorrentInfoPage.cpp
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "Skin.h"
#include "DlgTorrentInfoSheet.h"
#include "DlgTorrentInfoPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CTorrentInfoPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CTorrentInfoPage, CPropertyPage)
	//{{AFX_MSG_MAP(CTorrentInfoPage)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTorrentInfoPage property page

CTorrentInfoPage::CTorrentInfoPage(UINT nIDD) : 
	CPropertyPage( nIDD ), m_nIcon( -1 )
{
}

CTorrentInfoPage::~CTorrentInfoPage()
{
}

void CTorrentInfoPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTorrentInfoPage)
	//}}AFX_DATA_MAP
}

CBTInfo* CTorrentInfoPage::GetTorrentInfo()
{
	CTorrentInfoSheet* pSheet = (CTorrentInfoSheet*)GetParent();
	return &pSheet->m_pInfo;
}

Hashes::BtGuid CTorrentInfoPage::GetPeerID()
{
	CTorrentInfoSheet* pSheet = (CTorrentInfoSheet*)GetParent();
	return pSheet->m_pPeerID;
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentInfoPage message handlers

BOOL CTorrentInfoPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	Skin.Apply( NULL, this );

	m_pInfo = GetTorrentInfo();
	m_pPeerID = GetPeerID();
	
	return TRUE;
}

void CTorrentInfoPage::OnPaint()
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

void CTorrentInfoPage::PaintStaticHeader(CDC* pDC, CRect* prc, LPCTSTR psz)
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

HBRUSH CTorrentInfoPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CPropertyPage::OnCtlColor( pDC, pWnd, nCtlColor );

	if ( nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC )
	{
		pDC->SetBkColor( Skin.m_crDialog );
		hbr = Skin.m_brDialog;
	}

	return hbr;
}
