//
// WizardInterfacePage.h
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

#if !defined(AFX_WIZARDINTERFACEPAGE_H__4289CC7D_9047_4D46_9244_D1DE8A1655BE__INCLUDED_)
#define AFX_WIZARDINTERFACEPAGE_H__4289CC7D_9047_4D46_9244_D1DE8A1655BE__INCLUDED_

#pragma once

#include "WizardSheet.h"


class CWizardInterfacePage : public CWizardPage
{
// Construction
public:
	CWizardInterfacePage();
	virtual ~CWizardInterfacePage();

	DECLARE_DYNCREATE(CWizardInterfacePage)


// Dialog Data
public:
	//{{AFX_DATA(CWizardInterfacePage)
	enum { IDD = IDD_WIZARD_INTERFACE };
	CStatic	m_wndDescription1;
	CStatic	m_wndDescription0;
	CButton	m_wndInterface1;
	CButton	m_wndInterface0;
	int		m_bExpert;
	BOOL	m_bSimpleDownloadBars;
	BOOL	m_bHandleTorrents;
	//}}AFX_DATA

// Overrides
public:
	//{{AFX_VIRTUAL(CWizardInterfacePage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CWizardInterfacePage)
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_WIZARDINTERFACEPAGE_H__4289CC7D_9047_4D46_9244_D1DE8A1655BE__INCLUDED_)
