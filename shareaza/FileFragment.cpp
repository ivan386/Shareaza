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
#include "Shareaza.h"
#include "FileFragment.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CFileFragmentPool	FileFragmentPool;


//////////////////////////////////////////////////////////////////////
// CFileFragment serialize

void CFileFragment::Serialize(CArchive& ar, BOOL bInt64)
{
	if ( ar.IsStoring() )
	{
		ar << m_nOffset;
		ar << m_nLength;
	}
	else if ( bInt64 )
	{
		ar >> m_nOffset;
		ar >> m_nLength;
	}
	else
	{
		DWORD nInt32;
		ar >> nInt32; m_nOffset = nInt32;
		ar >> nInt32; m_nLength = nInt32;
	}
}

//////////////////////////////////////////////////////////////////////
// CFileFragment copy fragments

CFileFragment* CFileFragment::CreateCopy()
{
	CFileFragment* pFirst = NULL;
	CFileFragment* pLast = NULL;
	
	for ( CFileFragment* pFragment = this ; pFragment ; pFragment = pFragment->m_pNext )
	{
		CFileFragment* pCopy = New( pLast, NULL, pFragment->m_nOffset, pFragment->m_nLength );
		if ( ! pFirst ) pFirst = pCopy;
		if ( pLast ) pLast->m_pNext = pCopy;
		pLast = pCopy;
	}
	
	return pFirst;
}

//////////////////////////////////////////////////////////////////////
// CFileFragment invert fragments

CFileFragment* CFileFragment::CreateInverse(QWORD nSize)
{
	CFileFragment* pFirst	= NULL;
	CFileFragment* pLast	= NULL;
	QWORD nLast = 0;
	
	for ( CFileFragment* pFragment = this ; pFragment ; pFragment = pFragment->m_pNext )
	{
		if ( pFragment->m_nOffset > nLast )
		{
			CFileFragment* pCopy = New( pLast, NULL, nLast, pFragment->m_nOffset - nLast );
			if ( ! pFirst ) pFirst = pCopy;
			if ( pLast ) pLast->m_pNext = pCopy;
			pLast = pCopy;
		}
		
		nLast = pFragment->m_nOffset + pFragment->m_nLength;
	}
	
	if ( nSize > nLast )
	{
		CFileFragment* pCopy = New( pLast, NULL, nLast, nSize - nLast );
		if ( ! pFirst ) pFirst = pCopy;
		if ( pLast ) pLast->m_pNext = pCopy;
		pLast = pCopy;
	}
	
	return pFirst;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile and fragments

CFileFragment* CFileFragment::CreateAnd(CFileFragment* pAvailable)
{
	CFileFragment* pFirst	= NULL;
	CFileFragment* pLast	= NULL;
	
	for ( CFileFragment* pFragment = this ; pFragment ; pFragment = pFragment->m_pNext )
	{
		for ( CFileFragment* pOther = pAvailable ; pOther ; pOther = pOther->m_pNext )
		{
			QWORD nOffset, nLength;
			
			if ( pOther->m_nOffset <= pFragment->m_nOffset &&
				 pOther->m_nOffset + pOther->m_nLength >= pFragment->m_nOffset + pFragment->m_nLength )
			{
				nOffset = pFragment->m_nOffset;
				nLength = pFragment->m_nLength;
			}
			else if (	pOther->m_nOffset > pFragment->m_nOffset &&
						pOther->m_nOffset + pOther->m_nLength < pFragment->m_nOffset + pFragment->m_nLength )
			{
				nOffset = pOther->m_nOffset;
				nLength = pOther->m_nLength;
			}
			else if (	pOther->m_nOffset + pOther->m_nLength > pFragment->m_nOffset &&
						pOther->m_nOffset + pOther->m_nLength < pFragment->m_nOffset + pFragment->m_nLength )
			{
				nOffset = pFragment->m_nOffset;
				nLength = pOther->m_nLength - ( pFragment->m_nOffset - pOther->m_nOffset );
			}
			else if (	pOther->m_nOffset > pFragment->m_nOffset &&
						pOther->m_nOffset < pFragment->m_nOffset + pFragment->m_nLength )
			{
				nOffset = pOther->m_nOffset;
				nLength = pFragment->m_nOffset + pFragment->m_nLength - pOther->m_nOffset;
			}
			else
			{
				continue;
			}
			
			CFileFragment* pCopy = New( pLast, NULL, nOffset, nLength );
			if ( pFirst == NULL ) pFirst = pCopy;
			if ( pLast != NULL ) pLast->m_pNext = pCopy;
			pLast = pCopy;
		}
	}
	
	return pFirst;
}

//////////////////////////////////////////////////////////////////////
// CFileFragment count the fragments in the list

int CFileFragment::GetCount()
{
	int nCount = 0;
	for ( CFileFragment* pCount = this ; pCount ; pCount = pCount->m_pNext ) nCount++;
	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CFileFragment find random fragment

CFileFragment* CFileFragment::GetRandom(BOOL bPreferZero)
{
	if ( this == NULL ) return NULL;
	
	int nCount = 0;
	
	for ( CFileFragment* pCount = this ; pCount ; pCount = pCount->m_pNext )
	{
		if ( bPreferZero && pCount->m_nOffset == 0 ) return pCount;
		nCount++;
	}
	
	ASSERT( nCount > 0 );
	
	nCount = rand() % nCount;
	
	for ( pCount = this ; pCount ; pCount = pCount->m_pNext )
	{
		if ( nCount-- == 0 ) return pCount;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CFileFragment find largest fragment

CFileFragment* CFileFragment::GetLargest(CPtrList* pExcept, BOOL bZeroIsLargest)
{
	if ( this == NULL ) return NULL;
	
	if ( bZeroIsLargest && m_nOffset == 0 )
	{
		if ( ! pExcept || pExcept->Find( (LPVOID)this ) == NULL ) return this;
	}
	
	CFileFragment* pLargest = NULL;
	
	for ( CFileFragment* pFragment = this ; pFragment ; )
	{
		if ( ! pLargest || pFragment->m_nLength > pLargest->m_nLength )
		{
			if ( ! pExcept || pExcept->Find( pFragment ) == NULL ) pLargest = pFragment;
		}

		pFragment = pFragment->m_pNext;
	}
	
	return pLargest;
}

//////////////////////////////////////////////////////////////////////
// CFileFragment subtract fragments

QWORD CFileFragment::Subtract(CFileFragment** ppFragments, CFileFragment* pSubtract)
{
	QWORD nCount = 0;
	
	for ( ; pSubtract ; pSubtract = pSubtract->m_pNext )
	{
		if ( ppFragments == NULL || *ppFragments == NULL ) break;
		nCount += Subtract( ppFragments, pSubtract->m_nOffset, pSubtract->m_nLength );
	}
	
	return nCount;
}

QWORD CFileFragment::Subtract(CFileFragment** ppFragments, QWORD nOffset, QWORD nLength)
{
	if ( ppFragments == NULL || *ppFragments == NULL ) return 0;
	
	QWORD nCount = 0;
	
	for ( CFileFragment* pFragment = *ppFragments ; pFragment ; )
	{
		CFileFragment* pNext = pFragment->m_pNext;
		
		if ( nOffset <= pFragment->m_nOffset &&
			 nOffset + nLength >= pFragment->m_nOffset + pFragment->m_nLength )
		{
			if ( pFragment->m_pPrevious )
				pFragment->m_pPrevious->m_pNext = pNext;
			else
				*ppFragments = pNext;
			
			if ( pNext )
				pNext->m_pPrevious = pFragment->m_pPrevious;
		//	else
		//		m_pLast = pFragment->m_pPrevious;
			
			nCount += pFragment->m_nLength;
			
			pFragment->DeleteThis();
		}
		else if (	nOffset > pFragment->m_nOffset &&
					nOffset + nLength < pFragment->m_nOffset + pFragment->m_nLength )
		{
			CFileFragment* pNew = New( pFragment, pNext );
			pNew->m_nOffset	= nOffset + nLength;
			pNew->m_nLength	= pFragment->m_nOffset + pFragment->m_nLength - pNew->m_nOffset;
			
			pFragment->m_nLength	= nOffset - pFragment->m_nOffset;
			pFragment->m_pNext		= pNew;
			nCount					+= nLength;
			
			if ( pNext )
				pNext->m_pPrevious = pNew;
		//	else
		//		m_pLast = pNew;
			
			break;
		}
		else if (	nOffset + nLength > pFragment->m_nOffset &&
					nOffset + nLength < pFragment->m_nOffset + pFragment->m_nLength )
		{
			QWORD nFragment	= nLength - ( pFragment->m_nOffset - nOffset );
			
			pFragment->m_nOffset	+= nFragment;
			pFragment->m_nLength	-= nFragment;
			nCount					+= nFragment;
		}
		else if (	nOffset > pFragment->m_nOffset &&
					nOffset < pFragment->m_nOffset + pFragment->m_nLength )
		{
			QWORD nFragment	= pFragment->m_nOffset + pFragment->m_nLength - nOffset;
			
			pFragment->m_nLength	-= nFragment;
			nCount					+= nFragment;
		}
		
		pFragment = pNext;
	}
	
	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CFileFragment add and merge a fragment

void CFileFragment::AddMerge(CFileFragment** ppFragments, QWORD nOffset, QWORD nLength)
{
	if ( ppFragments == NULL || nLength == 0 ) return;
	
	for ( CFileFragment* pFragment = *ppFragments ; pFragment ; pFragment = pFragment->m_pNext )
	{
		if ( pFragment->m_nOffset + pFragment->m_nLength == nOffset )
		{
			pFragment->m_nLength += nLength;
			break;
		}
		else if ( nOffset + nLength == pFragment->m_nOffset )
		{
			pFragment->m_nOffset -= nLength;
			pFragment->m_nLength += nLength;
			break;
		}
	}
	
	if ( pFragment == NULL )
	{
		CFileFragment* pFragment = New( NULL, *ppFragments, nOffset, nLength );
		if ( *ppFragments != NULL ) (*ppFragments)->m_pPrevious = pFragment;
		*ppFragments = pFragment;
		return;
	}
	
	for ( CFileFragment* pOther = *ppFragments ; pOther ; pOther = pOther->m_pNext )
	{
		if ( pFragment == pOther )
		{
			continue;
		}
		else if ( pFragment->m_nOffset + pFragment->m_nLength == pOther->m_nOffset )
		{
			pFragment->m_nLength += pOther->m_nLength;
		}
		else if ( pOther->m_nOffset + pOther->m_nLength == pFragment->m_nOffset )
		{
			pFragment->m_nOffset -= pOther->m_nLength;
			pFragment->m_nLength += pOther->m_nLength;
		}
		else
		{
			continue;
		}
		
		if ( pOther->m_pPrevious )
			pOther->m_pPrevious->m_pNext = pOther->m_pNext;
		else
			*ppFragments = pOther->m_pNext;

		if ( pOther->m_pNext )
			pOther->m_pNext->m_pPrevious = pOther->m_pPrevious;
		
		pOther->DeleteThis();
		break;
	}
}
	

//////////////////////////////////////////////////////////////////////
// CFileFragmentPool construction

CFileFragmentPool::CFileFragmentPool()
{
	m_pFree = NULL;
	m_nFree = 0;
}

CFileFragmentPool::~CFileFragmentPool()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CFileFragmentPool clear

void CFileFragmentPool::Clear()
{
	for ( POSITION pos = m_pPools.GetHeadPosition() ; pos ; )
	{
		CFileFragment* pPool = (CFileFragment*)m_pPools.GetNext( pos );
		delete [] pPool;
	}
	
	m_pPools.RemoveAll();
	m_pFree = NULL;
	m_nFree = 0;
}

//////////////////////////////////////////////////////////////////////
// CFileFragmentPool new pool setup

void CFileFragmentPool::NewPool()
{
	int nSize = 2048;
	
	CFileFragment* pPool = new CFileFragment[ nSize ];
	m_pPools.AddTail( pPool );
	
	while ( nSize-- > 0 )
	{
		pPool->m_pNext = m_pFree;
		m_pFree = pPool++;
		m_nFree++;
	}
}
