//
// WndSearch.cpp
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
#include "QuerySearch.h"
#include "QueryHit.h"
#include "MatchObjects.h"
#include "Network.h"
#include "Packet.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "ManagedSearch.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "Skin.h"
#include "SHA.h"
#include "XML.h"

#include "WndSearch.h"
#include "WndMain.h"
#include "DlgNewSearch.h"
#include "DlgHitColumns.h"
#include "DlgHelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CSearchWnd, CBaseMatchWnd)

BEGIN_MESSAGE_MAP(CSearchWnd, CBaseMatchWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_WM_TIMER()
	ON_WM_NCLBUTTONUP()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SYSCOMMAND()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_LBN_SELCHANGE(IDC_MATCHES, OnSelChangeMatches)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_SEARCH, OnUpdateSearchSearch)
	ON_COMMAND(ID_SEARCH_SEARCH, OnSearchSearch)
	ON_COMMAND(ID_SEARCH_CLEAR, OnSearchClear)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_STOP, OnUpdateSearchStop)
	ON_COMMAND(ID_SEARCH_STOP, OnSearchStop)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_PANEL, OnUpdateSearchPanel)
	ON_COMMAND(ID_SEARCH_PANEL, OnSearchPanel)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_CLEAR, OnUpdateSearchClear)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_DETAILS, OnUpdateSearchDetails)
	ON_COMMAND(ID_SEARCH_DETAILS, OnSearchDetails)
	ON_WM_MDIACTIVATE()
END_MESSAGE_MAP()

#define SIZE_INTERNAL	1982
#define PANEL_WIDTH		200
#define TOOLBAR_HEIGHT	28
#define STATUS_HEIGHT	24
#define SPLIT_SIZE		6


/////////////////////////////////////////////////////////////////////////////
// CSearchWnd construction

CSearchWnd::CSearchWnd(CQuerySearch* pSearch)
{
	if ( pSearch != NULL ) 
	{
		m_pSearches.AddTail( new CManagedSearch( pSearch ) );
	}
	
	Create( IDR_SEARCHFRAME );
}

CSearchWnd::~CSearchWnd()
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );
	
	for ( POSITION pos = m_pSearches.GetHeadPosition() ; pos ; )
	{
		delete (CManagedSearch*)m_pSearches.GetNext( pos );
	}
	
	m_pSearches.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// CSearchWnd message handlers

int CSearchWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CBaseMatchWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	m_wndPanel.Create( this );
	m_wndDetails.Create( this );
	
	CQuerySearch* pSearch = GetLastSearch();
	
	if ( pSearch && pSearch->m_pSchema != NULL )
	{
		CPtrList pColumns;
		CSchemaColumnsDlg::LoadColumns( pSearch->m_pSchema, &pColumns );
		m_wndList.SelectSchema( pSearch->m_pSchema, &pColumns );
	}
	else if ( CSchema* pSchema = SchemaCache.Get( Settings.Search.BlankSchemaURI ) )
	{
		CPtrList pColumns;
		CSchemaColumnsDlg::LoadColumns( pSchema, &pColumns );
		m_wndList.SelectSchema( pSchema, &pColumns );
	}
	
	m_nCacheHubs		= 0;
	m_nCacheLeaves		= 0;
	m_bPanel			= Settings.Search.SearchPanel;
	m_bDetails			= Settings.Search.DetailPanelVisible;
	m_nDetails			= Settings.Search.DetailPanelSize;
	m_bPaused			= TRUE;
	m_bSetFocus			= TRUE;
	m_bWaitMore			= FALSE;
	m_nMaxResults		= 0;
	m_nMaxED2KResults	= 0;
	m_nMaxQueryCount	= 0;
	
	LoadState( _T("CSearchWnd"), TRUE );
	
	ExecuteSearch();
	
	if ( pSearch == NULL ) m_wndPanel.ShowSearch( NULL );
	
	OnSkinChange();
	
	PostMessage( WM_TIMER, 1 );
	
	return 0;
}

void CSearchWnd::OnDestroy() 
{
	CQuerySearch* pSearch = GetLastSearch();
	
	if ( pSearch && pSearch->m_pSchema == NULL )
	{
		if ( m_wndList.m_pSchema != NULL )
		{
			Settings.Search.BlankSchemaURI = m_wndList.m_pSchema->m_sURI;
		}
		else
		{
			Settings.Search.BlankSchemaURI.Empty();
		}
	}
	
	SaveState( _T("CSearchWnd") );
	
	CBaseMatchWnd::OnDestroy();
}

void CSearchWnd::OnSize(UINT nType, int cx, int cy) 
{
	if ( nType != SIZE_INTERNAL ) CPanelWnd::OnSize( nType, cx, cy );
	
	CRect rc;
	GetClientRect( &rc );
	
	if ( m_bPanel )
	{
		m_wndPanel.SetWindowPos( NULL, rc.left, rc.top, PANEL_WIDTH, rc.Height(),
			SWP_NOZORDER|SWP_SHOWWINDOW );
		rc.left += PANEL_WIDTH;
	}
	else if ( m_wndPanel.IsWindowVisible() )
	{
		m_wndPanel.ShowWindow( SW_HIDE );
	}
	
	if ( ! (m_bPaused||m_bWaitMore) ) rc.top += STATUS_HEIGHT;
	
	m_wndToolBar.SetWindowPos( NULL, rc.left, rc.bottom - TOOLBAR_HEIGHT, rc.Width(), TOOLBAR_HEIGHT, SWP_NOZORDER );
	rc.bottom -= TOOLBAR_HEIGHT;
	
	if ( m_bDetails )
	{
		m_wndDetails.SetWindowPos( NULL, rc.left, rc.bottom - m_nDetails, rc.Width(),
			m_nDetails, SWP_NOZORDER|SWP_SHOWWINDOW );
		rc.bottom -= m_nDetails + SPLIT_SIZE;
	}
	else if ( m_wndDetails.IsWindowVisible() )
	{
		m_wndDetails.ShowWindow( SW_HIDE );
	}
	
	m_wndList.SetWindowPos( NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER );
	
	Invalidate();
}

void CSearchWnd::OnSkinChange()
{
	CBaseMatchWnd::OnSkinChange();

	m_wndToolBar.Clear();
	
	if ( ! Skin.CreateToolBar( m_bPanel ? _T("CSearchWnd.Panel") : _T("CSearchWnd.Full"), &m_wndToolBar ) )
	{
		Skin.CreateToolBar( _T("CSearchWnd"), &m_wndToolBar );
	}
	
	OnSize( SIZE_INTERNAL, 0, 0 );
	UpdateMessages();
	
	m_wndPanel.OnSkinChange();
}

void CSearchWnd::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if ( m_bContextMenu )
	{
		TrackPopupMenu( _T("CSearchWnd"), point, ID_SEARCH_DOWNLOAD );
	}
	else
	{
		CBaseMatchWnd::OnContextMenu( pWnd, point );
	}
}

void CSearchWnd::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CBaseMatchWnd::OnMDIActivate( bActivate, pActivateWnd, pDeactivateWnd );
	
	if ( bActivate )
	{
		if ( m_pMatches->m_nFiles > 0 )
			m_wndList.SetFocus();
		else if ( m_wndPanel.IsWindowVisible() )
			m_wndPanel.SetSearchFocus();
		else if ( m_wndList.IsWindowVisible() )
			 m_wndList.SetFocus();
	}
}

void CSearchWnd::OnPaint() 
{
	CPaintDC dc( this );
	CRect rcClient;
	
	GetClientRect( &rcClient );
	rcClient.bottom -= TOOLBAR_HEIGHT;
	
	if ( m_wndDetails.IsWindowVisible() )
	{
		CRect rcBar(	rcClient.left,
						rcClient.bottom - m_nDetails - SPLIT_SIZE,
						rcClient.right,
						rcClient.bottom - m_nDetails );
		
		if ( m_bPanel ) rcBar.left += PANEL_WIDTH;
		
		dc.FillSolidRect( rcBar.left, rcBar.top, rcBar.Width(), 1, GetSysColor( COLOR_BTNFACE ) );
		dc.FillSolidRect( rcBar.left, rcBar.top + 1, rcBar.Width(), 1, GetSysColor( COLOR_3DHIGHLIGHT ) );
		dc.FillSolidRect( rcBar.left, rcBar.bottom - 1, rcBar.Width(), 1, GetSysColor( COLOR_3DSHADOW ) );
		dc.FillSolidRect( rcBar.left, rcBar.top + 2, rcBar.Width(), rcBar.Height() - 3,
			GetSysColor( COLOR_BTNFACE ) );
	}
	
	if ( m_bPaused || m_bWaitMore) return;
	
	CRect rc( &rcClient );
	rc.bottom = rc.top + STATUS_HEIGHT;
	
	int nTop = rc.top + 4;
	
	if ( m_bPanel )
	{
		rc.left += PANEL_WIDTH;
		rc.bottom --;
		dc.FillSolidRect( rc.left, rc.bottom, rc.Width(), 1, RGB( 255, 255, 255 ) );
		dc.Draw3dRect( &rc,
			CCoolInterface::CalculateColour( Skin.m_crBannerBack, RGB(255,255,255), 100 ),
			CCoolInterface::CalculateColour( Skin.m_crBannerBack, 0, 150 ) );
		rc.DeflateRect( 1, 1 );
		nTop --;
	}

	ShellIcons.Draw( &dc, SHI_SEARCH, 16, rc.left + 4, nTop, Skin.m_crBannerBack );
	dc.ExcludeClipRect( rc.left + 4, nTop, rc.left + 4 + 16, nTop + 16 );
	
	CFont* pFont = (CFont*)dc.SelectObject( &CoolInterface.m_fntNormal );
	
	CString str;
	LoadString( str, IDS_SEARCH_ACTIVE );
	
	dc.SetBkColor( Skin.m_crBannerBack );
	dc.SetTextColor( Skin.m_crBannerText );
	dc.ExtTextOut( rc.left + 8 + 16, nTop + 1, ETO_CLIPPED|ETO_OPAQUE,
		&rc, str, NULL );
	
	dc.SelectObject( pFont );
}

BOOL CSearchWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if ( m_wndDetails.IsWindowVisible() )
	{
		CRect rcClient, rc;
		CPoint point;
		
		GetCursorPos( &point );
		GetClientRect( &rcClient );
		ClientToScreen( &rcClient );
		
		rc.SetRect(	rcClient.left,
					rcClient.bottom - TOOLBAR_HEIGHT - m_nDetails - SPLIT_SIZE,
					rcClient.right,
					rcClient.bottom - TOOLBAR_HEIGHT - m_nDetails );
		
		if ( m_bPanel ) rc.left += PANEL_WIDTH;
		
		if ( rc.PtInRect( point ) )
		{
			SetCursor( AfxGetApp()->LoadStandardCursor( IDC_SIZENS ) );
			return TRUE;
		}
	}
	
	return CBaseMatchWnd::OnSetCursor( pWnd, nHitTest, message );
}

void CSearchWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rcClient, rc;
	GetClientRect( &rcClient );
	
	rc.SetRect(	rcClient.left,
				rcClient.bottom - TOOLBAR_HEIGHT - m_nDetails - SPLIT_SIZE,
				rcClient.right,
				rcClient.bottom - TOOLBAR_HEIGHT - m_nDetails );
	
	if ( m_bPanel ) rc.left += PANEL_WIDTH;
	
	if ( m_wndDetails.IsWindowVisible() && rc.PtInRect( point ) )
	{
		DoSizeDetails();
		return;
	}
	
	CBaseMatchWnd::OnLButtonDown( nFlags, point );
}

BOOL CSearchWnd::DoSizeDetails()
{
	MSG* pMsg = &AfxGetThreadState()->m_msgCur;
	CRect rcClient;
	CPoint point;
	
	GetClientRect( &rcClient );
	if ( m_bPanel ) rcClient.left += PANEL_WIDTH;
	if ( ! (m_bPaused||m_bWaitMore) ) rcClient.top += STATUS_HEIGHT;
	rcClient.bottom -= TOOLBAR_HEIGHT;
	ClientToScreen( &rcClient );
	ClipCursor( &rcClient );
	SetCapture();
	
	ScreenToClient( &rcClient );
	
	int nOffset = 0xFFFF;
	
	while ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 )
	{
		while ( ::PeekMessage( pMsg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE ) );
		
		if ( ! AfxGetThread()->PumpMessage() )
		{
			AfxPostQuitMessage( 0 );
			break;
		}
		
		GetCursorPos( &point );
		ScreenToClient( &point );
		
		int nSplit = rcClient.bottom - point.y;
		
		if ( nOffset == 0xFFFF ) nOffset = m_nDetails - nSplit;
		nSplit += nOffset;
		
		if ( nSplit < 8 )
			nSplit = 0;
		if ( nSplit > rcClient.Height() - SPLIT_SIZE - 8 )
			nSplit = rcClient.Height() - SPLIT_SIZE;
		
		if ( nSplit != m_nDetails )
		{
			m_nDetails = nSplit;
			Settings.Search.DetailPanelSize = nSplit;
			OnSize( SIZE_INTERNAL, 0, 0 );
			Invalidate();
		}
	}
	
	ReleaseCapture();
	ClipCursor( NULL );
	
	return TRUE;
}

void CSearchWnd::OnUpdateSearchSearch(CCmdUI* pCmdUI) 
{
	// pCmdUI->Enable( Network.IsWellConnected() );
	pCmdUI->Enable( TRUE );
}

void CSearchWnd::OnSearchSearch() 
{
	CSingleLock pLock( &m_pMatches->m_pSection );
	CManagedSearch* pSearch = NULL;
	
	if ( ! Network.IsWellConnected() ) Network.Connect( TRUE );

	//** The 'Search More' situation   (ToDo: Detect if search changed and skip)
	POSITION pos = m_pSearches.GetTailPosition();
	if( (!m_bPaused) && m_bWaitMore && pos )
	{
		//Re-activate search window
		theApp.Message( MSG_DEBUG, _T("Resuming Search") );
		pSearch = (CManagedSearch*)m_pSearches.GetPrev(pos);
		pSearch->m_bActive = TRUE;
		m_bWaitMore = FALSE;
		m_bUpdate = TRUE;

		//Resume G2 search
		m_nMaxResults = m_pMatches->m_nFilteredHits + Settings.Gnutella.MaxResults;
		m_nMaxQueryCount = pSearch->m_nQueryCount + Settings.Gnutella2.QueryLimit;

		//Resume ED2K search
		m_nMaxED2KResults = m_pMatches->m_nED2KHits + ( (DWORD)min( 201, Settings.eDonkey.MaxResults ) );														
		pSearch->m_tLastED2K = GetTickCount();

		return;
	}
	//** End of 'Search More'
	
	if ( m_pMatches->m_nFiles > 0 )
	{
		CString strMessage;
		LoadString( strMessage, IDS_SEARCH_CLEAR_PREVIOUS );
		
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES )
		{
			pLock.Lock();
			m_pMatches->Clear();
			m_bUpdate = TRUE;
			PostMessage( WM_TIMER, 2 );
			pLock.Unlock();
		}
	}
	
	if ( m_wndPanel.m_bSendSearch )
	{
		pSearch = m_wndPanel.GetSearch();
		if ( pSearch == NULL ) 
		{								//Invalid search, open help window
			CHelpDlg::Show( _T("SearchHelp.BadSearch") );
			return;
		}
		
		if ( m_pMatches->m_nFiles == 0 && pSearch->m_pSearch->m_pSchema != NULL )
		{
			CPtrList pColumns;
			CSchemaColumnsDlg::LoadColumns( pSearch->m_pSearch->m_pSchema, &pColumns );
			m_wndList.SelectSchema( pSearch->m_pSearch->m_pSchema, &pColumns );
		}
	}
	else
	{
		CQuerySearch* pCriteria = GetLastSearch();
		
		if ( pCriteria != NULL ) pCriteria = new CQuerySearch( pCriteria );
		
		CNewSearchDlg dlg( NULL, pCriteria, FALSE, TRUE );
		if ( dlg.DoModal() != IDOK ) return;
		
		pCriteria = dlg.GetSearch();
		pSearch = new CManagedSearch( pCriteria );
	}
	
	Network.CreateID( &pSearch->m_pSearch->m_pGUID );
	
	pLock.Lock();
	
	if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0x8000 )
	{
		for ( POSITION pos = m_pSearches.GetHeadPosition() ; pos ; )
		{
			CManagedSearch* pManaged = (CManagedSearch*)m_pSearches.GetNext( pos );
			pManaged->Stop();
		}
	}
	
	m_pSearches.AddTail( pSearch );
	
	pLock.Unlock();
	
	ExecuteSearch();
}

void CSearchWnd::OnUpdateSearchClear(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_pMatches->m_nFiles > 0 );
}

void CSearchWnd::OnSearchClear() 
{
	m_wndList.DestructiveUpdate();
	m_pMatches->Clear();
	m_bUpdate = TRUE;
	PostMessage( WM_TIMER, 2 );
	
	OnSearchStop();
}

void CSearchWnd::OnUpdateSearchStop(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( ! m_bPaused );
}

void CSearchWnd::OnSearchStop() 
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );
	
	for ( POSITION pos = m_pSearches.GetHeadPosition() ; pos ; )
	{
		CManagedSearch* pManaged = (CManagedSearch*)m_pSearches.GetNext( pos );
		pManaged->Stop();
		pManaged->m_bReceive = FALSE;
	}

	m_bPaused = TRUE;

	UpdateMessages();
}

void CSearchWnd::OnUpdateSearchPanel(CCmdUI* pCmdUI) 
{
	CCoolBarItem* pItem = m_wndToolBar.GetID( ID_SEARCH_PANEL );
	pItem->SetCheck( m_bPanel );
	pItem->SetTip( m_bPanel ? _T("Hide the Search Panel") : _T("Show the Search Panel") );
}

void CSearchWnd::OnSearchPanel() 
{
	Settings.Search.SearchPanel = m_bPanel = ! m_bPanel;
	OnSkinChange();
}

void CSearchWnd::OnUpdateSearchDetails(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_bDetails );
}

void CSearchWnd::OnSearchDetails() 
{
	Settings.Search.DetailPanelVisible = m_bDetails = ! m_bDetails;
	OnSkinChange();
}

void CSearchWnd::OnSysCommand(UINT nID, LPARAM lParam) 
{
	if ( ( ( nID & 0xFFF0 ) == SC_MAXIMIZE ) && m_bPanelMode )
	{
		PostMessage( WM_COMMAND, ID_SEARCH_SEARCH );
	}
	else
	{
		CBaseMatchWnd::OnSysCommand( nID, lParam );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSearchWnd operations

CManagedSearch* CSearchWnd::GetLastManager()
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );
	return m_pSearches.GetCount() ? (CManagedSearch*)m_pSearches.GetTail() : NULL;
}

CQuerySearch* CSearchWnd::GetLastSearch()
{
	CManagedSearch* pManaged = GetLastManager();
	return pManaged ? pManaged->m_pSearch : NULL;
}

void CSearchWnd::ExecuteSearch()
{
	CManagedSearch* pManaged = GetLastManager();
	
	if ( pManaged )
	{
		if(pManaged->m_pSearch->CheckValid())
		{
			m_bPaused			= FALSE;
			m_tSearch			= GetTickCount();
			m_bWaitMore			= FALSE;

			m_nMaxResults		= Settings.Gnutella.MaxResults;
			m_nMaxED2KResults	= (DWORD)min( 201, Settings.eDonkey.MaxResults );
			m_nMaxQueryCount	= Settings.Gnutella2.QueryLimit;
		
			pManaged->Stop();
			pManaged->Start();
		
			m_wndPanel.ShowSearch( pManaged );
		}
		else
		{
			//Bad search, open help window
			CHelpDlg::Show( _T("SearchHelp.BadSearch") );
		}
	}
	
	UpdateMessages();
}

void CSearchWnd::UpdateMessages(BOOL bActive)
{
	CManagedSearch* pManaged	= GetLastManager();
	CQuerySearch* pSearch		= pManaged ? pManaged->m_pSearch : NULL;
	
	CString strCaption;
	Skin.LoadString( strCaption, IDR_SEARCHFRAME );
	
	if ( pSearch != NULL )
	{
		strCaption += _T(" : ");

		if ( pSearch->m_sSearch.GetLength() )
		{
			strCaption += pSearch->m_sSearch;
		}
		else if ( pSearch->m_pSchema != NULL && pSearch->m_pXML != NULL )
		{
			strCaption += pSearch->m_pSchema->GetIndexedWords( pSearch->m_pXML->GetFirstElement() );
		}
		else if ( pSearch->m_bSHA1 ) 
		{
			strCaption += CSHA::HashToString( &pSearch->m_pSHA1, TRUE );
		}
		
		if ( pSearch->m_pSchema )
		{
			strCaption += _T(" (") + pSearch->m_pSchema->m_sTitle + _T(")");
		}
		
		if ( m_pMatches->m_nFilteredFiles || m_pMatches->m_nFilteredHits )
		{
			CString strStats;
			strStats.Format( _T(" [%lu/%lu]"), m_pMatches->m_nFilteredFiles, m_pMatches->m_nFilteredHits );
			strCaption += strStats;
			pManaged->m_nHits = m_pMatches->m_nFilteredHits;
		}
	}
	
	CString strOld;
	GetWindowText( strOld );
	
	if ( strOld != strCaption )
	{
		SetWindowText( strCaption );
		m_sCaption = strCaption;
	}
	
	if ( pManaged != NULL )
	{
		if ( m_nCacheHubs != pManaged->m_nHubs ||
			 m_nCacheLeaves != pManaged->m_nLeaves )
		{
			m_nCacheHubs = pManaged->m_nHubs;
			m_nCacheLeaves = pManaged->m_nLeaves;
			bActive = TRUE;
		}
	}

	if ( bActive )
	{
		m_wndPanel.ShowStatus( ! m_bPaused, !m_bWaitMore,
			m_pMatches->m_nFilteredFiles,
			m_pMatches->m_nFilteredHits,
			m_nCacheHubs, m_nCacheLeaves );
		
		CRect rcList;
		m_wndList.GetWindowRect( &rcList );
		ScreenToClient( &rcList );
		if ( ( rcList.top == 0 ) != (m_bPaused||m_bWaitMore) ) OnSize( SIZE_INTERNAL, 0, 0 );
	}

	if ( m_pMatches->m_nFilteredFiles == 0 )
	{
		if ( m_pMatches->m_nFiles > 0 )
		{
			m_wndList.SetMessage( IDS_SEARCH_FILTERED, ! m_bPanel );
		}
		else if ( m_bPaused )
		{
			m_wndList.SetMessage( IDS_SEARCH_NONE, ! m_bPanel );
		}
		else if ( GetTickCount() - m_tSearch < 16000 )
		{
			m_wndList.SetMessage( IDS_SEARCH_WORKING, FALSE );
		}
		else
		{
			m_wndList.SetMessage( IDS_SEARCH_EMPTY, ! m_bPanel );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSearchWnd event handlers

BOOL CSearchWnd::OnQueryHits(CQueryHit* pHits)
{
	if ( m_bPaused || m_hWnd == NULL ) return FALSE;
	
	CSingleLock pLock( &m_pMatches->m_pSection );
	if ( ! pLock.Lock( 100 ) || m_bPaused ) return FALSE;
	
	for ( POSITION pos = m_pSearches.GetTailPosition() ; pos ; )
	{
		CManagedSearch* pManaged = (CManagedSearch*)m_pSearches.GetPrev( pos );
		BOOL bNull = FALSE;
		
		if ( pManaged->m_bReceive &&
			 (	pManaged->m_pSearch->m_pGUID == pHits->m_pSearchID ||
				( bNull = ( pHits->m_pSearchID == (GGUID&)GUID_NULL ) ) ) )
		{
			m_pMatches->AddHits( pHits, pManaged->m_pSearch, bNull );
			m_bUpdate = TRUE;
			
			if ( m_pMatches->m_nED2KHits >= m_nMaxED2KResults )
			{
				if( !pManaged->m_bAllowG2 ) //If G2 is not active, pause the search now.
				{						
					m_bWaitMore = TRUE;
					pManaged->m_bActive = FALSE;
				}
				pManaged->m_tLastED2K = 0xFFFFFFFF;
				theApp.Message( MSG_DEBUG, _T("ED2K Search Reached Maximum Number of Files") );
			}

			if ( !m_bWaitMore&& ( (m_pMatches->m_nFilteredHits - m_pMatches->m_nED2KHits) >= m_nMaxResults ) )
			{
				m_bWaitMore = TRUE;
				pManaged->m_bActive = FALSE;
				theApp.Message( MSG_DEBUG, _T("Gnutella Search Reached Maximum Number of Files") );
			}
			
			return TRUE;
		}
	}
	
	return FALSE;
}

void CSearchWnd::OnTimer(UINT nIDEvent) 
{
	POSITION pos = m_pSearches.GetTailPosition();
	if( Settings.Gnutella2.QueryLimit && pos )
	{
		CManagedSearch* pManaged = (CManagedSearch*)m_pSearches.GetPrev(pos);
		if( ( pManaged->m_bActive ) && (pManaged->m_nQueryCount > m_nMaxQueryCount) )
		{
			m_bWaitMore = TRUE;
			pManaged->m_bActive = FALSE;
			theApp.Message( MSG_DEBUG, _T("Search Reached Maximum Duration") );
			m_bUpdate = TRUE;
		}
	}

	if ( nIDEvent == 1 )
	{
		if ( m_bSetFocus )
		{
			if ( m_bPanel && m_bPaused ) m_wndPanel.SetSearchFocus();
			else m_wndList.SetFocus();
			m_bSetFocus = FALSE;
		}
		
		if ( CManagedSearch* pManaged = GetLastManager() )
		{
			if ( m_nCacheHubs   != pManaged->m_nHubs ||
				 m_nCacheLeaves != pManaged->m_nLeaves )
			{
				UpdateMessages();
			}
		}
	}
	
	CBaseMatchWnd::OnTimer( nIDEvent );
	
	if ( m_pMatches->m_nFilteredHits == 0 ) m_wndDetails.Update( NULL );
}

void CSearchWnd::OnSelChangeMatches()
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );
	m_wndDetails.Update( m_pMatches->GetSelectedFile( TRUE ) );
}

/////////////////////////////////////////////////////////////////////////////
// CSearchWnd serialize

void CSearchWnd::Serialize(CArchive& ar)
{
	CSingleLock pLock( &m_pMatches->m_pSection, TRUE );
	CString strSchema;
	int nVersion = 1;
	
	if ( ar.IsStoring() )
	{
		ar << nVersion;
		
		ar.WriteCount( m_pSearches.GetCount() );
		
		for ( POSITION pos = m_pSearches.GetHeadPosition() ; pos ; )
		{
			CManagedSearch* pSearch = (CManagedSearch*)m_pSearches.GetNext( pos );
			pSearch->Serialize( ar );
		}
		
		if ( m_pMatches->m_pSchema )
		{
			ar << m_pMatches->m_pSchema->m_sURI;
		}
		else
		{
			ar << strSchema;
		}
	}
	else
	{
		ar >> nVersion;
		if ( nVersion != 1 ) AfxThrowUserException();
		
		m_bPaused = TRUE;
		
		for ( int nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			CManagedSearch* pSearch = new CManagedSearch();
			pSearch->Serialize( ar );
			m_pSearches.AddTail( pSearch );
		}
		
		ar >> strSchema;
		
		if ( CSchema* pSchema = SchemaCache.Get( strSchema ) )
		{
			CPtrList pColumns;
			CSchemaColumnsDlg::LoadColumns( pSchema, &pColumns );
			m_wndList.SelectSchema( pSchema, &pColumns );
		}
		
		if ( m_pSearches.GetCount() ) m_wndPanel.ShowSearch( GetLastManager() );
	}
	
	try
	{
		m_pMatches->Serialize( ar );
	}
	catch ( CException* pException )
	{
		pException->Delete();
		m_pMatches->Clear();
	}
	
	if ( ar.IsLoading() )
	{
		m_bUpdate = TRUE;
		PostMessage( WM_TIMER, 1 );
		SendMessage( WM_TIMER, 2 );
		SetAlert( FALSE );
	}
}
