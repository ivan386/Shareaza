//
// RichViewCtrl.h
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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

class CRichDocument;
class CRichElement;
class CRichFragment;

typedef struct
{
	int		nFragment;
	int		nOffset;
} RICHPOSITION;


class CRichViewCtrl : public CWnd
{
	DECLARE_DYNCREATE(CRichViewCtrl)

public:
	CRichViewCtrl();
	virtual ~CRichViewCtrl();

// Attributes
protected:
	CSyncObject*	m_pSyncRoot;
	BOOL			m_bSelectable;
	BOOL			m_bFollowBottom;
	BOOL			m_bDefaultLink;
protected:
	CRichDocument*	m_pDocument;
	DWORD			m_nCookie;
	CArray< CRichFragment* > m_pFragments;
	int				m_nLength;
	int				m_nScrollWheelLines;
protected:
	CRichElement*	m_pHover;
	BOOL			m_bSelecting;
	RICHPOSITION	m_pSelStart;
	RICHPOSITION	m_pSelEnd;
	RICHPOSITION	m_pSelAbsStart;
	RICHPOSITION	m_pSelAbsEnd;
protected:
	HCURSOR			m_hcHand;
	HCURSOR			m_hcText;
	CBrush			m_pBrush;

// Operations
public:
	void			SetSyncObject(CSyncObject* pSyncRoot);
	void			SetSelectable(BOOL bSelectable);
	void			SetFollowBottom(BOOL bFollowBottom);
	void			SetDefaultLink(BOOL bDefaultLink);
	void			SetDocument(CRichDocument* pDocument);
	BOOL			IsModified() const;
	void			InvalidateIfModified();
	int				FullHeightMove(int nX, int nY, int nWidth, BOOL bShow = FALSE);
	BOOL			GetElementRect(CRichElement* pElement, RECT* prc) const;
	CString			GetWordFromPoint(CPoint& point, LPCTSTR szTokens) const;
protected:
	void			ClearFragments();
	void			Layout(CDC* pDC, CRect* pRect);
	void			WrapLineHelper(CList< CRichFragment* >& pLine, CPoint& pt, int& nLineHeight, int nWidth, int nAlign);
	CRichFragment*	PointToFrag(CPoint& pt) const;
	RICHPOSITION	PointToPosition(CPoint& pt) const;
	CPoint			PositionToPoint(RICHPOSITION& pt) const;
	void			UpdateSelection();
	void			CopySelection() const;
protected:
	virtual void	OnLayoutComplete() {};
	virtual void	OnPaintBegin(CDC* /*pDC*/) {};
	virtual void	OnPaintComplete(CDC* /*pDC*/) {};
	virtual void	OnVScrolled() {};

// Overrides
public:
	//{{AFX_VIRTUAL(CRichViewCtrl)
	public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CRichViewCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	friend class CRichFragment;
};

typedef struct
{
	NMHDR			hdr;
	CRichElement*	pElement;
} RVN_ELEMENTEVENT;

#define RVN_CLICK		100
#define RVN_DBLCLICK	101
#define RVN_SETCURSOR	102
