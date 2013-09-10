//
// WndPacket.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2013.
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
#include "CoolMenu.h"
#include "LiveList.h"
#include "Neighbour.h"
#include "Neighbours.h"
#include "Network.h"
#include "Skin.h"
#include "WndPacket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_PAUSE		1
#define ID_TCP			2
#define ID_UDP			3
#define ID_NONE			999
#define ID_BASE_IN		1000
#define ID_BASE_OUT		2000
#define ID_BASE_G1		3000
#define ID_BASE_G2		3100
#define ID_BASE_ED2K	3200
#define ID_BASE_BT		3300
#define ID_BASE_DC		3400

IMPLEMENT_SERIAL(CPacketWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CPacketWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PACKETS, OnCustomDrawList)
	ON_WM_CONTEXTMENU()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_UPDATE_COMMAND_UI(ID_SYSTEM_CLEAR, OnUpdateSystemClear)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_UPDATE_COMMAND_UI_RANGE(ID_PAUSE, ID_BASE_DC, OnUpdateBlocker)
END_MESSAGE_MAP()

G2_PACKET CPacketWnd::m_nG2[nTypeG2Size] = {
	G2_PACKET_CACHED_HUB,
	G2_PACKET_CRAWL_ANS,
	G2_PACKET_CRAWL_REQ,
	G2_PACKET_DISCOVERY,
	G2_PACKET_HAW,
	G2_PACKET_HIT,
	G2_PACKET_HIT_WRAP,
	G2_PACKET_KHL,
	G2_PACKET_KHL_ANS,
	G2_PACKET_KHL_REQ,
	G2_PACKET_LNI,
	G2_PACKET_PING,
	G2_PACKET_PONG,
	G2_PACKET_PROFILE_CHALLENGE,
	G2_PACKET_PROFILE_DELIVERY,
	G2_PACKET_PUSH,
	G2_PACKET_QHT,
	G2_PACKET_QUERY,
	G2_PACKET_QUERY_ACK,
	G2_PACKET_QUERY_KEY_ANS,
	G2_PACKET_QUERY_KEY_REQ,
	G2_PACKET_QUERY_WRAP
};

/////////////////////////////////////////////////////////////////////////////
// CPacketWnd construction

CPacketWnd::CPacketWnd(CChildWnd* pOwner)
	: m_pOwner			( pOwner )
	, m_nInputFilter	( 0 )
	, m_nOutputFilter	( 0 )
	, m_bPaused			( TRUE )
	, m_bTCP			( TRUE )
	, m_bUDP			( TRUE )
	, m_bTypeED			( TRUE )
	, m_bTypeBT			( TRUE )
	, m_bTypeDC			( TRUE )
	, m_pCoolMenu		( NULL )
{
	for ( int nType = 0 ; nType < nTypeG1Size ; nType++ ) m_bTypeG1[ nType ] = TRUE;
	for ( int nType = 0 ; nType < nTypeG2Size ; nType++ ) m_bTypeG2[ nType ] = TRUE;

	Create( IDR_PACKETFRAME );
}

CPacketWnd::~CPacketWnd()
{
	CSingleLock pLock( &m_pSection, TRUE );

	m_bPaused = TRUE;
	theApp.m_pPacketWnd = NULL;

	for ( POSITION pos = m_pQueue.GetHeadPosition() ; pos ; )
		delete m_pQueue.GetNext( pos );
	m_pQueue.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// CPacketWnd create

int CPacketWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_wndList.Create( WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE |
		LVS_AUTOARRANGE | LVS_REPORT | LVS_SHOWSELALWAYS,
		rectDefault, this, IDC_PACKETS );
	m_pSizer.Attach( &m_wndList );

	m_wndList.SetExtendedStyle(
		LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP|LVS_EX_SUBITEMIMAGES );

	m_wndList.InsertColumn( 0, _T("Time"), LVCFMT_CENTER, 80, -1 );
	m_wndList.InsertColumn( 1, _T("Address"), LVCFMT_LEFT, 110, -1 );
	m_wndList.InsertColumn( 2, _T("Protocol"), LVCFMT_LEFT, 80, 0 );
    m_wndList.InsertColumn( 3, _T("Type"), LVCFMT_CENTER, 100, 1 );
	m_wndList.InsertColumn( 4, _T("T/H"), LVCFMT_CENTER, 40, 2 );
	m_wndList.InsertColumn( 5, _T("Hex"), LVCFMT_LEFT, 55, 3 );
	m_wndList.InsertColumn( 6, _T("ASCII"), LVCFMT_LEFT, 480, 4 );
	m_wndList.InsertColumn( 7, _T("G1-ID"), LVCFMT_LEFT, 55, 5 );

	LoadState();

	m_bPaused = FALSE;
	SetTimer( 2, 500, NULL );

	theApp.m_pPacketWnd = this;

	return 0;
}

void CPacketWnd::OnDestroy() 
{
	m_bPaused = TRUE;
	theApp.m_pPacketWnd = NULL;

	KillTimer( 2 );

	Settings.SaveList( _T("CPacketWnd"), &m_wndList );
	SaveState( _T("CPacketWnd") );

	CPanelWnd::OnDestroy();
}

void CPacketWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();

	// Columns
	Settings.LoadList( _T("CPacketWnd"), &m_wndList );

	// Fonts
	if ( m_pFont.m_hObject ) m_pFont.DeleteObject();
	m_pFont.CreateFont( -(int)(Settings.Fonts.FontSize - 1), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, theApp.m_nFontQuality,
		DEFAULT_PITCH|FF_DONTCARE, Settings.Fonts.PacketDumpFont );
	m_wndList.SetFont( &m_pFont );
	m_wndList.GetHeaderCtrl()->SetFont( &theApp.m_gdiFont );

	// Icons
	CoolInterface.LoadProtocolIconsTo( m_gdiImageList );
	VERIFY( m_gdiImageList.SetImageCount( m_gdiImageList.GetImageCount() + 2 ) );
	VERIFY( m_gdiImageList.Replace( PROTOCOL_LAST + 0, CoolInterface.ExtractIcon( IDI_OUTGOING ) ) != -1 );
	VERIFY( m_gdiImageList.Replace( PROTOCOL_LAST + 1, CoolInterface.ExtractIcon( IDI_INCOMING ) ) != -1 );
	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );
}

/////////////////////////////////////////////////////////////////////////////
// CPacketWnd operations

void CPacketWnd::SmartDump(const CPacket* pPacket, const SOCKADDR_IN* pAddress, BOOL bUDP, BOOL bOutgoing, DWORD_PTR nNeighbourUnique)
{
	if ( m_bPaused || m_hWnd == NULL )
		return;

	if ( bUDP )
	{
		if ( ! m_bUDP )
			return;
	}
	else
	{
		if ( ! m_bTCP )
			return;
	}

	if ( nNeighbourUnique )
	{
		if ( bOutgoing )
		{
			if ( m_nOutputFilter && m_nOutputFilter != nNeighbourUnique )
				return;
		}
		else
		{
			if ( m_nInputFilter && m_nInputFilter != nNeighbourUnique )
				return;
		}
	}
	else
	{
		if ( bOutgoing )
		{
			if ( m_nOutputFilter )
				return;
		}
		else
		{
			if ( m_nInputFilter )
				return;
		}
	}

	switch ( pPacket->m_nProtocol )
	{
	case PROTOCOL_G1:
		if ( ! m_bTypeG1[ ((CG1Packet*)pPacket)->m_nTypeIndex ] )
			return;
		break;

	case PROTOCOL_G2:
		for ( int nType = 0 ; nType < nTypeG2Size ; nType++ )
		{
			if ( ((CG2Packet*)pPacket)->IsType( m_nG2[ nType ] ) )
			{
				if ( ! m_bTypeG2[ nType ] )
					return;
				break;
			}
		}
		break;

	case PROTOCOL_ED2K:
		// TODO: Filter ED2K packets
		if ( ! m_bTypeED )
			return;
		break;

	case PROTOCOL_BT:
		// TODO: Filter BitTorrent packets
		if ( ! m_bTypeBT )
			return;
		break;

	case PROTOCOL_DC:
		// TODO: Filter DC++ packets
		if ( ! m_bTypeDC )
			return;
		break;

	default:
		;
	}
		
	CAutoPtr< CLiveItem > pItem( new CLiveItem( 8, bOutgoing ) );
	if ( ! pItem )
		// Out of memory
		return;

	CTime pNow( CTime::GetCurrentTime() );
	CString strNow;
	strNow.Format( _T("%0.2i:%0.2i:%0.2i"),
		pNow.GetHour(), pNow.GetMinute(), pNow.GetSecond() );
	CString sAddress( HostToString( pAddress ) );
	CString sProtocol( protocolAbbr[ pPacket->m_nProtocol ] );

	pItem->Set( 0, strNow );
	pItem->Set( 1, bUDP ? _T("(") + sAddress + _T(")") : sAddress );
	pItem->Set( 2, sProtocol + ( bUDP ? _T(" UDP") : _T(" TCP") ) );
	pItem->Set( 3, pPacket->GetType() );
	pItem->Set( 5, pPacket->ToHex() );
	pItem->Set( 6, pPacket->ToASCII() );

	pItem->SetImage( 0, PROTOCOL_LAST + ( bOutgoing ? 0 : 1 ) );
	pItem->SetImage( 2, pPacket->m_nProtocol );

	if ( pPacket->m_nProtocol == PROTOCOL_G1 )
	{
		pItem->Format( 4, _T("%u/%u"), ((CG1Packet*)pPacket)->m_nTTL, ((CG1Packet*)pPacket)->m_nHops );
		pItem->Set( 7, ((CG1Packet*)pPacket)->GetGUID() );
	}

	CQuickLock pLock( m_pSection );

	if ( ! theApp.m_pPacketWnd )
		return;

	m_pQueue.AddTail( pItem.Detach() );
}

void CPacketWnd::OnTimer(UINT_PTR nIDEvent) 
{
	if ( nIDEvent != 2 ) return;

	BOOL bScroll = m_wndList.GetTopIndex() + m_wndList.GetCountPerPage() >= m_wndList.GetItemCount();

	CSingleLock pLock( &m_pSection );
	BOOL bAny = FALSE;

	for (;;)
	{
		pLock.Lock();

		if ( m_pQueue.GetCount() == 0 ) break;
		CLiveItem* pItem = m_pQueue.RemoveHead();

		pLock.Unlock();

		if ( ! bAny )
		{
			bAny = TRUE;
		}

		if ( (DWORD)m_wndList.GetItemCount() >= Settings.Search.MonitorQueue && Settings.Search.MonitorQueue > 0 )
		{
			m_wndList.DeleteItem( 0 );
		}

		/*int nItem =*/ pItem->Add( &m_wndList, -1, 8 );

		delete pItem;
	}

	if ( bAny )
	{
		if ( bScroll ) m_wndList.EnsureVisible( m_wndList.GetItemCount() - 1, FALSE );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPacketWnd message handlers

void CPacketWnd::OnSize(UINT nType, int cx, int cy) 
{
	CPanelWnd::OnSize( nType, cx, cy );
	m_pSizer.Resize( cx );
	m_wndList.SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOZORDER );
}

void CPacketWnd::OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)pNMHDR;

	if ( pDraw->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( pDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
	{
		if ( pDraw->nmcd.lItemlParam )
			pDraw->clrText = CoolInterface.m_crNetworkUp ;
		else
			pDraw->clrText = CoolInterface.m_crNetworkDown ;

		*pResult = CDRF_DODEFAULT;
	}
}

void CPacketWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point) 
{
	CSingleLock pLock( &Network.m_pSection, TRUE );

	CMenu pMenu, pProtocols, pHosts[2], pTypesG1, pTypesG2, pTypesED, pTypesBT, pTypesDC;

	for ( int nGroup = 0 ; nGroup < 2 ; nGroup++ )
	{
		UINT nID = nGroup ? ID_BASE_OUT : ID_BASE_IN;

		pHosts[nGroup].CreatePopupMenu();

		AddNeighbour( pHosts, nGroup, nID++, 1, _T("Disable") );
		AddNeighbour( pHosts, nGroup, nID++, 0, _T("Any Neighbour") );
		pHosts[nGroup].AppendMenu( MF_SEPARATOR, ID_SEPARATOR );

		BOOL bEmpty = TRUE;
		for ( POSITION pos = Neighbours.GetIterator() ; pos ; nID++ )
		{
			CNeighbour* pNeighbour = Neighbours.GetNext( pos );
			if ( pNeighbour->m_nState < nrsConnected ) continue;
			AddNeighbour( pHosts, nGroup, nID, (DWORD_PTR)pNeighbour, pNeighbour->m_sAddress );
			bEmpty = FALSE;
		}

		if ( bEmpty )
			pHosts[nGroup].AppendMenu( MF_STRING|MF_GRAYED, ID_NONE, _T("No Neighbours") );
	}

	pProtocols.CreatePopupMenu();
	pProtocols.AppendMenu( MF_STRING | ( m_bTCP ? MF_CHECKED : 0 ), ID_TCP, _T("TCP") );
	pProtocols.AppendMenu( MF_STRING | ( m_bUDP ? MF_CHECKED : 0 ), ID_UDP, _T("UDP") );

	pTypesG1.CreatePopupMenu();
	for ( int nType = 0 ; nType < nTypeG1Size ; nType++ )
	{
		pTypesG1.AppendMenu( MF_STRING | ( m_bTypeG1[ nType ] ? MF_CHECKED : 0 ), ID_BASE_G1 + nType, CG1Packet::m_pszPackets[ nType ] );
	}

	pTypesG2.CreatePopupMenu();
	for ( int nType = 0 ; nType < nTypeG2Size ; nType++ )
	{
		CStringA tmp;
		tmp.Append( (LPCSTR)&m_nG2[ nType ], G2_TYPE_LEN( m_nG2[ nType ] ) );
		pTypesG2.AppendMenu( MF_STRING | (m_bTypeG2[ nType ] ? MF_CHECKED : 0 ), ID_BASE_G2 + nType, CString( tmp ) );
	}

	pTypesED.CreatePopupMenu();
	pTypesED.AppendMenu( MF_STRING | ( m_bTypeED ? MF_CHECKED : 0 ), ID_BASE_ED2K, _T("All") );

	pTypesBT.CreatePopupMenu();
	pTypesBT.AppendMenu( MF_STRING | ( m_bTypeBT ? MF_CHECKED : 0 ), ID_BASE_BT, _T("All") );

	pTypesDC.CreatePopupMenu();
	pTypesDC.AppendMenu( MF_STRING | ( m_bTypeDC ? MF_CHECKED : 0 ), ID_BASE_DC, _T("All") );

	pMenu.CreatePopupMenu();
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT_PTR)pHosts[0].GetSafeHmenu(), _T("Incoming") );
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT_PTR)pHosts[1].GetSafeHmenu(), _T("Outgoing") );
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT_PTR)pProtocols.GetSafeHmenu(), _T("Protocols") );
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT_PTR)pTypesG1.GetSafeHmenu(), protocolAbbr[ PROTOCOL_G1 ] );
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT_PTR)pTypesG2.GetSafeHmenu(), protocolAbbr[ PROTOCOL_G2 ] );
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT_PTR)pTypesED.GetSafeHmenu(), protocolAbbr[ PROTOCOL_ED2K ] );
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT_PTR)pTypesBT.GetSafeHmenu(), protocolAbbr[ PROTOCOL_BT ] );
	pMenu.AppendMenu( MF_STRING|MF_POPUP, (UINT_PTR)pTypesDC.GetSafeHmenu(), protocolAbbr[ PROTOCOL_DC ] );
	pMenu.AppendMenu( MF_SEPARATOR, ID_SEPARATOR );
	pMenu.AppendMenu( MF_STRING | ( m_bPaused ? MF_CHECKED : 0 ), ID_PAUSE, _T("&Pause Display") );
	pMenu.AppendMenu( MF_STRING, ID_SYSTEM_CLEAR, _T("&Clear Buffer") );

	m_pCoolMenu = new CCoolMenu();
	m_pCoolMenu->AddMenu( &pMenu, TRUE );
	m_pCoolMenu->SetWatermark( Skin.GetWatermark( _T("CCoolMenu") ) );

	pLock.Unlock();

	UINT nCmd = pMenu.TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD,
		point.x, point.y, this );

	delete m_pCoolMenu;
	m_pCoolMenu = NULL;

	DWORD_PTR* pModify = NULL;

	if ( nCmd == ID_PAUSE )
	{
		m_bPaused = ! m_bPaused;
		return;
	}
	else if ( nCmd == ID_TCP )
	{
		m_bTCP = ! m_bTCP;
		return;
	}
	else if ( nCmd == ID_UDP )
	{
		m_bUDP = ! m_bUDP;
		return;
	}
	else if ( nCmd == ID_SYSTEM_CLEAR )
	{
		m_wndList.DeleteAllItems();
		return;
	}
	else if ( nCmd >= ID_BASE_G1 && nCmd < ID_BASE_G1 + nTypeG1Size )
	{
		nCmd -= ID_BASE_G1;

		if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
		{
			for ( int nType = 0 ; nType < nTypeG1Size ; nType++ )
			{
				m_bTypeG1[ nType ] = ( nCmd == (UINT)nType ) ? TRUE : FALSE;
			}
		}
		else
		{
			m_bTypeG1[ nCmd ] = ! m_bTypeG1[ nCmd ];
		}

		return;
	}
	else if ( nCmd >= ID_BASE_G2 && nCmd < ID_BASE_G2 + nTypeG2Size )
	{
		nCmd -= ID_BASE_G2;

		if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
		{
			for ( int nType = 0 ; nType < nTypeG2Size ; nType++ )
			{
				m_bTypeG2[ nType ] = ( nCmd == (UINT)nType ) ? TRUE : FALSE;
			}
		}
		else
		{
			m_bTypeG2[ nCmd ] = ! m_bTypeG2[ nCmd ];
		}

		return;
	}
	else if ( nCmd == ID_BASE_ED2K )
	{
		nCmd -= ID_BASE_ED2K;

		if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
		{
			m_bTypeED = TRUE;
		}
		else
		{
			m_bTypeED = ! m_bTypeED;
		}

		return;
	}
	else if ( nCmd == ID_BASE_BT )
	{
		nCmd -= ID_BASE_BT;

		if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
		{
			m_bTypeBT = TRUE;
		}
		else
		{
			m_bTypeBT = ! m_bTypeBT;
		}

		return;
	}
	else if ( nCmd == ID_BASE_DC )
	{
		nCmd -= ID_BASE_DC;

		if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
		{
			m_bTypeDC = TRUE;
		}
		else
		{
			m_bTypeDC = ! m_bTypeDC;
		}

		return;
	}
	else if ( nCmd >= ID_BASE_IN && nCmd < ID_BASE_OUT )
	{
		pModify = &m_nInputFilter;
		nCmd -= ID_BASE_IN;
	}
	else if ( nCmd >= ID_BASE_OUT && nCmd < ID_BASE_G1 )
	{
		pModify = &m_nOutputFilter;
		nCmd -= ID_BASE_OUT;
	}
	else
	{
		return;
	}

	if ( nCmd == 0 )
	{
		*pModify = 1;
	}
	else if ( nCmd == 1 )
	{
		*pModify = 0;
	}
	else
	{
		pLock.Lock();
		nCmd -= 2;

		for ( POSITION pos = Neighbours.GetIterator() ; pos ; nCmd-- )
		{
			CNeighbour* pNeighbour = Neighbours.GetNext( pos );
			if ( ! nCmd )
			{
				*pModify = (DWORD_PTR)pNeighbour;
				break;
			}
		}
	}

	Invalidate();
}

void CPacketWnd::AddNeighbour(CMenu* pMenus, int nGroup, UINT nID, DWORD_PTR nTarget, LPCTSTR pszText)
{
	UINT nChecked = ( ( nGroup == 1 && m_nOutputFilter == nTarget ) ||
		 ( nGroup == 0 && m_nInputFilter == nTarget ) )
		 ? MF_CHECKED : 0;

	pMenus[nGroup].AppendMenu( MF_STRING|nChecked, nID, pszText );
}

void CPacketWnd::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	if ( m_pCoolMenu ) m_pCoolMenu->OnMeasureItem( lpMeasureItemStruct );
}

void CPacketWnd::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	if ( m_pCoolMenu ) m_pCoolMenu->OnDrawItem( lpDrawItemStruct );
}

void CPacketWnd::OnUpdateBlocker(CCmdUI* pCmdUI)
{
	if ( m_pCoolMenu )
		pCmdUI->Enable( pCmdUI->m_nID != ID_NONE );
	else
		pCmdUI->ContinueRouting();
}

void CPacketWnd::OnUpdateSystemClear(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( TRUE );	
}
