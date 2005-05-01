//
// PageSettingsBandwidth.h
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

#pragma once

#include "WndSettingsPage.h"


class CBandwidthSettingsPage : public CSettingsPage
{
// Construction
public:
	CBandwidthSettingsPage();
	virtual ~CBandwidthSettingsPage();

	DECLARE_DYNCREATE(CBandwidthSettingsPage)

// Dialog Data
public:
	enum { IDD = IDD_SETTINGS_BANDWIDTH };
	CStatic	m_wndHeadUP2;
	CStatic	m_wndHeadUP1;
	CStatic	m_wndHeadTransmit;
	CStatic	m_wndHeadReceive;
	CString	m_sUPInLimit;
	CString	m_sUPInMax;
	CString	m_sUPInTotal;
	CString	m_sUPOutLimit;
	CString	m_sUPOutMax;
	CString	m_sUPOutTotal;
	CString	m_sDLTotal;
	CString	m_sInTotal;
	CString	m_sOutTotal;
	CString	m_sPInTotal;
	CString	m_sPOutLimit;
	CString	m_sPOutMax;
	CString	m_sPOutTotal;
	CString	m_sULTotal;
	CString	m_sPInLimit;
	CString	m_sPInMax;
	CString	m_sLInLimit;
	CString	m_sLInMax;
	CString	m_sLInTotal;
	CString	m_sLOutLimit;
	CString	m_sLOutMax;
	CString	m_sLOutTotal;
	CString	m_sUDPTotal;

// Operations
public:
	void	Calculate(BOOL bForeward);
	void	Calculate(CString& strLimit, int nCount, CString& strCount, CString& strTotal, BOOL bForeward);
	DWORD	ToSpeed(CString& str);
	CString	ToString(DWORD nSpeed, BOOL bUnlimited = TRUE, BOOL bUnit = FALSE);
	DWORD	AddString(CString& str);
	void	SwapBytesBits(CString& str);

// Attributes
protected:
	BOOL	m_bActive;
	BOOL	m_bBytes;

// Overrides
public:
	virtual void OnOK();
	virtual BOOL OnSetActive();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

};
