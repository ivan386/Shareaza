//
// PageSettingsGnutella.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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
#include "WndSettingsSheet.h"
#include "PageSettingsNetworks.h"
#include "PageSettingsGnutella.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CGnutellaSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CGnutellaSettingsPage, CSettingsPage)
	//{{AFX_MSG_MAP(CGnutellaSettingsPage)
	ON_BN_CLICKED(IDC_G2_TODAY, OnG2Today)
	ON_BN_CLICKED(IDC_G1_TODAY, OnG1Today)
	ON_BN_CLICKED(IDC_G2_ALWAYS, OnG2Always)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGnutellaSettingsPage property page

CGnutellaSettingsPage::CGnutellaSettingsPage() : CSettingsPage( CGnutellaSettingsPage::IDD )
{
	//{{AFX_DATA_INIT(CGnutellaSettingsPage)
	m_bG2Today = FALSE;
	m_bG2Always = FALSE;
	m_bG1Today = FALSE;
	m_bG1Always = FALSE;
	m_nG1Hubs = 0;
	m_nG1Leafs = 0;
	m_nG1Peers = 0;
	m_nG2Hubs = 0;
	m_nG2Leafs = 0;
	m_nG2Peers = 0;
	m_bDeflateHub2Hub = FALSE;
	m_bDeflateLeaf2Hub = FALSE;
	m_bDeflateHub2Leaf = FALSE;
	//}}AFX_DATA_INIT
}

CGnutellaSettingsPage::~CGnutellaSettingsPage()
{
}

void CGnutellaSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGnutellaSettingsPage)
	DDX_Control(pDX, IDC_G2_PEERS_SPIN, m_wndG2Peers);
	DDX_Control(pDX, IDC_G2_LEAFS_SPIN, m_wndG2Leafs);
	DDX_Control(pDX, IDC_G2_HUBS_SPIN, m_wndG2Hubs);
	DDX_Control(pDX, IDC_G1_PEERS_SPIN, m_wndG1Peers);
	DDX_Control(pDX, IDC_G1_LEAFS_SPIN, m_wndG1Leafs);
	DDX_Control(pDX, IDC_G1_HUBS_SPIN, m_wndG1Hubs);
	DDX_Check(pDX, IDC_G2_TODAY, m_bG2Today);
	DDX_Check(pDX, IDC_G2_ALWAYS, m_bG2Always);
	DDX_Check(pDX, IDC_G1_TODAY, m_bG1Today);
	DDX_Check(pDX, IDC_G1_ALWAYS, m_bG1Always);
	DDX_Text(pDX, IDC_G1_HUBS, m_nG1Hubs);
	DDX_Text(pDX, IDC_G1_LEAFS, m_nG1Leafs);
	DDX_Text(pDX, IDC_G1_PEERS, m_nG1Peers);
	DDX_Text(pDX, IDC_G2_HUBS, m_nG2Hubs);
	DDX_Text(pDX, IDC_G2_LEAFS, m_nG2Leafs);
	DDX_Text(pDX, IDC_G2_PEERS, m_nG2Peers);
	DDX_Check(pDX, IDC_DEFLATE_HUB2HUB, m_bDeflateHub2Hub);
	DDX_Check(pDX, IDC_DEFLATE_LEAF2HUB, m_bDeflateLeaf2Hub);
	DDX_Check(pDX, IDC_DEFLATE_HUB2LEAF, m_bDeflateHub2Leaf);
	DDX_Control(pDX, IDC_G1_CLIENTMODE, m_wndG1ClientMode);
	DDX_Control(pDX, IDC_G2_CLIENTMODE, m_wndG2ClientMode);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CGnutellaSettingsPage message handlers

BOOL CGnutellaSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	//Load initial values from the settings variables
	m_bG2Today			= Settings.Gnutella2.EnableToday;
	m_bG2Always			= Settings.Gnutella2.EnableAlways;

	m_bG1Today			= Settings.Gnutella1.EnableToday;
	m_bG1Always			= Settings.Gnutella1.EnableAlways;

	m_bDeflateHub2Hub	= Settings.Gnutella.DeflateHub2Hub;
	m_bDeflateLeaf2Hub	= Settings.Gnutella.DeflateLeaf2Hub;
	m_bDeflateHub2Leaf	= Settings.Gnutella.DeflateHub2Leaf;

	m_nG1Hubs			= Settings.Gnutella1.NumHubs;
	m_nG1Leafs			= Settings.Gnutella1.NumLeafs;
	m_nG1Peers			= Settings.Gnutella1.NumPeers;

	m_nG2Hubs			= Settings.Gnutella2.NumHubs;
	m_nG2Leafs			= Settings.Gnutella2.NumLeafs;
	m_nG2Peers			= Settings.Gnutella2.NumPeers;

	Settings.SetRange( &Settings.Gnutella1.NumHubs, m_wndG1Hubs );
	Settings.SetRange( &Settings.Gnutella1.NumLeafs, m_wndG1Leafs );
	Settings.SetRange( &Settings.Gnutella1.NumPeers, m_wndG1Peers );
	Settings.SetRange( &Settings.Gnutella2.NumHubs, m_wndG2Hubs );
	Settings.SetRange( &Settings.Gnutella2.NumLeafs, m_wndG2Leafs );
	Settings.SetRange( &Settings.Gnutella2.NumPeers, m_wndG2Peers );

	m_wndG1ClientMode.SetItemData( 0, MODE_AUTO );
	m_wndG1ClientMode.SetItemData( 1, MODE_LEAF );
	m_wndG1ClientMode.SetItemData( 2, MODE_ULTRAPEER );

	m_wndG1ClientMode.SetCurSel( Settings.Gnutella1.ClientMode );

	m_wndG2ClientMode.SetItemData( 0, MODE_AUTO );
	m_wndG2ClientMode.SetItemData( 1, MODE_LEAF );
	m_wndG2ClientMode.SetItemData( 2, MODE_HUB );

	//********************					Temp- until UP mode fixed
	Settings.Gnutella1.ClientMode = MODE_LEAF;
	m_wndG1ClientMode.EnableWindow( FALSE );
	m_wndG1ClientMode.SetCurSel( MODE_LEAF );
	//********************

	m_wndG2ClientMode.SetCurSel( Settings.Gnutella2.ClientMode );

#ifdef LAN_MODE
	GetDlgItem( IDC_G2_TODAY )->EnableWindow( FALSE );
	GetDlgItem( IDC_G2_ALWAYS )->EnableWindow( FALSE );
	GetDlgItem( IDC_G1_TODAY )->EnableWindow( FALSE );
	GetDlgItem( IDC_G1_ALWAYS )->EnableWindow( FALSE );
	m_wndG1Peers.EnableWindow( FALSE );
	m_wndG1Leafs.EnableWindow( FALSE );
	m_wndG1Hubs.EnableWindow( FALSE );
	m_wndG1ClientMode.EnableWindow( FALSE );
	GetDlgItem( IDC_G1_HUBS )->EnableWindow( FALSE );
	GetDlgItem( IDC_G1_LEAFS )->EnableWindow( FALSE );
	GetDlgItem( IDC_G1_PEERS )->EnableWindow( FALSE );
#endif // LAN_MODE

	UpdateData( FALSE );

	return TRUE;
}

BOOL CGnutellaSettingsPage::OnSetActive()
{
	CNetworksSettingsPage* ppNetworks =
		(CNetworksSettingsPage*)GetPage( RUNTIME_CLASS(CNetworksSettingsPage) );

	if ( ppNetworks->GetSafeHwnd() != NULL )
	{
		ppNetworks->UpdateData( TRUE );
		m_bG2Today = ppNetworks->m_bG2Enable;
		m_bG1Today = ppNetworks->m_bG1Enable;
		UpdateData( FALSE );
	}

	return CSettingsPage::OnSetActive();
}

void CGnutellaSettingsPage::OnG2Always()
{
	UpdateData();

	if ( ! m_bG2Always )
	{
		CString strMessage;
		LoadString( strMessage, IDS_NETWORK_DISABLE_G2 );

		if ( AfxMessageBox( strMessage, MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2 ) != IDYES )
		{
			m_bG2Always = TRUE;
			UpdateData( FALSE );
		}
	}
}

void CGnutellaSettingsPage::OnG2Today()
{
	UpdateData();

	if ( ! m_bG2Today )
	{
		CString strMessage;
		LoadString( strMessage, IDS_NETWORK_DISABLE_G2 );

		if ( AfxMessageBox( strMessage, MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2 ) != IDYES )
		{
			m_bG2Today = TRUE;
			UpdateData( FALSE );
		}
	}

	CNetworksSettingsPage* ppNetworks =
		(CNetworksSettingsPage*)GetPage( RUNTIME_CLASS(CNetworksSettingsPage) );

	if ( ppNetworks->GetSafeHwnd() != NULL )
	{
		ppNetworks->UpdateData( TRUE );
		ppNetworks->m_bG2Enable = m_bG2Today;
		ppNetworks->UpdateData( FALSE );
	}
}

void CGnutellaSettingsPage::OnG1Today()
{
	UpdateData( TRUE );

	if ( m_bG1Today && ( Settings.GetOutgoingBandwidth() < 2 ) )
	{
		CString strMessage;
		LoadString( strMessage, IDS_NETWORK_BANDWIDTH_LOW );
		AfxMessageBox( strMessage, MB_OK );
		m_bG1Today = FALSE;
		UpdateData( FALSE );
	}

	CNetworksSettingsPage* ppNetworks =
		(CNetworksSettingsPage*)GetPage( RUNTIME_CLASS(CNetworksSettingsPage) );

	if ( ppNetworks->GetSafeHwnd() != NULL )
	{
		UpdateData( TRUE );
		ppNetworks->UpdateData( TRUE );
		ppNetworks->m_bG1Enable = m_bG1Today;
		ppNetworks->UpdateData( FALSE );
	}
}

void CGnutellaSettingsPage::OnOK()
{
	UpdateData();

	//Check if G2 hub mode is forced now, and wasn't forced before.
	if ( ( m_wndG2ClientMode.GetCurSel() == MODE_HUB ) && ( Settings.Gnutella2.ClientMode != MODE_HUB ) )
	{
		CString strMessage;
		LoadString( strMessage, IDS_NETWORK_FORCE_HUB );
		//Warn the user, give them a chance to reset it.
		if ( AfxMessageBox( strMessage, MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2 ) != IDYES )
		{
			m_wndG2ClientMode.SetCurSel( MODE_AUTO );
			Settings.Gnutella2.ClientMode = MODE_AUTO;
			UpdateData( FALSE );
		}
	}

	// Limit networks if low bandwidth
	if ( ( Settings.GetOutgoingBandwidth() < 2 ) )
	{
		m_bG1Today = m_bG1Always = FALSE;
	}

	//Load values into the settings variables
	Settings.Gnutella2.EnableToday		= m_bG2Today != FALSE;
	Settings.Gnutella2.EnableAlways		= m_bG2Always != FALSE;
	Settings.Gnutella1.EnableToday		= m_bG1Today != FALSE;
	Settings.Gnutella1.EnableAlways		= m_bG1Always != FALSE;
	Settings.Gnutella.DeflateHub2Hub	= m_bDeflateHub2Hub != FALSE;
	Settings.Gnutella.DeflateLeaf2Hub	= m_bDeflateLeaf2Hub != FALSE;
	Settings.Gnutella.DeflateHub2Leaf	= m_bDeflateHub2Leaf != FALSE;
	Settings.Gnutella1.NumHubs			= m_nG1Hubs;
	Settings.Gnutella1.NumLeafs			= m_nG1Leafs;
	Settings.Gnutella1.NumPeers			= m_nG1Peers;
	Settings.Gnutella2.NumHubs			= m_nG2Hubs;
	Settings.Gnutella2.NumLeafs			= m_nG2Leafs;
	Settings.Gnutella2.NumPeers			= m_nG2Peers;

	Settings.Gnutella1.ClientMode = m_wndG1ClientMode.GetCurSel(); // Mode is equal to select position
	if ( Settings.Gnutella1.ClientMode > MODE_ULTRAPEER ) Settings.Gnutella1.ClientMode = MODE_AUTO;

	Settings.Gnutella2.ClientMode = m_wndG2ClientMode.GetCurSel(); // Mode is equal to select position
	if ( Settings.Gnutella2.ClientMode > MODE_HUB ) Settings.Gnutella2.ClientMode = MODE_AUTO;

	Settings.Normalize( &Settings.Gnutella1.NumHubs );
	Settings.Normalize( &Settings.Gnutella1.NumLeafs );
	Settings.Normalize( &Settings.Gnutella1.NumPeers );
	Settings.Normalize( &Settings.Gnutella2.NumHubs );
	Settings.Normalize( &Settings.Gnutella2.NumLeafs );
	Settings.Normalize( &Settings.Gnutella2.NumPeers );

	// Update display in case settings were changed
	m_nG1Hubs			= Settings.Gnutella1.NumHubs;
	m_nG1Leafs			= Settings.Gnutella1.NumLeafs;
	m_nG1Peers			= Settings.Gnutella1.NumPeers;
	m_nG2Hubs			= Settings.Gnutella2.NumHubs;
	m_nG2Leafs			= Settings.Gnutella2.NumLeafs;
	m_nG2Peers			= Settings.Gnutella2.NumPeers;
	UpdateData( FALSE );

	CSettingsPage::OnOK();
}
