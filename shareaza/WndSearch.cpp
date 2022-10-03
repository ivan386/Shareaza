//
// WndSearch.cpp
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
#include "XML.h"

#include "WndSearch.h"
#include "WndMain.h"
#include "DlgNewSearch.h"
#include "DlgHitColumns.h"
#include "DlgHelp.h"
#include "Security.h"
#include "ResultFilters.h"
#include "SearchManager.h"

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
	ON_UPDATE_COMMAND_UI_RANGE(3000, 3100, OnUpdateFilters)
	ON_COMMAND_RANGE(3000, 3100, OnFilters)
END_MESSAGE_MAP()

#define SIZE_INTERNAL	1982
#define TOOLBAR_HEIGHT	28
#define STATUS_HEIGHT	24
#define SPLIT_SIZE		6


/////////////////////////////////////////////////////////////////////////////
// CSearchWnd construction

CSearchWnd::CSearchWnd(CQuerySearch* pSearch) :
	m_bPanel( Settings.Search.SearchPanel ),
	m_bSetFocus( TRUE ),
	m_bDetails( Settings.Search.DetailPanelVisible ),
	m_nDetails( Settings.Search.DetailPanelSize ),
	m_tSearch( 0 ),
	m_nCacheHits( 0 ),
	m_nCacheHubs( 0 ),
	m_nCacheLeaves( 0 ),
	m_sCaption( _T("") ),
	m_bWaitMore( FALSE ),
	m_nMaxResults( 0 ),
	m_nMaxED2KResults( 0 ),
	m_nMaxQueryCount( 0 )
{
	if ( pSearch )
	{
		CQuickLock pLock( m_pMatches->m_pSection );

		m_oSearches.push_back( new CManagedSearch( pSearch ) );
	}

	Create( IDR_SEARCHFRAME );
}

CSearchWnd::~CSearchWnd()
{
	CQuickLock pLock( m_pMatches->m_pSection );

	m_oSearches.clear();
}

/////////////////////////////////////////////////////////////////////////////
// CSearchWnd message handlers

int CSearchWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CBaseMatchWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_wndPanel.Create( this );
	m_wndDetails.Create( this );

	CQuerySearchPtr pSearch = GetLastSearch();

	if ( pSearch && pSearch->m_pSchema != NULL )
	{
		CSchemaMemberList pColumns;
		CSchemaColumnsDlg::LoadColumns( pSearch->m_pSchema, &pColumns );
		m_wndList.SelectSchema( pSearch->m_pSchema, &pColumns );
	}
	else if ( CSchemaPtr pSchema = SchemaCache.Get( Settings.Search.BlankSchemaURI ) )
	{
		CSchemaMemberList pColumns;
		CSchemaColumnsDlg::LoadColumns( pSchema, &pColumns );
		m_wndList.SelectSchema( pSchema, &pColumns );
	}

	LoadState( _T("CSearchWnd"), TRUE );

	ExecuteSearch();

	if ( ! pSearch || ! pSearch->m_bAutostart )
	{
		m_wndPanel.ShowSearch( NULL );
	}
	else
	{
		m_wndPanel.Disable();
		if ( m_bPanel && Settings.Search.HideSearchPanel )
			m_bPanel = FALSE;
	}

	PostMessage( WM_TIMER, 1 );

	return 0;
}

void CSearchWnd::OnDestroy()
{
	CQuerySearchPtr pSearch = GetLastSearch();

	if ( pSearch && pSearch->m_pSchema == NULL )
	{
		if ( m_wndList.m_pSchema != NULL )
		{
			Settings.Search.BlankSchemaURI = m_wndList.m_pSchema->GetURI();
		}
		else
		{
			Settings.Search.BlankSchemaURI.Empty();
		}
	}

	SaveState( _T("CSearchWnd") );

	OnSearchStop();

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

	if ( ::IsWindow( m_wndToolBar.GetSafeHwnd() ) ) m_wndToolBar.SetWindowPos( NULL, rc.left, rc.bottom - TOOLBAR_HEIGHT, rc.Width(), TOOLBAR_HEIGHT, SWP_NOZORDER );
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
	Skin.Translate( _T("CMatchCtrl"), &m_wndList.m_wndHeader );
}

void CSearchWnd::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if ( m_bContextMenu )
	{
		Skin.TrackPopupMenu( _T("CSearchWnd"), point, ID_SEARCH_DOWNLOAD );
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

		dc.FillSolidRect( rcBar.left, rcBar.top, rcBar.Width(), 1, CoolInterface.m_crResizebarEdge );
		dc.FillSolidRect( rcBar.left, rcBar.top + 1, rcBar.Width(), 1, CoolInterface.m_crResizebarHighlight );
		dc.FillSolidRect( rcBar.left, rcBar.bottom - 1, rcBar.Width(), 1, CoolInterface.m_crResizebarShadow );
		dc.FillSolidRect( rcBar.left, rcBar.top + 2, rcBar.Width(), rcBar.Height() - 3, CoolInterface.m_crResizebarFace );
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

	CoolInterface.Draw( &dc, IDR_SEARCHFRAME, 16, rc.left + 4, nTop, Skin.m_crBannerBack );
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

		if ( m_bPanel )
		{
			if ( Settings.General.LanguageRTL )
				rc.right -= PANEL_WIDTH;
			else
				rc.left += PANEL_WIDTH;
		}

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
	//pCmdUI->Enable( TRUE );

	if ( m_bPaused || m_bWaitMore )
		pCmdUI->Enable( TRUE );
	else if ( m_pMatches->m_nFilteredHits )
		pCmdUI->Enable( FALSE );
	else
		pCmdUI->Enable( TRUE );
}

void CSearchWnd::OnSearchSearch()
{
	if ( ! Network.IsWellConnected() ) Network.Connect( TRUE );

	SetModified();

	// The 'Search More' situation
	if ( ! m_bPaused && m_bWaitMore )
	{
		CQuickLock pLock( m_pMatches->m_pSection );

		if ( ! empty() )
		{
			CSearchPtr pManaged = m_oSearches.back();

			//Re-activate search window
			theApp.Message( MSG_DEBUG, _T("Resuming Search") );
			pManaged->SetActive( TRUE );
			m_bWaitMore = FALSE;

			//Resume G2 search
			m_nMaxResults = m_pMatches->m_nGnutellaHits + Settings.Gnutella.MaxResults;
			m_nMaxQueryCount = pManaged->m_nQueryCount + Settings.Gnutella2.QueryLimit;

			//Resume ED2K search
			m_nMaxED2KResults = m_pMatches->m_nED2KHits + Settings.eDonkey.MaxResults;
			pManaged->m_tLastED2K = GetTickCount();
			pManaged->m_tMoreResults = 0;

			if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0x8000 )
				pManaged->SetPriority( CManagedSearch::spMedium );

			m_bUpdate = TRUE;
			UpdateMessages();
			return;
		}
	}
	// End of 'Search More'

	// Check if user mistakenly pasted download link to search input box
	CString sText;
	m_wndPanel.m_boxSearch.m_wndSearch.GetWindowText( sText );
	if ( theApp.OpenURL( sText, TRUE, TRUE ) )
	{
		m_wndPanel.m_boxSearch.m_wndSearch.SetWindowText( _T("") );
		return;
	}

	// Ask whether to clear previous searches.
	if ( m_pMatches->m_nFiles > 0 )
	{
		if ( MsgBox( IDS_SEARCH_CLEAR_PREVIOUS, MB_ICONQUESTION | MB_YESNO, 0,
			&Settings.Search.ClearPrevious ) == IDYES )
		{
			CQuickLock oLock( m_pMatches->m_pSection );
			m_pMatches->Clear();
			m_bUpdate = TRUE;
			PostMessage( WM_TIMER, 2 );
		}
	}

	CSearchPtr pManaged;

	if ( m_wndPanel.m_bSendSearch )
	{
		// Create new search
		pManaged = m_wndPanel.GetSearch();

		CSchemaPtr pSchema = pManaged->GetSchema();
		if ( m_pMatches->m_nFiles == 0 && pSchema )
		{
			CSchemaMemberList pColumns;
			CSchemaColumnsDlg::LoadColumns( pSchema, &pColumns );
			m_wndList.SelectSchema( pSchema, &pColumns );
		}
	}
	else
	{
		CNewSearchDlg dlg( NULL, GetLastSearch(), FALSE, TRUE );
		if ( dlg.DoModal() != IDOK )
			return;

		pManaged = new CManagedSearch( dlg.GetSearch() );
	}

	pManaged->CreateGUID();

	{
		CQuickLock oLock( m_pMatches->m_pSection );

		if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0x8000 )
		{
			for_each( begin(), end(), std::mem_fun( &CManagedSearch::Stop ) );
		}

		m_oSearches.push_back( pManaged );
	}

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

	m_nMaxResults		= 0;
	m_nMaxED2KResults	= 0;
	m_nMaxQueryCount	= 0;

	OnSearchStop();
}

void CSearchWnd::OnUpdateSearchStop(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bPaused );
}

void CSearchWnd::OnSearchStop()
{
	CQuickLock pLock( m_pMatches->m_pSection );

	if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0x8000 )
	{
		if ( ( !m_bPaused ) && ( !m_bWaitMore ) )
		{
			// Pause search
			if ( ! empty() )
			{
				m_oSearches.back()->SetActive( FALSE );
				m_bWaitMore = TRUE;
				m_bUpdate = TRUE;
				return;
			}
		}
	}

	for ( iterator pManaged = begin(); pManaged != end(); ++pManaged )
	{
		(*pManaged)->Stop();
		(*pManaged)->m_bReceive = FALSE;
	}

	m_bPaused = TRUE;

	m_wndPanel.Enable();
	UpdateMessages();

	SetModified();
}

void CSearchWnd::OnUpdateSearchPanel(CCmdUI* /*pCmdUI*/)
{
	CString sText;
	CCoolBarItem* pItem = m_wndToolBar.GetID( ID_SEARCH_PANEL );
	pItem->SetCheck( m_bPanel );
	LoadString( sText, m_bPanel ? IDS_SEARCH_PANEL_HIDE : IDS_SEARCH_PANEL_SHOW );
	pItem->SetTip( sText );
}

void CSearchWnd::OnSearchPanel()
{
	Settings.Search.SearchPanel = m_bPanel = ! m_bPanel;
	OnSkinChange();
	UpdateMessages();
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

CQuerySearchPtr CSearchWnd::GetLastSearch() const
{
	CQuickLock pLock( m_pMatches->m_pSection );

	return empty() ? CQuerySearchPtr() : m_oSearches.back()->GetSearch();
}

void CSearchWnd::ExecuteSearch()
{
	CQuickLock pLock( m_pMatches->m_pSection );

	CSearchPtr pManaged;
	if ( ! empty() )
		pManaged = m_oSearches.back();

	m_wndPanel.ShowSearch( pManaged );

	if ( pManaged )
	{
		CQuerySearchPtr pSearch = pManaged->GetSearch();
		if ( pSearch && pSearch->m_bAutostart )
		{
			if ( ! pSearch->CheckValid() )
			{
				// Invalid search, open help window
				CQuerySearch::SearchHelp();
			}
			else if ( AdultFilter.IsSearchFiltered( pSearch->m_sKeywords ) )
			{
				// Adult search blocked, open help window
				CHelpDlg::Show( _T("SearchHelp.AdultSearch") );
			}
			else
			{
				m_bPaused			= FALSE;
				m_tSearch			= GetTickCount();
				m_bWaitMore			= FALSE;

				pManaged->Stop();
				pManaged->Start();

				m_nMaxResults = m_pMatches->m_nGnutellaHits + Settings.Gnutella.MaxResults;
				m_nMaxED2KResults = m_pMatches->m_nED2KHits + Settings.eDonkey.MaxResults;
				m_nMaxQueryCount = pManaged->m_nQueryCount + Settings.Gnutella2.QueryLimit;

				m_wndPanel.Disable();

				if ( m_bPanel && Settings.Search.HideSearchPanel )
				{
					m_bPanel = FALSE;
					OnSkinChange();
				}
			}
		}
	}

	UpdateMessages();
}

void CSearchWnd::UpdateMessages()
{
	CQuickLock pLock( m_pMatches->m_pSection );

	CSearchPtr pManaged;
	if ( ! empty() )
		pManaged = m_oSearches.back();

	CString strCaption;
	Skin.LoadString( strCaption, IDR_SEARCHFRAME );
	if ( Settings.General.LanguageRTL ) strCaption = _T("\x200F") + strCaption + _T("\x202E");

	if ( pManaged )
	{
		CQuerySearchPtr pSearch = pManaged->GetSearch();
		if ( pSearch )
		{
			strCaption += _T(" : ");
			if ( Settings.General.LanguageRTL ) strCaption += _T("\x202B");

			if ( pSearch->HasHash() )
			{
				strCaption += _T( "Hash " ) + pSearch->GetURN();
			}
			else if ( pSearch->m_sSearch.GetLength() )
			{
				strCaption += pSearch->m_sSearch;
			}
			else if ( pSearch->m_pSchema && pSearch->m_pXML )
			{
				strCaption += pSearch->m_pSchema->GetIndexedWords(
					pSearch->m_pXML->GetFirstElement() );
			}

			if ( pSearch->m_pSchema )
			{
				strCaption += _T(" (") + pSearch->m_pSchema->m_sTitle + _T(")");
			}

			if ( m_pMatches->m_nFilteredFiles || m_pMatches->m_nFilteredHits )
			{
				CString strStats;
				strStats.Format( _T(" [%lu/%lu]"), m_pMatches->m_nFilteredFiles, m_pMatches->m_nFilteredHits );
				if ( Settings.General.LanguageRTL ) strStats = _T("\x200F") + strStats;
				strCaption += strStats;
				pManaged->m_nHits = m_pMatches->m_nFilteredHits;
			}
		}
		
		// Check for Hub/Leaf cache size changes
		if ( m_nCacheHubs != pManaged->m_nHubs ||
			 m_nCacheLeaves != pManaged->m_nLeaves )
		{
			m_nCacheHubs = pManaged->m_nHubs;
			m_nCacheLeaves = pManaged->m_nLeaves;
		}
	}

	CString strOld;
	GetWindowText( strOld );

	if ( strOld != strCaption )
	{
		SetWindowText( strCaption );
		m_sCaption = strCaption;
	}

	m_wndPanel.ShowStatus( ! m_bPaused, !m_bWaitMore,
		m_pMatches->m_nFilteredFiles,
		m_pMatches->m_nFilteredHits,
		m_nCacheHubs, m_nCacheLeaves );

	CRect rcList;
	m_wndList.GetWindowRect( &rcList );
	ScreenToClient( &rcList );
	if ( ( rcList.top == 0 ) != (m_bPaused||m_bWaitMore) ) OnSize( SIZE_INTERNAL, 0, 0 );

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

BOOL CSearchWnd::OnQueryHits(const CQueryHit* pHits)
{
	if ( m_bPaused || m_hWnd == NULL )
		return FALSE;

	CSingleLock pLock( &m_pMatches->m_pSection );
	if ( ! pLock.Lock( 250 ) || m_bPaused )
		return FALSE;

	for ( reverse_iterator pManaged = rbegin(); pManaged != rend(); ++pManaged )
	{
		if ( (*pManaged)->m_bReceive )
		{
			if ( (*pManaged)->IsEqualGUID( pHits->m_oSearchID ) ||	// The hits GUID matches the search
				 ( ! pHits->m_oSearchID && ( (*pManaged)->IsLastSearch() ) ) )	// The hits have no GUID and the search is the most recent text search
			{
				m_pMatches->AddHits( pHits, (*pManaged)->GetSearch() );
				m_bUpdate = TRUE;

				SetModified();

				if ( ( m_pMatches->m_nED2KHits >= m_nMaxED2KResults ) &&
					 ( (*pManaged)->m_tLastED2K != 0xFFFFFFFF ) )
				{
					if ( ! (*pManaged)->m_bAllowG2 ) //If G2 is not active, pause the search now.
					{
						m_bWaitMore = TRUE;
						(*pManaged)->SetActive( FALSE );
					}
					(*pManaged)->m_tLastED2K = 0xFFFFFFFF;
					theApp.Message( MSG_DEBUG, _T("ED2K Search Reached Maximum Number of Files") );
				}
#ifndef LAN_MODE
				if ( !m_bWaitMore && ( m_pMatches->m_nGnutellaHits >= m_nMaxResults ) )
				{
					m_bWaitMore = TRUE;
					(*pManaged)->SetActive( FALSE );
					theApp.Message( MSG_DEBUG, _T("Gnutella Search Reached Maximum Number of Files") );
				}
#endif // LAN_MODE
				return TRUE;
			}
		}
	}

	return FALSE;
}

void CSearchWnd::OnTimer(UINT_PTR nIDEvent)
{
	CSearchPtr pManaged;

	CSingleLock pLock( &m_pMatches->m_pSection );
	if ( pLock.Lock( 100 ) )
	{
		if ( ! empty() )
			pManaged = m_oSearches.back();

		if ( pManaged )
		{
			if ( pManaged->IsActive() &&
				 pManaged->m_nQueryCount > m_nMaxQueryCount )
			{
				m_bWaitMore = TRUE;
				pManaged->SetActive( FALSE );
				theApp.Message( MSG_DEBUG, _T("Search Reached Maximum Duration") );
				m_bUpdate = TRUE;
			}
			// We need to keep the lock for now- release after we update the progress panel
		}
		else
		{
			// We don't need to hold the lock
			pLock.Unlock();
		}
	}


	if ( ( IsPartiallyVisible() ) && ( nIDEvent == 1 ) )
	{
		if ( m_bSetFocus )
		{
			if ( m_bPanel && m_bPaused ) m_wndPanel.SetSearchFocus();
			else m_wndList.SetFocus();
			m_bSetFocus = FALSE;
		}

		UpdateMessages();
	}

	// Unlock if we were locked
	if ( pManaged )
		pLock.Unlock();

	CBaseMatchWnd::OnTimer( nIDEvent );

	if ( m_pMatches->m_nFilteredHits == 0 ) m_wndDetails.SetFile( NULL );
}

void CSearchWnd::OnSelChangeMatches()
{
	CQuickLock pLock( m_pMatches->m_pSection );

	m_wndDetails.SetFile( m_pMatches->GetSelectedFile( TRUE ) );
}

/////////////////////////////////////////////////////////////////////////////
// CSearchWnd serialize

void CSearchWnd::Serialize(CArchive& ar)
{
	CQuickLock pLock( m_pMatches->m_pSection );

	int nVersion = 1;

	if ( ar.IsStoring() )
	{
		ar << nVersion;

		ar.WriteCount( size() );

		for( iterator pManaged = begin(); pManaged != end(); ++pManaged )
		{
			(*pManaged)->Serialize( ar );
		}
	}
	else
	{
		ar >> nVersion;
		if ( nVersion != 1 ) AfxThrowUserException();

		for ( DWORD_PTR nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			CSearchPtr pManaged( new CManagedSearch() );
			pManaged->Serialize( ar );
			m_oSearches.push_back( pManaged );
		}
	}

	CBaseMatchWnd::Serialize( ar );

	if ( ar.IsLoading() )
	{
		if ( ! empty() )
			m_wndPanel.ShowSearch( m_oSearches.back() );

		PostMessage( WM_TIMER, 1 );
		SendMessage( WM_TIMER, 2 );
		SetAlert( FALSE );
	}
}

void CSearchWnd::OnUpdateFilters(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( TRUE );
}

void CSearchWnd::OnFilters(UINT nID)
{
	CQuickLock pLock( m_pMatches->m_pSection );

	int nFilter = nID - 3000;
	if ( nFilter < 0 || (DWORD)nFilter > m_pMatches->m_pResultFilters->m_nFilters - 1 ) return;

	m_pMatches->m_bFilterBusy		= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_bFilterBusy;
	m_pMatches->m_bFilterPush		= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_bFilterPush;
	m_pMatches->m_bFilterUnstable	= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_bFilterUnstable;
	m_pMatches->m_bFilterReject		= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_bFilterReject;
	m_pMatches->m_bFilterLocal		= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_bFilterLocal;
	m_pMatches->m_bFilterBogus		= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_bFilterBogus;
	m_pMatches->m_bFilterDRM		= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_bFilterDRM;
	m_pMatches->m_bFilterAdult		= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_bFilterAdult;
	m_pMatches->m_bFilterSuspicious = m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_bFilterSuspicious;
	m_pMatches->m_nFilterMinSize	= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_nFilterMinSize;
	m_pMatches->m_nFilterMaxSize	= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_nFilterMaxSize;
	m_pMatches->m_nFilterSources	= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_nFilterSources;
	m_pMatches->m_sFilter			= m_pMatches->m_pResultFilters->m_pFilters[ nFilter ]->m_sFilter;

	m_pMatches->Filter();
	Invalidate();
}
