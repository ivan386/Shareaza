//
// CtrlLibraryHomeView.h
//
// Copyright (c) Shareaza Development Team, 2002-2004.
// This file is part of SHAREAZA (www.shareaza.com)
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

#if !defined(AFX_CTRLLIBRARYHOMEVIEW_H__4D0D45D7_AC36_40C0_9EDC_79D843FDEAD4__INCLUDED_)
#define AFX_CTRLLIBRARYHOMEVIEW_H__4D0D45D7_AC36_40C0_9EDC_79D843FDEAD4__INCLUDED_

#pragma once

#include "CtrlLibraryView.h"
#include "CtrlLibraryTileView.h"


class CLibraryHomeView : public CLibraryView
{
// Construction
public:
	CLibraryHomeView();
	virtual ~CLibraryHomeView();
	
	DECLARE_DYNCREATE(CLibraryHomeView)
	
// Attributes
protected:
	CLibraryTileView	m_wndTile;

// Operations
public:
	virtual BOOL	CheckAvailable(CLibraryTreeItem* pSel);
	virtual void	GetHeaderContent(int& nImage, CString& strTitle);
	virtual void	Update();

// Overrides
public:
	//{{AFX_VIRTUAL(CLibraryHomeView)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CLibraryHomeView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLLIBRARYHOMEVIEW_H__4D0D45D7_AC36_40C0_9EDC_79D843FDEAD4__INCLUDED_)
