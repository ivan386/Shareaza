//
// LibraryFolders.h
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

#pragma once

class CLibraryFolder;
class CAlbumFolder;
class CLibraryFile;
class CXMLElement;
class CCollectionFile;


class CLibraryFolders : public CComObject
{
// Construction
public:
	CLibraryFolders();
	virtual ~CLibraryFolders();

	DECLARE_DYNAMIC(CLibraryFolders)

// Attributes
protected:
	CList< CLibraryFolder* > m_pFolders;
	CAlbumFolder*	m_pAlbumRoot;

// Physical Folder Operations
public:
	POSITION		GetFolderIterator() const;
	CLibraryFolder*	GetNextFolder(POSITION& pos) const;
	INT_PTR			GetFolderCount() const { return m_pFolders.GetCount(); }
	CLibraryFolder*	GetFolder(LPCTSTR pszPath) const;
	BOOL			CheckFolder(CLibraryFolder* pFolder, BOOL bRecursive = FALSE) const;
public:
	CLibraryFolder*	AddFolder(LPCTSTR pszPath);
	CLibraryFolder*	AddFolder(LPCTSTR pszPath, BOOL bShared);
	BOOL			RemoveFolder(CLibraryFolder* pFolder);
	CLibraryFolder*	IsFolderShared(LPCTSTR pszPath);
	CLibraryFolder*	IsSubFolderShared(LPCTSTR pszPath);
	static BOOL		IsShareable(LPCTSTR pszPath);
	void			Maintain();

// Virtual Album Operations
public:
	CAlbumFolder*	GetAlbumRoot();
	BOOL			CheckAlbum(CAlbumFolder* pFolder) const;
	CAlbumFolder*	GetAlbumTarget(LPCTSTR pszSchemaURI, LPCTSTR pszMember, LPCTSTR pszValue) const;
    CAlbumFolder*	GetCollection(const Hashes::Sha1Hash& oSHA1);
    BOOL			MountCollection(const Hashes::Sha1Hash& oSHA1, CCollectionFile* pCollection);
protected:
	void			CreateAlbumTree();
	void			OnFileDelete(CLibraryFile* pFile, BOOL bDeleteGhost = FALSE);

// Core
protected:
	void			Clear();
	BOOL			ThreadScan(volatile BOOL* pbContinue, const BOOL bForce = FALSE);
	void			Serialize(CArchive& ar, int nVersion);

// COM
protected:
	BEGIN_INTERFACE_PART(LibraryFolders, ILibraryFolders)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_Library)(ILibrary FAR* FAR* ppLibrary);
		STDMETHOD(get__NewEnum)(IUnknown FAR* FAR* ppEnum);
		STDMETHOD(get_Item)(VARIANT vIndex, ILibraryFolder FAR* FAR* ppFolder);
		STDMETHOD(get_Count)(LONG FAR* pnCount);
	END_INTERFACE_PART(LibraryFolders)

	DECLARE_INTERFACE_MAP()

	friend class CLibrary;
};

extern CLibraryFolders LibraryFolders;
