//
// DlgSelect.h
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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

#include "DlgSkinDialog.h"

// CSelectDialog dialog

class CSelectDialog : public CSkinDialog
{
	DECLARE_DYNAMIC(CSelectDialog)

public:
	CSelectDialog(CWnd* pParent = NULL);

	enum { IDD = IDD_SELECT };

	class CItem
	{
	public:
		inline CItem(const CString& sItem = CString(), int nData = 0) :
			m_sItem( sItem ), m_nData( nData ) {}
		inline CItem(const CItem& it) :
			m_sItem( it.m_sItem ), m_nData( it.m_nData ) {}
		inline CItem& operator=(const CItem& it)
			{ m_sItem = it.m_sItem; m_nData = it.m_nData; return *this; }
		CString		m_sItem;
		DWORD_PTR	m_nData;
	};

	typedef CList< CItem > CItemList;

	// Add new item to selection list
	inline void Add(const CString& sItem, int nData)
	{
		m_List.AddTail( CItem( sItem, nData ) );
	}

	// Get current selection data
	inline DWORD_PTR Get() const
	{
		return m_nData;
	}

protected:
	CComboBox	m_ListCtrl;
	CItemList	m_List;
	DWORD_PTR	m_nData;

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnCbnDropdownList();

	DECLARE_MESSAGE_MAP()
};
