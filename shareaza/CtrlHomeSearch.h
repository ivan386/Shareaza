//
// CtrlHomeSearch.h
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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

#include "CtrlSchemaCombo.h"
#include "CtrlIconButton.h"

class CHomeSearchCtrl : public CWnd
{
	DECLARE_DYNCREATE(CHomeSearchCtrl)

public:
	CHomeSearchCtrl();

	void	OnSkinChange(COLORREF crWindow);
	void	Activate();

	virtual BOOL Create(CWnd* pParentWnd, UINT nID);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	CComboBox		m_wndText;
	CSchemaCombo	m_wndSchema;
	CIconButtonCtrl	m_wndSearch;
	CIconButtonCtrl	m_wndAdvanced;
	COLORREF		m_crWindow;

	void	FillHistory();
	void	Search(bool bAutostart);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnCloseUpText();
	afx_msg void OnSelChangeText();
	afx_msg void OnSearchStart();
	afx_msg void OnSearchAdvanced();

	DECLARE_MESSAGE_MAP()
};
