//
// WndSystem.h
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

#if !defined(AFX_WNDSYSTEM_H__15622BB7_9266_465C_8902_0F494E1DCAA0__INCLUDED_)
#define AFX_WNDSYSTEM_H__15622BB7_9266_465C_8902_0F494E1DCAA0__INCLUDED_

#pragma once

#include "WndPanel.h"
#include "CtrlText.h"


class CSystemWnd : public CPanelWnd
{
// Construction
public:
	CSystemWnd();
	virtual ~CSystemWnd();

	DECLARE_SERIAL(CSystemWnd)

// Attributes
protected:
	CTextCtrl	m_wndText;

// Operations
public:
	void		Add(int nType, LPCTSTR pszText);
	void		Clear();
	void		ShowStartupText();

// Overrides
public:
	//{{AFX_VIRTUAL(CSystemWnd)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CSystemWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSystemClear();
	afx_msg void OnDestroy();
	afx_msg void OnUpdateSystemVerbose(CCmdUI* pCmdUI);
	afx_msg void OnSystemVerbose();
	afx_msg void OnUpdateSystemTimestamp(CCmdUI* pCmdUI);
	afx_msg void OnSystemTimestamp();
	afx_msg void OnSystemTest();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_WNDSYSTEM_H__15622BB7_9266_465C_8902_0F494E1DCAA0__INCLUDED_)
