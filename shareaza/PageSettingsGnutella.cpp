//
// PageSettingsGnutella.cpp
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
	ON_BN_CLICKED(IDC_ULTRAPEER_ENABLE, OnUltrapeerEnable)
	ON_BN_CLICKED(IDC_ULTRALEAF_ENABLE, OnUltraLeafEnable)
	ON_BN_CLICKED(IDC_ULTRAPEER_FORCE, OnUltraPeerForce)
	ON_BN_CLICKED(IDC_ULTRALEAF_FORCE, OnUltraLeafForce)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGnutellaSettingsPage property page

CGnutellaSettingsPage::CGnutellaSettingsPage() : CSettingsPage( CGnutellaSettingsPage::IDD )
{
	//{{AFX_DATA_INIT(CGnutellaSettingsPage)
	m_bLeafEnable = FALSE;
	m_bHubEnable = FALSE;
	m_bHubForce = FALSE;
	m_bLeafForce = FALSE;
	m_bG2Today = FALSE;
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
	DDX_Control(pDX, IDC_ULTRALEAF_FORCE, m_wndUltraLeafForce);
	DDX_Control(pDX, IDC_ULTRAPEER_FORCE, m_wndUltraPeerForce);
	DDX_Check(pDX, IDC_ULTRALEAF_ENABLE, m_bLeafEnable);
	DDX_Check(pDX, IDC_ULTRAPEER_ENABLE, m_bHubEnable);
	DDX_Check(pDX, IDC_ULTRAPEER_FORCE, m_bHubForce);
	DDX_Check(pDX, IDC_ULTRALEAF_FORCE, m_bLeafForce);
	DDX_Check(pDX, IDC_G2_TODAY, m_bG2Today);
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
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CGnutellaSettingsPage message handlers

BOOL CGnutellaSettingsPage::OnInitDialog() 
{
	CSettingsPage::OnInitDialog();
	
	m_bG2Today			= Settings.Gnutella2.EnableToday;
	m_bG1Today			= Settings.Gnutella1.EnableToday;
	m_bG1Always			= Settings.Gnutella1.EnableAlways;
	m_bHubEnable		= Settings.Gnutella.HubEnable;
	m_bHubForce			= Settings.Gnutella.HubForce;
	m_bLeafEnable		= Settings.Gnutella.LeafEnable;
	m_bLeafForce		= Settings.Gnutella.LeafForce;
	m_bDeflateHub2Hub	= Settings.Gnutella.DeflateHub2Hub;
	m_bDeflateLeaf2Hub	= Settings.Gnutella.DeflateLeaf2Hub;
	m_bDeflateHub2Leaf	= Settings.Gnutella.DeflateHub2Leaf;
	m_nG1Hubs			= Settings.Gnutella1.NumHubs;
	m_nG1Leafs			= Settings.Gnutella1.NumLeafs;
	m_nG1Peers			= Settings.Gnutella1.NumPeers;
	m_nG2Hubs			= Settings.Gnutella2.NumHubs;
	m_nG2Leafs			= Settings.Gnutella2.NumLeafs;
	m_nG2Peers			= Settings.Gnutella2.NumPeers;
	
	m_wndG1Peers.SetRange( 0, 128 );
	m_wndG1Leafs.SetRange( 0, 1024 );
	m_wndG1Hubs.SetRange( 0, 2 );
	
	m_wndG2Peers.SetRange( 0, 128 );
	m_wndG2Leafs.SetRange( 0, 1024 );
	m_wndG2Hubs.SetRange( 0, 3 );
	
	m_wndUltraPeerForce.EnableWindow( m_bHubEnable );
	m_wndUltraLeafForce.EnableWindow( m_bLeafEnable );
	
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

void CGnutellaSettingsPage::OnUltrapeerEnable() 
{
	UpdateData();
	m_wndUltraPeerForce.EnableWindow( m_bHubEnable );
}

void CGnutellaSettingsPage::OnUltraLeafEnable() 
{
	UpdateData();
	m_wndUltraLeafForce.EnableWindow( m_bLeafEnable );
}

void CGnutellaSettingsPage::OnUltraPeerForce() 
{
	UpdateData();

	if ( m_bHubForce )
	{
		m_wndUltraLeafForce.SetCheck( FALSE );

		CString strMessage;
		LoadString( strMessage, IDS_NETWORK_FORCE_HUB );

		if ( AfxMessageBox( strMessage, MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2 ) != IDYES )
		{
			m_bHubForce = FALSE;
			UpdateData( FALSE );
		}
	}
}

void CGnutellaSettingsPage::OnUltraLeafForce() 
{
	UpdateData();
	if ( m_bLeafForce ) m_wndUltraPeerForce.SetCheck( FALSE );
}

void CGnutellaSettingsPage::OnOK() 
{
	UpdateData();
	
	Settings.Gnutella2.EnableToday		= m_bG2Today;
	Settings.Gnutella1.EnableToday		= m_bG1Today || m_bG1Always;
	Settings.Gnutella1.EnableAlways		= m_bG1Always;
	Settings.Gnutella.HubEnable			= m_bHubEnable;
	Settings.Gnutella.HubForce			= m_bHubForce;
	Settings.Gnutella.LeafEnable		= m_bLeafEnable;
	Settings.Gnutella.LeafForce			= m_bLeafForce;
	Settings.Gnutella.DeflateHub2Hub	= m_bDeflateHub2Hub;
	Settings.Gnutella.DeflateLeaf2Hub	= m_bDeflateLeaf2Hub;
	Settings.Gnutella.DeflateHub2Leaf	= m_bDeflateHub2Leaf;
	Settings.Gnutella1.NumHubs			= min( m_nG1Hubs, 2 );
	Settings.Gnutella1.NumLeafs			= m_nG1Leafs;
	Settings.Gnutella1.NumPeers			= m_nG1Peers;
	Settings.Gnutella2.NumHubs			= min( m_nG2Hubs, 3 );
	Settings.Gnutella2.NumLeafs			= m_nG2Leafs;
	Settings.Gnutella2.NumPeers			= m_nG2Peers;
	
	CSettingsPage::OnOK();
}
