//
// PageSettingsLibrary.h
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

#if !defined(AFX_PAGESETTINGSLIBRARY_H__7A9C2316_0CF6_4251_9BD2_C56B9DA5AD89__INCLUDED_)
#define AFX_PAGESETTINGSLIBRARY_H__7A9C2316_0CF6_4251_9BD2_C56B9DA5AD89__INCLUDED_

#pragma once

#include "WndSettingsPage.h"
#include "CtrlIconButton.h"


class CLibrarySettingsPage : public CSettingsPage
{
// Construction
public:
	CLibrarySettingsPage();
	virtual ~CLibrarySettingsPage();

	DECLARE_DYNCREATE(CLibrarySettingsPage)

// Dialog Data
public:
	//{{AFX_DATA(CLibrarySettingsPage)
	enum { IDD = IDD_SETTINGS_LIBRARY };
	CSpinButtonCtrl	m_wndRecentTotal;
	CSpinButtonCtrl	m_wndRecentDays;
	CButton	m_wndSafeRemove;
	CButton	m_wndSafeAdd;
	CComboBox	m_wndSafeList;
	CButton	m_wndPrivateRemove;
	CButton	m_wndPrivateAdd;
	CComboBox	m_wndPrivateList;
	BOOL	m_bSourceMesh;
	BOOL	m_bWatchFolders;
	DWORD	m_nRecentDays;
	int		m_nRecentTotal;
	BOOL	m_bStoreViews;
	BOOL	m_bSchemaTypes;
	BOOL	m_bHashIntegrity;
	BOOL	m_bBrowseFiles;
	CIconButtonCtrl	m_wndCollectionPath;
	CString	m_sCollectionPath;
	//}}AFX_DATA

// Overrides
public:
	//{{AFX_VIRTUAL(CLibrarySettingsPage)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CLibrarySettingsPage)
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
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGESETTINGSLIBRARY_H__7A9C2316_0CF6_4251_9BD2_C56B9DA5AD89__INCLUDED_)
