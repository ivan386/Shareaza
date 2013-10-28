//
// ComboListCtrl.h
//
// Copyright (c) Shareaza Development Team, 2008-2009.
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

#include "CtrlLibraryTip.h"


class CComboListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CComboListCtrl)

public:
	CComboListCtrl();
	virtual ~CComboListCtrl();

	typedef std::map< int, CString > CIntStringMap;
	typedef std::map< int, int > CIntIntMap;
	typedef std::vector< CIntIntMap > CIntIntMapVector;
	typedef std::map< int, CIntStringMap > CIntIntStringMapMap;

	void EnableTips(auto_ptr< CLibraryTipCtrl > pTip);
	int GetColumnData(int iItem, int iColumn) const;
	void SetColumnData(int iItem, int iColumn, int iData);
	void SetColumnValues(int iColumn, const CIntStringMap& oValues);

protected:
	auto_ptr< CLibraryTipCtrl >	m_pTip;
	CComboBox*					m_pCombo;
	int							m_iSelectedItem;
	int							m_iSelectedSubItem;
	CIntIntMapVector			m_oData;
	CIntIntStringMapMap			m_oColumns;

	int HitTest(const CPoint& ptAction);
	void Show(int iItem, int iSubItem);
	void Hide();

	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};

#define BEGIN_COLUMN_MAP() \
	{ CComboListCtrl::CIntStringMap x;

#define COLUMN_MAP(nValue, nString) \
	x.insert( CComboListCtrl::CIntStringMap::value_type( (nValue), (nString) ) );

#define END_COLUMN_MAP(oWnd, nColumn) \
	(oWnd).SetColumnValues( (nColumn), x ); }
