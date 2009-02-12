//
// PageSettingsConnection.h
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

#if !defined(AFX_PAGESETTINGSCONNECTION_H__0C9E9759_50EA_48EC_B7CC_E2B0C05B9B30__INCLUDED_)
#define AFX_PAGESETTINGSCONNECTION_H__0C9E9759_50EA_48EC_B7CC_E2B0C05B9B30__INCLUDED_

#pragma once

#include "WndSettingsPage.h"


class CConnectionSettingsPage : public CSettingsPage
{
// Construction
public:
	CConnectionSettingsPage();
	virtual ~CConnectionSettingsPage();

	DECLARE_DYNCREATE(CConnectionSettingsPage)

// Dialog Data
public:
	//{{AFX_DATA(CConnectionSettingsPage)
	enum { IDD = IDD_SETTINGS_CONNECTION };
	CEdit	m_wndInPort;
	CComboBox	m_wndInSpeed;
	CComboBox	m_wndOutSpeed;
	CComboBox	m_wndInHost;
	CComboBox	m_wndOutHost;
	CButton	m_wndInBind;
	CSpinButtonCtrl	m_wndTimeoutHandshake;
	CSpinButtonCtrl	m_wndTimeoutConnection;
	BOOL	m_bInBind;
	CString	m_sInHost;
	int		m_nInPort;
	CString	m_sOutHost;
	CComboBox	m_wndCanAccept;
	BOOL	m_bIgnoreLocalIP;
	BOOL	m_bEnableUPnP;
	DWORD	m_nTimeoutConnection;
	DWORD	m_nTimeoutHandshake;
	CString	m_sOutSpeed;
	CString	m_sInSpeed;
	BOOL	m_bInRandom;
	//}}AFX_DATA

// Overrides
public:
	//{{AFX_VIRTUAL(CConnectionSettingsPage)
	public:
	virtual void OnOK();
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString GetInOutHostTranslation();

	//{{AFX_MSG(CConnectionSettingsPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnEditChangeInboundHost();
	afx_msg void OnChangedInboundHost();
	afx_msg void OnChangeInboundPort();
	afx_msg void OnInboundRandom();
	afx_msg void OnClickedEnableUpnp();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGESETTINGSCONNECTION_H__0C9E9759_50EA_48EC_B7CC_E2B0C05B9B30__INCLUDED_)
