//
// CtrlText.h
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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

class CTextLine;

class CTextCtrl : public CWnd
{
// Construction
public:
	CTextCtrl();
	virtual ~CTextCtrl();

// Attributes
protected:
	CArray< CTextLine* > m_pLines;
	int					m_nPosition;
	int					m_nTotal;
	CSize				m_cCharacter;
	CFont				m_pFont;
	COLORREF			m_crBackground[4];
	COLORREF			m_crText[5];
	BOOL				m_bProcess;
	mutable CCriticalSection	m_pSection;
	UINT				m_nScrollWheelLines;	// number of lines to scroll when the mouse wheel is rotated
	int					m_nLastClicked;			// Index of last clicked item

// Operations
public:
	void	Add(WORD nType, const CString& strText);
	void	AddLine(WORD nType, const CString& strLine);
	void	Clear(BOOL bInvalidate = TRUE);
	CFont*	GetFont();
	void	CopyText() const;

protected:
	void	UpdateScroll(BOOL bFull = FALSE);
	int		HitTest(const CPoint& pt) const;

// Overrides
public:
	//{{AFX_VIRTUAL(CTextCtrl)
	public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CTextCtrl)
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CTextLine
{
// Construction
public:
	CTextLine(WORD nType, const CString& strText);
	virtual ~CTextLine();

// Attributes
public:
	CString	m_sText;
	int*	m_pLine;
	int		m_nLine;
	WORD	m_nType;
	BOOL	m_bSelected;

// Operations
public:
	int		Process(int nWidth);
	void	Paint(CDC* pDC, CRect* pRect);
protected:
	void	AddLine(int nLength);

};
