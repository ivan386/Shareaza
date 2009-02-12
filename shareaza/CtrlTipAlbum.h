//
// CtrlTipAlbum.h
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

#if !defined(AFX_CTRLTIPALBUM_H__0E00863D_B269_42BF_9A76_D145F7560390__INCLUDED_)
#define AFX_CTRLTIPALBUM_H__0E00863D_B269_42BF_9A76_D145F7560390__INCLUDED_

#pragma once

#include "CtrlCoolTip.h"
#include "MetaList.h"


class CAlbumTipCtrl : public CCoolTipCtrl
{
// Construction
public:
	CAlbumTipCtrl();
	virtual ~CAlbumTipCtrl();

	DECLARE_DYNAMIC(CAlbumTipCtrl)

// Attributes
protected:
	CString			m_sName;
	CString			m_sType;
	int				m_nIcon32;
	int				m_nIcon48;
	BOOL			m_bCollection;
	CMetaList		m_pMetadata;
	int				m_nKeyWidth;
	COLORREF		m_crLight;

// Operations
public:
	virtual BOOL OnPrepare();
	virtual void OnCalcSize(CDC* pDC);
	virtual void OnPaint(CDC* pDC);
protected:
	void		DrawThumb(CDC* pDC, CRect& rcThumb);

// Overrides
public:
	//{{AFX_VIRTUAL(CAlbumTipCtrl)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAlbumTipCtrl)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLTIPALBUM_H__0E00863D_B269_42BF_9A76_D145F7560390__INCLUDED_)
