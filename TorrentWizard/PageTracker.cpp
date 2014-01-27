//
// PageTracker.cpp
//
// Copyright (c) Shareaza Development Team, 2007-2014.
// This file is part of Shareaza Torrent Wizard (shareaza.sourceforge.net).
//
// Shareaza Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Torrent Wizard is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
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
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTrackerPage property page

CTrackerPage::CTrackerPage()
	: CWizardPage(CTrackerPage::IDD, _T("tracker"))
	, m_bPrivate(FALSE)
{
}

void CTrackerPage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_TRACKER, m_wndTracker);
	DDX_CBString(pDX, IDC_TRACKER, m_sTracker);
	DDX_Check(pDX, IDC_PRIVATE, m_bPrivate);
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
	if ( m_sTracker.IsEmpty() )
	{
		m_sTracker = _T("udp://tracker.openbittorrent.com:80");
	}

	m_bPrivate = ( theApp.GetProfileInt( _T("Trackers"), _T("Private"), FALSE ) != FALSE );

	UpdateData( FALSE );

	return TRUE;
}

BOOL CTrackerPage::OnSetActive() 
{
	SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

	if ( ! theApp.m_sCommandLineTracker.IsEmpty() )
	{
		m_sTracker = theApp.m_sCommandLineTracker;
		theApp.m_sCommandLineTracker.Empty();

		UpdateData( FALSE );

		Next();
	}

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

	UpdateData( TRUE );

	SaveTrackers();

	return pWelcome->m_nType ? IDD_PACKAGE_PAGE : IDD_SINGLE_PAGE;
}

LRESULT CTrackerPage::OnWizardNext() 
{
	UpdateData( TRUE );

	if ( ! ( StartsWith( m_sTracker, _PT("http://") ) ||
			 StartsWith( m_sTracker, _PT("udp://") ) ) )
	{
		if ( IDYES != AfxMessageBox( IDS_TRACKER_NEED_URL, MB_ICONQUESTION|MB_YESNO ) )
		{
			m_wndTracker.SetFocus();
			return -1;
		}
	}
	
	SaveTrackers();

	return IDD_COMMENT_PAGE;
}

void CTrackerPage::SaveTrackers()
{
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

	theApp.WriteProfileInt( _T("Trackers"), _T("Private"), m_bPrivate );
}
