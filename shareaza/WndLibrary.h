//
// WndLibrary.h
//
// Copyright (c) Shareaza Development Team, 2002-2013.
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

#include "WndPanel.h"
#include "CtrlLibraryFrame.h"

class CLibraryFile;
class CAlbumFolder;


class CLibraryWnd : public CPanelWnd
{
	DECLARE_SERIAL(CLibraryWnd)

public:
	CLibraryWnd();
	virtual ~CLibraryWnd();

	CLibraryFrame		m_wndFrame;
	DWORD				m_tLast;

	// Open Library window
	static CLibraryWnd*	GetLibraryWindow(BOOL bToggle = FALSE, BOOL bFocus = TRUE);

	BOOL	Display(const CLibraryFile* pFile);
	BOOL	Display(const CAlbumFolder* pFolder);
	BOOL	OnCollection(const CString& sPath);

protected:
	virtual void OnSkinChange();
	virtual HRESULT	GetGenericView(IGenericView** ppView);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};
