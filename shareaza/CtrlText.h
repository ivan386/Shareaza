//
// CtrlText.h
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

#if !defined(AFX_CTRLTEXT_H__ED51405E_BF72_48AF_9E0F_61E36C25FC12__INCLUDED_)
#define AFX_CTRLTEXT_H__ED51405E_BF72_48AF_9E0F_61E36C25FC12__INCLUDED_

#pragma once


class CTextCtrl : public CWnd
{
// Construction
public:
	CTextCtrl();
	virtual ~CTextCtrl();

// Attributes
protected:
	CPtrArray			m_pLines;
	int					m_nPosition;
	int					m_nTotal;
	CSize				m_cCharacter;
	CFont				m_pFont;
	COLORREF			m_crBackground;
	COLORREF			m_crText[5];
	BOOL				m_bProcess;
	CCriticalSection	m_pSection;
	UINT				m_nScrollWheelLines; // number of lines to scroll when the mouse wheel is rotated

// Operations
public:
	void	Add(int nType, LPCTSTR pszText);
	void	AddLine(int nType, LPCTSTR pszLine);
	void	Clear(BOOL bInvalidate = TRUE);
	CFont*	GetFont();
protected:
	void	UpdateScroll(BOOL bFull = FALSE);

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
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CTextLine
{
// Construction
public:
	CTextLine(int nType, LPCTSTR pszLine);
	virtual ~CTextLine();

// Attributes
public:
	CString	m_sText;
	int*	m_pLine;
	int		m_nLine;
	int		m_nType;

// Operations
public:
	int		Process(int nWidth);
	void	Paint(CDC* pDC, CRect* pRect);
protected:
	void	AddLine(int nLength);

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLTEXT_H__ED51405E_BF72_48AF_9E0F_61E36C25FC12__INCLUDED_)