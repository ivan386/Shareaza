//
// WndSearchPad.h
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

#if !defined(AFX_WNDSEARCHPAD_H__16971510_B95B_4B55_BA3B_AD7F47D8C9D1__INCLUDED_)
#define AFX_WNDSEARCHPAD_H__16971510_B95B_4B55_BA3B_AD7F47D8C9D1__INCLUDED_

#pragma once

#include "WndPanel.h"
#include "CtrlSchema.h"
#include "CtrlSchemaCombo.h"
#include "CtrlIconButton.h"

class CQuerySearch;


class CSearchPadWnd : public CPanelWnd
{
// Construction
public:
	CSearchPadWnd();
	virtual ~CSearchPadWnd();

	DECLARE_DYNCREATE(CSearchPadWnd)

// Attributes
protected:
	CEdit			m_wndText;
	CSchemaCombo	m_wndSchema;
	CSchemaCtrl		m_wndData;
	CIconButtonCtrl	m_wndSearch;
	CFont			m_pFont;

// Operations
public:
	virtual void OnSkinChange();
protected:
	CQuerySearch*	GetSearch();
	void			ClearSearch();

// Overrides
public:
	//{{AFX_VIRTUAL(CSearchPadWnd)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CSearchPadWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSelChangeSchemas();
	afx_msg void OnSearchCreate();
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#define IDC_SEARCH_TEXT		121
#define IDC_SEARCH_SCHEMAS	122
#define IDC_SEARCH_DATA		123
#define IDC_SEARCH_CREATE	124

#endif // !defined(AFX_WNDSEARCHPAD_H__16971510_B95B_4B55_BA3B_AD7F47D8C9D1__INCLUDED_)
