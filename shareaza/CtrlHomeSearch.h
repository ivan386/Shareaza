//
// CtrlHomeSearch.h
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

#if !defined(AFX_CTRLHOMESEARCH_H__40DC2FFE_0FF6_4A72_A7C5_74DC32305269__INCLUDED_)
#define AFX_CTRLHOMESEARCH_H__40DC2FFE_0FF6_4A72_A7C5_74DC32305269__INCLUDED_

#pragma once

#include "CtrlSchemaCombo.h"
#include "CtrlIconButton.h"


class CHomeSearchCtrl : public CWnd
{
// Construction
public:
	CHomeSearchCtrl();
	virtual ~CHomeSearchCtrl();

// Attributes
public:
	CComboBox		m_wndText;
	CSchemaCombo	m_wndSchema;
	CIconButtonCtrl	m_wndSearch;
	CIconButtonCtrl	m_wndAdvanced;
	COLORREF		m_crWindow;
private:
	CWnd*			m_pTextInput; // Holds pointer to window that receives text input

// Operations
public:
	void	Setup(COLORREF crWindow);
	void	FillHistory();

// Overrides
public:
	//{{AFX_VIRTUAL(CHomeSearchCtrl)
	public:
	virtual BOOL Create(CWnd* pParentWnd, UINT nID);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CHomeSearchCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnCloseUpText();
	afx_msg void OnSearchCreate();
	afx_msg void OnSearchAdvanced();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#define IDC_SEARCH_TEXT		121
#define IDC_SEARCH_SCHEMAS	122
#ifndef IDC_SEARCH_CREATE
#define IDC_SEARCH_CREATE	123
#endif
#define IDC_SEARCH_ADVANCED	124

#endif // !defined(AFX_CTRLHOMESEARCH_H__40DC2FFE_0FF6_4A72_A7C5_74DC32305269__INCLUDED_)
