//
// DlgSettingsManager.h
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

#if !defined(AFX_DLGSETTINGSMANAGER_H__7E76D5DE_A0EA_4CA6_B05B_8F4470E2CF03__INCLUDED_)
#define AFX_DLGSETTINGSMANAGER_H__7E76D5DE_A0EA_4CA6_B05B_8F4470E2CF03__INCLUDED_

#pragma once

#include "WndSettingsSheet.h"


class CSettingsManagerDlg : public CSettingsSheet
{
// Construction
public:
	CSettingsManagerDlg(CWnd* pParent = NULL);

// Dialog Data
public:
	//{{AFX_DATA(CSettingsManagerDlg)
	//}}AFX_DATA
	//{{AFX_VIRTUAL(CSettingsManagerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

	CBitmap	m_bmHeader;

// Static Run
public:
	static BOOL Run(LPCTSTR pszWindow = NULL);
	static void OnSkinChange(BOOL bSet);
	static CSettingsManagerDlg* m_pThis;

// Operations
public:
	int		DoModal(LPCTSTR pszWindow = NULL);
protected:
	void			AddPage(CSettingsPage* pPage);
	void			AddGroup(CSettingsPage* pPage);
	virtual void	DoPaint(CDC& dc);

// Implementation
protected:
	//{{AFX_MSG(CSettingsManagerDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	virtual void OnOK();
	virtual void OnApply();

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGSETTINGSMANAGER_H__7E76D5DE_A0EA_4CA6_B05B_8F4470E2CF03__INCLUDED_)
