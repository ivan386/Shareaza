//
// FileFragment.h
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

#if !defined(AFX_FILEFRAGMENT_H__4AF38CF1_81D5_4D59_95D4_E08B8CB3749E__INCLUDED_)
#define AFX_FILEFRAGMENT_H__4AF38CF1_81D5_4D59_95D4_E08B8CB3749E__INCLUDED_

#pragma once

class CFileFragmentPool;


class CFileFragment
{
// Construction
protected:
	CFileFragment() {};
	~CFileFragment() {};

// Attributes
public:
	CFileFragment*	m_pPrevious;
	CFileFragment*	m_pNext;
public:
	QWORD			m_nOffset;
	QWORD			m_nLength;
	
// Operations
public:
	void			Serialize(CArchive& ar, BOOL bInt64 = TRUE);
public:
	CFileFragment*	CreateCopy();
	CFileFragment*	CreateInverse(QWORD nSize);
	CFileFragment*	CreateAnd(CFileFragment* pAvailable);
	int				GetCount();
	CFileFragment*	GetRandom(BOOL bPreferZero = FALSE);
	CFileFragment*	GetLargest(CPtrList* pExcept = NULL, BOOL bZeroIsLargest = TRUE);
	static QWORD	Subtract(CFileFragment** ppFragments, CFileFragment* pSubtract);
	static QWORD	Subtract(CFileFragment** ppFragments, QWORD nOffset, QWORD nLength);
	static void		AddMerge(CFileFragment** ppFragments, QWORD nOffset, QWORD nLength);
	
// Inlines
public:
	static inline	CFileFragment* New(CFileFragment* pPrevious = NULL, CFileFragment* pNext = NULL, QWORD nOffset = 0, QWORD nLength = 0);
	inline void		DeleteThis();
	inline void		DeleteChain();
	
	friend class CFileFragmentPool;
};


class CFileFragmentPool
{
// Construction
public:
	CFileFragmentPool();
	~CFileFragmentPool();

// Attributes
protected:
	CFileFragment*	m_pFree;
	DWORD			m_nFree;
protected:
	CCriticalSection	m_pSection;
	CPtrList			m_pPools;

// Operations
protected:
	void	Clear();
	void	NewPool();

// Primary Inline Operations
public:
	inline CFileFragment* New(CFileFragment* pPrevious = NULL, CFileFragment* pNext = NULL, QWORD nOffset = 0, QWORD nLength = 0)
	{
		m_pSection.Lock();
		
		if ( m_nFree == 0 ) NewPool();
		ASSERT( m_nFree > 0 );
		
		CFileFragment* pFragment = m_pFree;
		m_pFree = m_pFree->m_pNext;
		m_nFree --;
		
		m_pSection.Unlock();
		
		pFragment->m_pPrevious	= pPrevious;
		pFragment->m_pNext		= pNext;
		pFragment->m_nOffset	= nOffset;
		pFragment->m_nLength	= nLength;
		
		return pFragment;
	}
	
	inline void Delete(CFileFragment* pFragment)
	{
		m_pSection.Lock();
		pFragment->m_pPrevious = NULL;
		pFragment->m_pNext = m_pFree;
		m_pFree = pFragment;
		m_nFree ++;
		m_pSection.Unlock();
	}

};

extern CFileFragmentPool FileFragmentPool;

inline CFileFragment* CFileFragment::New(CFileFragment* pPrevious, CFileFragment* pNext, QWORD nOffset, QWORD nLength)
{
	return FileFragmentPool.New( pPrevious, pNext, nOffset, nLength );
}

inline void CFileFragment::DeleteThis()
{
	if ( this == NULL ) return;
	FileFragmentPool.Delete( this );
}

inline void CFileFragment::DeleteChain()
{
	if ( this == NULL ) return;
	
	for ( CFileFragment* pFragment = m_pNext ; pFragment ; )
	{
		CFileFragment* pNext = pFragment->m_pNext;
		FileFragmentPool.Delete( pFragment );
		pFragment = pNext;
	}
	
	FileFragmentPool.Delete( this );
}

#endif // !defined(AFX_FILEFRAGMENT_H__4AF38CF1_81D5_4D59_95D4_E08B8CB3749E__INCLUDED_)
