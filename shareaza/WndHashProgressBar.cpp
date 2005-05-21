//
// WndHashProgressBar.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
#include "Library.h"
#include "LibraryBuilder.h"
#include "CoolInterface.h"
#include "WndHashProgressBar.h"
#include "Settings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CHashProgressBar, CWnd)
	//{{AFX_MSG_MAP(CHashProgressBar)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define WINDOW_WIDTH		320
#define WINDOW_HEIGHT		60
#define DISPLAY_THRESHOLD	5


/////////////////////////////////////////////////////////////////////////////
// CHashProgressBar construction

CHashProgressBar::CHashProgressBar()
{
	m_pParent		= NULL;
	m_hIcon			= NULL;
	m_nRemaining	= 0;
	m_nFlash		= 0;
}

CHashProgressBar::~CHashProgressBar()
{
}

/////////////////////////////////////////////////////////////////////////////
// CHashProgressBar operations

void CHashProgressBar::Create(CWnd* pParent)
{
	m_pParent = pParent;
}

void CHashProgressBar::Run()
{
	int nRemaining = LibraryBuilder.GetRemaining();
	LibraryBuilder.SanityCheck();

	BOOL bShow = Settings.Library.HashWindow;

	if ( m_hWnd == NULL )
	{
		if ( nRemaining > DISPLAY_THRESHOLD )
		{
			LPCTSTR hClass = AfxRegisterWndClass( 0 );
			CreateEx( WS_EX_TOPMOST|WS_EX_TOOLWINDOW, hClass, _T("Shareaza Hashing..."),
				WS_POPUP /*|WS_DISABLED*/, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
				NULL, 0 );
			bShow = TRUE;
		}
	}
	else if ( nRemaining == 0 )
	{
		DestroyWindow();
	}

	if ( m_hWnd != NULL ) Update();

	if ( bShow && m_hWnd != NULL )
	{
		Show( WINDOW_WIDTH, TRUE );
	}
}

void CHashProgressBar::Update()
{
	m_nRemaining = LibraryBuilder.GetRemaining();
	CString strFile = LibraryBuilder.GetCurrentFile();
	m_nTotal = LibraryMaps.GetFileCount();

	int nPos = strFile.ReverseFind( '\\' );
	if ( nPos > 0 ) strFile = strFile.Mid( nPos + 1 );

	if ( strFile != m_sCurrent )
	{
		m_sCurrent = strFile;

		CClientDC dc( this );
		CFont* pOld = (CFont*)dc.SelectObject( &CoolInterface.m_fntCaption );
		CSize sz = dc.GetTextExtent( m_sCurrent );
		dc.SelectObject( pOld );

		int nWidth = sz.cx + 4 + 48 + 8 + 16;
		nWidth = max( nWidth, WINDOW_WIDTH );
		nWidth = min( nWidth, GetSystemMetrics( SM_CXSCREEN ) );
		/*
		if ( ( theApp.m_dwWindowsVersion >= 5 ) && (GetSystemMetrics( SM_CMONITORS ) > 1) )
			nWidth = min( nWidth, GetSystemMetrics( SM_CXVIRTUALSCREEN ) );
		else
			nWidth = min( nWidth, GetSystemMetrics( SM_CXSCREEN ) );
		*/
		Show( nWidth, FALSE );
	}

	Invalidate();
}

void CHashProgressBar::Show(int nWidth, BOOL bShow)
{
	CRect rc;
	SystemParametersInfo( SPI_GETWORKAREA, 0, &rc, 0 );
	rc.left	= rc.right - nWidth;
	rc.top	= rc.bottom - WINDOW_HEIGHT;
	SetWindowPos( bShow ? &wndTopMost : NULL, rc.left, rc.top, rc.Width(), rc.Height(),
		( bShow ? SWP_SHOWWINDOW : SWP_NOZORDER ) | SWP_NOACTIVATE );
}

/////////////////////////////////////////////////////////////////////////////
// CHashProgressBar message handlers

int CHashProgressBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_hIcon = (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_SEARCH_FOLDER),
		IMAGE_ICON, 48, 48, 0 );

	if ( m_hIcon == NULL )
	{
		m_hIcon = (HICON)LoadImage( AfxGetResourceHandle(),
			MAKEINTRESOURCE(IDI_SEARCH_FOLDER), IMAGE_ICON, 32, 32, 0 );
	}

	m_crFill	= GetSysColor( COLOR_ACTIVECAPTION );
	m_crBorder	= CCoolInterface::CalculateColour( m_crFill, 0xFFFFFFFF, 128 );
	m_crText	= CCoolInterface::CalculateColour( m_crFill, 0xFFFFFFFF, 220 );

	if ( m_brFill.m_hObject != NULL ) m_brFill.DeleteObject();
	m_brFill.CreateSolidBrush( m_crFill );

	return 0;
}

BOOL CHashProgressBar::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CHashProgressBar::OnPaint()
{
	CRect rcClient, rcText;
	CPaintDC dc( this );

	GetClientRect( &rcClient );

	dc.Draw3dRect( &rcClient, m_crBorder, m_crBorder );
	rcClient.DeflateRect( 1, 1 );

	dc.SetBkMode( OPAQUE );
	dc.SetBkColor( m_crFill );
	dc.SetTextColor( ( m_nFlash++ & 1 ) ? RGB( 255, 255, 0 ) : m_crText );

	// Icon
	DrawIconEx( dc, rcClient.left + 4, rcClient.top + 4,
		m_hIcon, 48, 48, 0, m_brFill, DI_NORMAL );
	dc.ExcludeClipRect( rcClient.left + 4, rcClient.top + 4,
		rcClient.left + 4 + 48, rcClient.top + 4 + 48 );

	// Text
	CFont* pOld = dc.GetCurrentFont();
	CString strText, strFormat;
	CSize sz;

	LoadString( strFormat, IDS_HASH_MESSAGE );
	strText.Format( strFormat, m_nRemaining );

	dc.SelectObject( &CoolInterface.m_fntNormal );
	sz = dc.GetTextExtent( strText );
	rcText.SetRect( 4 + 48 + 8, 12, 4 + 48 + 8 + sz.cx, 12 + sz.cy );
	rcText.OffsetRect( rcClient.left, rcClient.top );
	dc.ExtTextOut( rcText.left, rcText.top, ETO_OPAQUE|ETO_CLIPPED,
		&rcText, strText, NULL );
	dc.ExcludeClipRect( rcText.left, rcText.top, rcText.right, rcText.bottom );

	dc.SelectObject( &CoolInterface.m_fntCaption );
	sz = dc.GetTextExtent( m_sCurrent );
	dc.ExtTextOut( rcText.left, rcClient.top + 4 + 48 - sz.cy - 8,
		ETO_OPAQUE|ETO_CLIPPED, &rcClient, m_sCurrent, NULL );

	dc.SelectObject( pOld );

	// Progress bar
	CRect rcProgress = rcClient;
	rcProgress.DeflateRect( 1, 1 );
	rcProgress.top = rcProgress.bottom - 3;
	float nPercentage = (float)( m_nTotal - m_nRemaining );
	nPercentage /= m_nTotal;
	if ( ( nPercentage < 0 ) || ( nPercentage > 1 ) ) nPercentage = 1;
	rcProgress.right = rcProgress.left + (LONG)( rcProgress.Width() * nPercentage );
	dc.Draw3dRect( &rcProgress, m_crText, m_crText );
}

void CHashProgressBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	Settings.Library.HashWindow = FALSE;
	ShowWindow( SW_HIDE );
}
