//
// PageSettingsUploads.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

	CSpinButtonCtrl	m_wndMaxPerHost;
	CComboBox		m_wndAgentList;
	CButton			m_wndAgentAdd;
	CButton			m_wndAgentRemove;
	CComboBox		m_wndBandwidthLimit;
	CDragListCtrl	m_wndQueues;
	CButton			m_wndQueueDelete;
	CButton			m_wndQueueEdit;


	BOOL			m_bSharePartials;
	BOOL			m_bHubUnshare;
	BOOL			m_bSharePreviews;
	DWORD			m_nMaxPerHost;
	CString			m_sBandwidthLimit;
	BOOL			m_bThrottleMode;

//
public:
	BOOL			m_bQueuesChanged;		//Have the queues been changed? (Rebuild hash table)
	void			UpdateQueues();

// Overrides
public:
	virtual void OnOK();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DWORD	m_nOldUploads;

	bool IsLimited(CString& strText) const;

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

	DECLARE_MESSAGE_MAP()
};
