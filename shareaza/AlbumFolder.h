//
// AlbumFolder.h
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

#if !defined(AFX_ALBUMFOLDER_H__168CEF4D_1C6F_4E24_A647_299CBAEA6670__INCLUDED_)
#define AFX_ALBUMFOLDER_H__168CEF4D_1C6F_4E24_A647_299CBAEA6670__INCLUDED_

#pragma once

class CLibrary;
class CLibraryFile;
class CCollectionFile;
class CSchema;
class CSchemaMember;
class CXMLElement;


class CAlbumFolder : public CComObject
{
// Construction
public:
	CAlbumFolder(CAlbumFolder* pParent = NULL, LPCTSTR pszSchemaURI = NULL, LPCTSTR pszName = NULL, BOOL bAutoDelete = FALSE);
	virtual ~CAlbumFolder();
	
	DECLARE_DYNAMIC(CAlbumFolder)
	
// Attributes
public:
	CAlbumFolder*	m_pParent;
	CObList			m_pFolders;
	CObList			m_pFiles;
public:
	CString			m_sSchemaURI;
	CSchema*		m_pSchema;
	CXMLElement*	m_pXML;
	BOOL			m_bCollSHA1;
	SHA1			m_pCollSHA1;
public:
	CString			m_sName;
	BOOL			m_bExpanded;
	BOOL			m_bAutoDelete;
	CString			m_sBestView;
public:
	DWORD				m_nUpdateCookie;
	DWORD				m_nSelectCookie;
	DWORD				m_nListCookie;
	CCollectionFile*	m_pCollection;
	
// Operations
public:
	CAlbumFolder*	AddFolder(LPCTSTR pszSchemaURI = NULL, LPCTSTR pszName = NULL, BOOL bAutoDelete = FALSE);
	POSITION		GetFolderIterator() const;
	CAlbumFolder*	GetNextFolder(POSITION& pos) const;
	CAlbumFolder*	GetFolder(LPCTSTR pszName) const;
	CAlbumFolder*	GetFolderByURI(LPCTSTR pszURI) const;
	int				GetFolderCount() const;
	BOOL			CheckFolder(CAlbumFolder* pFolder, BOOL bRecursive = FALSE) const;
	CAlbumFolder*	GetTarget(CSchemaMember* pMember, LPCTSTR pszValue) const;
	CAlbumFolder*	FindCollection(SHA1* pSHA1);
public:
	void			AddFile(CLibraryFile* pFile);
	POSITION		GetFileIterator() const;
	CLibraryFile*	GetNextFile(POSITION& pos) const;
	int				GetFileCount() const;
	int				GetSharedCount() const;
	void			RemoveFile(CLibraryFile* pFile);
	CAlbumFolder*	FindFile(CLibraryFile* pFile) const;
	int				GetFileList(CLibraryList* pList, BOOL bRecursive) const;
public:
	void			Delete(BOOL bIfEmpty = FALSE);
	BOOL			SetMetadata(CXMLElement* pXML);
	BOOL			MetaFromFile(CLibraryFile* pFile);
	BOOL			MetaToFiles(BOOL bAggressive = FALSE);
	BOOL			OrganiseFile(CLibraryFile* pFile);
	BOOL			MountCollection(SHA1* pSHA1, CCollectionFile* pCollection, BOOL bForce = FALSE);
	CCollectionFile*GetCollection();
	CString			GetBestView() const;
protected:
	void			SetCollection(SHA1* pSHA1, CCollectionFile* pCollection);
	void			OnFolderDelete(CAlbumFolder* pFolder);
	void			OnFileDelete(CLibraryFile* pFile);
	void			Serialize(CArchive& ar, int nVersion);
	void			Clear();
	
	friend class CLibrary;
	friend class CLibraryFolders;
	friend class CLibraryFrame;
	friend class CLibraryTreeView;
};

#endif // !defined(AFX_ALBUMFOLDER_H__168CEF4D_1C6F_4E24_A647_299CBAEA6670__INCLUDED_)
