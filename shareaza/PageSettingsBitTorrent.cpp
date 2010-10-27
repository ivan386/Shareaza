//
// PageSettingsBitTorrent.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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
#include "WndMain.h"
#include "PageSettingsBitTorrent.h"
#include "SchemaCache.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CBitTorrentSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CBitTorrentSettingsPage, CSettingsPage)
	ON_BN_CLICKED(IDC_TORRENT_AUTOCLEAR, OnTorrentsAutoClear)
	ON_BN_CLICKED(IDC_TORRENTS_BROWSE, OnTorrentsBrowse)
	ON_BN_CLICKED(IDC_TORRENTS_TORRENTMAKERBROWSE, OnMakerBrowse)
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
	nMaxTorrents = min ( 10ul, nMaxTorrents);

	m_wndClearPercentageSpin.SetRange( 100, 999 );

	m_wndLinksSpin.SetRange( 0, 200 );
	m_wndDownloadsSpin.SetRange( 0, (WORD)nMaxTorrents );
	UpdateData( FALSE );

	m_wndTorrentFolder.SubclassDlgItem( IDC_TORRENTS_FOLDER, this );

	return TRUE;
}

BOOL CBitTorrentSettingsPage::OnSetActive()
{
	DWORD nMaxTorrents = ( Settings.GetOutgoingBandwidth() / 2 ) + 2;
	nMaxTorrents = min( 10ul, nMaxTorrents );

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
	CString strPath( BrowseForFolder( _T("Select folder for torrents:"),
		m_sTorrentPath ) );
	if ( strPath.IsEmpty() )
		return;

	UpdateData( TRUE );
	m_sTorrentPath = strPath;
	UpdateData( FALSE );
}

void CBitTorrentSettingsPage::OnMakerBrowse()
{
	CFileDialog dlg( TRUE, _T("exe"), _T("TorrentWizard.exe"), OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
		SchemaCache.GetFilter( CSchema::uriApplicationAll ) +
		SchemaCache.GetFilter( CSchema::uriAllFiles ) +
		_T("|"), this );

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

	// Guestimate a good value based on available bandwidth
	if ( Settings.GetOutgoingBandwidth() < 16 )
		m_nLinks = min ( m_nLinks, 200 );
	else if ( Settings.GetOutgoingBandwidth() < 32 )
		m_nLinks = min ( m_nLinks, 300 );
	else if ( Settings.GetOutgoingBandwidth() < 64 )
		m_nLinks = min ( m_nLinks, 500 );
	else
		m_nLinks = min ( m_nLinks, 800 );

	m_nDownloads = min( m_nDownloads, (int)( ( Settings.GetOutgoingBandwidth() / 2 ) + 2 ) );

	UpdateData( FALSE );

	if ( Settings.BitTorrent.AdvancedInterface != ( m_bTorrentInterface != FALSE ) ) bRedraw = TRUE;

	Settings.BitTorrent.AdvancedInterface	= m_bTorrentInterface != FALSE;
	Settings.BitTorrent.Endgame				= m_bEndGame != FALSE;
	Settings.BitTorrent.DownloadConnections	= m_nLinks;
	Settings.BitTorrent.DownloadTorrents	= m_nDownloads;
	Settings.BitTorrent.AutoClear			= m_bAutoClear != FALSE;
	Settings.BitTorrent.ClearRatio			= m_nClearPercentage;
	Settings.BitTorrent.PreferenceBTSources	= m_bPrefBTSources != FALSE;
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
