//
// WizardSheet.h
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

#include "PagePropertyAdv.h"


class CWizardSheet : public CPropertySheetAdv
{
	DECLARE_DYNAMIC(CWizardSheet)

public:
	CWizardSheet(CWnd *pParentWnd = NULL, UINT iSelectPage = 0);

	CRect			m_rcPage;

	static BOOL	RunWizard(CWnd* pParent = NULL);

protected:
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	virtual BOOL OnInitDialog();

	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};


class CWizardPage : public CPropertyPageAdv
{
	DECLARE_DYNCREATE(CWizardPage)

public:
	CWizardPage(UINT nID = 0);

	CWizardSheet*	GetSheet();
	void			SetWizardButtons(DWORD dwFlags);
	void			StaticReplace(LPCTSTR pszSearch, LPCTSTR pszReplace);
	BOOL			IsConnectionCapable();

protected:
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};
