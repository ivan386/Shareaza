//
// GGEP.h
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

#if !defined(AFX_GGEP_H__7081FCAC_A207_412A_BD18_A72F09F89F05__INCLUDED_)
#define AFX_GGEP_H__7081FCAC_A207_412A_BD18_A72F09F89F05__INCLUDED_

#pragma once

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

// Operations
public:
	void		Clear();
	CGGEPItem*	Add(LPCTSTR pszID);
	CGGEPItem*	Find(LPCTSTR pszID, DWORD nMinLength = 0);
public:
	BOOL		ReadFromPacket(CPacket* pPacket);
	BOOL		ReadFromString(LPCTSTR pszData);
	void		Write(CPacket* pPacket);
	void		Write(CString& str);
	static		CGGEPBlock* FromPacket(CPacket* pPacket);
protected:
	BOOL	ReadInternal();
	BYTE	ReadByte();

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

// Operations
public:
	BOOL	IsNamed(LPCTSTR pszID);
	void	Read(LPVOID pData, int nLength);
	BYTE	ReadByte();
	void	Write(LPCVOID pData, int nLength);
	void	WriteByte(BYTE nValue);
	CString	ToString();
protected:
	BOOL	ReadFrom(CGGEPBlock* pBlock, BYTE nFlags);
	void	WriteTo(CPacket* pPacket);
	void	WriteTo(CString& str);
protected:
	BOOL	Encode(BOOL bIfZeros = FALSE);
	BOOL	Decode();
	BOOL	Deflate(BOOL bIfSmaller = FALSE);
	BOOL	Inflate();

	friend class CGGEPBlock;
};

//
// Constants
//

#define GGEP_HDR_LAST		0x80
#define GGEP_HDR_COBS		0x40
#define GGEP_HDR_DEFLATE	0x20
#define GGEP_HDR_IDLEN		0x0F
#define GGEP_LEN_MORE		0x80
#define GGEP_LEN_LAST		0x40
#define GGEP_LEN_MASK		0x3F

#endif // !defined(AFX_GGEP_H__7081FCAC_A207_412A_BD18_A72F09F89F05__INCLUDED_)
