//
// PageProfileContact.h
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

#if !defined(AFX_PAGEPROFILECONTACT_H__461EA38B_A8A0_4AF7_8D66_500AD26AF76A__INCLUDED_)
#define AFX_PAGEPROFILECONTACT_H__461EA38B_A8A0_4AF7_8D66_500AD26AF76A__INCLUDED_

#pragma once

#include "WndSettingsPage.h"


class CContactProfilePage : public CSettingsPage
{
// Construction
public:
	CContactProfilePage();
	virtual ~CContactProfilePage();

	DECLARE_DYNCREATE(CContactProfilePage)

// Dialog Data
public:
	//{{AFX_DATA(CContactProfilePage)
	enum { IDD = IDD_PROFILE_CONTACT };
	CString	m_sEmail;
	CString	m_sAOL;
	CString	m_sICQ;
	CString	m_sYahoo;
	CString	m_sMSN;
	CString m_sJabber;
	//}}AFX_DATA

	void	AddAddress(LPCTSTR pszClass, LPCTSTR pszName, LPCTSTR pszAddress);

// Overrides
public:
	//{{AFX_VIRTUAL(CContactProfilePage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CContactProfilePage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGEPROFILECONTACT_H__461EA38B_A8A0_4AF7_8D66_500AD26AF76A__INCLUDED_)
