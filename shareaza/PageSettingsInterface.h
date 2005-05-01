//
// PageSettingsInterface.h
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

#if !defined(AFX_PAGESETTINGSINTERFACE_H__A666BAAF_D6A1_4E8E_8546_D175DBAE3DFC__INCLUDED_)
#define AFX_PAGESETTINGSINTERFACE_H__A666BAAF_D6A1_4E8E_8546_D175DBAE3DFC__INCLUDED_

#pragma once

#include "WndSettingsPage.h"


class CInterfaceSettingsPage : public CSettingsPage
{
// Construction
public:
	CInterfaceSettingsPage();
	virtual ~CInterfaceSettingsPage();

	DECLARE_DYNCREATE(CInterfaceSettingsPage)

// Dialog Data
public:
	//{{AFX_DATA(CInterfaceSettingsPage)
	enum { IDD = IDD_SETTINGS_INTERFACE };
	//}}AFX_DATA

// Operations
protected:

// Overrides
public:
	//{{AFX_VIRTUAL(CInterfaceSettingsPage)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CInterfaceSettingsPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGESETTINGSINTERFACE_H__A666BAAF_D6A1_4E8E_8546_D175DBAE3DFC__INCLUDED_)
