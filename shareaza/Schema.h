//
// Schema.h
//
// Copyright (c) Shareaza Development Team, 2002-2004.
// This file is part of SHAREAZA (www.shareaza.com)
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

#if !defined(AFX_SCHEMA_H__D6D5E129_7D37_4FBB_BBEA_320E4659D76D__INCLUDED_)
#define AFX_SCHEMA_H__D6D5E129_7D37_4FBB_BBEA_320E4659D76D__INCLUDED_

#pragma once

#include "SchemaMember.h"

class CSchema;
class CSchemaMember;
class CSchemaChild;
class CXMLElement;


class CSchema  
{
// Construction
public:
	CSchema();
	virtual ~CSchema();
	
// Attributes
public:
	int			m_nType;
	CString		m_sTitle;
	CString		m_sURI;
	CString		m_sPlural;
	CString		m_sSingular;
	int			m_nAvailability;
	BOOL		m_bPrivate;
	CString		m_sDonkeyType;
public:
	CPtrList	m_pMembers;
	CStringList	m_pExtends;
	CPtrList	m_pContains;
	CString		m_sDefaultColumns;
	CString		m_sTypeFilter;
	CPtrList	m_pBitziMap;
	CString		m_sBitziTest;
	CString		m_sLibraryView;
	CString		m_sHeaderTitle;
	CString		m_sHeaderSubtitle;
	CString		m_sTileLine1;
	CString		m_sTileLine2;
public:
	CString		m_sIcon;
	int			m_nIcon16;
	int			m_nIcon32;
	int			m_nIcon48;
	
	enum { stFile, stFolder };
	enum { saDefault, saAdvanced, saSystem, saMax };
	
// Operations
public:
	POSITION		GetMemberIterator() const;
	CSchemaMember*	GetNextMember(POSITION& pos) const;
	CSchemaMember*	GetMember(LPCTSTR pszName) const;
	int				GetMemberCount() const;
	CString			GetFirstMemberName() const;
	void			Clear();
	BOOL			Load(LPCTSTR pszName);
	CSchemaChild*	GetContained(LPCTSTR pszURI) const;
	CString			GetContainedURI(int nType) const;
	CXMLElement*	Instantiate(BOOL bNamespace = FALSE) const;
	BOOL			Validate(CXMLElement* pXML, BOOL bFix);
	CString			GetIndexedWords(CXMLElement* pXML) const;
	void			ResolveTokens(CString& str, CXMLElement* pXML) const;
protected:
	BOOL			LoadSchema(LPCTSTR pszFile);
	BOOL			LoadPrimary(CXMLElement* pRoot, CXMLElement* pType);
	CXMLElement*	GetType(CXMLElement* pRoot, LPCTSTR pszName);
	BOOL			LoadDescriptor(LPCTSTR pszFile);
	void			LoadDescriptorTitles(CXMLElement* pElement);
	void			LoadDescriptorIcons(CXMLElement* pElement);
	void			LoadDescriptorMembers(CXMLElement* pElement);
	void			LoadDescriptorTypeFilter(CXMLElement* pElement);
	void			LoadDescriptorExtends(CXMLElement* pElement);
	void			LoadDescriptorContains(CXMLElement* pElement);
	void			LoadDescriptorBitziImport(CXMLElement* pElement);
	void			LoadDescriptorHeaderContent(CXMLElement* pElement);
	void			LoadDescriptorViewContent(CXMLElement* pElement);
	BOOL			LoadIcon();

// Inlines
public:
	inline BOOL Equals(CSchema* pSchema) const
	{
		if ( ! pSchema ) return FALSE;
		if ( this == pSchema ) return TRUE;
		
		for ( POSITION pos = m_pExtends.GetHeadPosition() ; pos ; )
		{
			CString strURI = m_pExtends.GetNext( pos );
			if ( strURI.CompareNoCase( pSchema->m_sURI ) == 0 ) return TRUE;
		}
		
		return FALSE;
	}

	inline BOOL CheckURI(LPCTSTR pszURI) const
	{
		if ( ! pszURI || ! this ) return FALSE;
		if ( m_sURI.CompareNoCase( pszURI ) == 0 ) return TRUE;
		
		for ( POSITION pos = m_pExtends.GetHeadPosition() ; pos ; )
		{
			CString strURI = m_pExtends.GetNext( pos );
			if ( strURI.CompareNoCase( pszURI ) == 0 ) return TRUE;
		}
		
		return FALSE;
	}

	inline BOOL FilterType(LPCTSTR pszFile, BOOL bDefault = FALSE) const
	{
		if ( m_sTypeFilter.IsEmpty() ) return bDefault;

		LPCTSTR pszExt = _tcsrchr( pszFile, '.' );
		if ( pszExt == NULL ) return FALSE;

		CString strExt = _T("|");
		strExt += pszExt;
		strExt += '|';
		strExt = CharLower( strExt.GetBuffer() );

		return m_sTypeFilter.Find( strExt ) >= 0;
	}


// Common Schemas
public:
	static LPCTSTR	uriApplication;
	static LPCTSTR	uriAudio;
	static LPCTSTR	uriBook;
	static LPCTSTR	uriImage;
	static LPCTSTR	uriVideo;
	static LPCTSTR	uriROM;
public:
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
	
	friend class CSchemaMember;
};


class CSchemaBitzi
{
public:
	CString		m_sFrom;
	CString		m_sTo;
	double		m_nFactor;
public:
	BOOL		Load(CXMLElement* pXML);
};

#endif // !defined(AFX_SCHEMA_H__D6D5E129_7D37_4FBB_BBEA_320E4659D76D__INCLUDED_)
