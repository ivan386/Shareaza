//
// Buffer.h
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

#pragma once

#include "Hashes.h"

class CBuffer
{
// Construction
public:
	CBuffer(DWORD* pLimit = NULL);
	virtual ~CBuffer();

// Attributes
public:
	CBuffer*	m_pNext;
	BYTE*		m_pBuffer;
	DWORD		m_nLength;
	DWORD		m_nBuffer;
	
// Operations
public:
	void	Add(const void* pData, DWORD nLength);
	void	Insert(DWORD nOffset, const void* pData, DWORD nLength);
	void	Remove(DWORD nLength);
	void	Clear();
	void	Print(LPCSTR pszText);
	void	Print(LPCWSTR pszText, UINT nCodePage = CP_ACP);
	DWORD	AddBuffer(CBuffer* pBuffer, DWORD nLength = 0xFFFFFFFF);
	void	AddReversed(const void* pData, DWORD nLength);
	void	Prefix(LPCSTR pszText);
	void	EnsureBuffer(DWORD nLength);
public:
	CString	ReadString(DWORD nBytes, UINT nCodePage = CP_ACP);
	BOOL	ReadLine(CString& strLine, BOOL bPeek = FALSE, UINT nCodePage = CP_ACP);
	BOOL	StartsWith(LPCSTR pszString, BOOL bRemove = FALSE);
	DWORD	Receive(SOCKET hSocket);
	DWORD	Send(SOCKET hSocket);
	BOOL	Deflate(BOOL bIfSmaller = FALSE);
	BOOL	Inflate(DWORD nSuggest = 0);
	BOOL	Ungzip();
	void	WriteDIME(DWORD nFlags, LPCSTR pszID, LPCSTR pszType, LPCVOID pBody, DWORD nBody);
	BOOL	ReadDIME(DWORD* pnFlags, CString* psID, CString* psType, DWORD* pnBody);
public:
	static void ReverseBuffer(const void* pInput, void* pOutput, DWORD nLength);
	// Add a Hash to the Buffer
	template < int nHashSize > inline void Add(const CHash < nHashSize > &oHash)
	{
		Add( &oHash.m_b, sizeof oHash.m_b );
	}
	// Add a Managed Hash to the Buffer
	template < class Hash > inline void Add(const CManagedHash < Hash > &oHash)
	{
		ASSERT( oHash.IsValid() );
		Add( &oHash.m_b, sizeof oHash.m_b );
	}
};

