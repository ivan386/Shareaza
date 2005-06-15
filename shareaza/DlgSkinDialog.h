//
// DlgSkinDialog.h
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

#if !defined(AFX_DLGSKINDIALOG_H__A87746E3_AE87_49DA_A466_3F8C64364750__INCLUDED_)
#define AFX_DLGSKINDIALOG_H__A87746E3_AE87_49DA_A466_3F8C64364750__INCLUDED_

#pragma once

class CSkinWindow;


class CSkinDialog : public CDialog
{
// Construction
public:
	CSkinDialog(UINT nResID = 0, CWnd* pParent = NULL);

	DECLARE_DYNAMIC(CSkinDialog)

// Dialog Data
public:
	//{{AFX_DATA(CSkinDialog)
	//}}AFX_DATA

// Skin Support
public:
	BOOL	SkinMe(LPCTSTR pszSkin = NULL, UINT nIcon = 0, BOOL bLanguage = TRUE);
	BOOL	SelectCaption(CWnd* pWnd, int nIndex);
protected:
	CSkinWindow*	m_pSkin;

// Overrides
public:
	//{{AFX_VIRTUAL(CSkinDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CSkinDialog)
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnNcPaint();
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LONG OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGSKINDIALOG_H__A87746E3_AE87_49DA_A466_3F8C64364750__INCLUDED_)
