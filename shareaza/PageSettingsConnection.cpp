//
// PageSettingsConnection.cpp
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
#include "PageSettingsConnection.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CConnectionSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CConnectionSettingsPage, CSettingsPage)
	//{{AFX_MSG_MAP(CConnectionSettingsPage)
	ON_CBN_EDITCHANGE(IDC_INBOUND_HOST, OnEditChangeInboundHost)
	ON_CBN_CLOSEUP(IDC_INBOUND_HOST, OnCloseUpInboundHost)
	ON_EN_CHANGE(IDC_INBOUND_PORT, OnChangeInboundPort)
	ON_BN_CLICKED(IDC_INBOUND_RANDOM, OnInboundRandom)
	//}}AFX_MSG_MAP
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CConnectionSettingsPage property page

CConnectionSettingsPage::CConnectionSettingsPage() : CSettingsPage(CConnectionSettingsPage::IDD)
{
	//{{AFX_DATA_INIT(CConnectionSettingsPage)
	m_bIgnoreLocalIP = FALSE;
	m_bInBind = FALSE;
	m_sInHost = _T("");
	m_nInPort = 0;
	m_sOutHost = _T("");
	m_nTimeoutConnection = 0;
	m_nTimeoutHandshake = 0;
	m_bCanAccept = FALSE;
	m_sOutSpeed = _T("");
	m_sInSpeed = _T("");
	m_bInRandom = FALSE;
	//}}AFX_DATA_INIT
}

CConnectionSettingsPage::~CConnectionSettingsPage()
{
}

void CConnectionSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConnectionSettingsPage)
	DDX_Control(pDX, IDC_INBOUND_PORT, m_wndInPort);
	DDX_Control(pDX, IDC_INBOUND_SPEED, m_wndInSpeed);
	DDX_Control(pDX, IDC_OUTBOUND_SPEED, m_wndOutSpeed);
	DDX_Control(pDX, IDC_INBOUND_HOST, m_wndInHost);
	DDX_Control(pDX, IDC_INBOUND_BIND, m_wndInBind);
	DDX_Control(pDX, IDC_TIMEOUT_HANDSHAKE_SPIN, m_wndTimeoutHandshake);
	DDX_Control(pDX, IDC_TIMEOUT_CONNECTION_SPIN, m_wndTimeoutConnection);
	DDX_Check(pDX, IDC_IGNORE_LOCAL, m_bIgnoreLocalIP);
	DDX_Check(pDX, IDC_INBOUND_BIND, m_bInBind);
	DDX_CBString(pDX, IDC_INBOUND_HOST, m_sInHost);
	DDX_Text(pDX, IDC_INBOUND_PORT, m_nInPort);
	DDX_CBString(pDX, IDC_OUTBOUND_HOST, m_sOutHost);
	DDX_Text(pDX, IDC_TIMEOUT_CONNECTION, m_nTimeoutConnection);
	DDX_Text(pDX, IDC_TIMEOUT_HANDSHAKE, m_nTimeoutHandshake);
	DDX_Check(pDX, IDC_CAN_ACCEPT, m_bCanAccept);
	DDX_CBString(pDX, IDC_OUTBOUND_SPEED, m_sOutSpeed);
	DDX_CBString(pDX, IDC_INBOUND_SPEED, m_sInSpeed);
	DDX_Check(pDX, IDC_INBOUND_RANDOM, m_bInRandom);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CConnectionSettingsPage message handlers

BOOL CConnectionSettingsPage::OnInitDialog() 
{
	CSettingsPage::OnInitDialog();
	
	CString strAutomatic = GetInOutHostTranslation();
	CComboBox* pOutHost = (CComboBox*) GetDlgItem( IDC_OUTBOUND_HOST );

	// update all dropdowns
	m_wndInHost.DeleteString( 0 );
	m_wndInHost.AddString( strAutomatic );
	pOutHost->DeleteString( 0 );
	pOutHost->AddString( strAutomatic );

	m_bCanAccept			= ! Settings.Connection.Firewalled;
	m_sInHost				= Settings.Connection.InHost;
	m_nInPort				= Settings.Connection.InPort;
	m_bInBind				= Settings.Connection.InBind;
	m_sOutHost				= Settings.Connection.OutHost;
	m_bIgnoreLocalIP		= Settings.Connection.IgnoreLocalIP;
	m_nTimeoutConnection	= Settings.Connection.TimeoutConnect / 1000;
	m_nTimeoutHandshake		= Settings.Connection.TimeoutHandshake / 1000;

	if ( m_sInHost.IsEmpty() ) m_sInHost = strAutomatic;
	if ( m_sOutHost.IsEmpty() ) m_sOutHost = strAutomatic;

	m_bInRandom = ( m_nInPort == 0 );

	m_wndTimeoutConnection.SetRange( 1, 480 );
	m_wndTimeoutHandshake.SetRange( 1, 480 );

	UpdateData( FALSE );
	
	m_wndInBind.EnableWindow( m_sInHost != strAutomatic);

	return TRUE;
}

CString CConnectionSettingsPage::FormatSpeed(DWORD nSpeed)
{
	return Settings.SmartVolume( nSpeed, TRUE, TRUE );
}

DWORD CConnectionSettingsPage::ParseSpeed(LPCTSTR psz)
{
	return (DWORD)Settings.ParseVolume( psz, TRUE ) / 1024;
}

void CConnectionSettingsPage::OnEditChangeInboundHost() 
{
	CString strAutomatic = GetInOutHostTranslation();

	UpdateData();

	m_wndInBind.EnableWindow( m_sInHost != strAutomatic );
}

void CConnectionSettingsPage::OnCloseUpInboundHost() 
{
	m_wndInBind.EnableWindow( m_wndInHost.GetCurSel() != 0 );
}

void CConnectionSettingsPage::OnChangeInboundPort() 
{
	UpdateData();
	BOOL bRandom = m_nInPort == 0;

	if ( bRandom != m_bInRandom )
	{
		m_bInRandom = bRandom;
		UpdateData( FALSE );
	}
}

void CConnectionSettingsPage::OnInboundRandom() 
{
	UpdateData();
	
	if ( m_bInRandom && m_nInPort != 0 )
	{
		m_nInPort = 0;
		UpdateData( FALSE );
	}
}

BOOL CConnectionSettingsPage::OnKillActive()
{
	UpdateData();
	
	if ( ParseSpeed( m_sInSpeed ) == 0 )
	{
		CString strMessage;
		LoadString( strMessage, IDS_SETTINGS_NEED_BANDWIDTH );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		m_wndInSpeed.SetFocus();
		return FALSE;
	}
	
	if ( ParseSpeed( m_sOutSpeed ) == 0 )
	{
		CString strMessage;
		LoadString( strMessage, IDS_SETTINGS_NEED_BANDWIDTH );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		m_wndOutSpeed.SetFocus();
		return FALSE;
	}
	
	return CSettingsPage::OnKillActive();
}

void CConnectionSettingsPage::OnOK() 
{
	UpdateData();
	
	CString strAutomatic = GetInOutHostTranslation();

	if ( m_sInHost.CompareNoCase( strAutomatic ) == 0 ) 
		m_sInHost.Empty();
	if ( m_sOutHost.CompareNoCase( strAutomatic ) == 0 ) 
		m_sOutHost.Empty();
	
	Settings.Connection.Firewalled			= ! m_bCanAccept;
	Settings.Connection.InHost				= m_sInHost;
	Settings.Connection.InPort				= m_nInPort;
	Settings.Connection.InBind				= m_bInBind;
	Settings.Connection.OutHost				= m_sOutHost;
	Settings.Connection.InSpeed				= ParseSpeed( m_sInSpeed );
	Settings.Connection.OutSpeed			= ParseSpeed( m_sOutSpeed );
	Settings.Connection.IgnoreLocalIP		= m_bIgnoreLocalIP;
	Settings.Connection.TimeoutConnect		= m_nTimeoutConnection * 1000;
	Settings.Connection.TimeoutHandshake	= m_nTimeoutHandshake  * 1000;

	UpdateData();
	CSettingsPage::OnOK();
}

CString CConnectionSettingsPage::GetInOutHostTranslation() 
{
	CString strAutomatic, strInCombo, strOutCombo, strNew;

	LoadString( strAutomatic, IDS_SETTINGS_AUTOMATIC_IP );

	m_wndInHost.GetLBText( 0, strInCombo );
	CComboBox* pOutHost = (CComboBox*) GetDlgItem( IDC_OUTBOUND_HOST );
	pOutHost->GetLBText( 0, strOutCombo );

	// get non-english string if any
	strNew = strInCombo.CompareNoCase( _T("Automatic") ) == 0 ? strOutCombo : strInCombo;
	return strAutomatic.CompareNoCase( _T("Automatic") ) == 0 ? strNew : strAutomatic;
}

void CConnectionSettingsPage::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CSettingsPage::OnShowWindow(bShow, nStatus);
	if ( bShow )
	{
		// Update speed units
		m_sOutSpeed	= FormatSpeed( Settings.Connection.OutSpeed );
		m_sInSpeed	= FormatSpeed( Settings.Connection.InSpeed );
		
		// Dropdown
		m_wndInSpeed.ResetContent();
		m_wndOutSpeed.ResetContent();
		const DWORD nSpeeds[] = { 28, 33, 56, 64, 128, 350, 576, 768, 1544, 3072, 45000, 100000, 155000, 0 };
		for ( int nSpeed = 0 ; nSpeeds[ nSpeed ] ; nSpeed++ )
		{
			CString str = FormatSpeed( nSpeeds[ nSpeed ] );
			m_wndInSpeed.AddString( str );
			m_wndOutSpeed.AddString( str );
		}

		UpdateData( FALSE );
	}
}
