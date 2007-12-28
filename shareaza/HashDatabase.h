//
// HashDatabase.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#if !defined(AFX_HASHDATABASE_H__01733EE8_54F0_440D_BAD9_59EE0272DD1C__INCLUDED_)
#define AFX_HASHDATABASE_H__01733EE8_54F0_440D_BAD9_59EE0272DD1C__INCLUDED_

#pragma once

typedef struct
{
	DWORD nIndex;
	DWORD nOffset;
	DWORD nLength;
} HASHDB_INDEX_1000;

typedef struct
{
	DWORD nIndex;
	DWORD nType;
	DWORD nOffset;
	DWORD nLength;
} HASHDB_INDEX_1001, HASHDB_INDEX;

class CTigerTree;
class CED2K;


class CHashDatabase
{
// Construction
public:
	CHashDatabase();
	virtual ~CHashDatabase();

// Attributes
protected:
	CCriticalSection m_pSection;
protected:
	CString		m_sPath;
	CFile		m_pFile;
	BOOL		m_bOpen;
protected:
	DWORD			m_nOffset;
	HASHDB_INDEX*	m_pIndex;
	DWORD			m_nIndex;
	DWORD			m_nBuffer;

// Operations
public:
	BOOL	Create();
	void	Close();
	BOOL	DeleteAll(DWORD nIndex);
protected:
	HASHDB_INDEX*	Lookup(DWORD nIndex, DWORD nType);
	HASHDB_INDEX*	PrepareToStore(DWORD nIndex, DWORD nType, DWORD nLength);
	BOOL			Erase(DWORD nIndex, DWORD nType);
	void			Commit();
public:
	BOOL	GetTiger(DWORD nIndex, CTigerTree* pTree);
	BOOL	StoreTiger(DWORD nIndex, CTigerTree* pTree);
	BOOL	DeleteTiger(DWORD nIndex);
public:
	BOOL	GetED2K(DWORD nIndex, CED2K* pSet);
	BOOL	StoreED2K(DWORD nIndex, CED2K* pSet);
	BOOL	DeleteED2K(DWORD nIndex);

};

extern CHashDatabase LibraryHashDB;

#endif // !defined(AFX_HASHDATABASE_H__01733EE8_54F0_440D_BAD9_59EE0272DD1C__INCLUDED_)
