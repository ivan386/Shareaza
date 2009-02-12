//
// DlgURLAction.cpp
//
// Copyright © Shareaza Development Team, 2002-2009.
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
#include "Transfers.h"
#include "Network.h"
#include "Library.h"
#include "SharedFile.h"
#include "HostCache.h"
#include "DiscoveryServices.h"
#include "Skin.h"
#include "DlgURLAction.h"
#include "WndMain.h"
#include "WndSearch.h"
#include "WndDownloads.h"
#include "WndBrowseHost.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CURLActionDlg, CSkinDialog)
	//{{AFX_MSG_MAP(CURLActionDlg)
	ON_BN_CLICKED(IDC_URL_DOWNLOAD, OnUrlDownload)
	ON_BN_CLICKED(IDC_URL_SEARCH, OnUrlSearch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CURLActionDlg construction

CURLActionDlg::CURLActionDlg(CWnd* pParent, CShareazaURL* pURL, BOOL bMultiple) : CSkinDialog(CURLActionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CURLActionDlg)
	m_sNameTitle = _T("");
	m_sNameValue = _T("");
	m_sHashTitle = _T("");
	m_sHashValue = _T("");
	m_bNewWindow = FALSE;
	m_bAlwaysOpen = FALSE;
	//}}AFX_DATA_INIT

	m_pURLs.AddTail( pURL );
	m_bMultiple = bMultiple;
}

CURLActionDlg::~CURLActionDlg()
{
	for ( POSITION pos = m_pURLs.GetHeadPosition() ; pos ; )
	{
		delete m_pURLs.GetNext( pos );
	}
}

void CURLActionDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CURLActionDlg)
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
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CURLActionDlg message handlers

BOOL CURLActionDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CURLActionDlg"), IDR_MAINFRAME );

	m_bAlwaysOpen	= Settings.General.AlwaysOpenURLs;
	m_bNewWindow	= Settings.Downloads.ShowMonitorURLs;

	Update();

	if ( m_bAlwaysOpen )
	{
		if ( m_wndDownload.IsWindowEnabled() )
			PostMessage( WM_COMMAND, IDC_URL_DOWNLOAD );
		else
			PostMessage( WM_COMMAND, IDC_URL_SEARCH );
	}
	else
	{
		m_bMultiple = FALSE;

		if ( CWnd* pWnd = AfxGetMainWnd() )
		{
			if ( pWnd->IsWindowVisible() && ! pWnd->IsIconic() )
			{
				pWnd->BringWindowToTop();
				pWnd->SetForegroundWindow();
			}
		}
	}

	return FALSE;
}

void CURLActionDlg::AddURL(CShareazaURL* pURL)
{
	if ( IsWindowVisible() && m_pURLs.GetCount() > 0 )
	{
		CShareazaURL* pFirst = m_pURLs.GetHead();

		if ( pFirst->m_nAction == pURL->m_nAction )
		{
			m_pURLs.AddTail( pURL );
			Update();
			return;
		}
	}

	delete pURL;
}

void CURLActionDlg::Update()
{
	CShareazaURL* pURL = m_pURLs.GetHead();

	CString strMessage;

	if ( pURL->m_nAction == CShareazaURL::uriHost ||
		 pURL->m_nAction == CShareazaURL::uriDonkeyServer )
	{
		LoadString(m_sNameTitle, IDS_URL_HOST );
		LoadString(m_sHashTitle, IDS_URL_PORT );

		m_sNameValue = pURL->m_sName;
		m_sHashValue.Format( _T("%lu"), pURL->m_nPort );

		m_wndMessage2.ShowWindow( SW_SHOW );
		m_wndNewWindow.ShowWindow( SW_HIDE );

		LoadString( strMessage, IDS_URL_CONNECT );
		m_wndDownload.SetWindowText( strMessage );
		m_wndDownload.SetFocus();

		if ( pURL->m_nAction == CShareazaURL::uriHost )
		{
			LoadString(strMessage, IDS_URL_BROWSE );
			m_wndSearch.SetWindowText( strMessage );
		}
		else
			m_wndSearch.ShowWindow( SW_HIDE );
	}
	else if ( pURL->m_nAction == CShareazaURL::uriBrowse )
	{
		LoadString(m_sNameTitle, IDS_URL_HOST );
		LoadString(m_sHashTitle, IDS_URL_PORT );

		m_sNameValue = pURL->m_sName;
		m_sHashValue.Format( _T("%lu"), pURL->m_nPort );

		m_wndMessage3.ShowWindow( SW_SHOW );
		m_wndNewWindow.ShowWindow( SW_HIDE );

		LoadString(strMessage, IDS_URL_BROWSE );
		m_wndDownload.SetWindowText( strMessage );
		m_wndDownload.SetFocus();
		LoadString(strMessage, IDS_URL_CONNECT );
		m_wndSearch.SetWindowText( strMessage );
	}
	else if ( pURL->m_nAction == CShareazaURL::uriDiscovery )
	{
		LoadString(m_sNameTitle, IDS_URL_URL );
		LoadString(m_sHashTitle, IDS_URL_TYPE );

		if ( m_pURLs.GetCount() == 1 )
		{
			m_sNameValue = pURL->m_sURL;
		}
		else
		{
			m_sNameValue.Format( _T("%i URL(s)"), m_pURLs.GetCount() );
		}

		switch ( pURL->m_nSize )
		{
		case CDiscoveryService::dsWebCache:
			m_sHashValue = _T("GWebCache");
			break;
		case CDiscoveryService::dsServerMet:
			m_sHashValue = _T("Server.met URL");
			break;
		}

		m_wndMessage4.ShowWindow( SW_SHOW );
		LoadString(strMessage, IDS_URL_ADD );
		m_wndDownload.SetWindowText( strMessage );
		m_wndSearch.ShowWindow( SW_HIDE );
		m_wndNewWindow.ShowWindow( SW_HIDE );
	}
	else if ( pURL->m_nAction == CShareazaURL::uriSource )
	{
		LoadString(m_sNameTitle, IDS_URL_URL );

		if ( m_pURLs.GetCount() == 1 )
		{
			m_sNameValue = pURL->m_sURL;
		}
		else
		{
			m_sNameValue.Format( _T("%i URL(s)"), m_pURLs.GetCount() );
		}

		m_wndMessage1.ShowWindow( SW_SHOW );
		m_wndSearch.ShowWindow( SW_HIDE );
	}
	else
	{
		LoadString(m_sNameTitle, IDS_URL_FILENAME );
		m_sHashTitle = _T("URN:");

		if ( m_pURLs.GetCount() > 1 )
		{
			m_sNameValue.Format( _T("%i file(s)"), m_pURLs.GetCount() );
		}
		else if ( pURL->m_sName.GetLength() )
		{
			m_sNameValue = pURL->m_sName;

			if ( pURL->m_bSize )
				m_sNameValue += _T(" (") + Settings.SmartVolume( pURL->m_nSize ) + _T(")");
		}
		else
		{
			LoadString(m_sNameValue, IDS_URL_UNSPECIFIED );
		}

		if ( m_pURLs.GetCount() > 1 )
		{
			m_sHashValue.Format( _T("%i file(s)"), m_pURLs.GetCount() );
		}
		else if ( pURL->m_oTiger && pURL->m_oSHA1 )
		{
			m_sHashValue	= _T("bitprint:")
							+ pURL->m_oSHA1.toString() + _T(".")
							+ pURL->m_oTiger.toString();
		}
		else if ( pURL->m_oTiger )
		{
			m_sHashValue = pURL->m_oTiger.toShortUrn();
		}
		else if ( pURL->m_oSHA1 )
		{
			m_sHashValue = pURL->m_oSHA1.toShortUrn();
		}
		else if ( pURL->m_oED2K )
		{
			m_sHashValue = pURL->m_oED2K.toShortUrn();
		}
		else
		{
			LoadString(m_sHashValue, IDS_URL_UNSPECIFIED );
		}

		m_wndMessage1.ShowWindow( SW_SHOW );

		if ( pURL->m_nAction == CShareazaURL::uriDownload )
		{
			m_wndDownload.SetFocus();
		}
		else if ( pURL->m_nAction == CShareazaURL::uriSearch )
		{
			m_wndDownload.EnableWindow( FALSE );
			m_wndDownload.ModifyStyle( BS_DEFPUSHBUTTON, 0 );
			m_wndSearch.ModifyStyle( 0, BS_DEFPUSHBUTTON );
			m_wndSearch.SetFocus();
			m_wndNewWindow.ShowWindow( SW_HIDE );
		}
	}

	UpdateData( FALSE );
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

	for ( POSITION pos = m_pURLs.GetHeadPosition() ; pos ; )
	{
		CShareazaURL* pURL = m_pURLs.GetNext( pos );

		if ( pURL->m_nAction == CShareazaURL::uriDownload ||
			 pURL->m_nAction == CShareazaURL::uriSource )
		{
			CLibraryFile* pFile;

			{
				CSingleLock oLock( &Library.m_pSection, TRUE );
				if ( ( pFile = LibraryMaps.LookupFileBySHA1( pURL->m_oSHA1 ) ) != NULL
					|| ( pFile = LibraryMaps.LookupFileByED2K( pURL->m_oED2K ) ) != NULL
					|| ( pFile = LibraryMaps.LookupFileByBTH( pURL->m_oBTH ) ) != NULL
					|| ( pFile = LibraryMaps.LookupFileByMD5( pURL->m_oMD5 ) ) != NULL )
				{
					CString strFormat, strMessage;
					::Skin.LoadString( strFormat, IDS_URL_ALREADY_HAVE );
					strMessage.Format( strFormat, (LPCTSTR)pFile->m_sName );
					oLock.Unlock();

					UINT nMBOX = AfxMessageBox( strMessage, MB_ICONINFORMATION|MB_YESNOCANCEL|MB_DEFBUTTON2 );
					if ( nMBOX == IDCANCEL ) return;
					if ( nMBOX == IDNO ) continue;
				}
			}

			CDownload* pDownload = Downloads.Add( *pURL );

			if ( pDownload == NULL ) continue;

			if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 )
			{
//				if ( pURL->m_bED2K && HostCache.eDonkey.GetNewest() != NULL )
//					Settings.eDonkey.EnableToday = TRUE;

				if ( ! Network.IsWellConnected() ) Network.Connect( TRUE );
			}

			if ( m_bMultiple == FALSE )
			{
				CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd();
				pMainWnd->m_pWindows.Open( RUNTIME_CLASS(CDownloadsWnd) );

				if ( Settings.Downloads.ShowMonitorURLs && m_pURLs.GetCount() == 1 )
				{
					CSingleLock pLock( &Transfers.m_pSection, TRUE );
					if ( Downloads.Check( pDownload ) ) pDownload->ShowMonitor( &pLock );
				}
			}
		}
		else if ( pURL->m_nAction == CShareazaURL::uriHost )
		{
			Network.ConnectTo( pURL->m_sName, pURL->m_nPort );
		}
		else if ( pURL->m_nAction == CShareazaURL::uriDonkeyServer )
		{
			Network.ConnectTo( pURL->m_sName, pURL->m_nPort, PROTOCOL_ED2K );
		}
		else if ( pURL->m_nAction == CShareazaURL::uriBrowse )
		{
			SOCKADDR_IN pAddress;

			if ( Network.Resolve( pURL->m_sName, pURL->m_nPort, &pAddress ) )
			{
				new CBrowseHostWnd( &pAddress );
			}
		}
		else if ( pURL->m_nAction == CShareazaURL::uriDiscovery )
		{
			DiscoveryServices.Add( pURL->m_sURL, (int)pURL->m_nSize );
		}
	}

	CSkinDialog::OnOK();
}

void CURLActionDlg::OnUrlSearch()
{
	Settings.General.AlwaysOpenURLs = m_bAlwaysOpen != FALSE;

	for ( POSITION pos = m_pURLs.GetHeadPosition() ; pos ; )
	{
		CShareazaURL* pURL = m_pURLs.GetNext( pos );

		if ( pURL->m_nAction == CShareazaURL::uriHost )
		{
			SOCKADDR_IN pAddress;

			if ( Network.Resolve( pURL->m_sName, pURL->m_nPort, &pAddress ) )
			{
				new CBrowseHostWnd( &pAddress );
			}
		}
		else if ( pURL->m_nAction == CShareazaURL::uriBrowse )
		{
			Network.ConnectTo( pURL->m_sName, pURL->m_nPort );
		}
		else if (	pURL->m_nAction == CShareazaURL::uriDownload ||
					pURL->m_nAction == CShareazaURL::uriSearch )
		{
			if ( ! Network.IsWellConnected() ) Network.Connect( TRUE );

			new CSearchWnd( pURL->ToQuery() );
		}
	}

	CSkinDialog::OnOK();
}
