//
// CollectionFile.h
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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

#include "ShareazaFile.h"

class CZIPFile;
class CXMLElement;
class CLibraryFile;


class CCollectionFile : public CComObject
{
// Construction
public:
	CCollectionFile();
	virtual ~CCollectionFile();
	DECLARE_DYNAMIC(CCollectionFile)

	enum CollectionType { ShareazaCollection, SimpleCollection };

// Member File Class
public:
	class File : public CShareazaFile
	{
	// Construction
	public:
		File(CCollectionFile* pParent);
		virtual ~File();

	// Attributes
	public:
		CCollectionFile*	m_pParent;
		CXMLElement*		m_pMetadata;
//		CString				m_sSource;

	// Operations
	public:
		BOOL	Parse(CXMLElement* pXML);	// Load from XML
		BOOL	Parse(CFile& pFile);		// Load from .emulecollection-file
		BOOL	Parse(LPCTSTR szText);		// Load from text line
		BOOL	IsComplete() const;
		BOOL	IsDownloading() const;
		BOOL	Download();
		BOOL	ApplyMetadata(CLibraryFile* pShared);

	};

// Operations
public:
	BOOL		Open(LPCTSTR lpszFileName);
	void		Close();
	void		Render(CString& strBuffer) const; // Render file list as HTML

	File*		FindByURN(LPCTSTR pszURN);
	File*		FindFile(CLibraryFile* pShared, BOOL bApply = FALSE);
	int			GetMissingCount() const;

protected:
	BOOL		LoadShareaza(LPCTSTR pszFile);	// Load zipped Shareaza collection
	BOOL		LoadEMule(LPCTSTR pszFile);		// Load binary eMule collection
	BOOL		LoadDC(LPCTSTR pszFile);		// Load DC++ file listing
	void		LoadDC(CXMLElement* pRoot);		// Load DC++ file listing
	BOOL		LoadText(LPCTSTR pszFile);		// Load simple text file with links
	static CXMLElement* CloneMetadata(CXMLElement* pMetadata);

// Attributes
protected:
	CList< File* >	m_pFiles;
	CString			m_sTitle;
	CString			m_sThisURI;
	CString			m_sParentURI;
	CXMLElement*	m_pMetadata;
	CollectionType	m_nType;

// Inlines
public:
	inline BOOL IsOpen() const
	{
		return ( m_pFiles.GetCount() > 0 );
	}

	inline POSITION GetFileIterator() const
	{
		return m_pFiles.GetHeadPosition();
	}

	inline File* GetNextFile(POSITION& pos) const
	{
		return m_pFiles.GetNext( pos );
	}

	inline INT_PTR GetFileCount() const
	{
		return m_pFiles.GetCount();
	}

	inline CString GetTitle() const
	{
		return m_sTitle;
	}

	inline CString GetThisURI() const
	{
		return m_sThisURI;
	}

	inline CString GetParentURI() const
	{
		return m_sParentURI;
	}

	inline CXMLElement* GetMetadata() const
	{
		return m_pMetadata;
	}

	inline bool IsType(CollectionType nType) const
	{
		return m_nType == nType;
	}
};
