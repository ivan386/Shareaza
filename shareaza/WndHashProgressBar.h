//
// WndHashProgressBar.h
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

#if !defined(AFX_WNDHASHPROGRESSBAR_H__812FCDD7_1E84_4398_86C5_47A9E5E6253F__INCLUDED_)
#define AFX_WNDHASHPROGRESSBAR_H__812FCDD7_1E84_4398_86C5_47A9E5E6253F__INCLUDED_

#pragma once


class CHashProgressBar : public CWnd
{
// Construction
public:
	CHashProgressBar();
	virtual ~CHashProgressBar();

// Attributes
protected:
	CWnd*		m_pParent;
	COLORREF	m_crBorder;
	COLORREF	m_crFill;
	COLORREF	m_crText;
	CBrush		m_brFill;
	HICON		m_hIcon;
	int			m_nFlash;
	int			m_nRemaining;			// Files left to hash
	int			m_nTotal;				// Total files to hash
	CString		m_sCurrent;				// Name of file currently hashing
	CString		m_sPrevious;			// Name of last file currently hashed

// Operations
public:
	void	Create(CWnd* pParent);
	void	Run();
	void	Update();
protected:
	void	Show(int nWidth, BOOL bShow);

// Overrides
public:
	//{{AFX_VIRTUAL(CHashProgressBar)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CHashProgressBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_WNDHASHPROGRESSBAR_H__812FCDD7_1E84_4398_86C5_47A9E5E6253F__INCLUDED_)
