//
// WizardFinishedPage.h
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

#if !defined(AFX_WIZARDFINISHEDPAGE_H__9DA437A4_6925_4E97_86B7_0D6C43ED01E4__INCLUDED_)
#define AFX_WIZARDFINISHEDPAGE_H__9DA437A4_6925_4E97_86B7_0D6C43ED01E4__INCLUDED_

#pragma once

#include "WizardSheet.h"


class CWizardFinishedPage : public CWizardPage
{
// Construction
public:
	CWizardFinishedPage();
	virtual ~CWizardFinishedPage();

	DECLARE_DYNCREATE(CWizardFinishedPage)

// Dialog Data
public:
	//{{AFX_DATA(CWizardFinishedPage)
	enum { IDD = IDD_WIZARD_FINISHED };
	BOOL	m_bAutoConnect;
	BOOL	m_bConnect;
	BOOL	m_bStartup;
	//}}AFX_DATA

// Overrides
public:
	//{{AFX_VIRTUAL(CWizardFinishedPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnWizardFinish();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CWizardFinishedPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_WIZARDFINISHEDPAGE_H__9DA437A4_6925_4E97_86B7_0D6C43ED01E4__INCLUDED_)
