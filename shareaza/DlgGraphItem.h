//
// DlgGraphItem.h
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

#if !defined(AFX_DLGGRAPHITEM_H__C643EFD4_C72F_431A_B0B5_97F6D8FBC3A1__INCLUDED_)
#define AFX_DLGGRAPHITEM_H__C643EFD4_C72F_431A_B0B5_97F6D8FBC3A1__INCLUDED_

#pragma once

#include "DlgSkinDialog.h"

class CGraphItem;


class CGraphItemDlg : public CSkinDialog
{
// Construction
public:
	CGraphItemDlg(CWnd* pParent = NULL, CGraphItem* pItem = NULL);

// Dialog Data
public:
	//{{AFX_DATA(CGraphItemDlg)
	enum { IDD = IDD_GRAPH_ITEM };
	CButton	m_wndOK;
	CEdit	m_wndUnits;
	CComboBox	m_wndSource;
	CButton	m_wndRemove;
	float m_nMultiplier;
	CStatic	m_wndColourBox;
	//}}AFX_DATA

	CGraphItem*		m_pItem;
	COLORREF		m_crColour;
	CImageList		m_gdiImageList;

// Overrides
public:
	//{{AFX_VIRTUAL(CGraphItemDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CGraphItemDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelChangeGraphSource();
	afx_msg void OnGraphColour();
	afx_msg void OnPaint();
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

void PASCAL DDX_Float(CDataExchange* pDX, int nIDC, float& nValue);

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGGRAPHITEM_H__C643EFD4_C72F_431A_B0B5_97F6D8FBC3A1__INCLUDED_)
