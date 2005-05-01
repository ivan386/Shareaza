//
// WndPanel.h
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

#if !defined(AFX_WNDPANEL_H__AB493D6A_88D2_4F4D_BFF0_A61E2A829AD4__INCLUDED_)
#define AFX_WNDPANEL_H__AB493D6A_88D2_4F4D_BFF0_A61E2A829AD4__INCLUDED_

#pragma once

#include "WndChild.h"


class CPanelWnd : public CChildWnd
{
// Construction
public:
	CPanelWnd(BOOL bTabMode = FALSE, BOOL bGroupMode = FALSE);
	virtual ~CPanelWnd();

	DECLARE_DYNCREATE(CPanelWnd)

// Attributes
protected:
	CRect	m_rcClose;
	BOOL	m_bPanelClose;

// Operations
protected:
	void	PaintCaption(CDC& dc);
	void	PanelSizeLoop();

// Overrides
public:
	//{{AFX_VIRTUAL(CPanelWnd)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CPanelWnd)
	afx_msg void OnNcPaint();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	afx_msg LONG OnSetText(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_WNDPANEL_H__AB493D6A_88D2_4F4D_BFF0_A61E2A829AD4__INCLUDED_)
