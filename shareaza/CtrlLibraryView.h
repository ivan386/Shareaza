//
// CtrlLibraryView.h
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

#if !defined(AFX_CTRLLIBRARYVIEW_H__9FA28C5B_A6A3_4616_9407_4661EA1C7B8A__INCLUDED_)
#define AFX_CTRLLIBRARYVIEW_H__9FA28C5B_A6A3_4616_9407_4661EA1C7B8A__INCLUDED_

#pragma once

class CAlbumFolder;
class CLibraryFrame;
class CLibraryTreeItem;
class CLibraryTipCtrl;


class CLibraryView : public CWnd
{
// Construction
public:
	CLibraryView();
	virtual ~CLibraryView();

	DECLARE_DYNAMIC(CLibraryView)

// Attributes
public:
	UINT			m_nCommandID;
	LPCTSTR			m_pszToolBar;
	BOOL			m_bAvailable;
public:
	CLibraryList	m_pSelection;

// Operations
public:
	virtual BOOL		CheckAvailable(CLibraryTreeItem* pSel);
	virtual void		GetHeaderContent(int& nImage, CString& strTitle);
	virtual void		Update();
	virtual BOOL		Select(DWORD nObject);
	virtual void		CacheSelection();
protected:
	void				PostUpdate();
	CLibraryFrame*		GetFrame() const;
	CLibraryTipCtrl*	GetToolTip() const;
	DWORD				GetFolderCookie() const;
	CLibraryTreeItem*	GetFolderSelection() const;
	CAlbumFolder*		GetSelectedAlbum(CLibraryTreeItem* pSel = NULL) const;
	void				DragObjects(CImageList* pImage, const CPoint& ptMouse);
protected:
	BOOL	SelAdd(DWORD nObject, BOOL bNotify = TRUE);
	BOOL	SelRemove(DWORD nObject, BOOL bNotify = TRUE);
	BOOL	SelClear(BOOL bNotify = TRUE);
	int		GetSelectedCount() const;

// Overrides
public:
	//{{AFX_VIRTUAL(CLibraryView)
	public:
	virtual BOOL Create(CWnd* pParentWnd);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CLibraryView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#define IDC_LIBRARY_VIEW	132

#endif // !defined(AFX_CTRLLIBRARYVIEW_H__9FA28C5B_A6A3_4616_9407_4661EA1C7B8A__INCLUDED_)
