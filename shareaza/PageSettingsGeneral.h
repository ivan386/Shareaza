//
// PageSettingsGeneral.h
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

#pragma once

#include "WndSettingsPage.h"


class CGeneralSettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CGeneralSettingsPage)

public:
	CGeneralSettingsPage();
	virtual ~CGeneralSettingsPage();

	enum { IDD = IDD_SETTINGS_GENERAL };
	int		m_bRatesInBytes;
	BOOL	m_bExpandMatches;
	BOOL	m_bAutoConnect;
	int		m_nCloseMode;
	int		m_bTrayMinimise;
	BOOL	m_bSwitchToTransfers;
	BOOL	m_bExpandDownloads;
	BOOL	m_bNewWindow;
	BOOL	m_bStartup;
	BOOL	m_bPromptURLs;
	BOOL	m_bHideSearch;
	BOOL	m_bAdultFilter;
	CSpinButtonCtrl	m_wndTipSpin;
	CListCtrl	m_wndTips;
	CSliderCtrl	m_wndTipAlpha;
	CComboBox	m_wndCloseMode;
	CComboBox	m_wndTrayMinimise;
	DWORD	m_nTipDelay;

protected:
	void Add(LPCTSTR pszName, BOOL bState);

	virtual void OnOK();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	afx_msg void OnDropdownCloseMode();
	afx_msg void OnDropdownTrayMinimise();

	DECLARE_MESSAGE_MAP()
};
