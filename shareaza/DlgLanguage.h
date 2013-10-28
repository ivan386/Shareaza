//
// DlgLanguage.h
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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

#include "DlgSkinDialog.h"


class CLanguageDlg : public CSkinDialog
{
public:
	CLanguageDlg(CWnd* pParent = NULL);

	enum { IDD = IDD_LANGUAGE };

	CString			m_sLanguage;
	bool			m_bLanguageRTL;

protected:
	CArray< CString >	m_pPaths;
	CArray< CString >	m_pTitles;
	CArray< CString >	m_pGUIDirs;
	CArray< CString >	m_pLangCodes;
	CImageList		m_pImages;
	int				m_nHover;
	int				m_nDown;
	BOOL			m_bKeyMode;
	CFont			m_fntNormal;
	CFont			m_fntBold;
	CFont			m_fntSmall;
	HCURSOR			m_hArrow;
	HCURSOR			m_hHand;
	int				m_nRows;

	void	PaintItem(int nItem, CDC* pDC, CRect* pRect);
	void	AddEnglishDefault();
	void	Enumerate(LPCTSTR pszPath = NULL);
	BOOL	AddSkin(LPCTSTR pszPath, LPCTSTR pszName);
	void	Execute(int nItem);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnDestroy();
	afx_msg void OnClose();

	DECLARE_MESSAGE_MAP()
};
