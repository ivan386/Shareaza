//
// DlgScheduleTask.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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
#include "Skin.h"
#include "Scheduler.h"
#include "DlgScheduleTask.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScheduleTaskDlg dialog

IMPLEMENT_DYNAMIC(CScheduleTaskDlg, CPropertySheetAdv)

BEGIN_MESSAGE_MAP(CScheduleTaskDlg, CPropertySheetAdv)
	ON_MESSAGE(WM_KICKIDLE, &CScheduleTaskDlg::OnKickIdle)
END_MESSAGE_MAP()

CScheduleTaskDlg::CScheduleTaskDlg(LPCTSTR szTaskName)
	: m_bNew		( szTaskName == NULL )
	, m_sActionTitle( _T("Action") )
	, m_phPages		()
{
	if ( szTaskName )
	{
		m_sTaskName = szTaskName;
	}
}

INT_PTR CScheduleTaskDlg::DoModal(int nPage)
{
	HRESULT hr;

	CScheduleTaskPage pPageAction;
	CPropertyPage pPageTask;
	CPropertyPage pPageSchedule;
	CPropertyPage pPageSettings;

	CComPtr< ITaskScheduler > pScheduler;
	hr = pScheduler.CoCreateInstance( CLSID_CTaskScheduler );
	if ( FAILED( hr ) )
	{
		ReportError( hr );
		return -1;
	}

	CComPtr< ITask > pTask;
	if ( m_bNew )
	{
		for ( int i = 1; ; ++i )
		{
			m_sTaskName.Format( _T("%s.%04d.job"), CLIENT_NAME_T, i );
			hr = pScheduler->NewWorkItem( m_sTaskName, CLSID_CTask,
				IID_ITask, (IUnknown**)&pTask );
			if ( hr != HRESULT_FROM_WIN32( ERROR_FILE_EXISTS ) )
				break;
			// Name collision - try again
		}
	}
	else
	{
		hr = pScheduler->Activate( m_sTaskName, IID_ITask, (IUnknown**)&pTask );
	}
	if ( FAILED( hr ) )
	{
		ReportError( hr );
		return -1;
	}

	if ( m_bNew )
	{
		// Default properties
		hr = pTask->SetApplicationName( _T("\"") + theApp.m_strBinaryPath + _T("\"") );
		hr = pTask->SetWorkingDirectory( _T("\"") + Settings.General.Path + _T("\"") );
		hr = pTask->SetMaxRunTime( INFINITE );
		
		// Current user
		CString sUser;
		DWORD nUserLength = 128;
		GetUserNameEx( NameSamCompatible, sUser.GetBuffer( nUserLength ), &nUserLength );
		sUser.ReleaseBuffer();
		hr = pTask->SetAccountInformation( sUser, NULL );
		DWORD nFlags = 0;
		hr = pTask->GetFlags( &nFlags );
		hr = pTask->SetFlags( nFlags | TASK_FLAG_RUN_ONLY_IF_LOGGED_ON );
		
		// Default trigger
		{
			WORD id;
			CComPtr< ITaskTrigger > pTrig;
			hr = pTask->CreateTrigger( &id, &pTrig );
			if ( SUCCEEDED( hr ) )
			{
				SYSTEMTIME stNow;
				GetLocalTime( &stNow );
				TASK_TRIGGER trig =
				{
					sizeof( TASK_TRIGGER ), 0,
					stNow.wYear, stNow.wMonth, stNow.wDay,
					0, 0, 0,
					stNow.wHour, stNow.wMinute,
					0, 0,
					0,
					TASK_TIME_TRIGGER_DAILY,
					{ 1 }
				};
				hr = pTrig->SetTrigger( &trig );
			}
		}

		// Save data for correct use of property pages
		CComQIPtr< IPersistFile > pFile( pTask );
		if ( ! pFile )
			return -1;
		hr = pFile->Save( NULL, TRUE );
		if ( FAILED( hr ) )
		{
			ReportError( hr );
			return -1;
		}
	}
	else
	{
		LPWSTR szParams = NULL;
		hr = pTask->GetParameters( &szParams );
		if ( SUCCEEDED( hr ) )
		{
			CString sTaskData = szParams;
			CoTaskMemFree( szParams );

			// Decode task options from command line
			int nPos = sTaskData.Trim().Find( _T("task") );
			if ( nPos != -1 )
			{
				sTaskData = sTaskData.Mid( nPos + 4 ).SpanExcluding( _T(" \t") );
				int nAction = 0, nLimitDown = 0, nLimitUp = 0, nDisabled = 0, nEnabled = 0;
				if ( _stscanf( sTaskData, _T("%d:%d:%d:%d:%d"),
					&nAction, &nLimitDown, &nLimitUp, &nDisabled, &nEnabled ) > 0 )
				{
					pPageAction.m_nAction = nAction;
					pPageAction.m_nLimitDown = nLimitDown;
					pPageAction.m_nLimitUp = nLimitUp;
					pPageAction.m_bG1   = ( nEnabled &  1 ) ? BST_CHECKED : ( ( nDisabled &  1 ) ? BST_UNCHECKED : BST_INDETERMINATE );
					pPageAction.m_bG2   = ( nEnabled &  2 ) ? BST_CHECKED : ( ( nDisabled &  2 ) ? BST_UNCHECKED : BST_INDETERMINATE );
					pPageAction.m_bED2K = ( nEnabled &  4 ) ? BST_CHECKED : ( ( nDisabled &  4 ) ? BST_UNCHECKED : BST_INDETERMINATE );
					pPageAction.m_bDC   = ( nEnabled &  8 ) ? BST_CHECKED : ( ( nDisabled &  8 ) ? BST_UNCHECKED : BST_INDETERMINATE );
					pPageAction.m_bBT   = ( nEnabled & 16 ) ? BST_CHECKED : ( ( nDisabled & 16 ) ? BST_UNCHECKED : BST_INDETERMINATE );
				}
			}
		}
	}

	SetTabTitle( &pPageAction, m_sActionTitle );
	m_pages.Add( &pPageAction );
	m_pages.Add( &pPageTask );
	m_pages.Add( &pPageSchedule );
	m_pages.Add( &pPageSettings );

	m_psh.dwFlags &= ~PSH_PROPSHEETPAGE;
	m_psh.nPages = (UINT)m_pages.GetCount();
	m_psh.nStartPage = nPage;

	// Load first page from resource
	m_phPages[ 0 ] = pPageAction.Create( IsWizard() );
	if ( ! m_phPages[ 0 ] )
		return -1;

	// Load other pages from scheduler task
	CComQIPtr< IProvideTaskPage > pPage( pTask );
	if ( ! pPage )
		return -1;
	hr = pPage->GetPage( TASKPAGE_TASK, FALSE, &m_phPages[ 1 ] );
	if ( FAILED( hr ) )
		return -1;
	hr = pPage->GetPage( TASKPAGE_SCHEDULE, FALSE, &m_phPages[ 2 ] );
	if ( FAILED( hr ) )
		return -1;
	hr = pPage->GetPage( TASKPAGE_SETTINGS, FALSE, &m_phPages[ 3 ] );
	if ( FAILED( hr ) )
		return -1;

	m_psh.phpage = m_phPages;
	INT_PTR ret = CPropertySheetAdv::DoModal();
	m_psh.ppsp = NULL; // Avoid deletion in destructor

	if ( ret == IDOK )
	{
		// Set task properties
		hr = pTask->SetApplicationName( _T("\"") + theApp.m_strBinaryPath + _T("\"") );
		hr = pTask->SetWorkingDirectory( _T("\"") + Settings.General.Path + _T("\"") );

		// Encode task options to command line
		CString sParams;
		if ( pPageAction.m_nAction == SYSTEM_START )
			sParams = _T("-tray");
		else
			sParams.Format( _T("-task%d:%d:%d:%d:%d"),
				pPageAction.m_nAction,
				pPageAction.m_nLimitDown,
				pPageAction.m_nLimitUp,
				// Disabled networks
				( ( pPageAction.m_bG1   == BST_UNCHECKED ) ?  1 : 0 ) |
				( ( pPageAction.m_bG2   == BST_UNCHECKED ) ?  2 : 0 ) |
				( ( pPageAction.m_bED2K == BST_UNCHECKED ) ?  4 : 0 ) |
				( ( pPageAction.m_bDC   == BST_UNCHECKED ) ?  8 : 0 ) |
				( ( pPageAction.m_bBT   == BST_UNCHECKED ) ? 16 : 0 ),
				// Enabled networks
				( ( pPageAction.m_bG1   == BST_CHECKED ) ?  1 : 0 ) |
				( ( pPageAction.m_bG2   == BST_CHECKED ) ?  2 : 0 ) |
				( ( pPageAction.m_bED2K == BST_CHECKED ) ?  4 : 0 ) |
				( ( pPageAction.m_bDC   == BST_CHECKED ) ?  8 : 0 ) |
				( ( pPageAction.m_bBT   == BST_CHECKED ) ? 16 : 0 ) );
		hr = pTask->SetParameters( sParams );

		CComQIPtr< IPersistFile > pFile( pTask );
		if ( ! pFile )
			return -1;
		pPage.Release();
		pTask.Release();
		hr = pFile->Save( NULL, TRUE );
		if ( FAILED( hr ) )
		{
			ReportError( hr );
			return -1;
		}
	}
	else
	{
		pPage.Release();
		pTask.Release();
	}

	if ( ret != IDOK  && m_bNew )
	{
		// Delete new but canceled task
		hr = pScheduler->Delete( m_sTaskName );
	}

	return ret;
}

void CScheduleTaskDlg::BuildPropPageArray()
{
}

#ifdef _DEBUG

void CScheduleTaskDlg::AssertValid() const
{
	CWnd::AssertValid();
	m_pages.AssertValid();
	ASSERT( m_psh.dwSize == sizeof( m_psh ) );
	ASSERT( ( m_psh.dwFlags & PSH_PROPSHEETPAGE ) != PSH_PROPSHEETPAGE );
}

#endif // _DEBUG

LRESULT CScheduleTaskDlg::OnKickIdle(WPARAM /*wp*/, LPARAM /*lp*/)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CScheduleTaskDlg message handlers

BOOL CScheduleTaskDlg::OnInitDialog() 
{
	BOOL bResult = CPropertySheetAdv::OnInitDialog();

	SetFont( &theApp.m_gdiFont );
	SetIcon( theApp.LoadIcon( IDR_SCHEDULERFRAME ), TRUE );

	CString strCaption;
	LoadString( strCaption, IDR_SCHEDULERFRAME );
	SetWindowText( strCaption );

	if ( GetDlgItem( IDOK ) )
	{
		CRect rc;
		GetDlgItem( IDOK )->GetWindowRect( &rc );
		ScreenToClient( &rc );
		GetDlgItem( IDOK )->SetWindowPos( NULL, 6, rc.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE );
		GetDlgItem( IDCANCEL )->SetWindowPos( NULL, 11 + rc.Width(), rc.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE );
	}

	if ( GetDlgItem( 0x3021 ) ) GetDlgItem( 0x3021 )->ShowWindow( SW_HIDE );
	if ( GetDlgItem( 0x0009 ) ) GetDlgItem( 0x0009 )->ShowWindow( SW_HIDE );

	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// CScheduleTaskPage dialog

IMPLEMENT_DYNAMIC(CScheduleTaskPage, CPropertyPageAdv)

BEGIN_MESSAGE_MAP(CScheduleTaskPage, CPropertyPageAdv)
	ON_BN_CLICKED(IDC_SCHEDULER_TOGGLE_BANDWIDTH, &CScheduleTaskPage::OnBnClickedSchedulerToggleBandwidth)
	ON_EN_CHANGE(IDC_SCHEDULER_LIMITED_DOWN, &CScheduleTaskPage::OnEnChangeSchedulerLimitedDown)
	ON_EN_CHANGE(IDC_SCHEDULER_LIMITED_UP, &CScheduleTaskPage::OnEnChangeSchedulerLimitedUp)
	ON_CBN_SELCHANGE(IDC_SCHEDULER_EVENTTYPE, &CScheduleTaskPage::OnCbnSelchangeEventtype)
END_MESSAGE_MAP()

CScheduleTaskPage::CScheduleTaskPage()
	: CPropertyPageAdv( CScheduleTaskPage::IDD )
	, m_nAction( SYSTEM_START )
	, m_bToggleBandwidth( TRUE )
	, m_nLimitDown( 0 )
	, m_nLimitUp( 0 )
	, m_bG1( BST_INDETERMINATE )
	, m_bG2( BST_INDETERMINATE )
	, m_bED2K( BST_INDETERMINATE )
	, m_bDC( BST_INDETERMINATE )
	, m_bBT( BST_INDETERMINATE )
{
}

void CScheduleTaskPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageAdv::DoDataExchange( pDX );

	DDX_Control(pDX, IDC_SCHEDULER_EVENTTYPE, m_wndAction);
	DDX_Check(pDX, IDC_SCHEDULER_TOGGLE_BANDWIDTH, m_bToggleBandwidth);
	DDX_Control(pDX, IDC_SCHEDULER_TOGGLE_BANDWIDTH, m_wndToggleBandwidth);
	DDX_Text(pDX, IDC_SCHEDULER_LIMITED_DOWN, m_nLimitDown);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_DOWN, m_wndLimitedEditDown);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_SPIN_DOWN, m_wndSpinDown);
	DDX_Text(pDX, IDC_SCHEDULER_LIMITED_UP, m_nLimitUp);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_UP, m_wndLimitedEditUp);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_SPIN_UP, m_wndSpinUp);
	DDX_Check(pDX, IDC_SCHEDULER_LIMITED_G1, m_bG1);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_G1, m_wndG1);
	DDX_Check(pDX, IDC_SCHEDULER_LIMITED_G2, m_bG2);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_G2, m_wndG2);
	DDX_Check(pDX, IDC_SCHEDULER_LIMITED_ED2K, m_bED2K);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_ED2K, m_wndED2K);
	DDX_Check(pDX, IDC_SCHEDULER_LIMITED_DC, m_bDC);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_DC, m_wndDC);
	DDX_Check(pDX, IDC_SCHEDULER_LIMITED_BT, m_bBT);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_BT, m_wndBT);
}

HPROPSHEETPAGE CScheduleTaskPage::Create(BOOL bWizard)
{
	auto_array< BYTE >pBuf( new BYTE[ m_psp.dwSize ] );
	PROPSHEETPAGE& psp = *(PROPSHEETPAGE*)pBuf.get();
	CopyMemory( &psp, &m_psp, m_psp.dwSize );
	if ( ! m_strHeaderTitle.IsEmpty() )
	{
		psp.pszHeaderTitle = m_strHeaderTitle;
		psp.dwFlags |= PSP_USEHEADERTITLE;
	}
	if ( ! m_strHeaderSubTitle.IsEmpty() )
	{
		psp.pszHeaderSubTitle = m_strHeaderSubTitle;
		psp.dwFlags |= PSP_USEHEADERSUBTITLE;
	}
	PreProcessPageTemplate( psp, bWizard );
	return CreatePropertySheetPage( &psp );
}

BOOL CScheduleTaskPage::OnInitDialog() 
{
	CPropertyPageAdv::OnInitDialog();

	ASSERT( m_nLimitDown >= 0 && m_nLimitDown <= 100 );
	ASSERT( m_nLimitUp >= 0 && m_nLimitUp <= 100 );

	m_bToggleBandwidth = ( m_nLimitDown == m_nLimitUp );

	// TODO: New tasks should be added to the bottom of the list with the same order it is
	// added to scheduler enum
	m_wndAction.AddString( LoadString( IDS_SCHEDULER_BANDWIDTH_FULLSPEED ) );
	m_wndAction.AddString( LoadString( IDS_SCHEDULER_BANDWIDTH_REDUCEDSPEED ) );
	m_wndAction.AddString( LoadString( IDS_SCHEDULER_BANDWIDTH_STOP ) );
	m_wndAction.AddString( LoadString( IDS_SCHEDULER_SYSTEM_DIALUP_DC ) );
	m_wndAction.AddString( LoadString( IDS_SCHEDULER_SYSTEM_EXIT ) );
	m_wndAction.AddString( LoadString( IDS_SCHEDULER_SYSTEM_SHUTDOWN ) );
	m_wndAction.AddString( LoadString( IDS_SCHEDULER_SYSTEM_START ) );
	m_wndAction.SetCurSel( m_nAction );

	m_wndSpinDown.SetRange( 5, 95 );
	m_wndSpinDown.SetPos( m_nLimitDown );
	m_wndSpinUp.SetRange( 5, 95 );
	m_wndSpinUp.SetPos( m_nLimitUp );

	UpdateData( FALSE );

	Update();

	return TRUE;
}

void CScheduleTaskPage::Update()
{
	BOOL bBandwidth =
		m_nAction == BANDWIDTH_FULLSPEED ||
		m_nAction == BANDWIDTH_REDUCEDSPEED;
	BOOL bReducedSpeed =
		m_nAction == BANDWIDTH_REDUCEDSPEED;

	m_wndToggleBandwidth.EnableWindow( bReducedSpeed );
	m_wndLimitedEditDown.EnableWindow( bReducedSpeed );
	m_wndSpinDown.EnableWindow( bReducedSpeed );
	m_wndLimitedEditUp.EnableWindow( bReducedSpeed && ! m_bToggleBandwidth );
	m_wndSpinUp.EnableWindow( bReducedSpeed && ! m_bToggleBandwidth );

	m_wndG1.EnableWindow( bBandwidth );
	m_wndG2.EnableWindow( bBandwidth );
	m_wndED2K.EnableWindow( bBandwidth );
	m_wndDC.EnableWindow( bBandwidth );
	m_wndBT.EnableWindow( bBandwidth );
}

/////////////////////////////////////////////////////////////////////////////
// CScheduleTaskPage message handlers

void CScheduleTaskPage::OnBnClickedSchedulerToggleBandwidth()
{
	if ( ! m_wndAction.m_hWnd ) return;

	UpdateData();

	Update();
}

void CScheduleTaskPage::OnEnChangeSchedulerLimitedDown()
{
	if ( ! m_wndAction.m_hWnd ) return;

	UpdateData();

	if ( m_bToggleBandwidth )
	{
		m_nLimitUp = m_nLimitDown;
		UpdateData( FALSE );
	}
}

void CScheduleTaskPage::OnEnChangeSchedulerLimitedUp()
{
	if ( ! m_wndAction.m_hWnd ) return;

	UpdateData();
}

void CScheduleTaskPage::OnCbnSelchangeEventtype()
{
	if ( ! m_wndAction.m_hWnd ) return;

	UpdateData();

	m_nAction = m_wndAction.GetCurSel();
	if ( m_nAction == BANDWIDTH_FULLSPEED )
	{
		m_nLimitDown = m_nLimitUp = 100;
		UpdateData( FALSE );
	}
	else if ( m_nAction == BANDWIDTH_REDUCEDSPEED )
	{
		m_nLimitDown = m_nLimitUp = 50;
		UpdateData( FALSE );
	}
	else if ( m_nAction == BANDWIDTH_STOP )
	{
		m_nLimitDown = m_nLimitUp = 0;
		UpdateData( FALSE );
	}

	Update();
}
