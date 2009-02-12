//
// PageSettingsProtocols.h
//
// Copyright © Shareaza Development Team, 2002-2009.
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

#if !defined(AFX_PAGESETTINGSPROTOCOLS_H__A8D2F964_2179_4E0D_BAF2_04ABE55BFA5A__INCLUDED_)
#define AFX_PAGESETTINGSPROTOCOLS_H__A8D2F964_2179_4E0D_BAF2_04ABE55BFA5A__INCLUDED_

#pragma once

#include "WndSettingsPage.h"


class CProtocolsSettingsPage : public CSettingsPage
{
// Construction
public:
	CProtocolsSettingsPage();
	virtual ~CProtocolsSettingsPage();

	DECLARE_DYNCREATE(CProtocolsSettingsPage)

// Dialog Data
public:
	//{{AFX_DATA(CProtocolsSettingsPage)
	enum { IDD = IDD_SETTINGS_PROTOCOLS };
	CTreeCtrl	m_wndTree;
	//}}AFX_DATA

	HTREEITEM	AddItem(HTREEITEM hParent, LPCTSTR pszText, LPCTSTR pszValue = NULL);

// Overrides
public:
	//{{AFX_VIRTUAL(CProtocolsSettingsPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CProtocolsSettingsPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGESETTINGSPROTOCOLS_H__A8D2F964_2179_4E0D_BAF2_04ABE55BFA5A__INCLUDED_)
