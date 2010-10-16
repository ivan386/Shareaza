//
// PageSettingsWeb.h
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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

// TODO: Add Settings.Web.Foxy option

class CWebSettingsPage : public CSettingsPage
{
// Construction
public:
	CWebSettingsPage();
	virtual ~CWebSettingsPage();

	DECLARE_DYNCREATE(CWebSettingsPage)

// Dialog Data
public:
	//{{AFX_DATA(CWebSettingsPage)
	enum { IDD = IDD_SETTINGS_WEB };
	CButton	m_wndExtRemove;
	CButton	m_wndExtAdd;
	CComboBox	m_wndExtensions;
	BOOL	m_bUriMagnet;
	BOOL	m_bUriGnutella;
	BOOL	m_bUriED2K;
	BOOL	m_bWebHook;
	BOOL	m_bUriPiolet;
	BOOL	m_bUriTorrent;
	BOOL	m_bUriDC;
	//}}AFX_DATA

// Overrides
public:
	//{{AFX_VIRTUAL(CWebSettingsPage)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CWebSettingsPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnEditChangeExtList();
	afx_msg void OnSelChangeExtList();
	afx_msg void OnExtAdd();
	afx_msg void OnExtRemove();
	afx_msg void OnWebHook();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};
