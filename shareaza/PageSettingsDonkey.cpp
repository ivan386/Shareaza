//
// PageSettingsDonkey.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
#include "PageSettingsDonkey.h"
#include "DlgDonkeyImport.h"
#include "DlgDonkeyServers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CDonkeySettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CDonkeySettingsPage, CSettingsPage)
	//{{AFX_MSG_MAP(CDonkeySettingsPage)
	ON_BN_CLICKED(IDC_DISCOVERY_GO, OnDiscoveryGo)
	ON_BN_CLICKED(IDC_SERVER_WALK, OnServerWalk)
	ON_BN_CLICKED(IDC_IMPORT_DOWNLOADS, OnImportDownloads)
	ON_BN_CLICKED(IDC_ENABLE_TODAY, OnEnableToday)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDonkeySettingsPage property page

CDonkeySettingsPage::CDonkeySettingsPage() : CSettingsPage( CDonkeySettingsPage::IDD )
{
	//{{AFX_DATA_INIT(CDonkeySettingsPage)
	m_nResults = 0;
	m_bServerWalk = FALSE;
	m_nLinks = 0;
	m_bEnableToday = FALSE;
	m_bEnableAlways = FALSE;
	//}}AFX_DATA_INIT
}

CDonkeySettingsPage::~CDonkeySettingsPage()
{
}

void CDonkeySettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDonkeySettingsPage)
	DDX_Control(pDX, IDC_LINKS_SPIN, m_wndLinksSpin);
	DDX_Control(pDX, IDC_RESULTS, m_wndResults);
	DDX_Control(pDX, IDC_RESULTS_SPIN, m_wndResultsSpin);
	DDX_Control(pDX, IDC_DISCOVERY_GO, m_wndDiscoveryGo);
	DDX_Text(pDX, IDC_RESULTS, m_nResults);
	DDX_Check(pDX, IDC_SERVER_WALK, m_bServerWalk);
	DDX_Text(pDX, IDC_LINKS, m_nLinks);
	DDX_Check(pDX, IDC_ENABLE_TODAY, m_bEnableToday);
	DDX_Check(pDX, IDC_ENABLE_ALWAYS, m_bEnableAlways);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CDonkeySettingsPage message handlers

BOOL CDonkeySettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	m_bEnableToday	= Settings.eDonkey.EnableToday;
	m_bEnableAlways	= Settings.eDonkey.EnableAlways;
	m_nLinks		= Settings.eDonkey.MaxLinks;
	m_bServerWalk	= Settings.eDonkey.ServerWalk;
	m_nResults		= Settings.eDonkey.MaxResults;

	UpdateData( FALSE );

	m_wndResults.EnableWindow( m_bServerWalk );
	m_wndResultsSpin.SetRange( 0, 201 );
	m_wndLinksSpin.SetRange( 0, 2048 );

	return TRUE;
}

BOOL CDonkeySettingsPage::OnSetActive()
{
	CNetworksSettingsPage* ppNetworks =
		(CNetworksSettingsPage*)GetPage( RUNTIME_CLASS(CNetworksSettingsPage) );

	if ( ppNetworks->GetSafeHwnd() != NULL )
	{
		ppNetworks->UpdateData( TRUE );
		m_bEnableToday = ppNetworks->m_bEDEnable;
		UpdateData( FALSE );
	}

	return CSettingsPage::OnSetActive();
}

void CDonkeySettingsPage::OnEnableToday()
{
	UpdateData( TRUE );

	if ( m_bEnableToday && ( Settings.GetOutgoingBandwidth() < 2 ) )
	{
		CString strMessage;
		LoadString( strMessage, IDS_NETWORK_BANDWIDTH_LOW );
		AfxMessageBox( strMessage, MB_OK );
		m_bEnableToday = FALSE;
		UpdateData( FALSE );
	}

	CNetworksSettingsPage* ppNetworks =
		(CNetworksSettingsPage*)GetPage( RUNTIME_CLASS(CNetworksSettingsPage) );

	if ( ppNetworks->GetSafeHwnd() != NULL )
	{
		ppNetworks->UpdateData( TRUE );
		ppNetworks->m_bEDEnable = m_bEnableToday;
		ppNetworks->UpdateData( FALSE );
	}
}

void CDonkeySettingsPage::OnServerWalk()
{
	UpdateData();
	m_wndResultsSpin.EnableWindow( m_bServerWalk );
	m_wndResults.EnableWindow( m_bServerWalk );
}

void CDonkeySettingsPage::OnImportDownloads()
{
	TCHAR szPath[MAX_PATH];
	LPITEMIDLIST pPath;
	LPMALLOC pMalloc;
	BROWSEINFO pBI;

	ZeroMemory( &pBI, sizeof(pBI) );
	pBI.hwndOwner		= AfxGetMainWnd()->GetSafeHwnd();
	pBI.pszDisplayName	= szPath;
	pBI.lpszTitle		= _T("Select your temp (download) folder:");
	pBI.ulFlags			= BIF_RETURNONLYFSDIRS;

	pPath = SHBrowseForFolder( &pBI );

	if ( pPath == NULL ) return;

	SHGetPathFromIDList( pPath, szPath );
	SHGetMalloc( &pMalloc );
	pMalloc->Free( pPath );
	pMalloc->Release();

	CDonkeyImportDlg dlg;
	dlg.m_pImporter.AddFolder( szPath );
	dlg.DoModal();
}

void CDonkeySettingsPage::OnDiscoveryGo()
{
	CDonkeyServersDlg dlg;
	dlg.DoModal();
}

void CDonkeySettingsPage::OnOK()
{
	UpdateData();

	Settings.eDonkey.EnableAlways	= m_bEnableAlways && ( Settings.GetOutgoingBandwidth() >= 2 );
	Settings.eDonkey.EnableToday	= ( m_bEnableToday || Settings.eDonkey.EnableAlways ) && ( Settings.GetOutgoingBandwidth() >= 2 );
	Settings.eDonkey.MaxLinks		= m_nLinks;
	Settings.eDonkey.ServerWalk		= m_bServerWalk;
	Settings.eDonkey.MaxResults		= m_nResults;

	CSettingsPage::OnOK();
}
