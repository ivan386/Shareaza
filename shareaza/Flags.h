//
// Flags.h
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

#if !defined(AFX_FLAGS_H__0D66330C_DB37_4BFF_B2D8_02065ED19D08__INCLUDED_)
#define AFX_FLAGS_H__0D66330C_DB37_4BFF_B2D8_02065ED19D08__INCLUDED_

#pragma once

class CImageFile;

class CFlags
{
// Construction
public:
	CFlags();
	virtual ~CFlags();

// Attributes
public:
	CImageList			m_pImage;

// Operations
public:
	BOOL	Load();
	void	Clear();
	int		GetFlagIndex(CString sCountry);
protected:
	void	AddFlag(CImageFile* pImage, CRect* pRect, COLORREF crBack);
};

extern CFlags Flags;

#endif // !defined(AFX_FLAGS_H__0D66330C_DB37_4BFF_B2D8_02065ED19D08__INCLUDED_)