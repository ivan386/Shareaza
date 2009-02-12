//
// RichFragment.h
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

#if !defined(AFX_RICHFRAGMENT_H__B52AE063_BDD3_41D8_BE1D_D78C47A634A6__INCLUDED_)
#define AFX_RICHFRAGMENT_H__B52AE063_BDD3_41D8_BE1D_D78C47A634A6__INCLUDED_

#pragma once

class CRichElement;
class CRichViewCtrl;


class CRichFragment
{
// Construction
public:
	CRichFragment(CRichElement* pElement, int nOffset, int nLength, CPoint* pPoint, CSize* pSize);
	CRichFragment(CRichElement* pElement, CPoint* pPoint);
	virtual ~CRichFragment();

// Attributes
public:
	CRichElement*	m_pElement;
	CPoint			m_pt;
	CSize			m_sz;
	WORD			m_nOffset;
	WORD			m_nLength;

// Operations
public:
	void	Add(int nLength, CSize* pSize);
	void	Paint(CDC* pDC, CRichViewCtrl* pCtrl, int nFragment);

};

#endif // !defined(AFX_RICHFRAGMENT_H__B52AE063_BDD3_41D8_BE1D_D78C47A634A6__INCLUDED_)
