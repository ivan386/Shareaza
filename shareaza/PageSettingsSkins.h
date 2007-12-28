//
// PageSettingsSkins.h
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

#if !defined(AFX_PAGESETTINGSSKINS_H__EDD85F64_191E_4ADA_A0F5_639CD7F04F84__INCLUDED_)
#define AFX_PAGESETTINGSSKINS_H__EDD85F64_191E_4ADA_A0F5_639CD7F04F84__INCLUDED_

#pragma once

#include "WndSettingsPage.h"


class CSkinsSettingsPage : public CSettingsPage
{
// Construction
public:
	CSkinsSettingsPage();
	virtual ~CSkinsSettingsPage();

	DECLARE_DYNCREATE(CSkinsSettingsPage)

// Dialog Data
public:
	//{{AFX_DATA(CSkinsSettingsPage)
	enum { IDD = IDD_SETTINGS_SKINS };
	CButton	m_wndDelete;
	CEdit	m_wndDesc;
	CStatic	m_wndName;
	CStatic	m_wndAuthor;
	CListCtrl	m_wndList;
	//}}AFX_DATA

// Attributes
public:
	CImageList	m_gdiImageList;
	int			m_nSelected;

// Operations
public:
	void	EnumerateSkins(LPCTSTR pszPath = NULL);
	BOOL	AddSkin(LPCTSTR pszPath, LPCTSTR pszName);

// Overrides
public:
	//{{AFX_VIRTUAL(CSkinsSettingsPage)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CSkinsSettingsPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnItemChangedSkins(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSkinsBrowse();
	afx_msg void OnSkinsWeb();
	afx_msg void OnSkinsDelete();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGESETTINGSSKINS_H__EDD85F64_191E_4ADA_A0F5_639CD7F04F84__INCLUDED_)
