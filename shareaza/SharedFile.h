//
// SharedFile.h
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

#if !defined(AFX_SHAREDFILE_H__8FFCC311_D43C_445D_BAEB_575AE2AE8E99__INCLUDED_)
#define AFX_SHAREDFILE_H__8FFCC311_D43C_445D_BAEB_575AE2AE8E99__INCLUDED_

#pragma once

class CLibraryFolder;
class CSharedSource;
class CSchema;
class CXMLElement;
class CQuerySearch;
class CLibraryDownload;
class CTigerTree;
class CED2K;


class CLibraryFile : public CComObject
{
// Construction
public:
	CLibraryFile(CLibraryFolder* pFolder, LPCTSTR pszName = NULL);
	virtual ~CLibraryFile();
	
	DECLARE_DYNAMIC(CLibraryFile)
	
// Attributes
public:
	CLibraryFile*	m_pNextSHA1;
	CLibraryFile*	m_pNextTiger;
	CLibraryFile*	m_pNextED2K;
	DWORD			m_nScanCookie;
	DWORD			m_nUpdateCookie;
	DWORD			m_nSelectCookie;
	DWORD			m_nListCookie;
public:
	CLibraryFolder*	m_pFolder;
	CString			m_sName;
	DWORD			m_nIndex;
	QWORD			m_nSize;
	FILETIME		m_pTime;
	TRISTATE		m_bShared;
	QWORD			m_nVirtualBase;
	QWORD			m_nVirtualSize;
public:
	BOOL			m_bSHA1;
	SHA1			m_pSHA1;
	BOOL			m_bTiger;
	TIGEROOT		m_pTiger;
	BOOL			m_bMD5;
	MD5				m_pMD5;
	BOOL			m_bED2K;
	MD4				m_pED2K;
	TRISTATE		m_bVerify;
public:
	CSchema*		m_pSchema;
	CXMLElement*	m_pMetadata;
	BOOL			m_bMetadataAuto;
	FILETIME		m_pMetadataTime;
	int				m_nRating;
	CString			m_sComments;
	CString			m_sShareTags;
public:
	DWORD			m_nHitsToday;
	DWORD			m_nHitsTotal;
	DWORD			m_nUploadsToday;
	DWORD			m_nUploadsTotal;
	BOOL			m_bCachedPreview;
	BOOL			m_bBogus;
	CPtrList		m_pSources;
public:
	DWORD			m_nSearchCookie;
	DWORD			m_nSearchWords;
	CLibraryFile*	m_pNextHit;
	DWORD			m_nCollIndex;
	int				m_nIcon16;
	
// Operations
public:
	CString			GetPath() const;
	CString			GetSearchName() const;
	BOOL			IsShared() const;
	inline BOOL		IsGhost() const { return m_pFolder == NULL; }
	inline BOOL		IsAvailable() const { return m_pFolder != NULL; }
	BOOL			IsSchemaURI(LPCTSTR pszURI) const;
public:
	BOOL			Rebuild();
	BOOL			Rename(LPCTSTR pszName);
	BOOL			Delete();
	BOOL			SetMetadata(CXMLElement* pXML);
	CString			GetMetadataWords() const;
	BOOL			SaveMetadata();
	CTigerTree*		GetTigerTree();
	CED2K*			GetED2K();
public:
	CSharedSource*	AddAlternateSource(LPCTSTR pszURL, BOOL bForce = TRUE);
	CSharedSource*	AddAlternateSources(LPCTSTR pszURL);
	CString			GetAlternateSources(CStringList* pState, int nMaximum, BOOL bHTTP);
protected:
	void			Serialize(CArchive& ar, int nVersion);
	BOOL			ThreadScan(CSingleLock& pLock, DWORD nScanCookie, QWORD nSize, FILETIME* pTime, LPCTSTR pszMetaData);
	BOOL			LoadMetadata(HANDLE hFile);
	void			OnDelete();
	void			Ghost();
	BOOL			OnVerifyDownload(const SHA1* pSHA1, const MD4* pED2K, LPCTSTR pszSources);
	
// Inlines
public:
	inline CString GetNameLC() const
	{
		CString str = m_sName;
		str = CharLower( str.GetBuffer() );
		return str;
	}
	
	inline QWORD GetSize() const
	{
		return ( m_nVirtualSize > 0 ) ? m_nVirtualSize : m_nSize;
	}
	
// Friends
public:
	friend class CLibrary;
	friend class CLibraryFolder;
	friend class CLibraryMaps;
	friend class CLibraryRecent;
	friend class CDeleteFileDlg;
	
// Automation
protected:
	BEGIN_INTERFACE_PART(LibraryFile, ILibraryFile)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_Library)(ILibrary FAR* FAR* ppLibrary);
		STDMETHOD(get_Folder)(ILibraryFolder FAR* FAR* ppFolder);
		STDMETHOD(get_Path)(BSTR FAR* psPath);
		STDMETHOD(get_Name)(BSTR FAR* psPath);
		STDMETHOD(get_Shared)(STRISTATE FAR* pnValue);
		STDMETHOD(put_Shared)(STRISTATE nValue);
		STDMETHOD(get_EffectiveShared)(VARIANT_BOOL FAR* pbValue);
		STDMETHOD(get_Size)(LONG FAR* pnSize);
		STDMETHOD(get_Index)(LONG FAR* pnIndex);
		STDMETHOD(get_URN)(BSTR sURN, BSTR FAR* psURN);
		STDMETHOD(get_MetadataAuto)(VARIANT_BOOL FAR* pbValue);
		STDMETHOD(get_Metadata)(ISXMLElement FAR* FAR* ppXML);
		STDMETHOD(put_Metadata)(ISXMLElement FAR* pXML);
		STDMETHOD(Execute)();
		STDMETHOD(SmartExecute)();
		STDMETHOD(Delete)();
		STDMETHOD(Rename)(BSTR sNewName);
		STDMETHOD(Copy)(BSTR sNewPath);
		STDMETHOD(Move)(BSTR sNewPath);
	END_INTERFACE_PART(LibraryFile)
	
	DECLARE_INTERFACE_MAP()
	
};


class CSharedSource
{
// Construction
public:
	CSharedSource(LPCTSTR pszURL = NULL, FILETIME* pTime = NULL);

// Attributes
public:
	CString		m_sURL;
	FILETIME	m_pTime;

// Operations
public:
	void	Serialize(CArchive& ar, int nVersion);
	void	Freshen(FILETIME* pTime = NULL);
	BOOL	IsExpired(FILETIME& tNow);

};


#endif // !defined(AFX_SHAREDFILE_H__8FFCC311_D43C_445D_BAEB_575AE2AE8E99__INCLUDED_)
