//
// LibraryMaps.h
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

#include "SharedFile.h"

class CLibrary;
class CQuerySearch;

#define FILE_HASH_SIZE	512
#define FILE_INDEX(x)	( *(WORD*)(&(x)[0]) & 511 )


class CLibraryMaps : public CComObject
{
	DECLARE_DYNAMIC(CLibraryMaps)

public:
	CLibraryMaps();
	virtual ~CLibraryMaps();

	POSITION		GetFileIterator() const;
	CLibraryFile*	GetNextFile(POSITION& pos) const;
	INT_PTR			GetFileCount() const { return m_pIndexMap.GetCount(); }
	INT_PTR			GetNameCount() const { return m_pNameMap.GetCount(); }
	INT_PTR			GetPathCount() const { return m_pPathMap.GetCount(); }
	void			GetStatistics(DWORD* pnFiles, QWORD* pnVolume);

	CLibraryFile*	LookupFile(DWORD nIndex, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE) const;
	CLibraryFile*	LookupFileByName(LPCTSTR pszName, QWORD nSize, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE) const;
	CLibraryFile*	LookupFileByPath(LPCTSTR pszPath, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE) const;
	CLibraryFile*	LookupFileByURN(LPCTSTR pszURN, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE) const;
	CLibraryFile*	LookupFileByHash(const CShareazaFile* pFilter, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE) const;
	CFileList*		LookupFilesByHash(const CShareazaFile* pFilter, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE, int nMaximum = 1) const;
	CLibraryFile*	LookupFileBySHA1(const Hashes::Sha1Hash& oSHA1, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE) const;
	CLibraryFile*	LookupFileByTiger(const Hashes::TigerHash& oTiger, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE) const;
	CLibraryFile*	LookupFileByED2K(const Hashes::Ed2kHash& oED2K, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE) const;
	CLibraryFile*	LookupFileByBTH(const Hashes::BtHash& oBTH, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE) const;
	CLibraryFile*	LookupFileByMD5(const Hashes::Md5Hash& oMD5, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE) const;

protected:
	typedef CAtlMap< DWORD, CLibraryFile* > CIndexMap;
	typedef CAtlMap< CString, CLibraryFile*, CStringElementTraitsI< CString > > CFileMap;

	CIndexMap			m_pIndexMap;
	CFileMap			m_pNameMap;
	CFileMap			m_pPathMap;
	CLibraryFile*		m_pSHA1Map[ FILE_HASH_SIZE ];
	CLibraryFile*		m_pTigerMap[ FILE_HASH_SIZE ];
	CLibraryFile*		m_pED2KMap[ FILE_HASH_SIZE ];
	CLibraryFile*		m_pBTHMap[ FILE_HASH_SIZE ];
	CLibraryFile*		m_pMD5Map[ FILE_HASH_SIZE ];
	CFileList			m_pDeleted;
	DWORD				m_nNextIndex;
	DWORD				m_nFiles;
	QWORD				m_nVolume;

	void			Clear();
	DWORD			AllocateIndex();
	void			OnFileAdd(CLibraryFile* pFile);
	void			OnFileRemove(CLibraryFile* pFile);
	void			CullDeletedFiles(CLibraryFile* pMatch);
	CFileList*		Browse(int nMaximum) const;
	CFileList*		WhatsNew(const CQuerySearch* pSearch, int nMaximum) const;
	void			Serialize1(CArchive& ar, int nVersion);
	void			Serialize2(CArchive& ar, int nVersion);

// COM
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
};

extern CLibraryMaps LibraryMaps;
