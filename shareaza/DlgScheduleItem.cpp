//
// DlgScheduleItem.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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
#include "DlgScheduleItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CScheduleItemDlg, CSkinDialog)
	//{{AFX_MSG_MAP(CScheduleItemDlg)
	//}}AFX_MSG_MAP
ON_BN_CLICKED(IDC_ONLYONCE, OnBnClickedOnlyonce)
ON_BN_CLICKED(IDC_SCHEDULER_TOGGLE_BANDWIDTH, OnBnClickedToggleBandwidth)
ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATE, OnDtnDatetimechangeDate)
ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIME, OnDtnDatetimechangeTime)
ON_BN_CLICKED(IDC_EVERYDAY, OnBnClickedEveryday)
ON_BN_CLICKED(IDC_ACTIVE, OnBnClickedActive)
ON_CBN_SELCHANGE(IDC_EVENTTYPE, OnCbnSelchangeEventtype)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSecureRuleDlg dialog

CScheduleItemDlg::CScheduleItemDlg(CWnd* pParent, CScheduleItem* pSchItem) : CSkinDialog(CScheduleItemDlg::IDD, pParent)
{

	//{{AFX_DATA_INIT(CScheduleItemDlg)

	//}}AFX_DATA_INIT
	m_pScheduleItem = pSchItem;
	if(m_pScheduleItem) m_bNew = FALSE;

}

CScheduleItemDlg::~CScheduleItemDlg()
{
	//If we are creating a new schedule item and it is created, it should be already
	//added to scheduler list so we delete dialog's object 
	if (  m_bNew && m_pScheduleItem)
		delete m_pScheduleItem;
}

void CScheduleItemDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScheduleItemDlg)

	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_DATE, m_wndDate);
	DDX_Control(pDX, IDC_TIME, m_wndTime);
	DDX_Text(pDX, IDC_DESCRIPTION, m_sDescription);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_SPIN, m_wndSpin);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_SPIN_DOWN, m_wndSpinDown);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_SPIN_UP, m_wndSpinUp);
	DDX_Check(pDX, IDC_SCHEDULER_TOGGLE_BANDWIDTH, m_bToggleBandwidth);
	DDX_Check(pDX, IDC_SCHEDULER_LIMITED_NETWORKS, m_bLimitedNetworks);
	DDX_Text(pDX, IDC_SCHEDULER_LIMITED, m_nLimited);
	DDX_Text(pDX, IDC_SCHEDULER_LIMITED_DOWN, m_nLimitedDown);
	DDX_Text(pDX, IDC_SCHEDULER_LIMITED_UP, m_nLimitedUp);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED, m_pwndLimitedEdit);
	DDX_Control(pDX, IDC_EVENTTYPE, m_pwndTypeSel);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_DOWN, m_pwndLimitedEditDown);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_UP, m_pwndLimitedEditUp);
	DDX_Control(pDX, IDC_SCHEDULER_LIMITED_NETWORKS, m_pwndLimitedCheck);
	DDX_Control(pDX, IDC_SCHEDULER_TOGGLE_BANDWIDTH, m_pwndLimitedCheckTgl);
	DDX_Control(pDX, IDC_STATIC_LIMITED, m_pwndLimitedStatic);
	DDX_Control(pDX, IDC_STATIC_LIMITED_DOWN, m_pwndLimitedStaticDown);
	DDX_Control(pDX, IDC_STATIC_LIMITED_UP, m_pwndLimitedStaticUp);
	DDX_Control(pDX, IDC_ACTIVE, m_pwndActiveCheck);
	DDX_Control(pDX, IDC_EVERYDAY, m_pwndRadioEveryDay);
	DDX_Control(pDX, IDC_ONLYONCE, m_pwndRadioOnce);
	
}

/////////////////////////////////////////////////////////////////////////////
// CSecureRuleDlg message handlers

BOOL CScheduleItemDlg::OnInitDialog() 
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CScheduleItemDlg"), IDR_SCHEDULERFRAME );


	m_pwndActiveCheck.SetCheck(1);

	m_wndSpin.SetRange( 5, 95 );
	m_wndSpinDown.SetRange( 5, 95 );
	m_wndSpinUp.SetRange( 5, 95 );

	//TODO: New tasks should be added to the buttom of the list with the same order it is
	//added to scheduler enum
	CString sTemp;
	sTemp.LoadStringW(IDS_SCHEDULER_BANDWIDTH_FULLSPEED);
	m_pwndTypeSel.AddString(sTemp);
	
	sTemp.LoadStringW(IDS_SCHEDULER_BANDWIDTH_REDUCEDSPEED);
	m_pwndTypeSel.AddString(sTemp);
	
	sTemp.LoadStringW(IDS_SCHEDULER_BANDWIDTH_STOP);
	m_pwndTypeSel.AddString(sTemp);

	sTemp.LoadStringW(IDS_SCHEDULER_SYSTEM_DIALUP_DC);
	m_pwndTypeSel.AddString(sTemp);

	sTemp.LoadStringW(IDS_SCHEDULER_SYSTEM_EXIT);
	m_pwndTypeSel.AddString(sTemp);

	sTemp.LoadStringW(IDS_SCHEDULER_SYSTEM_TURNOFF);
	m_pwndTypeSel.AddString(sTemp);

	if (m_bNew)	//We are creating new schedule item, setting default values
	{
		m_pScheduleItem  = new CScheduleItem ();
		m_bEveryDay = false;
		m_nAction = SCHEDULE_FULL_SPEED;
		m_sDescription = "";
		m_bActive = true;
		m_nLimited			= 50;
		m_nLimitedDown		= 50;
		m_nLimitedUp		= 25;
		m_bToggleBandwidth	= false;
		m_bLimitedNetworks	= false;
		m_wndDate.GetTime(m_tDateAndTime);	//Sets mtDateTime to now
		m_pwndRadioOnce.SetCheck(1);
		m_pwndRadioEveryDay.SetCheck(0);
		m_pwndLimitedEdit.EnableWindow(false);
		m_pwndLimitedEditDown.EnableWindow(false);
		m_pwndLimitedEditUp.EnableWindow(false);
		m_wndSpin.EnableWindow(false);
		m_wndSpinDown.EnableWindow(false);
		m_wndSpinUp.EnableWindow(false);
		m_pwndLimitedCheckTgl.EnableWindow(false);
		m_pwndLimitedCheck.EnableWindow(false);
		m_pwndLimitedStatic.EnableWindow(false);
		m_pwndLimitedStaticDown.EnableWindow(false);
		m_pwndLimitedStaticUp.EnableWindow(false);
	}
	else	//We are editing an existing schedule item, getting values from it
	{
		m_bEveryDay		= m_pScheduleItem->m_bEveryDay;
		m_nAction		= m_pScheduleItem->m_nAction;
		m_sDescription	= m_pScheduleItem->m_sDescription;
		m_tDateAndTime	= m_pScheduleItem->m_tScheduleDateTime;
		m_bActive		= m_pScheduleItem->m_bActive;
		m_nLimited		= m_pScheduleItem->m_nLimited;
		m_nLimitedDown	= m_pScheduleItem->m_nLimitedDown;
		m_nLimitedUp	= m_pScheduleItem->m_nLimitedUp;
		
		m_bToggleBandwidth	= m_pScheduleItem->m_bToggleBandwidth;
		m_bLimitedNetworks	= m_pScheduleItem->m_bLimitedNetworks;
		
		m_pwndTypeSel.SetCurSel(m_nAction);
		m_pwndRadioOnce.SetCheck(!m_bEveryDay);
		m_pwndRadioEveryDay.SetCheck(m_bEveryDay);
		m_wndDate.SetTime(&m_tDateAndTime);
		m_wndTime.SetTime(&m_tDateAndTime);
		
		//If task is scheduled for everyday disable date window
		if (m_pwndRadioEveryDay.GetCheck())
			m_wndDate.EnableWindow(false);
	
		if (m_pwndTypeSel.GetCurSel() == SCHEDULE_LIMITED_SPEED)
		{
			m_pwndLimitedCheckTgl.EnableWindow(true);
			m_pwndLimitedCheck.EnableWindow(true);
		}
		else
		{
			m_pwndLimitedEdit.EnableWindow(false);
			m_pwndLimitedEditDown.EnableWindow(false);
			m_pwndLimitedEditUp.EnableWindow(false);
			m_wndSpin.EnableWindow(false);
			m_wndSpinDown.EnableWindow(false);
			m_wndSpinUp.EnableWindow(false);
			m_pwndLimitedCheckTgl.EnableWindow(false);
			m_pwndLimitedCheck.EnableWindow(false);

		}
	}

	m_wndSpin.SetPos(m_nLimited);
	m_wndSpinDown.SetPos(m_nLimitedDown);
	m_wndSpinUp.SetPos(m_nLimitedUp);

	UpdateData(FALSE);
	if (m_pwndTypeSel.GetCurSel() == SCHEDULE_LIMITED_SPEED) OnBnClickedToggleBandwidth();
	return FALSE;
}

void CScheduleItemDlg::OnOK() 
{
	UpdateData(TRUE);
	
	if (!(m_pwndTypeSel.GetCurSel() + 1))
	{
		AfxMessageBox(IDS_SCHEDULER_SELECTTASK,MB_OK);
		return;
	}
	else
		m_nAction = m_pwndTypeSel.GetCurSel();


	if (m_pwndRadioOnce.GetCheck())
	{
		CTime tNow = CTime::GetCurrentTime();

		if (Scheduler.ScheduledTimePassed(&tNow,&m_tDateAndTime))
		{
			AfxMessageBox(IDS_SCHEDULER_TIME_PASSED,MB_OK);
			return;
		}
	}

	if (m_pwndActiveCheck.GetCheck())
		m_bActive = true;
	else
		m_bActive = false;

	if (m_pwndRadioOnce.GetCheck())
		m_bEveryDay = false;
	else
		m_bEveryDay = true;


	m_pScheduleItem->m_nLimited			= m_nLimited;
	m_pScheduleItem->m_nLimitedDown		= m_nLimitedDown;
	m_pScheduleItem->m_nLimitedUp		= m_nLimitedUp;
	m_pScheduleItem->m_bToggleBandwidth	= m_bToggleBandwidth;
	m_pScheduleItem->m_bLimitedNetworks	= m_bLimitedNetworks;

	
	m_pScheduleItem->m_tScheduleDateTime = m_tDateAndTime;
	m_pScheduleItem->m_bEveryDay		= m_bEveryDay;
	m_pScheduleItem->m_nAction			= m_nAction;
	m_pScheduleItem->m_sDescription		= m_sDescription;
	m_pScheduleItem->m_bActive			= m_bActive;
	m_wndDate.GetWindowText(m_pScheduleItem->m_sDate);
	m_wndTime.GetWindowText(m_pScheduleItem->m_sTime);
	m_pScheduleItem->m_bExecuted = false;

	Scheduler.Add(m_pScheduleItem);
	m_pScheduleItem = NULL;


	CSkinDialog::OnOK();
}

void CScheduleItemDlg::OnBnClickedOnlyonce()
{
	m_pwndRadioEveryDay.SetCheck(0);
	m_bEveryDay = false;

	m_wndDate.EnableWindow(true);
}

void CScheduleItemDlg::OnDtnDatetimechangeDate(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	//LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	SYSTEMTIME tDate;
	SYSTEMTIME tTime;
	m_wndDate.GetTime(&tDate);
	m_wndTime.GetTime(&tTime);
	CTime tTemp(tDate.wYear, tDate.wMonth, tDate.wDay, tTime.wHour, tTime.wMinute, tTime.wSecond);
	m_tDateAndTime = tTemp;
	*pResult = 0;
}

void CScheduleItemDlg::OnDtnDatetimechangeTime(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	//LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);

	SYSTEMTIME tDate;
	SYSTEMTIME tTime;
	m_wndDate.GetTime(&tDate);
	m_wndTime.GetTime(&tTime);
	CTime tTemp(tDate.wYear, tDate.wMonth, tDate.wDay, tTime.wHour, tTime.wMinute, tTime.wSecond);
	m_tDateAndTime = tTemp;

	*pResult = 0;
}

void CScheduleItemDlg::OnBnClickedEveryday()
{
	m_pwndRadioOnce.SetCheck(0);

	m_wndDate.EnableWindow(false);
}

void CScheduleItemDlg::OnBnClickedToggleBandwidth()
{
	if(m_pwndLimitedCheckTgl.GetCheck())
	{
			m_pwndLimitedEdit.EnableWindow(false);
			m_pwndLimitedEditDown.EnableWindow(true);
			m_pwndLimitedEditUp.EnableWindow(true);

			m_wndSpin.EnableWindow(false);
			m_wndSpinDown.EnableWindow(true);
			m_wndSpinUp.EnableWindow(true);

			m_pwndLimitedStatic.EnableWindow(false);
			m_pwndLimitedStaticDown.EnableWindow(true);
			m_pwndLimitedStaticUp.EnableWindow(true);
	}
	else
	{
			m_pwndLimitedEdit.EnableWindow(true);
			m_pwndLimitedEditDown.EnableWindow(false);
			m_pwndLimitedEditUp.EnableWindow(false);

			m_wndSpin.EnableWindow(true);
			m_wndSpinDown.EnableWindow(false);
			m_wndSpinUp.EnableWindow(false);

			m_pwndLimitedStatic.EnableWindow(true);
			m_pwndLimitedStaticDown.EnableWindow(false);
			m_pwndLimitedStaticUp.EnableWindow(false);
	}
}

void CScheduleItemDlg::OnBnClickedActive()
{
	if (!m_pwndActiveCheck.GetCheck())
	{
		m_pwndActiveCheck.SetCheck(0);
	}
	else
	{
		m_pwndActiveCheck.SetCheck(1);
	}
}

void CScheduleItemDlg::OnCbnSelchangeEventtype()
{
	if (m_pwndTypeSel.GetCurSel() == 1)
		{
			if (!m_pwndLimitedCheckTgl.GetCheck())
			{
				m_pwndLimitedEdit.EnableWindow(true);
				m_wndSpin.EnableWindow(true);
			}
			else
			{
				m_pwndLimitedEditDown.EnableWindow(true);
				m_pwndLimitedEditUp.EnableWindow(true);
				m_wndSpinDown.EnableWindow(true);
				m_wndSpinUp.EnableWindow(true);				
			}
			m_pwndLimitedCheckTgl.EnableWindow(true);
			m_pwndLimitedCheck.EnableWindow(true);
			m_pwndLimitedStatic.EnableWindow(true);
			m_pwndLimitedStaticDown.EnableWindow(true);
			m_pwndLimitedStaticUp.EnableWindow(true);
		}
	else
	{
		m_pwndLimitedEdit.EnableWindow(false);
		m_pwndLimitedEditDown.EnableWindow(false);
		m_pwndLimitedEditUp.EnableWindow(false);
		m_wndSpin.EnableWindow(false);
		m_wndSpinDown.EnableWindow(false);
		m_wndSpinUp.EnableWindow(false);
		m_pwndLimitedCheckTgl.EnableWindow(false);
		m_pwndLimitedCheck.EnableWindow(false);
		m_pwndLimitedStatic.EnableWindow(false);
		m_pwndLimitedStaticDown.EnableWindow(false);
		m_pwndLimitedStaticUp.EnableWindow(false);
	}

}
