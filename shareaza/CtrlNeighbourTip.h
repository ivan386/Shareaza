//
// CtrlNeighbourTip.h
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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

#include "CtrlCoolTip.h"

class CGraphItem;


class CNeighbourTipCtrl : public CCoolTipCtrl
{
	DECLARE_DYNAMIC(CNeighbourTipCtrl)

public:
	CNeighbourTipCtrl();
	virtual ~CNeighbourTipCtrl();

	void Show(DWORD_PTR pContext, HWND hAltWnd = NULL)
	{
		bool bChanged = ( pContext != m_nNeighbour );
		m_nNeighbour = pContext;
		m_hAltWnd = hAltWnd;
		ShowImpl( bChanged );
	}

protected:
	DWORD_PTR	m_nNeighbour;
	CLineGraph*	m_pGraph;
	CGraphItem*	m_pItemIn;
	CGraphItem*	m_pItemOut;
	CImageList	m_pProtocols;

	virtual BOOL OnPrepare();
	virtual void OnShow();
	virtual void OnHide();
	virtual void OnCalcSize(CDC* pDC);
	virtual void OnPaint(CDC* pDC);

	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};
