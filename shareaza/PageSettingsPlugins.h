//
// PageSettingsPlugins.h
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

#if !defined(AFX_PAGESETTINGSPLUGINS_H__F89745A8_9CC2_497C_B9E2_6F8C90C206D4__INCLUDED_)
#define AFX_PAGESETTINGSPLUGINS_H__F89745A8_9CC2_497C_B9E2_6F8C90C206D4__INCLUDED_

#pragma once

#include "WndSettingsPage.h"


class CPluginsSettingsPage : public CSettingsPage
{
// Construction
public:
	CPluginsSettingsPage();
	virtual ~CPluginsSettingsPage();

	DECLARE_DYNCREATE(CPluginsSettingsPage)

// Dialog Data
public:
	//{{AFX_DATA(CPluginsSettingsPage)
	enum { IDD = IDD_SETTINGS_PLUGINS };
	CButton	m_wndSetup;
	CEdit	m_wndDesc;
	CStatic	m_wndName;
	CListCtrl	m_wndList;
	//}}AFX_DATA

protected:
	CImageList	m_gdiImageList;
	BOOL		m_bRunning;
protected:
	void		InsertPlugin(LPCTSTR pszCLSID, LPCTSTR pszName, int nImage, TRISTATE bEnabled, LPVOID pPlugin = NULL);
	void		EnumerateGenericPlugins();
	void		EnumerateMiscPlugins();
	void		EnumerateMiscPlugins(LPCTSTR pszType, HKEY hRoot);
	void		AddMiscPlugin(LPCTSTR pszType, LPCTSTR pszCLSID);

// Overrides
public:
	//{{AFX_VIRTUAL(CPluginsSettingsPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CPluginsSettingsPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnItemChangingPlugins(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnItemChangedPlugins(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomDrawPlugins(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnPluginsSetup();
	afx_msg void OnPluginsWeb();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGESETTINGSPLUGINS_H__F89745A8_9CC2_497C_B9E2_6F8C90C206D4__INCLUDED_)
