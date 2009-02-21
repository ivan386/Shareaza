//
// Library.h
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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

#include "ComObject.h"
#include "ThreadImpl.h"

class CQuerySearch;
class CLibraryFile;
class CLibraryFolder;
class CAlbumFolder;

#define LIBRARY_SER_VERSION	29
// History:
// 27 - Changed CLibraryFile metadata saving order (ryo-oh-ki)
// 28 - Added CLibraryMaps m_pIndexMap, m_pNameMap and m_pPathMap counts (ryo-oh-ki)
// 29 - Added CLibraryDictionary serialize (ryo-oh-ki)

class CLibrary :
	public CComObject,
	public CThreadImpl
{
// Construction
public:
	CLibrary();
	virtual ~CLibrary();

	DECLARE_DYNAMIC(CLibrary)

// Attributes
public:
	CMutex			m_pSection;
	volatile DWORD	m_nUpdateCookie;		// Last library change time (ticks)
	volatile DWORD	m_nForcedUpdateCookie;	// Last time when library scan was forced (ticks)
	volatile DWORD	m_nScanCount;			// Library scan counter
	volatile DWORD	m_nScanCookie;			// Used by CLibraryFolder::ThreadScan()
	volatile DWORD	m_nScanTime;			// Last library scan time (ticks)
	volatile DWORD	m_nUpdateSaved;			// Last library save time (ticks)

protected:
	int				m_nFileSwitch;

// Sync Operations
public:
	inline void		Update(bool bForce=false)
	{
		InterlockedExchange( (volatile LONG*)&m_nUpdateCookie, GetTickCount() );
		if ( bForce )
			InterlockedExchange( (volatile LONG*)&m_nForcedUpdateCookie, 0 );
	}

// File and Folder Operations
public:
	void			CheckDuplicates(LPCTSTR pszMD5Hash);
	CLibraryFile*	LookupFile(DWORD_PTR nIndex, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE) const;
	CAlbumFolder*	GetAlbumRoot();
	void			AddFile(CLibraryFile* pFile);
	void			RemoveFile(CLibraryFile* pFile);

protected:
	void			CheckDuplicates(CLibraryFile* pFile, bool bForce = false);

// General Operations
public:
	CList< CLibraryFile* >*	Search(CQuerySearch* pSearch, int nMaximum = 0, BOOL bLocal = FALSE, BOOL bAvailableOnly = FALSE);
	void			Clear();
	BOOL			Load();
	BOOL			Save();
	inline void		StopThread()
	{
		CloseThread();
	}
	static BOOL		IsBadFile(LPCTSTR szFilenameOnly, LPCTSTR szPathOnly = NULL, DWORD dwFileAttributes = 0);

protected:
	void			OnRun();
	void			Serialize(CArchive& ar);
	BOOL			ThreadScan();
	BOOL			SafeReadTime(CFile& pFile, FILETIME* pFileTime) throw();
	BOOL			SafeSerialize(CArchive& ar) throw();

// Automation
protected:
	BEGIN_INTERFACE_PART(Library, ILibrary)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_Library)(ILibrary FAR* FAR* ppLibrary);
		STDMETHOD(get_Folders)(ILibraryFolders FAR* FAR* ppFolders);
		STDMETHOD(get_Albums)(IUnknown FAR* FAR* ppAlbums);
		STDMETHOD(get_Files)(ILibraryFiles FAR* FAR* ppFiles);
		STDMETHOD(FindByName)(BSTR sName, ILibraryFile FAR* FAR* ppFile);
		STDMETHOD(FindByPath)(BSTR sPath, ILibraryFile FAR* FAR* ppFile);
		STDMETHOD(FindByURN)(BSTR sURN, ILibraryFile FAR* FAR* ppFile);
		STDMETHOD(FindByIndex)(LONG nIndex, ILibraryFile FAR* FAR* ppFile);
	END_INTERFACE_PART(Library)

	DECLARE_INTERFACE_MAP()
};

extern CLibrary Library;

#include "LibraryList.h"
#include "LibraryMaps.h"
