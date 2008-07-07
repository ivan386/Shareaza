//
// PageDownloadEdit.h
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

#include "PagePropertyAdv.h"

class CDownload;


class CDownloadEditPage : public CPropertyPageAdv
{
public:
	CDownloadEditPage();
	virtual ~CDownloadEditPage();

	DECLARE_DYNAMIC(CDownloadEditPage)

	enum { IDD = IDD_DOWNLOAD_EDIT };

	CString m_sName;
	CString m_sDiskName;
	CString m_sFileSize;
	CString m_sSHA1;
	CString m_sTiger;
	CString m_sED2K;
	BOOL m_bSHA1Trusted;
	BOOL m_bTigerTrusted;
	BOOL m_bED2KTrusted;
	CStatic m_wndForgetVerify;
	CStatic m_wndForgetSources;
	CString m_sEraseFrom;
	CString m_sEraseTo;
	CStatic m_wndCompleteVerify;
	CStatic m_wndMergeVerify;

	BOOL	Commit();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnErase();
	afx_msg void OnMergeAndVerify();

	DECLARE_MESSAGE_MAP()
};
