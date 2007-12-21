//
// GGEP.h
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

#if !defined(AFX_GGEP_H__7081FCAC_A207_412A_BD18_A72F09F89F05__INCLUDED_)
#define AFX_GGEP_H__7081FCAC_A207_412A_BD18_A72F09F89F05__INCLUDED_

#pragma once

#define GGEP_MAGIC			0xC3 // GGEP extension prefix

#define GGEP_HDR_LAST		0x80 // Last extension in GGEP block
#define GGEP_HDR_COBS		0x40 // Whether COBS was used on payload
#define GGEP_HDR_DEFLATE	0x20 // Whether payload was deflated
#define GGEP_HDR_IDLEN		0x0F // Where ID length is stored

#define GGEP_LEN_MORE		0x80 // Continuation present
#define GGEP_LEN_LAST		0x40 // Last byte
#define GGEP_LEN_MASK		0x3F // Value

#define GGEP_H_SHA1			0x01 // Binary SHA1
#define GGEP_H_BITPRINT		0x02 // Bitprint (SHA1 + Tiger tree root)
#define GGEP_H_MD5			0x03 // Binary MD5
#define GGEP_H_UUID			0x04 // Binary UUID (GUID-like)
#define GGEP_H_MD4			0x05 // Binary MD4

// MetaType query support (GGEP_HEADER_META)
// 0x01 - Reserved
// 0x02 - Reserved
#define GGEP_META_AUDIO		0x04 // Audio
#define GGEP_META_VIDEO		0x08 // Video
#define GGEP_META_DOCUMENTS	0x10 // Documents
#define GGEP_META_IMAGES	0x20 // Images
#define GGEP_META_WINDOWS	0x40 // Windows Programs/Packages 
#define GGEP_META_UNIX		0x80 // Linux/Unix/Mac Programs/Packages

// Known GGEP Extension Blocks table:
// http://gnutella-specs.rakjar.de/index.php/Known_GGEP_Extension_Blocks

// Browse Host
const LPCTSTR GGEP_HEADER_BROWSE_HOST			= _T("BH");
// Average daily uptime
const LPCTSTR GGEP_HEADER_DAILY_AVERAGE_UPTIME	= _T("DU");
// Unicast protocol support
const LPCTSTR GGEP_HEADER_UNICAST_SUPPORT		= _T("GUE");
// Vendor info
const LPCTSTR GGEP_HEADER_VENDOR_INFO			= _T("VC");
// Ultrapeer support
const LPCTSTR GGEP_HEADER_UP_SUPPORT			= _T("UP");
// AddressSecurityToken support
const LPCTSTR GGEP_HEADER_QUERY_KEY_SUPPORT		= _T("QK");
// OOB v3 Security Token support
const LPCTSTR GGEP_HEADER_SECURE_OOB			= _T("SO");
// AddressSecurityToken support
const LPCTSTR GGEP_HEADER_MULTICAST_RESPONSE	= _T("MCAST");
// PushProxy support
const LPCTSTR GGEP_HEADER_PUSH_PROXY			= _T("PUSH");
// AlternateLocation support
const LPCTSTR GGEP_HEADER_ALTS					= _T("ALT");
// IpPort request
const LPCTSTR GGEP_HEADER_IPPORT				= _T("IP");
// UDP HostCache pongs
const LPCTSTR GGEP_HEADER_UDP_HOST_CACHE		= _T("UDPHC");
// Indicating support for packed ip/ports & udp host caches
const LPCTSTR GGEP_HEADER_SUPPORT_CACHE_PONGS	= _T("SCP");
// Packed IP/Ports
const LPCTSTR GGEP_HEADER_PACKED_IPPORTS		= _T("IPP");
// Packed UDP Host Caches
const LPCTSTR GGEP_HEADER_PACKED_HOSTCACHES		= _T("PHC");
// SHA1 URNs
const LPCTSTR GGEP_HEADER_SHA1					= _T("S1");
// Determine if a SHA1 is valid
const LPCTSTR GGEP_HEADER_SHA1_VALID			= _T("SV");
// TLS support
const LPCTSTR GGEP_HEADER_TLS_SUPPORT			= _T("TLS");
// DHT support
const LPCTSTR GGEP_HEADER_DHT_SUPPORT			= _T("DHT");
// DHT IPP requests
const LPCTSTR GGEP_HEADER_DHT_IPPORTS			= _T("DHTIPP");
// A feature query. This is 'WH' for legacy reasons, because 'What is New' was the first
const LPCTSTR GGEP_HEADER_FEATURE_QUERY			= _T("WH");
// The extension header disabling OOB proxying
const LPCTSTR GGEP_HEADER_NO_PROXY				= _T("NP");
// MetaType query support
const LPCTSTR GGEP_HEADER_META					= _T("M");
// Client locale
const LPCTSTR GGEP_HEADER_CLIENT_LOCALE			= _T("LOC");
// Creation time
const LPCTSTR GGEP_HEADER_CREATE_TIME			= _T("CT");
// Firewalled Transfer support in Hits
const LPCTSTR GGEP_HEADER_FW_TRANS				= _T("FW");
// The extension header (key) indicating the GGEP block is the 'secure' block
const LPCTSTR GGEP_HEADER_SECURE_BLOCK			= _T("SB");
// The extension header (key) indicating the value has a signature in it
const LPCTSTR GGEP_HEADER_SIGNATURE				= _T("SIG");
// Chat support
const LPCTSTR GGEP_HEADER_CHAT					= _T("CHAT");
// Equivalent of GGEP SCP but for GnucDNA peers only
// Unlike SCP, it's also used as acknowledgment
const LPCTSTR GGEP_HEADER_SUPPORT_GDNA			= _T("DNA");
// Legacy buggy version of GnucDNA DIPP
const LPCTSTR GGEP_HEADER_GDNA_PACKED_IPPORTS_x	= _T("DIP");
// Equivalent of GGEP IPP but contains GnucDNA peers only
const LPCTSTR GGEP_HEADER_GDNA_PACKED_IPPORTS	= _T("DIPP");
// File hash. SHA1 only or SHA1 + Tiger
const LPCTSTR GGEP_HEADER_HASH					= _T("H");
// URN but without "urn:" prefix
const LPCTSTR GGEP_HEADER_URN					= _T("u");


class CGGEPBlock;
class CGGEPItem;
class CPacket;


class CGGEPBlock
{
// Construction
public:
	CGGEPBlock();
	virtual ~CGGEPBlock();

// Attributes
public:
	CGGEPItem*	m_pFirst;
	CGGEPItem*	m_pLast;
	BYTE*		m_pInput;
	DWORD		m_nInput;
	BYTE		m_nItemCount;

// Operations
public:
	void		Clear();
	CGGEPItem*	Add(LPCTSTR pszID);
	CGGEPItem*	Find(LPCTSTR pszID, DWORD nMinLength = 0) const;
public:
	BOOL		ReadFromPacket(CPacket* pPacket);
	BOOL		ReadFromString(LPCTSTR pszData);
	BOOL		ReadFromBuffer(LPVOID pszData, DWORD nLength);
	void		Write(CPacket* pPacket);
	void		Write(CString& str);
	static		CGGEPBlock* FromPacket(CPacket* pPacket);
protected:
	BOOL		ReadInternal();
	BYTE		ReadByte();
public:
	inline BOOL	IsEmpty() const
	{
		return ( m_nItemCount == 0 );
	}

	friend class CGGEPItem;
};


class CGGEPItem
{
// Construction
public:
	CGGEPItem(LPCTSTR pszID = NULL);
	virtual ~CGGEPItem();

// Attributes
public:
	CGGEPItem*	m_pNext;
	CString		m_sID;
	BYTE*		m_pBuffer;
	DWORD		m_nLength;
	DWORD		m_nPosition;
	bool		m_bCOBS;
	bool		m_bSmall;

// Operations
public:
	BOOL		IsNamed(LPCTSTR pszID) const;
	void		Read(LPVOID pData, int nLength);
	BYTE		ReadByte();
	void		Write(LPCVOID pData, int nLength);
	void		WriteByte(BYTE nValue);
	CString		ToString() const;
	void		WriteUTF8( LPCWSTR pszText);

protected:
	BOOL		ReadFrom(CGGEPBlock* pBlock, BYTE nFlags);
	void		WriteTo(CPacket* pPacket, bool bSmall, bool bNeedCOBS);
	void		WriteTo(CString& str, bool bSmall, bool bNeedCOBS);
protected:
	BOOL		Encode(BOOL bIfZeros = FALSE);
	BOOL		Decode();
	BOOL		Deflate(BOOL bIfSmaller = FALSE);
	BOOL		Inflate();
public:
	inline void	SetCOBS(void)
	{
		m_bCOBS = true;
	}

	inline void	UnsetCOBS(void)
	{
		m_bCOBS = false;
	}

	inline void	SetSmall(void)
	{
		m_bSmall = true;
	}

	inline void	UnsetSmall(void)
	{
		m_bSmall = false;
	}

	friend class CGGEPBlock;
};

#endif // !defined(AFX_GGEP_H__7081FCAC_A207_412A_BD18_A72F09F89F05__INCLUDED_)
