//
// CtrlText.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "CtrlText.h"
#include "CoolInterface.h"

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
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define LINE_BUFFER_LIMIT		4096
#define LINE_BUFFER_BLOCK		64


/////////////////////////////////////////////////////////////////////////////
// CTextCtrl construction

CTextCtrl::CTextCtrl() :
	m_nPosition( 0 ),
	m_nTotal( 0 ),
	m_bProcess( TRUE ),
	m_nScrollWheelLines( 3 ),
	m_nLastClicked( -1 )
{
	m_crBackground	= CoolInterface.m_crWindow ;
	m_crText[0]		= CoolInterface.m_crText ;			// Black		- MSG_DEFAULT
	m_crText[1]		= CoolInterface.m_crNetworkDown ;	// Blue			- MSG_SYSTEM / MSG_DOWNLOAD
	m_crText[2]		= CoolInterface.m_crTextAlert ;		// Red			- MSG_ERROR
	m_crText[3]		= CoolInterface.m_crNetworkNull ;	// Gray			- MSG_DEBUG
	m_crText[4]		= CoolInterface.m_crTextLink ;		// Light Blue	- MSG_TEMP
	m_crText[5]		= CoolInterface.m_crTextAlert ;		// Red			- MSG_DISPLAYED_ERROR

	m_pFont.CreateFontW( -theApp.m_nDefaultFontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, theApp.m_sSystemLogFont );
	m_cCharacter = CSize( 0, 0 );

	// Try to get the number of lines to scroll when the mouse wheel is rotated
	SystemParametersInfo ( SPI_GETWHEELSCROLLLINES, 0, &m_nScrollWheelLines, 0);
}

CTextCtrl::~CTextCtrl()
{
	Clear( FALSE );
}

/////////////////////////////////////////////////////////////////////////////
// CTextCtrl operations

BOOL CTextCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID) 
{
	dwStyle |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_VSCROLL;
	return CWnd::Create( NULL, NULL, dwStyle, rect, pParentWnd, nID, NULL );
}

void CTextCtrl::Add(int nType, LPCTSTR pszText)
{
	CQuickLock pLock( m_pSection );
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
	CQuickLock pLock( m_pSection );

	if ( m_pLines.GetSize() >= LINE_BUFFER_LIMIT )
	{
		for ( int nCount = 0 ; nCount < LINE_BUFFER_BLOCK ; nCount++ )
		{
			delete m_pLines.GetAt( nCount );
		}
		m_pLines.RemoveAt( 0, LINE_BUFFER_BLOCK );

		m_bProcess = TRUE;

		if ( m_nLastClicked < LINE_BUFFER_BLOCK )
			m_nLastClicked = -1;
		else
			m_nLastClicked -= LINE_BUFFER_BLOCK;
	}

	m_pLines.Add( new CTextLine( nType, pszLine ) );
	Invalidate();
}

void CTextCtrl::Clear(BOOL bInvalidate)
{
	CQuickLock pLock( m_pSection );

	for ( int nLine = 0 ; nLine < m_pLines.GetSize() ; nLine++ )
	{
		delete m_pLines.GetAt( nLine );
	}
	m_pLines.RemoveAll();
	
	m_nPosition = m_nTotal = 0;

	m_nLastClicked = -1;

	if ( bInvalidate )
	{
		UpdateScroll( TRUE );
		Invalidate();
	}
}

void CTextCtrl::UpdateScroll(BOOL bFull)
{
	SCROLLINFO si = {};

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

void CTextCtrl::OnVScroll(UINT nSBCode, UINT /*nPos*/, CScrollBar* /*pScrollBar*/) 
{
	CQuickLock pLock( m_pSection );
	SCROLLINFO si = {};

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

BOOL CTextCtrl::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}

void CTextCtrl::OnPaint() 
{
	CQuickLock pLock( m_pSection );
	CRect rcClient;
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
		CTextLine* pLine = m_pLines.GetAt( nLine );

		if ( m_bProcess || ! pLine->m_nLine )
		{
			m_nTotal += pLine->Process( nWidth );
			bModified = TRUE;
		}
	}

	if ( bBottom ) m_nPosition = m_nTotal;
	if ( bModified ) UpdateScroll( TRUE );
	m_bProcess = FALSE;

	dc.SetBkMode( OPAQUE );

	CRect rcLine( rcClient );
	rcLine.bottom += ( m_nTotal - m_nPosition ) * m_cCharacter.cy;
	rcLine.top = rcLine.bottom - m_cCharacter.cy;

	for ( INT_PTR nLine = m_pLines.GetSize() - 1 ; nLine >= 0 && rcLine.bottom > 0 ; nLine-- )
	{
		CTextLine* pLine = m_pLines.GetAt( nLine );
		dc.SetTextColor( pLine->m_bSelected ? CoolInterface.m_crHiText :
			m_crText[ pLine->m_nType ] );
		dc.SetBkColor( pLine->m_bSelected ? CoolInterface.m_crHighlight :
			m_crBackground );
		pLine->Paint( &dc, &rcLine );
	}

	if ( rcLine.bottom > 0 )
	{
		rcLine.top = 0;
		dc.FillSolidRect( &rcLine, m_crBackground );
	}

	dc.SelectObject( pOldFont );
}

int CTextCtrl::HitTest(const CPoint& pt) const
{
	CQuickLock pLock( m_pSection );

	if ( m_cCharacter.cy != 0 )
	{
		CRect rcClient;
		GetClientRect( &rcClient );
		CRect rcLine( rcClient );
		rcLine.bottom += ( m_nTotal - m_nPosition ) * m_cCharacter.cy;
		for ( int nLine = m_pLines.GetCount() - 1;
			nLine >= 0 && rcLine.bottom > rcClient.top ; nLine-- )
		{
			CTextLine* pLine = m_pLines.GetAt( nLine );
			rcLine.top = rcLine.bottom - pLine->m_nLine * m_cCharacter.cy;
			if ( rcLine.PtInRect( pt ) )
				return nLine;
			rcLine.bottom -= pLine->m_nLine * m_cCharacter.cy;
		}
	}
	return -1;
}

void CTextCtrl::CopyText() const
{
	CQuickLock pLock( m_pSection );

	CString str;
	bool bGotIt = false;
	for ( int i = 0; i < m_pLines.GetCount(); i++ )
	{
		CTextLine* pLineTemp = m_pLines.GetAt( i );
		if ( pLineTemp->m_bSelected )
		{
			str += pLineTemp->m_sText + _T("\r\n");
			bGotIt = true;
		}
	}

	if ( ! bGotIt )
	{
		if ( m_nLastClicked != -1 )
		{
			CTextLine* pLineTemp = m_pLines.GetAt( m_nLastClicked );
			str = pLineTemp->m_sText;
			bGotIt = true;
		}
	}

	if ( bGotIt && AfxGetMainWnd()->OpenClipboard() )
	{
		EmptyClipboard();

		if ( theApp.m_bNT )
		{
			CT2W pszWide( (LPCTSTR)str );
			DWORD nSize = ( lstrlenW(pszWide) + 1 ) * sizeof(WCHAR);
			HANDLE hMem = GlobalAlloc( GMEM_MOVEABLE|GMEM_DDESHARE, nSize );
			LPVOID pMem = GlobalLock( hMem );
			CopyMemory( pMem, pszWide, nSize );
			GlobalUnlock( hMem );
			SetClipboardData( CF_UNICODETEXT, hMem );
		}
		else
		{
			CT2A pszASCII( (LPCTSTR)str );
			DWORD nSize = lstrlenA(pszASCII) + 1;
			HANDLE hMem = GlobalAlloc( GMEM_MOVEABLE|GMEM_DDESHARE, nSize );
			LPVOID pMem = GlobalLock( hMem );
			CopyMemory( pMem, pszASCII, nSize );
			GlobalUnlock( hMem );
			SetClipboardData( CF_TEXT, hMem );
		}

		CloseClipboard();
	}
}

void CTextCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize( nType, cx, cy );
	m_bProcess = TRUE;
}

void CTextCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CQuickLock pLock( m_pSection );

	SetFocus();

	int nLine = HitTest( point );
	if ( nLine != -1 )
	{
		CTextLine* pLine = m_pLines.GetAt( nLine );

		if ( ( nFlags & MK_CONTROL ) == MK_CONTROL )
		{
			// Invert
			pLine->m_bSelected = ! pLine->m_bSelected;
			m_nLastClicked = nLine;
		}
		else if ( ( nFlags & MK_SHIFT ) == MK_SHIFT )
		{
			// Select from m_nLastClicked to nLine
			if ( m_nLastClicked == -1 )
			{
				m_nLastClicked = nLine;
			}
			for ( int i = 0; i < m_pLines.GetCount(); i++ )
			{
				CTextLine* pLineTemp = m_pLines.GetAt( i );
				pLineTemp->m_bSelected = ( m_nLastClicked < nLine ) ? 
					( i >= m_nLastClicked && i <= nLine ) :
					( i <= m_nLastClicked && i >= nLine );
			}
		}
		else
		{
			// Select one, unselect others
			for ( int i = 0; i < m_pLines.GetCount(); i++ )
			{
				CTextLine* pLineTemp = m_pLines.GetAt( i );
				pLineTemp->m_bSelected = ( pLineTemp == pLine );
			}
			m_nLastClicked = nLine;
		}
	}

	InvalidateRect( NULL );
}

void CTextCtrl::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	CQuickLock pLock( m_pSection );

	if ( m_nLastClicked == -1 )
	{
		m_nLastClicked = HitTest( point );
	}
}

void CTextCtrl::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	CQuickLock pLock( m_pSection );

	if ( GetKeyState( VK_CONTROL ) < 0 )
	{
		switch ( nChar )
		{
		// Ctrl+C, Ctrl+X or Ctrl+Insert
		case 'C':
		case 'X':
		case VK_INSERT:
			CopyText();
			break;

		// Ctrl+A
		case 'A':
			// Select all
			for ( int i = 0; i < m_pLines.GetCount(); i++ )
			{
				CTextLine* pLineTemp = m_pLines.GetAt( i );
				pLineTemp->m_bSelected = TRUE;
			}
			InvalidateRect( NULL );
			break;
		}
	}

	// Esc
	if ( nChar == VK_ESCAPE )
	{
		// Unselect all
		for ( int i = 0; i < m_pLines.GetCount(); i++ )
		{
			CTextLine* pLineTemp = m_pLines.GetAt( i );
			pLineTemp->m_bSelected = FALSE;
		}
		InvalidateRect( NULL );
	}
}

BOOL CTextCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	short nRows = ( zDelta / WHEEL_DELTA );

	if ( WHEEL_PAGESCROLL == m_nScrollWheelLines )
	{
		// scroll by page is activated
		SCROLLINFO si = {};

		si.cbSize	= sizeof( si );
		si.fMask	= SIF_ALL;

		GetScrollInfo( SB_VERT, &si );

		nRows = short( nRows * ( si.nPage - 1 ) );
	}
	else
		nRows = short( nRows * m_nScrollWheelLines );

	CQuickLock pLock( m_pSection );

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
	m_bSelected = FALSE;
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
