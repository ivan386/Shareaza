//
// WndSettingsSheet.h
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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
#include "CtrlIconButton.h"

class CSettingsPage;


class CSettingsSheet : public CSkinDialog
{
	DECLARE_DYNAMIC(CSettingsSheet)

public:
	CSettingsSheet(CWnd* pParent = NULL, UINT nCaptionID = 0);
	virtual ~CSettingsSheet();


	virtual BOOL SkinMe(LPCTSTR pszSkin = NULL, UINT nIcon = 0, BOOL bLanguage = TRUE);

// Attributes
protected:
	CArray< CSettingsPage* > m_pPages;
	CSettingsPage*	m_pPage;
	CSettingsPage*	m_pFirst;
	CSize			m_szPages;
protected:
	CTreeCtrl		m_wndTree;
	CButton			m_wndOK;
	CButton			m_wndCancel;
	CButton			m_wndApply;
	BOOL			m_bModified;
protected:
	CString			m_sCaption;
	int				m_nListWidth;
	int				m_nListMargin;
	int				m_nButtonWidth;
	int				m_nButtonHeight;

// Operations
public:
	void			AddPage(CSettingsPage* pPage, LPCTSTR pszCaption = NULL);
	void			AddGroup(CSettingsPage* pPage, LPCTSTR pszCaption = NULL);
	CSettingsPage*	GetPage(INT_PTR nPage) const;
	CSettingsPage*	GetPage(CRuntimeClass* pClass) const;
	CSettingsPage*	GetPage(LPCTSTR pszClass) const;
	INT_PTR			GetPageIndex(CSettingsPage* pPage) const;
	INT_PTR			GetPageCount() const;
	CSettingsPage*	GetActivePage() const;
	BOOL			SetActivePage(CSettingsPage* pPage);
	BOOL			IsModified() const;
	void			SetModified(BOOL bChanged = TRUE);
	INT_PTR			DoModal();
protected:
	void			BuildTree();
	BOOL			CreatePage(const CRect& rc, CSettingsPage* pPage);
	virtual void	DoPaint(CDC& dc);

// Overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnApply();
	afx_msg void OnTreeExpanding(NMHDR* pNotify, LRESULT *pResult);
	afx_msg void OnSelectPage(NMHDR* pNotify, LRESULT *pResult);

};

#define IDC_SETTINGS_TREE	100
