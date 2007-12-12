//
// Buffer.h
//
// Copyright (c) Shareaza Development Team, 2007.
// This file is part of Shareaza Torrent Wizard (shareaza.sourceforge.net).
//
// Shareaza Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Torrent Wizard is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#pragma once


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
	LPVOID	Allocate(DWORD nLength);
	void	Add(const void* pData, DWORD nLength);
	void	Insert(DWORD nOffset, const void* pData, DWORD nLength);
	void	Remove(DWORD nLength);
	void	Clear();
	void	Print(LPCSTR pszText);
	DWORD	Append(CBuffer* pBuffer, DWORD nLength = 0xFFFFFFFF);
	BOOL	ReadLine(CString& strLine, BOOL bPeek = FALSE);

// Extras
public:
#ifdef _WINSOCKAPI_
	DWORD	Receive(SOCKET hSocket);
	DWORD	Send(SOCKET hSocket);
#endif
#ifdef _DEFLATE_
	BOOL	Deflate(BOOL bIfSmaller = FALSE);
	BOOL	Inflate(DWORD nSuggest = 0);
#endif

};

