//
// DlgFilePropertiesSheet.h
//
// Copyright © Shareaza Development Team, 2002-2009.
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

#if !defined(AFX_DLGFILEPROPERTIESSHEET_H__FE20E80E_A1AA_438B_95B5_CC2F05412743__INCLUDED_)
#define AFX_DLGFILEPROPERTIESSHEET_H__FE20E80E_A1AA_438B_95B5_CC2F05412743__INCLUDED_

#pragma once

#include "LibraryList.h"

class CSkinWindow;


class CFilePropertiesSheet : public CPropertySheet
{
// Construction
public:
	CFilePropertiesSheet(CLibraryListItem oObject = CLibraryListItem());
	virtual ~CFilePropertiesSheet();

	DECLARE_DYNAMIC(CFilePropertiesSheet)

// Attributes
public:
	CLibraryList	m_pList;
protected:
	CSkinWindow*	m_pSkin;
	CBrush			m_brDialog;
	CString			m_sGeneralTitle;
	CString			m_sMetadataTitle;
	CString			m_sCommentsTitle;
	CString			m_sSharingTitle;
	CString			m_sSourcesTitle;

// Operations
public:
	void	Add(CLibraryListItem oObject);
	void	Add(CLibraryList* pList);

// Overrides
public:
	//{{AFX_VIRTUAL(CFilePropertiesSheet)
	public:
	virtual INT_PTR DoModal(int nPage = -1);
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SetTabTitle(CPropertyPage* pPage, CString& strTitle);

	//{{AFX_MSG(CFilePropertiesSheet)
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg ONNCHITTESTRESULT OnNcHitTest(CPoint point);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnNcPaint();
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGFILEPROPERTIESSHEET_H__FE20E80E_A1AA_438B_95B5_CC2F05412743__INCLUDED_)
