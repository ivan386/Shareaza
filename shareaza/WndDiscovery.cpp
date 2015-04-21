//
// WndDiscovery.cpp
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
#include "CoolInterface.h"
#include "DiscoveryServices.h"
#include "DlgDiscoveryService.h"
#include "LiveList.h"
#include "Network.h"
#include "Skin.h"
#include "WndDiscovery.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const static UINT nImageID[] =
{
	IDR_HOSTCACHEFRAME,
	IDI_DISCOVERY_GRAY,
	IDR_DISCOVERYFRAME,
	IDI_WEB_URL,
	IDI_DISCOVERY_BLUE,
	IDI_FIREWALLED,
	NULL
};

IMPLEMENT_SERIAL(CDiscoveryWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CDiscoveryWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_NOTIFY(NM_DBLCLK, IDC_SERVICES, OnDblClkList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_SERVICES, OnSortList)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_QUERY, OnUpdateDiscoveryQuery)
	ON_COMMAND(ID_DISCOVERY_QUERY, OnDiscoveryQuery)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_REMOVE, OnUpdateDiscoveryRemove)
	ON_COMMAND(ID_DISCOVERY_REMOVE, OnDiscoveryRemove)
	ON_COMMAND(ID_DISCOVERY_ADD, OnDiscoveryAdd)
	ON_COMMAND(ID_DISCOVERY_EDIT, OnDiscoveryEdit)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_EDIT, OnUpdateDiscoveryEdit)
	ON_WM_CONTEXTMENU()
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_GNUTELLA, OnUpdateDiscoveryGnutella)
	ON_COMMAND(ID_DISCOVERY_GNUTELLA, OnDiscoveryGnutella)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_WEBCACHE, OnUpdateDiscoveryWebcache)
	ON_COMMAND(ID_DISCOVERY_WEBCACHE, OnDiscoveryWebcache)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_SERVERMET, OnUpdateDiscoveryServerMet)
	ON_COMMAND(ID_DISCOVERY_SERVERMET, OnDiscoveryServerMet)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_BLOCKED, OnUpdateDiscoveryBlocked)
	ON_COMMAND(ID_DISCOVERY_BLOCKED, OnDiscoveryBlocked)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_ADVERTISE, OnUpdateDiscoveryAdvertise)
	ON_COMMAND(ID_DISCOVERY_ADVERTISE, OnDiscoveryAdvertise)
	ON_UPDATE_COMMAND_UI(ID_DISCOVERY_BROWSE, OnUpdateDiscoveryBrowse)
	ON_COMMAND(ID_DISCOVERY_BROWSE, OnDiscoveryBrowse)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SERVICES, OnCustomDrawList)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryWnd construction

CDiscoveryWnd::CDiscoveryWnd()
	: m_bShowGnutella	( TRUE )
	, m_bShowWebCache	( TRUE )
	, m_bShowServerMet	( TRUE )
	, m_bShowBlocked	( TRUE )
{
	Create( IDR_DISCOVERYFRAME );
}

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryWnd message handlers

int CDiscoveryWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	m_wndList.Create( WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE |
		LVS_AUTOARRANGE | LVS_REPORT | LVS_SHOWSELALWAYS,
		rectDefault, this, IDC_SERVICES, 11 );
	m_pSizer.Attach( &m_wndList );
	
	m_wndList.SetExtendedStyle(
		LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP|LVS_EX_SUBITEMIMAGES );

	m_wndList.InsertColumn( 0, _T("Address"), LVCFMT_LEFT, 260, -1 );
	m_wndList.InsertColumn( 1, _T("Type"), LVCFMT_CENTER, 80, 0 );
	m_wndList.InsertColumn( 2, _T("Last Access"), LVCFMT_CENTER, 130, 1 );
	m_wndList.InsertColumn( 3, _T("Hosts"), LVCFMT_CENTER, 50, 2 );
	m_wndList.InsertColumn( 4, _T("Total Hosts"), LVCFMT_CENTER, 70, 3 );
	m_wndList.InsertColumn( 5, _T("URLs"), LVCFMT_CENTER, 50, 4 );
	m_wndList.InsertColumn( 6, _T("Total URLs"), LVCFMT_CENTER, 70, 5 );
	m_wndList.InsertColumn( 7, _T("Accesses"), LVCFMT_CENTER, 70, 6 );
	m_wndList.InsertColumn( 8, _T("Updates"), LVCFMT_CENTER, 55, 7 );
	m_wndList.InsertColumn( 9, _T("Failures"), LVCFMT_CENTER, 55, 8 );
	m_wndList.InsertColumn( 10, _T("Pong"), LVCFMT_CENTER, 150, 9 );
	
	LoadState( _T("CDiscoveryWnd"), TRUE );

	PostMessage( WM_TIMER, 1 );
	
	return 0;
}

void CDiscoveryWnd::OnDestroy() 
{
	DiscoveryServices.Save();

	Settings.SaveList( _T("CDiscoveryWnd"), &m_wndList );		
	SaveState( _T("CDiscoveryWnd") );

	CPanelWnd::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryWnd operations

void CDiscoveryWnd::Update()
{
	CSingleLock pLock( &Network.m_pSection, FALSE );
	if ( ! pLock.Lock( 250 ) )
		return;

	for ( POSITION pos = DiscoveryServices.GetIterator() ; pos ; )
	{
		CDiscoveryService* pService = DiscoveryServices.GetNext( pos );

		CLiveItem* pItem = NULL;

		if ( pService->m_nType == CDiscoveryService::dsGnutella )
		{
			if ( ! m_bShowGnutella ) continue;
			pItem = m_wndList.Add( pService );
			pItem->Set( 1, _T("Gnutella Bootstrap") );
			pItem->SetImage( 0, 0 );
		}
		else if ( pService->m_nType == CDiscoveryService::dsWebCache )
		{
			if ( ! m_bShowWebCache ) continue;
			pItem = m_wndList.Add( pService );
			if ( pService->m_bGnutella2 && pService->m_bGnutella1 )
			{
				pItem->Set( 1, _T("G1/G2 GWebCache") );
				pItem->SetImage( 0, 2 );			// Multi-coloured icon
			}
			else
			{
				ASSERT( pService->m_bGnutella2 || pService->m_bGnutella1 );
				if ( pService->m_bGnutella2 )
				{
					pItem->Set( 1, _T("G2 GWebCache") );
					pItem->SetImage( 0, 4 );		// Blue icon
				}
				else
				{
					pItem->Set( 1, _T("G1 GWebCache") );
					pItem->SetImage( 0, 1 );		// Grey icon
				}
			}
		}
		else if ( pService->m_nType == CDiscoveryService::dsServerMet )
		{
			if ( ! m_bShowServerMet ) continue;
			pItem = m_wndList.Add( pService );
			pItem->Set( 1, _T("Server.met URL") );
			pItem->SetImage( 0, 3 );				// URL icon
		}
		else if ( pService->m_nType == CDiscoveryService::dsDCHubList )
		{
			if ( ! m_bShowServerMet ) continue;
			pItem = m_wndList.Add( pService );
			pItem->Set( 1, _T( "DC++ Hub List URL" ) );
			pItem->SetImage( 0, 3 );				// URL icon
		}
		else if ( pService->m_nType == CDiscoveryService::dsBlocked )
		{
			if ( ! m_bShowBlocked ) continue;
			pItem = m_wndList.Add( pService );
			pItem->Set( 1, _T("Blocked") );
			pItem->SetImage( 0, 5 );				// Block icon
		}
		else
		{
			continue;
		}
		
		pItem->Set( 0, pService->m_sAddress );
		
		if ( pService->m_tAccessed )
		{
			CTime pTime( (time_t)pService->m_tAccessed );
			pItem->Set( 2, pTime.Format( _T("%Y-%m-%d %H:%M:%S") ) );
		}
		else
		{
			if ( pService->m_nType != CDiscoveryService::dsBlocked )
				pItem->Set( 2, _T("0 - Never") );
		}
		
		if ( pService->m_nType != CDiscoveryService::dsBlocked )
		{
			pItem->Format( 7, _T("%u"), pService->m_nAccesses );
			pItem->Format( 9, _T("%u"), pService->m_nFailures );
			
			if ( pService->m_tAccessed )
			{
				pItem->Format( 3, _T("%u"), pService->m_nHosts );
				pItem->Format( 4, _T("%u"), pService->m_nTotalHosts );
				pItem->Format( 8, _T("%u"), pService->m_nUpdates );
				pItem->Format( 5, _T("%u"), pService->m_nURLs );
				pItem->Format( 6, _T("%u"), pService->m_nTotalURLs );
				
				if ( ( ! pService->m_sPong.IsEmpty() ) && pService->m_nType == CDiscoveryService::dsWebCache && pService->m_bGnutella2 )
				{
					pItem->Set( 10, pService->m_sPong );
				}
			}
		}
	}
	
	m_wndList.Apply();
}

CDiscoveryService* CDiscoveryWnd::GetItem(int nItem)
{
	if ( nItem >= 0 && m_wndList.GetItemState( nItem, LVIS_SELECTED ) )
	{
		CDiscoveryService* pService = (CDiscoveryService*)m_wndList.GetItemData( nItem );
		if ( DiscoveryServices.Check( pService ) ) return pService;
	}
	
	return NULL;
}

void CDiscoveryWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();

	// Columns
	Settings.LoadList( _T("CDiscoveryWnd"), &m_wndList, 3 );

	// Fonts
	m_wndList.SetFont( &theApp.m_gdiFont );

	// Icons
	CoolInterface.LoadIconsTo( m_gdiImageList, nImageID );
	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );
}

void CDiscoveryWnd::OnSize(UINT nType, int cx, int cy) 
{
	CPanelWnd::OnSize(nType, cx, cy);
	m_pSizer.Resize( cx );
	m_wndList.SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOZORDER );
}

void CDiscoveryWnd::OnTimer(UINT_PTR nIDEvent) 
{
	if ( nIDEvent == 1 ) Update();
}

void CDiscoveryWnd::OnDblClkList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	OnDiscoveryEdit();
	*pResult = 0;
}

void CDiscoveryWnd::OnSortList(NMHDR* pNotifyStruct, LRESULT *pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNotifyStruct;
	CLiveList::Sort( &m_wndList, pNMListView->iSubItem );
	*pResult = 0;
}

void CDiscoveryWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point) 
{
	Skin.TrackPopupMenu( _T("CDiscoveryWnd"), point, ID_DISCOVERY_EDIT );
}

void CDiscoveryWnd::OnUpdateDiscoveryQuery(CCmdUI* pCmdUI) 
{
	CSingleLock pLock( &Network.m_pSection );

	if (  m_wndList.GetSelectedCount() == 1 && pLock.Lock( 100 ) )
	{
		CDiscoveryService* pService = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
		if ( pService && pService->m_nType != CDiscoveryService::dsBlocked )
		{
			pCmdUI->Enable( TRUE );
			return;
		}
	}

	pCmdUI->Enable( FALSE );
}

void CDiscoveryWnd::OnDiscoveryQuery() 
{
	CSingleLock pLock( &Network.m_pSection, FALSE );
	if ( ! pLock.Lock( 250 ) )
		return;
	
	for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVIS_SELECTED ) ) >= 0 ; )
	{
		CDiscoveryService* pService = GetItem( nItem );
		if ( pService && pService->m_nType != CDiscoveryService::dsBlocked )
		{
			DiscoveryServices.Query( pService, ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) ? CDiscoveryServices::wcmCaches : CDiscoveryServices::wcmHosts );
			break;
		}
	}
}

void CDiscoveryWnd::OnUpdateDiscoveryAdvertise(CCmdUI* pCmdUI) 
{
	CSingleLock pLock( &Network.m_pSection );

	if ( pLock.Lock( 100 ) )
	{
		CDiscoveryService* pService = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
		pCmdUI->Enable( pService && pService->m_nType == CDiscoveryService::dsWebCache );
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}
}

void CDiscoveryWnd::OnDiscoveryAdvertise() 
{
	CSingleLock pLock( &Network.m_pSection, FALSE );
	if ( ! pLock.Lock( 250 ) )
		return;

	CDiscoveryService* pService = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
	if ( pService && pService->m_nType == CDiscoveryService::dsWebCache )
	{
		DiscoveryServices.Query( pService, CDiscoveryServices::wcmSubmit );
	}
}

void CDiscoveryWnd::OnUpdateDiscoveryBrowse(CCmdUI* pCmdUI) 
{
	OnUpdateDiscoveryAdvertise( pCmdUI );
}

void CDiscoveryWnd::OnDiscoveryBrowse() 
{
	CSingleLock pLock( &Network.m_pSection, FALSE );
	if ( ! pLock.Lock( 250 ) )
		return;

	CDiscoveryService* pService = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
	CString strURL;

	if ( pService && pService->m_nType == CDiscoveryService::dsWebCache )
		strURL = pService->m_sAddress;

	pLock.Unlock();

	if ( strURL.GetLength() )
	{
		ShellExecute( GetSafeHwnd(), _T("open"), strURL, NULL, NULL, SW_SHOWNORMAL );
	}
}

void CDiscoveryWnd::OnUpdateDiscoveryRemove(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 );
}

void CDiscoveryWnd::OnDiscoveryRemove() 
{
	if ( m_wndList.GetSelectedCount() <= 0 )
		return;

	CSingleLock pLock( &Network.m_pSection, FALSE );
	if ( ! pLock.Lock( 250 ) )
		return;

	for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVIS_SELECTED ) ) >= 0 ; )
	{
		CDiscoveryService* pService = GetItem( nItem );
		if ( pService ) pService->Remove( FALSE );
	}

	DiscoveryServices.CheckMinimumServices();
	
	m_wndList.ClearSelection();

	Update();
}

void CDiscoveryWnd::OnUpdateDiscoveryEdit(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() == 1 );
}

void CDiscoveryWnd::OnDiscoveryEdit() 
{
	CSingleLock pLock( &Network.m_pSection, FALSE );
	if ( ! pLock.Lock( 250 ) )
		return;

	CDiscoveryService* pService = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
	if ( ! pService ) return;

	pLock.Unlock();

	CDiscoveryServiceDlg dlg( NULL, pService );

	if ( dlg.DoModal() == IDOK ) Update();
}

void CDiscoveryWnd::OnUpdateDiscoveryGnutella(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_bShowGnutella );
}

void CDiscoveryWnd::OnDiscoveryGnutella() 
{
	m_bShowGnutella = ! m_bShowGnutella;
	Update();
}

void CDiscoveryWnd::OnUpdateDiscoveryWebcache(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_bShowWebCache );
}

void CDiscoveryWnd::OnDiscoveryWebcache() 
{
	m_bShowWebCache = ! m_bShowWebCache;
	Update();
}

void CDiscoveryWnd::OnUpdateDiscoveryServerMet(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_bShowServerMet );
}

void CDiscoveryWnd::OnDiscoveryServerMet() 
{
	m_bShowServerMet = ! m_bShowServerMet;
	Update();
}

void CDiscoveryWnd::OnUpdateDiscoveryBlocked(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_bShowBlocked );
}

void CDiscoveryWnd::OnDiscoveryBlocked() 
{
	m_bShowBlocked = ! m_bShowBlocked;
	Update();
}

void CDiscoveryWnd::OnDiscoveryAdd() 
{
	CDiscoveryServiceDlg dlg;

	if ( dlg.DoModal() == IDOK ) Update();
}

void CDiscoveryWnd::OnCustomDrawList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = CDRF_DODEFAULT;
}

BOOL CDiscoveryWnd::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->wParam == 'A' && GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
		{
			for ( int nItem = m_wndList.GetItemCount() - 1; nItem >= 0; nItem-- )
			{
				m_wndList.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
			}
			return TRUE;
		}
		else if ( pMsg->wParam == VK_DELETE )
		{
			PostMessage( WM_COMMAND, ID_DISCOVERY_REMOVE );
			return TRUE;
		}
		else if ( pMsg->wParam == VK_INSERT )
		{
			PostMessage( WM_COMMAND, ID_DISCOVERY_ADD );
			return TRUE;
		}
		else if ( pMsg->wParam == VK_RETURN )
		{
			PostMessage( WM_COMMAND, ID_DISCOVERY_EDIT );
			return TRUE;
		}
	}

	return CPanelWnd::PreTranslateMessage( pMsg );
}
