//
// WndHashProgressBar.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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
#include "Library.h"
#include "LibraryBuilder.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "Settings.h"
#include "WndHashProgressBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CHashProgressBar, CWnd)

BEGIN_MESSAGE_MAP(CHashProgressBar, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

#define WINDOW_WIDTH		320
#define WINDOW_HEIGHT		48

/////////////////////////////////////////////////////////////////////////////
// CHashProgressBar construction

CHashProgressBar::CHashProgressBar()
	: m_nRemaining		( 0 )
	, m_nPercentage		( 0 )
	, m_nLastShow		( 0 )
	, m_nPerfectWidth	( 0 )
	, m_nAlpha			( 0 )
	, m_nIcon			( SHI_FILE )
{
}

/////////////////////////////////////////////////////////////////////////////
// CHashProgressBar operations

void CHashProgressBar::Run()
{
	const CString sCurrent = LibraryBuilder.GetCurrent();
	m_nRemaining = LibraryBuilder.GetRemaining();
	const BOOL bFullscreen = IsUserUsingFullscreen();
	const BOOL bShow = Settings.Library.HashWindow && ( m_nRemaining || ! sCurrent.IsEmpty() ) && ! bFullscreen;

	if ( ! sCurrent.IsEmpty() )
	{
		m_sCurrent = PathFindFileName( sCurrent );
		m_nIcon = ShellIcons.Get( sCurrent, 32 );
	}
	else if ( m_sCurrent.IsEmpty() && m_nRemaining )
	{
		m_sCurrent = _T( "File cooling..." );
		m_nIcon = SHI_FILE;
	}
	m_nPercentage = min( LibraryBuilder.GetProgress(), 100ul );

	if ( bShow )
	{
		m_nLastShow = GetTickCount();
		if ( m_hWnd == NULL )
		{
			try
			{
				CreateEx( WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
					AfxRegisterWndClass( CS_SAVEBITS ),
					CLIENT_NAME_T _T(" Hashing..."), WS_POPUP | WS_VISIBLE, 0, 0,
					WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0 );

				m_nPerfectWidth = 0;
			}
			catch (CResourceException* pEx)
			{
				pEx->Delete();
			}
		}
	}
	else if ( m_hWnd && ( bFullscreen || GetTickCount() > m_nLastShow + 2500 ) ) // 2.5 sec delay
	{
		DestroyWindow();
	}

	if ( m_hWnd )
	{
		CRect rcWindow;
		GetWindowRect( &rcWindow );

		// Resize window to fit filename
		int nWidth = rcWindow.Width();
		if ( m_nPerfectWidth != nWidth )
		{
			if ( ! m_nPerfectWidth )
				m_nPerfectWidth = WINDOW_WIDTH;

			nWidth = ( m_nPerfectWidth > nWidth ) ? 
				min( nWidth + 8, m_nPerfectWidth ) :
				max( nWidth - 8, m_nPerfectWidth );

			// Move window to lower right edge of desktop
			CRect rcWorkArea;
			SystemParametersInfo( SPI_GETWORKAREA, 0, &rcWorkArea, 0 );

			rcWindow.left = rcWorkArea.right - nWidth - 4;
			rcWindow.top = rcWorkArea.bottom - WINDOW_HEIGHT - 4;
			rcWindow.right = rcWindow.left + nWidth;
			rcWindow.bottom = rcWindow.top + WINDOW_HEIGHT;
			MoveWindow( &rcWindow, FALSE );
		}

		// Re-draw window
		CClientDC dcClient( this );
		Draw( &dcClient );

		BOOL bAlpha = TRUE;
		if ( bShow )
		{
			// Hide window under cursor
			CPoint ptMouse;
			GetCursorPos( &ptMouse );
			bAlpha = rcWindow.PtInRect( ptMouse );
		}
		const BYTE nNewAlpha = (BYTE)( bAlpha ? max( m_nAlpha - 20, 0 ) : min( m_nAlpha + 20, 240 ) );
		if ( m_nAlpha != nNewAlpha )
		{
			m_nAlpha = nNewAlpha;
			SetLayeredWindowAttributes( NULL, m_nAlpha, LWA_ALPHA );
		}

		dcClient.SelectStockObject( SYSTEM_FONT ); // GDI font leak fix
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHashProgressBar message handlers

int CHashProgressBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	m_nAlpha = 0;

	SetTimer( 1, 100, NULL );

	return 0;
}

void CHashProgressBar::OnDestroy()
{
	KillTimer( 1 );

	m_sCurrent.Empty();
	m_nRemaining = 0;
	m_nPercentage = 0;
	m_nLastShow = 0;
	m_nPerfectWidth = 0;
	m_nAlpha = 0;
	m_nIcon = SHI_FILE;

	CWnd::OnDestroy();
}

BOOL CHashProgressBar::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CHashProgressBar::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent == 1 )
	{
		Run();
	}

	CWnd::OnTimer( nIDEvent );
}

void CHashProgressBar::OnPaint()
{
	CPaintDC dcClient( this );

	Draw( &dcClient );
}

void CHashProgressBar::Draw(CDC* pDC)
{
	CRect rcClient;
	GetClientRect( &rcClient );

	CDC dc;
	dc.CreateCompatibleDC( pDC );
	CBitmap bm;
	bm.CreateCompatibleBitmap( pDC, rcClient.Width(), rcClient.Height() );
	CBitmap* pOldBitmap = dc.SelectObject( &bm );

	dc.Draw3dRect( &rcClient, CoolInterface.m_crTipBorder, CoolInterface.m_crTipBorder );
	rcClient.DeflateRect( 1, 1 );
	dc.FillSolidRect( &rcClient, CoolInterface.m_crTipBack );

	dc.SetBkMode( TRANSPARENT );
	dc.SetTextColor( CoolInterface.m_crTipText );

	// Icon
	ShellIcons.Draw( &dc, m_nIcon, 32,
		rcClient.left + 4, rcClient.top + 4, CoolInterface.m_crTipBack );

	// Text
	CString strText;
	strText.Format( LoadString( IDS_HASH_MESSAGE ), m_nRemaining );
	
	CFont* pOld = dc.SelectObject( &CoolInterface.m_fntNormal );

	CRect rcText( rcClient.left + 4 + 32 + 8, rcClient.top + 4,
		rcClient.right - 8, rcClient.top + 4 + 12 );
	dc.DrawText( strText, rcText,
		DT_LEFT | DT_VCENTER | DT_SINGLELINE );

	dc.SelectObject( &CoolInterface.m_fntCaption );

	CRect rcFilename( rcClient.left + 4 + 32 + 8, rcClient.top + 4 + 12 + 4,
		rcClient.right - 8, rcClient.top + 4 + 12 + 4 + 18 );
	dc.DrawText( m_sCurrent, rcFilename,
		DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS );

	// Calculate perfect width
	dc.DrawText( m_sCurrent, rcFilename,
		DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_CALCRECT );
	m_nPerfectWidth = min( max( 1 + 4 + 32 + 8 + rcFilename.Width() + 8 + 1, WINDOW_WIDTH ),
		GetSystemMetrics( SM_CXSCREEN ) / 2 );

	dc.SelectObject( pOld );

	// Progress bar
	if ( m_nPercentage > 0 )
	{
		CRect rcProgress = rcClient;
		rcProgress.DeflateRect( 1, 1 );
		rcProgress.top = rcProgress.bottom - 3;
		rcProgress.right = rcProgress.left + (LONG)( ( ( rcProgress.Width() - 1 ) * m_nPercentage ) / 100 ) + 1;
		dc.Draw3dRect( &rcProgress, CoolInterface.m_crTipBorder, CoolInterface.m_crTipBorder );
	}

	rcClient.InflateRect( 1, 1 );
	pDC->BitBlt( rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), &dc, 0, 0, SRCCOPY );

	dc.SelectObject( pOldBitmap );

	ValidateRect( NULL );
}

void CHashProgressBar::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	ShowWindow( SW_HIDE );
}
