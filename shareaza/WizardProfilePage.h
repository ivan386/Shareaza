//
// WizardProfilePage.h
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

#if !defined(AFX_WIZARDPROFILEPAGE_H__D22752BC_5296_49BC_94F2_EC87CC176D48__INCLUDED_)
#define AFX_WIZARDPROFILEPAGE_H__D22752BC_5296_49BC_94F2_EC87CC176D48__INCLUDED_

#pragma once

#include "WizardSheet.h"

class CWorldGPS;


class CWizardProfilePage : public CWizardPage
{
// Construction
public:
	CWizardProfilePage();
	virtual ~CWizardProfilePage();

	DECLARE_DYNCREATE(CWizardProfilePage)

// Dialog Data
public:
	//{{AFX_DATA(CWizardProfilePage)
	enum { IDD = IDD_WIZARD_PROFILE };
	CString m_sNick;
	CComboBox	m_wndCity;
	CComboBox	m_wndCountry;
	CString	m_sLocCity;
	CString	m_sLocCountry;
	CComboBox m_wndAge;
	int m_nAge;
	int m_nGender;
	//}}AFX_DATA

	CWorldGPS*	m_pWorld;

// Overrides
public:
	//{{AFX_VIRTUAL(CWizardProfilePage)
	public:
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CWizardProfilePage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangeCountry();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_WIZARDPROFILEPAGE_H__D22752BC_5296_49BC_94F2_EC87CC176D48__INCLUDED_)
