//
// PageProfileFiles.h
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

#if !defined(AFX_PAGEPROFILEFILES_H__0AA47105_BB43_453D_9A80_8A27255593F7__INCLUDED_)
#define AFX_PAGEPROFILEFILES_H__0AA47105_BB43_453D_9A80_8A27255593F7__INCLUDED_

#pragma once

#include "WndSettingsPage.h"


class CFilesProfilePage : public CSettingsPage
{
// Construction
public:
	CFilesProfilePage();
	virtual ~CFilesProfilePage();

	DECLARE_DYNCREATE(CFilesProfilePage)

// Dialog Data
public:
	//{{AFX_DATA(CFilesProfilePage)
	enum { IDD = IDD_PROFILE_FILES };
	CListCtrl	m_wndList;
	//}}AFX_DATA

// Overrides
public:
	//{{AFX_VIRTUAL(CFilesProfilePage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CFilesProfilePage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGEPROFILEFILES_H__0AA47105_BB43_453D_9A80_8A27255593F7__INCLUDED_)
