//
// CtrlTipList.h
//
// Copyright © Shareaza Development Team, 2002-2009.
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

#if !defined(AFX_CTRLTIPLIST_H__DAA67E67_69E9_400A_9DA9_6D38FC119CC5__INCLUDED_)
#define AFX_CTRLTIPLIST_H__DAA67E67_69E9_400A_9DA9_6D38FC119CC5__INCLUDED_

#pragma once

#include "CtrlCoolTip.h"


class CTipListCtrl : public CListCtrl
{
// Construction
public:
	CTipListCtrl();
	virtual ~CTipListCtrl();

	DECLARE_DYNAMIC(CTipListCtrl)

// Attributes
protected:
	CCoolTipCtrl*	m_pTip;

// Operations
public:
	void	SetTip(CCoolTipCtrl* pTip);

// Overrides
public:
	//{{AFX_VIRTUAL(CTipListCtrl)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CTipListCtrl)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLTIPLIST_H__DAA67E67_69E9_400A_9DA9_6D38FC119CC5__INCLUDED_)
