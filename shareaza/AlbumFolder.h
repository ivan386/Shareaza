//
// AlbumFolder.h
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

#include "Schema.h"

class CCollectionFile;
class CLibraryFile;
class CLibraryList;
class CSchemaMember;
class CXMLElement;


class CAlbumFolder : private boost::noncopyable
{
// Construction
public:
	CAlbumFolder(CAlbumFolder* pParent = NULL, LPCTSTR pszSchemaURI = NULL, LPCTSTR pszName = NULL, BOOL bAutoDelete = FALSE);
	virtual ~CAlbumFolder();

// Attributes
public:
	CAlbumFolder*			m_pParent;
	CList< CAlbumFolder* >	m_pFolders;
	CList< CLibraryFile* >	m_pFiles;
	CString					m_sSchemaURI;
	CSchemaPtr				m_pSchema;
	CXMLElement*			m_pXML;
	Hashes::Sha1Hash		m_oCollSHA1;
	CString					m_sName;
	BOOL					m_bExpanded;
	BOOL					m_bAutoDelete;
	CString					m_sBestView;
	DWORD					m_nUpdateCookie;
	DWORD					m_nSelectCookie;
	DWORD					m_nListCookie;
	Hashes::Guid			m_oGUID;

private:
	CCollectionFile*		m_pCollection;

// Operations
public:
	CAlbumFolder*	AddFolder(LPCTSTR pszSchemaURI = NULL, LPCTSTR pszName = NULL, BOOL bAutoDelete = FALSE);
	POSITION		GetFolderIterator() const;
	CAlbumFolder*	GetNextFolder(POSITION& pos) const;
	CAlbumFolder*	GetFolder(LPCTSTR pszName) const;
	CAlbumFolder*	GetFolderByURI(LPCTSTR pszURI) const;
	INT_PTR			GetFolderCount() const { return m_pFolders.GetCount(); }
	BOOL			CheckFolder(CAlbumFolder* pFolder, BOOL bRecursive = FALSE) const;
	CAlbumFolder*	GetTarget(CSchemaMember* pMember, LPCTSTR pszValue) const;
	CAlbumFolder*	FindCollection(const Hashes::Sha1Hash& oSHA1);
	CAlbumFolder*	FindFolder(const Hashes::Guid& oGUID);
public:
	void			AddFile(CLibraryFile* pFile);
	POSITION		GetFileIterator() const;
	CLibraryFile*	GetNextFile(POSITION& pos) const;
	INT_PTR			GetFileCount() const { return m_pFiles.GetCount(); }
	int				GetSharedCount() const;
	void			RemoveFile(CLibraryFile* pFile);
	CAlbumFolder*	FindFile(CLibraryFile* pFile);
	int				GetFileList(CLibraryList* pList, BOOL bRecursive) const;
public:
	void			Delete(BOOL bIfEmpty = FALSE);
	BOOL			SetMetadata(CXMLElement* pXML);
	BOOL			MetaFromFile(CLibraryFile* pFile);
	BOOL			MetaToFiles(BOOL bAggressive = FALSE);
	BOOL			OrganiseFile(CLibraryFile* pFile);
	BOOL			MountCollection(const Hashes::Sha1Hash& oSHA1, CCollectionFile* pCollection, BOOL bForce = FALSE);
	CCollectionFile*GetCollection();
	CString			GetBestView() const;
	void			Serialize(CArchive& ar, int nVersion);
	bool			operator==(const CAlbumFolder& val) const;
	void			RenewGUID();
	void			SetCollection(const Hashes::Sha1Hash& oSHA1, CCollectionFile* pCollection);
	bool			OnFolderDelete(CAlbumFolder* pFolder);
	void			OnFileDelete(CLibraryFile* pFile, BOOL bDeleteGhost = FALSE);
	void			Clear();
};
