//
// DlgDownload.h
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

#pragma once

#include "DlgSkinDialog.h"

class CDownload;
class CShareazaURL;


class CDownloadDlg : public CSkinDialog
{
	DECLARE_DYNAMIC(CDownloadDlg)

public:
	CDownloadDlg(CWnd* pParent = NULL, CDownload* pDownload = NULL);

	enum { IDD = IDD_DOWNLOAD };

	CList< CString >	m_pURLs;

protected:
	CDownload*			m_pDownload;
	CButton				m_wndTorrentFile;
	CButton				m_wndOK;
	CEdit				m_wndURL;
	CString				m_sURL;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeURL();
	virtual void OnOK();
	afx_msg void OnTorrentFile();

	DECLARE_MESSAGE_MAP()
};
