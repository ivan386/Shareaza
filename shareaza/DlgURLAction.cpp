//
// DlgURLAction.cpp
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
#include "ShareazaURL.h"
#include "Download.h"
#include "Downloads.h"
#include "HostCache.h"
#include "Transfers.h"
#include "Network.h"
#include "Library.h"
#include "SharedFile.h"
#include "HostCache.h"
#include "DiscoveryServices.h"
#include "Skin.h"
#include "DlgURLAction.h"
#include "DlgExistingFile.h"
#include "WndMain.h"
#include "WndSearch.h"
#include "WndDownloads.h"
#include "WndBrowseHost.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CURLActionDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CURLActionDlg, CSkinDialog)
	ON_BN_CLICKED(IDC_URL_DOWNLOAD, &CURLActionDlg::OnUrlDownload)
	ON_BN_CLICKED(IDC_URL_SEARCH, &CURLActionDlg::OnUrlSearch)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CURLActionDlg construction

CURLActionDlg::CURLActionDlg(CShareazaURL* pURL)
	: CSkinDialog( CURLActionDlg::IDD )
	, m_bNewWindow( FALSE )
	, m_bAlwaysOpen( FALSE )
{
	if ( pURL )
	{
		m_pURL = pURL;
		Create( CURLActionDlg::IDD );
		ShowWindow( SW_SHOW );
	}
}

CURLActionDlg::~CURLActionDlg()
{
	delete m_pURL;
}

void CURLActionDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MESSAGE_4, m_wndMessage4);
	DDX_Control(pDX, IDC_MESSAGE_3, m_wndMessage3);
	DDX_Control(pDX, IDC_NEW_WINDOW, m_wndNewWindow);
	DDX_Control(pDX, IDCANCEL, m_wndCancel);
	DDX_Control(pDX, IDC_MESSAGE_2, m_wndMessage2);
	DDX_Control(pDX, IDC_MESSAGE_1, m_wndMessage1);
	DDX_Control(pDX, IDC_URL_SEARCH, m_wndSearch);
	DDX_Control(pDX, IDC_URL_DOWNLOAD, m_wndDownload);
	DDX_Text(pDX, IDC_URL_NAME_TITLE, m_sNameTitle);
	DDX_Text(pDX, IDC_URL_NAME_VALUE, m_sNameValue);
	DDX_Text(pDX, IDC_URL_URN_TITLE, m_sHashTitle);
	DDX_Text(pDX, IDC_URL_URN_VALUE, m_sHashValue);
	DDX_Check(pDX, IDC_NEW_WINDOW, m_bNewWindow);
	DDX_Check(pDX, IDC_ALWAYS_OPEN, m_bAlwaysOpen);
}

/////////////////////////////////////////////////////////////////////////////
// CURLActionDlg message handlers

BOOL CURLActionDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CURLActionDlg"), IDR_MAINFRAME );

	m_bAlwaysOpen	= Settings.General.AlwaysOpenURLs;
	m_bNewWindow	= Settings.Downloads.ShowMonitorURLs;

	CString strMessage;

	if ( m_pURL->m_nAction == CShareazaURL::uriHost )
	{
		LoadString(m_sNameTitle, IDS_URL_HOST );
		LoadString(m_sHashTitle, IDS_URL_PORT );

		m_sNameValue = m_pURL->m_sName;
		m_sHashValue.Format( _T("%lu"), m_pURL->m_nPort );

		m_wndMessage2.ShowWindow( SW_SHOW );
		m_wndNewWindow.ShowWindow( SW_HIDE );

		LoadString( strMessage, IDS_URL_CONNECT );
		m_wndDownload.SetWindowText( strMessage );
		m_wndDownload.SetFocus();

		if ( m_pURL->m_nProtocol != PROTOCOL_ED2K &&
			 m_pURL->m_nProtocol != PROTOCOL_BT &&
			 m_pURL->m_nProtocol != PROTOCOL_KAD )
		{
			LoadString(strMessage, IDS_URL_BROWSE );
			m_wndSearch.SetWindowText( strMessage );
		}
		else
			m_wndSearch.ShowWindow( SW_HIDE );
	}
	else if ( m_pURL->m_nAction == CShareazaURL::uriBrowse )
	{
		LoadString(m_sNameTitle, IDS_URL_HOST );
		LoadString(m_sHashTitle, IDS_URL_PORT );

		m_sNameValue = m_pURL->m_sName;
		m_sHashValue.Format( _T("%lu"), m_pURL->m_nPort );

		m_wndMessage3.ShowWindow( SW_SHOW );
		m_wndNewWindow.ShowWindow( SW_HIDE );

		LoadString(strMessage, IDS_URL_BROWSE );
		m_wndDownload.SetWindowText( strMessage );
		m_wndDownload.SetFocus();
		LoadString(strMessage, IDS_URL_CONNECT );
		m_wndSearch.SetWindowText( strMessage );
	}
	else if ( m_pURL->m_nAction == CShareazaURL::uriDiscovery )
	{
		LoadString(m_sNameTitle, IDS_URL_URL );
		LoadString(m_sHashTitle, IDS_URL_TYPE );

		m_sNameValue = m_pURL->m_sURL;

		switch ( m_pURL->GetDiscoveryService() )
		{
		case CDiscoveryService::dsGnutella:
			m_sHashValue = _T( "Gnutella Bootstrap" );
			break;
		case CDiscoveryService::dsWebCache:
			m_sHashValue = _T( "G1/G2 GWebCache" );
			break;
		case CDiscoveryService::dsServerMet:
			m_sHashValue = _T( "Server.met URL" );
			break;
		case CDiscoveryService::dsDCHubList:
			m_sHashValue = _T( "DC++ Hub List URL" );
			break;
		default:
			m_sHashValue.Empty();
		}

		m_wndMessage4.ShowWindow( SW_SHOW );
		LoadString(strMessage, IDS_URL_ADD );
		m_wndDownload.SetWindowText( strMessage );
		m_wndSearch.ShowWindow( SW_HIDE );
		m_wndNewWindow.ShowWindow( SW_HIDE );
	}
	else if ( m_pURL->m_nAction == CShareazaURL::uriSource )
	{
		LoadString(m_sNameTitle, IDS_URL_URL );

		m_sNameValue = m_pURL->m_sURL;

		m_wndMessage1.ShowWindow( SW_SHOW );
		m_wndSearch.ShowWindow( SW_HIDE );
	}
	else
	{
		LoadString(m_sNameTitle, IDS_URL_FILENAME );
		m_sHashTitle = _T("URN:");

		if ( m_pURL->m_sName.GetLength() )
		{
			m_sNameValue = m_pURL->m_sName;

			if ( m_pURL->m_nSize != SIZE_UNKNOWN )
				m_sNameValue += _T(" (") + Settings.SmartVolume( m_pURL->m_nSize ) + _T(")");
		}
		else
		{
			LoadString(m_sNameValue, IDS_URL_UNSPECIFIED );
		}

		if ( m_pURL->HasHash() )
		{
			m_sHashValue = m_pURL->GetShortURN();
		}
		else
		{
			LoadString(m_sHashValue, IDS_URL_UNSPECIFIED );
		}

		m_wndMessage1.ShowWindow( SW_SHOW );

		if ( m_pURL->m_nAction == CShareazaURL::uriDownload )
		{
			m_wndDownload.SetFocus();
		}
		else if ( m_pURL->m_nAction == CShareazaURL::uriSearch )
		{
			m_wndDownload.EnableWindow( FALSE );
			m_wndDownload.ModifyStyle( BS_DEFPUSHBUTTON, 0 );
			m_wndSearch.ModifyStyle( 0, BS_DEFPUSHBUTTON );
			m_wndSearch.SetFocus();
			m_wndNewWindow.ShowWindow( SW_HIDE );
		}
	}

	UpdateData( FALSE );

	if ( m_bAlwaysOpen )
	{
		if ( m_wndDownload.IsWindowEnabled() )
			PostMessage( WM_COMMAND, IDC_URL_DOWNLOAD );
		else
			PostMessage( WM_COMMAND, IDC_URL_SEARCH );
	}

	return FALSE;
}

BOOL CURLActionDlg::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN )
	{
		if ( GetFocus() == &m_wndCancel )
			PostMessage( WM_COMMAND, IDCANCEL );
		else if ( m_wndDownload.IsWindowEnabled() )
			PostMessage( WM_COMMAND, IDC_URL_DOWNLOAD );
		else if ( m_wndSearch.IsWindowEnabled() )
			PostMessage( WM_COMMAND, IDC_URL_SEARCH );
		return TRUE;
	}

	return CSkinDialog::PreTranslateMessage( pMsg );
}

void CURLActionDlg::OnUrlDownload()
{
	UpdateData();

	Settings.General.AlwaysOpenURLs		= m_bAlwaysOpen != FALSE;
	Settings.Downloads.ShowMonitorURLs	= m_bNewWindow != FALSE;

	if ( m_pURL->m_nAction == CShareazaURL::uriDownload ||
		 m_pURL->m_nAction == CShareazaURL::uriSource )
	{
		CExistingFileDlg::Action action = CExistingFileDlg::CheckExisting( m_pURL );
		if ( action == CExistingFileDlg::Cancel )
		{
			return;
		}
		else if ( action != CExistingFileDlg::Download )
		{
			DestroyWindow();
			return;
		}

		if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 && ! Network.IsWellConnected() )
			Network.Connect( TRUE );

		if ( CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd() )
			pMainWnd->m_pWindows.Open( RUNTIME_CLASS(CDownloadsWnd) );

		CSingleLock pLock( &Transfers.m_pSection, TRUE );

		if ( CDownload* pDownload = Downloads.Add( *m_pURL ) )
		{
			if ( Settings.Downloads.ShowMonitorURLs )
				pDownload->ShowMonitor();
		}
	}
	else if ( m_pURL->m_nAction == CShareazaURL::uriHost )
	{
		Network.ConnectTo( m_pURL->m_sName, m_pURL->m_nPort, m_pURL->m_nProtocol );
	}
	else if ( m_pURL->m_nAction == CShareazaURL::uriBrowse )
	{
		SOCKADDR_IN pAddress;

		if ( Network.Resolve( m_pURL->m_sName, m_pURL->m_nPort, &pAddress ) )
		{
			new CBrowseHostWnd( m_pURL->m_nProtocol, &pAddress );
		}
	}
	else if ( m_pURL->m_nAction == CShareazaURL::uriDiscovery )
	{
		DiscoveryServices.Add( m_pURL->m_sURL, m_pURL->GetDiscoveryService() );
	}

	DestroyWindow();
}

void CURLActionDlg::OnUrlSearch()
{
	Settings.General.AlwaysOpenURLs = m_bAlwaysOpen != FALSE;

	if ( m_pURL->m_nAction == CShareazaURL::uriHost )
	{
		SOCKADDR_IN pAddress;

		if ( Network.Resolve( m_pURL->m_sName, m_pURL->m_nPort, &pAddress ) )
		{
			new CBrowseHostWnd( m_pURL->m_nProtocol, &pAddress );
		}
	}
	else if ( m_pURL->m_nAction == CShareazaURL::uriBrowse )
	{
		Network.ConnectTo( m_pURL->m_sName, m_pURL->m_nPort );
	}
	else if (	m_pURL->m_nAction == CShareazaURL::uriDownload ||
				m_pURL->m_nAction == CShareazaURL::uriSearch )
	{
		if ( ! Network.IsWellConnected() ) Network.Connect( TRUE );

		new CSearchWnd( m_pURL->ToQuery() );
	}

	DestroyWindow();
}

void CURLActionDlg::OnCancel()
{
	DestroyWindow();
}

void CURLActionDlg::PostNcDestroy()
{
	delete this;
}
