//
// CtrlLibraryHeaderPanel.h
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

#if !defined(AFX_CTRLLIBRARYHEADERPANEL_H__9E5471F0_5957_470D_9B52_9C8D18A76C3D__INCLUDED_)
#define AFX_CTRLLIBRARYHEADERPANEL_H__9E5471F0_5957_470D_9B52_9C8D18A76C3D__INCLUDED_

#pragma once

#include "MetaList.h"

class CAlbumFolder;


class CLibraryHeaderPanel : public CWnd
{
// Construction
public:
	CLibraryHeaderPanel();
	virtual ~CLibraryHeaderPanel();

// Attributes
protected:
	int			m_nIcon32;
	int			m_nIcon48;
	CString		m_sTitle;
	CString		m_sSubtitle;
	CMetaList	m_pMetadata;
	int			m_nMetaWidth;
	int			m_nKeyWidth;
protected:
	CSize		m_szBuffer;
	CDC			m_dcBuffer;
	CBitmap		m_bmBuffer;
	HBITMAP		m_hBuffer;
	CBitmap		m_bmWatermark;

// Operations
public:
	int				Update();
	void			OnSkinChange();
protected:
	CAlbumFolder*	GetSelectedAlbum() const;
	void			DoPaint(CDC* pDC, CRect& rcClient);
	void			DrawText(CDC* pDC, int nX, int nY, LPCTSTR pszText);

// Overrides
public:
	//{{AFX_VIRTUAL(CLibraryHeaderPanel)
	public:
	virtual BOOL Create(CWnd* pParentWnd);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CLibraryHeaderPanel)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#define ID_LIBRARY_HEADER	134

#endif // !defined(AFX_CTRLLIBRARYHEADERPANEL_H__9E5471F0_5957_470D_9B52_9C8D18A76C3D__INCLUDED_)
