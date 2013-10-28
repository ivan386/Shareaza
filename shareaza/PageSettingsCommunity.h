//
// PageSettingsCommunity.h
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

#if !defined(AFX_PAGESETTINGSCOMMUNITY_H__9DE0B4CC_656F_4552_B4F4_E6E80893BD3E__INCLUDED_)
#define AFX_PAGESETTINGSCOMMUNITY_H__9DE0B4CC_656F_4552_B4F4_E6E80893BD3E__INCLUDED_

#pragma once

#include "WndSettingsPage.h"


class CCommunitySettingsPage : public CSettingsPage
{
// Construction
public:
	CCommunitySettingsPage();
	virtual ~CCommunitySettingsPage();

	DECLARE_DYNCREATE(CCommunitySettingsPage)

// Dialog Data
public:
	//{{AFX_DATA(CCommunitySettingsPage)
	enum { IDD = IDD_SETTINGS_COMMUNITY };
	BOOL	m_bChatEnable;
	BOOL	m_bChatAllNetworks;
	BOOL	m_bChatFilter;
	BOOL	m_bChatCensor;
	//}}AFX_DATA

// Overrides
public:
	//{{AFX_VIRTUAL(CCommunitySettingsPage)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CCommunitySettingsPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnEditProfile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGESETTINGSCOMMUNITY_H__9DE0B4CC_656F_4552_B4F4_E6E80893BD3E__INCLUDED_)
