//
// PageSettingsUploads.cpp
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
#include "Transfers.h"
#include "UploadQueue.h"
#include "UploadQueues.h"
#include "PageSettingsUploads.h"
#include "DlgQueueProperties.h"
#include "CoolInterface.h"
#include "LiveList.h"
#include "Skin.h"

#include "LibraryDictionary.h"

IMPLEMENT_DYNCREATE(CUploadsSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CUploadsSettingsPage, CSettingsPage)
	ON_CBN_SELCHANGE(IDC_AGENT_LIST, OnSelChangeAgentList)
	ON_CBN_EDITCHANGE(IDC_AGENT_LIST, OnEditChangeAgentList)
	ON_BN_CLICKED(IDC_AGENT_ADD, OnAgentAdd)
	ON_BN_CLICKED(IDC_AGENT_REMOVE, OnAgentRemove)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_QUEUES, OnItemChangedQueues)
	ON_BN_CLICKED(IDC_QUEUE_NEW, OnQueueNew)
	ON_BN_CLICKED(IDC_QUEUE_EDIT, OnQueueEdit)
	ON_BN_CLICKED(IDC_QUEUE_DELETE, OnQueueDelete)
	ON_NOTIFY(NM_DBLCLK, IDC_QUEUES, OnDblClkQueues)
	ON_NOTIFY(LVN_DRAGDROP, IDC_QUEUES, OnQueueDrop)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CUploadsSettingsPage property page

CUploadsSettingsPage::CUploadsSettingsPage() : CSettingsPage( CUploadsSettingsPage::IDD )
{
	m_bSharePartials = FALSE;
	m_nMaxPerHost = 0;
	m_bHubUnshare = FALSE;
	m_bSharePreviews = FALSE;
	m_bThrottleMode = FALSE;
}

CUploadsSettingsPage::~CUploadsSettingsPage()
{
}

void CUploadsSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_QUEUE_DELETE, m_wndQueueDelete);
	DDX_Control(pDX, IDC_QUEUE_EDIT, m_wndQueueEdit);
	DDX_Control(pDX, IDC_QUEUES, m_wndQueues);
	DDX_Control(pDX, IDC_AGENT_REMOVE, m_wndAgentRemove);
	DDX_Control(pDX, IDC_AGENT_ADD, m_wndAgentAdd);
	DDX_Control(pDX, IDC_AGENT_LIST, m_wndAgentList);
	DDX_Control(pDX, IDC_MAX_HOST_SPIN, m_wndMaxPerHost);
	DDX_Check(pDX, IDC_SHARE_PARTIALS, m_bSharePartials);
	DDX_Text(pDX, IDC_MAX_HOST, m_nMaxPerHost);
	DDX_Check(pDX, IDC_HUB_UNSHARE, m_bHubUnshare);
	DDX_Check(pDX, IDC_SHARE_PREVIEW, m_bSharePreviews);
	DDX_Text(pDX, IDC_BANDWIDTH, m_sBandwidth);
	DDX_CBIndex(pDX, IDC_THROTTLE_MODE, m_bThrottleMode);
}

/////////////////////////////////////////////////////////////////////////////
// CUploadsSettingsPage message handlers

BOOL CUploadsSettingsPage::OnInitDialog() 
{
	CSettingsPage::OnInitDialog();
	
	m_wndMaxPerHost.SetRange( 1, 64 );
	
	CRect rcList;
	m_wndQueues.GetClientRect( &rcList );
	rcList.right -= GetSystemMetrics( SM_CXVSCROLL );
	
	m_wndQueues.SetImageList( &CoolInterface.m_pImages, LVSIL_SMALL );
	
	m_wndQueues.InsertColumn( 0, _T("Name"), LVCFMT_LEFT, rcList.right - 100 - 70 - 70, -1 );
	m_wndQueues.InsertColumn( 1, _T("Criteria"), LVCFMT_LEFT, 100, 0 );
	m_wndQueues.InsertColumn( 2, _T("Bandwidth"), LVCFMT_CENTER, 70, 1 );
	m_wndQueues.InsertColumn( 3, _T("Transfers"), LVCFMT_CENTER, 70, 2 );
	m_wndQueues.InsertColumn( 4, _T("Order"), LVCFMT_CENTER, 0, 3 );
	Skin.Translate( _T("CUploadQueueList"), m_wndQueues.GetHeaderCtrl() );
	
	m_wndQueues.SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE,
		LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP, LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP );
	m_wndQueues.EnableToolTips();
	
	CLiveList::Sort( &m_wndQueues, 4, FALSE );
	CLiveList::Sort( &m_wndQueues, 4, FALSE );
	
	m_nMaxPerHost		= Settings.Uploads.MaxPerHost;
	m_bSharePartials	= Settings.Uploads.SharePartials;
	m_bSharePreviews	= Settings.Uploads.SharePreviews;
	m_bHubUnshare		= Settings.Uploads.HubUnshare;
	m_bThrottleMode		= Settings.Uploads.ThrottleMode;
	
	for ( CString strList = Settings.Uploads.BlockAgents + '|' ; strList.GetLength() ; )
	{
		CString strType = strList.SpanExcluding( _T("|") );
		strList = strList.Mid( strType.GetLength() + 1 );
		strType.TrimLeft();
		strType.TrimRight();
		if ( strType.GetLength() ) m_wndAgentList.AddString( strType );
	}
	
	UpdateData( FALSE );
	UpdateQueues();
	
	m_wndAgentAdd.EnableWindow( m_wndAgentList.GetWindowTextLength() > 0 );
	m_wndAgentRemove.EnableWindow( m_wndAgentList.GetCurSel() >= 0 );
	m_wndQueueEdit.EnableWindow( m_wndQueues.GetSelectedCount() == 1 );
	m_wndQueueDelete.EnableWindow( m_wndQueues.GetSelectedCount() > 0 );

	m_bQueuesChanged = FALSE;
	
	return TRUE;
}

BOOL CUploadsSettingsPage::OnSetActive() 
{
	UpdateQueues();
	return CSettingsPage::OnSetActive();
}

void CUploadsSettingsPage::UpdateQueues()
{
	UpdateData( TRUE );
	
	DWORD nTotal = Settings.Connection.OutSpeed * 1024 / 8;
	DWORD nLimit = (DWORD)Settings.ParseVolume( m_sBandwidth, TRUE ) / 8;
	
	if ( nLimit == 0 || nLimit > nTotal ) nLimit = nTotal;
	
	CSingleLock pLock( &UploadQueues.m_pSection, TRUE );
	CLiveList pQueues( 5 );
	int nIndex = 1;
	
	for ( POSITION pos = UploadQueues.GetIterator() ; pos ; nIndex++ )
	{
		BOOL bDonkeyOnlyDisabled = FALSE;

		CUploadQueue* pQueue = UploadQueues.GetNext( pos );

		// If this queue is for ed2k only and we have to be connected to upload
		if ( ( ( pQueue->m_nProtocols & ( 1 << PROTOCOL_ED2K ) ) != 0 ) && ( Settings.Connection.RequireForTransfers ) )
		{	// Then the queue is inactive if we haven't got ed2k enabled
			bDonkeyOnlyDisabled = !( Settings.eDonkey.EnableAlways | Settings.eDonkey.EnableToday );
		}

		// If the queue is inactive and we're in basic GUI mode
		if( ( bDonkeyOnlyDisabled ) && (Settings.General.GUIMode == GUI_BASIC) )
			continue;	// Skip drawing this queue

		CLiveItem* pItem = pQueues.Add( pQueue );
		
		if( ( pQueue->m_bEnable ) && ( ! bDonkeyOnlyDisabled ) )
		{
			DWORD nBandwidth = nLimit * pQueue->m_nBandwidthPoints / max( 1, UploadQueues.GetTotalBandwidthPoints( TRUE ) );
			pItem->Set( 2, Settings.SmartVolume( nBandwidth * 8, FALSE, TRUE ) + '+' );
			pItem->Format( 3, _T("%i-%i"), pQueue->m_nMinTransfers, pQueue->m_nMaxTransfers );

			pItem->m_nImage = CoolInterface.ImageForID( ID_VIEW_UPLOADS );
		}
		else
		{
			pItem->Set( 2, _T("- ") );
			pItem->Format( 3, _T("-"));

			pItem->m_nImage = CoolInterface.ImageForID( ID_SYSTEM_CLEAR );
		}
		
		pItem->Set( 0, pQueue->m_sName );
		pItem->Set( 1, pQueue->GetCriteriaString() );
		
		pItem->Format( 4, _T("%i"), nIndex );
		 
	}
	
	pLock.Unlock();
	pQueues.Apply( &m_wndQueues, TRUE );
}

void CUploadsSettingsPage::OnSelChangeAgentList() 
{
	m_wndAgentRemove.EnableWindow( m_wndAgentList.GetCurSel() >= 0 );
}

void CUploadsSettingsPage::OnEditChangeAgentList() 
{
	m_wndAgentAdd.EnableWindow( m_wndAgentList.GetWindowTextLength() > 0 );
}

void CUploadsSettingsPage::OnAgentAdd() 
{
	CString strType;
	m_wndAgentList.GetWindowText( strType );

	CharLower( strType.GetBuffer() );
	strType.ReleaseBuffer();
	strType.TrimLeft(); strType.TrimRight();
	if ( strType.IsEmpty() ) return;

	if ( m_wndAgentList.FindString( -1, strType ) >= 0 ) return;

	m_wndAgentList.AddString( strType );
	m_wndAgentList.SetWindowText( _T("") );
}

void CUploadsSettingsPage::OnAgentRemove() 
{
	int nItem = m_wndAgentList.GetCurSel();
	if ( nItem >= 0 ) m_wndAgentList.DeleteString( nItem );
	m_wndAgentRemove.EnableWindow( FALSE );
}

void CUploadsSettingsPage::OnItemChangedQueues(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	m_wndQueueEdit.EnableWindow( m_wndQueues.GetSelectedCount() == 1 );
	m_wndQueueDelete.EnableWindow( m_wndQueues.GetSelectedCount() > 0 );
	*pResult = 0;
}

void CUploadsSettingsPage::OnDblClkQueues(NMHDR* pNMHDR, LRESULT* pResult) 
{
	PostMessage( WM_COMMAND, MAKELONG( IDC_QUEUE_EDIT, BN_CLICKED ) );
	*pResult = 0;
}

void CUploadsSettingsPage::OnQueueNew() 
{
	CString strQueueName;
	LoadString( strQueueName, IDS_UPLOAD_QUEUE_NEW );
	CUploadQueue* pQueue = UploadQueues.Create( strQueueName, TRUE );
	UpdateQueues();
	
	CQueuePropertiesDlg dlg( pQueue, TRUE, this );
	if ( dlg.DoModal() != IDOK ) UploadQueues.Delete( pQueue );
	
	UploadQueues.Save();
	UpdateQueues();
	m_bQueuesChanged = TRUE;
}

void CUploadsSettingsPage::OnQueueEdit() 
{
	int nSelected = m_wndQueues.GetNextItem( -1, LVNI_SELECTED );
	if ( nSelected < 0 ) return;
	
	CUploadQueue* pQueue = (CUploadQueue*)m_wndQueues.GetItemData( nSelected );
	
	CQueuePropertiesDlg dlg( pQueue, FALSE, this );
	dlg.DoModal();
	
	UploadQueues.Save();
	UpdateQueues();
	m_bQueuesChanged = TRUE;
}

void CUploadsSettingsPage::OnQueueDelete() 
{
	for ( int nItem = -1 ; ( nItem = m_wndQueues.GetNextItem( nItem, LVNI_SELECTED ) ) >= 0 ; )
	{
		CUploadQueue* pQueue = (CUploadQueue*)m_wndQueues.GetItemData( nItem );
		UploadQueues.Delete( pQueue );
	}
	
	UploadQueues.Save();
	UpdateQueues();
	m_bQueuesChanged = TRUE;
}

void CUploadsSettingsPage::OnQueueDrop(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	NM_LISTVIEW* pNM = (NM_LISTVIEW*)pNMHDR;
	
	CUploadQueue* pTarget = NULL;
	
	if ( pNM->iItem >= 0 && pNM->iItem < m_wndQueues.GetItemCount() )
	{
		pTarget = (CUploadQueue*)m_wndQueues.GetItemData( pNM->iItem );
		if ( ! UploadQueues.Check( pTarget ) ) pTarget = NULL;
	}
	
	for ( int nItem = -1 ; ( nItem = m_wndQueues.GetNextItem( nItem, LVNI_SELECTED ) ) >= 0 ; )
	{
		CUploadQueue* pQueue = (CUploadQueue*)m_wndQueues.GetItemData( nItem );
		
		if ( UploadQueues.Check( pQueue ) && pQueue != pTarget )
		{
			UploadQueues.Reorder( pQueue, pTarget );
		}
	}
	
	UploadQueues.Save();
	UpdateQueues();	
	*pResult = 0;
}

BOOL CUploadsSettingsPage::OnKillActive()
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

void CUploadsSettingsPage::OnOK()
{
	UpdateData();
	
	Settings.Uploads.MaxPerHost			= m_nMaxPerHost;
	Settings.Uploads.SharePartials		= m_bSharePartials;
	Settings.Uploads.SharePreviews		= m_bSharePreviews;
	Settings.Uploads.HubUnshare			= m_bHubUnshare;
	Settings.Bandwidth.Uploads			= (DWORD)Settings.ParseVolume( m_sBandwidth, TRUE ) / 8;
	Settings.Uploads.ThrottleMode		= m_bThrottleMode;
	
	Settings.Uploads.BlockAgents.Empty();
	
	for ( int nItem = 0 ; nItem < m_wndAgentList.GetCount() ; nItem++ )
	{
		CString str;
		m_wndAgentList.GetLBText( nItem, str );
		
		if ( str.GetLength() )
		{
			if ( Settings.Uploads.BlockAgents.IsEmpty() )
				Settings.Uploads.BlockAgents += '|';
			Settings.Uploads.BlockAgents += str;
			Settings.Uploads.BlockAgents += '|';
		}
	}
	
	if ( UploadQueues.GetCount() == 0 )
		UploadQueues.CreateDefault();
	else
		UploadQueues.Validate();

	if ( m_bQueuesChanged )
	{
		// Changing queues might change what files are in the hash table
		LibraryDictionary.RebuildHashTable(); 
		// ED2k file list will automatically update on next server connection
	}
}


void CUploadsSettingsPage::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CSettingsPage::OnShowWindow(bShow, nStatus);
	if ( bShow )
	{
		// Update speed units
		if ( Settings.Bandwidth.Uploads )
		{
			m_sBandwidth = Settings.SmartVolume( Settings.Bandwidth.Uploads * 8, FALSE, TRUE );
		}
		else
		{
			m_sBandwidth	= Settings.SmartVolume( 0, FALSE, TRUE );
			int nSpace		= m_sBandwidth.Find( ' ' );
			m_sBandwidth	= _T("MAX") + m_sBandwidth.Mid( nSpace );
		}
		UpdateData( FALSE );
	}
}
