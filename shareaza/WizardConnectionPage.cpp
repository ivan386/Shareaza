//
// WizardConnectionPage.cpp
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
#include "Network.h"
#include "WizardSheet.h"
#include "WizardConnectionPage.h"
#include "UploadQueues.h"
#include "Skin.h"
#include "DlgHelp.h"

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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWizardConnectionPage property page

CWizardConnectionPage::CWizardConnectionPage() : CWizardPage(CWizardConnectionPage::IDD)
{
	//{{AFX_DATA_INIT(CWizardConnectionPage)
	//}}AFX_DATA_INIT
}

CWizardConnectionPage::~CWizardConnectionPage()
{
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
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CWizardConnectionPage message handlers

BOOL CWizardConnectionPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	Skin.Apply( _T("CWizardConnectionPage"), this );
	
	m_wndGroup.SetCurSel( 0 );
	
	/*
	m_wndType.SetItemData( 0, 0 );
	m_wndType.SetItemData( 1, 56 );
	m_wndType.SetItemData( 2, 128 );
	m_wndType.SetItemData( 3, 350 );
	m_wndType.SetItemData( 4, 576 );
	m_wndType.SetItemData( 5, 1544 );
	m_wndType.SetItemData( 6, 45000 );
	m_wndType.SetItemData( 7, 100000 );
	m_wndType.SetItemData( 8, 155000 );
	*/
	//; Dialup Modem; ISDN; Cable Modem/DSL; T1; T3; LAN; OC3;

	m_wndType.SetItemData( 0, 0 );
	m_wndType.SetItemData( 1, 56 );
	m_wndType.SetItemData( 2, 128 );
	m_wndType.SetItemData( 3, 256);
	m_wndType.SetItemData( 4, 512);
	m_wndType.SetItemData( 5, 768);
	m_wndType.SetItemData( 6, 1536 );
	m_wndType.SetItemData( 7, 1544 );
	m_wndType.SetItemData( 8, 1544 );
	m_wndType.SetItemData( 9, 45000 );
	m_wndType.SetItemData(10, 100000 );
	m_wndType.SetItemData(11, 155000 );
	//; Dial up Modem; ISDN; ADSL (256K); ADSL (512K); ADSL (768K); ADSL (1.5M); Cable Modem/SDSL; T1; T3; LAN; OC3;
	
	CString strSpeed;
	strSpeed.Format( _T(" %lu.0 kbps"), Settings.Connection.InSpeed );
	m_wndSpeed.SetWindowText( strSpeed );

	//; 28.8 kbps; 33.6 kbps; 56.6 kbps; 64.0 kbps; 128 kbps; 256 kbps; 512 kbps; 1024 kbps; 1536 kbps; 2048 kbps; 3072 kbps; 4096 kbps; 5120 kbps;
	
	return TRUE;
}

BOOL CWizardConnectionPage::OnSetActive() 
{
	SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );
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
		CString strFormat, strMessage;
		
		switch ( m_wndHomeSelect.GetCurSel() )
		{
		case 0:
			Settings.Connection.Firewalled	= FALSE;
			if ( Settings.Connection.InPort == 6346 )
				Settings.Connection.InPort	= Network.RandomPort();
			break;
		case 1:
			Settings.Connection.Firewalled	= FALSE;
			// Settings.Connection.InPort		= 6346;
			LoadString( strFormat, IDS_WIZARD_PORT_FORWARD );
			strMessage.Format( strFormat, Settings.Connection.InPort );
			AfxMessageBox( strMessage, MB_ICONINFORMATION );
			break;
		case 2:
			Settings.Connection.Firewalled	= TRUE;
			Settings.Connection.InPort		= 6346;
			break;
		case 3:
			Settings.Connection.Firewalled	= TRUE;
			Settings.Connection.InPort		= 6346;
			break;
		}
	}
	else
	{
		switch ( m_wndLanSelect.GetCurSel() )
		{
		case 0:
			Settings.Connection.Firewalled	= FALSE;
			if ( Settings.Connection.InPort == 6346 )
				Settings.Connection.InPort	= Network.RandomPort();
			break;
		case 1:
			Settings.Connection.Firewalled	= TRUE;
			Settings.Connection.InPort		= 6346;
			break;
		case 2:
			Settings.Connection.Firewalled	= TRUE;
			Settings.Connection.InPort		= 6346;
			break;
		}
	}
	
	int nIndex	= m_wndType.GetCurSel();
	int nSpeed	= 0;
	
	if ( nIndex > 0 )
	{
		nSpeed = m_wndType.GetItemData( nIndex );
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
	//Settings.Connection.OutSpeed	= nSpeed == 56 ? 32 : nSpeed;
	if( nSpeed <= 56 )
		Settings.Connection.OutSpeed = 32;			// Dial up modem
	else if( nSpeed <= 128 )
		Settings.Connection.OutSpeed = nSpeed;		// ISDN
	else if( nSpeed <= 700 )
		Settings.Connection.OutSpeed = nSpeed / 4;	// ADSL (4:1)
	else if( nSpeed <= 1536 )
		Settings.Connection.OutSpeed = nSpeed / 6;	// ADSL (6:1)
	else
		Settings.Connection.OutSpeed = nSpeed;		//Cable, SDSL, and the big boys.


	
	if ( nSpeed > 750 )
	{
		Settings.Gnutella2.NumPeers = max( Settings.Gnutella2.NumPeers, 4 );
	}
	
	Settings.eDonkey.MaxLinks = ( nSpeed < 100 || ! theApp.m_bNT ) ? 35 : 250;
	
	if ( nSpeed > 256 && theApp.m_bNT && !theApp.m_bLimitedConnections )
	{
		Settings.Downloads.MaxFiles			= 32;
		Settings.Downloads.MaxTransfers		= 128;
		Settings.Downloads.MaxFileTransfers	= 8;
	}
	else if ( nSpeed > 80 && theApp.m_bNT )
	{
		Settings.Downloads.MaxFiles			= 16;
		Settings.Downloads.MaxTransfers		= 64;
		Settings.Downloads.MaxFileTransfers	= 6;
	}
	else
	{
		Settings.Downloads.MaxFiles			= 8;
		Settings.Downloads.MaxTransfers		= 32;
		Settings.Downloads.MaxFileTransfers	= 4;
	}
	
	UploadQueues.CreateDefault();

	if ( theApp.m_bLimitedConnections ) 
	{	//Window XP Service Pack 2
		theApp.Message( MSG_ERROR, _T("Warning - Windows XP Service Pack 2 detected. Performance may be reduced.") );
		Settings.Downloads.ConnectThrottle	= max( Settings.Downloads.ConnectThrottle, 2000 );
		Settings.Gnutella.ConnectFactor		= min( Settings.Downloads.ConnectThrottle, 2 );
		Settings.Gnutella2.NumHubs			= min( Settings.Gnutella2.NumHubs, 2 );
		Settings.Gnutella1.EnableAlways		= FALSE;
		Settings.Gnutella1.EnableToday		= FALSE;
		Settings.Downloads.MaxFileSearches	= 1;
		//Settings.Connection.TimeoutConnect	= 30000;
		Settings.Connection.TimeoutHandshake= 60000;

		CHelpDlg::Show( _T("GeneralHelp.XPsp2") );
	}
	
	return 0;
}

