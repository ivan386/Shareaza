//
// CtrlBrowseHeader.h
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

#if !defined(AFX_CTRLBROWSEHEADER_H__B41F0189_568D_4748_B329_4E248FFFB648__INCLUDED_)
#define AFX_CTRLBROWSEHEADER_H__B41F0189_568D_4748_B329_4E248FFFB648__INCLUDED_

#pragma once

#include "CtrlCoolBar.h"

class CHostBrowser;


class CBrowseHeaderCtrl : public CCoolBarCtrl
{
// Construction
public:
	CBrowseHeaderCtrl();
	virtual ~CBrowseHeaderCtrl();

// Attributes
protected:
	CString	m_sTitle;
	CString	m_sIntro;
	int		m_nIcon32;
	int		m_nIcon48;

// Operations
public:
	void	Update(CHostBrowser* pBrowser);
	void	OnSkinChange();
protected:
	virtual void PrepareRect(CRect* pRect) const;
	virtual void DoPaint(CDC* pDC, CRect& rcBar, BOOL bTransparent);

// Overrides
public:
	//{{AFX_VIRTUAL(CBrowseHeaderCtrl)
	public:
	virtual BOOL Create(CWnd* pParentWnd);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CBrowseHeaderCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#define IDC_BROWSE_HEADER	123

#endif // !defined(AFX_CTRLBROWSEHEADER_H__B41F0189_568D_4748_B329_4E248FFFB648__INCLUDED_)
