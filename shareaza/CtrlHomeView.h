//
// CtrlHomeView.h
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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

#include "RichViewCtrl.h"
#include "RichDocument.h"
#include "CtrlHomeSearch.h"


class CHomeViewCtrl : public CRichViewCtrl
{
	DECLARE_DYNCREATE(CHomeViewCtrl)

public:
	CHomeViewCtrl();

	CRichDocument	m_pDocument;
	CRichElement*	m_peHeader;
	CRichElement*	m_peSearch;
	CRichElement*	m_peUpgrade;
	CRichElement*	m_peRemote1;
	CRichElement*	m_peRemote2;
	CHomeSearchCtrl	m_wndSearch;

	virtual BOOL	Create(const RECT& rect, CWnd* pParentWnd);
	void			OnSkinChange();
	void			Activate();
	void			Update();

protected:
	virtual void	OnLayoutComplete();
	virtual void	OnPaintBegin(CDC* pDC);
	virtual void	OnVScrolled();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

#define IDC_HOME_VIEW		150
#define IDC_HOME_SEARCH		151
