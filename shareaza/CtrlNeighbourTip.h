//
// CtrlNeighbourTip.h
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

#if !defined(AFX_CTRLNEIGHBOURTIP_H__07D9C280_7166_4F2E_932B_1C9F5DB936C2__INCLUDED_)
#define AFX_CTRLNEIGHBOURTIP_H__07D9C280_7166_4F2E_932B_1C9F5DB936C2__INCLUDED_

#pragma once

#include "CtrlCoolTip.h"

class CGraphItem;


class CNeighbourTipCtrl : public CCoolTipCtrl
{
// Construction
public:
	CNeighbourTipCtrl();
	virtual ~CNeighbourTipCtrl();

// Attributes
public:
	CLineGraph*	m_pGraph;
	CGraphItem*	m_pItemIn;
	CGraphItem*	m_pItemOut;

// Operations
protected:
	virtual BOOL OnPrepare();
	virtual void OnShow();
	virtual void OnHide();
	virtual void OnCalcSize(CDC* pDC);
	virtual void OnPaint(CDC* pDC);

// Overrides
public:
	//{{AFX_VIRTUAL(CNeighbourTipCtrl)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CNeighbourTipCtrl)
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLNEIGHBOURTIP_H__07D9C280_7166_4F2E_932B_1C9F5DB936C2__INCLUDED_)
