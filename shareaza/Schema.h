//
// Schema.h
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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
class CSchemaChild;
class CXMLElement;

typedef const CSchema* CSchemaPtr;
typedef const CSchemaChild* CSchemaChildPtr;

#ifdef _WIN64

template<>
AFX_INLINE UINT AFXAPI HashKey( CSchemaPtr key )
{
	return HashKey< __int64 >( (__int64)key );
}

#endif // _WIN64

#include "SchemaMember.h"

class CSchema  
{
public:
	CSchema();
	~CSchema();
	
	enum Type { stAny = -1, stFile, stFolder };
	enum Availability { saDefault, saAdvanced, saSystem, saMax };
	
	Type		m_nType;
	CString		m_sTitle;
	CString		m_sPlural;
	CString		m_sSingular;
	Availability m_nAvailability;
	BOOL		m_bPrivate;
	CString		m_sDonkeyType;

	CList< CSchemaMember* >	m_pMembers;
	CList< CString >		m_pExtends;
	CList< CSchemaChildPtr >m_pContains;
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

	POSITION		GetFilterIterator() const;
	void			GetNextFilter(POSITION& pos, CString& sType, BOOL& bResult) const;
	BOOL			FilterType(LPCTSTR pszFile) const;
	CString			GetFilterSet() const;

	POSITION		GetMemberIterator() const;
	CSchemaMemberPtr GetNextMember(POSITION& pos) const;
	CSchemaMemberPtr GetMember(LPCTSTR pszName) const;
	INT_PTR			GetMemberCount() const;
	const CXMLElement* GetType(const CXMLElement* pRoot, LPCTSTR pszName) const;
	CString			GetFirstMemberName() const;
	void			Clear();
	BOOL			Load(LPCTSTR pszName);
	CSchemaChildPtr	GetContained(LPCTSTR pszURI) const;
	CString			GetContainedURI(Type nType) const;
	CXMLElement*	Instantiate(BOOL bNamespace = FALSE) const;
	BOOL			Validate(CXMLElement* pXML, BOOL bFix) const;
	CString			GetIndexedWords(const CXMLElement* pXML) const;
	CString			GetVisibleWords(const CXMLElement* pXML) const;
	void			ResolveTokens(CString& str, CXMLElement* pXML) const;

protected:
	CString			m_sURI;
	typedef CAtlMap < CString, BOOL, CStringElementTraitsI< CString > > CSBMap;
	CSBMap			m_pTypeFilters;

	BOOL			LoadSchema(LPCTSTR pszFile);
	BOOL			LoadPrimary(const CXMLElement* pRoot, const CXMLElement* pType);
	BOOL			LoadDescriptor(LPCTSTR pszFile);
	void			LoadDescriptorTitles(const CXMLElement* pElement);
	void			LoadDescriptorIcons(const CXMLElement* pElement);
	void			LoadDescriptorMembers(const CXMLElement* pElement);
	void			LoadDescriptorTypeFilter(const CXMLElement* pElement);
	void			LoadDescriptorExtends(const CXMLElement* pElement);
	void			LoadDescriptorContains(const CXMLElement* pElement);
	void			LoadDescriptorHeaderContent(const CXMLElement* pElement);
	void			LoadDescriptorViewContent(const CXMLElement* pElement);
	BOOL			LoadIcon();

	CSchemaMember*	GetWritableMember(LPCTSTR pszName) const;

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
			const CString strURI = m_pExtends.GetNext( pos );
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

private:
	CSchema(const CSchema&);
	CSchema& operator=(const CSchema&);
};

#define NO_VALUE		(_T("(~ns~)"))
#define MULTI_VALUE		(_T("(~mt~)"))
