//
// LiveList.h
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

#if !defined(AFX_LIVELIST_H__D1A833C9_1477_43C7_9644_DB0C85370511__INCLUDED_)
#define AFX_LIVELIST_H__D1A833C9_1477_43C7_9644_DB0C85370511__INCLUDED_

#pragma once

class CLiveItem;


class CLiveList  
{
// Construction
public:
	CLiveList(int nColumns);
	virtual ~CLiveList();

// Attributes
protected:
	int			m_nColumns;
	CMap<DWORD, DWORD, CLiveItem*, CLiveItem*&>	m_pItems;
protected:
	static CBitmap m_bmSortAsc;
	static CBitmap m_bmSortDesc;

// Operations
public:
	CLiveItem*	Add(DWORD nParam);
	CLiveItem*	Add(LPVOID pParam);
	void		Apply(CListCtrl* pCtrl, BOOL bSort = FALSE);
protected:
	void		Clear();

// Sort Helpers
public:
	static void Sort(CListCtrl* pCtrl, int nColumn = -1, BOOL bGraphic = TRUE);
	static int CALLBACK SortCallback(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int SortProc(LPCTSTR sA, LPCTSTR sB, BOOL bNumeric = FALSE);
	static inline BOOL IsNumber(LPCTSTR pszString);

// Drag Helpers
public:
	static CImageList* CreateDragImage(CListCtrl* pList, const CPoint& ptMouse);
	static COLORREF crDrag;

};


class CLiveItem
{
// Construction
public:
	CLiveItem(int nColumns, DWORD nParam);
	virtual ~CLiveItem();

// Attributes
public:
	DWORD		m_nParam;
	int			m_nImage;
	UINT		m_nMaskOverlay;
	UINT		m_nMaskState;
public:
	CString*	m_pColumn;

// Operations
public:
	void	Set(int nColumn, LPCTSTR pszText);
	void	Format(int nColumn, LPCTSTR pszFormat, ...);
public:
	int		Add(CListCtrl* pCtrl, int nItem, int nColumns);
	BOOL	Update(CListCtrl* pCtrl, int nItem, int nColumns);

};

#ifndef CDRF_NOTIFYSUBITEMDRAW

#define LVS_EX_NOHSCROLL        0x10000000
#define LVS_EX_FLATSB			0x00000100
#define LVS_EX_REGIONAL			0x00000200
#define LVS_EX_INFOTIP			0x00000400
#define LVS_EX_LABELTIP			0x00004000
#define LVS_EX_UNDERLINEHOT		0x00000800
#define LVS_EX_UNDERLINECOLD	0x00001000
#define LVS_EX_MULTIWORKAREAS	0x00002000

#define CDRF_NOTIFYSUBITEMDRAW  0x00000020
#define CDDS_SUBITEM            0x00020000

#define LVM_GETSUBITEMRECT      (LVM_FIRST + 56)
#define ListView_GetSubItemRect(hwnd, iItem, iSubItem, code, prc) \
        (BOOL)SNDMSG((hwnd), LVM_GETSUBITEMRECT, (WPARAM)(int)(iItem), \
                ((prc) ? ((((LPRECT)(prc))->top = iSubItem), (((LPRECT)(prc))->left = code), (LPARAM)(prc)) : (LPARAM)(LPRECT)NULL))

/*
typedef struct tagLVHITTESTINFOEX
{
    POINT pt;
    UINT flags;
    int iItem;
    int iSubItem;
} LVHITTESTINFOEX, FAR* LPLVHITTESTINFOEX;

#define LVM_SUBITEMHITTEST      (LVM_FIRST + 57)
#define ListView_SubItemHitTest(hwnd, plvhti) \
        (int)SNDMSG((hwnd), LVM_SUBITEMHITTEST, 0, (LPARAM)(LPLVHITTESTINFOEX)(plvhti))
*/

#endif

#endif // !defined(AFX_LIVELIST_H__D1A833C9_1477_43C7_9644_DB0C85370511__INCLUDED_)
