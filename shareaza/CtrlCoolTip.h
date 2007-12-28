//
// CtrlCoolTip.h
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

#if !defined(AFX_CTRLCOOLTIP_H__9F287EE6_069E_4FBB_8657_533279376816__INCLUDED_)
#define AFX_CTRLCOOLTIP_H__9F287EE6_069E_4FBB_8657_533279376816__INCLUDED_

#pragma once

class CLineGraph;


class CCoolTipCtrl : public CWnd
{
// Construction
public:
	CCoolTipCtrl();
	virtual ~CCoolTipCtrl();

	DECLARE_DYNAMIC(CCoolTipCtrl)

// Attributes
protected:
	bool*	m_pbEnable;
	void*	m_pContext;
	HWND	m_hAltWnd;
	BOOL	m_bTimer;
	BOOL	m_bVisible;
	CPoint	m_pOpen;
	DWORD	m_tOpen;
	CSize	m_sz;
protected:
	static LPCTSTR	m_hClass;

// Operations
public:
	virtual void Show(void* pContext, HWND hAltWnd = NULL);
	virtual void Hide();
protected:
	virtual void ShowImpl();
	void	CalcSizeHelper();
	void	AddSize(CDC* pDC, LPCTSTR pszText, int nBase = 0);
	void	GetPaintRect(RECT* pRect);
	void	DrawText(CDC* pDC, POINT* pPoint, LPCTSTR pszText, int nBase = 0);
	void	DrawRule(CDC* pDC, POINT* pPoint, BOOL bPos = FALSE);
	BOOL	WindowFromPointBelongsToOwner(const CPoint& point);
protected:
	CLineGraph*	CreateLineGraph();
protected:
	virtual BOOL OnPrepare();
	virtual void OnCalcSize(CDC* pDC);
	virtual void OnShow();
	virtual void OnHide();
	virtual void OnPaint(CDC* pDC);

// Overrides
public:
	//{{AFX_VIRTUAL(CCoolTipCtrl)
	virtual BOOL Create(CWnd* pParentWnd, bool* pbEnable = NULL);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CCoolTipCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED		0x80000
#define LWA_ALPHA			0x02
#endif

#define TIP_TEXTHEIGHT	14
#define TIP_RULE		11
#define TIP_GAP			5

#endif // !defined(AFX_CTRLCOOLTIP_H__9F287EE6_069E_4FBB_8657_533279376816__INCLUDED_)
