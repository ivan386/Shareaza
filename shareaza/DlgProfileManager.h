//
// DlgProfileManager.h
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

#if !defined(AFX_DLGPROFILEMANAGER_H__0F4D4873_886C_4D77_8979_A8F546643384__INCLUDED_)
#define AFX_DLGPROFILEMANAGER_H__0F4D4873_886C_4D77_8979_A8F546643384__INCLUDED_

#pragma once

#include "WndSettingsSheet.h"


class CProfileManagerDlg : public CSettingsSheet
{
// Construction
public:
	CProfileManagerDlg(CWnd* pParent = NULL);
	virtual ~CProfileManagerDlg();

	DECLARE_DYNAMIC(CProfileManagerDlg)

// Attributes
protected:
	CBitmap	m_bmHeader;

// Operations
public:
	static BOOL		Run(LPCTSTR pszWindow = NULL);
	INT_PTR			DoModal(LPCTSTR pszWindow = NULL);
protected:
	void			AddPage(CSettingsPage* pPage);
	void			AddGroup(CSettingsPage* pPage);
	virtual void	DoPaint(CDC& dc);

// Overrides
public:
	//{{AFX_VIRTUAL(CProfileManagerDlg)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CProfileManagerDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	virtual void OnOK();
	virtual void OnApply();

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGPROFILEMANAGER_H__0F4D4873_886C_4D77_8979_A8F546643384__INCLUDED_)
