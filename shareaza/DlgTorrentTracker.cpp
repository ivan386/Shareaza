//
// DlgTorrentTracker.cpp
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
#include "DlgTorrentTracker.h"
#include "BENode.h"
#include "Skin.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CTorrentTrackerDlg, CSkinDialog)
	//{{AFX_MSG_MAP(CTorrentTrackerDlg)
	ON_CBN_SELCHANGE(IDC_TORRENT_VIEW, OnSelChangeTorrentView)
	ON_BN_CLICKED(IDC_TORRENT_REFRESH, OnTorrentRefresh)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTorrentTrackerDlg dialog

CTorrentTrackerDlg::CTorrentTrackerDlg(CBTInfo* pInfo, int* pStart, CWnd* pParent) : CSkinDialog(CTorrentTrackerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTorrentTrackerDlg)
	m_sName = _T("");
	m_sTracker = _T("");
	//}}AFX_DATA_INIT

	m_pInfo.Copy( pInfo );
	m_pInfo.m_bValid = FALSE;

	m_pStartTorrentDownloads = pStart;
}

void CTorrentTrackerDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTorrentTrackerDlg)
	DDX_Control(pDX, IDC_TORRENT_VIEW, m_wndView);
	DDX_Control(pDX, IDC_TORRENT_REFRESH, m_wndRefresh);
	DDX_Control(pDX, IDC_TORRENT_FILES, m_wndFiles);
	DDX_Control(pDX, IDC_TORRENT_COMPLETED, m_wndComplete);
	DDX_Control(pDX, IDC_TORRENT_INCOMPLETE, m_wndIncomplete);
	DDX_Text(pDX, IDC_TORRENT_NAME, m_sName);
	DDX_Text(pDX, IDC_TORRENT_TRACKER, m_sTracker);
	DDX_Control(pDX, IDC_STARTTORRENTDOWNLOADS, m_wndStartDownloads);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentTrackerDlg message handlers

BOOL CTorrentTrackerDlg::OnInitDialog() 
{
	CSkinDialog::OnInitDialog();
	
	SetIcon( theApp.LoadIcon( IDR_MAINFRAME ), TRUE );
	SkinMe( _T("CTorrentTrackerDlg") );
	
	m_sName		= m_pInfo.m_sName;
	m_sTracker	= m_pInfo.m_sTracker;
	
	CRect rc;
	m_wndFiles.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL );
	m_wndFiles.SetImageList( ShellIcons.GetObject( 16 ), LVSIL_SMALL );
	m_wndFiles.InsertColumn( 0, _T("Filename"), LVCFMT_LEFT, rc.right - 80, -1 );
	m_wndFiles.InsertColumn( 1, _T("Size"), LVCFMT_RIGHT, 80, 0 );
	Skin.Translate( _T("CTorrentTrackerList"), m_wndFiles.GetHeaderCtrl() );

	for ( int nFile = 0 ; nFile < m_pInfo.m_nFiles ; nFile++ )
	{
		CBTInfo::CBTFile* pFile = m_pInfo.m_pFiles + nFile;
		LV_ITEM pItem;
		
		ZeroMemory( &pItem, sizeof(pItem) );
		pItem.mask		= LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		pItem.iItem		= m_wndFiles.GetItemCount();
		pItem.lParam	= (LPARAM)nFile;
		pItem.iImage	= ShellIcons.Get( pFile->m_sPath, 16 );
		pItem.pszText	= (LPTSTR)(LPCTSTR)pFile->m_sPath;
		pItem.iItem		= m_wndFiles.InsertItem( &pItem );
		
		m_wndFiles.SetItemText( pItem.iItem, 1, Settings.SmartVolume( pFile->m_nSize, FALSE ) );
	}

	m_wndStartDownloads.SetItemData( 0, dtAlways );
	m_wndStartDownloads.SetItemData( 1, dtWhenRatio );
	m_wndStartDownloads.SetItemData( 2, dtNever );

	m_wndStartDownloads.SetCurSel( *m_pStartTorrentDownloads );
	
	UpdateData( FALSE );
	m_hThread = NULL;
	
	PostMessage( WM_COMMAND, MAKELONG( IDC_TORRENT_REFRESH, BN_CLICKED ), (LPARAM)m_wndRefresh.GetSafeHwnd() );

	return TRUE;
}

void CTorrentTrackerDlg::OnDestroy() 
{
	if ( m_hThread != NULL ) 
	{
		m_pRequest.Cancel();
		CHttpRequest::CloseThread( &m_hThread, _T("CTorrentTrackerDlg") );
	}
	
	CSkinDialog::OnDestroy();
}

void CTorrentTrackerDlg::OnSelChangeTorrentView() 
{
}

void CTorrentTrackerDlg::OnTorrentRefresh() 
{
	if ( m_wndRefresh.IsWindowEnabled() == FALSE ) return;
	
	if ( m_hThread != NULL ) 
	{
		m_pRequest.Cancel();
		CHttpRequest::CloseThread( &m_hThread, _T("CTorrentTrackerDlg") );
	}
	
	m_wndRefresh.EnableWindow( FALSE );
	m_hThread = AfxBeginThread( ThreadStart, this )->m_hThread;
}

void CTorrentTrackerDlg::OnTimer(UINT nIDEvent) 
{
	if ( nIDEvent == 1 )
	{
		m_wndRefresh.EnableWindow( TRUE );
		KillTimer( 1 );
	}
	else
	{
		CHttpRequest::CloseThread( &m_hThread, _T("CTorrentTrackerDlg") );
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

void CTorrentTrackerDlg::OnOK() 
{
	UpdateData();
	
	//Check if tracker has been changed
	if ( m_pInfo.m_sTracker != m_sTracker )
	{
		CString strMessage;
		LoadString( strMessage, IDS_BT_TRACK_CHANGE );
		
		//Display warning
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES )
		{
			m_pInfo.m_sTracker = m_sTracker;
			m_pInfo.m_bValid = TRUE;
		}
	}

	//Update the starting of torrent transfers
	*m_pStartTorrentDownloads = m_wndStartDownloads.GetCurSel();
	
	CSkinDialog::OnOK();
}

UINT CTorrentTrackerDlg::ThreadStart(LPVOID pParam)
{
	CTorrentTrackerDlg* pObject = (CTorrentTrackerDlg*)pParam;
	pObject->OnRun();
	return 0;
}

void CTorrentTrackerDlg::OnRun()
{
	m_pRequest.Clear();
	m_pRequest.AddHeader( _T("Accept-Encoding"), _T("deflate, gzip") );
	
	CString strURL = m_pInfo.m_sTracker;
	Replace( strURL, _T("/announce"), _T("/scrape") );

	// Skip obviously invalid trackers
	if ( m_pInfo.m_sTracker.GetLength() > 7 ) 
	{
		m_pRequest.SetURL( strURL );
		
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
	
	m_pRequest.Clear();
	PostMessage( WM_TIMER, 2 );
}

BOOL CTorrentTrackerDlg::OnTree(CBENode* pNode)
{
	if ( ! pNode->IsType( CBENode::beDict ) ) return FALSE;
	
	CBENode* pFiles = pNode->GetNode( "files" );
	if ( ! pFiles->IsType( CBENode::beDict ) ) return FALSE;
	
	SHA1* pSHA1 = &m_pInfo.m_pInfoSHA1;
	
	CBENode* pFile = pFiles->GetNode( (LPBYTE)pSHA1, sizeof(SHA1) );
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
