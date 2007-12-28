//
// CtrlDragList.h
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

#if !defined(AFX_CTRLDRAGLIST_H__34DF1A23_88C6_4F76_91DF_575F07A37F2E__INCLUDED_)
#define AFX_CTRLDRAGLIST_H__34DF1A23_88C6_4F76_91DF_575F07A37F2E__INCLUDED_

#pragma once


class CDragListCtrl : public CListCtrl
{
// Construction
public:
	CDragListCtrl();
	virtual ~CDragListCtrl();

	DECLARE_DYNAMIC(CDragListCtrl)

// Attributes
protected:
	CImageList*	m_pDragImage;
	int			m_nDragDrop;
	BOOL		m_bCreateDragImage;

// Operations
public:
	virtual void	OnDragDrop(int nDrop);

// Overrides
public:
	//{{AFX_VIRTUAL(CDragListCtrl)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CDragListCtrl)
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#define LVN_DRAGDROP	(LVN_FIRST+1)

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLDRAGLIST_H__34DF1A23_88C6_4F76_91DF_575F07A37F2E__INCLUDED_)
