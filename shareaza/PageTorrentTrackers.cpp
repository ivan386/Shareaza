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
#include "BENode.h"
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
	ON_BN_CLICKED(IDC_TORRENT_REFRESH, OnTorrentRefresh)
	ON_WM_TIMER()
	ON_WM_DESTROY()
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
	DDX_Control(pDX, IDC_TORRENT_REFRESH, m_wndRefresh);
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
	m_hThread = NULL;

	//PostMessage( WM_COMMAND, MAKELONG( IDC_TORRENT_REFRESH, BN_CLICKED ), (LPARAM)m_wndRefresh.GetSafeHwnd() );

	return TRUE;
}


void CTorrentTrackersPage::OnDestroy() 
{
	if ( m_hThread != NULL ) 
	{
		m_pRequest.Cancel();
		CHttpRequest::CloseThread( &m_hThread, _T("CTorrentTrackersPage") );
	}
	
	CTorrentInfoPage::OnDestroy();
}

void CTorrentTrackersPage::OnTorrentRefresh() 
{
	if ( m_wndRefresh.IsWindowEnabled() == FALSE ) return;
	
	if ( m_hThread != NULL ) 
	{
		m_pRequest.Cancel();
		CHttpRequest::CloseThread( &m_hThread, _T("CTorrentTrackersPage") );
	}
	
	m_wndRefresh.EnableWindow( FALSE );
	m_hThread = AfxBeginThread( ThreadStart, this )->m_hThread;
}

void CTorrentTrackersPage::OnTimer(UINT_PTR nIDEvent) 
{
	if ( nIDEvent == 1 )
	{
		m_wndRefresh.EnableWindow( TRUE );
		KillTimer( 1 );
	}
	else
	{
		CHttpRequest::CloseThread( &m_hThread, _T("CTorrentTrackersPage") );
		SetTimer( 1, 60000, NULL );
		
		if ( nIDEvent == 3 )
		{
			CString str;
			str.Format( _T("%i"), m_nComplete );
			m_wndComplete.SetWindowText( str );
			str.Format( _T("%i"), m_nIncomplete );
			m_wndIncomplete.SetWindowText( str );
		}
	}
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

UINT CTorrentTrackersPage::ThreadStart(LPVOID pParam)
{
	CTorrentTrackersPage* pObject = (CTorrentTrackersPage*)pParam;
	pObject->OnRun();
	return 0;
}

void CTorrentTrackersPage::OnRun()
{
	m_pRequest.Clear();
	m_pRequest.AddHeader( _T("Accept-Encoding"), _T("deflate, gzip") );
	
	CString strURL = m_sTracker;
	Replace( strURL, _T("/announce"), _T("/scrape") );

	if ( ( strURL.Find( _T("http") ) == 0 ) && ( strURL.Find( _T("/scrape") ) != -1 ) )
	{
		// Skip obviously invalid trackers
		if ( strURL.GetLength() > 7 ) 
		{
			m_pRequest.SetURL( strURL );

			theApp.Message( MSG_DEBUG, _T("Sending scrape to %s"), strURL );
			
			if ( m_pRequest.Execute( FALSE ) && m_pRequest.InflateResponse() )
			{
				CBuffer* pResponse = m_pRequest.GetResponseBuffer();
				
				if ( CBENode* pNode = CBENode::Decode( pResponse ) )
				{
					if ( OnTree( pNode ) )
					{
						delete pNode;
						PostMessage( WM_TIMER, 3 );
						return;
					}
					
					delete pNode;
				}
			}
		}
	}
	
	m_pRequest.Clear();
	PostMessage( WM_TIMER, 2 );
}

BOOL CTorrentTrackersPage::OnTree(CBENode* pNode)
{
	CBTInfo *pInfo = GetTorrentInfo();
	if ( ! pNode->IsType( CBENode::beDict ) ) return FALSE;
	
	CBENode* pFiles = pNode->GetNode( "files" );
	if ( ! pFiles->IsType( CBENode::beDict ) ) return FALSE;
	
    CBENode* pFile = pFiles->GetNode( &pInfo->m_oInfoBTH[ 0 ], Hashes::BtHash::byteCount );
	if ( ! pFile->IsType( CBENode::beDict ) ) return FALSE;	
	
	m_nComplete		= 0;
	m_nIncomplete	= 0;
	
	if ( CBENode* pComplete = pFile->GetNode( "complete" ) )
	{
		if ( ! pComplete->IsType( CBENode::beInt ) ) return FALSE;	
		m_nComplete = (int)pComplete->GetInt();
	}
	
	if ( CBENode* pIncomplete = pFile->GetNode( "incomplete" ) )
	{
		if ( ! pIncomplete->IsType( CBENode::beInt ) ) return FALSE;	
		m_nIncomplete = (int)pIncomplete->GetInt();
	}
	
	return TRUE;
}