//
// SharedFile.h
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

#include "ShareazaFile.h"

class CLibraryFolder;
class CSharedSource;
class CSchema;
class CXMLElement;
class CQuerySearch;
class CLibraryDownload;
class CTigerTree;
class CED2K;
class CDownload;


class CLibraryFile : public CShareazaFile
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
	CLibraryFile*	m_pNextBTH;
	CLibraryFile*	m_pNextMD5;
	DWORD			m_nScanCookie;
	DWORD			m_nUpdateCookie;
	DWORD			m_nSelectCookie;
	DWORD			m_nListCookie;
public:
	CLibraryFolder*	m_pFolder;	// NULL for Ghost files
	DWORD			m_nIndex;
	FILETIME		m_pTime;
	TRISTATE		m_bShared;
	QWORD			m_nVirtualBase;
	QWORD			m_nVirtualSize;
public:
	TRISTATE		m_bVerify;
public:
	CSchema*		m_pSchema;
	CXMLElement*	m_pMetadata;
	BOOL			m_bMetadataAuto;		// Metadata is auto-generated
	FILETIME		m_pMetadataTime;		// Metadata time
	BOOL			m_bMetadataModified;	// Metadata must be saved
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
	CList< CSharedSource* > m_pSources;
public:
	DWORD			m_nSearchCookie;
	DWORD			m_nSearchWords;
	CLibraryFile*	m_pNextHit;
	DWORD			m_nCollIndex;
	int				m_nIcon16;
	BOOL			m_bNewFile;
	
// Operations
public:
	CString			GetPath() const;
	CString			GetSearchName() const;
	BOOL			IsShared() const;
	inline BOOL		IsGhost() const { return m_pFolder == NULL; }
	inline BOOL		IsAvailable() const { return m_pFolder != NULL; }
	BOOL			IsSchemaURI(LPCTSTR pszURI) const;
	BOOL			IsRated() const;		// File rated (or commented)
	BOOL			IsRatedOnly() const;	// File rated but have no metadata
	BOOL			IsHashed() const;
	BOOL			IsNewFile() const;
	BOOL			IsReadable() const;
public:
	BOOL			Rebuild();
	BOOL			Rename(LPCTSTR pszName);
	BOOL			Delete(BOOL bDeleteGhost = FALSE);
	void			UpdateMetadata(const CDownload* pDownload);
	BOOL			SetMetadata(CXMLElement*& pXML, BOOL bMerge = FALSE, BOOL bOverwrite = FALSE);
	BOOL			MergeMetadata(CXMLElement*& pXML, BOOL bOverwrite);
	void			ClearMetadata();
	CString			GetMetadataWords() const;
	void			ModifyMetadata();		// Mark metadata as modified
	CTigerTree*		GetTigerTree();
	CED2K*			GetED2K();
public:
	CSharedSource*	AddAlternateSource(LPCTSTR pszURL, FILETIME* tSeen = NULL);
	CSharedSource*	AddAlternateSources(LPCTSTR pszURL);
	CString			GetAlternateSources(CList< CString >* pState, int nMaximum, PROTOCOLID nProtocol);
protected:
	void			Serialize(CArchive& ar, int nVersion);
	BOOL			ThreadScan(CSingleLock& pLock, DWORD nScanCookie, QWORD nSize, FILETIME* pTime/*, LPCTSTR pszMetaData*/);
	void			OnDelete(BOOL bDeleteGhost = FALSE, TRISTATE bCreateGhost = TRI_UNKNOWN);
	void			Ghost();
    BOOL			OnVerifyDownload(
						const Hashes::Sha1ManagedHash& oSHA1,
						const Hashes::TigerManagedHash& oTiger,
						const Hashes::Ed2kManagedHash& oED2K,
						const Hashes::BtManagedHash& oBTH,
						const Hashes::Md5ManagedHash& oMD5,
						LPCTSTR pszSources);
	
// Inlines
public:
	inline CString GetNameLC() const
	{
		CString str( m_sName );
		return ToLower( str );
	}
	
	inline QWORD GetSize() const
	{
		return ( m_nVirtualSize ) ? m_nVirtualSize :
			( ( m_nSize == SIZE_UNKNOWN ) ? 0 : m_nSize );
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
		STDMETHOD(get_Path)(BSTR FAR* psPath);
		STDMETHOD(get_Name)(BSTR FAR* psName);
		STDMETHOD(get_Size)(ULONGLONG FAR* pnSize);
		STDMETHOD(get_URN)(BSTR sURN, BSTR FAR* psURN);
		STDMETHOD(get_Hash)(URN_TYPE nType, ENCODING nBase, BSTR FAR* psURN);
		STDMETHOD(get_URL)(BSTR FAR* psURL);
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_Library)(ILibrary FAR* FAR* ppLibrary);
		STDMETHOD(get_Folder)(ILibraryFolder FAR* FAR* ppFolder);
		STDMETHOD(get_Shared)(TRISTATE FAR* pnValue);
		STDMETHOD(put_Shared)(TRISTATE nValue);
		STDMETHOD(get_EffectiveShared)(VARIANT_BOOL FAR* pbValue);
		STDMETHOD(get_Index)(LONG FAR* pnIndex);
		STDMETHOD(get_MetadataAuto)(VARIANT_BOOL FAR* pbValue);
		STDMETHOD(get_Metadata)(ISXMLElement FAR* FAR* ppXML);
		STDMETHOD(put_Metadata)(ISXMLElement FAR* pXML);
		STDMETHOD(Execute)();
		STDMETHOD(SmartExecute)();
		STDMETHOD(Delete)();
		STDMETHOD(Rename)(BSTR sNewName);
		STDMETHOD(Copy)(BSTR sNewPath);
		STDMETHOD(Move)(BSTR sNewPath);
		STDMETHOD(MergeMetadata)(ISXMLElement* pXML, VARIANT_BOOL bOverwrite, VARIANT_BOOL* pbValue);
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
	CString		m_sURL;									// The URL
	FILETIME	m_pTime;								// Time last seen

// Operations
public:
	void	Serialize(CArchive& ar, int nVersion);
	void	Freshen(FILETIME* pTime = NULL);
	BOOL	IsExpired(FILETIME& tNow);

};
