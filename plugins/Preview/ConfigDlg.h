//
// ConfigDlg.h : Declaration of the CConfigDlg
//
// Copyright (c) Nikolay Raspopov, 2009.
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

#include "resource.h"

// CConfigDlg

class CConfigDlg : 
	public CAxDialogImpl< CConfigDlg >
{
public:
	CConfigDlg();

	enum { IDD = IDD_CONFIGDLG };

BEGIN_MSG_MAP(CConfigDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
	COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
	COMMAND_HANDLER(IDC_ADD, BN_CLICKED, OnBnClickedAdd)
	COMMAND_HANDLER(IDC_DEL, BN_CLICKED, OnBnClickedDel)
	NOTIFY_HANDLER(IDC_LIST, NM_CLICK, OnNMClickList)
	COMMAND_HANDLER(IDC_BROWSE, BN_CLICKED, OnBnClickedBrowse)
	NOTIFY_HANDLER(IDC_LIST, LVN_ITEMACTIVATE, OnLvnItemActivateList)
	COMMAND_HANDLER(IDC_EXT, EN_CHANGE, OnEnChangeExt)
	COMMAND_HANDLER(IDC_COMMAND, EN_CHANGE, OnEnChangeCommand)
	COMMAND_HANDLER(IDC_WEB, BN_CLICKED, OnBnClickedWeb)
	CHAIN_MSG_MAP(CAxDialogImpl<CConfigDlg>)
END_MSG_MAP()

protected:
	int		m_nActive;			// Selected item index (or -1 if no item)

	void Load();
	void Save();
	int GetActive() const;
	void Select(int nIndex);	// Select item and update interface
	void Update(int nIndex);	// Update interface
	void AddNew();				// Add new item and update interface
	int Add(LPCTSTR szExt, LPCTSTR szCommand);
	void Delete(int nIndex);	// Remove item and update interface

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBnClickedAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBnClickedDel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnNMClickList(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled);
	LRESULT OnBnClickedBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnLvnItemActivateList(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled);
	LRESULT OnEnChangeExt(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEnChangeCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBnClickedWeb(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};
