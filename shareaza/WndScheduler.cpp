//
// WndScheduler.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2016.
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
#include "Skin.h"
#include "Network.h"
#include "LiveList.h"
#include "WndScheduler.h"
#include "CoolInterface.h"
#include "DlgScheduleTask.h"
#include "Scheduler.h"
#include "XML.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum
{
	SCHEDULE_NO_ITEM = 1,
	SCHEDULE_ITEM_ACTIVE,
	SCHEDULE_ITEM_INACTIVE
};

const static UINT nImageID[] =
{
	IDR_SCHEDULERFRAME,
	IDI_NOTASK,
	ID_SCHEDULER_ACTIVATE,
	ID_SCHEDULER_DEACTIVATE,
	NULL
};

IMPLEMENT_SERIAL(CSchedulerWnd, CPanelWnd, 0)

BEGIN_MESSAGE_MAP(CSchedulerWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SCHEDULE, &CSchedulerWnd::OnCustomDrawList)
	ON_NOTIFY(NM_DBLCLK, IDC_SCHEDULE, &CSchedulerWnd::OnDblClkList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_SCHEDULE, &CSchedulerWnd::OnSortList)
	ON_COMMAND(ID_SCHEDULER_ADD, &CSchedulerWnd::OnSchedulerAdd)
	ON_UPDATE_COMMAND_UI(ID_SCHEDULER_ACTIVATE, &CSchedulerWnd::OnUpdateSchedulerActivate)
	ON_COMMAND(ID_SCHEDULER_ACTIVATE, &CSchedulerWnd::OnSchedulerActivate)
	ON_UPDATE_COMMAND_UI(ID_SCHEDULER_DEACTIVATE, &CSchedulerWnd::OnUpdateSchedulerDeactivate)
	ON_COMMAND(ID_SCHEDULER_DEACTIVATE, &CSchedulerWnd::OnSchedulerDeactivate)
	ON_UPDATE_COMMAND_UI(ID_SCHEDULER_EDIT, &CSchedulerWnd::OnUpdateSchedulerEdit)
	ON_COMMAND(ID_SCHEDULER_EDIT, &CSchedulerWnd::OnSchedulerEdit)
	ON_UPDATE_COMMAND_UI(ID_SCHEDULER_REMOVE, &CSchedulerWnd::OnUpdateSchedulerRemove)
	ON_COMMAND(ID_SCHEDULER_REMOVE, &CSchedulerWnd::OnSchedulerRemove)
	ON_WM_CONTEXTMENU()
	ON_UPDATE_COMMAND_UI(ID_SCHEDULER_REMOVE_ALL, &CSchedulerWnd::OnUpdateSchedulerRemoveAll)
	ON_COMMAND(ID_SCHEDULER_REMOVE_ALL, &CSchedulerWnd::OnSchedulerRemoveAll)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSchedulerWnd construction

CSchedulerWnd::CSchedulerWnd()
	: m_tLastUpdate ( 0 )
{
	Create( IDR_SCHEDULERFRAME );
}

BOOL CSchedulerWnd::IsTaskActive(LPCWSTR szTaskName) const
{
	HRESULT hr;

	CComPtr< ITask > pTask;
	hr = m_pScheduler->Activate( szTaskName, IID_ITask, (IUnknown**)&pTask );
	if ( FAILED( hr ) )
		return FALSE;

	DWORD nFlags = 0;
	hr = pTask->GetFlags( &nFlags );
	if ( FAILED( hr ) )
		return FALSE;

	return ( ( nFlags & TASK_FLAG_DISABLED ) != TASK_FLAG_DISABLED );
}

/////////////////////////////////////////////////////////////////////////////
// CSchedulerWnd message handlers

int CSchedulerWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	HRESULT hr;

	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	hr = m_pScheduler.CoCreateInstance( CLSID_CTaskScheduler );
	if ( FAILED( hr ) ) return -1;

	if ( ! m_wndToolBar.Create( this, WS_CHILD|WS_VISIBLE|CBRS_NOALIGN, AFX_IDW_TOOLBAR ) ) return -1;
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP );

	m_wndList.Create( WS_VISIBLE|LVS_ICON|LVS_AUTOARRANGE|LVS_REPORT|LVS_SHOWSELALWAYS,
		rectDefault, this, IDC_SCHEDULE );

	m_pSizer.Attach( &m_wndList );

	m_wndList.SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE,
		LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP,
		LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP );

	CoolInterface.LoadIconsTo( m_gdiImageList, nImageID );
	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );

	m_wndList.InsertColumn( 0, _T("Action"), LVCFMT_LEFT, 250, -1 );
	m_wndList.InsertColumn( 1, _T("Trigger"), LVCFMT_LEFT, 250, -1 );
	m_wndList.InsertColumn( 2, _T("Next Run"), LVCFMT_CENTER, 100, -1 );
	m_wndList.InsertColumn( 3, _T("Active"), LVCFMT_CENTER, 100, -1 );
	m_wndList.InsertColumn( 4, _T("Status"), LVCFMT_CENTER, 100, -1);
	m_wndList.InsertColumn( 5, _T("Description"), LVCFMT_LEFT, 100, -1 );

	m_wndList.SetFont( &theApp.m_gdiFont );

	LoadState( _T("CSchedulerWnd"), TRUE );

	Update();

	return 0;
}

void CSchedulerWnd::OnDestroy() 
{
	Settings.SaveList( _T("CSchedulerWnd"), &m_wndList );		
	SaveState( _T("CSchedulerWnd") );

	CPanelWnd::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CSchedulerWnd operations

void CSchedulerWnd::Update(int nColumn, BOOL bSort)
{
	HRESULT hr;

	CComPtr< IEnumWorkItems > pEnum;
	hr = m_pScheduler->Enum( &pEnum );
	if ( FAILED( hr ) )
		return;

	CLiveList pLiveList( 6 );

	int nCount = 1;
	for ( ; ; )
	{
		LPWSTR* pszTaskName = NULL;
		hr = pEnum->Next( 1, &pszTaskName, NULL );
		if ( hr != S_OK )
			// No tasks left
			break;
		CString sTaskName = *pszTaskName;
		CoTaskMemFree( pszTaskName );

		CString sVendor = sTaskName.SpanExcluding( _T(".") );
		if ( sVendor.Compare( CLIENT_NAME_T ) != 0 )
			// Wrong name
			continue;
		DWORD_PTR nNumber = _tstoi( sTaskName.Mid( sVendor.GetLength() + 1 ) );

		CComPtr< ITask > pTask;
		hr = m_pScheduler->Activate( sTaskName, IID_ITask, (IUnknown**)&pTask );
		if ( FAILED( hr ) )
			// Can't open task
			continue;

		DWORD nFlags = 0;
		hr = pTask->GetFlags( &nFlags );
		BOOL bActive = ( ( nFlags & TASK_FLAG_DISABLED ) != TASK_FLAG_DISABLED );

		HRESULT hrStatus = S_OK;
		hr = pTask->GetStatus( &hrStatus );

		CString sTriggerString;
		LPWSTR szTriggerString = NULL;
		hr = pTask->GetTriggerString( 0, &szTriggerString );
		if ( hr == S_OK )
		{
			sTriggerString = szTriggerString;
			CoTaskMemFree( szTriggerString );
		}

		CString strDate, strTime;
		SYSTEMTIME pTime = {};
		hr = pTask->GetNextRunTime( &pTime );
		if ( hr == S_OK )
		{
			GetDateFormat( LOCALE_USER_DEFAULT, DATE_SHORTDATE, &pTime, NULL, strDate.GetBuffer( 64 ), 64 );
			GetTimeFormat( LOCALE_USER_DEFAULT, TIME_NOSECONDS, &pTime, NULL, strTime.GetBuffer( 64 ), 64 );
			strDate.ReleaseBuffer();
			strTime.ReleaseBuffer();
		}
		else if ( hr == SCHED_S_TASK_DISABLED )
			bActive = FALSE;

		CString sStatus;
		DWORD nExitCode = 0;
		hr = pTask->GetExitCode( &nExitCode );
		if ( SUCCEEDED( hr ) )
		{
			sStatus.Format( _T("0x%08X"), nExitCode );
		}

		int nAction = -1;
		LPWSTR szParams = NULL;
		hr = pTask->GetParameters( &szParams );
		if ( SUCCEEDED( hr ) )
		{
			CString sParams = szParams;
			CoTaskMemFree( szParams );

			int nPos = sParams.Find( _T("task") );
			if ( nPos != -1 )
				nAction = _tstoi( sParams.Mid( nPos + 4 ) );
			else
				nAction = SYSTEM_START;
		}

		CString sComment;
		LPWSTR szComment = NULL;
		hr = pTask->GetComment( &szComment );
		if ( SUCCEEDED( hr ) )
		{
			sComment = szComment;
			CoTaskMemFree( szComment );
		}

		//Adding tasks we got from Scheduler to temp list and getting a handle
		//to modify their properties according to scheduler item.
		CLiveItem* pItem = pLiveList.Add( nNumber );

		if ( ( nFlags & TASK_FLAG_DISABLED ) != TASK_FLAG_DISABLED )
			pItem->SetImage( 0, SCHEDULE_ITEM_ACTIVE );
		else 
			pItem->SetImage( 0, SCHEDULE_ITEM_INACTIVE );

		//Action column
		switch ( nAction )
		{
		case BANDWIDTH_FULLSPEED:
			pItem->Set( 0, LoadString( IDS_SCHEDULER_BANDWIDTH_FULLSPEED ) );
			break;
		case BANDWIDTH_REDUCEDSPEED:
			pItem->Set( 0, LoadString( IDS_SCHEDULER_BANDWIDTH_REDUCEDSPEED ) );
			break;
		case BANDWIDTH_STOP:
			pItem->Set( 0, LoadString( IDS_SCHEDULER_BANDWIDTH_STOP ) );
			break;
		case SYSTEM_DIALUP_DC:
			pItem->Set( 0, LoadString( IDS_SCHEDULER_SYSTEM_DIALUP_DC ) );
			break;
		case SYSTEM_EXIT:
			pItem->Set( 0, LoadString( IDS_SCHEDULER_SYSTEM_EXIT ) );
			break;
		case SYSTEM_SHUTDOWN:
			pItem->Set( 0, LoadString( IDS_SCHEDULER_SYSTEM_SHUTDOWN ) );
			break;
		case SYSTEM_START:
			pItem->Set( 0, LoadString( IDS_SCHEDULER_SYSTEM_START ) );
			break;
		}

		// Date column
		pItem->Set( 1, sTriggerString );

		// Time column
		pItem->Set( 2, strDate + _T(" ") + strTime );

		// Active column
		if ( bActive )
		{
			switch ( hrStatus )
			{
			case SCHED_S_TASK_NO_MORE_RUNS:
				pItem->Set( 3, LoadString( IDS_SCHEDULER_TASK_DONE ) );
				break;
			case SCHED_S_TASK_RUNNING:
				pItem->Set( 3, LoadString( IDS_SCHEDULER_TASK_ACTIVE ) );
				break;
			default:
				pItem->Set( 3, LoadString( IDS_SCHEDULER_TASK_WAITING ) );
			}
		}
		else
			pItem->Set( 3, LoadString( IDS_SCHEDULER_TASK_INACTIVE ) );

		// Status column 
		pItem->Set( 4, sStatus );

		//Description column
		pItem->Set( 5, sComment );

		++nCount;
	}

	// In case scheduler gave nothing
	if ( nCount == 1 )
	{
		CLiveItem* pDefault = pLiveList.Add( (DWORD_PTR)0 );
		pDefault->Set( 0, LoadString( IDS_SCHEDULER_NOTASK ) );
		pDefault->SetImage( 0, SCHEDULE_NO_ITEM );
	}

	if ( nColumn >= 0 )
	{
		SetWindowLongPtr( m_wndList.GetSafeHwnd(), GWLP_USERDATA, 0 - nColumn - 1 );
	}

	pLiveList.Apply( &m_wndList, bSort );	//Putting items in the main list

	m_tLastUpdate = GetTickCount();	// Update time after it's done doing its work
}

CString CSchedulerWnd::GetItem(int nItem)
{
	if ( nItem > -1 )
	{
		CString sTaskName;
		sTaskName.Format( _T("%s.%04u"), CLIENT_NAME_T, (DWORD)m_wndList.GetItemData( nItem ) );
		return sTaskName;
	}
	return CString();
}

/////////////////////////////////////////////////////////////////////////////
// CSchedulerWnd message handlers

void CSchedulerWnd::OnSize(UINT nType, int cx, int cy) 
{
	CPanelWnd::OnSize( nType, cx, cy );

	SizeListAndBar( &m_wndList, &m_wndToolBar );
	m_wndList.SetWindowPos( NULL, 0, 0, cx, cy - 28, SWP_NOZORDER );
}

void CSchedulerWnd::OnTimer(UINT_PTR nIDEvent) 
{
	if ( nIDEvent == 1 && IsPartiallyVisible() )
	{
		DWORD tTicks = GetTickCount();
		if ( ( tTicks - m_tLastUpdate ) > 1000ul )
		{
			Update();
		}
	}
}

void CSchedulerWnd::OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)pNMHDR;

	if ( pDraw->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( pDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
	{
		LV_ITEM pItem;
		pItem.mask		= LVIF_IMAGE;
		pItem.iItem		= static_cast< int >( pDraw->nmcd.dwItemSpec );
		pItem.iSubItem	= 0;
		m_wndList.GetItem( &pItem );

		switch ( pItem.iImage )
		{
		case 2:
			pDraw->clrText = RGB( 0, 127, 0 );
			break;
		case 3:
			pDraw->clrText = RGB( 224, 0, 0 );
			break;
		}

		*pResult = CDRF_DODEFAULT;
	}
}

void CSchedulerWnd::OnDblClkList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	OnSchedulerEdit();
	*pResult = 0;
}

void CSchedulerWnd::OnSortList(NMHDR* pNotifyStruct, LRESULT *pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNotifyStruct;
	CLiveList::Sort( &m_wndList, pNMListView->iSubItem );
	*pResult = 0;
}

void CSchedulerWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point) 
{
	Skin.TrackPopupMenu( _T("CSchedulerWnd"), point, ID_SCHEDULER_EDIT );
}

void CSchedulerWnd::OnUpdateSchedulerEdit(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() == 1 );
}

void CSchedulerWnd::OnSchedulerEdit() 
{
	CString sTaskName = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
	if ( sTaskName.GetLength() )
	{
		CScheduleTaskDlg dlg( sTaskName );
		if ( dlg.DoModal() == IDOK )
		{
			Update();
		}
	}
}

void CSchedulerWnd::OnUpdateSchedulerRemove(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 );
}

void CSchedulerWnd::OnSchedulerRemove() 
{
	HRESULT hr;

	for ( int nItem = -1 ; ( nItem = m_wndList.GetNextItem( nItem, LVIS_SELECTED ) ) >= 0 ; )
	{
		CString sTaskName = GetItem( nItem );
		if ( sTaskName.GetLength() )
		{
			hr = m_pScheduler->Delete( sTaskName );
		}
	}

	Update();
}

void CSchedulerWnd::OnSchedulerAdd() 
{
	CScheduleTaskDlg dlg;
	if ( dlg.DoModal() == IDOK )
	{
		Update();
	}
}

void CSchedulerWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();

	Settings.LoadList( _T("CSchedulerWnd"), &m_wndList, -2 );
	Skin.CreateToolBar( _T("CSchedulerWnd"), &m_wndToolBar );

	CoolInterface.LoadIconsTo( m_gdiImageList, nImageID );
	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );
}

BOOL CSchedulerWnd::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->wParam == VK_DELETE )
		{
			OnSchedulerRemove();
			return TRUE;
		}
		else if ( pMsg->wParam == VK_INSERT )
		{
			PostMessage( WM_COMMAND, ID_SCHEDULER_ADD );
			return TRUE;
		}
	}

	return CPanelWnd::PreTranslateMessage( pMsg );
}

void CSchedulerWnd::OnUpdateSchedulerDeactivate(CCmdUI* pCmdUI) 
{	
	CString sTaskName = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
	if ( sTaskName.IsEmpty() )
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 && IsTaskActive( sTaskName ) );
}

void CSchedulerWnd::OnSchedulerDeactivate()
{
	HRESULT hr;

	CString sTaskName = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
	if ( sTaskName.IsEmpty() )
		return;

	CComPtr< ITask > pTask;
	hr = m_pScheduler->Activate( sTaskName, IID_ITask, (IUnknown**)&pTask );
	if ( FAILED( hr ) )
		return;

	DWORD nFlags = 0;
	hr = pTask->GetFlags( &nFlags );
	if ( ( nFlags & TASK_FLAG_DISABLED ) != TASK_FLAG_DISABLED )
	{
		hr = pTask->SetFlags( nFlags | TASK_FLAG_DISABLED );
		if ( SUCCEEDED( hr ) )
		{
			CComQIPtr< IPersistFile > pFile( pTask );
			if ( pFile )
			{
				hr = pFile->Save( NULL, TRUE );
				if ( SUCCEEDED( hr ) )
				{
					Update();
				}
			}
		}
	}
}

void CSchedulerWnd::OnUpdateSchedulerActivate(CCmdUI* pCmdUI) 
{	
	CString sTaskName = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
	if ( sTaskName.IsEmpty() )
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	pCmdUI->Enable( m_wndList.GetSelectedCount() > 0 && ! IsTaskActive( sTaskName ) );
}

void CSchedulerWnd::OnSchedulerActivate()
{
	HRESULT hr;

	CString sTaskName = GetItem( m_wndList.GetNextItem( -1, LVIS_SELECTED ) );
	if ( sTaskName.IsEmpty() )
		return;

	CComPtr< ITask > pTask;
	hr = m_pScheduler->Activate( sTaskName, IID_ITask, (IUnknown**)&pTask );
	if ( FAILED( hr ) )
		return;

	DWORD nFlags = 0;
	hr = pTask->GetFlags( &nFlags );
	if ( ( nFlags & TASK_FLAG_DISABLED ) == TASK_FLAG_DISABLED )
	{
		hr = pTask->SetFlags( nFlags & ~TASK_FLAG_DISABLED );
		if ( SUCCEEDED( hr ) )
		{
			CComQIPtr< IPersistFile > pFile( pTask );
			if ( pFile )
			{
				hr = pFile->Save( NULL, TRUE );
				if ( SUCCEEDED( hr ) )
				{
					Update();
				}
			}
		}
	}
}

void CSchedulerWnd::OnUpdateSchedulerRemoveAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetItemCount() > 0 );
}

void CSchedulerWnd::OnSchedulerRemoveAll()
{
	HRESULT hr;

	CString strMessage;
	LoadString( strMessage, IDS_SCHEDULER_REMOVEALL_CONFIRM );
	if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) != IDYES )
		return;

	for ( int nItem = 0 ;  nItem < m_wndList.GetItemCount() ; nItem++)
	{
		CString sTaskName = GetItem( nItem );
		if ( sTaskName.GetLength() )
		{
			hr = m_pScheduler->Delete( sTaskName );
		}
	}

	Update();
}
