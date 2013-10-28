//
// GraphBase.h
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

#if !defined(AFX_GRAPHBASE_H__63FD4BDF_BEBE_4526_94FC_746EE0641321__INCLUDED_)
#define AFX_GRAPHBASE_H__63FD4BDF_BEBE_4526_94FC_746EE0641321__INCLUDED_

#pragma once


class CGraphBase
{
// Construction
public:
	CGraphBase();
	virtual ~CGraphBase();

// Attributes
protected:
	CDC			m_pDC;
	CBitmap		m_pImage;
	HBITMAP		m_hOldImage;
	CSize		m_szImage;

// Operations
public:
	virtual void	CreateDefaults();
	virtual void	Serialize(CArchive& ar);
	virtual BOOL	Update();
	virtual void	Clear();
	virtual void	Paint(CDC* pDC, CRect* pRect);
	virtual void	BufferedPaint(CDC* pDC, CRect* pRect);

};

#endif // !defined(AFX_GRAPHBASE_H__63FD4BDF_BEBE_4526_94FC_746EE0641321__INCLUDED_)
