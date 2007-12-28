//
// MetaPanel.h
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

#if !defined(AFX_METAPANEL_H__D5B8BC2C_86FB_4B82_B0B9_F82623F9CB3F__INCLUDED_)
#define AFX_METAPANEL_H__D5B8BC2C_86FB_4B82_B0B9_F82623F9CB3F__INCLUDED_

#pragma once

#include "MetaList.h"


class CMetaPanel : public CMetaList
{
// Construction
public:
	CMetaPanel();
	virtual ~CMetaPanel();

// Operations
public:
	int		Layout(CDC* pDC, int nWidth);
	void	Paint(CDC* pDC, const CRect* prcArea);
	BOOL	OnClick(const CPoint& point);

// Attributes
public:
	int		m_nHeight;

protected:
    CBitmap m_bmMusicBrainz;
};

#endif // !defined(AFX_METAPANEL_H__D5B8BC2C_86FB_4B82_B0B9_F82623F9CB3F__INCLUDED_)
