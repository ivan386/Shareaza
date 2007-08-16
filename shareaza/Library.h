//
// Library.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#if !defined(AFX_LIBRARY_H__28EBDAFA_BD15_4BBF_874D_1B6116B0E603__INCLUDED_)
#define AFX_LIBRARY_H__28EBDAFA_BD15_4BBF_874D_1B6116B0E603__INCLUDED_

#pragma once

class CQuerySearch;
class CLibraryFile;
class CLibraryFolder;
class CAlbumFolder;

#define LIBRARY_SER_VERSION	26

class CLibrary : public CComObject
{
// Construction
public:
	CLibrary();
	virtual ~CLibrary();

	DECLARE_DYNAMIC(CLibrary)

// Attributes
public:
	CMutex			m_pSection;
	DWORD			m_nUpdateCookie;
	DWORD			m_nScanCount;
	DWORD			m_nScanCookie;
	DWORD			m_nScanTime;
	DWORD			m_nUpdateSaved;
	BOOL			(WINAPI* m_pfnGFAEW)(LPCWSTR, GET_FILEEX_INFO_LEVELS, LPVOID);
	BOOL			(WINAPI* m_pfnGFAEA)(LPCSTR, GET_FILEEX_INFO_LEVELS, LPVOID);

protected:
	int				m_nFileSwitch;
	HANDLE			m_hThread;
	BOOL			m_bThread;
	CEvent			m_pWakeup;
	HINSTANCE		m_hKernel;

// Sync Operations
public:
	inline void		Update()
	{
		CQuickLock oLock( m_pSection );
		m_nUpdateCookie = GetTickCount();
	}
	void			CheckDuplicates(LPCTSTR pszEd2kHash);

// File and Folder Operations
public:
	CLibraryFile*	LookupFile(DWORD nIndex, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = FALSE);
	CAlbumFolder*	GetAlbumRoot();

protected:
	void			AddFile(CLibraryFile* pFile);
	void			RemoveFile(CLibraryFile* pFile);
	void			OnFileDelete(CLibraryFile* pFile, BOOL bDeleteGhost = FALSE);
	void			CheckDuplicates(CLibraryFile* pFile, bool bForce = false);

// General Operations
public:
	CList< CLibraryFile* >*	Search(CQuerySearch* pSearch, int nMaximum = 0, BOOL bLocal = FALSE, BOOL bAvailableOnly = FALSE);
	void			Clear();
	BOOL			Load();
	BOOL			Save();
	void			StartThread();
	void			StopThread();

	static BOOL		IsBadFile(LPCTSTR szFilenameOnly, LPCTSTR szPathOnly = NULL, DWORD dwFileAttributes = 0);

protected:
	void			Serialize(CArchive& ar);
	static UINT		ThreadStart(LPVOID pParam);
	void			OnRun();
	BOOL			ThreadScan();

	friend class CLibraryFolder;
	friend class CLibraryFile;
	friend class CLibraryBuilder;

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

#endif // !defined(AFX_LIBRARY_H__28EBDAFA_BD15_4BBF_874D_1B6116B0E603__INCLUDED_)
