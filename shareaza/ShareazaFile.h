//
// SharedFile.h
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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

typedef CMap< CString, CString&, FILETIME, FILETIME& > CMapStringToFILETIME;

class CShareazaFile : boost::noncopyable
{
public:
	CShareazaFile() :
		m_nSize( SIZE_UNKNOWN )
	{
	}

	CString				m_sName;	// Filename only
	QWORD				m_nSize;	/*
									Size if any
									 (there is no size if it equal to 0 or SIZE_UNKNOWN)
									*/
	Hashes::Sha1Hash	m_oSHA1;	// SHA1 (Base32)
	Hashes::TigerHash	m_oTiger;	// TigerTree Root Hash (Base32)
	Hashes::Ed2kHash	m_oED2K;	// ED2K (MD4, Base16)
	Hashes::BtHash		m_oBTH;		// BitTorrent Info Hash (Base32
	Hashes::Md5Hash		m_oMD5;		// MD5 (Base16)
	CString				m_sPath;	/*
									Use:
									 CDownloadBase : Full local path (.partial)
									 CShareazaURL  : Path part of URL
									 CLibraryFile  : Local path without filename
									 CBTFile       : Relative path inside .torrent
									 CUploadFile   : Full local path
									*/
	CString				m_sURL;		// Host if any

	// Returns "urn:bitprint:SHA1.TIGER" or "urn:sha1:SHA1" or "urn:tree:tiger/:TIGER"
	CString GetBitprint() const;

	// Returns "http://nAddress:nPort/uri-res/N2R?{SHA1|TIGER|ED2K|MD5|BTH}"
	CString GetURL(const IN_ADDR& nAddress, WORD nPort) const;

	// Split string of URLs delimited by commas to URL list
	bool SplitStringToURLs(LPCTSTR pszURLs, CMapStringToFILETIME& oUrls) const;
};
