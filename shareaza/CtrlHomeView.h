//
// CtrlHomeView.h
//
// Copyright (c) Shareaza Development Team, 2002-2004.
// This file is part of SHAREAZA (www.shareaza.com)
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

#include "RichViewCtrl.h"
#include "RichDocument.h"
#include "CtrlHomeSearch.h"


class CHomeViewCtrl : public CRichViewCtrl
{
// Construction
public:
	CHomeViewCtrl();
	virtual ~CHomeViewCtrl();

// Attributes
public:
	CRichDocument	m_pDocument;
	CRichElement*	m_peHeader;
	CRichElement*	m_peSearch;
	CRichElement*	m_peUpgrade;
	CRichElement*	m_peRemote1;
	CRichElement*	m_peRemote2;
	CHomeSearchCtrl	m_wndSearch;
	CBitmap			m_bmHeader1;
	CBitmap			m_bmHeader2;

// Operations
public:
	virtual BOOL	Create(const RECT& rect, CWnd* pParentWnd);
	void			Setup();
	void			Update();
protected:
	virtual void	OnLayoutComplete();
	virtual void	OnPaintBegin(CDC* pDC);
	virtual void	OnVScrolled();
	
// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);


};

#define IDC_HOME_VIEW		150
#define IDC_HOME_SEARCH		151
