//
// PageSettingsUploads.h
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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

#pragma once

#include "WndSettingsPage.h"
#include "CtrlDragList.h"


class CUploadsSettingsPage : public CSettingsPage
{
// Construction
public:
	CUploadsSettingsPage();
	virtual ~CUploadsSettingsPage();

	DECLARE_DYNCREATE(CUploadsSettingsPage)

// Dialog Data
public:
	enum { IDD = IDD_SETTINGS_UPLOADS };
	CButton	m_wndQueueDelete;
	CButton	m_wndQueueEdit;
	CDragListCtrl	m_wndQueues;
	CButton	m_wndAgentRemove;
	CButton	m_wndAgentAdd;
	CComboBox	m_wndAgentList;
	CSpinButtonCtrl	m_wndMaxPerHost;
	BOOL	m_bSharePartials;
	DWORD	m_nMaxPerHost;
	BOOL	m_bHubUnshare;
	BOOL	m_bSharePreviews;
	CString	m_sBandwidth;
	int m_bThrottleMode;
	BOOL	m_bQueuesChanged;				//Have the queues been changed? (Rebuild hash table)
	
	void	UpdateQueues();

// Overrides
public:
	virtual void OnOK();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangeAgentList();
	afx_msg void OnEditChangeAgentList();
	afx_msg void OnAgentAdd();
	afx_msg void OnAgentRemove();
	afx_msg void OnItemChangedQueues(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnQueueNew();
	afx_msg void OnQueueEdit();
	afx_msg void OnQueueDelete();
	afx_msg void OnDblClkQueues(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnQueueDrop(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
