//
// WizardSheet.h
//
// Copyright (c) Shareaza Development Team, 2007-2011.
// This file is part of Shareaza Torrent Wizard (shareaza.sourceforge.net).
//
// Shareaza Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Torrent Wizard is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#pragma once

class CWizardPage;


class CWizardSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CWizardSheet)

public:
	CWizardSheet(CWnd *pParentWnd = NULL, UINT iSelectPage = 0);

	CRect			m_rcPage;

	CWizardPage*	GetPage(CRuntimeClass* pClass);
	void			DoReset();

protected:
	CBitmap			m_bmHeader;

	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	virtual BOOL OnInitDialog();

	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};


class CWizardPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CWizardPage)

public:
	CWizardPage(UINT nID = 0, LPCTSTR szHelp = NULL);

	CString		m_sHelp;
	COLORREF	m_crWhite;
	CBrush		m_brWhite;

	void			Next();
	CWizardSheet*	GetSheet();
	CWizardPage*	GetPage(CRuntimeClass* pClass);
	void			SetWizardButtons(DWORD dwFlags);
	void			StaticReplace(LPCTSTR pszSearch, LPCTSTR pszReplace);

protected:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnPressButton(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

#define GET_PAGE(gpClass, gpVar)	gpClass * gpVar = ( gpClass * )GetPage( RUNTIME_CLASS( gpClass ) )

#define WM_PRESSBUTTON (WM_APP + 100)
