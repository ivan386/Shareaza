//
// DlgExistingFile.h
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

class CLibraryFile;
class CShareazaFile;
class CShareazaURL;


class CExistingFileDlg : public CSkinDialog
{
	DECLARE_DYNAMIC(CExistingFileDlg)

public:
	CExistingFileDlg(const CLibraryFile* pFile, CWnd* pParent = NULL);

	enum { IDD = IDD_EXISTING_FILE };
	enum Action { ShowInLibrary, Download, DontDownload, Cancel };

	static Action CheckExisting(const CShareazaURL* pURL, BOOL bInteracive = TRUE);
	static Action CheckExisting(const CShareazaFile* pFile, BOOL bInteracive = TRUE);

	inline Action GetResult() const
	{
		switch ( m_nAction )
		{
		case 0:
			return ShowInLibrary;
		case 1:
			return Download;
		case 2:
			return DontDownload;
		default:
			return Cancel;
		}
	}

	virtual INT_PTR DoModal();

protected:
	CButton	m_wndOK;
	CStatic	m_wndName;
	CString	m_sName;
	CString	m_sURN;
	int		m_nAction;
	CStatic m_wndComments;
	CStatic m_wndMessageAvailable;
	CStatic m_wndMessageDeleted;
	CStatic m_wndMessageDuplicates;
	CButton m_wndLocate;
	CButton m_wndDownload;
	CButton m_wndDontDownload;
	CString m_sComments;
	TRISTATE m_bAvailable;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnAction0();
	afx_msg void OnAction1();
	afx_msg void OnAction2();

	DECLARE_MESSAGE_MAP()
};
