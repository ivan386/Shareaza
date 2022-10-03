//
// CtrlTaskPanel.h
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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

class CTaskBox;


class CTaskPanel : public CWnd
{
// Construction
public:
	CTaskPanel();

	DECLARE_DYNAMIC(CTaskPanel)

// Attributes
protected:
	CList< CTaskBox* >	m_pBoxes;
	CTaskBox*	m_pStretch;
	int			m_nMargin;
	int			m_nCurve;
	CString		m_sWatermark;
	CString		m_sFooter;
	BOOL		m_bLayout;

// Operations
public:
	CTaskBox*	AddBox(CTaskBox* pBox, POSITION posBefore = NULL);
	POSITION	GetBoxIterator() const;
	CTaskBox*	GetNextBox(POSITION& pos) const;
	INT_PTR		GetBoxCount() const;
	void		RemoveBox(CTaskBox* pBox);
	void		ClearBoxes(BOOL bDelete);
public:
	void		SetStretchBox(CTaskBox* pBox);
	void		SetMargin(int nMargin, int nCurve = 2);
	void		SetWatermark(LPCTSTR szWatermark);
	void		SetFooter(LPCTSTR szWatermark);
	void		OnChanged();
protected:
	void		Layout(CRect& rcClient);

// Overrides
public:
	virtual BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Implementation
protected:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()

	friend class CTaskBox;
};


class CTaskBox : public CButton
{
public:
	CTaskBox();
	virtual ~CTaskBox();

	DECLARE_DYNAMIC(CTaskBox)

	CTaskPanel*	GetPanel() const;
	void		SetCaption(LPCTSTR pszCaption);
	void		SetIcon(HICON hIcon);
	void		SetSize(int nHeight);
	void		SetPrimary(BOOL bPrimary = TRUE);
	void		SetWatermark(LPCTSTR szWatermark);
	void		SetCaptionmark(LPCTSTR szWatermark);
	void		Expand(BOOL bOpen = TRUE);

protected:
	CTaskPanel*	m_pPanel;
	int			m_nHeight;
	BOOL		m_bVisible;
	BOOL		m_bOpen;
	BOOL		m_bHover;
	BOOL		m_bPrimary;
	HICON		m_hIcon;
	BOOL		m_bIconDel;
	CString		m_sWatermark;
	CString		m_sCaptionmark;

	int			GetOuterHeight() const;
	void		PaintBorders();
	void		InvalidateNonclient();
	virtual void OnExpanded(BOOL bOpen);

public:
	virtual BOOL Create(CTaskPanel* pPanel, int nHeight = 0, LPCTSTR pszCaption = NULL, UINT nIDIcon = 0, UINT nID = 0);
	virtual void DrawItem(LPDRAWITEMSTRUCT) {}

protected:
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnPaint();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);

	DECLARE_MESSAGE_MAP()

	friend class CTaskPanel;
};
