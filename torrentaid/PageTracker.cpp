//
// PageTracker.cpp
//
// Copyright (c) Shareaza Pty. Ltd., 2003.
// This file is part of TorrentAid Torrent Wizard (www.torrentaid.com).
//
// TorrentAid Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// TorrentAid is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TorrentAid; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "StdAfx.h"
#include "TorrentWizard.h"
#include "PageTracker.h"
#include "PageWelcome.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CTrackerPage, CWizardPage)

BEGIN_MESSAGE_MAP(CTrackerPage, CWizardPage)
	//{{AFX_MSG_MAP(CTrackerPage)
	ON_BN_CLICKED(IDC_CLEAR_TRACKERS, OnClearTrackers)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTrackerPage property page

CTrackerPage::CTrackerPage() : CWizardPage(CTrackerPage::IDD)
{
	//{{AFX_DATA_INIT(CTrackerPage)
	m_sTracker = _T("");
	//}}AFX_DATA_INIT
}

CTrackerPage::~CTrackerPage()
{
}

void CTrackerPage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTrackerPage)
	DDX_Control(pDX, IDC_TRACKER, m_wndTracker);
	DDX_CBString(pDX, IDC_TRACKER, m_sTracker);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTrackerPage message handlers

BOOL CTrackerPage::OnInitDialog() 
{
	CWizardPage::OnInitDialog();
	
	int nCount = theApp.GetProfileInt( _T("Trackers"), _T("Count"), 0 );
	
	for ( int nItem = 0 ; nItem < nCount ; nItem++ )
	{
		CString strName, strURL;
		strName.Format( _T("%.3i.URL"), nItem + 1 );
		strURL = theApp.GetProfileString( _T("Trackers"), strName );
		if ( strURL.GetLength() ) m_wndTracker.AddString( strURL );
	}
	
	m_sTracker = theApp.GetProfileString( _T("Trackers"), _T("Last") );
	UpdateData( FALSE );
	
	return TRUE;
}

BOOL CTrackerPage::OnSetActive() 
{
	SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );
	return CWizardPage::OnSetActive();
}

void CTrackerPage::OnClearTrackers() 
{
	theApp.WriteProfileInt( _T("Trackers"), _T("Count"), 0 );
	m_sTracker.Empty();
	UpdateData( FALSE );
	m_wndTracker.ResetContent();
	m_wndTracker.SetFocus();
}

LRESULT CTrackerPage::OnWizardBack() 
{
	GET_PAGE( CWelcomePage, pWelcome );
	return pWelcome->m_nType ? IDD_PACKAGE_PAGE : IDD_SINGLE_PAGE;
}

LRESULT CTrackerPage::OnWizardNext() 
{
	UpdateData( TRUE );
	
	if ( m_sTracker.IsEmpty() || m_sTracker.Find( _T("http") ) != 0 )
	{
		if ( IDYES != AfxMessageBox( IDS_TRACKER_NEED_URL, MB_ICONQUESTION|MB_YESNO ) )
		{
			m_wndTracker.SetFocus();
			return -1;
		}
	}
	
	if ( m_sTracker.GetLength() > 0 && m_wndTracker.FindStringExact( -1, m_sTracker ) < 0 )
	{
		m_wndTracker.AddString( m_sTracker );
		
		CString strName;
		int nCount = theApp.GetProfileInt( _T("Trackers"), _T("Count"), 0 );
		strName.Format( _T("%.3i.URL"), ++nCount );
		theApp.WriteProfileInt( _T("Trackers"), _T("Count"), nCount );
		theApp.WriteProfileString( _T("Trackers"), strName, m_sTracker );
	}
	
	theApp.WriteProfileString( _T("Trackers"), _T("Last"), m_sTracker );
	
	return IDD_COMMENT_PAGE;
}

