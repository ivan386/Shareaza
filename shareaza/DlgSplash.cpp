//
// DlgSplash.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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
#include "ImageFile.h"
#include "DlgSplash.h"
#include "FragmentBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CSplashDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYENDSESSION()
END_MESSAGE_MAP()

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED		0x80000
#define LWA_ALPHA			0x02
#endif
#define AW_BLEND			0x00080000
#define AW_HIDE				0x00010000

#define SPLASH_WIDTH		528
#define SPLASH_HEIGHT		236

CBitmap CSplashDlg::m_bmSplash;

/////////////////////////////////////////////////////////////////////////////
// CSplashDlg construction

CSplashDlg::CSplashDlg(int nMax, bool bClosing)
	: CDialog		( CSplashDlg::IDD, GetDesktopWindow() )
	, m_nPos		( 0 )
	, m_nMax		( nMax )
	, m_bClosing	( bClosing )
	, m_sState		( Settings.SmartAgent() )
{
	if ( ! m_bmSplash.m_hObject )
		m_bmSplash.Attach( CImageFile::LoadBitmapFromFile( Settings.General.Path + L"\\Data\\Splash.png" ) );

	Create( IDD, GetDesktopWindow() );
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

	SetClassLongPtr( GetSafeHwnd(), GCL_STYLE, GetClassLongPtr( GetSafeHwnd(), GCL_STYLE ) | CS_SAVEBITS | CS_DROPSHADOW );

	SetWindowText( m_sState );

	SetWindowPos( NULL, 0, 0, SPLASH_WIDTH, SPLASH_HEIGHT, SWP_NOMOVE );
	CenterWindow();

	if ( GetSystemMetrics( SM_REMOTESESSION ) == 0 )
	{
		AnimateWindow( 250, AW_BLEND );
	}

	SetWindowPos( &wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW );
	UpdateWindow();

	return TRUE;
}

BOOL CSplashDlg::OnQueryEndSession()
{
	UpdateWindow();

	CDialog::OnQueryEndSession();

	return FALSE;
}

void CSplashDlg::Step(LPCTSTR pszText)
{
	// Check if m_nMax was set high enough during construction to allow another
	// step to take place
	ASSERT( m_nPos < m_nMax );

	m_nPos ++;
	m_sState.Format( m_bClosing ? _T("%s...") : _T("Starting %s..."), pszText );
	SetWindowText( m_sState );

	if ( theApp.m_pfnShutdownBlockReasonCreate )
		theApp.m_pfnShutdownBlockReasonCreate( m_hWnd, m_sState );

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

void CSplashDlg::Hide(BOOL bAbort)
{
	if ( ! bAbort )
	{
		// Check if m_nMax was set too high during construction, or if not enough
		// steps were run
		ASSERT( m_nPos == m_nMax );

		LoadString( m_sState, AFX_IDS_IDLEMESSAGE );
		SetWindowText( m_sState );
		Invalidate();

		if ( GetSystemMetrics( SM_REMOTESESSION ) == 0 )
		{
			AnimateWindow( 250, AW_HIDE | AW_BLEND );
		}
	}

	if ( theApp.m_pfnShutdownBlockReasonDestroy )
		theApp.m_pfnShutdownBlockReasonDestroy( m_hWnd );

	::DestroyWindow( m_hWnd );
	delete this;
}

void CSplashDlg::OnPaint()
{
	CPaintDC dc( this );
	DoPaint( &dc );
}

void CSplashDlg::DoPaint(CDC* pDC)
{
	CDC dcSplash;
	dcSplash.CreateCompatibleDC( pDC );
	CBitmap* pOld1 = (CBitmap*)dcSplash.SelectObject( &m_bmSplash );

	CDC dcMemory;
	dcMemory.CreateCompatibleDC( pDC );
	CBitmap bmBuffer;
	bmBuffer.CreateCompatibleBitmap( pDC, SPLASH_WIDTH, SPLASH_HEIGHT );
	CBitmap* pOld2 = (CBitmap*)dcMemory.SelectObject( &bmBuffer );

	dcMemory.BitBlt( 0, 0, SPLASH_WIDTH, SPLASH_HEIGHT, &dcSplash, 0, 0, SRCCOPY );

	CFont* pOld3 = (CFont*)dcMemory.SelectObject( &theApp.m_gdiFontBold );
	dcMemory.SetBkMode( TRANSPARENT );
	dcMemory.SetTextColor( RGB( 0, 0, 0 ) );

	CRect rc( 8, 201, 520, SPLASH_HEIGHT );
	UINT nFormat = DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX;

	rc.OffsetRect( -1, 0 );
	dcMemory.DrawText( m_sState, &rc, nFormat );
	rc.OffsetRect( 2, 0 );
	dcMemory.DrawText( m_sState, &rc, nFormat );
	rc.OffsetRect( -1, -1 );
	dcMemory.DrawText( m_sState, &rc, nFormat );
	rc.OffsetRect( 0, 2 );
	dcMemory.DrawText( m_sState, &rc, nFormat );
	rc.OffsetRect( 0, -1 );

	dcMemory.SetTextColor( RGB( 255, 255, 255 ) );
	dcMemory.DrawText( m_sState, &rc, nFormat );

	dcMemory.SelectObject( pOld3 );

	rc.SetRect( 440, 223, 522, 231 );
	dcMemory.Draw3dRect( &rc, RGB( 0x40, 0x40, 0x40 ), RGB( 0x40, 0x40, 0x40 ) );
	rc.DeflateRect( 1, 1 );
	dcMemory.FillSolidRect( &rc, RGB( 0x25, 0x25, 0x25 ) );

	int nOffset;
	if ( Settings.General.LanguageRTL )
		nOffset = m_nMax - min( m_nPos, m_nMax );
	else
		nOffset = 0;

	CFragmentBar::DrawFragment( &dcMemory, &rc, m_nMax, nOffset, min( m_nPos, m_nMax ), RGB( 0x20, 0xB0, 0x20 ), true );
	dcMemory.SelectClipRgn( NULL );

	pDC->BitBlt( 0, 0, SPLASH_WIDTH, SPLASH_HEIGHT, &dcMemory, 0, 0, SRCCOPY );

	dcMemory.SelectObject( pOld2 );
	dcSplash.SelectObject( pOld1 );
}
