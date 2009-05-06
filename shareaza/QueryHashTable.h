//
// QueryHashTable.h
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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
	bool				m_bLive;
	DWORD				m_nCookie;
	BYTE*				m_pHash;
	DWORD				m_nHash;
	DWORD				m_nBits;
	DWORD				m_nInfinity;
	DWORD				m_nCount;
	CBuffer*			m_pBuffer;
	CQueryHashGroup*	m_pGroup;

// Operations
public:
	void		Create();
	void		Clear();
	const bool	Merge(const CQueryHashTable& pSource);
	const bool	Merge(const CQueryHashGroup* pSource);
	const bool	PatchTo(CQueryHashTable* pTarget, CNeighbour* pNeighbour);
	const bool	OnPacket(CPacket* pPacket);
	const int	AddString(const CString& strString);
	const int	AddExactString(const CString& strString);
	const bool	CheckString(const CString& strString) const;
	const bool	Check(const CQuerySearch& pSearch) const;
	const bool	CheckHash(const DWORD nHash) const;
	const int	GetPercent() const;
	void		Draw(HDC hDC, const RECT* pRC);
protected:
	const bool	OnReset(CPacket* pPacket);
	const bool	OnPatch(CPacket* pPacket);
	const int	Add(LPCTSTR pszString, size_t nStart, size_t nLength);
	const int	AddExact(LPCTSTR pszString, size_t nStart, size_t nLength);
	const bool	PatchToOldShareaza(CQueryHashTable* pTarget, CNeighbour* pNeighbour);

// Statics
public:
	static const DWORD	HashWord(LPCTSTR pszString, size_t nStart, size_t nLength, DWORD nBits);
protected:
	static const DWORD	HashNumber(DWORD nNumber, int nBits);
};
