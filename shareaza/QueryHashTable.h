//
// QueryHashTable.h
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

#if !defined(AFX_QUERYHASHTABLE_H__4D0F77C8_D0BB_4AA5_97EA_3859BA3590B5__INCLUDED_)
#define AFX_QUERYHASHTABLE_H__4D0F77C8_D0BB_4AA5_97EA_3859BA3590B5__INCLUDED_

#pragma once

class CPacket;
class CBuffer;
class CQuerySearch;
class CXMLElement;
class CNeighbour;
class CQueryHashGroup;


class CQueryHashTable  
{
// Construction
public:
	CQueryHashTable();
	virtual ~CQueryHashTable();
	
// Attributes
public:
	BOOL		m_bLive;
	DWORD		m_nCookie;
public:
	BYTE*		m_pHash;
	DWORD		m_nHash;
	DWORD		m_nBits;
	DWORD		m_nInfinity;
	DWORD		m_nCount;
	CBuffer*	m_pBuffer;
public:
	CQueryHashGroup*	m_pGroup;
	
// Operations
public:
	void	Create();
	void	Clear();
	BOOL	Merge(CQueryHashTable* pSource);
	BOOL	Merge(CQueryHashGroup* pSource);
	BOOL	PatchTo(CQueryHashTable* pTarget, CNeighbour* pNeighbour);
	BOOL	OnPacket(CPacket* pPacket);
	int		AddPhrase(LPCTSTR pszPhrase);
	int		AddString(LPCTSTR pszString);
	BOOL	CheckPhrase(LPCTSTR pszSearch);
	BOOL	CheckString(LPCTSTR pszString);
	BOOL	Check(CQuerySearch* pSearch);
	int		GetPercent();
protected:
	BOOL	OnReset(CPacket* pPacket);
	BOOL	OnPatch(CPacket* pPacket);
	int		Add(LPCTSTR pszString, int nStart, int nLength);
	BOOL	PatchToOldShareaza(CQueryHashTable* pTarget, CNeighbour* pNeighbour);
protected:
	static inline DWORD	HashWord(LPCTSTR pszString, int nStart, int nLength, int nBits);
	static inline DWORD	HashNumber(DWORD nNumber, int nBits);
	
};

#endif // !defined(AFX_QUERYHASHTABLE_H__4D0F77C8_D0BB_4AA5_97EA_3859BA3590B5__INCLUDED_)
