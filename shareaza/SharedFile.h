//
// SharedFile.h
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

#include "ShareazaFile.h"
#include "LibraryFolders.h"
#include "Schema.h"

class CDownload;
class CED2K;
class CLibraryDownload;
class CLibraryRecent;
class CQuerySearch;
class CSharedSource;
class CTigerTree;
class CXMLElement;


class CLibraryFile : public CShareazaFile
{
	DECLARE_DYNAMIC(CLibraryFile)

public:
	CLibraryFile(CLibraryFolder* pFolder, LPCTSTR pszName = NULL);
	virtual ~CLibraryFile();

	CLibraryFile*	m_pNextSHA1;
	CLibraryFile*	m_pNextTiger;
	CLibraryFile*	m_pNextED2K;
	CLibraryFile*	m_pNextBTH;
	CLibraryFile*	m_pNextMD5;
	DWORD			m_nScanCookie;
	DWORD			m_nUpdateCookie;
	DWORD			m_nSelectCookie;
	DWORD			m_nListCookie;
	DWORD			m_nIndex;
	FILETIME		m_pTime;
	QWORD			m_nVirtualBase;
	QWORD			m_nVirtualSize;
	TRISTATE		m_bVerify;
	CSchemaPtr		m_pSchema;
	CXMLElement*	m_pMetadata;
	BOOL			m_bMetadataAuto;		// Metadata is auto-generated
	FILETIME		m_pMetadataTime;		// Metadata time
	BOOL			m_bMetadataModified;	// Metadata must be saved
	int				m_nRating;
	CString			m_sComments;
	CString			m_sShareTags;
	DWORD			m_nUploadsToday;
	DWORD			m_nUploadsTotal;
	BOOL			m_bCachedPreview;
	BOOL			m_bBogus;
	CList< CSharedSource* >		m_pSources;
	// Search helper variables
	DWORD			m_nHitsToday;
	DWORD			m_nHitsTotal;
	DWORD			m_nSearchCookie;
	DWORD			m_nSearchWords;
	CLibraryFile*	m_pNextHit;
	DWORD			m_nCollIndex;
	int				m_nIcon16;
	BOOL			m_bNewFile;

	// Get folder path only
	CString			GetFolder() const;
	// Get full path (folder + file name)
	CString			GetPath() const;
	CString			GetSearchName() const;
	CXMLElement*	CreateXML(CXMLElement* pRoot, BOOL bSharedOnly, XmlType nType) const;
	bool			IsShared(bool bIgnoreOverride = false) const;
	void			SetShared(bool bShared, bool bOverride = false);
	bool			IsPrivateTorrent() const;
	// Get network wide file creation time (seconds, as time())
	DWORD			GetCreationTime();
	// Set network wide file creation time (seconds, as time())
	BOOL			SetCreationTime(DWORD tTime);
	BOOL			CheckFileAttributes(QWORD nSize, BOOL bSharedOnly, BOOL bAvailableOnly) const;
	inline BOOL		IsSharedOverride() const { return m_bShared != TRI_UNKNOWN; }
	// Is it a real file (i.e. not a ghost file)?
	inline bool		IsAvailable() const { return m_pFolder != NULL; }
	const CLibraryFolder* GetFolderPtr() const;
	BOOL			IsSchemaURI(LPCTSTR pszURI) const;
	BOOL			IsRated() const;		// File rated (or commented)
	BOOL			IsRatedOnly() const;	// File rated but have no metadata
	BOOL			IsHashed() const;		// File fully hashed
	BOOL			IsNewFile() const;
	BOOL			IsReadable() const;
	BOOL			Rebuild();
	BOOL			Rename(LPCTSTR pszName);
	BOOL			Delete(BOOL bDeleteGhost = FALSE);
	// Get any useful data (i.e. metadata, sources, hashes etc.) from specified file
	BOOL			AddMetadata(const CLibraryFile* pFile);
	void			UpdateMetadata(const CDownload* pDownload);
	BOOL			SetMetadata(CXMLElement*& pXML, BOOL bMerge = FALSE, BOOL bOverwrite = FALSE);
	BOOL			MergeMetadata(CXMLElement*& pXML, BOOL bOverwrite);
	BOOL			MergeMetadata(const CXMLElement* pXML);
	void			ClearMetadata();
	CString			GetMetadataWords() const;
	void			ModifyMetadata();		// Mark metadata as modified
	CTigerTree*		GetTigerTree();
	CED2K*			GetED2K();
	CSharedSource*	AddAlternateSource(LPCTSTR pszURL, const FILETIME* tSeen = NULL);
	CSharedSource*	AddAlternateSources(LPCTSTR pszURL);
	CString			GetAlternateSources(CList< CString >* pState, int nMaximum, PROTOCOLID nProtocol);
	BOOL			OnVerifyDownload(const CLibraryRecent* pRecent);

	// Adds file data to string array using template. Supported template variables:
	// $meta:name$		- file name
	// $meta:comments$	- file comments
	// $meta:hours$		- hours as decimal from file metadata "minutes" or "seconds" field
	// $meta:minutes$	- minutes as decimal from file metadata "minutes" or "seconds" field
	// $meta:seconds$	- seconds as  decimal from file metadata "minutes" or "seconds" field
	// $meta:time$		- time as string "hours:minutes:seconds" from file metadata "minutes" or "seconds" field
	// $meta:track$		- track as decimal
	// $meta:*$			- other file metadata fields as is
	// $meta:sizebytes$	- file size in bytes
	// $meta:size$		- file size in KB or MB
	// $meta:sha1$		- file SHA1 hash
	// $meta:gnutella$	- file SHA1 link (gnutella://)
	// $meta:tiger$		- file Tiger hash
	// $meta:bitprint$	- file SHA1.Tiger hash
	// $meta:ed2khash$	- file ED2K hash
	// $meta:ed2k$		- file ED2K link (ed2k://|file|)
	// $meta:md5$		- file MD5 hash
	// $meta:btih$		- file BitTorrnet info hash
	// $meta:magnet$	- file magnet-link
	// $meta:number$	- file number in string array
	// Unknown variables will be replaced by "N/A" string.
	BOOL			PrepareDoc(LPCTSTR pszTemplate, CArray< CString >& oDocs) const;

	inline QWORD GetBase() const
	{
		return ( m_nVirtualSize ) ? m_nVirtualBase : 0;
	}

	inline QWORD GetSize() const
	{
		return ( m_nVirtualSize ) ? m_nVirtualSize :
			( ( m_nSize == SIZE_UNKNOWN ) ? 0 : m_nSize );
	}

protected:
	TRISTATE		m_bShared;
	CLibraryFolder*	m_pFolder;		// NULL for Ghost files
	DWORD			m_tCreateTime;	// Cached network wide file creation time (seconds, as time())

	void			Serialize(CArchive& ar, int nVersion);
	BOOL			ThreadScan(DWORD nScanCookie, QWORD nSize, FILETIME* pTime/*, LPCTSTR pszMetaData*/);
	void			OnDelete(BOOL bDeleteGhost = FALSE, TRISTATE bCreateGhost = TRI_UNKNOWN);
	void			Ghost();

	BEGIN_INTERFACE_PART(LibraryFile, ILibraryFile)
		DECLARE_DISPATCH()
		STDMETHOD(get_Path)(BSTR FAR* psPath);
		STDMETHOD(get_Name)(BSTR FAR* psName);
		STDMETHOD(get_Size)(ULONGLONG FAR* pnSize);
		STDMETHOD(get_URN)(BSTR sURN, BSTR FAR* psURN);
		STDMETHOD(get_Hash)(URN_TYPE nType, ENCODING nBase, BSTR FAR* psURN);
		STDMETHOD(get_URL)(BSTR FAR* psURL);
		STDMETHOD(get_Magnet)(BSTR FAR* psMagnet);
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

	friend class CLibrary;
	friend class CLibraryFolder;
	friend class CLibraryMaps;
	friend class CDeleteFileDlg;

private:
	CLibraryFile(const CLibraryFile& pFile);
	CLibraryFile& operator=(const CLibraryFile& pFile);
};


typedef CList< CLibraryFile* > CFileList;


struct Earlier : public std::binary_function < CLibraryFile*, CLibraryFile*, bool >
{
	inline bool operator()( const CLibraryFile* _Left, const CLibraryFile* _Right ) const
	{
		return CompareFileTime( &_Left->m_pTime, &_Right->m_pTime ) < 0;
	}
};


class CSharedSource
{
// Construction
public:
	CSharedSource(LPCTSTR pszURL = NULL, const FILETIME* pTime = NULL);

// Attributes
public:
	CString		m_sURL;									// The URL
	FILETIME	m_pTime;								// Time last seen

// Operations
public:
	void	Serialize(CArchive& ar, int nVersion);
	void	Freshen(const FILETIME* pTime = NULL);
	BOOL	IsExpired(FILETIME& tNow) const;

};
