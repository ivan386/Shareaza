//
// CtrlNetworkCombo.h
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


class CNetworkCombo : public CComboBox
{
// Construction
public:
	CNetworkCombo();
	virtual ~CNetworkCombo();
	
	DECLARE_DYNAMIC(CNetworkCombo)
	
// Operations
public:
	virtual BOOL Create(DWORD dwStyle, CWnd* pParentWnd, UINT nID);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void OnSkinChange();
public:
	int		GetNetwork();
	void	SetNetwork(int nProtocol);

// Attributes
protected:
	CImageList	m_gdiImageList;

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};


