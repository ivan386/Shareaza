//
// PageSettingsDownloads.cpp
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
#include "Library.h"
#include "LibraryFolders.h"
#include "SharedFolder.h"
#include "Skin.h"
#include "PageSettingsDownloads.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CDownloadsSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CDownloadsSettingsPage, CSettingsPage)
	//{{AFX_MSG_MAP(CDownloadsSettingsPage)
	ON_BN_CLICKED(IDC_DOWNLOADS_BROWSE, OnDownloadsBrowse)
	ON_BN_CLICKED(IDC_INCOMPLETE_BROWSE, OnIncompleteBrowse)
	//}}AFX_MSG_MAP
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDownloadsSettingsPage property page

CDownloadsSettingsPage::CDownloadsSettingsPage() : CSettingsPage(CDownloadsSettingsPage::IDD)
{
	//{{AFX_DATA_INIT(CDownloadsSettingsPage)
	m_sDownloadsPath = _T("");
	m_sIncompletePath = _T("");
	m_nMaxDownFiles = 0;
	m_nMaxFileTransfers = 0;
	m_nMaxDownTransfers = 0;
	m_sBandwidthLimit = _T("");
	m_sQueueLimit = _T("");
	m_bRequireConnect = FALSE;
	//}}AFX_DATA_INIT
}

CDownloadsSettingsPage::~CDownloadsSettingsPage()
{
}

void CDownloadsSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDownloadsSettingsPage)
	DDX_Control(pDX, IDC_MAX_TRANSFERS_SPIN, m_wndMaxDownTransfers);
	DDX_Control(pDX, IDC_MAX_TPF_SPIN, m_wndMaxFileTransfers);
	DDX_Control(pDX, IDC_MAX_FILES_SPIN, m_wndMaxDownFiles);
	DDX_Control(pDX, IDC_INCOMPLETE_BROWSE, m_wndIncompletePath);
	DDX_Control(pDX, IDC_DOWNLOADS_BROWSE, m_wndDownloadsPath);
	DDX_Control(pDX, IDC_DOWNLOADS_BANDWIDTH_LIMIT, m_wndBandwidthLimit);
	DDX_Control(pDX, IDC_DOWNLOADS_QUEUE_LIMIT, m_wndQueueLimit);
	DDX_Text(pDX, IDC_DOWNLOADS_FOLDER, m_sDownloadsPath);
	DDX_Text(pDX, IDC_INCOMPLETE_FOLDER, m_sIncompletePath);
	DDX_Text(pDX, IDC_MAX_FILES, m_nMaxDownFiles);
	DDX_Text(pDX, IDC_MAX_TPF, m_nMaxFileTransfers);
	DDX_Text(pDX, IDC_MAX_TRANSFERS, m_nMaxDownTransfers);
	DDX_CBString(pDX, IDC_DOWNLOADS_BANDWIDTH_LIMIT, m_sBandwidthLimit);
	DDX_CBString(pDX, IDC_DOWNLOADS_QUEUE_LIMIT, m_sQueueLimit);
	DDX_Check(pDX, IDC_REQUIRE_CONNECT, m_bRequireConnect);
	//}}AFX_DATA_MAP
	
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadsSettingsPage message handlers

BOOL CDownloadsSettingsPage::OnInitDialog() 
{
	CSettingsPage::OnInitDialog();
	
	m_sDownloadsPath		= Settings.Downloads.CompletePath;
	m_sIncompletePath		= Settings.Downloads.IncompletePath;
	m_nMaxDownFiles			= Settings.Downloads.MaxFiles;
	m_nMaxDownTransfers		= Settings.Downloads.MaxTransfers;
	m_nMaxFileTransfers		= Settings.Downloads.MaxFileTransfers;
	m_bRequireConnect		= Settings.Connection.RequireForTransfers;
	
	m_wndMaxDownFiles.SetRange( 1, 100 );
	m_wndMaxDownTransfers.SetRange( 1, 200 );
	m_wndMaxFileTransfers.SetRange( 1, 100 );
	
	m_wndDownloadsPath.SetIcon( IDI_BROWSE );
	m_wndIncompletePath.SetIcon( IDI_BROWSE );

	if ( Settings.Downloads.QueueLimit )
		m_sQueueLimit.Format( _T("%d"), Settings.Downloads.QueueLimit );
	else
		m_sQueueLimit = _T("MAX");

	m_bDownloadsChanged = FALSE;
	
	UpdateData( FALSE );
	
	return TRUE;
}

void CDownloadsSettingsPage::OnDownloadsBrowse() 
{
	TCHAR szPath[MAX_PATH];
	LPITEMIDLIST pPath;
	LPMALLOC pMalloc;
	BROWSEINFO pBI;
		
	ZeroMemory( &pBI, sizeof(pBI) );
	pBI.hwndOwner		= AfxGetMainWnd()->GetSafeHwnd();
	pBI.pszDisplayName	= szPath;
	pBI.lpszTitle		= _T("Select folder for downloads:");
	pBI.ulFlags			= BIF_RETURNONLYFSDIRS;
	
	pPath = SHBrowseForFolder( &pBI );

	if ( pPath == NULL ) return;

	SHGetPathFromIDList( pPath, szPath );
	SHGetMalloc( &pMalloc );
	pMalloc->Free( pPath );
	pMalloc->Release();
	
	UpdateData( TRUE );
	m_sDownloadsPath = szPath;
	m_bDownloadsChanged = TRUE;
	UpdateData( FALSE );
}

void CDownloadsSettingsPage::OnIncompleteBrowse() 
{
	TCHAR szPath[MAX_PATH];
	LPITEMIDLIST pPath;
	LPMALLOC pMalloc;
	BROWSEINFO pBI;
		
	ZeroMemory( &pBI, sizeof(pBI) );
	pBI.hwndOwner		= AfxGetMainWnd()->GetSafeHwnd();
	pBI.pszDisplayName	= szPath;
	pBI.lpszTitle		= _T("Select folder for incomplete files:");
	pBI.ulFlags			= BIF_RETURNONLYFSDIRS;
	
	pPath = SHBrowseForFolder( &pBI );

	if ( pPath == NULL ) return;

	SHGetPathFromIDList( pPath, szPath );
	SHGetMalloc( &pMalloc );
	pMalloc->Free( pPath );
	pMalloc->Release();

	UpdateData( TRUE );
	m_sIncompletePath = szPath;
	UpdateData( FALSE );
}

BOOL CDownloadsSettingsPage::OnKillActive()
{
	UpdateData();
	
	if ( ( ! IsNotLimited( m_sBandwidthLimit ) ) && Settings.ParseVolume( m_sBandwidthLimit, TRUE ) == 0 )
	{
		CString strMessage;
		LoadString( strMessage, IDS_SETTINGS_NEED_BANDWIDTH );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		GetDlgItem( IDC_DOWNLOADS_BANDWIDTH_LIMIT )->SetFocus();
		return FALSE;
	}
	
	return CSettingsPage::OnKillActive();
}

void CDownloadsSettingsPage::OnOK() 
{
	DWORD nQueueLimit = 0;
	UpdateData( TRUE );

	// Figure out what the text in the queue limit box means
	if ( IsNotLimited( m_sQueueLimit ) )
	{
		// Max queue is not limited
		nQueueLimit = 0;
	}
	else
	{
		int nPosition = 1, nCount = m_sQueueLimit.GetLength();
		while ( nCount-- )
		{
			TCHAR cCharacter = m_sQueueLimit.GetAt( nCount );
			if ( ( cCharacter >= '0' ) &&
				 ( cCharacter <= '9' ) )
			{
				nQueueLimit += ( ( cCharacter - '0') * nPosition );
				nPosition *= 10;
			}
		}
	}

	// Check the queue limit value is okay
	if ( ( nQueueLimit > 0 ) && ( nQueueLimit < 2000 ) && ( ! Settings.Live.QueueLimitWarning ) &&
		 ( Settings.eDonkey.EnableToday || Settings.eDonkey.EnableAlways ) )
	{
		// Warn the user about setting the max queue wait limit too low
		CString strMessage;
		LoadString( strMessage, IDS_SETTINGS_WARN_QUEUELIMIT );
					
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDNO )
		{
			nQueueLimit = 0;
		}
		else
		{
			// Don't need to warn the user again.
			Settings.Live.QueueLimitWarning = TRUE;
		}
	}

	// Redraw the text in the queue limit box (in case the limit changed)
	if ( nQueueLimit > 0 )
		m_sQueueLimit.Format( _T("%d"), nQueueLimit );
	else
		m_sQueueLimit = _T("MAX");

	// Apply limits to display
	m_nMaxDownFiles = min ( m_nMaxDownFiles, 100 );
	if ( Settings.GetOutgoingBandwidth() < 16 )
		m_nMaxDownTransfers = min ( m_nMaxDownTransfers, 200 );
	else if ( Settings.GetOutgoingBandwidth() < 32 )
		m_nMaxDownTransfers = min ( m_nMaxDownTransfers, 250 );
	else if ( Settings.GetOutgoingBandwidth() < 64 )
		m_nMaxDownTransfers = min ( m_nMaxDownTransfers, 500 );
	else
		m_nMaxDownTransfers = min ( m_nMaxDownTransfers, 800 );
	m_nMaxFileTransfers = min ( m_nMaxFileTransfers, 100 );

	// Display any data changes
	UpdateData( FALSE );

	// Put new values in the settings.
	Settings.Downloads.CompletePath			= m_sDownloadsPath;
	Settings.Downloads.IncompletePath		= m_sIncompletePath;
	Settings.Downloads.MaxFiles				= m_nMaxDownFiles;
	Settings.Downloads.MaxTransfers			= m_nMaxDownTransfers;
	Settings.Downloads.MaxFileTransfers		= m_nMaxFileTransfers;
	Settings.Downloads.QueueLimit			= nQueueLimit;
	Settings.Bandwidth.Downloads			= (DWORD)Settings.ParseVolume( m_sBandwidthLimit, TRUE ) / 8;
	Settings.Connection.RequireForTransfers	= m_bRequireConnect;
	


	CreateDirectory( m_sDownloadsPath, NULL );
	CreateDirectory( m_sIncompletePath, NULL );
	//CreateDirectory( m_sTorrentPath, NULL );
	
	if ( m_bDownloadsChanged )
	{
		if ( LibraryFolders.GetFolderCount() == 0 )
		{
			LibraryFolders.AddFolder( m_sDownloadsPath );
		}
		else if ( ! LibraryFolders.IsFolderShared( m_sDownloadsPath ) )
		{
			CString strFormat, strMessage;
			
			LoadString( strFormat, IDS_LIBRARY_DOWNLOADS_ADD );
			strMessage.Format( strFormat, (LPCTSTR)m_sDownloadsPath );
			
			if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES )
			{
				CLibraryFolder* pFolder = LibraryFolders.AddFolder( m_sDownloadsPath );
				
				if ( pFolder )
				{
					LoadString( strMessage, IDS_LIBRARY_DOWNLOADS_SHARE );
					
					BOOL bShare = AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES;
					
					CQuickLock oLock( Library.m_pSection );
					if ( LibraryFolders.CheckFolder( pFolder, TRUE ) )
						pFolder->m_bShared = bShare ? TS_TRUE : TS_FALSE;
					Library.Update();
				}
			}
		}
	}
	
	CSettingsPage::OnOK();
}

void CDownloadsSettingsPage::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CSettingsPage::OnShowWindow(bShow, nStatus);
	if ( bShow )
	{
		CString str;

		// Update speed units for the text in the bandwidth limit combo
		if ( Settings.Bandwidth.Downloads )
		{
			m_sBandwidthLimit = Settings.SmartVolume( Settings.Bandwidth.Downloads * 8, FALSE, TRUE );
		}
		else
		{
			m_sBandwidthLimit	= Settings.SmartVolume( 0, FALSE, TRUE );
			int nSpace			= m_sBandwidthLimit.Find( ' ' );
			m_sBandwidthLimit	= _T("MAX") + m_sBandwidthLimit.Mid( nSpace );
		}

		// Update the bandwidth limit combo values

		// Remove any existing strings
		while ( m_wndBandwidthLimit.GetCount() ) m_wndBandwidthLimit.DeleteString( 0 );
		// Add the new ones
		m_wndBandwidthLimit.AddString( Settings.SmartVolume( Settings.Connection.InSpeed / 4, TRUE, TRUE ) );
		m_wndBandwidthLimit.AddString( Settings.SmartVolume( Settings.Connection.InSpeed / 2, TRUE, TRUE ) );
		m_wndBandwidthLimit.AddString( Settings.SmartVolume( (Settings.Connection.InSpeed/2)+(Settings.Connection.InSpeed/4), TRUE, TRUE ) );
		m_wndBandwidthLimit.AddString( Settings.SmartVolume( Settings.Connection.InSpeed, TRUE, TRUE ) );
		m_wndBandwidthLimit.AddString( _T("MAX") );

		// Update the queue limit combo values

		// Remove any existing strings
		while ( m_wndQueueLimit.GetCount() ) m_wndQueueLimit.DeleteString( 0 );
		// Add the new ones
		if ( Settings.eDonkey.EnableToday || Settings.eDonkey.EnableAlways )
		{
			m_wndQueueLimit.AddString( _T("2000") );
			m_wndQueueLimit.AddString( _T("5000") );
			m_wndQueueLimit.AddString( _T("10000") );
			m_wndQueueLimit.AddString( _T("MAX") );
		}
		else
		{
			m_wndQueueLimit.AddString( _T("5") );
			m_wndQueueLimit.AddString( _T("10") );
			m_wndQueueLimit.AddString( _T("20") );
			m_wndQueueLimit.AddString( _T("MAX") );
		}

		UpdateData( FALSE );
	}
}

BOOL CDownloadsSettingsPage::IsNotLimited(LPCTSTR pText)
{
	if ( ( _tcslen( pText ) == 0 ) ||
		 ( _tcsistr( pText, _T("MAX") ) != NULL ) || 
		 ( _tcsistr( pText, _T("NONE") ) != NULL ) )
		return TRUE;
	else
		return FALSE;
}