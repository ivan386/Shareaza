//
// CtrlLibraryHeaderBar.h
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

#if !defined(AFX_CTRLLIBRARYHEADERBAR_H__34CB5F68_1928_4F00_BAB3_EA34F4379DBD__INCLUDED_)
#define AFX_CTRLLIBRARYHEADERBAR_H__34CB5F68_1928_4F00_BAB3_EA34F4379DBD__INCLUDED_

#pragma once

#include "CtrlCoolBar.h"

class CLibraryView;
class CCoolMenu;


class CLibraryHeaderBar : public CCoolBarCtrl
{
// Construction
public:
	CLibraryHeaderBar();
	virtual ~CLibraryHeaderBar();

	DECLARE_DYNAMIC(CLibraryHeaderBar)

// Attributes
protected:
	CLibraryView*	m_pLastView;
	int				m_nImage;
	CString			m_sTitle;
protected:
	CCoolMenu*		m_pCoolMenu;

// Operations
public:
	void	Update(CLibraryView* pView);
protected:
	virtual void PrepareRect(CRect* pRect) const;
	virtual void DoPaint(CDC* pDC, CRect& rcBar, BOOL bTransparent);
protected:
	void	PaintHeader(CDC* pDC, CRect& rcBar, BOOL bTransparent);

// Overrides
public:
	//{{AFX_VIRTUAL(CLibraryHeaderBar)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CLibraryHeaderBar)
	afx_msg void OnLibraryView();
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg void OnEnterIdle(UINT nWhy, CWnd* pWho);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLLIBRARYHEADERBAR_H__34CB5F68_1928_4F00_BAB3_EA34F4379DBD__INCLUDED_)
