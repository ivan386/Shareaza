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

#include "stdafx.h"

class CFileFragmentList;
class CFileFragmentQueue;
class CFileFragmentPool;

class CFileFragment
{
protected:
	QWORD			m_nOffset;
	QWORD			m_nNext;
	CFileFragment*	m_pNext;
	CFileFragment*	m_pPrevious;
public:
	inline	QWORD			Offset() const;
	inline	QWORD			Next() const;
	inline	QWORD			Length() const;
	inline	CFileFragment*	GetNext() const;
	inline	CFileFragment*	GetPrevious() const;

	friend CFileFragmentList;
	friend CFileFragmentQueue;
	friend CFileFragmentPool;
};

class CFileFragmentList
{
private:
		CFileFragment*	m_pFirst;
		CFileFragment*	m_pLast;
		CFileFragment*	m_pFirstFree;
		CFileFragment*	m_pLastFree;
		DWORD			m_nCount;					// # of fragments
		DWORD			m_nFree;
		QWORD			m_nSize;
public:
		CFileFragmentList();
		~CFileFragmentList();
private:
inline	void			New(const QWORD nOffset, const QWORD nNext, CFileFragment* pNext, CFileFragment* pPrevious);
inline	void			NewHead(const QWORD nOffset, const QWORD nNext);
inline	void			NewTail(const QWORD nOffset, const QWORD nNext);
public:
		void			Delete();
		void			Flush();
		void			Delete( CFileFragment* pFragment );
private:
		CFileFragment*	DeleteAndGetNext( CFileFragment* pFragment );
public:
inline	CFileFragment*	GetFirst() const;
inline	CFileFragment*	GetLast() const;
inline	QWORD			GetEnd() const;
inline	DWORD			GetCount() const;
inline	QWORD			GetSize() const;
		CFileFragment*	GetRandom(const BOOL bPreferZero = FALSE) const;
		CFileFragment*	GetLargest(const CPtrList* pExcept = NULL, const BOOL bZeroIsLargest = TRUE) const;
		void			Serialize(CArchive& ar, const int nVersion, const BOOL bUseArchiveCount = FALSE);
		QWORD			Add(const QWORD nOffset, const QWORD nNext);
		void			GetCopy(const CFileFragmentList& SourceList);
		void			GetInverse(const CFileFragmentList& SourceList, const QWORD nSize);
		void			GetAnd(const CFileFragmentList& SourceList1, const CFileFragmentList& SourceList2);
		QWORD			Subtract(const QWORD nOffset, const QWORD nNext);
		QWORD			Subtract(const CFileFragmentList& SubtractList);
		QWORD			Subtract(const CFileFragmentQueue& SubtractQueue);
		void			Extract(CFileFragmentList& SourceList, const QWORD nOffset, const QWORD nNext);
		void			Extract(CFileFragmentQueue& SourceQueue, const QWORD nOffset, const QWORD nNext);
		void			Merge(const CFileFragmentList& MergeList);
inline	void			Move(CFileFragmentList& SourceList);
inline	BOOL			IsEmpty() const;
		BOOL			ContainsRange(const QWORD nOffset, const QWORD nNext) const;
		BOOL			OverlapsRange(const QWORD nOffset, const QWORD nNext) const;
		QWORD			GetRangeOverlap(const QWORD nOffset, const QWORD nNext) const;
		CFileFragment*	FindNextFragment(const QWORD nOffset) const;
		BOOL			LessOrEqualMatch(const CFileFragmentList& SourceList, const QWORD nOffset, const QWORD nNext) const;
#ifdef _DEBUG
		BOOL			BelongsToList(const CFileFragment* pFragment) const;
		BOOL			ValidateThis() const;
		BOOL			ValidateThisWithSize() const;
#endif
};

class CFileFragmentQueue
{
private:
		CFileFragment*	m_pFirst;
		CFileFragment*	m_pLast;
		CFileFragment*	m_pFirstFree;
		CFileFragment*	m_pLastFree;
		DWORD			m_nCount;					// # of fragments
		DWORD			m_nFree;
public:
		CFileFragmentQueue();
		~CFileFragmentQueue();
private:
inline	void			New( const QWORD nOffset, const QWORD nNext );
public:
		void			Delete();
		void			Delete( CFileFragment* pFragment );
		CFileFragment*	DeleteAndGetNext( CFileFragment* pFragment );
inline	CFileFragment*	GetFirst() const;
inline	BOOL			GetFirst( QWORD &nOffset, QWORD &nLength );
inline	DWORD			GetCount() const;
		void			Add(const QWORD nOffset, const QWORD nNext);
		void			Subtract(const QWORD nOffset, const QWORD nNext);
		void			Subtract(const CFileFragmentList& SubtractList);
inline	void			Move(CFileFragmentQueue& SourceQueue);
inline	BOOL			IsEmpty() const;
		BOOL			ContainsRange(const QWORD nOffset, const QWORD nNext) const;
#ifdef _DEBUG
		BOOL			BelongsToQueue(const CFileFragment* pFragment) const;
		BOOL			ValidateThis() const;
#endif
};

class CFileFragmentPool
{
private:
		CFileFragment*	m_pFirstPool;
		CFileFragment*	m_pLastPool;
		CFileFragment*	m_pAvailable;
		CCriticalSection m_oSection;
		DWORD			m_nAvailable;
		DWORD			m_nTotal;
public:
		CFileFragmentPool();
		~CFileFragmentPool();
		void			Request( const DWORD nNeed, CFileFragment* &pFirst, CFileFragment* &pLast );
		void			Release( const DWORD nReleased, CFileFragment* pFirst, CFileFragment* pLast );
private:
		void			Expand( const DWORD nNeed );
};

extern CFileFragmentPool FileFragmentPool;

inline QWORD CFileFragment::Offset() const
{
	ASSERT( this != NULL );
	return m_nOffset;
}

inline QWORD CFileFragment::Next() const
{
	ASSERT( this != NULL );
	return m_nNext;
}

inline QWORD CFileFragment::Length() const
{
	ASSERT( this != NULL );
	return m_nNext - m_nOffset;
}

inline CFileFragment* CFileFragment::GetNext() const
{
	ASSERT( this != NULL );
	return m_pNext;
}

inline CFileFragment* CFileFragment::GetPrevious() const
{
	ASSERT( this != NULL );
	return m_pPrevious;
}

inline CFileFragment* CFileFragmentList::GetFirst() const
{
	ASSERT( this != NULL );
	return m_pFirst;
}

inline CFileFragment* CFileFragmentList::GetLast() const
{
	ASSERT( this != NULL );
	return m_pLast;
}

inline QWORD CFileFragmentList::GetEnd() const
{
	ASSERT( ValidateThisWithSize() );
	if ( m_pLast ) return m_pLast->m_nNext; else return 0;
}

inline DWORD CFileFragmentList::GetCount() const
{
	ASSERT( ValidateThisWithSize() );
	return m_nCount;
}

inline QWORD CFileFragmentList::GetSize() const
{
	ASSERT( ValidateThisWithSize() );
	return m_nSize;
}

inline void CFileFragmentList::Move(CFileFragmentList& SourceList)
{
	ASSERT( ValidateThis() );
	ASSERT( SourceList.ValidateThis() );
	Delete();
	m_pFirst = SourceList.m_pFirst;
	m_pLast = SourceList.m_pLast;
	m_nCount = SourceList.m_nCount;
	m_nSize = SourceList.m_nSize;
	SourceList.m_pLast = SourceList.m_pFirst = NULL;
	SourceList.m_nSize = SourceList.m_nCount = 0;
	ASSERT( ValidateThis() );
	ASSERT( SourceList.ValidateThis() );
}

inline BOOL CFileFragmentList::IsEmpty() const
{
	ASSERT( ValidateThis() );
	return ( ! m_pFirst );
}

inline CFileFragment* CFileFragmentQueue::GetFirst() const
{
	ASSERT( this != NULL );
	return m_pFirst;
}

inline BOOL CFileFragmentQueue::GetFirst( QWORD &nOffset, QWORD &nLength )
{
	if ( m_pFirst )
	{
		nOffset = m_pFirst->m_nOffset;
		nLength = m_pFirst->Length();
		Delete( m_pFirst );
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

inline DWORD CFileFragmentQueue::GetCount() const
{
	ASSERT( this != NULL );
	return m_nCount;
}

inline void CFileFragmentQueue::Move(CFileFragmentQueue& SourceQueue)
{
	ASSERT( ValidateThis() );
	ASSERT( SourceQueue.ValidateThis() );
	Delete();
	m_pFirst = SourceQueue.m_pFirst;
	m_pLast = SourceQueue.m_pLast;
	m_nCount = SourceQueue.m_nCount;
	SourceQueue.m_pLast = SourceQueue.m_pFirst = NULL;
	SourceQueue.m_nCount = 0;
	ASSERT( ValidateThis() );
	ASSERT( SourceQueue.ValidateThis() );
}
	
inline BOOL CFileFragmentQueue::IsEmpty() const
{
	ASSERT( ValidateThis() );
	return ( !m_pFirst );
}

#endif // !defined(AFX_FILEFRAGMENT_H__4AF38CF1_81D5_4D59_95D4_E08B8CB3749E__INCLUDED_)
