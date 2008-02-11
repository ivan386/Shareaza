//
// CtrlFontCombo.h
//
//  Created by:		Rolandas
//	Date:			"$Date: 2005/08/08 03:47:40 $"
//	Revision:		"$Revision: 1.1.2.1 $"
//  Last change by:	"$Author: rolandas $"
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

#if !defined(CTRLFONTCOMBO_H_INCLUDED)
#define CTRLFONTCOMBO_H_INCLUDED

#pragma once

class CFontCombo : public CComboBox
{
// Construction
public:
	CFontCombo();
	virtual ~CFontCombo();

	DECLARE_DYNAMIC(CFontCombo)

// Attributes
	CString m_sSelectedFont;
	
// Operations
public:
	void	Initialize();
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
// Overrides
protected:
	virtual void PreSubclassWindow();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

// Implementation
public:
	int GetFontHeight();
	void SetFontHeight(int nNewHeight, BOOL bReinitialize = TRUE);
	
protected:
	CImageList m_pImages;	
	CMapStringToPtr m_pFonts;

	int m_nFontHeight;

	BOOL AddFont(const CString& strFontName);
	void DeleteAllFonts();
	static BOOL CALLBACK EnumFontProc(LPENUMLOGFONTEX lplf, NEWTEXTMETRICEX* lpntm, DWORD dwFontType, LPVOID lpData);

	// Generated message map functions
	//{{AFX_MSG(CFontCombo)
	afx_msg void OnDropdown();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnOcmDrawItem(WPARAM /*wParam*/, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

void PASCAL DDX_FontCombo(CDataExchange* pDX, int nIDC, CString& strFontName);

#endif // !defined(CTRLFONTCOMBO_H_INCLUDED)
