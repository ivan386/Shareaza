//
// LiveList.h
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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

class CLiveItem;
class CLiveList;
class CLiveListCtrl;


class CLiveItem : public CObject
{
	DECLARE_DYNAMIC( CLiveItem )

public:
	CLiveItem(int nColumns, DWORD_PTR nParam);
	virtual ~CLiveItem();

#ifdef _DEBUG
	virtual void AssertValid() const
	{
		CObject::AssertValid();
		m_nImage.AssertValid();
		m_pColumn.AssertValid();
	}
#endif

	void	Set(int nColumn, const CString& sText);
	void	SetImage(int nColumn, int nImage);
	void	SetMaskOverlay(UINT nMaskOverlay);
	void	Format(int nColumn, LPCTSTR pszFormat, ...);

	int		Add(CListCtrl* pCtrl, int nItem, int nColumns);
	BOOL	Update(CListCtrl* pCtrl, int nItem, int nColumns);
	BOOL	SetImage(CListCtrl* pCtrl, LPARAM nParam, int nColumn, int nImageIndex);

protected:
	DWORD_PTR	m_nParam;
	bool		m_bModified;	// Is data modified?
	UINT		m_nModified;	// Modified columns (bitmask)
	bool		m_bOld;			// Is item old? (marked to deletion)
	UINT		m_nMaskOverlay;
	UINT		m_nMaskState;
	CArray< int >		m_nImage;		// Image for each column (-1 : no image, 0..N : image index)
	CArray< CString >	m_pColumn;		// Text for each column

	friend class CLiveList;
	friend class CLiveListCtrl;
};

typedef CLiveItem* CLiveItemPtr;


class CLiveList : public CObject
{
	DECLARE_DYNAMIC( CLiveList )

public:
	CLiveList(int nColumns, UINT nHash = 0);
	virtual ~CLiveList();

	static CBitmap		m_bmSortAsc;
	static CBitmap		m_bmSortDesc;

#ifdef _DEBUG
	virtual void		AssertValid() const
	{
		CObject::AssertValid();
		ASSERT( m_nColumns > 0 && m_nColumns < 100 );
		ASSERT_VALID( &m_pItems );
		ASSERT_VALID( &m_pSection );
		ASSERT_VALID( &m_bmSortAsc );
		ASSERT_VALID( &m_bmSortDesc );
	}
#endif

	CLiveItem*			Add(DWORD_PTR nParam);
	CLiveItem*			Add(LPVOID pParam);
	void				Apply(CListCtrl* pCtrl, BOOL bSort = FALSE);

// Sort Helpers
	static void			Sort(CListCtrl* pCtrl, int nColumn = -1, BOOL bGraphic = TRUE);
	static int CALLBACK	SortCallback(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static bool			Less(const CLiveItemPtr& _Left, const CLiveItemPtr& _Right, int nSortColumn);
	static int			SortProc(LPCTSTR sA, LPCTSTR sB, BOOL bNumeric = FALSE);
	static inline BOOL	IsNumber(LPCTSTR pszString);

// Drag Helpers
	static HBITMAP		CreateDragImage(CListCtrl* pList, const CPoint& ptMouse, CPoint& ptMiddle);
	static CImageList*	CreateDragImage(CListCtrl* pList, const CPoint& ptMouse);

protected:
	typedef CMap< DWORD_PTR, const DWORD_PTR&, CLiveItem*, CLiveItem*& > CLiveItemMap;

	int					m_nColumns;
	CLiveItemMap		m_pItems;
	CCriticalSection	m_pSection;

	void				Clear();
};


#ifndef CDRF_NOTIFYSUBITEMDRAW

#define LVS_EX_DOUBLEBUFFER 	0x00010000
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


class CLiveListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CLiveListCtrl)

public:
	CLiveListCtrl();
	virtual ~CLiveListCtrl();

	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID,
		int nColumns );

	CLiveItemPtr Add(DWORD_PTR nParam);
	CLiveItemPtr Add(LPVOID pParam);	
	void Apply();
	void Sort(int nColumn = -1);
	void ClearSelection();

	DWORD_PTR GetItemData(int nItem) const;
	UINT GetItemOverlayMask(int nItem) const;

protected:
	typedef std::map< DWORD_PTR, CLiveItemPtr >		CLiveMap;
	typedef std::pair< DWORD_PTR, CLiveItemPtr >	CLiveMapPair;
	typedef std::vector< CLiveItemPtr >				CLiveIndex;

	int					m_nColumns;
	CLiveMap			m_pItems;
	CLiveIndex			m_pIndex;

	afx_msg void OnLvnGetdispinfoW(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnGetdispinfoA(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnOdfinditemW(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnOdfinditemA(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnOdcachehint(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
};
