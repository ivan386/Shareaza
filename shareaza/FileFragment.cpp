//
// FileFragment.cpp
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

#include "StdAfx.h"
#include "FileFragment.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CFileFragmentPool FileFragmentPool;

//////////////////////////////////////////////////////////////////////
// CFileFragmentList constructor

CFileFragmentList::CFileFragmentList()
{
	m_pLastFree = m_pFirstFree = m_pLast = m_pFirst = NULL;
	m_nSize = m_nFree = m_nCount = 0;
}

CFileFragmentList::~CFileFragmentList()
{
	Flush();
}

inline void CFileFragmentList::New(const QWORD nOffset, const QWORD nNext, CFileFragment* pNext, CFileFragment* pPrevious)
{
	ASSERT( ValidateThis() );
	CFileFragment* pFragment;
	if ( !m_nFree )
	{
		FileFragmentPool.Request( 64, m_pFirstFree, m_pLastFree );
		m_nFree = 64;
	}
	m_nFree--;
	pFragment = m_pFirstFree;
	m_pFirstFree = m_pFirstFree->m_pNext;
	pFragment->m_nOffset = nOffset;
	pFragment->m_nNext = nNext;
	pFragment->m_pNext = pNext;
	pFragment->m_pPrevious = pPrevious;
	m_nCount++;
	if ( pNext ) pNext->m_pPrevious = pFragment; else m_pLast = pFragment;
	if ( pPrevious ) pPrevious->m_pNext = pFragment; else m_pFirst = pFragment;
	m_nSize += ( nNext - nOffset );
	ASSERT( ValidateThis() );
}

inline void CFileFragmentList::NewHead(const QWORD nOffset, const QWORD nNext)
{
	ASSERT( ValidateThis() );
	if ( !m_nFree )
	{
		FileFragmentPool.Request( 64, m_pFirstFree, m_pLastFree );
		m_nFree = 64;
	}
	m_nFree--;
	if ( m_pFirst )
	{
		m_pFirst->m_pPrevious = m_pFirstFree;
		m_pFirstFree = m_pFirstFree->m_pNext;
		m_pFirst->m_pPrevious->m_pNext = m_pFirst;
		m_pFirst = m_pFirst->m_pPrevious;
	}
	else
	{
		m_pLast = m_pFirst = m_pFirstFree;
		m_pFirstFree = m_pFirstFree->m_pNext;
		m_pFirst->m_pNext = NULL;
	}
	m_pFirst->m_nOffset = nOffset;
	m_pFirst->m_nNext = nNext;
	m_pFirst->m_pPrevious = NULL;
	m_nCount++;
	m_nSize += ( nNext - nOffset );
	ASSERT( ValidateThis() );
}

inline void CFileFragmentList::NewTail(const QWORD nOffset, const QWORD nNext)
{
	ASSERT( ValidateThis() );
	if ( !m_nFree )
	{
		FileFragmentPool.Request( 64, m_pFirstFree, m_pLastFree );
		m_nFree = 64;
	}
	m_nFree--;
	m_pFirstFree->m_pPrevious = m_pLast;
	if ( m_pLast )
	{
		m_pLast->m_pNext = m_pFirstFree;
		m_pLast = m_pFirstFree;
	}
	else
	{
		m_pLast = m_pFirst = m_pFirstFree;
	}
	m_pLast = m_pFirstFree;
	m_pFirstFree = m_pFirstFree->m_pNext;
	m_pLast->m_nOffset = nOffset;
	m_pLast->m_nNext = nNext;
	m_pLast->m_pNext = NULL;
	m_nCount++;
	m_nSize += ( nNext - nOffset );
	ASSERT( ValidateThis() );
}

//////////////////////////////////////////////////////////////////////
// CFileFragment Delete

void CFileFragmentList::Delete()
{
	ASSERT( ValidateThis() );
	if ( m_pFirst )
	{
		m_pLast->m_pNext = m_pFirstFree;
		m_pFirstFree = m_pFirst;
		if ( !m_nFree ) m_pLastFree = m_pLast;
		m_nFree += m_nCount;
		m_pLast = m_pFirst = NULL;
		m_nSize = m_nCount = 0;
	}
}

void CFileFragmentList::Flush()
{
	ASSERT( ValidateThis() );
	if ( m_pFirst )
	{
		m_pLast->m_pNext = m_pFirstFree;
		m_pFirstFree = m_pFirst;
		if ( !m_nFree ) m_pLastFree = m_pLast;
		m_nFree += m_nCount;
		m_pLast = m_pFirst = NULL;
		m_nSize = m_nCount = 0;
	}
	if ( m_nFree ) FileFragmentPool.Release( m_nFree, m_pFirstFree, m_pLastFree );
	m_pLastFree = m_pFirstFree = NULL;
	m_nFree = 0;
}
void CFileFragmentList::Delete( CFileFragment* pFragment )
{
	ASSERT( ValidateThis() );
	ASSERT( pFragment != NULL );
	ASSERT( BelongsToList( pFragment ) );
	if ( pFragment->m_pPrevious )
	{
		if ( pFragment->m_pNext )
		{
			pFragment->m_pNext->m_pPrevious = pFragment->m_pPrevious;
			pFragment->m_pPrevious->m_pNext = pFragment->m_pNext;
		}
		else
		{
			m_pLast = pFragment->m_pPrevious;
			m_pLast->m_pNext = NULL;
		}
	}
	else
	{
		if ( pFragment->m_pNext )
		{
			m_pFirst = pFragment->m_pNext;
			m_pFirst->m_pPrevious = NULL;
		}
		else
		{
			m_pLast = m_pFirst = NULL;
		}
	}
	m_nCount--;
	pFragment->m_pNext = m_pFirstFree;
	m_pFirstFree = pFragment;
	if ( !m_nFree ) m_pLastFree = pFragment;
	m_nFree++;
	m_nSize -= pFragment->Length();
    ASSERT( ValidateThis() );
}

CFileFragment* CFileFragmentList::DeleteAndGetNext( CFileFragment* pFragment )
{
	ASSERT( pFragment != NULL );
	CFileFragment* pNext = pFragment->m_pNext;
	Delete( pFragment );
	return pNext;
}

//////////////////////////////////////////////////////////////////////
// CFileFragment find random fragment

CFileFragment* CFileFragmentList::GetRandom(const BOOL bPreferZero) const
{
	ASSERT( ValidateThisWithSize() );
	CFileFragment* pFragment = m_pFirst;
	if ( m_pFirst )
	{
		if ( ( ! bPreferZero ) || ( m_pFirst->m_nOffset != 0 ) )
		{
			QWORD nCount = rand() % m_nCount;
			while ( nCount-- ) pFragment = pFragment->m_pNext;
		}
	}
	return pFragment;
}

//////////////////////////////////////////////////////////////////////
// CFileFragment find largest fragment

CFileFragment* CFileFragmentList::GetLargest(const CPtrList* pExcept, const BOOL bZeroIsLargest) const
{
	ASSERT( ValidateThisWithSize() );
	CFileFragment *pFragment, *pLargest;
	QWORD nLargest = 0;
	if ( ( pLargest = pFragment = m_pFirst ) &&
		( !bZeroIsLargest || m_pFirst->m_nOffset || !pExcept || !pExcept->Find( (LPVOID)m_pFirst ) ) ) do
	{
		if ( ( pFragment->Length() > nLargest ) && ( !pExcept || !pExcept->Find( pFragment ) ) )
		{
			pLargest = pFragment;
			nLargest = pFragment->Length();
		}
	}
	while ( pFragment = pFragment->m_pNext );
	return pLargest;
}

//////////////////////////////////////////////////////////////////////
// CFileFragment serialize

void CFileFragmentList::Serialize(CArchive& ar, const int nVersion, const BOOL bUseArchiveCount)
{
	ASSERT( ValidateThisWithSize() );
	QWORD nOffset, nLength, nNext, nCount64;
	DWORD nInt32, nCount32;
	CFileFragment* pFragment;
	if ( nVersion >= 32 )
	{
		if ( ar.IsLoading() )
		{
			Delete();
			ar >> nCount32;
			while ( nCount32-- )
			{
				ar >> nOffset;
				ar >> nNext;
				Add( nOffset, nNext );
			}
		}
		else
		{
			nCount32 = GetCount();
			ar << nCount32;
			if ( pFragment = GetFirst() ) do
			{
				ar << pFragment->Offset();
				ar << pFragment->Next();
			}
			while ( pFragment = pFragment->GetNext() );
		}
		return;
	}
	if ( bUseArchiveCount )
	{
		if ( ar.IsLoading() )
		{
			Delete();
			if ( nVersion >= 29 )
			{
				nCount64 = ar.ReadCount();
				while ( nCount64-- )
				{
					ar >> nOffset;
					ar >> nLength;
					Add( nOffset, nOffset + nLength );
				}
			}
			else if ( nVersion >= 20 )
			{
				nCount64 = ar.ReadCount();
				while ( nCount64-- )
				{
					ar >> nInt32; nOffset = nInt32;
					ar >> nInt32; nLength = nInt32;
					Add( nOffset, nOffset + nLength );
				}
			}
			else if ( nVersion >= 5 )
			{
				while ( ar.ReadCount() )
				{
					ar >> nInt32; nOffset = nInt32;
					ar >> nInt32; nLength = nInt32;
					Add( nOffset, nOffset + nLength );
				}
			}
		}
		else
		{
			nCount32 = (DWORD)m_nCount;
			ar.WriteCount( nCount32 );
			pFragment = m_pFirst;
			while ( ( pFragment ) && ( nCount32-- ) )
			{
				ar << pFragment->Offset();
				ar << pFragment->Length();
				pFragment = pFragment->m_pNext;
			}
		}
	}
	else
	{
		if ( ar.IsLoading() )
		{
			ar >> nCount32;
			while ( nCount32-- )
			{
				ar >> nOffset;
				ar >> nLength;
				Add( nOffset, nOffset + nLength );
			}
		}
		else
		{
			nCount32 = (DWORD)m_nCount;
			ar << nCount32;
			pFragment = m_pFirst;
			while ( ( pFragment ) && ( nCount32-- ) )
			{
				ar << pFragment->m_nOffset;
				ar << pFragment->Length();
				pFragment = pFragment->m_pNext;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CFileFragmentList add fragment

QWORD CFileFragmentList::Add(QWORD nOffset, QWORD nNext)
{
	ASSERT( ValidateThisWithSize() );
	QWORD nOldSize = m_nSize;
	CFileFragment *pFragment, *pNext;
	if ( nNext > nOffset )
	{
		if ( m_pFirst == NULL ) NewHead( nOffset, nNext );
		else
		{	// test Tail
			if ( m_pLast->m_nOffset <= nOffset )
			{
				if ( m_pLast->m_nNext >= nOffset )
				{
					if ( m_pLast->m_nNext < nNext )
					{
						m_nSize += ( nNext - m_pLast->m_nNext );
						m_pLast->m_nNext = nNext;
					}
				}
				else NewTail( nOffset, nNext );
			}
			else
			{	// walk list
				pFragment = m_pFirst;
				while ( pFragment->m_nNext < nOffset ) pFragment = pFragment->m_pNext;
				if ( nNext < pFragment->m_nOffset ) New( nOffset, nNext, pFragment, pFragment->m_pPrevious );
				else
				{
					if ( nOffset < pFragment->m_nOffset )
					{
						m_nSize += ( pFragment->m_nOffset - nOffset );
						pFragment->m_nOffset = nOffset;
					}
					if ( nNext > pFragment->m_nNext )
					{
						m_nSize += ( nNext - pFragment->m_nNext );
						pFragment->m_nNext = nNext;
						while ( ( pNext = pFragment->m_pNext ) && ( pFragment->m_nNext >= pNext->m_nOffset ) )
						{
							if ( pNext->m_nNext > pFragment->m_nNext )
							{
								m_nSize += ( pNext->m_nNext - pFragment->m_nNext );
								pFragment->m_nNext = pNext->m_nNext;
							}
							Delete( pNext );
						}
					}
				}
			}
		}
	}
	ASSERT( ValidateThisWithSize() );
	ASSERT( m_nSize >= nOldSize );
	return m_nSize - nOldSize;
}

//////////////////////////////////////////////////////////////////////
// CFileFragment copy fragments

void CFileFragmentList::GetCopy(const CFileFragmentList &SourceList)
{
	ASSERT( ValidateThisWithSize() );
	ASSERT( SourceList.ValidateThisWithSize() );
	Delete();
	CFileFragment *pFragment;
	if ( pFragment = SourceList.m_pFirst )
	{
		if ( pFragment ) do
		{
			NewTail( pFragment->m_nOffset, pFragment->m_nNext );
		}
		while ( pFragment = pFragment->m_pNext );
	}
	ASSERT( ValidateThisWithSize() );
}

//////////////////////////////////////////////////////////////////////
// CFileFragment invert fragments

void CFileFragmentList::GetInverse(const CFileFragmentList& SourceList, const QWORD nSize)
{
	ASSERT( ValidateThisWithSize() );
	ASSERT( SourceList.ValidateThisWithSize() );
	Delete();
	ASSERT( SourceList.GetEnd() <= nSize );
	CFileFragment *pFragment;
	QWORD nLast = 0;
	if ( pFragment = SourceList.m_pFirst ) do
	{
		if ( nLast < pFragment->m_nOffset ) NewTail( nLast, pFragment->m_nOffset );
		nLast = pFragment->m_nNext;
	}
	while ( pFragment = pFragment->m_pNext );
	if ( nLast < nSize ) NewTail( nLast, nSize );
	ASSERT( ValidateThisWithSize() );
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile and fragments

void CFileFragmentList::GetAnd(const CFileFragmentList& SourceList1, const CFileFragmentList& SourceList2)
{
	ASSERT( ValidateThisWithSize() );
	ASSERT( SourceList1.ValidateThisWithSize() );
	ASSERT( SourceList2.ValidateThisWithSize() );
	Delete();
	CFileFragment *pFragment1, *pFragment2;
	if ( SourceList1.m_nCount >= SourceList2.m_nCount )
	{
		if ( ! ( pFragment1 = SourceList1.GetFirst() ) || ( ! ( pFragment2 = SourceList2.GetFirst() ) ) ) return;
	}
	else
	{
		if ( ! ( pFragment1 = SourceList2.GetFirst() ) || ( ! ( pFragment2 = SourceList1.GetFirst() ) ) ) return;
	}
	while ( TRUE )
	{
		if ( pFragment1->m_nNext <= pFragment2->m_nOffset )
		{
			if ( pFragment1 = pFragment1->m_pNext ) continue;
			return;
		}
		else if ( pFragment2->m_nNext <= pFragment1->m_nOffset )
		{
			if ( pFragment2 = pFragment2->m_pNext ) continue;
			return;
		}
		else if ( pFragment2->m_nNext >= pFragment1->m_nNext )
		{
			if ( pFragment2->m_nOffset <= pFragment1->m_nOffset )
			{
				NewTail( pFragment1->m_nOffset, pFragment1->m_nNext );
				if ( pFragment1 = pFragment1->m_pNext ) continue;
				return;
			}
			else
			{
				NewTail( pFragment2->m_nOffset, pFragment1->m_nNext );
				if ( pFragment1 = pFragment1->m_pNext ) continue;
				return;
			}
		}
		else
		{
			if ( pFragment2->m_nOffset <= pFragment1->m_nOffset )
			{
				NewTail( pFragment1->m_nOffset, pFragment2->m_nNext );
				if ( pFragment2 = pFragment2->m_pNext ) continue;
				return;
			}
			else
			{
				NewTail( pFragment2->m_nOffset, pFragment2->m_nNext);
				if ( pFragment2 = pFragment2->m_pNext ) continue;
				return;
			}
		}
	}
	ASSERT( FALSE );
}

//////////////////////////////////////////////////////////////////////
// CFileFragment subtract fragments

QWORD CFileFragmentList::Subtract(const QWORD nOffset, const QWORD nNext)
{
	ASSERT( ValidateThisWithSize() );
	QWORD nOldSize = m_nSize;
	CFileFragment* pFragment = m_pFirst;
	while ( pFragment )
	{
		if ( nOffset >= pFragment->m_nNext )
		{
			pFragment = pFragment->m_pNext;
		}
		else if ( nNext <= pFragment->m_nOffset ) return nOldSize - m_nSize;
		else if ( nOffset <= pFragment->m_nOffset )
		{
			if ( nNext >= pFragment->m_nNext )
			{
				pFragment = DeleteAndGetNext( pFragment );
			}
			else
			{
				m_nSize -= ( nNext - pFragment->m_nOffset );
				pFragment->m_nOffset = nNext;
				return nOldSize - m_nSize;
			}
		}
		else
		{
			if	( nNext >= pFragment->m_nNext )
			{
				m_nSize -= ( pFragment->m_nNext - nOffset );
				pFragment->m_nNext = nOffset;
				pFragment = pFragment->m_pNext;
			}
			else
			{
				m_nSize -= ( pFragment->m_nNext - nOffset );
				New( nNext, pFragment->m_nNext, pFragment->m_pNext, pFragment );
				pFragment->m_nNext = nOffset;
				return nOldSize - m_nSize;
			}
		}
	}
	ASSERT( ValidateThisWithSize() );
	return nOldSize - m_nSize;
}

QWORD CFileFragmentList::Subtract(const CFileFragmentList& SubtractList)
{
	ASSERT( ValidateThisWithSize() );
	ASSERT( SubtractList.ValidateThisWithSize() );
	QWORD nOldSize = m_nSize;
	CFileFragment* thisIterator = m_pFirst;
	CFileFragment* subtractIterator = SubtractList.m_pFirst;
	while ( subtractIterator && thisIterator )
	{
		if ( thisIterator->m_nNext <= subtractIterator->m_nOffset )
		{
			thisIterator = thisIterator->m_pNext;
		}
		else if ( subtractIterator->m_nNext <= thisIterator->m_nOffset )
		{
			subtractIterator = subtractIterator->m_pNext;
		}
		else if ( subtractIterator->m_nOffset <= thisIterator->m_nOffset )
		{
			if ( subtractIterator->m_nNext >= thisIterator->m_nNext )
			{
				thisIterator = DeleteAndGetNext( thisIterator );
			}
			else
			{
				m_nSize -= ( subtractIterator->m_nNext - thisIterator->m_nOffset );
				thisIterator->m_nOffset = subtractIterator->m_nNext;
				subtractIterator = subtractIterator->m_pNext;
			}
		}
		else
		{
			if	( subtractIterator->m_nNext >= thisIterator->m_nNext )
			{
				m_nSize -= ( thisIterator->m_nNext - subtractIterator->m_nOffset );
				thisIterator->m_nNext = subtractIterator->m_nOffset;
				thisIterator = thisIterator->m_pNext;
			}
			else
			{
				m_nSize -= ( thisIterator->m_nNext - subtractIterator->m_nOffset );
				New( subtractIterator->m_nNext, thisIterator->m_nNext, thisIterator->m_pNext, thisIterator );
				thisIterator->m_nNext = subtractIterator->m_nOffset;
				subtractIterator = subtractIterator->m_pNext;
			}
		}
	}
	ASSERT( ValidateThisWithSize() );
	return nOldSize - m_nSize;
}
QWORD CFileFragmentList::Subtract(const CFileFragmentQueue& SubtractQueue)
{
	ASSERT( ValidateThisWithSize() );
	ASSERT( SubtractQueue.ValidateThis() );
	QWORD nCount = 0;
	CFileFragment* pFragment;
	if ( pFragment = SubtractQueue.GetFirst() ) do
	{
		nCount += Subtract( pFragment->m_nOffset, pFragment->m_nNext );
	}
	while ( pFragment = pFragment->GetNext() );
	return nCount;
}

void CFileFragmentList::Extract(CFileFragmentList& SourceList, const QWORD nOffset, const QWORD nNext)
{
	ASSERT( nNext > nOffset );
	ASSERT( ValidateThisWithSize() );
	ASSERT( SourceList.ValidateThisWithSize() );
	Delete();
	CFileFragment* pFragment = SourceList.m_pFirst;
	while ( pFragment )
	{
		if ( nOffset >= pFragment->m_nNext )
		{
			pFragment = pFragment->m_pNext;
		}
		else if ( nNext <= pFragment->m_nOffset )
		{
			ASSERT( ValidateThis() );
			return;
		}
		else
		{
			if ( nOffset <= pFragment->m_nOffset )
			{
				if ( nNext >= pFragment->m_nNext )
				{
					Add( pFragment->m_nOffset, pFragment->m_nNext );
					pFragment = SourceList.DeleteAndGetNext( pFragment );
				}
				else
				{
					Add( pFragment->m_nOffset, nNext );
					SourceList.m_nSize -= ( nNext - pFragment->m_nOffset );
					pFragment->m_nOffset = nNext;
					ASSERT( ValidateThis() );
					return;
				}
			}
			else
			{
				if	( nNext >= pFragment->m_nNext )
				{
					Add( nOffset, pFragment->m_nNext );
					SourceList.m_nSize -= ( pFragment->m_nNext - nOffset );
					pFragment->m_nNext = nOffset;
					pFragment = pFragment->m_pNext;
				}
				else
				{
					Add( nOffset, nNext );
					SourceList.m_nSize -= ( pFragment->m_nNext - nOffset );
					SourceList.New( nNext, pFragment->m_nNext, pFragment->m_pNext, pFragment );
					pFragment->m_nNext = nOffset;
					ASSERT( ValidateThis() );
					return;
				}
			}
		}
	}
	ASSERT( ValidateThisWithSize() );
	ASSERT( SourceList.ValidateThisWithSize() );
}

void CFileFragmentList::Extract(CFileFragmentQueue& SourceQueue, const QWORD nOffset, const QWORD nNext)
{
	ASSERT( ValidateThisWithSize() );
	Delete();
	ASSERT( SourceQueue.ValidateThis() );
	CFileFragment *pNext, *pFragment = SourceQueue.GetFirst();
	if ( pFragment ) do
	{
		pNext = pFragment->m_pNext;
		if ( ( nOffset <= pFragment->m_nOffset ) && ( nNext >= pFragment->m_nNext ) )
		{
			Add( pFragment->m_nOffset, pFragment->m_nNext );
			SourceQueue.Delete( pFragment );
		}
	}
	while ( pFragment = pNext );
	ASSERT( ValidateThisWithSize() );
}

 //////////////////////////////////////////////////////////////////////
// CFileFragment Merge fragment lists

void CFileFragmentList::Merge(const CFileFragmentList& MergeList)
{
	ASSERT( ValidateThisWithSize() );
	ASSERT( MergeList.ValidateThisWithSize() );
	if ( MergeList.m_pFirst == NULL ) return;
	if ( m_pFirst == NULL )
	{
		GetCopy( MergeList );
		return;
	}
	CFileFragment* addIterator = MergeList.m_pFirst;
	CFileFragment* thisIterator = m_pFirst;
	CFileFragment *pNext;
	while ( addIterator && thisIterator )
	{
		if ( thisIterator->m_nNext < addIterator->m_nOffset )
		{
			thisIterator = thisIterator->m_pNext;
		}
		else if ( addIterator->m_nNext < thisIterator->m_nOffset )
		{
			New( addIterator->m_nOffset, addIterator->m_nNext, thisIterator, thisIterator->m_pPrevious );
			addIterator = addIterator->m_pNext;
		}
		else
		{
			if ( addIterator->m_nOffset < thisIterator->m_nOffset )
			{
				m_nSize += ( thisIterator->m_nOffset - addIterator->m_nOffset );
				thisIterator->m_nOffset = addIterator->m_nOffset;
			}
			if ( addIterator->m_nNext > thisIterator->m_nNext )
			{
				m_nSize += ( addIterator->m_nNext - thisIterator->m_nNext );
				thisIterator->m_nNext = addIterator->m_nNext;
				while ( ( pNext = thisIterator->m_pNext) && ( thisIterator->m_nNext >= pNext->m_nOffset ) )
				{
					if ( pNext->m_nNext > thisIterator->m_nNext )
					{
						m_nSize += ( pNext->m_nNext - thisIterator->m_nNext );
						thisIterator->m_nNext = pNext->m_nNext;
					}
					Delete( pNext );
				}
			}
			addIterator = addIterator->m_pNext;
		}
	}
	while ( addIterator )
	{
		Add( addIterator->m_nOffset, addIterator->m_nNext );
		addIterator = addIterator->m_pNext;
	}
	ASSERT( ValidateThisWithSize() );
}

BOOL CFileFragmentList::ContainsRange(QWORD nOffset, QWORD nNext) const
{
	ASSERT( ValidateThisWithSize() );
	CFileFragment* pFragment = m_pFirst;
	while ( pFragment && ( pFragment->m_nOffset < nNext ) )
	{
		if ( ( pFragment->m_nNext >= nNext ) && ( pFragment->m_nOffset <= nOffset ) ) return TRUE;
		pFragment = pFragment->m_pNext;
	}
	return FALSE;
}
BOOL CFileFragmentList::OverlapsRange(QWORD nOffset, QWORD nNext) const
{
	ASSERT( ValidateThisWithSize() );
	CFileFragment* pFragment = m_pFirst;
	while ( pFragment && ( pFragment->m_nOffset < nNext ) )
	{
		if ( pFragment->m_nNext > nOffset ) return TRUE;
		pFragment = pFragment->m_pNext;
	}
	return FALSE;
}
QWORD CFileFragmentList::GetRangeOverlap(const QWORD nOffset, const QWORD nNext) const
{
	ASSERT( ValidateThisWithSize() );
	QWORD nCount = 0;
	CFileFragment* pFragment = m_pFirst;
	while ( pFragment && ( pFragment->m_nOffset < nNext ) )
	{
		if ( pFragment->m_nNext > nOffset )
		{
			if ( pFragment->m_nNext < nNext )
			{
				if ( pFragment->m_nOffset <= nOffset )
				{
					nCount += ( pFragment->m_nNext - nOffset );
				}
				else
				{
					nCount += pFragment->Length();
				}
			}
			else
			{
				if ( pFragment->m_nOffset <= nOffset )
				{
					return nCount + ( nNext - nOffset );
				}
				else
				{
					return nCount + ( nNext - pFragment->m_nOffset );
				}
			}
		}
		pFragment = pFragment->m_pNext;
	}
	return nCount;
}

CFileFragment* CFileFragmentList::FindNextFragment(const QWORD nOffset) const
{
	ASSERT( ValidateThisWithSize() );
	CFileFragment *pFragment;
	if ( pFragment = m_pFirst ) do
	{
	}
	while ( ( pFragment->Offset() < nOffset ) && ( pFragment = pFragment->GetNext() ) );
	return pFragment;
}

BOOL CFileFragmentList::LessOrEqualMatch(const CFileFragmentList& SourceList, const QWORD nOffset, const QWORD nNext) const
{
	ASSERT( ValidateThisWithSize() );
	ASSERT( SourceList.ValidateThisWithSize() );
	CFileFragment *pThisFragment, *pSourceFragment;
	if ( pSourceFragment = SourceList.GetFirst() ) do
	{
	}
	while ( ( pSourceFragment->Next() <= nOffset ) && ( pSourceFragment = pSourceFragment->GetNext() ) );
	if ( pThisFragment = GetFirst() ) do
	{
	}
	while ( ( pThisFragment->Next() <= nOffset ) && ( pThisFragment = pThisFragment->GetNext() ) );
	while ( pThisFragment && pSourceFragment )
	{
		if ( pThisFragment->Offset() >= nNext ) return TRUE;
		else if ( pSourceFragment->Offset() >= nNext ) return FALSE;
		else if ( pSourceFragment->Next() <= pThisFragment->Offset() )
		{
			pSourceFragment = pSourceFragment->GetNext();
		}
		else if ( pThisFragment->Offset() <= nOffset )
		{
			if ( pSourceFragment->Offset() > nOffset ) return FALSE;
			else if ( pThisFragment->Next() > pSourceFragment->Next() ) return FALSE;
			else if ( pThisFragment->Next() >= nNext ) return pSourceFragment->Next() >= nNext;
			else
			{
				pThisFragment = pThisFragment->GetNext();
			}
		}
		else
		{
			if ( pThisFragment->Offset() < pSourceFragment->Offset() ) return FALSE;
			else if ( pThisFragment->Next() > pSourceFragment->Next() ) return FALSE;
			else if ( pThisFragment->Next() >= nNext ) return pSourceFragment->Next() >= nNext;
			else
			{
				pThisFragment = pThisFragment->GetNext();
			}
		}
	}
	if ( !pThisFragment ) return TRUE;
	do
	{
	}
	while ( pThisFragment->Next() <= nOffset && ( pThisFragment = pThisFragment->GetNext() ) );
	return ( !pThisFragment || pThisFragment->Offset() >= nNext );
}

#ifdef _DEBUG
BOOL CFileFragmentList::BelongsToList(const CFileFragment* pFragment) const
{
	if ( CFileFragment* pIterator = m_pFirst ) do
	{
		if ( pIterator == pFragment ) return TRUE;
	}
	while ( pIterator = pIterator->m_pNext );
	return FALSE;
}

BOOL CFileFragmentList::ValidateThis() const
{
	ASSERT( this != NULL );
	QWORD nCount = 0;
	if ( ! m_pFirst )
	{
		if ( m_pLast || m_nCount ) return FALSE;
	}
	else
	{
		if ( !m_pLast || !m_nCount ) return FALSE;
		if ( m_pFirst->m_pPrevious != NULL ) return FALSE;
	}
	if ( m_pLast && m_pLast->m_pNext != NULL ) return FALSE;
	CFileFragment* pFragment = m_pFirst;
	CFileFragment *pLast = NULL;
	while ( pFragment )
	{
		nCount++;
		if ( pFragment->m_pNext )
		{
			if ( pFragment->m_pNext->m_pPrevious != pFragment ) return FALSE;
		}
		if ( pFragment->m_nNext <= pFragment->m_nOffset ) return FALSE;
		pLast = pFragment;
		pFragment = pFragment->m_pNext;
	}
	return ( ( nCount == m_nCount ) && ( m_pLast == pLast ) );
}

BOOL CFileFragmentList::ValidateThisWithSize() const
{
	ASSERT( this != NULL );
	QWORD nCount = 0;
	QWORD nSize = 0;
	if ( ! m_pFirst )
	{
		if ( m_pLast || m_nCount ) return FALSE;
	}
	else
	{
		if ( !m_pLast || !m_nCount ) return FALSE;
		if ( m_pFirst->m_pPrevious != NULL ) return FALSE;
	}
	if ( m_pLast && m_pLast->m_pNext != NULL ) return FALSE;
	CFileFragment* pFragment = m_pFirst;
	CFileFragment *pLast = NULL;
	while ( pFragment )
	{
		nCount++;
		if ( pFragment->m_pNext )
		{
			if ( pFragment->m_pNext->m_pPrevious != pFragment ) return FALSE;
		}
		if ( pFragment->m_nNext <= pFragment->m_nOffset ) return FALSE;
		pLast = pFragment;
		nSize += pFragment->Length();
		pFragment = pFragment->m_pNext;
	}
	return ( ( nCount == m_nCount ) && ( m_pLast == pLast ) && ( nSize == m_nSize ) );
}
#endif

CFileFragmentQueue::CFileFragmentQueue()
{
	m_pLastFree = m_pFirstFree = m_pLast = m_pFirst = NULL;
	m_nFree = m_nCount = 0;
}

CFileFragmentQueue::~CFileFragmentQueue()
{
	Delete();
	if ( m_nFree ) FileFragmentPool.Release( m_nFree, m_pFirstFree, m_pLastFree );
}

inline void CFileFragmentQueue::New( const QWORD nOffset, const QWORD nNext )
{
	ASSERT( ValidateThis() );
	CFileFragment* pFragment;
	if ( !m_nFree )
	{
		FileFragmentPool.Request( 8, m_pFirstFree, m_pLastFree );
		m_nFree = 8;
	}
	m_nFree--;
	pFragment = m_pFirstFree;
	m_pFirstFree = m_pFirstFree->m_pNext;
	pFragment->m_nOffset = nOffset;
	pFragment->m_nNext = nNext;
	pFragment->m_pNext = NULL;
	pFragment->m_pPrevious = m_pLast;
	if ( m_pLast ) m_pLast->m_pNext = pFragment; else m_pFirst = pFragment;
	m_pLast = pFragment;
	m_nCount++;
	ASSERT( ValidateThis() );
}

//////////////////////////////////////////////////////////////////////
// CFileFragment Delete

void CFileFragmentQueue::Delete()
{
	ASSERT( ValidateThis() );
	if ( m_pFirst )
	{
		m_pLast->m_pNext = m_pFirstFree;
		m_pFirstFree = m_pFirst;
		if ( !m_nFree ) m_pLastFree = m_pLast;
		m_nFree += m_nCount;
		m_pLast = m_pFirst = NULL;
		m_nCount = 0;
	}
}

void CFileFragmentQueue::Delete( CFileFragment* pFragment )
{
	ASSERT( ValidateThis() );
	ASSERT( pFragment != NULL );
	ASSERT( BelongsToQueue( pFragment ) );
	if ( pFragment->m_pPrevious )
	{
		if ( pFragment->m_pNext )
		{
			pFragment->m_pNext->m_pPrevious = pFragment->m_pPrevious;
			pFragment->m_pPrevious->m_pNext = pFragment->m_pNext;
		}
		else
		{
			m_pLast = pFragment->m_pPrevious;
			m_pLast->m_pNext = NULL;
		}
	}
	else
	{
		if ( pFragment->m_pNext )
		{
			m_pFirst = pFragment->m_pNext;
			m_pFirst->m_pPrevious = NULL;
		}
		else
		{
			m_pLast = m_pFirst = NULL;
		}
	}
	m_nCount--;
	pFragment->m_pNext = m_pFirstFree;
	m_pFirstFree = pFragment;
	if ( !m_nFree ) m_pLastFree = pFragment;
	m_nFree++;
    ASSERT( ValidateThis() );
}

CFileFragment* CFileFragmentQueue::DeleteAndGetNext( CFileFragment* pFragment )
{
	ASSERT( pFragment != NULL );
	CFileFragment* pNext = pFragment->m_pNext;
	Delete( pFragment );
	return pNext;
}

void CFileFragmentQueue::Add(const QWORD nOffset, const QWORD nNext)
{
	ASSERT( ValidateThis() );
	if ( nNext > nOffset ) New( nOffset, nNext );
}

void CFileFragmentQueue::Subtract(const QWORD nOffset, const QWORD nNext)
{
	CFileFragment *pNext, *pFragment;
	if ( pFragment = m_pFirst ) do
	{
		pNext = pFragment->m_pNext;
		if ( ( nOffset < pFragment->m_nNext ) && ( nNext > pFragment->m_nOffset ) ) Delete( pFragment );
	}
	while ( pFragment = pNext );
}

void CFileFragmentQueue::Subtract(const CFileFragmentList& SubtractList)
{
	ASSERT( ValidateThis() );
	ASSERT( SubtractList.ValidateThis() );
	CFileFragment *pFragment;
	if ( pFragment = SubtractList.GetFirst() ) do
	{
		Subtract( pFragment->m_nOffset, pFragment->m_nNext );
	}
	while ( pFragment = pFragment->GetNext() );
}

BOOL CFileFragmentQueue::ContainsRange(const QWORD nOffset, const QWORD nNext) const
{
	ASSERT( nNext > nOffset );
	ASSERT( ValidateThis() );
	CFileFragment *pFragment;
	if ( pFragment = m_pFirst ) do
	{
		if ( ( pFragment->m_nOffset <= nOffset ) && ( pFragment->m_nNext >= nNext ) ) return TRUE;
	}
	while ( pFragment = pFragment->m_pNext );
	return FALSE;
}

#ifdef _DEBUG
BOOL CFileFragmentQueue::BelongsToQueue(const CFileFragment* pFragment) const
{
	if ( CFileFragment* pIterator = m_pFirst ) do
	{
		if ( pIterator == pFragment ) return TRUE;
	}
	while ( pIterator = pIterator->m_pNext );
	return FALSE;
}

BOOL CFileFragmentQueue::ValidateThis() const
{
	ASSERT( this != NULL );
	QWORD nCount = 0;
	if ( ! m_pFirst )
	{
		if ( m_pLast || m_nCount ) return FALSE;
	}
	else
	{
		if ( !m_pLast || !m_nCount ) return FALSE;
		if ( m_pFirst->m_pPrevious != NULL ) return FALSE;
	}
	if ( m_pLast && m_pLast->m_pNext != NULL ) return FALSE;
	CFileFragment* pFragment = m_pFirst;
	CFileFragment *pLast = NULL;
	while ( pFragment )
	{
		nCount++;
		if ( pFragment->m_pNext )
		{
			if ( pFragment->m_pNext->m_pPrevious != pFragment ) return FALSE;
		}
		pLast = pFragment;
		pFragment = pFragment->m_pNext;
	}
	return ( ( nCount == m_nCount ) && ( m_pLast == pLast ) );
}
#endif

CFileFragmentPool::CFileFragmentPool()
{
	m_oSection.Lock();
    m_pAvailable = m_pLastPool = m_pFirstPool = NULL;
	m_nTotal = m_nAvailable = 0;
	Expand( 32768 );					// start out with 32768 fragments == 768KB
	m_oSection.Unlock();
}

CFileFragmentPool::~CFileFragmentPool()
{
	CFileFragment* pFragmentPool, *pNextPool;
	if ( pFragmentPool = m_pFirstPool ) do
	{
		pNextPool = pFragmentPool[0].m_pNext;
		delete [] pFragmentPool;
	}
	while ( pFragmentPool = pNextPool );
    m_pAvailable = m_pLastPool = m_pFirstPool = NULL;
	m_nTotal = m_nAvailable = 0;
}

void CFileFragmentPool::Request( const DWORD nNeed, CFileFragment* &pFirst, CFileFragment* &pLast )
{
	ASSERT( nNeed > 0 );
	CFileFragment* pFragment;
	DWORD nCount = nNeed;
	m_oSection.Lock();
	if ( nNeed > m_nAvailable ) Expand( m_nTotal );
	pFragment = pFirst = m_pAvailable;
	while ( --nCount ) pFragment = pFragment->m_pNext;
	m_pAvailable = pFragment->m_pNext;
	m_nAvailable -= nNeed;
	m_oSection.Unlock();
	pLast = pFragment;
}

void CFileFragmentPool::Release( const DWORD nReleased, CFileFragment *pFirst, CFileFragment *pLast )
{
	ASSERT( nReleased > 0 );
	m_oSection.Lock();
	pLast->m_pNext = m_pAvailable;
	m_pAvailable = pFirst;
	m_nAvailable += nReleased;
	m_oSection.Unlock();
}

void CFileFragmentPool::Expand( const DWORD nNeed )
{	// already in Critical Section
	if ( m_pLastPool )
	{
		m_pLastPool[ 0 ].m_pNext = new CFileFragment[ nNeed + 1 ];
		m_pLastPool = m_pLastPool[ 0 ].m_pNext;
	}
	else
	{
		m_pFirstPool = m_pLastPool = new CFileFragment[ nNeed + 1 ];
	}
	m_pLastPool[ 0 ].m_pNext = NULL;
	for ( DWORD i = 1 ; i < nNeed ; i++ )
	{
		m_pLastPool[ i ].m_pNext = &m_pLastPool[ i + 1 ];
	}
	m_pLastPool[ i ].m_pNext = m_pAvailable;
	m_pAvailable = &m_pLastPool[ 1 ];
	m_nAvailable += nNeed;
	m_nTotal += nNeed;
}