//
// DlgScheduleItem.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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
#include "Scheduler.h"
#include "Network.h"
#include "Skin.h"
#include "DlgScheduleTask.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CScheduleTaskDlg, CSkinDialog)
	//{{AFX_MSG_MAP(CScheduleTaskDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_ONLYONCE, OnBnClickedOnlyonce)
	ON_BN_CLICKED(IDC_SCHEDULER_TOGGLE_BANDWIDTH, OnBnClickedToggleBandwidth)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATE, OnDtnDatetimechangeDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIME, OnDtnDatetimechangeTime)
	ON_BN_CLICKED(IDC_EVERYDAY, OnBnClickedEveryday)
	ON_BN_CLICKED(IDC_ACTIVE, OnBnClickedActive)
	ON_CBN_SELCHANGE(IDC_EVENTTYPE, OnCbnSelchangeEventtype)
	ON_BN_CLICKED(IDC_BUTTON_ALLDAYS, &CScheduleTaskDlg::OnBnClickedButtonAllDays)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CScheduleTaskDlg dialog

CScheduleTaskDlg::CScheduleTaskDlg(CWnd* pParent, CScheduleTask* pSchTask) : CSkinDialog(CScheduleTaskDlg::IDD, pParent)
{
	m_pScheduleTask = pSchTask;
	if ( m_pScheduleTask )
		m_bNew = false;
	else
		m_bNew = true;
}

CScheduleTaskDlg::~CScheduleTaskDlg()
{
	// If we are creating a new schedule item and it is created, it should be already
	// added to scheduler list so we delete dialog's object 
	if (  m_bNew && m_pScheduleTask )
		delete m_pScheduleTask;
}

void CScheduleTaskDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScheduleTaskDlg)

	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_DATE, m_wndDate);
	DDX_Control(pDX, IDC_TIME, m_wndTime);
	DDX_Text(pDX, IDC_DESCRIPTION, m_sDescription);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_SPIN, m_wndSpin);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_SPIN_DOWN, m_wndSpinDown);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_SPIN_UP, m_wndSpinUp);
	DDX_Check(pDX, IDC_SCHEDULER_TOGGLE_BANDWIDTH, m_bToggleBandwidth);
	DDX_Check(pDX, IDC_SCHEDULER_LIMITED_NETWORKS, m_bLimitedNetworks);
	DDX_Text(pDX, IDC_SCHEDULER_LIMITED, m_nLimit);
	DDX_Text(pDX, IDC_SCHEDULER_LIMITED_DOWN, m_nLimitDown);
	DDX_Text(pDX, IDC_SCHEDULER_LIMITED_UP, m_nLimitUp);
	DDX_Control(pDX, IDC_ACTIVE, m_wndActiveCheck);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED, m_wndLimitedEdit);
	DDX_Control(pDX, IDC_EVENTTYPE, m_wndTypeSel);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_DOWN, m_wndLimitedEditDown);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_UP, m_wndLimitedEditUp);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_NETWORKS, m_wndLimitedCheck);
	DDX_Control(pDX, IDC_SCHEDULER_TOGGLE_BANDWIDTH, m_wndLimitedCheckTgl);
	DDX_Control(pDX, IDC_STATIC_LIMITED, m_wndLimitedStatic);
	DDX_Control(pDX, IDC_STATIC_LIMITED_DOWN, m_wndLimitedStaticDown);
	DDX_Control(pDX, IDC_STATIC_LIMITED_UP, m_wndLimitedStaticUp);
	DDX_Control(pDX, IDC_EVERYDAY, m_wndRadioEveryDay);
	DDX_Control(pDX, IDC_ONLYONCE, m_wndRadioOnce);
	DDX_Control(pDX, IDC_CHECK_SUN, m_wndChkDaySun);
	DDX_Control(pDX, IDC_CHECK_MON, m_wndChkDayMon);
	DDX_Control(pDX, IDC_CHECK_TUES, m_wndChkDayTues);
	DDX_Control(pDX, IDC_CHECK_WED, m_wndChkDayWed);
	DDX_Control(pDX, IDC_CHECK_THU, m_wndChkDayThu);
	DDX_Control(pDX, IDC_CHECK_FRI, m_wndChkDayFri);
	DDX_Control(pDX, IDC_CHECK_SAT, m_wndChkDaySat);
	DDX_Control(pDX, IDC_DAYOFWEEK_GBOX, m_wndGrpBoxDayOfWeek);
	DDX_Control(pDX, IDC_BUTTON_ALLDAYS, m_wndBtnAllDays);
}

/////////////////////////////////////////////////////////////////////////////
// CScheduleTaskDlg message handlers

BOOL CScheduleTaskDlg::OnInitDialog() 
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CScheduleTaskDlg"), IDR_SCHEDULERFRAME );

	m_wndSpin.SetRange( 5, 95 );
	m_wndSpinDown.SetRange( 5, 95 );
	m_wndSpinUp.SetRange( 5, 95 );

	// TODO: New tasks should be added to the buttom of the list with the same order it is
	// added to scheduler enum
	m_wndTypeSel.AddString( LoadString( IDS_SCHEDULER_BANDWIDTH_FULLSPEED ) );
	m_wndTypeSel.AddString( LoadString( IDS_SCHEDULER_BANDWIDTH_REDUCEDSPEED ) );
	m_wndTypeSel.AddString( LoadString( IDS_SCHEDULER_BANDWIDTH_STOP ) );
	m_wndTypeSel.AddString( LoadString( IDS_SCHEDULER_SYSTEM_DIALUP_DC ) );
	m_wndTypeSel.AddString( LoadString( IDS_SCHEDULER_SYSTEM_EXIT ) );
	m_wndTypeSel.AddString( LoadString( IDS_SCHEDULER_SYSTEM_TURNOFF ) );

	if ( m_bNew )	//We are creating new schedule task, setting default values
	{
		m_pScheduleTask		= new CScheduleTask ();
		m_bSpecificDays		= false;
		m_nAction			= BANDWIDTH_FULL_SPEED;
		m_bActive			= true;
		m_nDays				= 0x7F;
		m_nLimit			= 50;
		m_nLimitDown		= 50;
		m_nLimitUp			= 25;
		m_bToggleBandwidth	= false;
		m_bLimitedNetworks	= false;
		m_wndActiveCheck.SetCheck(1);
		m_wndDate.GetTime(m_tDateAndTime);	//Sets mtDateTime to now
		m_wndRadioOnce.SetCheck(1);
		m_wndRadioEveryDay.SetCheck(0);
		m_wndLimitedEdit.EnableWindow( false );
		m_wndLimitedEditDown.EnableWindow( false );
		m_wndLimitedEditUp.EnableWindow( false );
		m_wndSpin.EnableWindow( false );
		m_wndSpinDown.EnableWindow( false );
		m_wndSpinUp.EnableWindow( false );
		m_wndLimitedCheckTgl.EnableWindow( false );
		m_wndLimitedCheck.EnableWindow( false );
		m_wndLimitedStatic.EnableWindow( false );
		m_wndLimitedStaticDown.EnableWindow( false );
		m_wndLimitedStaticUp.EnableWindow( false );
	}
	else	//We are editing an existing schedule task, getting values from it
	{
		m_bSpecificDays	= m_pScheduleTask->m_bSpecificDays;
		m_nAction		= m_pScheduleTask->m_nAction;
		m_sDescription	= m_pScheduleTask->m_sDescription;
		m_tDateAndTime	= m_pScheduleTask->m_tScheduleDateTime;
		m_bActive		= m_pScheduleTask->m_bActive;
		m_nLimit		= m_pScheduleTask->m_nLimit;
		m_nLimitDown	= m_pScheduleTask->m_nLimitDown;
		m_nLimitUp		= m_pScheduleTask->m_nLimitUp;
		m_nDays			= m_pScheduleTask->m_nDays;
		m_bToggleBandwidth	= m_pScheduleTask->m_bToggleBandwidth;
		m_bLimitedNetworks	= m_pScheduleTask->m_bLimitedNetworks;

		if ( m_pScheduleTask->m_bExecuted && ! m_pScheduleTask->m_bSpecificDays )
			m_bActive = true;

		m_wndActiveCheck.SetCheck(m_bActive);

		switch ( m_nAction )
		{
		case 0:
			// Should never happen
			m_wndTypeSel.SetCurSel(-1);
			break;
		case BANDWIDTH_FULL_SPEED:
			m_wndTypeSel.SetCurSel(0);
			break;
		case BANDWIDTH_REDUCED_SPEED:
			m_wndTypeSel.SetCurSel(1);
			break;
		case BANDWIDTH_STOP:
			m_wndTypeSel.SetCurSel(2);
			break;
		case SYSTEM_DISCONNECT:
			m_wndTypeSel.SetCurSel(3);
			break;
		case SYSTEM_EXIT:
			m_wndTypeSel.SetCurSel(4);
			break;
		case SYSTEM_SHUTDOWN:
			m_wndTypeSel.SetCurSel(5);
			break;
		}

		m_wndRadioOnce.SetCheck(!m_bSpecificDays);
		m_wndRadioEveryDay.SetCheck(m_bSpecificDays);
		m_wndDate.SetTime(&m_tDateAndTime);
		m_wndTime.SetTime(&m_tDateAndTime);

		// If task is scheduled for everyday disable date window
		if ( m_wndRadioEveryDay.GetCheck() )
			m_wndDate.EnableWindow( false );

		if ( m_wndTypeSel.GetCurSel() + 1 == BANDWIDTH_REDUCED_SPEED )
		{
			if ( m_bToggleBandwidth )
			{
				m_wndLimitedEdit.EnableWindow( false );
				m_wndLimitedEditDown.EnableWindow( true );
				m_wndLimitedEditUp.EnableWindow( true );

				m_wndSpin.EnableWindow( false );
				m_wndSpinDown.EnableWindow( true );
				m_wndSpinUp.EnableWindow( true );

				m_wndLimitedStatic.EnableWindow( false );
				m_wndLimitedStaticDown.EnableWindow( true );
				m_wndLimitedStaticUp.EnableWindow( true );
			}
			else
			{
				m_wndLimitedEdit.EnableWindow( true );
				m_wndLimitedEditDown.EnableWindow( false );
				m_wndLimitedEditUp.EnableWindow( false );

				m_wndSpin.EnableWindow( true );
				m_wndSpinDown.EnableWindow( false );
				m_wndSpinUp.EnableWindow( false );

				m_wndLimitedStatic.EnableWindow( true );
				m_wndLimitedStaticDown.EnableWindow( false );
				m_wndLimitedStaticUp.EnableWindow( false );
			}
			//m_wndLimitedCheckTgl.EnableWindow(m_bToggleBandwidth);
			//m_wndLimitedCheck.EnableWindow(m_bLimitedNetworks);
		}
		else
		{
			m_wndLimitedEdit.EnableWindow( false );
			m_wndLimitedEditDown.EnableWindow( false );
			m_wndLimitedEditUp.EnableWindow( false );
			m_wndSpin.EnableWindow( false );
			m_wndSpinDown.EnableWindow( false );
			m_wndSpinUp.EnableWindow( false );
			m_wndLimitedCheckTgl.EnableWindow( false );
			m_wndLimitedCheck.EnableWindow( false );

		}
	}

	m_wndChkDaySun.SetCheck( m_nDays & SUNDAY );
	m_wndChkDayMon.SetCheck( m_nDays & MONDAY );
	m_wndChkDayTues.SetCheck( m_nDays & TUESDAY );
	m_wndChkDayWed.SetCheck( m_nDays & WEDNESDAY );
	m_wndChkDayThu.SetCheck( m_nDays & THURSDAY );
	m_wndChkDayFri.SetCheck( m_nDays & FRIDAY );
	m_wndChkDaySat.SetCheck( m_nDays & SATURDAY );

	m_wndSpin.SetPos(m_nLimit);
	m_wndSpinDown.SetPos(m_nLimitDown);
	m_wndSpinUp.SetPos(m_nLimitUp);

	if( m_bSpecificDays )
		EnableDaysOfWeek( true );
	else
		EnableDaysOfWeek( false );

	UpdateData( FALSE );

	//if (m_wndTypeSel.GetCurSel()+1 == BANDWIDTH_REDUCED_SPEED) OnBnClickedToggleBandwidth();

	return FALSE;
}

void CScheduleTaskDlg::OnOK() 
{
	UpdateData( TRUE );

	switch ( m_wndTypeSel.GetCurSel() )
	{
	case -1:
		AfxMessageBox( IDS_SCHEDULER_SELECTTASK );
		return;
	case 0:
		m_nAction = BANDWIDTH_FULL_SPEED;
		break;
	case 1:
		m_nAction = BANDWIDTH_REDUCED_SPEED;
		break;
	case 2:
		m_nAction = BANDWIDTH_STOP;
		break;
	case 3:
		m_nAction = SYSTEM_DISCONNECT;
		break;
	case 4:
		m_nAction = SYSTEM_EXIT;
		break;
	case 5:
		m_nAction = SYSTEM_SHUTDOWN;
		break;
	}

	if ( m_wndRadioOnce.GetCheck() )
	{
		if ( CTime::GetCurrentTime() >= m_tDateAndTime )
		{
			AfxMessageBox( IDS_SCHEDULER_TIME_PASSED );
			return;
		}
	}

	if ( m_wndRadioOnce.GetCheck() )
		m_bSpecificDays = false;
	else
	{
		m_nDays = 0;
		if( m_wndChkDaySun.GetCheck() ) m_nDays |= SUNDAY; 
		if( m_wndChkDayMon.GetCheck() ) m_nDays |= MONDAY;
		if( m_wndChkDayTues.GetCheck() ) m_nDays |= TUESDAY; 
		if( m_wndChkDayWed.GetCheck() ) m_nDays |= WEDNESDAY;
		if( m_wndChkDayThu.GetCheck() ) m_nDays |= THURSDAY;
		if( m_wndChkDayFri.GetCheck() ) m_nDays |= FRIDAY;
		if( m_wndChkDaySat.GetCheck() ) m_nDays |= SATURDAY;
		if( ! m_nDays )
		{
			AfxMessageBox( IDS_SCHEDULER_SELECTADAY );
			return;
		}

		m_bSpecificDays = true;
	}

	if ( m_wndActiveCheck.GetCheck() )
		m_bActive = true;
	else
		m_bActive = false;

	m_pScheduleTask->m_nLimit			= m_nLimit;
	m_pScheduleTask->m_nLimitDown		= m_nLimitDown;
	m_pScheduleTask->m_nLimitUp			= m_nLimitUp;
	m_pScheduleTask->m_bToggleBandwidth	= m_bToggleBandwidth != 0;
	m_pScheduleTask->m_bLimitedNetworks	= m_bLimitedNetworks != 0;
	m_pScheduleTask->m_tScheduleDateTime = m_tDateAndTime;
	m_pScheduleTask->m_bSpecificDays	= m_bSpecificDays;
	m_pScheduleTask->m_nAction			= m_nAction;
	m_pScheduleTask->m_sDescription		= m_sDescription;
	m_pScheduleTask->m_bActive			= m_bActive;
	m_pScheduleTask->m_bExecuted		= false;
	m_pScheduleTask->m_nDays			= m_nDays;

	Scheduler.Add(m_pScheduleTask);
	m_pScheduleTask = NULL;

	CSkinDialog::OnOK();
}

void CScheduleTaskDlg::OnBnClickedOnlyonce()
{
	m_wndRadioEveryDay.SetCheck(0);
	m_bSpecificDays = false;

	m_wndDate.EnableWindow( true );

	EnableDaysOfWeek( false );
}

void CScheduleTaskDlg::OnDtnDatetimechangeDate(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	//LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	SYSTEMTIME tDate;
	SYSTEMTIME tTime;
	m_wndDate.GetTime( &tDate );
	m_wndTime.GetTime( &tTime );
	CTime tTemp( tDate.wYear, tDate.wMonth, tDate.wDay, tTime.wHour, tTime.wMinute, tTime.wSecond );
	m_tDateAndTime = tTemp;
	*pResult = 0;
}

void CScheduleTaskDlg::OnDtnDatetimechangeTime(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	//LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);

	SYSTEMTIME tDate;
	SYSTEMTIME tTime;
	m_wndDate.GetTime( &tDate );
	m_wndTime.GetTime( &tTime );
	CTime tTemp( tDate.wYear, tDate.wMonth, tDate.wDay, tTime.wHour, tTime.wMinute, tTime.wSecond );
	m_tDateAndTime = tTemp;

	*pResult = 0;
}

void CScheduleTaskDlg::OnBnClickedEveryday()
{
	m_wndRadioOnce.SetCheck(0);

	m_wndDate.EnableWindow( false );

	EnableDaysOfWeek( true );
}

void CScheduleTaskDlg::OnBnClickedToggleBandwidth()
{
	if ( m_wndLimitedCheckTgl.GetCheck() )
	{
		m_wndLimitedEdit.EnableWindow( false );
		m_wndLimitedEditDown.EnableWindow( true );
		m_wndLimitedEditUp.EnableWindow( true );

		m_wndSpin.EnableWindow( false );
		m_wndSpinDown.EnableWindow( true );
		m_wndSpinUp.EnableWindow( true );

		m_wndLimitedStatic.EnableWindow( false );
		m_wndLimitedStaticDown.EnableWindow( true );
		m_wndLimitedStaticUp.EnableWindow( true );
	}
	else
	{
		m_wndLimitedEdit.EnableWindow( true );
		m_wndLimitedEditDown.EnableWindow( false );
		m_wndLimitedEditUp.EnableWindow( false );

		m_wndSpin.EnableWindow( true );
		m_wndSpinDown.EnableWindow( false );
		m_wndSpinUp.EnableWindow( false );

		m_wndLimitedStatic.EnableWindow( true );
		m_wndLimitedStaticDown.EnableWindow( false );
		m_wndLimitedStaticUp.EnableWindow( false );
	}
}

void CScheduleTaskDlg::OnBnClickedActive()
{
	if ( ! m_wndActiveCheck.GetCheck() )
	{
		m_wndActiveCheck.SetCheck(0);
	}
	else
	{
		m_wndActiveCheck.SetCheck(1);
	}
}

void CScheduleTaskDlg::OnCbnSelchangeEventtype()
{
	if ( m_wndTypeSel.GetCurSel() + 1 == BANDWIDTH_REDUCED_SPEED )
	{
		if ( ! m_wndLimitedCheckTgl.GetCheck() )
		{
			m_wndLimitedEdit.EnableWindow( true );
			m_wndSpin.EnableWindow( true );
		}
		else
		{
			m_wndLimitedEditDown.EnableWindow( true );
			m_wndLimitedEditUp.EnableWindow( true );
			m_wndSpinDown.EnableWindow( true );
			m_wndSpinUp.EnableWindow( true );				
		}
		m_wndLimitedCheckTgl.EnableWindow( true );
		m_wndLimitedCheck.EnableWindow( true );
		m_wndLimitedStatic.EnableWindow( true );
		m_wndLimitedStaticDown.EnableWindow( true );
		m_wndLimitedStaticUp.EnableWindow( true );
	}
	else
	{
		m_wndLimitedEdit.EnableWindow( false );
		m_wndLimitedEditDown.EnableWindow( false );
		m_wndLimitedEditUp.EnableWindow( false );
		m_wndSpin.EnableWindow( false );
		m_wndSpinDown.EnableWindow( false );
		m_wndSpinUp.EnableWindow( false );
		m_wndLimitedCheckTgl.EnableWindow( false );
		m_wndLimitedCheck.EnableWindow( false );
		m_wndLimitedStatic.EnableWindow( false );
		m_wndLimitedStaticDown.EnableWindow( false );
		m_wndLimitedStaticUp.EnableWindow( false );
	}

}

void CScheduleTaskDlg::EnableDaysOfWeek(bool bEnable)
{
	m_wndChkDaySun.EnableWindow( bEnable );
	m_wndChkDayMon.EnableWindow( bEnable );
	m_wndChkDayTues.EnableWindow( bEnable );
	m_wndChkDayWed.EnableWindow( bEnable );
	m_wndChkDayThu.EnableWindow( bEnable );
	m_wndChkDayFri.EnableWindow( bEnable );
	m_wndChkDaySat.EnableWindow( bEnable );
	m_wndBtnAllDays.EnableWindow( bEnable );
}
void CScheduleTaskDlg::OnBnClickedButtonAllDays()
{
	m_wndChkDaySun.SetCheck( true );
	m_wndChkDayMon.SetCheck( true );
	m_wndChkDayTues.SetCheck( true );
	m_wndChkDayWed.SetCheck( true );
	m_wndChkDayThu.SetCheck( true );
	m_wndChkDayFri.SetCheck( true );
	m_wndChkDaySat.SetCheck( true );
}
