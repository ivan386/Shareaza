//
// DlgTorrentSeed.cpp
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
#include "Library.h"
#include "SharedFile.h"
#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"
#include "ShareazaURL.h"
#include "HttpRequest.h"
#include "DlgTorrentSeed.h"
#include "WndMain.h"
#include "WndDownloads.h"
#include "DlgExistingFile.h"
#include "DlgHelp.h"
#include "DownloadTask.h"
#include "FragmentedFile.h"
#include "LibraryHistory.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CTorrentSeedDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CTorrentSeedDlg, CSkinDialog)
	ON_BN_CLICKED(IDC_DOWNLOAD, OnDownload)
	ON_BN_CLICKED(IDC_SEED, OnSeed)
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()

const DWORD BUFFER_SIZE = 2 * 1024 * 1024u;


/////////////////////////////////////////////////////////////////////////////
// CTorrentSeedDlg construction

CTorrentSeedDlg::CTorrentSeedDlg(LPCTSTR pszTorrent, BOOL bForceSeed, CWnd* pParent) :
	CSkinDialog( CTorrentSeedDlg::IDD, pParent )
,	m_sTorrent( pszTorrent )
,	m_bForceSeed( bForceSeed )
{
}

void CTorrentSeedDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_DOWNLOAD, m_wndDownload );
	DDX_Control( pDX, IDC_SEED, m_wndSeed );
	DDX_Control( pDX, IDC_PROGRESS, m_wndProgress );
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentSeedDlg message handlers

BOOL CTorrentSeedDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( NULL, IDR_MAINFRAME );

	if ( Settings.General.LanguageRTL ) m_wndProgress.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
	m_wndProgress.SetRange( 0, 1000 );
	m_wndProgress.SetPos( 0 );

	if ( m_pInfo.LoadTorrentFile( m_sTorrent ) )
	{
		CSingleLock pTransfersLock( &Transfers.m_pSection, TRUE );
		if ( Downloads.FindByBTH( m_pInfo.m_oBTH ) )
		{
			pTransfersLock.Unlock();

			// We are already seeding the torrent
			CString strMessage;
			strMessage.Format( LoadString( IDS_BT_SEED_ALREADY ), (LPCTSTR)m_pInfo.m_sName );
			theApp.Message( MSG_ERROR, LoadString( IDS_BT_SEED_ALREADY ), (LPCTSTR)m_pInfo.m_sName );
			AfxMessageBox( strMessage, MB_ICONEXCLAMATION );

			EndDialog( IDOK );
		}
		else
		{
			pTransfersLock.Unlock();

			if ( m_pInfo.HasEncodingError() )		// Check the torrent is valid
			{
				CHelpDlg::Show( _T("GeneralHelp.BadTorrentEncoding") );
			}

			if ( m_bForceSeed )
			{
				m_wndDownload.EnableWindow( FALSE );
				if ( Settings.BitTorrent.AutoSeed )
					PostMessage( WM_TIMER, 4 );
			}
			else
			{
				// Check for already downloaded file (first one only)
				CShareazaURL oURL( new CBTInfo ( m_pInfo ) );
				CExistingFileDlg::Action action = CExistingFileDlg::CheckExisting( &oURL, FALSE );
				if ( action == CExistingFileDlg::Download )
				{
					PostMessage( WM_TIMER, 5 );
				}
			}
		}
	}
	else
	{
		// We couldn't load the .torrent file
		CString strMessage;
		strMessage.Format( LoadString( IDS_BT_SEED_PARSE_ERROR ), (LPCTSTR)m_sTorrent );
		theApp.Message( MSG_ERROR, LoadString( IDS_BT_SEED_PARSE_ERROR ), (LPCTSTR)m_sTorrent );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );

		EndDialog( IDOK );
	}

	return TRUE;
}

void CTorrentSeedDlg::OnDownload()
{
	CShareazaURL oURL( new CBTInfo ( m_pInfo ) );

	CSingleLock pTransfersLock( &Transfers.m_pSection, TRUE );

	{
		CSingleLock oLibraryLock( &Library.m_pSection, TRUE );
	
		CExistingFileDlg::Action action = CExistingFileDlg::CheckExisting( &oURL );
		if ( action == CExistingFileDlg::Cancel )
			return;
		else if ( action == CExistingFileDlg::Download )
		{
			if ( CDownload* pDownload = Downloads.Add( oURL ) )
			{
				pDownload->OpenDownload();

				// Automatically merge download with local files on start-up
				if ( Settings.BitTorrent.AutoMerge )
				{
					CList< CString > oFiles;
					for ( POSITION pos = pDownload->m_pTorrent.m_pFiles.GetHeadPosition() ; pos ; )
					{
						CBTInfo::CBTFile* pBTFile = pDownload->m_pTorrent.m_pFiles.GetNext( pos );

						pBTFile->FindFile();

						const CString& strFile = pBTFile->GetBestPath();
						if ( ! strFile.IsEmpty() )
							oFiles.AddTail( strFile );
					}

					if ( oFiles.GetCount() )
						pDownload->MergeFile( &oFiles );
				}

				if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 && ! Network.IsWellConnected() )
					Network.Connect( TRUE );

				if ( CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd() )
					pMainWnd->m_pWindows.Open( RUNTIME_CLASS(CDownloadsWnd) );

				if ( Settings.Downloads.ShowMonitorURLs )
					pDownload->ShowMonitor();
			}
		}
	}

	EndDialog( IDOK );
}

void CTorrentSeedDlg::OnSeed()
{
	m_wndDownload.EnableWindow( FALSE );
	m_wndSeed.EnableWindow( FALSE );
	m_bCancel = FALSE;

	// Connect if (we aren't)
	if ( ! Network.IsConnected() ) Network.Connect();

	// Update the last seeded torrent
	CSingleLock pLock( &Library.m_pSection );
	if ( pLock.Lock( 250 ) )
	{
		LibraryHistory.LastSeededTorrent.m_sName		= m_pInfo.m_sName.Left( 40 );
		LibraryHistory.LastSeededTorrent.m_sPath = m_sTorrent;
		LibraryHistory.LastSeededTorrent.m_tLastSeeded = static_cast< DWORD >( time( NULL ) );

		// If it's a 'new' torrent, reset the counters
		if ( !validAndEqual( LibraryHistory.LastSeededTorrent.m_oBTH, m_pInfo.m_oBTH ) )
		{
			LibraryHistory.LastSeededTorrent.m_nUploaded	= 0;
			LibraryHistory.LastSeededTorrent.m_nDownloaded	= 0;
			LibraryHistory.LastSeededTorrent.m_oBTH 		= m_pInfo.m_oBTH;
		}
	}

	// Start the torrent seed process
	BeginThread( "DlgTorrentSeed" );
}

void CTorrentSeedDlg::OnCancel()
{
	if ( m_wndDownload.IsWindowEnabled() || m_bCancel )
	{
		CSkinDialog::OnCancel();
	}
	else
	{
		m_bCancel = TRUE;
	}
}

void CTorrentSeedDlg::OnDestroy()
{
	m_bCancel = TRUE;
	CloseThread();

	CSkinDialog::OnDestroy();
}

void CTorrentSeedDlg::OnTimer(UINT_PTR nIDEvent)
{
	CSkinDialog::OnTimer( nIDEvent );

	if ( nIDEvent == 1 )
	{
		EndDialog( IDOK );
	}
	else if ( nIDEvent == 2 )
	{
		if ( m_bCancel == FALSE ) AfxMessageBox( m_sMessage, MB_ICONEXCLAMATION );
		EndDialog( IDCANCEL );
	}
	else if ( nIDEvent == 3 )
	{
		if ( m_nScaled != m_nOldScaled )
		{
			m_nOldScaled = m_nScaled;
			m_wndProgress.SetPos( m_nScaled );
		}
	}
	else if ( nIDEvent == 4 )
	{
		OnSeed();
	}
	else if ( nIDEvent == 5 )
	{
		OnDownload();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentSeedDlg thread run

void CTorrentSeedDlg::OnRun()
{
	if ( CreateDownload() )
	{
		PostMessage( WM_TIMER, 1 );
	}
	else
	{
		PostMessage( WM_TIMER, 2 );
	}
}

BOOL CTorrentSeedDlg::CreateDownload()
{
	CSingleLock pTransfersLock( &Transfers.m_pSection );
	if ( pTransfersLock.Lock( 2000 ) )
	{
		if ( Downloads.FindByBTH( m_pInfo.m_oBTH ) )
		{
			// Already seeding
			m_sMessage.Format( LoadString( IDS_BT_SEED_ALREADY ), (LPCTSTR)m_pInfo.m_sName );
		}
		else
		{
			if ( CDownload* pDownload = Downloads.Add( CShareazaURL( new CBTInfo( m_pInfo ) ) ) )
			{
				if ( pDownload->SeedTorrent() )
					return TRUE;
				m_sMessage = pDownload->GetFileErrorString() + _T(" ") + GetErrorString( pDownload->GetFileError() );
				pDownload->Remove();
			}
		}
	}

	if ( m_sMessage.IsEmpty() )
	{
		m_sMessage.Format( LoadString( IDS_BT_SEED_ERROR ), (LPCTSTR)m_pInfo.m_sName );
	}

	return FALSE;
}
