//
// CtrlLibraryHistoryPanel.cpp
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
#include "Settings.h"
#include "Library.h"
#include "LibraryHistory.h"
#include "AlbumFolder.h"
#include "SharedFile.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "Skin.h"

#include "CtrlLibraryFrame.h"
#include "CtrlLibraryHistoryPanel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CLibraryHistoryPanel, CLibraryPanel)
	//{{AFX_MSG_MAP(CLibraryHistoryPanel)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLibraryHistoryPanel construction

CLibraryHistoryPanel::CLibraryHistoryPanel()
{
}

CLibraryHistoryPanel::~CLibraryHistoryPanel()
{
	for ( int nItem = 0 ; nItem < m_pList.GetSize() ; nItem++ )
	{
		delete (Item*)m_pList.GetAt( nItem );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryHistoryPanel operations

BOOL CLibraryHistoryPanel::CheckAvailable(CLibraryTreeItem* pFolders, CLibraryList* pObjects)
{
	m_bAvailable = ( pFolders == NULL );
	return m_bAvailable;
}

void CLibraryHistoryPanel::Update()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );
	BOOL bChanged = FALSE;
	
	for ( int nItem = m_pList.GetSize() - 1 ; nItem >= 0 ; nItem-- )
	{
		Item* pItem = (Item*)m_pList.GetAt( nItem );
		
		if ( ! LibraryHistory.Check( pItem->m_pRecent ) ||
			 ! pItem->m_pRecent->m_pFile )
		{
			delete pItem;
			m_pList.RemoveAt( nItem );
			bChanged = TRUE;
		}
	}
	
	int nCount = 0;
	
	for ( POSITION pos = LibraryHistory.GetIterator() ; pos ; )
	{
		CLibraryRecent* pRecent = LibraryHistory.GetNext( pos );
		if ( ! pRecent->m_pFile ) continue;
		
        int nItem = m_pList.GetSize() - 1;
		for ( ; nItem >= 0 ; nItem-- )
		{
			Item* pItem = (Item*)m_pList.GetAt( nItem );
			if ( pItem->m_pRecent == pRecent ) break;
		}
		
		if ( nItem < 0 )
		{
			Item* pItem			= new Item();
			pItem->m_pRecent	= pRecent;
			pItem->m_nIndex		= pRecent->m_pFile->m_nIndex;
			pItem->m_sText		= pRecent->m_pFile->m_sName;
			pItem->m_nIcon16	= ShellIcons.Get( pItem->m_sText, 16 );
			
			FileTimeToSystemTime( &pRecent->m_tAdded, &pItem->m_pTime );
			SystemTimeToTzSpecificLocalTime( NULL, &pItem->m_pTime, &pItem->m_pTime );
			GetDateFormat( LOCALE_USER_DEFAULT, NULL, &pItem->m_pTime,
				_T("ddd',' MMM dd"), pItem->m_sTime.GetBuffer( 64 ), 64 );
			pItem->m_sTime.ReleaseBuffer();
			
			m_pList.InsertAt( nCount++, pItem );
			bChanged = TRUE;
		}
	}
	
	SCROLLINFO pInfo;
	CRect rc;
	
	GetClientRect( &rc );
	
	m_nColumns		= ( rc.Width() > 500 ) ? 2 : 1;
	int nHeight		= ( m_pList.GetSize() + m_nColumns - 1 ) / m_nColumns * 24 + 2;
	
	pInfo.cbSize	= sizeof(pInfo);
	pInfo.fMask		= SIF_ALL & ~SIF_TRACKPOS;
	pInfo.nMin		= 0;
	pInfo.nMax		= nHeight;
	pInfo.nPage		= rc.Height() - 18;
	pInfo.nPos		= GetScrollPos( SB_VERT );
	pInfo.nPos		= max( 0, min( pInfo.nPos, pInfo.nMax - (int)pInfo.nPage + 1 ) );
	
	SetScrollInfo( SB_VERT, &pInfo, TRUE );
	
	if ( bChanged )
	{
		m_pHover = NULL;
		Invalidate();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryHistoryPanel message handlers

void CLibraryHistoryPanel::OnSize(UINT nType, int cx, int cy) 
{
	CLibraryPanel::OnSize( nType, cx, cy );
	Update();
}

void CLibraryHistoryPanel::OnPaint() 
{
	CRect rcClient, rcItem;
	CPaintDC dc( this );
	CString str;
	
	GetClientRect( &rcClient );
	
	rcItem.CopyRect( &rcClient );
	rcItem.bottom = rcItem.top + 18;
	rcClient.top += 18;

	LoadString( str, IDS_LIBPANEL_RECENT_ADDITIONS );
	
	CFont* pFontOld = (CFont*)dc.SelectObject( &CoolInterface.m_fntCaption );
	CSize szText = dc.GetTextExtent( str );
	int nY = ( rcItem.top + rcItem.bottom + 1 ) / 2 - szText.cy / 2;
	
	dc.SetBkMode( OPAQUE );
	dc.SetBkColor( Skin.m_crBannerBack );
	dc.SetTextColor( Skin.m_crBannerText );	
	dc.ExtTextOut( 4, nY, ETO_CLIPPED|ETO_OPAQUE, &rcItem, str, NULL );
	dc.ExcludeClipRect( &rcItem );
		
	dc.SetBkColor( CoolInterface.m_crWindow );
	dc.SetViewportOrg( 0, -GetScrollPos( SB_VERT ) );
	
	CRect rcWork( &rcClient );
	rcWork.top += 1;
	
	for ( int nRow = 0, nItem = 0 ; nItem < m_pList.GetSize() ; nRow++ )
	{
		dc.SetBkColor( Skin.m_crSchemaRow[ nRow & 1 ] );
		
		for ( int nColumn = 0 ; nColumn < m_nColumns ; nColumn++ )
		{
			Item* pItem = ( nItem < m_pList.GetSize() ) ? (Item*)m_pList.GetAt( nItem++ ) : NULL;
			
			rcItem.SetRect( rcWork.left, rcWork.top, rcWork.left, rcWork.top + 22 );
			
			rcItem.left		+= nColumn * rcWork.Width() / m_nColumns + 1;
			rcItem.right	+= ( nColumn + 1 ) * rcWork.Width() / m_nColumns - 1;
			
			if ( pItem != NULL )
			{
				ShellIcons.Draw( &dc, pItem->m_nIcon16, 16,
					rcItem.left + 3, rcItem.top + 3, dc.GetBkColor() );
				dc.ExcludeClipRect( rcItem.left + 3, rcItem.top + 3,
					rcItem.left + 3 + 16, rcItem.top + 3 + 16 );
				
				dc.SelectObject( &CoolInterface.m_fntNormal );
				dc.SetTextColor( CoolInterface.m_crDisabled );
				
				szText = dc.GetTextExtent( pItem->m_sTime );
				nY = ( rcItem.top + rcItem.bottom ) / 2 - szText.cy / 2;
				
				CRect rcText( rcItem.right - szText.cx - 5, rcItem.top, rcItem.right, rcItem.bottom );
				
				dc.ExtTextOut( rcText.left + 2, nY,
					ETO_CLIPPED|ETO_OPAQUE, &rcText, pItem->m_sTime, NULL );
				dc.ExcludeClipRect( &rcText );
				
				dc.SetTextColor( RGB( 0, 0, 255 ) );
				dc.SelectObject( &CoolInterface.m_fntUnder );
				
				rcText.right	= rcText.left;
				rcText.left		= rcItem.left + 3 + 16 + 3;
				
				str = pItem->m_sText;
				szText = dc.GetTextExtent( str );
				
				if ( szText.cx > rcText.Width() - 4 )
				{
					while ( str.GetLength() > 0 )
					{
						szText = dc.GetTextExtent( str + _T('…') );
						if ( szText.cx < rcText.Width() - 4 ) break;
						str = str.Left( str.GetLength() - 1 );
					}
					
					str += _T('…');
				}
				
				rcText.right = rcText.left + szText.cx + 4;
				
				dc.ExtTextOut( rcText.left + 2, nY, ETO_CLIPPED|ETO_OPAQUE,
					&rcItem, str, NULL );
				
				pItem->m_rect.CopyRect( &rcText );
			}
			else
			{
				dc.ExtTextOut( 0, 0, ETO_OPAQUE, &rcItem, NULL, 0, NULL );
			}
			
			dc.ExcludeClipRect( &rcItem );
		}
		
		rcWork.top += 24;
	}
	
	dc.SetViewportOrg( 0, 0 );
	dc.SelectObject( pFontOld );
	dc.FillSolidRect( &rcClient, CoolInterface.m_crWindow );
}

void CLibraryHistoryPanel::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SCROLLINFO pScroll;

	ZeroMemory( &pScroll, sizeof(pScroll) );
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
		pScroll.nPos = pScroll.nTrackPos;
		break;
	}
	
	pScroll.fMask	= SIF_POS;
	pScroll.nPos	= max( 0, min( pScroll.nPos, pScroll.nMax ) );
	
	SetScrollInfo( SB_VERT, &pScroll );
	Invalidate();
}

BOOL CLibraryHistoryPanel::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint point;
	
	GetCursorPos( &point );
	ScreenToClient( &point );
	point.y += GetScrollPos( SB_VERT );
	
	for ( int nItem = 0 ; nItem < m_pList.GetSize() ; nItem++ )
	{
		Item* pItem = (Item*)m_pList.GetAt( nItem );

		if ( pItem->m_rect.PtInRect( point ) )
		{
			SetCursor( AfxGetApp()->LoadCursor( IDC_HAND ) );
			return TRUE;
		}
	}
	
	return CLibraryPanel::OnSetCursor( pWnd, nHitTest, message );
}

void CLibraryHistoryPanel::OnLButtonUp(UINT nFlags, CPoint point) 
{
	point.y += GetScrollPos( SB_VERT );
	
	for ( int nItem = 0 ; nItem < m_pList.GetSize() ; nItem++ )
	{
		Item* pItem = (Item*)m_pList.GetAt( nItem );
		
		if ( pItem->m_rect.PtInRect( point ) )
		{
			OnClickFile( pItem->m_nIndex );
			break;
		}
	}
	
	point.y -= GetScrollPos( SB_VERT );
	
	CLibraryPanel::OnLButtonUp( nFlags, point );
}

void CLibraryHistoryPanel::OnClickFile(DWORD nFile)
{
	CLibraryFile* pFile = Library.LookupFile( nFile, TRUE );
	
	CLibraryFrame* pFrame = (CLibraryFrame*)GetParent();
	ASSERT_KINDOF(CLibraryFrame, pFrame);
	
	pFrame->Display( pFile );
	
	Library.Unlock();
}
