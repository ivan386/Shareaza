//
// CtrlRichTaskBox.h
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

#if !defined(AFX_CTRLRICHTASKBOX_H__B88F0CFD_3869_4C15_A677_0FF0A78F6F99__INCLUDED_)
#define AFX_CTRLRICHTASKBOX_H__B88F0CFD_3869_4C15_A677_0FF0A78F6F99__INCLUDED_

#pragma once

#include "CtrlTaskPanel.h"
#include "RichViewCtrl.h"

class CRichTaskBox : public CTaskBox
{
// Construction
public:
	CRichTaskBox();
	virtual ~CRichTaskBox();

	DECLARE_DYNAMIC(CRichTaskBox)

// Attributes
protected:
	CRichViewCtrl	m_wndView;
	int				m_nWidth;
	CRichDocument*	m_pDocument;

// Operations
public:
	inline CRichViewCtrl& GetView() const { return (CRichViewCtrl&)m_wndView; }
public:
	void	SetDocument(CRichDocument* pDocument);
	void	Update();

// Overrides
public:
	//{{AFX_VIRTUAL(CRichTaskBox)
	public:
	virtual BOOL Create(CTaskPanel* pPanel, LPCTSTR pszCaption = NULL, UINT nIDIcon = 0);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CRichTaskBox)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLRICHTASKBOX_H__B88F0CFD_3869_4C15_A677_0FF0A78F6F99__INCLUDED_)
