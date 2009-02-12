//
// CtrlLibraryPanel.h
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

#if !defined(AFX_CTRLLIBRARYPANEL_H__B67FA20B_ECB1_462F_AAEF_789FDCA3151E__INCLUDED_)
#define AFX_CTRLLIBRARYPANEL_H__B67FA20B_ECB1_462F_AAEF_789FDCA3151E__INCLUDED_

#pragma once

class CLibraryTreeItem;


class CLibraryPanel : public CWnd
{
// Construction
public:
	CLibraryPanel();
	virtual ~CLibraryPanel();

	DECLARE_DYNAMIC(CLibraryPanel)

// Attributes
public:
	BOOL	m_bAvailable;

// Operations
public:
	virtual BOOL CheckAvailable(CLibraryTreeItem* pFolders, CLibraryList* pObjects);
	virtual void Update();
protected:
	CLibraryTreeItem*	GetFolderSelection();
	CLibraryList*		GetViewSelection();

// Overrides
public:
	//{{AFX_VIRTUAL(CLibraryPanel)
	public:
	virtual BOOL Create(CWnd* pParentWnd);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CLibraryPanel)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#define IDC_LIBRARY_PANEL	133

#endif // !defined(AFX_CTRLLIBRARYPANEL_H__B67FA20B_ECB1_462F_AAEF_789FDCA3151E__INCLUDED_)
