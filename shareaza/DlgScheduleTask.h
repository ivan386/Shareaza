//
// DlgScheduleTask.h
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

#pragma once

#include "PagePropertyAdv.h"

//////////////////////////////////////////////////////////////////////
// CScheduleTaskDlg class: Schedule Item Dialog

class CScheduleTaskDlg : public CPropertySheetAdv
{
	DECLARE_DYNAMIC(CScheduleTaskDlg)

public:
	CScheduleTaskDlg(LPCTSTR szTaskName = NULL);

	virtual INT_PTR DoModal(int nPage = -1);

protected:
	CString			m_sTaskName;
	bool			m_bNew;
	CString			m_sActionTitle;
	HPROPSHEETPAGE	m_phPages[ 4 ];

#ifdef _DEBUG
	virtual void AssertValid() const;
#endif

	virtual BOOL OnInitDialog();
	virtual void BuildPropPageArray();

	afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
};


class CScheduleTaskPage : public CPropertyPageAdv
{
	DECLARE_DYNAMIC(CScheduleTaskPage)

public:
	CScheduleTaskPage();

	enum { IDD = IDD_SCHEDULE_TASK };

	int					m_nAction;
	int					m_nLimitDown;
	int					m_nLimitUp;
	BOOL				m_bG1;
	BOOL				m_bG2;
	BOOL				m_bED2K;
	BOOL				m_bDC;
	BOOL				m_bBT;

	HPROPSHEETPAGE Create(BOOL bWizard);

protected:
	CComboBox			m_wndAction;
	BOOL				m_bToggleBandwidth;
	CButton				m_wndToggleBandwidth;
	CEdit				m_wndLimitedEditDown;
	CSpinButtonCtrl		m_wndSpinDown;
	CEdit				m_wndLimitedEditUp;
	CSpinButtonCtrl		m_wndSpinUp;
	CButton				m_wndG1;
	CButton				m_wndG2;
	CButton				m_wndED2K;
	CButton				m_wndDC;
	CButton				m_wndBT;

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
	
	void Update();

	afx_msg void OnBnClickedSchedulerToggleBandwidth();
	afx_msg void OnEnChangeSchedulerLimitedDown();
	afx_msg void OnEnChangeSchedulerLimitedUp();
	afx_msg void OnCbnSelchangeEventtype();

	DECLARE_MESSAGE_MAP()
};
