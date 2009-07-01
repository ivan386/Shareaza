//
// WndSettingsPage.h
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

class CSettingsSheet;


class CSettingsPage : public CDialog
{
	DECLARE_DYNAMIC(CSettingsPage)

public:
	CSettingsPage(UINT nIDTemplate, LPCTSTR pszCaption = NULL);
	virtual ~CSettingsPage();

	CToolTipCtrl	m_wndToolTip;
	CString			m_sCaption;
	BOOL			m_bGroup;

	BOOL			Create(CRect& rcPage, CWnd* pSheetWnd);
	BOOL			LoadDefaultCaption();
	CSettingsPage*	GetPage(CRuntimeClass* pClass) const;

	inline CSettingsSheet* GetSheet() const
	{
		return (CSettingsSheet*)GetParent();
	}

	inline LPCTSTR GetTemplateName() const
	{
		return m_lpszTemplateName;
	}

// Events
public:
	virtual void SetModified(BOOL bChanged = TRUE);
	virtual BOOL OnApply();
	virtual void OnReset();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	DECLARE_MESSAGE_MAP()
};

// CEditPath
// Same functionality as CEdit has but with ability to run specified
// file or folder on mouse double click.

class CEditPath : public CEdit
{
	DECLARE_DYNCREATE(CEditPath)

protected:
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
