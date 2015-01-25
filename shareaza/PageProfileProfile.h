//
// PageProfileProfile.h
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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

class CWorldGPS;


class CProfileProfilePage : public CSettingsPage
{
	DECLARE_DYNCREATE(CProfileProfilePage)

public:
	CProfileProfilePage();
	virtual ~CProfileProfilePage();

	enum { IDD = IDD_PROFILE_PROFILE };

	CButton		m_wndInterestRemove;
	CButton		m_wndInterestAdd;
	CComboBox	m_wndInterestAll;
	CListBox	m_wndInterestList;
	CComboBox	m_wndAge;
	CComboBox	m_wndCity;
	CComboBoxEx	m_wndCountry;
	CImageList	m_gdiFlags;
	CString		m_sLocCity;
	CString		m_sLocCountry;
	CString		m_sLocLatitude;
	CString		m_sLocLongitude;
	CString		m_sAge;
	CString		m_sGender;
	CWorldGPS*	m_pWorld;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual BOOL OnInitDialog();

	afx_msg void OnSelChangeCountry();
	afx_msg void OnSelChangeCity();
	afx_msg void OnSelChangeInterestList();
	afx_msg void OnSelChangeInterestAll();
	afx_msg void OnEditChangeInterestAll();
	afx_msg void OnInterestAdd();
	afx_msg void OnInterestRemove();

	DECLARE_MESSAGE_MAP()
};
