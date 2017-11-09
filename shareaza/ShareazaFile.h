//
// ShareazaFile.h
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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

#include "ShareazaOM.h"

typedef CMap< CString, const CString&, FILETIME, FILETIME& > CMapStringToFILETIME;

class CShareazaFile : public CComObject
{
	DECLARE_DYNAMIC(CShareazaFile)

public:
	CShareazaFile();
	CShareazaFile(const CShareazaFile& pFile);
	CShareazaFile& operator=(const CShareazaFile& pFile);

	CString				m_sName;	// Filename only
	QWORD				m_nSize;	/* Size if any
									 (there is no size if it equal to 0 or SIZE_UNKNOWN)
									*/
	Hashes::Sha1Hash	m_oSHA1;	// SHA1 (Base32)
	Hashes::TigerHash	m_oTiger;	// TigerTree Root Hash (Base32)
	Hashes::Ed2kHash	m_oED2K;	// ED2K (MD4, Base16)
	Hashes::BtHash		m_oBTH;		// BitTorrent Info Hash (Base32)
	Hashes::Md5Hash		m_oMD5;		// MD5 (Base16)
	CString				m_sPath;	/* Use:
									 CShareazaURL : Path part of URL
									 CLibraryFile : Local path without filename
									 CBTFile      : Relative path inside .torrent
									 CDownload    : Path of .sd-file
									 CUploadFile  : Path of requested file
									*/
	CString				m_sURL;		// Host if any

	// Returns "urn:bitprint:SHA1.TIGER", "urn:sha1:SHA1" or "urn:tree:tiger/:TIGER" only (else empty string)
	CString GetBitprint() const;

	// Returns any available URNs
	CString GetURN() const;
	CString GetShortURN() const;

	// Returns "sha1_SHA1", "ttr_TIGER" etc.
	CString GetFilename() const;

	// Returns "http://nAddress:nPort/uri-res/N2R?{SHA1|TIGER|ED2K|MD5|BTH}"
	CString GetURL(const IN_ADDR& nAddress, WORD nPort) const;

	// Split string of URLs delimited by commas to URL list
	bool SplitStringToURLs(LPCTSTR pszURLs, CMapStringToFILETIME& oUrls) const;

	// Are files sufficient equal?
	bool operator==(const CShareazaFile& pFile) const;

	// Are files sufficient unequal? (i.e. has diffrent sizes or hashes)
	bool operator!=(const CShareazaFile& pFile) const;

	// Are some of hashes present?
	inline bool HasHash() const
	{
		return m_oSHA1 || m_oTiger || m_oED2K || m_oBTH || m_oMD5;
	}
	
	// Printable file size
	inline QWORD GetSize() const throw()
	{
		return ( ( m_nSize == SIZE_UNKNOWN ) ? 0 : m_nSize );
	}

	// Get file name suitable for searching
	virtual CString GetSearchName() const
	{
		CString sName = m_sName;
		ToLower( sName );
		return sName;
	}

// Automation
protected:
	BEGIN_INTERFACE_PART(ShareazaFile, IShareazaFile)
		DECLARE_DISPATCH()
		STDMETHOD(get_Path)(BSTR FAR* psPath);
		STDMETHOD(get_Name)(BSTR FAR* psName);
		STDMETHOD(get_Size)(ULONGLONG FAR* pnSize);
		STDMETHOD(get_URN)(BSTR sURN, BSTR FAR* psURN);
		STDMETHOD(get_Hash)(URN_TYPE nType, ENCODING nBase, BSTR FAR* psURN);
		STDMETHOD(get_URL)(BSTR FAR* psURL);
		STDMETHOD(get_Magnet)(BSTR FAR* psMagnet);
	END_INTERFACE_PART(ShareazaFile)

	DECLARE_INTERFACE_MAP()
};
