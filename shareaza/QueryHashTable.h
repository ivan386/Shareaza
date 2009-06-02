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

class CBuffer;
class CNeighbour;
class CPacket;
class CQueryHashGroup;
class CQuerySearch;
class CShareazaFile;
class CXMLElement;


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

public:
	// Split phrase to keywords
	static void		MakeKeywords(const CString& strPhrase, CStringList& oKeywords);
	static DWORD	HashWord(LPCTSTR pszString, size_t nStart, size_t nLength, DWORD nBits);

protected:
	// Split word to keywords (Katakana/Hiragana/Kanji helper)
	static void		MakeKeywords(const CString& strWord, WORD nWordType, CStringList& oKeywords);
	static DWORD	HashNumber(DWORD nNumber, int nBits);

// Operations
public:
	void	Create();
	void	Clear();
	bool	Merge(const CQueryHashTable* pSource);
	bool	Merge(const CQueryHashGroup* pSource);
	bool	PatchTo(const CQueryHashTable* pTarget, CNeighbour* pNeighbour);
	bool	OnPacket(CPacket* pPacket);
	// Add file hashes and file name splitted on keywords
	int		AddFile(const CShareazaFile& oFile);
	// Add file hashes
	int		AddHashes(const CShareazaFile& oFile);
	// Add string with steaming
	int		AddString(const CString& strString);
	// Add string exactly
	int		AddExactString(const CString& strString);
	bool	CheckString(const CString& strString) const;
	bool	Check(const CQuerySearch& oSearch) const;
	bool	CheckHash(const DWORD nHash) const;
	int		GetPercent() const;
	void	Draw(HDC hDC, const RECT* pRC);
protected:
	bool	OnReset(CPacket* pPacket);
	bool	OnPatch(CPacket* pPacket);
	int		Add(LPCTSTR pszString, size_t nStart, size_t nLength);
	int		AddExact(LPCTSTR pszString, size_t nStart, size_t nLength);
	bool	PatchToOldShareaza(const CQueryHashTable* pTarget, CNeighbour* pNeighbour);
};
