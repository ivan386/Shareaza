//
// CtrlLibraryHistoryPanel.cpp
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
#include "Settings.h"
#include "Library.h"
#include "LibraryHistory.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "CtrlLibraryFrame.h"
#include "CtrlLibraryHistoryPanel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CLibraryHistoryPanel, CPanelCtrl)

BEGIN_MESSAGE_MAP(CLibraryHistoryPanel, CPanelCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
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
		delete m_pList.GetAt( nItem );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryHistoryPanel operations

void CLibraryHistoryPanel::Update()
{
	m_wndTip.Hide();

	CSingleLock pLock( &Library.m_pSection, TRUE );
	BOOL bChanged = FALSE;
	
	for ( INT_PTR nItem = m_pList.GetSize() - 1 ; nItem >= 0 ; nItem-- )
	{
		Item* pItem = m_pList.GetAt( nItem );
		
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
		
        INT_PTR nItem = m_pList.GetSize() - 1;
		for ( ; nItem >= 0 ; nItem-- )
		{
			Item* pItem = m_pList.GetAt( nItem );
			if ( pItem->m_pRecent == pRecent ) break;
		}
		
		if ( nItem < 0 )
		{
			Item* pItem			= new Item();
			pItem->m_pRecent	= pRecent;
			pItem->m_nIndex		= pRecent->m_pFile->m_nIndex;
			pItem->m_sText		= pRecent->m_pFile->m_sName;
			pItem->m_nIcon16	= ShellIcons.Get( pRecent->m_pFile->GetPath(), 16 );
			
			FileTimeToSystemTime( &pRecent->m_tAdded, &pItem->m_pTime );
			SystemTimeToTzSpecificLocalTime( NULL, &pItem->m_pTime, &pItem->m_pTime );
			GetDateFormat( LOCALE_USER_DEFAULT, NULL, &pItem->m_pTime,
				_T("ddd',' MMM dd"), pItem->m_sTime.GetBuffer( 64 ), 64 );
			pItem->m_sTime.ReleaseBuffer();
			
			m_pList.InsertAt( nCount++, pItem );
			bChanged = TRUE;
		}
	}
	
	SCROLLINFO pInfo = {};
	CRect rc;
	
	GetClientRect( &rc );
	
	m_nColumns		= ( rc.Width() > 500 ) ? 2 : 1;
	int nHeight		= static_cast< int >( ( m_pList.GetSize() + m_nColumns - 1 ) / m_nColumns * 24 + 2 );
	
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

int CLibraryHistoryPanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelCtrl::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_wndTip.Create( this, &Settings.Interface.TipLibrary );

	return 0;
}

void CLibraryHistoryPanel::OnDestroy()
{
	if ( m_wndTip.m_hWnd ) m_wndTip.DestroyWindow();

	CPanelCtrl::OnDestroy();
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
			Item* pItem = ( nItem < m_pList.GetSize() ) ? m_pList.GetAt( nItem++ ) : NULL;
			
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
				
				dc.SetTextColor( CoolInterface.m_crTextLink );
				dc.SelectObject( &CoolInterface.m_fntUnder );
				
				rcText.right	= rcText.left;
				rcText.left		= rcItem.left + 3 + 16 + 3;
				
				str = pItem->m_sText;
				szText = dc.GetTextExtent( str );
				
				if ( szText.cx > rcText.Width() - 4 )
				{
					while ( str.GetLength() > 0 )
					{
						szText = dc.GetTextExtent( str + _T('\x2026') );
						if ( szText.cx < rcText.Width() - 4 ) break;
						str = str.Left( str.GetLength() - 1 );
					}
					
					str += _T('\x2026');
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

BOOL CLibraryHistoryPanel::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint point;
	
	GetCursorPos( &point );
	ScreenToClient( &point );
	point.y += GetScrollPos( SB_VERT );
	
	for ( int nItem = 0 ; nItem < m_pList.GetSize() ; nItem++ )
	{
		Item* pItem = m_pList.GetAt( nItem );

		if ( pItem->m_rect.PtInRect( point ) )
		{
			m_wndTip.Show( pItem->m_nIndex );
			SetCursor( AfxGetApp()->LoadCursor( IDC_HAND ) );
			return TRUE;
		}
	}

	m_wndTip.Hide();
	
	return CPanelCtrl::OnSetCursor( pWnd, nHitTest, message );
}

void CLibraryHistoryPanel::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_wndTip.Hide();

	CPoint pt( point.x, point.y + GetScrollPos( SB_VERT ) );
	for ( int nItem = 0 ; nItem < m_pList.GetSize() ; nItem++ )
	{
		Item* pItem = m_pList.GetAt( nItem );
		
		if ( pItem->m_rect.PtInRect( pt ) )
		{
			OnClickFile( pItem->m_nIndex );
			break;
		}
	}
	
	CPanelCtrl::OnLButtonUp( nFlags, point );
}

void CLibraryHistoryPanel::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	m_wndTip.Hide();

	SetFocus();
}

void CLibraryHistoryPanel::OnClickFile(DWORD nFile)
{
	CQuickLock oLock( Library.m_pSection );
	if ( CLibraryFile* pFile = Library.LookupFile( nFile ) )
	{	
		if ( CLibraryFrame* pFrame = (CLibraryFrame*)GetParent() )
		{
			ASSERT_KINDOF(CLibraryFrame, pFrame);
			pFrame->Display( pFile );
		}
	}
}
