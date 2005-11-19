//
// PageTorrentTrackers.cpp
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

#include "ShellIcons.h"
#include "BTInfo.h"
#include "Download.h"
#include "PageTorrentTrackers.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CTorrentTrackersPage, CTorrentInfoPage)

BEGIN_MESSAGE_MAP(CTorrentTrackersPage, CTorrentInfoPage)
	//{{AFX_MSG_MAP(CTorrentTrackersPage)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTorrentTrackersPage property page

CTorrentTrackersPage::CTorrentTrackersPage() : CTorrentInfoPage( CTorrentTrackersPage::IDD )
{
	//{{AFX_DATA_INIT(CTorrentTrackersPage)
	m_sName = _T("");
	m_sTracker = _T("");
	//}}AFX_DATA_INIT
}

CTorrentTrackersPage::~CTorrentTrackersPage()
{
}

void CTorrentTrackersPage::DoDataExchange(CDataExchange* pDX)
{
	CTorrentInfoPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTorrentTrackersPage)
	DDX_Text(pDX, IDC_TORRENT_NAME, m_sName);
	DDX_Text(pDX, IDC_TORRENT_TRACKER, m_sTracker);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentTrackersPage message handlers

BOOL CTorrentTrackersPage::OnInitDialog()
{
	CBTInfo *pInfo = GetTorrentInfo();
	CTorrentInfoPage::OnInitDialog();

	m_sName			= pInfo->m_sName;
	m_sTracker		= pInfo->m_sTracker;
	
	UpdateData( FALSE );

	return TRUE;
}

void CTorrentTrackersPage::OnOK()
{
	UpdateData();
	CBTInfo *pInfo = GetTorrentInfo();

	// Check if tracker has been changed, and the new value could be valid
	if ( ( pInfo->m_sTracker != m_sTracker ) && ( m_sTracker.Find( _T("http") ) == 0 ) )
	{
		CString strMessage;
		LoadString( strMessage, IDS_BT_TRACK_CHANGE );
		
		// Display warning
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES )
		{
			pInfo->m_sTracker = m_sTracker;
			pInfo->m_nTrackerType = tCustom;
			pInfo->m_oInfoBTH.validate();
		}
	}
	
	CTorrentInfoPage::OnOK();
}