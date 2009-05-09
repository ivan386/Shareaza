//
// PageTorrentFiles.h
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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

#include "PagePropertyAdv.h"
#include "ComboListCtrl.h"


class CTorrentFilesPage : public CPropertyPageAdv
{
public:
	CTorrentFilesPage();
	virtual ~CTorrentFilesPage();

	DECLARE_DYNCREATE(CTorrentFilesPage)

	enum { IDD = IDD_TORRENT_FILES };

protected:
	CComboListCtrl	m_wndFiles;

	// Update interface
	void Update();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnNMDblclkTorrentFiles(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
};
