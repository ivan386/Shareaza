//
// PageSettingsRich.h
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

#if !defined(AFX_PAGESETTINGSRICH_H__747373E3_A634_4552_8AEB_8E9D275282E8__INCLUDED_)
#define AFX_PAGESETTINGSRICH_H__747373E3_A634_4552_8AEB_8E9D275282E8__INCLUDED_

#pragma once

#include "WndSettingsPage.h"
#include "RichViewCtrl.h"


class CRichSettingsPage : public CSettingsPage
{
// Construction
public:
	CRichSettingsPage(LPCTSTR pszName = NULL);
	virtual ~CRichSettingsPage();

	DECLARE_DYNCREATE(CRichSettingsPage)

// Dialog Data
public:
	//{{AFX_DATA(CRichSettingsPage)
	enum { IDD = IDD_SETTINGS_RICH };
	//}}AFX_DATA

// Attributes
public:
	CString			m_sName;
	CString			m_sCaption;
	CRichViewCtrl	m_wndView;
	CRichDocument*	m_pDocument;

// Overrides
public:
	//{{AFX_VIRTUAL(CRichSettingsPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CRichSettingsPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	afx_msg void OnClickView(NMHDR* pNotify, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#define IDC_RICH_VIEW	100

#endif // !defined(AFX_PAGESETTINGSRICH_H__747373E3_A634_4552_8AEB_8E9D275282E8__INCLUDED_)
