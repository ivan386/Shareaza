//
// GraphLine.h
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

#if !defined(AFX_GRAPHLINE_H__90D244AB_D902_42D6_BD55_E8F34845EB1A__INCLUDED_)
#define AFX_GRAPHLINE_H__90D244AB_D902_42D6_BD55_E8F34845EB1A__INCLUDED_

#pragma once

#include "GraphBase.h"

class CGraphItem;


class CLineGraph : public CGraphBase
{
// Construction
public:
	CLineGraph();
	virtual ~CLineGraph();

// Attributes
public:
	BOOL		m_bShowAxis;
	BOOL		m_bShowGrid;
	BOOL		m_bShowLegend;
	COLORREF	m_crBack;
	COLORREF	m_crGrid;
	DWORD		m_nMinGridVert;
public:
	DWORD		m_nSpeed;
	DWORD		m_nScale;	// How much points to store in memory; default is 2, which means 200% of display width 
	DWORD		m_nMaximum;
	DWORD		m_nUpdates;
	DWORD		m_tLastScale;
protected:
	CList< CGraphItem* > m_pItems;
	CPen		m_pGridPen;

// Operations
public:
	void		AddItem(CGraphItem* pItem);
	POSITION	GetItemIterator() const;
	CGraphItem*	GetNextItem(POSITION& pos) const;
	INT_PTR		GetItemCount() const { return m_pItems.GetCount(); }
	void		RemoveItem(CGraphItem* pItem);
	void		ClearItems();
	void		ResetMaximum(BOOL bForce = TRUE);
public:
	virtual void	CreateDefaults();
	virtual void	Serialize(CArchive& ar);
	virtual BOOL	Update();
	virtual void	Clear();
	virtual void	Paint(CDC* pDC, CRect* pRect);
protected:
	void	PaintGrid(CDC* pDC, CRect* pRect);
	void	PaintLegend(CDC* pDC, CRect* pRect);

};

#endif // !defined(AFX_GRAPHLINE_H__90D244AB_D902_42D6_BD55_E8F34845EB1A__INCLUDED_)
