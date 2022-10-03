//
// AlbumFolder.h
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

#include "Schema.h"

class CCollectionFile;
class CLibraryFile;
class CLibraryList;
class CXMLElement;


class CAlbumFolder
{
public:
	CAlbumFolder(CAlbumFolder* pParent = NULL, LPCTSTR pszSchemaURI = NULL, LPCTSTR pszName = NULL, BOOL bAutoDelete = FALSE);
	virtual ~CAlbumFolder();

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

// Operations
public:
	void			AddFolder(CAlbumFolder* pFolder);
	CAlbumFolder*	AddFolder(LPCTSTR pszSchemaURI = NULL, LPCTSTR pszName = NULL, BOOL bAutoDelete = FALSE);
	POSITION		GetFolderIterator() const;
	CAlbumFolder*	GetNextFolder(POSITION& pos) const;
	CAlbumFolder*	GetParent() const;
	CAlbumFolder*	GetFolder(LPCTSTR pszName) const;
	CAlbumFolder*	GetFolderByURI(LPCTSTR pszURI) const;
	DWORD			GetFolderCount() const;
	BOOL			CheckFolder(CAlbumFolder* pFolder, BOOL bRecursive = FALSE) const;
	CAlbumFolder*	GetTarget(CSchemaMemberPtr pMember, LPCTSTR pszValue) const;
	CAlbumFolder*	FindCollection(const Hashes::Sha1Hash& oSHA1);
	CAlbumFolder*	FindFolder(const Hashes::Guid& oGUID);

	void			AddFile(CLibraryFile* pFile);
	POSITION		GetFileIterator() const;
	CLibraryFile*	GetNextFile(POSITION& pos) const;
	DWORD			GetFileCount(BOOL bRecursive = FALSE) const;
	QWORD			GetFileVolume(BOOL bRecursive = FALSE) const;
	DWORD			GetSharedCount(BOOL bRecursive = FALSE) const;
	void			RemoveFile(CLibraryFile* pFile);
	const CAlbumFolder*	FindFile(const CLibraryFile* pFile) const;
	int				GetFileList(CLibraryList* pList, BOOL bRecursive) const;

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
	CXMLElement*	CreateXML() const;

protected:
	CAlbumFolder*			m_pParent;
	CList< CAlbumFolder* >	m_pFolders;
	CList< CLibraryFile* >	m_pFiles;
	CCollectionFile*		m_pCollection;

	CXMLElement*	CopyMetadata(CXMLElement* pMetadata) const;

private:
	CAlbumFolder(const CAlbumFolder&);
	CAlbumFolder& operator=(const CAlbumFolder&);
};
