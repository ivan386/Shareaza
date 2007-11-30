//
// PageSettingsBitTorrent.cpp
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
#include "WndMain.h"
//#include "Library.h"
//#include "LibraryHistory.h"
#include "PageSettingsBitTorrent.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CBitTorrentSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CBitTorrentSettingsPage, CSettingsPage)
	//{{AFX_MSG_MAP(CBitTorrentSettingsPage)
	ON_BN_CLICKED(IDC_TORRENT_AUTOCLEAR, OnTorrentsAutoClear)
	ON_BN_CLICKED(IDC_TORRENTS_BROWSE, OnTorrentsBrowse)
	ON_BN_CLICKED(IDC_TORRENTS_TORRENTMAKERBROWSE, OnMakerBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBitTorrentSettingsPage property page

CBitTorrentSettingsPage::CBitTorrentSettingsPage() : CSettingsPage(CBitTorrentSettingsPage::IDD)
{
	//{{AFX_DATA_INIT(CBitTorrentSettingsPage)
	m_bTorrentInterface	= FALSE;
	m_bEndGame			= FALSE;
	m_nLinks			= 0;
	m_nDownloads		= 0;
	m_bAutoClear		= FALSE;
	m_nClearPercentage	= 0;
	m_bPrefBTSources	= TRUE;
	m_sTracker			= _T("");
	m_sTorrentPath		= _T("");
	m_sMakerPath		= _T("");
	//}}AFX_DATA_INIT
}

CBitTorrentSettingsPage::~CBitTorrentSettingsPage()
{
}

void CBitTorrentSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBitTorrentSettingsPage)
	DDX_Check(pDX, IDC_TORRENT_INTERFACE, m_bTorrentInterface);
	DDX_Check(pDX, IDC_TORRENT_ENDGAME, m_bEndGame);
	DDX_Text(pDX, IDC_TORRENT_CLIENTLINKS, m_nLinks);
	DDX_Control(pDX, IDC_TORRENT_LINKS_SPIN, m_wndLinksSpin);
	DDX_Text(pDX, IDC_TORRENT_DOWNLOADS, m_nDownloads);
	DDX_Control(pDX, IDC_TORRENT_DOWNLOADS_SPIN, m_wndDownloadsSpin);
	DDX_Check(pDX, IDC_TORRENT_AUTOCLEAR, m_bAutoClear);
	DDX_Control(pDX, IDC_TORRENT_CLEAR_PERCENTAGE, m_wndClearPercentage);
	DDX_Control(pDX, IDC_TORRENT_CLEAR_SPIN, m_wndClearPercentageSpin);
	DDX_Text(pDX, IDC_TORRENT_CLEAR_PERCENTAGE, m_nClearPercentage);
	DDX_Check(pDX, IDC_TORRENT_PREFERENCE, m_bPrefBTSources);
	DDX_Text(pDX, IDC_TORRENT_DEFAULTTRACKER, m_sTracker);
	DDX_Control(pDX, IDC_TORRENTS_BROWSE, m_wndTorrentPath);
	DDX_Text(pDX, IDC_TORRENTS_FOLDER, m_sTorrentPath);
	DDX_Control(pDX, IDC_TORRENTS_TORRENTMAKERBROWSE, m_wndMakerPath);
	DDX_Text(pDX, IDC_TORRENTS_TORRENTMAKER, m_sMakerPath);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CBitTorrentSettingsPage message handlers

BOOL CBitTorrentSettingsPage::OnInitDialog() 
{
	CSettingsPage::OnInitDialog();
	m_bTorrentInterface = Settings.BitTorrent.AdvancedInterface;
	m_bEndGame			= Settings.BitTorrent.Endgame;
	m_nLinks			= Settings.BitTorrent.DownloadConnections;
	m_sTracker			= Settings.BitTorrent.DefaultTracker;
	m_sTorrentPath		= Settings.Downloads.TorrentPath;
	m_nDownloads		= Settings.BitTorrent.DownloadTorrents;
	m_sMakerPath		= Settings.BitTorrent.TorrentCreatorPath;
	m_bAutoClear		= Settings.BitTorrent.AutoClear;
	m_nClearPercentage	= Settings.BitTorrent.ClearRatio;
	m_bPrefBTSources	= Settings.BitTorrent.PreferenceBTSources;

	m_wndTorrentPath.SetIcon( IDI_BROWSE );
	m_wndMakerPath.SetIcon( IDI_BROWSE );

	m_wndClearPercentage.EnableWindow( m_bAutoClear );

	DWORD nMaxTorrents = ( Settings.GetOutgoingBandwidth() / 2 ) + 2;
	nMaxTorrents = min ( 10u, nMaxTorrents);

	m_wndClearPercentageSpin.SetRange( 100, 999 );

	m_wndLinksSpin.SetRange( 0, 200 );
	m_wndDownloadsSpin.SetRange( 0, (WORD)nMaxTorrents );
	UpdateData( FALSE );

	return TRUE;
}

BOOL CBitTorrentSettingsPage::OnSetActive() 
{
	DWORD nMaxTorrents = ( Settings.GetOutgoingBandwidth() / 2 ) + 2;
	nMaxTorrents = min( 10u, nMaxTorrents );

	m_nDownloads	= min( m_nDownloads, (int)nMaxTorrents );
	m_wndDownloadsSpin.SetRange( 0, (WORD)nMaxTorrents );

	UpdateData( FALSE );

	return CSettingsPage::OnSetActive();
}

void CBitTorrentSettingsPage::OnTorrentsAutoClear() 
{
	UpdateData();
	m_wndClearPercentage.EnableWindow( m_bAutoClear );
	m_wndClearPercentageSpin.EnableWindow( m_bAutoClear );
}

void CBitTorrentSettingsPage::OnTorrentsBrowse() 
{
	TCHAR szPath[MAX_PATH];
	LPITEMIDLIST pPath;
	CComPtr< IMalloc > pMalloc;
		
	BROWSEINFO pBI = {};
	pBI.hwndOwner		= AfxGetMainWnd()->GetSafeHwnd();
	pBI.pszDisplayName	= szPath;
	pBI.lpszTitle		= _T("Select folder for torrents:");
	pBI.ulFlags			= BIF_RETURNONLYFSDIRS;
	
	pPath = SHBrowseForFolder( &pBI );

	if ( pPath == NULL ) return;

	SHGetPathFromIDList( pPath, szPath );
	if ( SUCCEEDED( SHGetMalloc( &pMalloc ) ) )
		pMalloc->Free( pPath );
	
	UpdateData( TRUE );
	m_sTorrentPath = szPath;
	UpdateData( FALSE );
}

void CBitTorrentSettingsPage::OnMakerBrowse() 
{
	CFileDialog dlg( TRUE, _T("exe"), _T("TorrentWizard.exe") , OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
		_T("Executable Files|*.exe;*.com|All Files|*.*||"), this );
	
	if ( dlg.DoModal() != IDOK ) return;
	
	UpdateData( TRUE );
	m_sMakerPath = dlg.GetPathName();
	UpdateData( FALSE );
}

void CBitTorrentSettingsPage::OnOK() 
{
	BOOL bRedraw = FALSE;
	UpdateData( TRUE );

	m_nClearPercentage = min (m_nClearPercentage, 999);
	m_nClearPercentage = max (m_nClearPercentage, 100);

	if ( ! theApp.m_bNT )
	{
		// Win9x is unable to handle high numbers of connections
		m_nLinks = min ( m_nLinks, 80 );
	}
	else
	{
		// For other systems we can guestimate a good value based on available bandwidth
	if ( Settings.GetOutgoingBandwidth() < 16 )
		m_nLinks = min ( m_nLinks, 200 );
	else if ( Settings.GetOutgoingBandwidth() < 32 )
		m_nLinks = min ( m_nLinks, 300 );
	else if ( Settings.GetOutgoingBandwidth() < 64 )
		m_nLinks = min ( m_nLinks, 500 );
	else
		m_nLinks = min ( m_nLinks, 800 );
	}

	m_nDownloads = min( m_nDownloads, (int)( ( Settings.GetOutgoingBandwidth() / 2 ) + 2 ) );

	UpdateData( FALSE );

	if ( Settings.BitTorrent.AdvancedInterface != m_bTorrentInterface ) bRedraw = TRUE;

	Settings.BitTorrent.AdvancedInterface	= m_bTorrentInterface;
	Settings.BitTorrent.Endgame				= m_bEndGame;
	Settings.BitTorrent.DownloadConnections	= m_nLinks;
	Settings.BitTorrent.DownloadTorrents	= m_nDownloads;
	Settings.BitTorrent.AutoClear			= m_bAutoClear;
	Settings.BitTorrent.ClearRatio			= m_nClearPercentage;
	Settings.BitTorrent.PreferenceBTSources	= m_bPrefBTSources;
	Settings.BitTorrent.DefaultTracker		= m_sTracker;
	Settings.Downloads.TorrentPath			= m_sTorrentPath;
	Settings.BitTorrent.TorrentCreatorPath	= m_sMakerPath;

	/*
	// Redraw the GUI to make torrents box show/hide if we need to
	if ( bRedraw ) 
	{
		CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd();
		pMainWnd->SetGUIMode( Settings.General.GUIMode, FALSE );
	}
	*/

	CSettingsPage::OnOK();
}