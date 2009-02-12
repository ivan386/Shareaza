//
// DlgURLCopy.h
//
// Copyright © Shareaza Development Team, 2002-2009.
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

#if !defined(AFX_DLGURLCOPY_H__CFF49DD5_9C74_4024_B7C0_551F4B05DFB6__INCLUDED_)
#define AFX_DLGURLCOPY_H__CFF49DD5_9C74_4024_B7C0_551F4B05DFB6__INCLUDED_

#pragma once

#include "DlgSkinDialog.h"
#include "ShareazaFile.h"

class CURLCopyDlg : public CSkinDialog
{
	DECLARE_DYNAMIC(CURLCopyDlg)

public:
	CURLCopyDlg(CWnd* pParent = NULL);

	enum { IDD = IDD_URL_COPY };
	CButton			m_wndIncludeSelf;
	CStatic			m_wndMessage;
	CString			m_sHost;
	CString			m_sMagnet;
	CString			m_sGnutella;
	CString			m_sED2K;

	void		Add(const CShareazaFile* pFile);
	static BOOL	SetClipboardText(CString& strText);

protected:
	const CShareazaFile*			m_pFile;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnIncludeSelf();

	DECLARE_MESSAGE_MAP()

};

#endif // !defined(AFX_DLGURLCOPY_H__CFF49DD5_9C74_4024_B7C0_551F4B05DFB6__INCLUDED_)
