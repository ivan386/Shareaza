//
// CtrlMonitorBar.h
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

#pragma once

class CGraphItem;


class CMonitorBarCtrl : public CControlBar
{
// Construction
public:
	CMonitorBarCtrl();
	virtual ~CMonitorBarCtrl();

// Operations
public:
	BOOL	Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID);
	void	OnSkinChange();

// Attributes
public:
	CControlBar*	m_pSnapBar[2];
	CGraphItem*		m_pTxItem;
	CGraphItem*		m_pRxItem;
	DWORD			m_nMaximum;
	DWORD			m_nCount;
protected:
	CBitmap			m_bmWatermark;
	CRect			m_rcTrack;
	CRect			m_rcTab;
	BOOL			m_bTab;
	HICON			m_hTab;
	HICON			m_hUpDown;

// Overrides
protected:
	virtual CSize	CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual void	OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHandler*/) {};
	virtual INT_PTR	OnToolHitTest(CPoint /*point*/, TOOLINFO* /*pTI*/) const { return -1; }
	virtual void	DoPaint(CDC* pDC);
protected:
	void			PaintHistory(CDC* pDC, CRect* prc);
	void			PaintCurrent(CDC* pDC, CRect* prc, CGraphItem* pItem);
	void			PaintTab(CDC* pDC);

// Message Map
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
