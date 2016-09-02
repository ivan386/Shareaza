//
// PageSettingsAdvanced.h
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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

#include "WndSettingsPage.h"
#include "CtrlFontCombo.h"

class CSettingEdit;


class CAdvancedSettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CAdvancedSettingsPage)

public:
	CAdvancedSettingsPage();


protected:
	enum { IDD = IDD_SETTINGS_ADVANCED };

	class EditItem
	{
	public:
		EditItem(const CSettings::Item* pItem);

		CSettings::Item*	m_pItem;	// Parent item
		CString				m_sName;	// Item name
		DWORD				m_nValue;	// Current value for DWORD
		bool				m_bValue;	// Current value for bool
		CString				m_sValue;	// Current value for CString
		DWORD				m_nOriginalValue;	// Original value for DWORD
		bool				m_bOriginalValue;	// Original value for bool
		CString				m_sOriginalValue;	// Original value for CString

		void	Update();				// Reload data from parent item
		void	Commit();				// Commit data to parent item
		bool	IsModified() const;		// Value was modified
		bool	IsDefault() const;		// Check if value is equal to default value
		void	Default();				// Restore default value
	};

	typedef CList< EditItem* > CEditItemList;

	CEdit				m_wndQuickFilter;
	CStatic				m_wndQuickFilterIcon;
	CSpinButtonCtrl		m_wndValueSpin;
	CEdit				m_wndValue;
	CListCtrl			m_wndList;
	CFontCombo			m_wndFonts;
	CButton				m_wndDefaultBtn;
	CButton				m_wndBool;
	bool				m_bUpdating;
	CEditItemList		m_pSettings;
	UINT_PTR			m_nTimer;

	void	AddSettings();				// Add settings to list
	int		GetListItem(const EditItem* pItem);
	void	UpdateListItem(int nItem);	// Update list item
	void	UpdateInputArea();			// Update edit box, spins and buttons
	void	CommitAll();				// Commit all data to settings
	void	UpdateAll();				// Update settings list
	bool	IsModified() const;			// Check if some of settings was modified

	virtual void OnOK();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	afx_msg void OnDestroy();
	afx_msg void OnItemChangedProperties(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeValue();
	afx_msg void OnColumnClickProperties(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedDefaultValue();
	afx_msg void OnEnChangeQuickfilter();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()	
};
