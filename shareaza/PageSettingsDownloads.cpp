//
// PageSettingsDownloads.cpp
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
#include "AntiVirus.h"
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
	ON_BN_CLICKED(IDC_DOWNLOADS_BROWSE, &CDownloadsSettingsPage::OnDownloadsBrowse)
	ON_BN_CLICKED(IDC_INCOMPLETE_BROWSE, &CDownloadsSettingsPage::OnIncompleteBrowse)
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
	ON_CBN_DROPDOWN(IDC_ANTIVIRUS, &CDownloadsSettingsPage::OnCbnDropdownAntivirus)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDownloadsSettingsPage property page

CDownloadsSettingsPage::CDownloadsSettingsPage()
	: CSettingsPage			( CDownloadsSettingsPage::IDD )
	, m_nMaxDownFiles		( 0 )
	, m_nMaxFileTransfers	( 0 )
	, m_nMaxDownTransfers	( 0 )
	, m_bRequireConnect		( FALSE )
{
}

void CDownloadsSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MAX_TRANSFERS_SPIN, m_wndMaxDownTransfers);
	DDX_Control(pDX, IDC_MAX_TPF_SPIN, m_wndMaxFileTransfers);
	DDX_Control(pDX, IDC_MAX_FILES_SPIN, m_wndMaxDownFiles);
	DDX_Control(pDX, IDC_INCOMPLETE_BROWSE, m_wndIncompletePath);
	DDX_Control(pDX, IDC_DOWNLOADS_BROWSE, m_wndDownloadsPath);
	DDX_Control(pDX, IDC_ANTIVIRUS, m_wndAntiVirus);
	DDX_Control(pDX, IDC_DOWNLOADS_BANDWIDTH_LIMIT, m_wndBandwidthLimit);
	DDX_Control(pDX, IDC_DOWNLOADS_QUEUE_LIMIT, m_wndQueueLimit);
	DDX_Control(pDX, IDC_DOWNLOADS_FOLDER, m_wndDownloadsFolder);
	DDX_Control(pDX, IDC_INCOMPLETE_FOLDER, m_wndIncompleteFolder);
	DDX_Text(pDX, IDC_MAX_FILES, m_nMaxDownFiles);
	DDX_Text(pDX, IDC_MAX_TPF, m_nMaxFileTransfers);
	DDX_Text(pDX, IDC_MAX_TRANSFERS, m_nMaxDownTransfers);
	DDX_CBString(pDX, IDC_DOWNLOADS_BANDWIDTH_LIMIT, m_sBandwidthLimit);
	DDX_CBString(pDX, IDC_DOWNLOADS_QUEUE_LIMIT, m_sQueueLimit);
	DDX_Check(pDX, IDC_REQUIRE_CONNECT, m_bRequireConnect);
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadsSettingsPage message handlers

BOOL CDownloadsSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	AddAndSelect( m_wndDownloadsFolder, theApp.GetDownloadsFolder() );
	{
		CQuickLock oLock( Library.m_pSection );
		for ( POSITION pos = LibraryFolders.GetFolderIterator(); pos; )
		{
			const CLibraryFolder* pFolder = LibraryFolders.GetNextFolder( pos );
			AddAndSelect( m_wndDownloadsFolder, pFolder->m_sPath );
		}
	}
	AddAndSelect( m_wndDownloadsFolder, Settings.Downloads.CompletePath );

	AddAndSelect( m_wndIncompleteFolder, theApp.GetLocalAppDataFolder() + _T("\\") CLIENT_NAME_T _T("\\Incomplete") );
	AddAndSelect( m_wndIncompleteFolder, Settings.Downloads.IncompletePath );

	m_nMaxDownFiles			= Settings.Downloads.MaxFiles;
	m_nMaxDownTransfers		= Settings.Downloads.MaxTransfers;
	m_nMaxFileTransfers		= Settings.Downloads.MaxFileTransfers;
	m_bRequireConnect		= Settings.Connection.RequireForTransfers;

	Settings.SetRange( &Settings.Downloads.MaxFiles, m_wndMaxDownFiles );
	Settings.SetRange( &Settings.Downloads.MaxTransfers, m_wndMaxDownTransfers );
	Settings.SetRange( &Settings.Downloads.MaxFileTransfers, m_wndMaxFileTransfers );

	// Enum available anti-viruses
	AntiVirus.Enum( m_wndAntiVirus );

	m_wndDownloadsPath.SetIcon( IDI_BROWSE );
	m_wndIncompletePath.SetIcon( IDI_BROWSE );

	if ( Settings.Downloads.QueueLimit )
		m_sQueueLimit.Format( _T("%u"), Settings.Downloads.QueueLimit );
	else
		m_sQueueLimit = _T("MAX");

	// Update the text in the bandwidth limit combo
	if ( Settings.Bandwidth.Downloads )
		m_sBandwidthLimit = Settings.SmartSpeed( Settings.Bandwidth.Downloads );
	else
		m_sBandwidthLimit	= _T("MAX");

	UpdateData( FALSE );

	return TRUE;
}

void CDownloadsSettingsPage::OnDownloadsBrowse()
{
	CString sDownloadsPath;
	m_wndDownloadsFolder.GetWindowText( sDownloadsPath );
	sDownloadsPath = BrowseForFolder( IDS_SELECT_FOLDER_DOWNLOAD, sDownloadsPath );
	if ( sDownloadsPath.IsEmpty() )
		return;

	// Warn user about a path that's too long
	if ( _tcslen( sDownloadsPath ) > MAX_PATH - 33 )
	{
		AfxMessageBox( IDS_SETTINGS_FILEPATH_TOO_LONG, MB_ICONEXCLAMATION );
		return;
	}

	// Make sure download/incomplete folders aren't the same
	CString sIncompletePath;
	m_wndIncompleteFolder.GetWindowText( sIncompletePath );
	if ( _tcsicmp( sDownloadsPath, sIncompletePath ) == 0 )
	{
		AfxMessageBox( IDS_SETTINGS_FILEPATH_NOT_SAME, MB_ICONEXCLAMATION );
		return;
	}

	AddAndSelect( m_wndDownloadsFolder, sDownloadsPath );
}

void CDownloadsSettingsPage::OnIncompleteBrowse()
{
	CString sIncompletePath;
	m_wndIncompleteFolder.GetWindowText( sIncompletePath );
	sIncompletePath = BrowseForFolder( IDS_SELECT_FOLDER_INCOMPLETE, sIncompletePath );
	if ( sIncompletePath.IsEmpty() )
		return;

	// Warn user about a path that's too long
	if ( _tcslen( sIncompletePath ) > MAX_PATH - 60 )
	{
		AfxMessageBox( IDS_SETTINGS_FILEPATH_TOO_LONG, MB_ICONEXCLAMATION );
		return;
	}

	// Make sure download/incomplete folders aren't the same
	CString sDownloadsPath;
	m_wndDownloadsFolder.GetWindowText( sDownloadsPath );
	if ( _tcsicmp( sIncompletePath, sDownloadsPath ) == 0 )
	{
		AfxMessageBox( IDS_SETTINGS_FILEPATH_NOT_SAME, MB_ICONEXCLAMATION );
		return;
	}

	// Warn user about an incomplete folder in the library
	if ( LibraryFolders.IsFolderShared( sIncompletePath ) )
	{
		AfxMessageBox( IDS_SETTINGS_INCOMPLETE_LIBRARY, MB_ICONEXCLAMATION );
		return;
	}

	AddAndSelect( m_wndIncompleteFolder, sIncompletePath );
}

BOOL CDownloadsSettingsPage::OnKillActive()
{
	UpdateData();

	if ( IsLimited( m_sBandwidthLimit ) && !Settings.ParseVolume( m_sBandwidthLimit ) )
	{
		AfxMessageBox( IDS_SETTINGS_NEED_BANDWIDTH, MB_ICONEXCLAMATION );
		GetDlgItem( IDC_DOWNLOADS_BANDWIDTH_LIMIT )->SetFocus();
		return FALSE;
	}

	return CSettingsPage::OnKillActive();
}

void CDownloadsSettingsPage::OnOK()
{
	UpdateData( TRUE );

	AntiVirus.UpdateData( m_wndAntiVirus );

	// Figure out what the text in the queue limit box means
	DWORD nQueueLimit = 0;
	if ( IsLimited( m_sQueueLimit ) )
	{
		// Max queue is limited, calculate number
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
	else
	{
		// Max queue is not limited
		nQueueLimit = 0;
	}

	// Check the queue limit value is okay
	if ( ( nQueueLimit > 0 ) && ( nQueueLimit < 2000 ) && ( ! Settings.Live.QueueLimitWarning ) &&
		 ( Settings.eDonkey.EnableToday || Settings.eDonkey.EnableAlways ) && ( Settings.Downloads.QueueLimit != (int)nQueueLimit ) )
	{
		// Warn the user about setting the max queue wait limit too low
		if ( AfxMessageBox( IDS_SETTINGS_WARN_QUEUELIMIT, MB_ICONQUESTION|MB_YESNO ) == IDNO )
		{
			nQueueLimit = 0;
		}
		else
		{
			// Don't need to warn the user again.
			Settings.Live.QueueLimitWarning = TRUE;
		}
	}

	CString sDownloadsPath;
	m_wndDownloadsFolder.GetWindowText( sDownloadsPath );
	const bool bDownloadsChanged = ( sDownloadsPath.CompareNoCase( Settings.Downloads.CompletePath ) != 0 );

	CString sIncompletePath;
	m_wndIncompleteFolder.GetWindowText( sIncompletePath );
	//const bool bIncompleteChanged = ( sIncompletePath.CompareNoCase( Settings.Downloads.IncompletePath ) != 0 );

	// Put new values in the settings.
	Settings.Downloads.CompletePath			= sDownloadsPath;
	Settings.Downloads.IncompletePath		= sIncompletePath;
	Settings.Downloads.MaxFiles				= m_nMaxDownFiles;
	Settings.Downloads.MaxTransfers			= m_nMaxDownTransfers;
	Settings.Downloads.MaxFileTransfers		= m_nMaxFileTransfers;
	Settings.Downloads.QueueLimit			= nQueueLimit;
	Settings.Bandwidth.Downloads			= static_cast< DWORD >( Settings.ParseVolume( m_sBandwidthLimit ) );
	Settings.Connection.RequireForTransfers	= m_bRequireConnect != FALSE;

	// Normalize data
	Settings.Normalize( &Settings.Downloads.MaxFiles );
	m_nMaxDownFiles		= Settings.Downloads.MaxFiles;
	Settings.Normalize( &Settings.Downloads.MaxTransfers );
	m_nMaxDownTransfers	= Settings.Downloads.MaxTransfers;
	Settings.Normalize( &Settings.Downloads.MaxFileTransfers );
	m_nMaxFileTransfers	= Settings.Downloads.MaxFileTransfers;
	Settings.Normalize( &Settings.Downloads.QueueLimit );

	// Redraw the text in the queue limit box (in case the limit changed)
	if ( Settings.Downloads.QueueLimit > 0 )
		m_sQueueLimit.Format( _T("%u"), Settings.Downloads.QueueLimit );
	else
		m_sQueueLimit = _T("MAX");

	// Display any data changes
	UpdateData( FALSE );

	CreateDirectory( sDownloadsPath );
	CreateDirectory( sIncompletePath );

	if ( bDownloadsChanged )
	{
		if ( LibraryFolders.GetFolderCount() == 0 )
		{
			LibraryFolders.AddFolder( sDownloadsPath );
		}
		else if ( ! LibraryFolders.IsFolderShared( sDownloadsPath ) )
		{
			CString strMessage;
			strMessage.Format( LoadString( IDS_LIBRARY_DOWNLOADS_ADD ), (LPCTSTR)sDownloadsPath );
			if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES )
			{
				if ( CLibraryFolder* pFolder = LibraryFolders.AddFolder( sDownloadsPath ) )
				{
					const BOOL bShare = AfxMessageBox( IDS_LIBRARY_DOWNLOADS_SHARE, MB_ICONQUESTION|MB_YESNO ) == IDYES;

					CQuickLock oLock( Library.m_pSection );
					if ( LibraryFolders.CheckFolder( pFolder, TRUE ) )
						pFolder->SetShared( bShare ? TRI_TRUE : TRI_FALSE );
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
		// Update the bandwidth limit combo values

		// Update speed units
		if ( Settings.Bandwidth.Downloads )
			m_sBandwidthLimit	= Settings.SmartSpeed( Settings.Bandwidth.Downloads );
		else
			m_sBandwidthLimit	= _T("MAX");

		// Remove any existing strings
		m_wndBandwidthLimit.ResetContent();

		// Add the new ones
		const DWORD nSpeeds[] =
		{
			Settings.Connection.InSpeed / 4,			//  25%
			Settings.Connection.InSpeed / 2,			//  50%
			( Settings.Connection.InSpeed * 3 ) / 4,	//  75%
			( Settings.Connection.InSpeed * 17 ) / 20,	//  85%
			Settings.Connection.InSpeed					// 100%
		};
		for ( int nSpeed = 0 ; nSpeed < sizeof( nSpeeds ) / sizeof( DWORD ) ; nSpeed++ )
		{
			CString strSpeed = Settings.SmartSpeed( nSpeeds[ nSpeed ], Kilobits );
			if ( Settings.ParseVolume( strSpeed, Kilobits )
				&& m_wndBandwidthLimit.FindStringExact( -1, strSpeed ) == CB_ERR )
			{
				m_wndBandwidthLimit.AddString( strSpeed );
			}
		}
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

bool CDownloadsSettingsPage::IsLimited(CString& strText) const
{
	return ! ( ( strText.GetLength() == 0 ) ||
		( _tcsistr( strText, _T("MAX") ) != NULL ) ||
		( _tcsistr( strText, _T("NONE") ) != NULL ) );
}

void CDownloadsSettingsPage::OnDestroy()
{
	AntiVirus.Free( m_wndAntiVirus );

	CSettingsPage::OnDestroy();
}

void CDownloadsSettingsPage::OnCbnDropdownAntivirus()
{
	RecalcDropWidth( &m_wndAntiVirus );
}
