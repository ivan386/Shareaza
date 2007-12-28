//
// SharedFolder.h
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

#if !defined(AFX_SHAREDFOLDER_H__3538DA5D_21AB_428B_90C7_E1BE8F1B927E__INCLUDED_)
#define AFX_SHAREDFOLDER_H__3538DA5D_21AB_428B_90C7_E1BE8F1B927E__INCLUDED_

#pragma once

class CLibraryFile;
class CLibraryList;


class CLibraryFolder : public CComObject
{
// Construction
public:
	CLibraryFolder(CLibraryFolder* pParent, LPCTSTR pszPath = NULL);
	virtual ~CLibraryFolder();

	DECLARE_DYNAMIC(CLibraryFolder)

// Attributes
public:
	DWORD			m_nUpdateCookie;
	DWORD			m_nSelectCookie;
	CLibraryFolder*	m_pParent;
	CString			m_sName;
	CString			m_sPath;
	BOOL			m_bExpanded;
	DWORD			m_nFiles;
	QWORD			m_nVolume;
	CMap< CString, const CString&, CLibraryFile*, CLibraryFile* >	m_pFiles;

protected:
	DWORD			m_nScanCookie;
	CString			m_sNameLC;
	TRISTATE		m_bShared;
	CMap< CString, const CString&, CLibraryFolder*, CLibraryFolder* >	m_pFolders;
	HANDLE			m_hMonitor;
	BOOL			m_bForceScan;		// TRUE - next scan forced (root folder only)
	BOOL			m_bOffline;			// TRUE - folder absent (root folder only)
	Hashes::Guid	m_oGUID;

// Operations
public:
	POSITION		GetFolderIterator() const;
	CLibraryFolder*	GetNextFolder(POSITION& pos) const;
	CLibraryFolder*	GetFolderByName(LPCTSTR pszName) const;
	CLibraryFolder*	GetFolderByPath(LPCTSTR pszPath) const;
	BOOL			CheckFolder(CLibraryFolder* pFolder, BOOL bRecursive = FALSE) const;
	INT_PTR			GetFolderCount() const;

	POSITION		GetFileIterator() const;
	CLibraryFile*	GetNextFile(POSITION& pos) const;
	CLibraryFile*	GetFile(LPCTSTR pszName) const;
	INT_PTR			GetFileCount() const;
	int				GetFileList(CLibraryList* pList, BOOL bRecursive) const;
	int				GetSharedCount() const;

	void			Scan();
	BOOL			IsShared() const;
	// Set to TRUE if shared, to FALSE if not, and leave unchanged if unknown
	void			GetShared(BOOL& bShared) const;
	void			SetShared(TRISTATE bShared);
	BOOL			IsOffline() const;
	BOOL			SetOffline();
	BOOL			SetOnline();
	void			Serialize(CArchive& ar, int nVersion);
	BOOL			ThreadScan(volatile BOOL* pbContinue, DWORD nScanCookie = 0);
	// Manage filesystem change notification. Returns TRUE if changes detected.
	BOOL			IsChanged();
	void			OnDelete(TRISTATE bCreateGhost = TRI_UNKNOWN);
	void			OnFileRename(CLibraryFile* pFile);

protected:
	// Disable change notification monitor
	void			CloseMonitor();
	void			Clear();
	void			PathToName();
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

#endif // !defined(AFX_SHAREDFOLDER_H__3538DA5D_21AB_428B_90C7_E1BE8F1B927E__INCLUDED_)
