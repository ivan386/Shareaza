//
// DlgTorrentInfoSheet.h
//
// Copyright (c) Shareaza Development Team, 2002-2006.
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

#if !defined(AFX_DLGTORRENTINFOSHEET_H__FE20E80E_A1AA_CC2F05412743__INCLUDED_)
#define AFX_DLGTORRENTINFOSHEET_H__FE20E80E_A1AA_CC2F05412743__INCLUDED_

#pragma once

class CSkinWindow;

#include "BTInfo.h"


class CTorrentInfoSheet : public CPropertySheet
{
// Construction
public:
	CTorrentInfoSheet(CBTInfo* pInfo, const Hashes::BtGuid& oPeerID);
	virtual ~CTorrentInfoSheet();

	DECLARE_DYNAMIC(CTorrentInfoSheet)

// Attributes
public:
	CBTInfo			m_pInfo;
	Hashes::BtGuid	m_pPeerID;
protected:
	CSkinWindow*	m_pSkin;
	CBrush			m_brDialog;
	CString			m_sGeneralTitle;
	CString			m_sFilesTitle;
	CString			m_sTrackersTitle;

// Overrides
public:
	//{{AFX_VIRTUAL(CTorrentInfoSheet)
	public:
	virtual INT_PTR DoModal(int nPage = -1);
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SetTabTitle(CPropertyPage* pPage, CString& strTitle);

	//{{AFX_MSG(CTorrentInfoSheet)
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

#endif // !defined(AFX_DLGTORRENTINFOSHEET_H__FE20E80E_A1AA_CC2F05412743__INCLUDED_)
