//
// CtrlText.h
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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
	DECLARE_DYNCREATE(CTextCtrl)

public:
	CTextCtrl();
	virtual ~CTextCtrl();

	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	void	CopyText() const;
	void	Clear(BOOL bInvalidate = TRUE);
	void	Add(const CLogMessage* pMsg);

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

	void	AddLine(WORD nType, const CString& strLine);
	void	UpdateScroll(BOOL bFull = FALSE);
	int		HitTest(const CPoint& pt) const;

	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	DECLARE_MESSAGE_MAP()
};

class CTextLine
{
public:
	CTextLine(WORD nType, const CString& strText);
	~CTextLine();

	int		Process(int nWidth);
	void	Paint(CDC* pDC, CRect* pRect);

	CString	m_sText;
	int		m_nLine;
	WORD	m_nType;
	BOOL	m_bSelected;

protected:
	int*	m_pLine;

	void	AddLine(int nLength);
};
