//
// PageSettingsDownloads.h
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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
#include "CtrlIconButton.h"


class CDownloadsSettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CDownloadsSettingsPage)

public:
	CDownloadsSettingsPage();

	enum { IDD = IDD_SETTINGS_DOWNLOADS };

	virtual void OnOK();
	virtual BOOL OnKillActive();

protected:
	CSpinButtonCtrl	m_wndMaxDownTransfers;
	CSpinButtonCtrl	m_wndMaxFileTransfers;
	CSpinButtonCtrl	m_wndMaxDownFiles;
	CIconButtonCtrl	m_wndIncompletePath;
	CIconButtonCtrl	m_wndDownloadsPath;
	CComboBox		m_wndAntiVirus;
	CComboBox		m_wndBandwidthLimit;
	CComboBox		m_wndQueueLimit;
	int				m_nMaxDownFiles;
	int				m_nMaxFileTransfers;
	int				m_nMaxDownTransfers;
	CString			m_sBandwidthLimit;
	CString			m_sQueueLimit;
	BOOL			m_bRequireConnect;
	CComboBoxPath	m_wndDownloadsFolder;
	CComboBoxPath	m_wndIncompleteFolder;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	bool	IsLimited(CString& strText) const;

	afx_msg void OnDownloadsBrowse();
	afx_msg void OnIncompleteBrowse();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnDestroy();
	afx_msg void OnCbnDropdownAntivirus();

	DECLARE_MESSAGE_MAP()
};
