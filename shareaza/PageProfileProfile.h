//
// PageProfileProfile.h
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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

#if !defined(AFX_PAGEPROFILEPROFILE_H__A45BF28A_6652_470F_9E69_C1EBD900F148__INCLUDED_)
#define AFX_PAGEPROFILEPROFILE_H__A45BF28A_6652_470F_9E69_C1EBD900F148__INCLUDED_

#pragma once

#include "WndSettingsPage.h"

class CWorldGPS;


class CProfileProfilePage : public CSettingsPage
{
// Construction
public:
	CProfileProfilePage();
	virtual ~CProfileProfilePage();

	DECLARE_DYNCREATE(CProfileProfilePage)

// Dialog Data
public:
	//{{AFX_DATA(CProfileProfilePage)
	enum { IDD = IDD_PROFILE_PROFILE };
	CButton	m_wndInterestRemove;
	CButton	m_wndInterestAdd;
	CComboBox	m_wndInterestAll;
	CListBox	m_wndInterestList;
	CComboBox	m_wndAge;
	CComboBox	m_wndCity;
	CComboBox	m_wndCountry;
	CString	m_sLocCity;
	CString	m_sLocCountry;
	CString	m_sLocLatitude;
	CString	m_sLocLongitude;
	CString	m_sAge;
	CString	m_sGender;
	//}}AFX_DATA

	CWorldGPS*	m_pWorld;

// Overrides
public:
	//{{AFX_VIRTUAL(CProfileProfilePage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CProfileProfilePage)
	virtual BOOL OnInitDialog();
	afx_msg void OnCloseUpCountry();
	afx_msg void OnCloseUpCity();
	afx_msg void OnSelChangeInterestList();
	afx_msg void OnSelChangeInterestAll();
	afx_msg void OnEditChangeInterestAll();
	afx_msg void OnInterestAdd();
	afx_msg void OnInterestRemove();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGEPROFILEPROFILE_H__A45BF28A_6652_470F_9E69_C1EBD900F148__INCLUDED_)
