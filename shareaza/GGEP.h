//
// GGEP.h
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
	CGGEPItem*	Find(LPCTSTR pszID, DWORD nMinLength = 0);
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
	inline BOOL	IsEmpty()
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
	BOOL		IsNamed(LPCTSTR pszID);
	void		Read(LPVOID pData, int nLength);
	BYTE		ReadByte();
	void		Write(LPCVOID pData, int nLength);
	void		WriteByte(BYTE nValue);
	CString		ToString();
	void		WriteUTF8( LPCWSTR pszText);

protected:
	BOOL		ReadFrom(CGGEPBlock* pBlock, BYTE nFlags);
	void		WriteTo(CPacket* pPacket, bool bSmall=true, bool bNeedCOBS=true);
	void		WriteTo(CString& str, bool bSmall=true, bool bNeedCOBS=true);
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
