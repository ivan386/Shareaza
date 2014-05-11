//
// Schema.h
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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

class CSchema;
class CSchemaMember;
class CSchemaChild;
class CXMLElement;

typedef const CSchema* CSchemaPtr;

#include "SchemaMember.h"

class CSchema  
{
public:
	CSchema();
	virtual ~CSchema();
	
	int			m_nType;
	CString		m_sTitle;
	CString		m_sPlural;
	CString		m_sSingular;
	int			m_nAvailability;
	BOOL		m_bPrivate;
	CString		m_sDonkeyType;

	CList< CSchemaMember* >	m_pMembers;
	CList< CString >	m_pExtends;
	CList< CSchemaChild* >	m_pContains;
	CString		m_sDefaultColumns;
	CString		m_sLibraryView;
	CString		m_sHeaderTitle;
	CString		m_sHeaderSubtitle;
	CString		m_sTileLine1;
	CString		m_sTileLine2;

	CString		m_sIcon;
	int			m_nIcon16;
	int			m_nIcon32;
	int			m_nIcon48;
	
	enum { stFile, stFolder };
	enum { saDefault, saAdvanced, saSystem, saMax };

	POSITION		GetFilterIterator() const;
	void			GetNextFilter(POSITION& pos, CString& sType, BOOL& bResult) const;
	BOOL			FilterType(LPCTSTR pszFile) const;
	CString			GetFilterSet() const;

	POSITION		GetMemberIterator() const;
	CSchemaMember*	GetNextMember(POSITION& pos) const;
	CSchemaMember*	GetMember(LPCTSTR pszName) const;
	INT_PTR			GetMemberCount() const;
	CString			GetFirstMemberName() const;
	void			Clear();
	BOOL			Load(LPCTSTR pszName);
	CSchemaChild*	GetContained(LPCTSTR pszURI) const;
	CString			GetContainedURI(int nType) const;
	CXMLElement*	Instantiate(BOOL bNamespace = FALSE) const;
	BOOL			Validate(CXMLElement* pXML, BOOL bFix) const;
	CString			GetIndexedWords(CXMLElement* pXML) const;
	CString			GetVisibleWords(CXMLElement* pXML) const;
	void			ResolveTokens(CString& str, CXMLElement* pXML) const;

protected:
	CString			m_sURI;
	typedef CMap < CString, const CString&, BOOL, BOOL& > CSBMap;
	CSBMap			m_pTypeFilters;

	BOOL			LoadSchema(LPCTSTR pszFile);
	BOOL			LoadPrimary(CXMLElement* pRoot, CXMLElement* pType);
	CXMLElement*	GetType(CXMLElement* pRoot, LPCTSTR pszName) const;
	BOOL			LoadDescriptor(LPCTSTR pszFile);
	void			LoadDescriptorTitles(CXMLElement* pElement);
	void			LoadDescriptorIcons(CXMLElement* pElement);
	void			LoadDescriptorMembers(CXMLElement* pElement);
	void			LoadDescriptorTypeFilter(CXMLElement* pElement);
	void			LoadDescriptorExtends(CXMLElement* pElement);
	void			LoadDescriptorContains(CXMLElement* pElement);
	void			LoadDescriptorHeaderContent(CXMLElement* pElement);
	void			LoadDescriptorViewContent(CXMLElement* pElement);
	BOOL			LoadIcon();

// Inlines
public:
	inline CString GetURI() const
	{
		return m_sURI;
	}

	inline bool Equals(CSchemaPtr pSchema) const
	{
		return ( pSchema && ( ( this == pSchema ) || CheckURI( pSchema->m_sURI ) ) );
	}

	inline bool CheckURI(LPCTSTR pszURI) const
	{
		if ( ! pszURI ) return false;
		if ( m_sURI.CompareNoCase( pszURI ) == 0 ) return true;
		for ( POSITION pos = m_pExtends.GetHeadPosition() ; pos ; )
		{
			CString strURI = m_pExtends.GetNext( pos );
			if ( strURI.CompareNoCase( pszURI ) == 0 ) return true;
		}
		return false;
	}

// Common Schemas
public:
	static LPCTSTR	uriApplication;
	static LPCTSTR	uriAudio;
	static LPCTSTR	uriArchive;
	static LPCTSTR	uriBook;
	static LPCTSTR	uriImage;
	static LPCTSTR	uriVideo;
	static LPCTSTR	uriROM;
	static LPCTSTR	uriDocument;
	static LPCTSTR	uriSpreadsheet;
	static LPCTSTR	uriPresentation;
	static LPCTSTR	uriCollection;
	static LPCTSTR	uriLibrary;
	static LPCTSTR	uriFolder;
	static LPCTSTR	uriCollectionsFolder;
	static LPCTSTR	uriFavouritesFolder;
	static LPCTSTR	uriSearchFolder;
	static LPCTSTR	uriAllFiles;
	static LPCTSTR	uriApplicationRoot;
	static LPCTSTR	uriApplicationAll;
	static LPCTSTR	uriBookRoot;
	static LPCTSTR	uriBookAll;
	static LPCTSTR	uriImageRoot;
	static LPCTSTR	uriImageAll;
	static LPCTSTR	uriImageAlbum;
	static LPCTSTR	uriMusicRoot;
	static LPCTSTR	uriMusicAll;
	static LPCTSTR	uriMusicAlbumCollection;
	static LPCTSTR	uriMusicArtistCollection;
	static LPCTSTR	uriMusicGenreCollection;
	static LPCTSTR	uriMusicAlbum;
	static LPCTSTR	uriMusicArtist;
	static LPCTSTR	uriMusicGenre;
	static LPCTSTR	uriVideoRoot;
	static LPCTSTR	uriVideoAll;
	static LPCTSTR	uriVideoSeriesCollection;
	static LPCTSTR	uriVideoSeries;
	static LPCTSTR	uriVideoFilmCollection;
	static LPCTSTR	uriVideoFilm;
	static LPCTSTR	uriVideoMusicCollection;
	static LPCTSTR	uriDocumentRoot;
	static LPCTSTR	uriDocumentAll;
	static LPCTSTR	uriGhostFolder;
	static LPCTSTR	uriComments;
	static LPCTSTR	uriBitTorrent;

	friend class CSchemaMember;

private:
	CSchema(const CSchema&);
	CSchema& operator=(const CSchema&);
};
