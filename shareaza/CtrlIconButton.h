//
// CtrlIconButton.h
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

#if !defined(AFX_CTRLICONBUTTON_H__5E629D93_681A_4631_BD16_166C2E025871__INCLUDED_)
#define AFX_CTRLICONBUTTON_H__5E629D93_681A_4631_BD16_166C2E025871__INCLUDED_

#pragma once

class CIconButtonCtrl : public CWnd
{
	DECLARE_DYNAMIC(CIconButtonCtrl)

public:
	CIconButtonCtrl();

	void	SetText(LPCTSTR pszText);
	void	SetIcon(HICON hIcon, BOOL bMirrored = FALSE);
	void	SetCoolIcon(UINT nIconID, BOOL bMirrored = FALSE);
	void	SetIcon(UINT nIconID, BOOL bMirrored = FALSE);
	void	SetHandCursor(BOOL bCursor);

	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nControlID, DWORD dwStyle = 0);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	CImageList	m_pImageList;
	BOOL		m_bCapture;
	BOOL		m_bDown;
	BOOL		m_bCursor;

	BOOL	RemoveStyle();

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_CTRLICONBUTTON_H__5E629D93_681A_4631_BD16_166C2E025871__INCLUDED_)
