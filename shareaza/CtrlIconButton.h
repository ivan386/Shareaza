//
// CtrlIconButton.h
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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

#if !defined(AFX_CTRLICONBUTTON_H__5E629D93_681A_4631_BD16_166C2E025871__INCLUDED_)
#define AFX_CTRLICONBUTTON_H__5E629D93_681A_4631_BD16_166C2E025871__INCLUDED_

#pragma once


class CIconButtonCtrl : public CWnd
{
// Construction
public:
	CIconButtonCtrl();
	virtual ~CIconButtonCtrl();

// Attributes
protected:
	CImageList	m_pImageList;
	BOOL		m_bCapture;
	BOOL		m_bDown;
	BOOL		m_bCursor;

// Operations
public:
	void	SetText(LPCTSTR pszText);
	void	SetIcon(UINT nIconID);
	void	SetIcon(HICON hIcon);
	void	SetHandCursor(BOOL bCursor);
protected:
	BOOL	RemoveStyle();

// Overrides
public:
	//{{AFX_VIRTUAL(CIconButtonCtrl)
	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nControlID);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CIconButtonCtrl)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnEnable(BOOL bEnable);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLICONBUTTON_H__5E629D93_681A_4631_BD16_166C2E025871__INCLUDED_)

