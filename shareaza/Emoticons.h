//
// Emoticons.h
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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

class CImageFile;
class CRichDocument;


class CEmoticons
{
// Construction
public:
	CEmoticons();
	virtual ~CEmoticons();

// Attributes
public:
	CImageList			m_pImage;
	CArray< CString >	m_pIndex;
	LPTSTR				m_pTokens;
	CArray< UINT >		m_pButtons;

// Operations
public:
	LPCTSTR	FindNext(LPCTSTR pszText, int* pnIndex);
	int		Lookup(LPCTSTR pszText, int nLen = -1) const;
	LPCTSTR	GetText(int nIndex) const;
	void	Draw(CDC* pDC, int nIndex, int nX, int nY, COLORREF crBack = CLR_NONE);
	CMenu*	CreateMenu();
	void	FormatText(CRichDocument* pDocument, LPCTSTR pszBody, BOOL bNewlines = FALSE, COLORREF cr = 0);
public:
	BOOL	Load();
	void	Clear();
protected:
	int		AddEmoticon(LPCTSTR pszText, CImageFile* pImage, CRect* pRect, COLORREF crBack, BOOL bButton);
	void	BuildTokens();
	BOOL	LoadTrillian(LPCTSTR pszFile);


};

extern CEmoticons Emoticons;
