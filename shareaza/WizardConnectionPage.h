//
// WizardConnectionPage.h
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

#if !defined(AFX_WIZARDCONNECTIONPAGE_H__0DAE1F77_BC93_4B54_88A2_67AFDAA1305F__INCLUDED_)
#define AFX_WIZARDCONNECTIONPAGE_H__0DAE1F77_BC93_4B54_88A2_67AFDAA1305F__INCLUDED_

#pragma once

#include "WizardSheet.h"


class CWizardConnectionPage : public CWizardPage
{
// Construction
public:
	CWizardConnectionPage();
	virtual ~CWizardConnectionPage();

	DECLARE_DYNCREATE(CWizardConnectionPage)

// Dialog Data
public:
	//{{AFX_DATA(CWizardConnectionPage)
	enum { IDD = IDD_WIZARD_CONNECTION };
	CComboBox	m_wndLanSelect;
	CStatic	m_wndLanLabel;
	CComboBox	m_wndHomeSelect;
	CStatic	m_wndHomeLabel;
	CComboBox	m_wndGroup;
	CComboBox	m_wndSpeed;
	CComboBox	m_wndType;
	//}}AFX_DATA

// Overrides
public:
	//{{AFX_VIRTUAL(CWizardConnectionPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CWizardConnectionPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangeConnectionType();
	afx_msg void OnEditChangeConnectionSpeed();
	afx_msg void OnSelChangeConnectionSpeed();
	afx_msg void OnSelChangeConnectionGroup();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_WIZARDCONNECTIONPAGE_H__0DAE1F77_BC93_4B54_88A2_67AFDAA1305F__INCLUDED_)
