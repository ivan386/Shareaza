//
// PageSettingsLibrary.h
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


class CLibrarySettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CLibrarySettingsPage)

public:
	CLibrarySettingsPage();

	enum { IDD = IDD_SETTINGS_LIBRARY };

	virtual void OnOK();

protected:
	CSpinButtonCtrl	m_wndRecentTotal;
	CSpinButtonCtrl	m_wndRecentDays;
	CButton			m_wndSafeRemove;
	CButton			m_wndSafeAdd;
	CComboBox		m_wndSafeList;
	CButton			m_wndPrivateRemove;
	CButton			m_wndPrivateAdd;
	CComboBox		m_wndPrivateList;
	BOOL			m_bWatchFolders;
	DWORD			m_nRecentDays;
	int				m_nRecentTotal;
	BOOL			m_bStoreViews;
	BOOL			m_bHighPriorityHash;
	BOOL			m_bBrowseFiles;
	BOOL			m_bMakeGhosts;
	BOOL			m_bSmartSeries;
	CIconButtonCtrl	m_wndCollectionPath;
	CComboBoxPath	m_wndCollectionFolder;
	CButton			m_wndRecentClear;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	afx_msg void OnSelChangeSafeTypes();
	afx_msg void OnEditChangeSafeTypes();
	afx_msg void OnSafeAdd();
	afx_msg void OnSafeRemove();
	afx_msg void OnSelChangePrivateTypes();
	afx_msg void OnEditChangePrivateTypes();
	afx_msg void OnPrivateAdd();
	afx_msg void OnPrivateRemove();
	afx_msg void OnRecentClear();
	afx_msg void OnCollectionsBrowse();

	DECLARE_MESSAGE_MAP()
};
