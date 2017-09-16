//
// WndNeighbours.cpp
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
#include "ChatCore.h"
#include "ChatWindows.h"
#include "CoolInterface.h"
#include "DCNeighbour.h"
#include "DlgHex.h"
#include "DlgSettingsManager.h"
#include "DlgURLCopy.h"
#include "EDNeighbour.h"
#include "EDPacket.h"
#include "Flags.h"
#include "G1Neighbour.h"
#include "G2Neighbour.h"
#include "GProfile.h"
#include "HostCache.h"
#include "LiveList.h"
#include "Neighbours.h"
#include "Network.h"
#include "Security.h"
#include "Skin.h"
#include "WindowManager.h"
#include "WndBrowseHost.h"
#include "WndMain.h"
#include "WndNeighbours.h"
#include "WndPacket.h"
#include "WndSystem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CNeighboursWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CNeighboursWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
	ON_WM_DESTROY()
	ON_WM_ACTIVATE()
	ON_WM_QUERYNEWPALETTE()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_NEIGHBOURS, OnSortList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_NEIGHBOURS, OnCustomDrawList)
	ON_UPDATE_COMMAND_UI(ID_NEIGHBOURS_DISCONNECT, OnUpdateNeighboursDisconnect)
	ON_COMMAND(ID_NEIGHBOURS_DISCONNECT, OnNeighboursDisconnect)
	ON_UPDATE_COMMAND_UI(ID_NEIGHBOURS_VIEW_ALL, OnUpdateNeighboursViewAll)
	ON_COMMAND(ID_NEIGHBOURS_VIEW_ALL, OnNeighboursViewAll)
	ON_UPDATE_COMMAND_UI(ID_NEIGHBOURS_VIEW_INCOMING, OnUpdateNeighboursViewIncoming)
	ON_COMMAND(ID_NEIGHBOURS_VIEW_INCOMING, OnNeighboursViewIncoming)
	ON_UPDATE_COMMAND_UI(ID_NEIGHBOURS_VIEW_OUTGOING, OnUpdateNeighboursViewOutgoing)
	ON_COMMAND(ID_NEIGHBOURS_VIEW_OUTGOING, OnNeighboursViewOutgoing)
	ON_UPDATE_COMMAND_UI(ID_NEIGHBOURS_CHAT, OnUpdateNeighboursChat)
	ON_COMMAND(ID_NEIGHBOURS_CHAT, OnNeighboursChat)
	ON_UPDATE_COMMAND_UI(ID_SECURITY_BAN, OnUpdateSecurityBan)
	ON_COMMAND(ID_SECURITY_BAN, OnSecurityBan)
	ON_UPDATE_COMMAND_UI(ID_BROWSE_LAUNCH, OnUpdateBrowseLaunch)
	ON_COMMAND(ID_BROWSE_LAUNCH, OnBrowseLaunch)
	ON_UPDATE_COMMAND_UI(ID_NEIGHBOURS_COPY, OnUpdateNeighboursCopy)
	ON_COMMAND(ID_NEIGHBOURS_COPY, OnNeighboursCopy)
	ON_COMMAND(ID_NEIGHBOURS_SETTINGS, OnNeighboursSettings)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CNeighboursWnd construction

CNeighboursWnd::CNeighboursWnd()
	: CPanelWnd( TRUE, TRUE )
	, m_tLastUpdate( 0 )
{
	Create( IDR_NEIGHBOURSFRAME );
}

UINT CNeighboursWnd::GetSelectedCount() const
{
	static DWORD tLastUpdate = 0;
	static UINT nCount = 0;
	DWORD tNow = GetTickCount();
	if ( tNow > tLastUpdate + 250 || tNow < tLastUpdate )
	{
		tLastUpdate = tNow;
		nCount = m_wndList.GetSelectedCount();
	}
	return nCount;
}

/////////////////////////////////////////////////////////////////////////////
// CNeighboursWnd system message handlers

int CNeighboursWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	if ( ! m_wndToolBar.Create( this, WS_CHILD|WS_VISIBLE|CBRS_NOALIGN, AFX_IDW_TOOLBAR ) ) return -1;
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP );

	m_wndList.Create( WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE |
		LVS_AUTOARRANGE | LVS_REPORT | LVS_SHOWSELALWAYS,
		rectDefault, this, IDC_NEIGHBOURS );
	m_pSizer.Attach( &m_wndList );

	m_wndTip.Create( &m_wndList, &Settings.Interface.TipNeighbours );
	m_wndList.SetTip( &m_wndTip );

	m_wndList.SetExtendedStyle(
		LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP|LVS_EX_SUBITEMIMAGES );

	m_wndList.InsertColumn( 0, _T("Address"), LVCFMT_LEFT, 110, -1 );
	m_wndList.InsertColumn( 1, _T("Port"), LVCFMT_CENTER, 45, 0 );
	m_wndList.InsertColumn( 2, _T("Time"), LVCFMT_CENTER, 80, 1 );
	m_wndList.InsertColumn( 3, _T("Packets"), LVCFMT_CENTER, 80, 2 );
	m_wndList.InsertColumn( 4, _T("Bandwidth"), LVCFMT_CENTER, 80, 3 );
	m_wndList.InsertColumn( 5, _T("Total"), LVCFMT_CENTER, 85, 4 );
	m_wndList.InsertColumn( 6, _T("Flow"), LVCFMT_CENTER, 0, 5 );
	m_wndList.InsertColumn( 7, _T("Leaves"), LVCFMT_CENTER, 45, 6 );
	m_wndList.InsertColumn( 8, _T("Mode"), LVCFMT_CENTER, 60, 7 );
	m_wndList.InsertColumn( 9, _T("Client"), LVCFMT_LEFT, 100, 8 );
	m_wndList.InsertColumn( 10, _T("Name"), LVCFMT_LEFT, 100, 9 );
	m_wndList.InsertColumn( 11, _T("Country"), LVCFMT_LEFT, 40, 10 );

	LoadState( _T("CNeighboursWnd"), FALSE );

	PostMessage( WM_TIMER, 1 );

	return 0;
}

void CNeighboursWnd::OnDestroy()
{
	Settings.SaveList( _T("CNeighboursWnd"), &m_wndList );
	SaveState( _T("CNeighboursWnd") );
	CPanelWnd::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CNeighboursWnd operations

void CNeighboursWnd::Update()
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 50 ) ) return;

	CLiveList pLiveList( 12 );

	m_tLastUpdate = GetTickCount();

	for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
	{
		CString str;
		CNeighbour* pNeighbour = Neighbours.GetNext( pos );
		CLiveItem* pItem = pLiveList.Add( pNeighbour );

		pItem->Set( 0, pNeighbour->m_sAddress );
		pItem->Format( 1, _T("%hu"), htons( pNeighbour->m_pHost.sin_port ) );

		DWORD nTime = ( m_tLastUpdate - pNeighbour->m_tConnected ) / 1000;

		switch ( pNeighbour->m_nState )
		{
		case nrsConnecting:
			LoadString ( str,IDS_NEIGHBOUR_CONNECTING );
			break;
		case nrsHandshake1:
		case nrsHandshake2:
		case nrsHandshake3:
			LoadString ( str,IDS_NEIGHBOUR_HANDSHAKING );
			break;
		case nrsRejected:
			LoadString ( str,IDS_NEIGHBOUR_REJECTED );
			break;
		case nrsClosing:
			LoadString ( str,IDS_NEIGHBOUR_CLOSING );
			break;
		case nrsConnected:
			if ( nTime > 86400 )
				str.Format( _T("%u:%.2u:%.2u:%.2u"), nTime / 86400, ( nTime / 3600 ) % 24, ( nTime / 60 ) % 60, nTime % 60 );
			else
				str.Format( _T("%u:%.2u:%.2u"), nTime / 3600, ( nTime / 60 ) % 60, nTime % 60 );
			break;
		case nrsNull:
		default:
			LoadString ( str,IDS_NEIGHBOUR_UNKNOWN );
			break;
		}

		pItem->Set( 2, ( pNeighbour->m_nState == nrsConnected ) ? str :
			( _T(" ") + str + _T(" ") ) );

		pNeighbour->Measure();

		pItem->Format( 3, _T("%u - %u"), pNeighbour->m_nInputCount, pNeighbour->m_nOutputCount );
		pItem->Format( 4, _T("%s - %s"),
			(LPCTSTR)Settings.SmartSpeed( pNeighbour->m_mInput.nMeasure ),
			(LPCTSTR)Settings.SmartSpeed( pNeighbour->m_mOutput.nMeasure ) );
		pItem->Format( 5, _T("%s - %s"),
			(LPCTSTR)Settings.SmartVolume( pNeighbour->m_mInput.nTotal ),
			(LPCTSTR)Settings.SmartVolume( pNeighbour->m_mOutput.nTotal ) );
		pItem->Format( 6, _T("%u (%u)"), pNeighbour->m_nOutbound, pNeighbour->m_nLostCount );

		if ( pNeighbour->m_nState >= nrsConnected )
		{
			pItem->SetImage( 0, pNeighbour->m_nProtocol );

			if ( pNeighbour->GetUserCount() )
			{
				if ( pNeighbour->GetUserLimit() )
				{
					pItem->Format( 7, _T("%u/%u"), pNeighbour->GetUserCount(), pNeighbour->GetUserLimit() );
				}
				else
				{
					pItem->Format( 7, _T("%u"), pNeighbour->GetUserCount() );
				}
			}
		}
		else
		{
			pItem->SetImage( 0, PROTOCOL_NULL );
		}

		pItem->Set( 8, Neighbours.GetName( pNeighbour ) );
		pItem->Set( 9, Neighbours.GetAgent( pNeighbour ) );
		pItem->Set( 10, Neighbours.GetNick( pNeighbour ) );

		pItem->Set( 11, pNeighbour->m_sCountry );
		int nFlag = Flags.GetFlagIndex( pNeighbour->m_sCountry );
		if ( nFlag >= 0 )
			pItem->SetImage( 11, PROTOCOL_LAST + nFlag );
	}

	pLiveList.Apply( &m_wndList, TRUE );
}

CNeighbour* CNeighboursWnd::GetItem(int nItem)
{
	if ( m_wndList.GetItemState( nItem, LVIS_SELECTED ) )
	{
		return Neighbours.Get( m_wndList.GetItemData( nItem ) );
	}

	return NULL;
}

void CNeighboursWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();

	// Columns
	Settings.LoadList( _T("CNeighboursWnd"), &m_wndList );

	// Toolbar
	Skin.CreateToolBar( _T("CNeighboursWnd"), &m_wndToolBar );

	// Fonts
	m_wndList.SetFont( &theApp.m_gdiFont );

	// Icons (merge protocols and flags in one image list)
	CoolInterface.LoadProtocolIconsTo( m_gdiImageList );
	int nImages = m_gdiImageList.GetImageCount();
	int nFlags = Flags.GetCount();
	VERIFY( m_gdiImageList.SetImageCount( nImages + nFlags ) );
	for ( int nFlag = 0 ; nFlag < nFlags ; nFlag++ )
	{
		if ( HICON hIcon = Flags.ExtractIcon( nFlag ) )
		{
			VERIFY( m_gdiImageList.Replace( nImages + nFlag, hIcon ) != -1 );
			VERIFY( DestroyIcon( hIcon ) );
		}
	}
	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );
}

/////////////////////////////////////////////////////////////////////////////
// CNeighboursWnd message handlers

BOOL CNeighboursWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if ( m_wndToolBar.m_hWnd )
	{
		if ( m_wndToolBar.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) ) return TRUE;
	}

	return CPanelWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}

void CNeighboursWnd::OnSize(UINT nType, int cx, int cy)
{
	CPanelWnd::OnSize( nType, cx, cy );

	BOOL bSized = m_pSizer.Resize( cx );

	SizeListAndBar( &m_wndList, &m_wndToolBar );

	if ( bSized && m_wndList.GetItemCount() == 0 ) m_wndList.Invalidate();
}

void CNeighboursWnd::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent == 1 )
	{
		if ( ( IsPartiallyVisible() ) || ( GetTickCount() - m_tLastUpdate > 30000 ) )
			 Update();
	}
}

void CNeighboursWnd::OnSortList(NMHDR* pNotifyStruct, LRESULT *pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNotifyStruct;
	CLiveList::Sort( &m_wndList, pNMListView->iSubItem );
	*pResult = 0;
}

void CNeighboursWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	Skin.TrackPopupMenu( _T("CNeighboursWnd"), point );
}

void CNeighboursWnd::OnUpdateNeighboursDisconnect(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() > 0 );
}

void CNeighboursWnd::OnNeighboursDisconnect()
{
	CSingleLock pLock( &Network.m_pSection, TRUE );

	for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVNI_SELECTED ) ) >= 0 ; )
	{
		if ( CNeighbour* pNeighbour = GetItem( nItem ) )
		{
			pNeighbour->Close( IDS_CONNECTION_CLOSED );
		}
	}
}

void CNeighboursWnd::OnUpdateNeighboursCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() == 1 );

}

void CNeighboursWnd::OnNeighboursCopy()
{
	CSingleLock pLock( &Network.m_pSection, TRUE );

	CNeighbour* pNeighbour = GetItem( m_wndList.GetNextItem( -1, LVNI_SELECTED ) );
	if ( ! pNeighbour ) return;

	CString strURL;

	if ( pNeighbour->m_nProtocol == PROTOCOL_G1 || pNeighbour->m_nProtocol == PROTOCOL_G2 )
	{
		strURL.Format( _T("gnutella:host:%s:%u"),
			(LPCTSTR)pNeighbour->m_sAddress, htons( pNeighbour->m_pHost.sin_port ) );
	}
	else if ( pNeighbour->m_nProtocol == PROTOCOL_ED2K )
	{
		strURL.Format( _T("ed2k://|server|%s|%u|/"),
			(LPCTSTR)pNeighbour->m_sAddress, htons( pNeighbour->m_pHost.sin_port ) );
	}
	else if ( pNeighbour->m_nProtocol == PROTOCOL_DC )
	{
		strURL.Format( _T("dchub://%s:%u/"),
			(LPCTSTR)pNeighbour->m_sAddress, htons( pNeighbour->m_pHost.sin_port ) );
	}

	theApp.SetClipboardText( strURL );
}

void CNeighboursWnd::OnUpdateNeighboursChat(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;
	if ( GetSelectedCount() && Settings.Community.ChatEnable )
	{
		CSingleLock pNetworkLock( &Network.m_pSection );
		if ( pNetworkLock.Lock( 500 ) )
		{
			if ( CNeighbour* pNeighbour = GetItem( m_wndList.GetNextItem( -1, LVNI_SELECTED ) ) )
			{
				bEnable = (
					pNeighbour->m_nProtocol == PROTOCOL_G1 ||
					pNeighbour->m_nProtocol == PROTOCOL_G2 ||
					pNeighbour->m_nProtocol == PROTOCOL_DC );
			}
		}
	}
	pCmdUI->Enable( bEnable );
}

void CNeighboursWnd::OnNeighboursChat()
{
	if ( GetSelectedCount() && Settings.Community.ChatEnable )
	{
		CSingleLock pLock( &Network.m_pSection, TRUE );

		for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVNI_SELECTED ) ) >= 0 ; )
		{
			if ( CNeighbour* pNeighbour = GetItem( nItem ) )
			{
				switch ( pNeighbour->m_nProtocol )
				{
				case PROTOCOL_G1:
				case PROTOCOL_G2:
					ChatWindows.OpenPrivate( pNeighbour->m_oGUID,
						&pNeighbour->m_pHost, FALSE, pNeighbour->m_nProtocol );
					break;

				case PROTOCOL_DC:
					ChatCore.OnMessage( static_cast< CDCNeighbour*>( pNeighbour ) );
					break;

				default:
					;
				}
			}
		}
	}
}

void CNeighboursWnd::OnUpdateSecurityBan(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() );
}

void CNeighboursWnd::OnSecurityBan()
{
	CSingleLock pLock( &Network.m_pSection, TRUE );

	for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVNI_SELECTED ) ) >= 0 ; )
	{
		if ( CNeighbour* pNeighbour = GetItem( nItem ) )
		{
			IN_ADDR pAddress = pNeighbour->m_pHost.sin_addr;
			pNeighbour->Close();
			pLock.Unlock();
			Security.Ban( &pAddress, banSession );
			pLock.Lock();
		}
	}
}

void CNeighboursWnd::OnUpdateBrowseLaunch(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;
	if ( GetSelectedCount() == 1 )
	{
		CSingleLock pNetworkLock( &Network.m_pSection );
		if ( pNetworkLock.Lock( 500 ) )
		{
			if ( CNeighbour* pNeighbour = GetItem( m_wndList.GetNextItem( -1, LVNI_SELECTED ) ) )
			{
				bEnable = (
					pNeighbour->m_nProtocol == PROTOCOL_G1 ||
					pNeighbour->m_nProtocol == PROTOCOL_G2 );
			}
		}
	}
	pCmdUI->Enable( bEnable );
}

void CNeighboursWnd::OnBrowseLaunch()
{
	CSingleLock pLock( &Network.m_pSection, TRUE );

	if ( CNeighbour* pNeighbour = GetItem( m_wndList.GetNextItem( -1, LVNI_SELECTED ) ) )
	{
		if ( pNeighbour->m_nProtocol == PROTOCOL_G1 ||
			 pNeighbour->m_nProtocol == PROTOCOL_G2 )
		{
			PROTOCOLID nProtocol = pNeighbour->m_nProtocol;
			SOCKADDR_IN pAddress = pNeighbour->m_pHost;
			Hashes::Guid oGUID = pNeighbour->m_oGUID;

			pLock.Unlock();

			new CBrowseHostWnd( nProtocol, &pAddress, FALSE, oGUID );
		}
	}
}

void CNeighboursWnd::OnUpdateNeighboursViewAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() == 1 );
}

void CNeighboursWnd::OnNeighboursViewAll()
{
	OpenPacketWnd( TRUE, TRUE );
}

void CNeighboursWnd::OnUpdateNeighboursViewIncoming(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() == 1 );
}

void CNeighboursWnd::OnNeighboursViewIncoming()
{
	OpenPacketWnd( TRUE, FALSE );
}

void CNeighboursWnd::OnUpdateNeighboursViewOutgoing(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() == 1 );
}

void CNeighboursWnd::OnNeighboursViewOutgoing()
{
	OpenPacketWnd( FALSE, TRUE );
}

void CNeighboursWnd::OnNeighboursSettings()
{
	CSettingsManagerDlg::Run( _T("CNetworksSettingsPage") );
}

void CNeighboursWnd::OpenPacketWnd(BOOL bIncoming, BOOL bOutgoing)
{
	CSingleLock pLock( &Network.m_pSection, TRUE );

	CWindowManager* pManager = GetManager();
	CPacketWnd* pWnd = NULL;

	while ( ( pWnd = (CPacketWnd*)pManager->Find( RUNTIME_CLASS(CPacketWnd), pWnd ) ) != NULL )
	{
		if ( pWnd->m_pOwner == this ) break;
	}

	if ( ! pWnd ) pWnd = new CPacketWnd( this );

	for ( int nItem = 0 ; nItem < m_wndList.GetItemCount() ; nItem++ )
	{
		if ( CNeighbour* pNeighbour = GetItem( nItem ) )
		{
			pWnd->m_nInputFilter	= bIncoming ? (DWORD_PTR)pNeighbour : 1;
			pWnd->m_nOutputFilter	= bOutgoing ? (DWORD_PTR)pNeighbour : 1;
		}
	}

	pWnd->m_bPaused = FALSE;
	pWnd->BringWindowToTop();
}

void CNeighboursWnd::OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)pNMHDR;

	if ( ! ::IsWindow( m_wndList.GetSafeHwnd() ) ) return;

	if ( pDraw->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		if ( m_wndList.GetItemCount() == 0 && ! Network.IsConnected() )
		{
			DrawEmptyMessage( CDC::FromHandle( pDraw->nmcd.hdc ) );
		}

		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( pDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
	{
		LV_ITEM pItem = { LVIF_IMAGE, static_cast< int >( pDraw->nmcd.dwItemSpec ) };
		m_wndList.GetItem( &pItem );

		int nImage = pItem.iImage;
		switch ( nImage )
		{
		case PROTOCOL_NULL:
			pDraw->clrText = CoolInterface.m_crNetworkNull ;
			break;
		case PROTOCOL_G1:
			pDraw->clrText = CoolInterface.m_crNetworkG1 ;
			break;
		case PROTOCOL_G2:
			pDraw->clrText = CoolInterface.m_crNetworkG2 ;
			break;
		case PROTOCOL_ED2K:
			pDraw->clrText = CoolInterface.m_crNetworkED2K ;
			break;
		case PROTOCOL_DC:
			pDraw->clrText = CoolInterface.m_crNetworkDC ;
			break;
		}

		*pResult = CDRF_NOTIFYPOSTPAINT;
	}
}

void CNeighboursWnd::DrawEmptyMessage(CDC* pDC)
{
	CRect rcClient, rcText;
	CString strText;

	m_wndList.GetClientRect( &rcClient );

	if ( CWnd* pHeader = m_wndList.GetWindow( GW_CHILD ) )
	{
		pHeader->GetWindowRect( &rcText );
		rcClient.top += rcText.Height();
	}

	rcText.SetRect( rcClient.left, 16, rcClient.right, 0 );
	rcText.bottom = ( rcClient.top + rcClient.bottom ) / 2;
	rcText.top = rcText.bottom - rcText.top;

	pDC->SetBkMode( TRANSPARENT );
	CFont* pOldFont = (CFont*)pDC->SelectObject( &theApp.m_gdiFont );
	pDC->SetTextColor( CoolInterface.m_crText );
	LoadString( strText, IDS_NEIGHBOURS_NOT_CONNECTED );
	pDC->DrawText( strText, &rcText, DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_NOPREFIX );

	rcText.OffsetRect( 0, rcText.Height() );

	LoadString( strText, IDS_NEIGHBOURS_CONNECT );
	pDC->DrawText( strText, &rcText, DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_NOPREFIX );

	pDC->SelectObject( pOldFont );
}

void CNeighboursWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CPanelWnd::OnActivate(nState, pWndOther, bMinimized);
	Update();
}

BOOL CNeighboursWnd::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB )
	{
		GetManager()->Open( RUNTIME_CLASS(CSystemWnd) );
		return TRUE;
	}
	// Ctrl+H
	else if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == 'H' && ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) )
	{
		CSingleLock pLock( &Network.m_pSection, TRUE );
		if ( CNeighbour* pNeighbour = GetItem( m_wndList.GetNextItem( -1, LVNI_SELECTED ) ) )
		{
			pLock.Unlock();

			CHexDlg dlg;
			if ( dlg.DoModal() == IDOK )
			{
				pLock.Lock();

				BOOL bResult = pNeighbour->ProcessPackets( dlg.GetData() );

				pLock.Unlock();

				if ( bResult )
				{
					AfxMessageBox( _T("Packet was successfully processed.") );
				}
				else
				{
					AfxMessageBox( _T("Packet was rejected.") );
				}
			}
		}
		return TRUE;
	}

	return CPanelWnd::PreTranslateMessage(pMsg);
}
