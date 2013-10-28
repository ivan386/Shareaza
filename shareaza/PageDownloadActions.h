//
// PageDownloadActions.h
//
// Copyright (c) Shareaza Development Team, 2008-2011.
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


class CDownloadActionsPage : public CPropertyPageAdv
{
public:
	CDownloadActionsPage();
	virtual ~CDownloadActionsPage();

	DECLARE_DYNAMIC(CDownloadActionsPage)

	enum { IDD = IDD_DOWNLOAD_ACTIONS };

protected:
	CStatic m_wndForgetVerify;
	CStatic m_wndForgetSources;
	CString m_sEraseFrom;
	CString m_sEraseTo;
	CStatic m_wndCompleteVerify;
	CStatic m_wndMergeVerify;
	CStatic m_wndCancelDownload;

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	void OnForgetVerify();
	void OnForgetSources();
	void OnCompleteVerify();
	void OnMergeAndVerify();
	void OnCancelDownload();

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnErase();

	DECLARE_MESSAGE_MAP()
};
