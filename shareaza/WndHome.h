//
// WndHome.h
//
// Copyright © Shareaza Development Team, 2002-2009.
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

#if !defined(AFX_WNDHOME_H__9BECD60C_B412_4687_B4AE_9086BAA8EC34__INCLUDED_)
#define AFX_WNDHOME_H__9BECD60C_B412_4687_B4AE_9086BAA8EC34__INCLUDED_

#pragma once

#include "WndPanel.h"
#include "CtrlHomeView.h"
#include "CtrlHomePanel.h"

class CRichDocument;
class CRichElement;


class CHomeWnd : public CPanelWnd
{
// Construction
public:
	CHomeWnd();

	DECLARE_DYNCREATE(CHomeWnd)

// Attributes
public:
	CHomeViewCtrl	m_wndView;
	CHomePanel		m_wndPanel;

// Operations
public:
	virtual void	OnSkinChange();

// Overrides
public:
	//{{AFX_VIRTUAL(CHomeWnd)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CHomeWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnClickView(NMHDR* pNotify, LRESULT *pResult);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_WNDHOME_H__9BECD60C_B412_4687_B4AE_9086BAA8EC34__INCLUDED_)
