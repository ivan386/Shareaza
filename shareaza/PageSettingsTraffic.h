//
// PageSettingsTraffic.h
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

#if !defined(AFX_PAGESETTINGSTRAFFIC_H__20363ACA_EC88_4DEB_A9C1_D8B4CE8A365E__INCLUDED_)
#define AFX_PAGESETTINGSTRAFFIC_H__20363ACA_EC88_4DEB_A9C1_D8B4CE8A365E__INCLUDED_

#pragma once

#include "WndSettingsPage.h"

class CSettingEdit;


class CAdvancedSettingsPage : public CSettingsPage
{
// Construction
public:
	CAdvancedSettingsPage();
	virtual ~CAdvancedSettingsPage();

	DECLARE_DYNCREATE(CAdvancedSettingsPage)

// Operations
public:
	void	AddSetting(LPVOID pValue, DWORD nScale = 1, DWORD nMin = 0, DWORD nMax = 0, LPCTSTR pszSuffix = NULL);
	void	UpdateItem(int nItem);
	
	class EditItem
	{
	// Construction
	public:
		EditItem(CSettings::Item* pItem, DWORD nScale = 1, DWORD nMin = 0, DWORD nMax = 0, LPCTSTR pszSuffix = NULL);
		virtual ~EditItem();
	
	// Attributes
	public:
		CSettings::Item* m_pItem;
	public:
		CString		m_sName;
		DWORD		m_nScale;
		DWORD		m_nMin;
		DWORD		m_nMax;
		CString		m_sSuffix;
		DWORD		m_nValue;
		
	// Operations
	public:
		void		Commit();

	};
	
// Dialog Data
public:
	//{{AFX_DATA(CAdvancedSettingsPage)
	enum { IDD = IDD_SETTINGS_ADVANCED };
	CSpinButtonCtrl	m_wndValueSpin;
	CEdit	m_wndValue;
	CListCtrl	m_wndList;
	//}}AFX_DATA

// Overrides
public:
	//{{AFX_VIRTUAL(CAdvancedSettingsPage)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAdvancedSettingsPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnItemChangedProperties(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeValue();
	afx_msg void OnColumnClickProperties(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGESETTINGSTRAFFIC_H__20363ACA_EC88_4DEB_A9C1_D8B4CE8A365E__INCLUDED_)
