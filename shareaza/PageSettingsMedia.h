//
// PageSettingsMedia.h
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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


class CMediaSettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CMediaSettingsPage)

public:
	CMediaSettingsPage();
	virtual ~CMediaSettingsPage();

	enum { IDD = IDD_SETTINGS_MEDIA };

	CButton		m_wndRemove;
	CButton		m_wndAdd;
	CComboBox	m_wndList;
	CComboBox	m_wndServices;
	CString		m_sType;
	BOOL		m_bEnablePlay;
	BOOL		m_bEnableEnqueue;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual BOOL OnInitDialog();

	void Update();

	afx_msg void OnMediaPlay();
	afx_msg void OnMediaEnqueue();
	afx_msg void OnSelChangeMediaTypes();
	afx_msg void OnEditChangeMediaTypes();
	afx_msg void OnSelChangeMediaService();
	afx_msg void OnMediaAdd();
	afx_msg void OnMediaRemove();
	afx_msg void OnMediaVis();
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()
};
