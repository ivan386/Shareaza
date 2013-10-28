//
// DlgSecureRule.h
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

#if !defined(AFX_DLGSECURERULE_H__1919626B_533C_474B_9CE3_933ECDEAA0F0__INCLUDED_)
#define AFX_DLGSECURERULE_H__1919626B_533C_474B_9CE3_933ECDEAA0F0__INCLUDED_

#pragma once

#include "DlgSkinDialog.h"

class CSecureRule;


class CSecureRuleDlg : public CSkinDialog
{
// Construction
public:
	CSecureRuleDlg(CWnd* pParent = NULL, CSecureRule* pRule = NULL);
	virtual ~CSecureRuleDlg();

// Dialog Data
public:
	//{{AFX_DATA(CSecureRuleDlg)
	enum { IDD = IDD_SECURE_RULE };
	CEdit	m_wndContent;
	CButton	m_wndGroupContent;
	CButton	m_wndGroupNetwork;
	CEdit	m_wndExpireM;
	CEdit	m_wndExpireH;
	CEdit	m_wndExpireD;
	CEdit	m_wndMask4;
	CEdit	m_wndMask3;
	CEdit	m_wndMask2;
	CEdit	m_wndMask1;
	CEdit	m_wndIP4;
	CEdit	m_wndIP3;
	CEdit	m_wndIP2;
	CEdit	m_wndIP1;
	int		m_nExpireD;
	int		m_nExpireH;
	int		m_nExpireM;
	int		m_nAction;
	int		m_nExpire;
	CString	m_sComment;
	int		m_nType;
	CString	m_sContent;
	int		m_nMatch;
	//}}AFX_DATA

	CSecureRule*	m_pRule;
	BOOL			m_bNew;

	void	ShowGroup(CWnd* pWnd, BOOL bShow);

// Overrides
public:
	//{{AFX_VIRTUAL(CSecureRuleDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CSecureRuleDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelChangeRuleExpire();
	afx_msg void OnSelChangeRuleType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGSECURERULE_H__1919626B_533C_474B_9CE3_933ECDEAA0F0__INCLUDED_)
