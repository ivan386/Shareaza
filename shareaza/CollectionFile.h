//
// CollectionFile.h
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

#pragma once

#include "Hashes.h"

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
	
// Member File Class
public:
	class File
	{
	// Construction
	public:
		File(CCollectionFile* pParent);
		~File();
		
	// Attributes
	public:
		CCollectionFile*	m_pParent;
		CManagedSHA1		m_oSHA1;
		CManagedMD5			m_oMD5;
		CManagedTiger		m_oTiger;
		CManagedED2K		m_oED2K;
	public:
		CString				m_sName;
		QWORD				m_nSize;
		CXMLElement*		m_pMetadata;
		CString				m_sSource;
		
	// Operations
	public:
		BOOL	Parse(CXMLElement* pXML);
		BOOL	IsComplete() const;
		BOOL	IsDownloading() const;
		BOOL	Download();
		BOOL	ApplyMetadata(CLibraryFile* pShared);
		
	};
	
// Operations
public:
	BOOL		Open(LPCTSTR pszFile);
	BOOL		Attach(HANDLE hFile);
	void		Close();
public:
	File*		FindByURN(LPCTSTR pszURN);
	File*		FindFile(CLibraryFile* pShared, BOOL bApply = FALSE);
	int			GetMissingCount();
protected:
	BOOL		LoadManifest(CZIPFile& pZIP);
	static CXMLElement* CloneMetadata(CXMLElement* pMetadata);
	
// Attributes
protected:
	CPtrList		m_pFiles;
	CString			m_sTitle;
	CString			m_sThisURI;
	CString			m_sParentURI;
	CXMLElement*	m_pMetadata;
	
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
		return (File*)m_pFiles.GetNext( pos );
	}
	
	inline int GetFileCount() const
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
};
