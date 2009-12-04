//
// RelatedSearch.h
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

#if !defined(AFX_RELATEDSEARCH_H__B8423011_3B98_44E5_9B09_40AA1E08EA4A__INCLUDED_)
#define AFX_RELATEDSEARCH_H__B8423011_3B98_44E5_9B09_40AA1E08EA4A__INCLUDED_

#pragma once

#include "ShareazaFile.h"

class CXMLElement;
class CMatchFile;
class CLibraryFile;


class CRelatedSearch : public CShareazaFile
{
// Construction
public:
	CRelatedSearch(CMatchFile* pFile);
	CRelatedSearch(CLibraryFile* pFile);
	virtual ~CRelatedSearch();

// Attributes
public:
	CSchemaPtr		m_pSchema;
	CXMLElement*	m_pXML;
	BOOL			m_bXML;

// Operations
public:
	BOOL		CanSearchForThis();
	BOOL		RunSearchForThis();
public:
	BOOL		CanSearchForSimilar();
	BOOL		RunSearchForSimilar();
public:
	BOOL		CanSearchForArtist();
	BOOL		RunSearchForArtist();
public:
	BOOL		CanSearchForAlbum();
	BOOL		RunSearchForAlbum();
public:
	BOOL		CanSearchForSeries();
	BOOL		RunSearchForSeries();
protected:
	static CString Tokenise(LPCTSTR psz);

};

#endif // !defined(AFX_RELATEDSEARCH_H__B8423011_3B98_44E5_9B09_40AA1E08EA4A__INCLUDED_)
