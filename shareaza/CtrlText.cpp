//
// CtrlText.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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
#include "CtrlText.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CTextCtrl, CWnd)
	//{{AFX_MSG_MAP(CTextCtrl)
	ON_WM_VSCROLL()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define LINE_BUFFER_LIMIT		1024
#define LINE_BUFFER_BLOCK		64


/////////////////////////////////////////////////////////////////////////////
// CTextCtrl construction

CTextCtrl::CTextCtrl()
{
	m_crBackground	= GetSysColor( COLOR_WINDOW );
	m_crText[0]		= RGB( 0, 0, 0 );
	m_crText[1]		= RGB( 0, 0, 127 );
	m_crText[2]		= RGB( 255, 0, 0 );
	m_crText[3]		= RGB( 192, 192, 192 );
	m_crText[4]		= RGB( 0, 0, 255 );

	m_pFont.CreateFont( -11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, _T("Courier New") );
	m_cCharacter = CSize( 0, 0 );
}

CTextCtrl::~CTextCtrl()
{
	Clear( FALSE );
}

/////////////////////////////////////////////////////////////////////////////
// CTextCtrl operations

BOOL CTextCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID) 
{
	dwStyle |= WS_CHILD|WS_VSCROLL;
	return CWnd::Create( NULL, NULL, dwStyle, rect, pParentWnd, nID, NULL );
}

void CTextCtrl::Add(int nType, LPCTSTR pszText)
{
	CSingleLock pLock( &m_pSection, TRUE );
	CString strLine;

	for ( ; *pszText ; pszText++ )
	{
		if ( *pszText == 13 )
		{
			if ( strLine.GetLength() ) AddLine( nType, strLine );
			strLine.Empty();
		}
		else if ( *pszText != 10 )
		{
			strLine += *pszText;
		}
	}

	if ( strLine.GetLength() ) AddLine( nType, strLine );
}

void CTextCtrl::AddLine(int nType, LPCTSTR pszLine)
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( m_pLines.GetSize() >= LINE_BUFFER_LIMIT )
	{
		for ( int nCount = 0 ; nCount < LINE_BUFFER_BLOCK ; nCount++ )
		{
			delete (CTextLine*)m_pLines.GetAt( 0 );
			m_pLines.RemoveAt( 0 );
		}
		m_bProcess = TRUE;
	}

	m_pLines.Add( new CTextLine( nType, pszLine ) );
	Invalidate();
}

void CTextCtrl::Clear(BOOL bInvalidate)
{
	CSingleLock pLock( &m_pSection, TRUE );

	for ( int nLine = 0 ; nLine < m_pLines.GetSize() ; nLine++ )
	{
		delete (CTextLine*)m_pLines.GetAt( nLine );
	}
	m_pLines.RemoveAll();
	
	m_nPosition = m_nTotal = 0;

	if ( bInvalidate )
	{
		UpdateScroll( TRUE );
		Invalidate();
	}
}

void CTextCtrl::UpdateScroll(BOOL bFull)
{
	SCROLLINFO si;

	ZeroMemory( &si, sizeof(si) );
	si.cbSize = sizeof(si);

	if ( bFull )
	{
		CRect rc;
		GetClientRect( &rc );

		si.fMask	= SIF_POS|SIF_PAGE|SIF_RANGE|SIF_DISABLENOSCROLL;
		si.nPos		= m_nPosition;
		si.nPage	= rc.Height() / m_cCharacter.cy;
		si.nMin		= 0;
		si.nMax		= m_nTotal + si.nPage - 1;
	}
	else
	{
		si.fMask	= SIF_POS;
		si.nPos		= m_nPosition;
	}
	
	SetScrollInfo( SB_VERT, &si );
}

CFont* CTextCtrl::GetFont()
{
	return &m_pFont;
}

/////////////////////////////////////////////////////////////////////////////
// CTextCtrl message handlers

int CTextCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	m_nPosition = m_nTotal = 0;
	m_bProcess = TRUE;
	
	// Try to get the number of lines to scroll when the mouse wheel is rotated
	if( !SystemParametersInfo ( SPI_GETWHEELSCROLLLINES, 0, &m_nScrollWheelLines, 0) )
		m_nScrollWheelLines = 3;

	return 0;
}

void CTextCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CSingleLock pLock( &m_pSection, TRUE );
	SCROLLINFO si;

	ZeroMemory( &si, sizeof(si) );
	si.cbSize	= sizeof(si);
	si.fMask	= SIF_ALL;

	GetScrollInfo( SB_VERT, &si );

	switch ( nSBCode )
	{
	case SB_TOP:
		m_nPosition = 0;
		break;
	case SB_BOTTOM:
		m_nPosition = m_nTotal - 1;
		break;
	case SB_LINEUP:
		m_nPosition--;
		break;
	case SB_LINEDOWN:
		m_nPosition++;
		break;
	case SB_PAGEUP:
		m_nPosition -= ( si.nPage - 1 );
		break;
	case SB_PAGEDOWN:
		m_nPosition += ( si.nPage - 1 );
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		m_nPosition = si.nTrackPos;
		break;
	}

	m_nPosition = max( 0, min( m_nTotal, m_nPosition ) );

	UpdateScroll();
	Invalidate();
}

BOOL CTextCtrl::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

void CTextCtrl::OnPaint() 
{
	CSingleLock pLock( &m_pSection, TRUE );
	CRect rcClient, rcLine;
	CPaintDC dc( this );
	
	GetClientRect( &rcClient );

	CFont* pOldFont = (CFont*)dc.SelectObject( &m_pFont );
	
	if ( m_cCharacter.cx == 0 ) m_cCharacter = dc.GetTextExtent( _T("X") );

	BOOL bBottom	= ( m_nPosition >= m_nTotal );
	BOOL bModified	= m_bProcess;

	if ( m_bProcess ) m_nTotal = 0;
	
	int nWidth = ( rcClient.right - 4 ) / m_cCharacter.cx;

	for ( int nLine = 0 ; nLine < m_pLines.GetSize() ; nLine++ )
	{
		CTextLine* pLine = (CTextLine*)m_pLines.GetAt( nLine );

		if ( m_bProcess || ! pLine->m_nLine )
		{
			m_nTotal += pLine->Process( nWidth );
			bModified = TRUE;
		}
	}

	if ( bBottom ) m_nPosition = m_nTotal;
	if ( bModified ) UpdateScroll( TRUE );
	m_bProcess = FALSE;

	dc.SetTextColor( m_crText[0] );
	dc.SetBkColor( m_crBackground );
	dc.SetBkMode( OPAQUE );

	rcLine.CopyRect( &rcClient );
	rcLine.bottom += ( m_nTotal - m_nPosition ) * m_cCharacter.cy;
	rcLine.top = rcLine.bottom - m_cCharacter.cy;

	for ( nLine = m_pLines.GetSize() - 1 ; nLine >= 0 && rcLine.bottom > 0 ; nLine-- )
	{
		CTextLine* pLine = (CTextLine*)m_pLines.GetAt( nLine );

		dc.SetTextColor( m_crText[ pLine->m_nType ] );
		pLine->Paint( &dc, &rcLine );
	}

	if ( rcLine.bottom > 0 )
	{
		rcLine.top = 0;
		dc.FillSolidRect( &rcLine, m_crBackground );
	}

	dc.SelectObject( pOldFont );
}

void CTextCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize( nType, cx, cy );
	m_bProcess = TRUE;
}

void CTextCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();
}

BOOL CTextCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	short nRows = (zDelta / WHEEL_DELTA);

	if ( WHEEL_PAGESCROLL==m_nScrollWheelLines )
	{
		// scroll by page is activated
		SCROLLINFO si;

		ZeroMemory( &si, sizeof(si) );
		si.cbSize	= sizeof(si);
		si.fMask	= SIF_ALL;

		GetScrollInfo( SB_VERT, &si );

		nRows *=( si.nPage - 1 );
	}
	else
		nRows *= m_nScrollWheelLines;

	CSingleLock pLock( &m_pSection, TRUE );

	m_nPosition-= nRows;
	m_nPosition = max( 0, min( m_nTotal, m_nPosition ) );

	UpdateScroll();
	Invalidate();

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

/////////////////////////////////////////////////////////////////////////////
// CTextLine construction

CTextLine::CTextLine(int nType, LPCTSTR pszText)
{
	m_sText	= pszText;
	m_pLine	= NULL;
	m_nLine	= 0;
	m_nType = nType;
}

CTextLine::~CTextLine()
{
	if ( m_pLine ) delete [] m_pLine;
}

/////////////////////////////////////////////////////////////////////////////
// CTextLine process

int CTextLine::Process(int nWidth)
{
	if ( m_pLine ) delete [] m_pLine;
	m_pLine = NULL;
	m_nLine = 0;
	
	int nLength = 0;
	int nLast = 0;

	for ( LPCTSTR pszText = m_sText ; ; pszText++, nLength++ )
	{
		if ( *pszText == 32 || *pszText == 0 )
		{
			if ( nLength <= nWidth )
			{
				nLast = nLength;
			}
			else if ( nLast )
			{
				AddLine( nLast );
				nLength = nLast = nLength - nLast;
			}
			else
			{
				break;
			}
		}

		if ( *pszText == 0 ) break;
	}

	if ( nLength || !m_nLine ) AddLine( nLength );

	return m_nLine;
}

void CTextLine::AddLine(int nLength)
{
	int* pLine = new int[ m_nLine + 1 ];
	CopyMemory( pLine, m_pLine, m_nLine * sizeof(int) );
	if ( m_pLine ) delete [] m_pLine;
	m_pLine = pLine;
	m_pLine[ m_nLine++ ] = nLength;
}

/////////////////////////////////////////////////////////////////////////////
// CTextLine paint

void CTextLine::Paint(CDC* pDC, CRect* pRect)
{
	int nHeight = pRect->bottom - pRect->top;

	pRect->top		-= ( m_nLine - 1 ) * nHeight;
	pRect->bottom	-= ( m_nLine - 1 ) * nHeight;

	LPCTSTR pszLine	= m_sText;
	int* pLength	= m_pLine;

	for ( int nLine = 0 ; nLine < m_nLine ; nLine++, pLength++ )
	{
		if ( pDC->RectVisible( pRect ) )
		{
			pDC->ExtTextOut( pRect->left + 2, pRect->top, ETO_CLIPPED|ETO_OPAQUE,
				pRect, pszLine, *pLength, NULL );
		}
		pszLine += *pLength;

		pRect->top += nHeight;
		pRect->bottom += nHeight;
	}

	pRect->top		-= ( m_nLine + 1 ) * nHeight;
	pRect->bottom	-= ( m_nLine + 1 ) * nHeight;
}
