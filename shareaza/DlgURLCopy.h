//
// DlgURLCopy.h
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

#include "DlgSkinDialog.h"
#include "ShareazaFile.h"


class CURLCopyDlg : public CSkinDialog
{
	DECLARE_DYNAMIC(CURLCopyDlg)

public:
	CURLCopyDlg(CWnd* pParent = NULL);

	enum { IDD = IDD_URL_COPY };

	void		Add(const CShareazaFile* pFile);

	// Gathers more information about file (including trackers list)
	static void Resolve(CShareazaFile& pFile, CString& sTracker, CString& sWebSeed);

	// Gathers more information about file (including trackers list) and returns magnet-link
	static CString CreateMagnet(CShareazaFile& pFile);

protected:
	CButton					m_wndIncludeSelf;
	CStatic					m_wndMessage;
	CString					m_sHost;
	CString					m_sMagnet;
	CString					m_sED2K;
	CShareazaFile			m_pFile;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnIncludeSelf();
	afx_msg void OnStnClickedUrlHost();
	afx_msg void OnStnClickedUrlMagnet();
	afx_msg void OnStnClickedUrlEd2k();

	DECLARE_MESSAGE_MAP()
};
