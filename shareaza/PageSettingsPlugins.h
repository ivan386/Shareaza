//
// PageSettingsPlugins.h
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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

class CPlugin;

class CPluginsSettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CPluginsSettingsPage)

public:
	CPluginsSettingsPage();

	enum { IDD = IDD_SETTINGS_PLUGINS };

	void		UpdateList();

protected:
	CButton		m_wndSetup;
	CEdit		m_wndDesc;
	CStatic		m_wndName;
	CListCtrl	m_wndList;
	CImageList	m_gdiImageList;
	BOOL		m_bRunning;

	void		InsertPlugin(LPCTSTR pszCLSID, LPCTSTR pszName, CPlugin* pPlugin = NULL, LPCTSTR pszExtension = NULL);
	void		EnumerateGenericPlugins();
	void		EnumerateMiscPlugins();
	void		EnumerateMiscPlugins(LPCTSTR pszType, HKEY hRoot);
	void		AddMiscPlugin(LPCTSTR pszType, LPCTSTR pszCLSID, LPCTSTR pszExtension = NULL);
	CString		GetPluginComments(LPCTSTR pszCLSID) const;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual BOOL OnInitDialog();

	afx_msg void OnItemChangingPlugins(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnItemChangedPlugins(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkPlugins(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnPluginsSetup();
	afx_msg void OnPluginsWeb();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};
