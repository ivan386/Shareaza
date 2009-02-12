//
// WndChat.h
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

#if !defined(AFX_WNDCHAT_H__27BC4B5E_C182_404F_B17C_FD0B867A64F2__INCLUDED_)
#define AFX_WNDCHAT_H__27BC4B5E_C182_404F_B17C_FD0B867A64F2__INCLUDED_

#pragma once

#include "WndChild.h"

class CChatFrame;


class CChatWnd : public CChildWnd
{
// Construction
public:
	CChatWnd(CChatFrame* pFrame);
	virtual ~CChatWnd();

	DECLARE_DYNAMIC(CChatWnd)

// Attributes
public:
	CChatFrame*	m_pFrame;

// Operations
public:

// Overrides
public:
	//{{AFX_VIRTUAL(CChatWnd)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

	virtual void OnSkinChange();

// Implementation
protected:
	//{{AFX_MSG(CChatWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_WNDCHAT_H__27BC4B5E_C182_404F_B17C_FD0B867A64F2__INCLUDED_)
