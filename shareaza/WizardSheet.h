//
// WizardSheet.h
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

#if !defined(AFX_WIZARDSHEET_H__E2500C10_98B2_48B0_9165_975E5444679A__INCLUDED_)
#define AFX_WIZARDSHEET_H__E2500C10_98B2_48B0_9165_975E5444679A__INCLUDED_

#pragma once

class CSkinWindow;


class CWizardSheet : public CPropertySheet
{
// Construction
public:
	CWizardSheet(CWnd *pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CWizardSheet();

// Attributes
public:
	CRect			m_rcPage;
	CBitmap			m_bmHeader;
	CSkinWindow*	m_pSkin;

// Operations
public:
	static BOOL	RunWizard(CWnd* pParent = NULL);

// Implementation
public:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg ONNCHITTESTRESULT OnNcHitTest(CPoint point);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnNcPaint();
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
};


class CWizardPage : public CPropertyPage
{
// Construction
public:
	CWizardPage(UINT nID = 0);
	virtual ~CWizardPage();

	DECLARE_DYNCREATE(CWizardPage)

// Attributes
public:
	COLORREF	m_crWhite;
	CBrush		m_brWhite;

// Operations
public:
	CWizardSheet*	GetSheet();
	void			SetWizardButtons(DWORD dwFlags);
	void			StaticReplace(LPCTSTR pszSearch, LPCTSTR pszReplace);
	BOOL			IsConnectionCapable();

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

};

#endif // !defined(AFX_WIZARDSHEET_H__E2500C10_98B2_48B0_9165_975E5444679A__INCLUDED_)
