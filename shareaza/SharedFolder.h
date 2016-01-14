//
// SharedFolder.h
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

#include "LibraryFolders.h"

class CLibraryList;
class CXMLElement;


class CLibraryFolder : public CComObject
{
	DECLARE_DYNAMIC( CLibraryFolder )

public:
	CLibraryFolder(CLibraryFolder* pParent, LPCTSTR pszPath = _T("") );
	virtual ~CLibraryFolder();

// Attributes
public:
	DWORD			m_nUpdateCookie;
	DWORD			m_nSelectCookie;
	CLibraryFolder*	m_pParent;
	CString			m_sPath;
	LPCTSTR			m_sName;			// Points to name part of m_sPath
	BOOL			m_bExpanded;
	DWORD			m_nFiles;
	QWORD			m_nVolume;

protected:
	typedef CAtlMap< CString, CLibraryFile*, CStringElementTraitsI< CString > > CFileMap;
	typedef CAtlMap< CString, CLibraryFolder*, CStringElementTraitsI< CString > > CFolderMap;

	DWORD			m_nScanCookie;
	TRISTATE		m_bShared;
	CFileMap		m_pFiles;
	CFolderMap		m_pFolders;
	HANDLE			m_hMonitor;
	BOOL			m_bForceScan;		// TRUE - next scan forced (root folder only)
	BOOL			m_bOffline;			// TRUE - folder absent (root folder only)
	Hashes::Guid	m_oGUID;

// Operations
public:
	CXMLElement*	CreateXML(CXMLElement* pRoot, BOOL bSharedOnly, XmlType nType) const;
	POSITION		GetFolderIterator() const;
	CLibraryFolder*	GetNextFolder(POSITION& pos) const;
	CLibraryFolder*	GetFolderByName(LPCTSTR pszName) const;
	CLibraryFolder*	GetFolderByPath(const CString& strPath) const;
	BOOL			CheckFolder(CLibraryFolder* pFolder, BOOL bRecursive = FALSE) const;
	DWORD			GetFolderCount() const;
	// Add new or get existing file
	CLibraryFile*	AddFile(LPCTSTR szName, BOOL& bNew);
	// Remove existing file recursively
	// Returns: TRUE - file found and removed, FALSE - not found
	BOOL			OnFileDelete(CLibraryFile* pRemovingFile);
	POSITION		GetFileIterator() const;
	CLibraryFile*	GetNextFile(POSITION& pos) const;
	CLibraryFile*	GetFile(LPCTSTR pszName) const;
	DWORD			GetFileCount() const;
	DWORD			GetFileList(CLibraryList* pList, BOOL bRecursive) const;
	DWORD			GetSharedCount(BOOL bRecursive) const;
	CString			GetRelativeName() const;

	void			Scan();
	BOOL			IsShared() const;
	// Set to TRUE if shared, to FALSE if not, and leave unchanged if unknown
	void			GetShared(BOOL& bShared) const;
	void			SetShared(TRISTATE bShared);
	BOOL			IsOffline() const;
	BOOL			SetOffline();
	BOOL			SetOnline();
	void			Serialize(CArchive& ar, int nVersion);
	BOOL			ThreadScan(DWORD nScanCookie = 0);
	// Manage filesystem change notification. Returns TRUE if changes detected.
	BOOL			IsChanged();
	void			OnDelete(TRISTATE bCreateGhost = TRI_UNKNOWN);
	void			OnFileRename(CLibraryFile* pFile);
	void			Maintain(BOOL bAdd);

protected:
	// Disable change notification monitor
	void			CloseMonitor();
	void			Clear();
	bool			operator==(const CLibraryFolder& val) const;
	void			RenewGUID();

// Automation
protected:
	BEGIN_INTERFACE_PART(LibraryFolder, ILibraryFolder)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_Library)(ILibrary FAR* FAR* ppLibrary);
		STDMETHOD(get_Parent)(ILibraryFolder FAR* FAR* ppFolder);
		STDMETHOD(get_Path)(BSTR FAR* psPath);
		STDMETHOD(get_Name)(BSTR FAR* psPath);
		STDMETHOD(get_Shared)(TRISTATE FAR* pnValue);
		STDMETHOD(put_Shared)(TRISTATE nValue);
		STDMETHOD(get_EffectiveShared)(VARIANT_BOOL FAR* pbValue);
		STDMETHOD(get_Folders)(ILibraryFolders FAR* FAR* ppFolders);
		STDMETHOD(get_Files)(ILibraryFiles FAR* FAR* ppFiles);
	END_INTERFACE_PART(LibraryFolder)
	BEGIN_INTERFACE_PART(LibraryFolders, ILibraryFolders)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_Library)(ILibrary FAR* FAR* ppLibrary);
		STDMETHOD(get__NewEnum)(IUnknown FAR* FAR* ppEnum);
		STDMETHOD(get_Item)(VARIANT vIndex, ILibraryFolder FAR* FAR* ppFolder);
		STDMETHOD(get_Count)(LONG FAR* pnCount);
	END_INTERFACE_PART(LibraryFolders)
	BEGIN_INTERFACE_PART(LibraryFiles, ILibraryFiles)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_Library)(ILibrary FAR* FAR* ppLibrary);
		STDMETHOD(get__NewEnum)(IUnknown FAR* FAR* ppEnum);
		STDMETHOD(get_Item)(VARIANT vIndex, ILibraryFile FAR* FAR* ppFile);
		STDMETHOD(get_Count)(LONG FAR* pnCount);
	END_INTERFACE_PART(LibraryFiles)

	DECLARE_INTERFACE_MAP()
};
