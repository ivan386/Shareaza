//
// WizardConnectionPage.cpp
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
#include "Network.h"
#include "WizardSheet.h"
#include "WizardConnectionPage.h"
#include "UploadQueues.h"
#include "Skin.h"
#include "DlgHelp.h"
#include "HostCache.h"
#include "DiscoveryServices.h"
#include "UPnPFinder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CWizardConnectionPage, CWizardPage)

BEGIN_MESSAGE_MAP(CWizardConnectionPage, CWizardPage)
	//{{AFX_MSG_MAP(CWizardConnectionPage)
	ON_CBN_SELCHANGE(IDC_CONNECTION_TYPE, OnSelChangeConnectionType)
	ON_CBN_EDITCHANGE(IDC_CONNECTION_SPEED, OnEditChangeConnectionSpeed)
	ON_CBN_SELCHANGE(IDC_CONNECTION_SPEED, OnSelChangeConnectionSpeed)
	ON_CBN_SELCHANGE(IDC_CONNECTION_GROUP, OnSelChangeConnectionGroup)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWizardConnectionPage property page

CWizardConnectionPage::CWizardConnectionPage() : CWizardPage(CWizardConnectionPage::IDD)
, m_bQueryDiscoveries(false)
, m_bUpdateDonkeyServers(false)
, m_nProgressSteps(0)
, m_hThread(NULL)
{
}

CWizardConnectionPage::~CWizardConnectionPage()
{
	ASSERT( m_hThread == NULL );
}

void CWizardConnectionPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizardConnectionPage)
	DDX_Control(pDX, IDC_CONNECTION_LAN_SELECT, m_wndLanSelect);
	DDX_Control(pDX, IDC_CONNECTION_LAN_LABEL, m_wndLanLabel);
	DDX_Control(pDX, IDC_CONNECTION_HOME_SELECT, m_wndHomeSelect);
	DDX_Control(pDX, IDC_CONNECTION_HOME_LABEL, m_wndHomeLabel);
	DDX_Control(pDX, IDC_CONNECTION_GROUP, m_wndGroup);
	DDX_Control(pDX, IDC_CONNECTION_SPEED, m_wndSpeed);
	DDX_Control(pDX, IDC_CONNECTION_TYPE, m_wndType);
	DDX_Control(pDX, IDC_CONNECTION_PROGRESS, m_wndProgress);
	DDX_Control(pDX, IDC_CONNECTION_STATUS, m_wndStatus);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CWizardConnectionPage message handlers

BOOL CWizardConnectionPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	Skin.Apply( _T("CWizardConnectionPage"), this );
	
	m_wndGroup.SetCurSel( 0 );
	
	m_wndType.SetItemData( 0, 0 );
	m_wndType.SetItemData( 1, 56 );		// Dial up Modem;
	m_wndType.SetItemData( 2, 128 );	// ISDN
	m_wndType.SetItemData( 3, 256);		// ADSL (256K)
	m_wndType.SetItemData( 4, 512);		// ADSL (512K)
	m_wndType.SetItemData( 5, 768);		// ADSL (768K)
	m_wndType.SetItemData( 6, 1536 );	// ADSL (1.5M)
	m_wndType.SetItemData( 7, 4096 );	// ADSL (4.0M)
	m_wndType.SetItemData( 8, 8192 );	// ADSL2 (8.0M)
	m_wndType.SetItemData( 9, 12288 );	// ADSL2 (12.0M)
	m_wndType.SetItemData(10, 24576 );	// ADSL2+ (24.0M)
	m_wndType.SetItemData(11, 1550 );	// Cable Modem/SDSL
	m_wndType.SetItemData(12, 1544 );	// T1
	m_wndType.SetItemData(13, 45000 );	// T3
	m_wndType.SetItemData(14, 100000 );	// LAN
	m_wndType.SetItemData(15, 155000 );	// OC3
	//; Dial up Modem; ISDN; ADSL (256K); ADSL (512K); ADSL (768K); ADSL (1.5M); ADSL (4.0M); ADSL2 (8.0M); ADSL2 (12.0M); ADSL2+ (24.0M); Cable Modem/SDSL; T1; T3; LAN; OC3;
	
	CString strSpeed;
	strSpeed.Format( _T(" %lu.0 kbps"), Settings.Connection.InSpeed );
	m_wndSpeed.SetWindowText( strSpeed );

	//; 28.8 kbps; 33.6 kbps; 56.6 kbps; 64.0 kbps; 128 kbps; 256 kbps; 384 kbps; 512 kbps; 1024 kbps; 1536 kbps; 2048 kbps; 3072 kbps; 4096 kbps; 5120 kbps; 8192 kbps; 12288 kbps;

	// 3 steps with 30 sub-steps each
	m_wndProgress.SetRange( 0, 90 );
	m_wndProgress.SetPos( 0 );

	m_wndStatus.SetWindowText( L"" );

	return TRUE;
}

BOOL CWizardConnectionPage::OnSetActive() 
{
	SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );
	m_wndProgress.SetPos( 0 );
	m_wndStatus.SetWindowText( L"" );
	return CWizardPage::OnSetActive();
}

void CWizardConnectionPage::OnSelChangeConnectionGroup() 
{
	int nGroup = m_wndGroup.GetCurSel();
	
	m_wndHomeLabel.ShowWindow( nGroup == 1 ? SW_SHOW : SW_HIDE );
	m_wndHomeSelect.ShowWindow( nGroup == 1 ? SW_SHOW : SW_HIDE );
	m_wndLanLabel.ShowWindow( nGroup > 1 ? SW_SHOW : SW_HIDE );
	m_wndLanSelect.ShowWindow( nGroup > 1 ? SW_SHOW : SW_HIDE );
	
	m_wndHomeSelect.SetCurSel( 2 );
	m_wndLanSelect.SetCurSel( nGroup == 2 ? 0 : 1 );
	
	m_wndSpeed.SetWindowText( _T("") );
	m_wndSpeed.SetCurSel( -1 );
	m_wndSpeed.EnableWindow( FALSE );
	m_wndType.EnableWindow( TRUE );
	m_wndType.SetCurSel( nGroup == 1 ? 1 : 8 );
}

void CWizardConnectionPage::OnSelChangeConnectionType() 
{
	m_wndSpeed.EnableWindow( m_wndType.GetCurSel() < 1 );
	if ( m_wndType.GetCurSel() > 0 ) m_wndSpeed.SetWindowText( _T("") );
}

void CWizardConnectionPage::OnEditChangeConnectionSpeed()
{
	CString strSpeed;
	m_wndSpeed.GetWindowText( strSpeed );
	m_wndType.EnableWindow( strSpeed.IsEmpty() );
	if ( strSpeed.GetLength() ) m_wndType.SetCurSel( -1 );
}

void CWizardConnectionPage::OnSelChangeConnectionSpeed()
{
	m_wndType.EnableWindow( m_wndSpeed.GetCurSel() < 1 );
	if ( m_wndSpeed.GetCurSel() > 0 ) m_wndType.SetCurSel( -1 );
}

LRESULT CWizardConnectionPage::OnWizardNext() 
{
	if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) return 0;

	int nGroup = m_wndGroup.GetCurSel();
	
	if ( nGroup <= 0 )
	{
		CString strMessage;
		LoadString( strMessage, IDS_WIZARD_NEED_CONNECTION );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		return -1;
	}
	else if ( nGroup == 1 )
	{
		switch ( m_wndHomeSelect.GetCurSel() )
		{
		case 0:
			Settings.Connection.FirewallStatus = CONNECTION_OPEN;
			if ( Settings.Connection.InPort == 6346 )
				Settings.Connection.InPort	= Network.RandomPort();
			break;
		case 1:
			Settings.Connection.FirewallStatus = CONNECTION_OPEN;
			// Settings.Connection.InPort		= 6346;
			break;
		case 2:
			Settings.Connection.FirewallStatus = CONNECTION_FIREWALLED;
			Settings.Connection.InPort		= 6346;
			break;
		case 3:
			Settings.Connection.FirewallStatus = CONNECTION_AUTO;
			Settings.Connection.InPort		= 6346;
			break;
		}
	}
	else
	{
		switch ( m_wndLanSelect.GetCurSel() )
		{
		case 0:
			Settings.Connection.FirewallStatus = CONNECTION_OPEN;
			if ( Settings.Connection.InPort == 6346 )
				Settings.Connection.InPort	= Network.RandomPort();
			break;
		case 1:
			Settings.Connection.FirewallStatus = CONNECTION_FIREWALLED;
			Settings.Connection.InPort		= 6346;
			break;
		case 2:
			Settings.Connection.FirewallStatus = CONNECTION_AUTO;
			Settings.Connection.InPort		= 6346;
			break;
		}
	}
	
	int nIndex	= m_wndType.GetCurSel();
	DWORD nSpeed	= 0;
	
	if ( nIndex > 0 )
	{
		nSpeed = static_cast< DWORD >( m_wndType.GetItemData( nIndex ) );
	}
	else
	{
		CString strSpeed;
		m_wndSpeed.GetWindowText( strSpeed );
		
		if ( _stscanf( strSpeed, _T("%lu"), &nSpeed ) != 1 || nSpeed <= 0 )
		{
			LoadString( strSpeed, IDS_WIZARD_NEED_SPEED );
			AfxMessageBox( strSpeed, MB_ICONEXCLAMATION );
			return -1;
		}
	}
	
	Settings.Connection.InSpeed		= nSpeed;

	if( nSpeed <= 56 )								// Dial up modem
		Settings.Connection.OutSpeed = 32;
	else if( nSpeed <= 128 )						// ISDN
		Settings.Connection.OutSpeed = nSpeed;
	else if( nSpeed == 384 )						// 384/128 DSL (Europe)
		Settings.Connection.OutSpeed = 128;
	else if( nSpeed <= 700 )						// ADSL (4:1)
		Settings.Connection.OutSpeed = nSpeed / 4;
	else if( nSpeed <  1544 )						// ADSL (6:1)
		Settings.Connection.OutSpeed = nSpeed / 6;
	else if( nSpeed == 1544 )						// T1 (1:1)
		Settings.Connection.OutSpeed = nSpeed;
	else if( nSpeed <= 4000 )						// Cable (2:1)
		Settings.Connection.OutSpeed = nSpeed / 2;
	else if( nSpeed <= 8192 )						// ADSL2 (8:1)
		Settings.Connection.OutSpeed = nSpeed / 8;
	else if( nSpeed <= 12288 )						// ADSL2 (10:1)
		Settings.Connection.OutSpeed = nSpeed / 10;
	else if( nSpeed <= 24576 )						// ADSL2+ (12:1)
		Settings.Connection.OutSpeed = nSpeed / 12;
	else											// High capacity lines. (LAN, etc)
		Settings.Connection.OutSpeed = nSpeed;

	// Set upload limit to 90% of capacity, trimmed down to the nearest KB. (Usually works out at ~85% total)
	Settings.Bandwidth.Uploads = (DWORD)( Settings.Connection.OutSpeed * 0.9 );
	Settings.Bandwidth.Uploads >>= 3;
	Settings.Bandwidth.Uploads *= 1024;
	
	if ( nSpeed > 750 )
	{
		Settings.Gnutella2.NumPeers = max( Settings.Gnutella2.NumPeers, 4 );
	}
	
	Settings.eDonkey.MaxLinks = ( nSpeed < 100 || ! theApp.m_bNT ) ? 35 : 250;
	
	if ( nSpeed > 2500 && theApp.m_bNT && ( !theApp.m_bLimitedConnections || Settings.General.IgnoreXPsp2 ) )
	{	// Very high capacity connection
		Settings.Downloads.MaxFiles				= 32;
		Settings.Downloads.MaxTransfers			= 200;
		Settings.Downloads.MaxFileTransfers		= 32;
		Settings.Downloads.MaxConnectingSources	= 32;
		Settings.Downloads.MaxFileSearches		= 3;

		Settings.Gnutella2.NumLeafs				= 400; //Can probably support more leaves
		Settings.BitTorrent.DownloadTorrents	= 4;	// Should be able to handle several torrents
	}
	else if ( nSpeed > 768 && theApp.m_bNT )
	{	// Fast broadband
		Settings.Downloads.MaxFiles				= 26;
		Settings.Downloads.MaxTransfers			= 100;
		Settings.Downloads.MaxFileTransfers		= 10;
		Settings.Downloads.MaxConnectingSources	= 28;
		Settings.Downloads.MaxFileSearches		= 2;
	}
	else if ( nSpeed > 256 && theApp.m_bNT )
	{	// Slower broadband
		Settings.Downloads.MaxFiles				= 20;
		Settings.Downloads.MaxTransfers			= 64;
		Settings.Downloads.MaxFileTransfers		= 8;
		Settings.Downloads.MaxConnectingSources	= 24;
		Settings.Downloads.MaxFileSearches		= 1;
		Settings.Search.GeneralThrottle			= 250;
	}
	else if ( nSpeed > 80 && theApp.m_bNT )
	{	// IDSN, Dual modems, etc
		Settings.Downloads.MaxFiles				= 14;
		Settings.Downloads.MaxTransfers			= 32;
		Settings.Downloads.MaxFileTransfers		= 6;
		Settings.Downloads.MaxConnectingSources	= 20;
		Settings.Downloads.MaxFileSearches		= 0;
		Settings.Search.GeneralThrottle			= 250;	// Slow searches a little so we don't get flooded
	}
	else
	{	// Modem users / Win9x
		Settings.Downloads.MaxFiles				= 8;
		Settings.Downloads.MaxTransfers			= 24;
		Settings.Downloads.MaxFileTransfers		= 4;
		Settings.Downloads.MaxConnectingSources	= 16;
		Settings.Downloads.MaxFileSearches		= 0;
		Settings.Downloads.SourcesWanted		= 200;	// Don't bother requesting so many sources
		Settings.Search.GeneralThrottle			= 300;	// Slow searches a little so we don't get flooded

		Settings.BitTorrent.DownloadTorrents	= 1;	// Best not to try too many torrents
	}
	
	UploadQueues.CreateDefault();

	if ( ( theApp.m_bLimitedConnections ) && ( ! Settings.General.IgnoreXPsp2 ) ) 
	{	// Window XP Service Pack 2
		theApp.Message( MSG_ERROR, _T("Warning  - Windows XP Service Pack 2 detected. Performance may be reduced.") );
		Settings.Downloads.ConnectThrottle		= max( Settings.Downloads.ConnectThrottle, 800u );
		Settings.Connection.ConnectThrottle		= max( Settings.Connection.ConnectThrottle, 250u );
		Settings.Gnutella.ConnectFactor			= min( Settings.Gnutella.ConnectFactor, 3u );
		Settings.Gnutella2.NumHubs				= min( Settings.Gnutella2.NumHubs, 2 );
		Settings.Gnutella1.EnableAlways			= FALSE;
		Settings.Gnutella1.EnableToday			= FALSE;
		Settings.Downloads.MaxConnectingSources	= 8;
		Settings.Connection.RequireForTransfers	= TRUE;
		Settings.Connection.SlowConnect			= TRUE;
		// Settings.Connection.TimeoutConnect	= 30000;
		// Settings.Connection.TimeoutHandshake	= 60000;

		CHelpDlg::Show( _T("GeneralHelp.XPsp2") );
	}

	// Update the G2 host cache (if necessary)
	if ( HostCache.Gnutella2.CountHosts() < 25 )
	{
		m_bQueryDiscoveries = true;
		m_nProgressSteps += 30;
	}

	// Load default ed2k server list (if necessary)
	if ( HostCache.eDonkey.CountHosts() < 8 )
	{
		m_bUpdateDonkeyServers = true;
		m_nProgressSteps += 30;
	}

	CWaitCursor pCursor;
	// Create UPnP finder object if it doesn't exist
	try
	{
		if ( !theApp.m_pUPnPFinder )
			theApp.m_pUPnPFinder.reset( new CUPnPFinder );
		if ( theApp.m_pUPnPFinder->AreServicesHealthy() )
			theApp.m_pUPnPFinder->StartDiscovery();
	}
	catch ( CUPnPFinder::UPnPError& ) {}
	catch ( CException* e ) { e->Delete(); }

	CWinThread* pThread = AfxBeginThread( ThreadStart, this, THREAD_PRIORITY_NORMAL );
	SetThreadName( pThread->m_nThreadID, "CWizardConnectionPage" );
	m_hThread = pThread->m_hThread;

	// Disable all navigation buttons while the thread is running
	CWizardSheet* pSheet = GetSheet();
	if ( pSheet->GetDlgItem( 0x3023 ) )
		pSheet->GetDlgItem( 0x3023 )->EnableWindow( FALSE );
	if ( pSheet->GetDlgItem( 0x3024 ) )
		pSheet->GetDlgItem( 0x3024 )->EnableWindow( FALSE );
	if ( pSheet->GetDlgItem( 2 ) )
		pSheet->GetDlgItem( 2 )->EnableWindow( FALSE );
	return -1; // don't move to the next page; the thread will do this work
}

/////////////////////////////////////////////////////////////////////////////
// CWizardConnectionPage thread

UINT CWizardConnectionPage::ThreadStart(LPVOID pParam)
{
	CWizardConnectionPage* pDlg = (CWizardConnectionPage*)pParam;
	pDlg->OnRun();
	return 0;
}

void CWizardConnectionPage::OnRun()
{
	short nCurrentStep = 0;
	CString strMessage;

	m_nProgressSteps += 30; // UPnP device detection
	m_wndProgress.PostMessage( PBM_SETRANGE32, 0, (LPARAM)m_nProgressSteps );

	LoadString( strMessage, IDS_WIZARD_UPNP_SETUP );
	m_wndStatus.SetWindowText( strMessage );
	
	DWORD tStart = GetTickCount();

	while ( theApp.m_pUPnPFinder && 
			theApp.m_pUPnPFinder->IsAsyncFindRunning() )
	{
		Sleep( 1000 );
		if ( GetTickCount() - tStart > 30000 )
		{
			theApp.m_pUPnPFinder->StopAsyncFind();
			Settings.Connection.EnableUPnP = TS_FALSE;
			break;
		}
		if ( nCurrentStep < 30  )
			nCurrentStep++;

		m_wndProgress.PostMessage( PBM_SETPOS, nCurrentStep );
	}

	nCurrentStep = 30;
	m_wndProgress.PostMessage( PBM_SETPOS, nCurrentStep );

	if ( m_bQueryDiscoveries )
	{
		LoadString( strMessage, IDS_WIZARD_DISCOVERY );
		m_wndStatus.SetWindowText( strMessage );
		DiscoveryServices.QueryForHosts( PROTOCOL_G2 );
		nCurrentStep +=30;
		m_wndProgress.PostMessage( PBM_SETPOS, nCurrentStep );
	}

	if ( m_bUpdateDonkeyServers )
	{
		LoadString( strMessage, IDS_WIZARD_ED2K );
		m_wndStatus.SetWindowText( strMessage );
		HostCache.eDonkey.LoadDefaultED2KServers();
		nCurrentStep +=30;
		m_wndProgress.PostMessage( PBM_SETPOS, nCurrentStep );
	}

	CWizardSheet* pSheet = GetSheet();
	if ( pSheet->GetDlgItem( 0x3023 ) )
		pSheet->GetDlgItem( 0x3023 )->EnableWindow();
	if ( pSheet->GetDlgItem( 0x3024 ) )
		pSheet->GetDlgItem( 0x3024 )->EnableWindow();
	if ( pSheet->GetDlgItem( 2 ) )
		pSheet->GetDlgItem( 2 )->EnableWindow();

	pSheet->SendMessage( PSM_SETCURSEL, 2, 0 ); // Go to the 3rd page
	PostMessage( WM_TIMER, 1 ); // Terminate thread if necessarily
}
BOOL CWizardConnectionPage::OnQueryCancel()
{
	if ( m_hThread )
		return FALSE;

	return CWizardPage::OnQueryCancel();
}

void CWizardConnectionPage::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent != 1 ) return;

	if ( m_hThread != NULL )
	{
        int nAttempt = 5;
		for ( ; nAttempt > 0 ; nAttempt-- )
		{
			DWORD nCode;

			if ( ! GetExitCodeThread( m_hThread, &nCode ) ) break;
			if ( nCode != STILL_ACTIVE ) break;
			Sleep( 100 );
		}

		if ( nAttempt == 0 )
		{
			TerminateThread( m_hThread, 0 );
			theApp.Message( MSG_DEBUG, _T("WARNING: Terminating CWizardConnectionPage thread.") );
			Sleep( 100 );
		}

		m_hThread = NULL;
	}

	if ( theApp.m_bUPnPPortsForwarded == TS_TRUE )
	{
		Settings.Connection.FirewallStatus = CONNECTION_OPEN;
	}
	else if ( m_wndGroup.GetCurSel() == 1 && 
			  m_wndHomeSelect.GetCurSel() == 1 )
	{
		CString strFormat, strMessage;
		LoadString( strFormat, IDS_WIZARD_PORT_FORWARD );
		strMessage.Format( strFormat, Settings.Connection.InPort );
		AfxMessageBox( strMessage, MB_ICONINFORMATION );
	}
}
