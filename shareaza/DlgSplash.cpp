//
// DlgSplash.cpp
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
#include "ImageServices.h"
#include "DlgSplash.h"
#include "FragmentBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CSplashDlg, CDialog)

BEGIN_MESSAGE_MAP(CSplashDlg, CDialog)
	ON_WM_PAINT()
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
END_MESSAGE_MAP()

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED		0x80000
#define LWA_ALPHA			0x02
#endif
#define AW_BLEND			0x00080000
#define AW_HIDE				0x00010000

#define SPLASH_WIDTH		528
#define SPLASH_HEIGHT		236


/////////////////////////////////////////////////////////////////////////////
// CSplashDlg construction

CSplashDlg::CSplashDlg(int nMax) :
	CDialog( CSplashDlg::IDD, NULL ),
	m_nPos( 0 ),
	m_nMax( nMax ),
	m_sState( theApp.m_sSmartAgent ),
	m_pfnAnimateWindow( NULL )
{
	Create( IDD );
}

CSplashDlg::~CSplashDlg()
{
}

void CSplashDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

/////////////////////////////////////////////////////////////////////////////
// CSplashDlg message handlers

BOOL CSplashDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText( m_sState );

	CClientDC dcScreen( this );

	CImageServices::LoadBitmap( &m_bmSplash, IDR_SPLASH, RT_PNG );

	m_bmBuffer.CreateCompatibleBitmap( &dcScreen, SPLASH_WIDTH, SPLASH_HEIGHT );
	m_dcBuffer1.CreateCompatibleDC( &dcScreen );
	m_dcBuffer2.CreateCompatibleDC( &dcScreen );

	SetWindowPos( NULL, 0, 0, SPLASH_WIDTH, SPLASH_HEIGHT, SWP_NOMOVE );
	CenterWindow();

	if ( theApp.m_bNT && theApp.m_hUser32 != 0 && 
		 theApp.m_dwWindowsVersion >= 5 && GetSystemMetrics( SM_REMOTESESSION ) == 0 )
	{
		(FARPROC&)m_pfnAnimateWindow = GetProcAddress( theApp.m_hUser32, "AnimateWindow" );

		if ( m_pfnAnimateWindow != NULL )
		{
			(*m_pfnAnimateWindow)( GetSafeHwnd(), 250, AW_BLEND );
		}
	}

	SetWindowPos( &wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW );
	UpdateWindow();

	return TRUE;
}

void CSplashDlg::Step(LPCTSTR pszText, bool bClosing)
{
	m_nPos ++;
	m_sState.Format( bClosing ? _T("%s...") : _T("Starting %s..."), pszText );
	SetWindowText( m_sState );

	CClientDC dc( this );
	DoPaint( &dc );
}

void CSplashDlg::Topmost()
{
	if ( IsWindowVisible() )
	{
		SetWindowPos( &wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW );
	}
}

void CSplashDlg::Hide()
{
	m_sState = _T("Ready");
	SetWindowText( m_sState );
	Invalidate();

	if ( m_pfnAnimateWindow != NULL )
	{
		(*m_pfnAnimateWindow)( GetSafeHwnd(), 250, AW_HIDE|AW_BLEND );
	}

	::DestroyWindow( m_hWnd );
	delete this;
}

LRESULT CSplashDlg::OnPrintClient(WPARAM wParam, LPARAM /*lParam*/)
{
	LRESULT lResult = Default();

	CDC* pDC = CDC::FromHandle( (HDC)wParam );
	DoPaint( pDC );

	return lResult;
}

void CSplashDlg::OnPaint()
{
	CPaintDC dc( this );
	DoPaint( &dc );
}

void CSplashDlg::DoPaint(CDC* pDC)
{
	CBitmap* pOld1 = (CBitmap*)m_dcBuffer1.SelectObject( &m_bmSplash );
	CBitmap* pOld2 = (CBitmap*)m_dcBuffer2.SelectObject( &m_bmBuffer );

	m_dcBuffer2.BitBlt( 0, 0, SPLASH_WIDTH, SPLASH_HEIGHT, &m_dcBuffer1, 0, 0, SRCCOPY );

	CFont* pOld3 = (CFont*)m_dcBuffer2.SelectObject( &theApp.m_gdiFontBold );
	m_dcBuffer2.SetBkMode( TRANSPARENT );
	m_dcBuffer2.SetTextColor( RGB( 0, 0, 0 ) );

	CRect rc( 8, 201, 520, SPLASH_HEIGHT );
	UINT nFormat = DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX;

	rc.OffsetRect( -1, 0 );
	m_dcBuffer2.DrawText( m_sState, &rc, nFormat );
	rc.OffsetRect( 2, 0 );
	m_dcBuffer2.DrawText( m_sState, &rc, nFormat );
	rc.OffsetRect( -1, -1 );
	m_dcBuffer2.DrawText( m_sState, &rc, nFormat );
	rc.OffsetRect( 0, 2 );
	m_dcBuffer2.DrawText( m_sState, &rc, nFormat );
	rc.OffsetRect( 0, -1 );

	m_dcBuffer2.SetTextColor( RGB( 255, 255, 255 ) );
	m_dcBuffer2.DrawText( m_sState, &rc, nFormat );

	m_dcBuffer2.SelectObject( pOld3 );

	rc.SetRect( 440, 223, 522, 231 );
	m_dcBuffer2.Draw3dRect( &rc, RGB( 0x40, 0x40, 0x40 ), RGB( 0x40, 0x40, 0x40 ) );
	rc.DeflateRect( 1, 1 );
	m_dcBuffer2.FillSolidRect( &rc, RGB( 0x25, 0x25, 0x25 ) );

	int nOffset;
	if ( theApp.m_bRTL )
		nOffset = m_nMax - min( m_nPos, m_nMax );
	else
		nOffset = 0;

	CFragmentBar::DrawFragment( &m_dcBuffer2, &rc, m_nMax, nOffset, min( m_nPos, m_nMax ),
		RGB( 0x20, 0xB0, 0x20 ), TRUE );
	m_dcBuffer2.SelectClipRgn( NULL );

	pDC->BitBlt( 0, 0, SPLASH_WIDTH, SPLASH_HEIGHT, &m_dcBuffer2, 0, 0, SRCCOPY );

	m_dcBuffer2.SelectObject( pOld2 );
	m_dcBuffer1.SelectObject( pOld1 );
}
