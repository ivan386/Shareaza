//
// CtrlHomeView.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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
#include "CtrlHomeView.h"
#include "RichElement.h"
#include "Skin.h"

#include "Network.h"
#include "Datagrams.h"
#include "Neighbour.h"
#include "Neighbours.h"
#include "VersionChecker.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CHomeViewCtrl, CRichViewCtrl)

BEGIN_MESSAGE_MAP(CHomeViewCtrl, CRichViewCtrl)
	ON_WM_CREATE()
END_MESSAGE_MAP()

#define GROUP_DISCONNECTED		1
#define GROUP_CONNECTED			2
#define GROUP_UPGRADE			3
#define GROUP_FIREWALLED		4
#define GROUP_REMOTE			5
#define GROUP_FIREWALLED_TCP	6
#define GROUP_FIREWALLED_UDP	7


/////////////////////////////////////////////////////////////////////////////
// CHomeViewCtrl construction

CHomeViewCtrl::CHomeViewCtrl()
{
	m_peHeader = m_peSearch = m_peUpgrade = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CHomeViewCtrl system message handlers

BOOL CHomeViewCtrl::Create(const RECT& rect, CWnd* pParentWnd)
{
	return CRichViewCtrl::Create( WS_CHILD|WS_CLIPCHILDREN, rect, pParentWnd, IDC_HOME_VIEW );
}

int CHomeViewCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CRichViewCtrl::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_wndSearch.Create( this, IDC_HOME_SEARCH );

	OnSkinChange();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CHomeViewCtrl operations

void CHomeViewCtrl::OnSkinChange()
{
	m_pDocument.Clear();
	m_peHeader = m_peSearch = m_peUpgrade = m_peRemote1 = m_peRemote2 = NULL;

	CXMLElement* pXML = Skin.GetDocument( _T("CHomeViewCtrl") );
	CElementMap pMap;

	if ( pXML == NULL || ! m_pDocument.LoadXML( pXML, &pMap ) )
	{
		SetDocument( &m_pDocument );
		m_wndSearch.OnSkinChange( m_pDocument.m_crBackground );
		return;
	}

	pMap.Lookup( _T("Header"), m_peHeader );
	pMap.Lookup( _T("SearchBox"), m_peSearch );
	pMap.Lookup( _T("Upgrade"), m_peUpgrade );
	pMap.Lookup( _T("RemoteAccessURL1"), m_peRemote1 );
	pMap.Lookup( _T("RemoteAccessURL2"), m_peRemote2 );

	m_wndSearch.OnSkinChange( m_pDocument.m_crBackground );

	SetDocument( &m_pDocument );
	Update();
}

void CHomeViewCtrl::Activate()
{
	if ( m_wndSearch.IsWindowVisible() )
		m_wndSearch.Activate();
}

void CHomeViewCtrl::Update()
{
	BOOL bConnected = Network.IsConnected();
	m_pDocument.ShowGroup( GROUP_DISCONNECTED, ! bConnected );
	m_pDocument.ShowGroup( GROUP_CONNECTED, bConnected );

	BOOL bOnG2 = bConnected && Settings.Gnutella2.EnableToday && ( Neighbours.GetCount( PROTOCOL_G2, nrsConnected, -1 ) >= Settings.Gnutella2.NumHubs );
	// BOOL bTCPFirewalled = Network.IsFirewalled(CHECK_TCP);
	BOOL bUDPFirewalled = Network.IsFirewalled(CHECK_UDP);

	m_pDocument.ShowGroup( GROUP_FIREWALLED, bOnG2 && bUDPFirewalled && ! Network.m_bUPnPPortsForwarded );
	m_pDocument.ShowGroup( GROUP_FIREWALLED_TCP, FALSE );
	m_pDocument.ShowGroup( GROUP_FIREWALLED_UDP, FALSE );

	/*m_pDocument.ShowGroup( GROUP_FIREWALLED, bOnG2 && bTCPFirewalled && bUDPFirewalled );
	m_pDocument.ShowGroup( GROUP_FIREWALLED_TCP, bOnG2 && bTCPFirewalled && !bUDPFirewalled );
	m_pDocument.ShowGroup( GROUP_FIREWALLED_UDP, bOnG2 && !bTCPFirewalled && bUDPFirewalled );*/	// Temp disabled

	if ( VersionChecker.IsUpgradeAvailable() )
	{
		if ( m_peUpgrade ) m_peUpgrade->SetText( Settings.VersionCheck.UpgradePrompt );
		m_pDocument.ShowGroup( GROUP_UPGRADE, TRUE );
	}
	else
	{
		m_pDocument.ShowGroup( GROUP_UPGRADE, FALSE );
	}

	if ( Settings.Remote.Enable && ! Settings.Remote.Username.IsEmpty() &&
		 ! Settings.Remote.Password.IsEmpty() && Network.IsListening() )
	{
		CString strURL;

		if ( m_peRemote1 )
		{
			strURL.Format( _T("http://%s:%i/remote/"),
				(LPCTSTR)CString( inet_ntoa( Network.m_pHost.sin_addr ) ),
				(int)ntohs( Network.m_pHost.sin_port ) );
			m_peRemote1->SetText( Settings.General.LanguageRTL ? _T("\x202A") + strURL : strURL );
			m_peRemote1->m_sLink = strURL;
		}
		if ( m_peRemote2 )
		{
			strURL.Format( _T("http://localhost:%i/remote/"),
				(int)ntohs( Network.m_pHost.sin_port ) );
			m_peRemote2->SetText( Settings.General.LanguageRTL ? _T("\x202A") + strURL : strURL );
			m_peRemote2->m_sLink = strURL;
		}

		m_pDocument.ShowGroup( GROUP_REMOTE, TRUE );
	}
	else
	{
		m_pDocument.ShowGroup( GROUP_REMOTE, FALSE );
	}

	InvalidateIfModified();
}

/////////////////////////////////////////////////////////////////////////////
// CHomeViewCtrl layout event

void CHomeViewCtrl::OnLayoutComplete()
{
	if ( m_peSearch != NULL && ( m_peSearch->m_nFlags & retfHidden ) == 0 )
	{
		CRect rcClient, rcAnchor( 0, 0, 0, 0 );

		GetClientRect( &rcClient );
		GetElementRect( m_peSearch, &rcAnchor );
		rcAnchor.OffsetRect( 0, -GetScrollPos( SB_VERT ) );

		rcAnchor.left = rcClient.left;
		rcAnchor.right = rcClient.right;

		rcAnchor.DeflateRect( m_pDocument.m_szMargin.cx + 39, 0 );

		BOOL bShowed = m_wndSearch.IsWindowVisible() == FALSE;

		m_wndSearch.SetWindowPos( NULL, rcAnchor.left, rcAnchor.top, rcAnchor.Width(), rcAnchor.Height(), SWP_SHOWWINDOW|SWP_NOACTIVATE );

		if ( bShowed && GetFocus() == this ) m_wndSearch.SetFocus();
	}
	else
	{
		m_wndSearch.ShowWindow( SW_HIDE );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHomeViewCtrl paint event

void CHomeViewCtrl::OnPaintBegin(CDC* pDC)
{
	if ( m_peHeader == NULL || ( m_peHeader->m_nFlags & retfHidden ) != 0 )
		// No headers
		return;

	HBITMAP hbmHeader1 = Skin.GetWatermark( _T( "CHomeViewCtrl.Header1" ), TRUE );
	if ( hbmHeader1 != NULL )
	{
		CBitmap* pbmHeader1 = CBitmap::FromHandle( hbmHeader1 );

		CRect rcAnchor( 0, 0, 0, 0 );
		GetElementRect( m_peHeader, &rcAnchor );

		CRect rcClient;
		GetClientRect( &rcClient );
		rcAnchor.left = rcClient.left;
		rcAnchor.right = rcClient.right;

		CRect rcMark( &rcAnchor );

		CDC dcHeader;
		if ( dcHeader.CreateCompatibleDC( pDC ) )
		{
			BITMAP pHeader1 = {};
			if ( pbmHeader1->GetBitmap( &pHeader1 ) )
			{
				CBitmap* pbmOld = (CBitmap*)dcHeader.SelectObject( pbmHeader1 );

				pDC->BitBlt( rcAnchor.left, rcAnchor.top, min( (LONG)rcAnchor.Width(), pHeader1.bmWidth ),
					min( (LONG)rcAnchor.Height(), pHeader1.bmHeight ), &dcHeader, 0, 0, SRCCOPY );

				rcMark.left += pHeader1.bmWidth;

				dcHeader.SelectObject( pbmOld );
			}

			if ( pDC->RectVisible( &rcMark ) )
			{
				HBITMAP hbmHeader2 = Skin.GetWatermark( _T( "CHomeViewCtrl.Header2" ), TRUE );
				if ( hbmHeader2 != NULL )
				{
					CBitmap* pbmHeader2 = CBitmap::FromHandle( hbmHeader2 );

					BITMAP pHeader2 = {};
					if ( pbmHeader2->GetBitmap( &pHeader2 ) )
					{
						CBitmap* pbmOld = (CBitmap*)dcHeader.SelectObject( pbmHeader2 );

						while ( rcMark.left < rcMark.right )
						{
							pDC->BitBlt( rcMark.left, rcMark.top, min( (LONG)rcMark.Width(), pHeader2.bmWidth ),
								min( (LONG)rcMark.Height(), pHeader2.bmHeight ), &dcHeader, 0, 0, SRCCOPY );
							rcMark.left += pHeader2.bmWidth;
						}

						dcHeader.SelectObject( pbmOld );
					}
				}
			}

			dcHeader.DeleteDC();

			pDC->ExcludeClipRect( &rcAnchor );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHomeViewCtrl other event handlers

void CHomeViewCtrl::OnVScrolled()
{
	OnLayoutComplete();
}

