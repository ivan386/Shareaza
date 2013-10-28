//
// CtrlPanel.cpp
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
#include "CtrlPanel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CPanelCtrl, CWnd)

BEGIN_MESSAGE_MAP(CPanelCtrl, CWnd)
	ON_WM_MOUSEWHEEL()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPanelCtrl construction

CPanelCtrl::CPanelCtrl()
{
	// Try to get the number of lines to scroll when the mouse wheel is rotated
	if( !SystemParametersInfo ( SPI_GETWHEELSCROLLLINES, 0, &m_nScrollWheelLines, 0) )
	{
		m_nScrollWheelLines = 3;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPanelCtrl operations

BOOL CPanelCtrl::Create(CWnd* pParentWnd)
{
	CRect rect( 0, 0, 0, 0 );
	return CWnd::CreateEx( WS_EX_CONTROLPARENT, NULL, _T("CPanelCtrl"), WS_CHILD | WS_TABSTOP |
		WS_VSCROLL | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, rect, pParentWnd, 0, NULL );
}

BOOL CPanelCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// Scroll window under cursor
	if ( CWnd* pWnd = WindowFromPoint( pt ) )
	{
		if ( pWnd != this )
		{
			const LPCTSTR szScrollable[] = {
				// Search window
				_T("CMatchCtrl"),
				// Library window
				_T("CLibraryTreeView"),
				_T("CLibraryAlbumView"),
				_T("CLibraryThumbView"),
				_T("CLibraryTileView"),
				_T("CLibraryFileView"),
				_T("CLibraryDetailView"),
				NULL
			};
			for ( int i = 0; szScrollable[ i ]; ++i )
			{
				if ( pWnd == FindWindowEx(
					GetParent()->GetSafeHwnd(), NULL, NULL, szScrollable[ i ] ) )
				{
					return pWnd->PostMessage( WM_MOUSEWHEEL, MAKEWPARAM( nFlags, zDelta ), MAKELPARAM( pt.x, pt.y ) );
				}
			}
		}
	}

	PostMessage( WM_VSCROLL, MAKEWPARAM( SB_THUMBPOSITION, GetScrollPos( SB_VERT ) -
		zDelta / WHEEL_DELTA * m_nScrollWheelLines * 8 ), NULL );

	return TRUE;
}

void CPanelCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/) 
{
	SCROLLINFO pScroll = {};
	pScroll.cbSize	= sizeof(pScroll);
	pScroll.fMask	= SIF_ALL;
	GetScrollInfo( SB_VERT, &pScroll );

	switch ( nSBCode )
	{
	case SB_TOP:
		pScroll.nPos = 0;
		break;
	case SB_BOTTOM:
		pScroll.nPos = pScroll.nMax - 1;
		break;
	case SB_LINEUP:
		pScroll.nPos -= 8;
		break;
	case SB_LINEDOWN:
		pScroll.nPos += 8;
		break;
	case SB_PAGEUP:
		pScroll.nPos -= pScroll.nPage;
		break;
	case SB_PAGEDOWN:
		pScroll.nPos += pScroll.nPage;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		pScroll.nPos = nPos;
		break;
	}
	
	pScroll.fMask	= SIF_POS;
	pScroll.nPos	= max( 0, min( pScroll.nPos, pScroll.nMax ) );
	
	SetScrollInfo( SB_VERT, &pScroll );

	Invalidate();
}

void CPanelCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize( nType, cx, cy );

	Update();

	UpdateWindow();
}
