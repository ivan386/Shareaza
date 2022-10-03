//
// WizardConnectionPage.cpp
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
#include "Network.h"
#include "WizardSheet.h"
#include "WizardConnectionPage.h"
#include "UploadQueues.h"
#include "Skin.h"
#include "DlgHelp.h"
#include "HostCache.h"
#include "DiscoveryServices.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CWizardConnectionPage, CWizardPage)

BEGIN_MESSAGE_MAP(CWizardConnectionPage, CWizardPage)
	ON_CBN_SELCHANGE(IDC_CONNECTION_TYPE, &CWizardConnectionPage::OnSelChangeConnectionType)
	ON_CBN_EDITCHANGE(IDC_WIZARD_DOWNLOAD_SPEED, &CWizardConnectionPage::OnChangeConnectionSpeed)
	ON_CBN_SELCHANGE(IDC_WIZARD_DOWNLOAD_SPEED, &CWizardConnectionPage::OnChangeConnectionSpeed)
	ON_CBN_EDITCHANGE(IDC_WIZARD_UPLOAD_SPEED, &CWizardConnectionPage::OnChangeConnectionSpeed)
	ON_CBN_SELCHANGE(IDC_WIZARD_UPLOAD_SPEED, &CWizardConnectionPage::OnChangeConnectionSpeed)
	ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizardConnectionPage property page

CWizardConnectionPage::CWizardConnectionPage()
	: CWizardPage			( CWizardConnectionPage::IDD )
	, m_bQueryDiscoveries(false)
	, m_bUpdateDonkeyServers(false)
	, m_nProgressSteps(0)
{
}

CWizardConnectionPage::~CWizardConnectionPage()
{
}

void CWizardConnectionPage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CONNECTION_TYPE, m_wndType);
	DDX_Control(pDX, IDC_WIZARD_DOWNLOAD_SPEED, m_wndDownloadSpeed);
	DDX_Control(pDX, IDC_WIZARD_UPLOAD_SPEED, m_wndUploadSpeed);
	DDX_Control(pDX, IDC_WIZARD_UPNP, m_wndUPnP);
	DDX_Control(pDX, IDC_CONNECTION_PROGRESS, m_wndProgress);
	DDX_Control(pDX, IDC_CONNECTION_STATUS, m_wndStatus);
}

/////////////////////////////////////////////////////////////////////////////
// CWizardConnectionPage message handlers

BOOL CWizardConnectionPage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	Skin.Apply( _T("CWizardConnectionPage"), this );

	CString strTemp;

	m_wndType.SetItemData( 0, 56 );		// Dial up Modem;
	m_wndType.SetItemData( 1, 128 );	// ISDN
	m_wndType.SetItemData( 2, 256);		// ADSL (256K)
	m_wndType.SetItemData( 3, 512);		// ADSL (512K)
	m_wndType.SetItemData( 4, 768);		// ADSL (768K)
	m_wndType.SetItemData( 5, 1536 );	// ADSL (1.5M)
	m_wndType.SetItemData( 6, 4096 );	// ADSL (4.0M)
	m_wndType.SetItemData( 7, 8192 );	// ADSL2 (8.0M)
	m_wndType.SetItemData( 8, 12288 );	// ADSL2 (12.0M)
	m_wndType.SetItemData( 9, 24576 );	// ADSL2+ (24.0M)
	m_wndType.SetItemData(10, 1550 );	// Cable Modem/SDSL
	m_wndType.SetItemData(11, 1544 );	// T1
	m_wndType.SetItemData(12, 45000 );	// T3
	m_wndType.SetItemData(13, 102400 );	// LAN
	m_wndType.SetItemData(14, 155000 );	// OC3
	m_wndType.SetCurSel( -1 );
	//Dial up Modem;ISDN;ADSL (256K);ADSL (512K);ADSL (768K);ADSL (1.5M);ADSL (4.0M);ADSL2 (8.0M);ADSL2 (12.0M);ADSL2+ (24.0M);Cable Modem/SDSL;T1;T3;LAN;OC3;

	const double nSpeeds[] = { 28.8, 33.6, 56, 64, 128, 256, 384, 512, 640, 768, 1024, 1536, 1544, 1550, 2048, 3072, 4096, 5120, 8192, 10240, 12288, 24576, 45000, 102400, 155000, 0 };
	for ( int nSpeed = 0 ; nSpeeds[ nSpeed ] ; nSpeed++ )
	{
		strTemp.Format( _T("%lg kbps"), nSpeeds[ nSpeed ] );
		m_wndDownloadSpeed.AddString( strTemp );
		m_wndUploadSpeed.AddString( strTemp );
	}

	strTemp.Format( _T("%lu kbps"), Settings.Connection.InSpeed );
	m_wndDownloadSpeed.SetWindowText( strTemp );
	strTemp.Format( _T("%lu kbps"), Settings.Connection.OutSpeed );
	m_wndUploadSpeed.SetWindowText( strTemp );

	m_wndUPnP.AddString( LoadString( IDS_GENERAL_YES ) );
	m_wndUPnP.AddString( LoadString( IDS_GENERAL_NO ) );
	m_wndUPnP.SetCurSel( Settings.Connection.EnableUPnP ? 0 : 1 );

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

void CWizardConnectionPage::OnSelChangeConnectionType()
{
	m_wndDownloadSpeed.SetWindowText( _T("") );
	m_wndUploadSpeed.SetWindowText( _T("") );
}

void CWizardConnectionPage::OnChangeConnectionSpeed()
{
	m_wndType.SetCurSel( -1 );
}

LRESULT CWizardConnectionPage::OnWizardNext()
{
	CWaitCursor pCursor;

	if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) return 0;

	DWORD nDownloadSpeed = 0, nUploadSpeed = 0;
	DWORD nSpeed	= 0;
	int nIndex		= m_wndType.GetCurSel();
	
	Settings.Connection.EnableUPnP = ( m_wndUPnP.GetCurSel() == 0 );

	if ( nIndex >= 0 )
	{
		nSpeed = static_cast< DWORD >( m_wndType.GetItemData( nIndex ) );
		nDownloadSpeed = nSpeed;

		if( nSpeed <= 56 )								// Dial up modem
			nUploadSpeed = 32;
		else if( nSpeed <= 128 )						// ISDN
			nUploadSpeed = nSpeed;
		else if( nSpeed == 384 )						// 384/128 DSL (Europe)
			nUploadSpeed = 128;
		else if( nSpeed <= 700 )						// ADSL (4:1)
			nUploadSpeed = nSpeed / 4;
		else if( nSpeed <  1544 )						// ADSL (6:1)
			nUploadSpeed = nSpeed / 6;
		else if( nSpeed == 1544 )						// T1 (1:1)
			nUploadSpeed = nSpeed;
		else if( nSpeed <= 4000 )						// Cable (2:1)
			nUploadSpeed = nSpeed / 2;
		else if( nSpeed <= 8192 )						// ADSL2 (8:1)
			nUploadSpeed = nSpeed / 8;
		else if( nSpeed <= 12288 )						// ADSL2 (10:1)
			nUploadSpeed = nSpeed / 10;
		else if( nSpeed <= 24576 )						// ADSL2+ (12:1)
			nUploadSpeed = nSpeed / 12;
		else											// High capacity lines. (LAN, etc)
			nUploadSpeed = nSpeed;
	}
	else
	{
		CString strSpeed;
		double nTemp;

		m_wndDownloadSpeed.GetWindowText( strSpeed );
		if ( _stscanf( strSpeed, _T("%lf"), &nTemp ) == 1 )
			nDownloadSpeed = (DWORD)nTemp;

		m_wndUploadSpeed.GetWindowText( strSpeed );
		if ( _stscanf( strSpeed, _T("%lf"), &nTemp ) == 1 )
			nUploadSpeed = (DWORD)nTemp;
	}

	if ( nDownloadSpeed <= 0 || nUploadSpeed <= 0 )
	{
		CString strSpeed;
		LoadString( strSpeed, IDS_WIZARD_NEED_SPEED );
		AfxMessageBox( strSpeed, MB_ICONEXCLAMATION );
		return -1;
	}

#ifdef LAN_MODE
	Settings.Connection.InSpeed = 40960;
	Settings.Connection.OutSpeed = 40960;
#else  // LAN_MODE
	Settings.Connection.InSpeed = nDownloadSpeed;
	Settings.Connection.OutSpeed = nUploadSpeed;
#endif // LAN_MODE

	// Set upload limit to 90% of capacity, trimmed down to the nearest KB.
	Settings.Bandwidth.Uploads = ( ( ( Settings.Connection.OutSpeed *
		( 100 - Settings.Uploads.FreeBandwidthFactor ) ) / 100 ) / 8 ) * 1024;

	Settings.eDonkey.MaxLinks = nSpeed < 100 ? 35 : 250;
	Settings.OnChangeConnectionSpeed();
	UploadQueues.CreateDefault();

	//if ( theApp.m_bLimitedConnections && !Settings.General.IgnoreXPsp2 )
	//	CHelpDlg::Show( _T("GeneralHelp.XPsp2") );

	m_nProgressSteps = 0;

	// Load default ed2k server list (if necessary)
	m_bUpdateDonkeyServers = true;
	m_nProgressSteps += 30;

	// Update the G1, G2 and eDonkey host cache (if necessary)
	m_bQueryDiscoveries = true;
	m_nProgressSteps += 30;

	BeginThread( "WizardConnectionPage" );

	// Disable all navigation buttons while the thread is running
	CWizardSheet* pSheet = GetSheet();
	if ( pSheet->GetDlgItem( ID_WIZBACK ) )
		pSheet->GetDlgItem( ID_WIZBACK )->EnableWindow( FALSE );
	if ( pSheet->GetDlgItem( ID_WIZNEXT ) )
		pSheet->GetDlgItem( ID_WIZNEXT )->EnableWindow( FALSE );
	if ( pSheet->GetDlgItem( 2 ) )
		pSheet->GetDlgItem( 2 )->EnableWindow( FALSE );
	return -1; // don't move to the next page; the thread will do this work
}

/////////////////////////////////////////////////////////////////////////////
// CWizardConnectionPage thread

void CWizardConnectionPage::OnRun()
{
	short nCurrentStep = 0;
	CString strMessage;

	m_wndProgress.PostMessage( PBM_SETRANGE32, 0, (LPARAM)m_nProgressSteps );

	if ( m_bUpdateDonkeyServers )
	{
		LoadString( strMessage, IDS_WIZARD_ED2K );
		m_wndStatus.SetWindowText( strMessage );

		HostCache.CheckMinimumServers( PROTOCOL_ED2K );
		nCurrentStep +=30;
		m_wndProgress.PostMessage( PBM_SETPOS, nCurrentStep );
	}

	if ( m_bQueryDiscoveries )
	{
		LoadString( strMessage, IDS_WIZARD_DISCOVERY );
		m_wndStatus.SetWindowText( strMessage );

		DiscoveryServices.CheckMinimumServices();
		nCurrentStep +=10;
		m_wndProgress.PostMessage( PBM_SETPOS, nCurrentStep );

		BOOL bConnected = Network.IsConnected();
		if ( bConnected || Network.Connect(TRUE) )
		{
			int i;
			// It will be checked if it is needed inside DiscoveryServices.Execute()
			for ( i = 0; i < 2 && !DiscoveryServices.Execute( PROTOCOL_G1, 2); i++ ) Sleep(200);
			nCurrentStep += 5;
			m_wndProgress.PostMessage( PBM_SETPOS, nCurrentStep );
			for ( i = 0; i < 2 && !DiscoveryServices.Execute( PROTOCOL_G2, 2); i++ ) Sleep(200);
			nCurrentStep += 5;
			m_wndProgress.PostMessage( PBM_SETPOS, nCurrentStep );
			for ( i = 0; i < 2 && !DiscoveryServices.Execute( PROTOCOL_ED2K, 2); i++ ) Sleep(200);
			nCurrentStep += 5;
			m_wndProgress.PostMessage( PBM_SETPOS, nCurrentStep );
			for ( i = 0; i < 2 && !DiscoveryServices.Execute( PROTOCOL_DC, 2); i++ ) Sleep(200);
			nCurrentStep += 5;
			m_wndProgress.PostMessage( PBM_SETPOS, nCurrentStep );
		}
		else
		{
			nCurrentStep += 20;
			m_wndProgress.PostMessage( PBM_SETPOS, nCurrentStep );
		}
	}

	CWizardSheet* pSheet = GetSheet();
	if ( pSheet->GetDlgItem( ID_WIZBACK ) )
		pSheet->GetDlgItem( ID_WIZBACK )->EnableWindow();
	if ( pSheet->GetDlgItem( ID_WIZNEXT ) )
		pSheet->GetDlgItem( ID_WIZNEXT )->EnableWindow();
	if ( pSheet->GetDlgItem( 2 ) )
		pSheet->GetDlgItem( 2 )->EnableWindow();

	pSheet->SendMessage( PSM_SETCURSEL, 2, 0 );	// Go to the 3rd page
	PostMessage( WM_TIMER, 1 );					// Terminate thread if necessarily
}

BOOL CWizardConnectionPage::OnQueryCancel()
{
	if ( IsThreadAlive() )
		return FALSE;

	return CWizardPage::OnQueryCancel();
}

void CWizardConnectionPage::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent != 1 ) return;

	CloseThread();
}
