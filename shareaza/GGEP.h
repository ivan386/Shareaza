//
// GGEP.h
//
// Copyright (c) Shareaza Development Team, 2002-2011.
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

#define GGEP_MAGIC			0xC3 // GGEP extension prefix

#define GGEP_HDR_LAST		0x80 // Last extension in GGEP block
#define GGEP_HDR_COBS		0x40 // Whether COBS was used on payload
#define GGEP_HDR_DEFLATE	0x20 // Whether payload was deflated
#define GGEP_HDR_RESERVE	0x10 // Reserved. It must be set to 0.
#define GGEP_HDR_IDLEN		0x0F // Where ID length is stored

#define GGEP_LEN_MORE		0x80 // Continuation present
#define GGEP_LEN_LAST		0x40 // Last byte
#define GGEP_LEN_MASK		0x3F // Value


class CGGEPBlock;
class CGGEPItem;
class CPacket;


class CGGEPBlock
{
public:
	CGGEPBlock();
	virtual ~CGGEPBlock();

	CGGEPItem*	Add(LPCTSTR pszID);
	CGGEPItem*	Find(LPCTSTR pszID, DWORD nMinLength = 0) const;
	BOOL		ReadFromPacket(CPacket* pPacket);
	void		Write(CPacket* pPacket);

	inline BOOL	IsEmpty() const
	{
		return ( m_nItemCount == 0 );
	}

	inline DWORD GetCount() const
	{
		return m_nItemCount;
	}

	inline CGGEPItem* GetFirst() const
	{
		return m_pFirst;
	}

protected:
	CGGEPItem*	m_pFirst;
	CGGEPItem*	m_pLast;
	BYTE		m_nItemCount;
	const BYTE*	m_pInput;		// Current read position
	DWORD		m_nInput;		// Remaining size available for reading

	void		Clear();
	BOOL		ReadInternal();
	BYTE		ReadByte();
	CGGEPItem*	ReadItem(BYTE nFlags);
};


class CGGEPItem
{
public:
	CGGEPItem(LPCTSTR pszID = NULL);
	virtual ~CGGEPItem();

	CGGEPItem*	m_pNext;
	CString		m_sID;
	BYTE*		m_pBuffer;
	DWORD		m_nLength;
	DWORD		m_nPosition;

	inline BOOL	IsNamed(LPCTSTR pszID) const
	{
		return ( m_sID == pszID );
	}

	void		Read(LPVOID pData, int nLength);
	BYTE		ReadByte();
	void		Write(LPCVOID pData, int nLength);
	void		WriteByte(BYTE nValue);
	void		WriteShort(WORD nValue);
	void		WriteLong(DWORD nValue);
	void		WriteInt64(QWORD nValue);
	CString		ToString() const;
	void		WriteUTF8(const CString& strText);
	void		WriteVary(QWORD nValue);	// Write variable length (1..8 bytes) according parameter value

protected:
	void		WriteTo(CPacket* pPacket);
	BOOL		Encode();
	BOOL		Decode();
	BOOL		Deflate();
	BOOL		Inflate();

friend class CGGEPBlock;
};
