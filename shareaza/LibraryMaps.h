//
// LibraryMaps.h
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

class CLibrary;
class CLibraryFile;
class CQuerySearch;


class CLibraryMaps : public CComObject
{
// Consturction
public:
	CLibraryMaps();
	virtual ~CLibraryMaps();
	
	DECLARE_DYNAMIC(CLibraryMaps)

// Attributes
protected:
	CMapPtrToPtr		m_pIndexMap;
	CMapStringToOb		m_pNameMap;
	CMapStringToOb		m_pPathMap;
	CLibraryFile**		m_pSHA1Map;
	CLibraryFile**		m_pTigerMap;
	CLibraryFile**		m_pED2KMap;
	CPtrList			m_pDeleted;
protected:
	DWORD				m_nNextIndex;
	DWORD				m_nFiles;
	QWORD				m_nVolume;

// File Operations
public:
	POSITION		GetFileIterator() const;
	CLibraryFile*	GetNextFile(POSITION& pos) const;
	int				GetFileCount() const;
	void			GetStatistics(DWORD* pnFiles, QWORD* pnVolume);
public:
	CLibraryFile*	LookupFile(DWORD nIndex, BOOL bLockOnSuccess = FALSE, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE);
	CLibraryFile*	LookupFileByName(LPCTSTR pszName, BOOL bLockOnSuccess = FALSE, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE);
	CLibraryFile*	LookupFileByPath(LPCTSTR pszPath, BOOL bLockOnSuccess = FALSE, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE);
	CLibraryFile*	LookupFileByURN(LPCTSTR pszURN, BOOL bLockOnSuccess = FALSE, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE);
	CLibraryFile*	LookupFileBySHA1(const CHashSHA1 &oSHA1, BOOL bLockOnSuccess = FALSE, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE);
	CLibraryFile*	LookupFileByTiger(const CHashTiger &oTiger, BOOL bLockOnSuccess = FALSE, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE);
	CLibraryFile*	LookupFileByED2K(const CHashED2K &oED2K, BOOL bLockOnSuccess = FALSE, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE);
protected:
	void			Clear();
	DWORD			AllocateIndex();
	void			OnFileAdd(CLibraryFile* pFile);
	void			OnFileRemove(CLibraryFile* pFile);
	void			CullDeletedFiles(CLibraryFile* pMatch);
	CPtrList*		Search(CQuerySearch* pSearch, int nMaximum, BOOL bLocal);
	void			Serialize1(CArchive& ar, int nVersion);
	void			Serialize2(CArchive& ar, int nVersion);
	
// COM
protected:
	BEGIN_INTERFACE_PART(LibraryFiles, ILibraryFiles)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_Library)(ILibrary FAR* FAR* ppLibrary);
		STDMETHOD(get__NewEnum)(IUnknown FAR* FAR* ppEnum);
		STDMETHOD(get_Item)(VARIANT vIndex, ILibraryFile FAR* FAR* ppFile);
		STDMETHOD(get_Count)(LONG FAR* pnCount);
	END_INTERFACE_PART(LibraryFiles)
	
	DECLARE_INTERFACE_MAP()
	
	friend class CLibrary;
	friend class CLibraryBuilder;
	friend class CLibraryFile;
};

extern CLibraryMaps LibraryMaps;
