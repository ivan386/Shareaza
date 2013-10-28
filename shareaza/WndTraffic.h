//
// WndTraffic.h
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

#if !defined(AFX_WNDTRAFFIC_H__55ECA7E2_A2AC_406C_9042_5C0EA494E783__INCLUDED_)
#define AFX_WNDTRAFFIC_H__55ECA7E2_A2AC_406C_9042_5C0EA494E783__INCLUDED_

#pragma once

#include "WndChild.h"

class CGraphBase;


class CTrafficWnd : public CChildWnd
{
// Construction
public:
	CTrafficWnd(DWORD nUnique = 0);
	virtual ~CTrafficWnd();

	DECLARE_SERIAL(CTrafficWnd)

// Attributes
public:
	DWORD		m_nUnique;
	CString		m_sName;
	CGraphBase*	m_pGraph;

// Operations
protected:
	void	FindFreeUnique();
	BOOL	Serialize(BOOL bSave);
	void	SetUpdateRate();
	void	UpdateCaption();

// Overrides
public:
	//{{AFX_VIRTUAL(CTrafficWnd)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CTrafficWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnUpdateTrafficGrid(CCmdUI* pCmdUI);
	afx_msg void OnTrafficGrid();
	afx_msg void OnUpdateTrafficAxis(CCmdUI* pCmdUI);
	afx_msg void OnTrafficAxis();
	afx_msg void OnUpdateTrafficLegend(CCmdUI* pCmdUI);
	afx_msg void OnTrafficLegend();
	afx_msg void OnTrafficSetup();
	afx_msg void OnTrafficClear();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnTrafficWindow();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_WNDTRAFFIC_H__55ECA7E2_A2AC_406C_9042_5C0EA494E783__INCLUDED_)
