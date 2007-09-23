//
// PageSettingsDownloads.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#if !defined(AFX_PAGESETTINGSDOWNLOADS_H__07DD1C53_0E05_459B_812F_CE9FCA0BA808__INCLUDED_)
#define AFX_PAGESETTINGSDOWNLOADS_H__07DD1C53_0E05_459B_812F_CE9FCA0BA808__INCLUDED_

#pragma once

#include "WndSettingsPage.h"
#include "CtrlIconButton.h"
#include "afxcmn.h"


class CDownloadsSettingsPage : public CSettingsPage
{
// Construction
public:
	CDownloadsSettingsPage();
	virtual ~CDownloadsSettingsPage();

	DECLARE_DYNCREATE(CDownloadsSettingsPage)

// Dialog Data
public:
	//{{AFX_DATA(CDownloadsSettingsPage)
	enum { IDD = IDD_SETTINGS_DOWNLOADS };
	CSpinButtonCtrl	m_wndMaxDownTransfers;
	CSpinButtonCtrl	m_wndMaxFileTransfers;
	CSpinButtonCtrl	m_wndMaxDownFiles;
	CIconButtonCtrl	m_wndIncompletePath;
	CIconButtonCtrl	m_wndDownloadsPath;
	CComboBox		m_wndBandwidthLimit;
	CComboBox		m_wndQueueLimit;
	CString			m_sDownloadsPath;
	CString			m_sIncompletePath;
	int				m_nMaxDownFiles;
	int				m_nMaxFileTransfers;
	int				m_nMaxDownTransfers;
	CString			m_sBandwidthLimit;
	CString			m_sQueueLimit;
	BOOL			m_bRequireConnect;
	//}}AFX_DATA

	BOOL m_bDownloadsChanged;

// Overrides
public:
	//{{AFX_VIRTUAL(CDownloadsSettingsPage)
	public:
	virtual void OnOK();
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void CalculateMaxValues();
	bool IsLimited(CString& strText) const;
	//{{AFX_MSG(CDownloadsSettingsPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnDownloadsBrowse();
	afx_msg void OnIncompleteBrowse();
	//}}AFX_MSG
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGESETTINGSDOWNLOADS_H__07DD1C53_0E05_459B_812F_CE9FCA0BA808__INCLUDED_)
