//
// PageSettingsDownloads.cpp
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
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDownloadsSettingsPage property page

CDownloadsSettingsPage::CDownloadsSettingsPage() : CSettingsPage(CDownloadsSettingsPage::IDD)
, m_nQueueLimit(0)
{
	//{{AFX_DATA_INIT(CDownloadsSettingsPage)
	m_sDownloadsPath = _T("");
	m_sIncompletePath = _T("");
	m_nMaxDownFiles = 0;
	m_nMaxFileTransfers = 0;
	m_nMaxDownTransfers = 0;
	m_sBandwidth = _T("");
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
	DDX_Text(pDX, IDC_DOWNLOADS_FOLDER, m_sDownloadsPath);
	DDX_Text(pDX, IDC_INCOMPLETE_FOLDER, m_sIncompletePath);
	DDX_Text(pDX, IDC_MAX_FILES, m_nMaxDownFiles);
	DDX_Text(pDX, IDC_MAX_TPF, m_nMaxFileTransfers);
	DDX_Text(pDX, IDC_MAX_TRANSFERS, m_nMaxDownTransfers);
	DDX_Text(pDX, IDC_BANDWIDTH, m_sBandwidth);
	DDX_Check(pDX, IDC_REQUIRE_CONNECT, m_bRequireConnect);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_QUEUE_LIMIT, m_nQueueLimit);
	DDX_Control(pDX, IDC_QUEUE_LIMIT_SPIN, m_wndQueueLimit);
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
	m_nQueueLimit			= Settings.Downloads.QueueLimit;
	m_bRequireConnect		= Settings.Connection.RequireForTransfers;
	
	if ( Settings.Bandwidth.Downloads )
	{
		m_sBandwidth = Settings.SmartVolume( Settings.Bandwidth.Downloads * 8, FALSE, TRUE );
	}
	else
	{
		m_sBandwidth	= Settings.SmartVolume( 0, FALSE, TRUE );
		int nSpace		= m_sBandwidth.Find( ' ' );
		m_sBandwidth	= _T("MAX") + m_sBandwidth.Mid( nSpace );
	}
	
	m_wndMaxDownFiles.SetRange( 1, 128 );
	m_wndMaxDownTransfers.SetRange( 1, 128 );
	m_wndMaxFileTransfers.SetRange( 1, 128 );
	m_wndQueueLimit.SetRange( 0, 10000 );
	
	m_wndDownloadsPath.SetIcon( IDI_BROWSE );
	m_wndIncompletePath.SetIcon( IDI_BROWSE );
	
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
	
	if ( m_sBandwidth.GetLength() > 0 && m_sBandwidth.Find( _T("MAX") ) < 0 &&
		 Settings.ParseVolume( m_sBandwidth, TRUE ) == 0 )
	{
		CString strMessage;
		LoadString( strMessage, IDS_SETTINGS_NEED_BANDWIDTH );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		GetDlgItem( IDC_BANDWIDTH )->SetFocus();
		return FALSE;
	}
	
	return CSettingsPage::OnKillActive();
}

void CDownloadsSettingsPage::OnOK() 
{
	UpdateData();

	Settings.Downloads.CompletePath			= m_sDownloadsPath;
	Settings.Downloads.IncompletePath		= m_sIncompletePath;
	Settings.Downloads.MaxFiles				= m_nMaxDownFiles;
	Settings.Downloads.MaxTransfers			= m_nMaxDownTransfers;
	Settings.Downloads.MaxFileTransfers		= m_nMaxFileTransfers;
	Settings.Downloads.QueueLimit			= m_nQueueLimit;
	Settings.Bandwidth.Downloads			= (DWORD)Settings.ParseVolume( m_sBandwidth, TRUE ) / 8;
	Settings.Connection.RequireForTransfers	= m_bRequireConnect;
	
	CreateDirectory( m_sDownloadsPath, NULL );
	CreateDirectory( m_sIncompletePath, NULL );
	
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
					
					Library.Lock();
					if ( LibraryFolders.CheckFolder( pFolder, TRUE ) )
						pFolder->m_bShared = bShare ? TS_TRUE : TS_FALSE;
					Library.Unlock( TRUE );
				}
			}
		}
	}
	
	CSettingsPage::OnOK();
}

