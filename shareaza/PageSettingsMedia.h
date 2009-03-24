//
// PageSettingsMedia.h
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

#if !defined(AFX_PAGESETTINGSMEDIA_H__BAECC3D7_E23B_428F_8F6F_D53D6D3939FA__INCLUDED_)
#define AFX_PAGESETTINGSMEDIA_H__BAECC3D7_E23B_428F_8F6F_D53D6D3939FA__INCLUDED_

#pragma once

#include "WndSettingsPage.h"


class CMediaSettingsPage : public CSettingsPage
{
// Construction
public:
	CMediaSettingsPage();
	virtual ~CMediaSettingsPage();

	DECLARE_DYNCREATE(CMediaSettingsPage)

// Dialog Data
public:
	//{{AFX_DATA(CMediaSettingsPage)
	enum { IDD = IDD_SETTINGS_MEDIA };
	CButton		m_wndRemove;
	CButton		m_wndAdd;
	CComboBox	m_wndList;
	CComboBox	m_wndServices;
	CString		m_sServicePath;
	CString		m_sType;
	BOOL		m_bEnablePlay;
	BOOL		m_bEnableEnqueue;
	//}}AFX_DATA

// Overrides
public:
	//{{AFX_VIRTUAL(CMediaSettingsPage)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void Update();

	//{{AFX_MSG(CMediaSettingsPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnMediaPlay();
	afx_msg void OnMediaEnqueue();
	afx_msg void OnSelChangeMediaTypes();
	afx_msg void OnEditChangeMediaTypes();
	afx_msg void OnSelChangeMediaService();
	afx_msg void OnMediaAdd();
	afx_msg void OnMediaRemove();
	afx_msg void OnMediaVis();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGESETTINGSMEDIA_H__BAECC3D7_E23B_428F_8F6F_D53D6D3939FA__INCLUDED_)
