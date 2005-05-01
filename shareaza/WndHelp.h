//
// WndHelp.h
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

#if !defined(AFX_WNDHELP_H__B195445C_9FD3_4753_837E_AA048DC907FD__INCLUDED_)
#define AFX_WNDHELP_H__B195445C_9FD3_4753_837E_AA048DC907FD__INCLUDED_

#pragma once

#include "WndPanel.h"
#include "CtrlCoolBar.h"
#include "CtrlTaskPanel.h"
#include "RichViewCtrl.h"


class CHelpWnd : public CPanelWnd
{
// Construction
public:
	CHelpWnd();
	virtual ~CHelpWnd();

	DECLARE_SERIAL(CHelpWnd)

// Attributes
public:
	CCoolBarCtrl	m_wndToolBar;
	CTaskPanel		m_wndPanel;
	CRichViewCtrl	m_wndView;

// Operations
public:
	virtual void OnSkinChange();

// Overrides
public:
	//{{AFX_VIRTUAL(CHelpWnd)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CHelpWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_WNDHELP_H__B195445C_9FD3_4753_837E_AA048DC907FD__INCLUDED_)
