//
// FragmentedFile.cpp
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
#include "Settings.h"
#include "FragmentedFile.h"
#include "TransferFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#include <winioctl.h>


//////////////////////////////////////////////////////////////////////
// CFragmentedFile construction

CFragmentedFile::CFragmentedFile()
{
	m_pFile			= NULL;
	m_nTotal		= 0;
	m_nRemaining	= 0;
	m_nUnflushed	= 0;
	m_nFragments	= 0;
	m_pFirst		= NULL;
	m_pLast			= NULL;
}

CFragmentedFile::~CFragmentedFile()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile create

BOOL CFragmentedFile::Create(LPCTSTR pszFile, QWORD nLength)
{
	if ( m_pFile != NULL || m_nTotal > 0 ) return FALSE;
	if ( nLength == 0 ) return FALSE;
	
	m_pFile = TransferFiles.Open( pszFile, TRUE, TRUE );
	if ( m_pFile == NULL ) return FALSE;
	
	m_nRemaining = m_nTotal = nLength;
	m_nFragments = 1;
	
	m_pFirst = m_pLast = CFileFragment::New( NULL, NULL, 0, m_nTotal );
	
	if ( Settings.Downloads.SparseThreshold > 0 && theApp.m_bNT &&
		 m_nRemaining >= Settings.Downloads.SparseThreshold * 1024 )
	{
		DWORD dwOut = 0;
		HANDLE hFile = m_pFile->GetHandle( TRUE );
		
		if ( ! DeviceIoControl( hFile, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &dwOut, NULL ) )
		{
			DWORD nError = GetLastError();
			theApp.Message( MSG_ERROR, _T("Unable to set sparse file: \"%s\", Win32 error %x."), pszFile, nError );
		}
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile open

BOOL CFragmentedFile::Open(LPCTSTR pszFile)
{
	if ( m_pFile != NULL || m_nTotal == 0 ) return FALSE;
	
	m_pFile = TransferFiles.Open( pszFile, TRUE, FALSE );
	
	if ( m_pFile == NULL ) return FALSE;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile flush

BOOL CFragmentedFile::Flush()
{
	if ( m_nUnflushed == 0 ) return FALSE;
	if ( m_pFile == NULL || ! m_pFile->IsOpen() ) return FALSE;
	FlushFileBuffers( m_pFile->GetHandle() );
	m_nUnflushed = 0;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile close

void CFragmentedFile::Close()
{
	if ( m_pFile != NULL )
	{
		m_pFile->Release( TRUE );
		m_pFile = NULL;
		m_nUnflushed = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile clear

void CFragmentedFile::Clear()
{
	Close();
	
	m_pFirst->DeleteChain();
	
	m_nTotal = m_nRemaining = m_nUnflushed = m_nFragments = 0;
	m_pFirst = m_pLast = NULL;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile make complete

BOOL CFragmentedFile::MakeComplete()
{
	if ( m_nTotal == 0 || m_nRemaining == 0 ) return FALSE;
	
	m_pFirst->DeleteChain();
	m_nRemaining = m_nFragments = 0;
	m_pFirst = m_pLast = NULL;
	
	if ( m_pFile != NULL )
	{
		HANDLE hFile = m_pFile->GetHandle( TRUE );
		
		if ( hFile != INVALID_HANDLE_VALUE )
		{
			DWORD nSizeHigh	= (DWORD)( m_nTotal >> 32 );
			DWORD nSizeLow	= (DWORD)( m_nTotal & 0xFFFFFFFF );
			SetFilePointer( hFile, nSizeLow, (PLONG)&nSizeHigh, FILE_BEGIN );
			SetEndOfFile( hFile );
		}
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile serialize

void CFragmentedFile::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar << m_nTotal;
		ar << m_nRemaining;
		ar << m_nFragments;
		
		for ( CFileFragment* pFragment = m_pFirst ; pFragment ; )
		{
			pFragment->Serialize( ar );
			pFragment = pFragment->m_pNext;
		}
	}
	else
	{
		ASSERT( m_nTotal == 0 );
		
		if ( nVersion >= 29 )
		{
			ar >> m_nTotal;
			ar >> m_nRemaining;
		}
		else
		{
			DWORD nInt32;
			ar >> nInt32; m_nTotal = nInt32;
			ar >> nInt32; m_nRemaining = nInt32;
		}
		
		ar >> m_nFragments;
		
		CFileFragment* pPrevious = NULL;
		
		for ( DWORD nFragment = 0 ; nFragment < m_nFragments ; nFragment++ )
		{
			m_pLast = CFileFragment::New( pPrevious );
						
			if ( pPrevious ) pPrevious->m_pNext = m_pLast;
			else m_pFirst = m_pLast;
			pPrevious = m_pLast;
			
			m_pLast->Serialize( ar, nVersion >= 29 );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile simple fragment operations

void CFragmentedFile::SetEmptyFragments(CFileFragment* pInput)
{
	m_pFirst->DeleteChain();
	
	m_pFirst		= m_pLast = pInput;
	m_nFragments	= 0;
	m_nRemaining	= 0;
	
	for ( ; pInput ; pInput = pInput->m_pNext )
	{
		m_nFragments ++;
		m_nRemaining += pInput->m_nLength;
		m_pLast = pInput;
	}
}

CFileFragment* CFragmentedFile::CopyFreeFragments() const
{
	return m_pFirst->CreateCopy();
}

CFileFragment* CFragmentedFile::CopyFilledFragments() const
{
	return m_pFirst->CreateInverse( m_nTotal );
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile simple intersections

BOOL CFragmentedFile::IsPositionRemaining(QWORD nOffset) const
{
	if ( m_nRemaining == 0 || m_nFragments == 0 ) return FALSE;
	if ( nOffset >= m_nTotal ) return FALSE;
	
	for ( CFileFragment* pFragment = m_pFirst ; pFragment ; )
	{
		if ( nOffset >= pFragment->m_nOffset && nOffset < pFragment->m_nOffset + pFragment->m_nLength )
			return TRUE;
		
		pFragment = pFragment->m_pNext;
	}
	
	return FALSE;
}

BOOL CFragmentedFile::DoesRangeOverlap(QWORD nOffset, QWORD nLength) const
{
	if ( m_nRemaining == 0 || m_nFragments == 0 ) return FALSE;
	if ( nLength == 0 ) return FALSE;
	
	for ( CFileFragment* pFragment = m_pFirst ; pFragment ; pFragment = pFragment->m_pNext )
	{
		if ( nOffset <= pFragment->m_nOffset &&
			 nOffset + nLength >= pFragment->m_nOffset + pFragment->m_nLength )
		{
			if ( pFragment->m_nLength ) return TRUE;
		}
		else if (	nOffset > pFragment->m_nOffset &&
					nOffset + nLength < pFragment->m_nOffset + pFragment->m_nLength )
		{
			return TRUE;
		}
		else if (	nOffset + nLength > pFragment->m_nOffset &&
					nOffset + nLength < pFragment->m_nOffset + pFragment->m_nLength )
		{
			if ( nLength - ( pFragment->m_nOffset - nOffset ) ) return TRUE;
		}
		else if (	nOffset > pFragment->m_nOffset &&
					nOffset < pFragment->m_nOffset + pFragment->m_nLength )
		{
			if ( pFragment->m_nOffset + pFragment->m_nLength - nOffset ) return TRUE;
		}
	}
	
	return FALSE;
}

QWORD CFragmentedFile::GetRangeOverlap(QWORD nOffset, QWORD nLength) const
{
	if ( m_nRemaining == 0 || m_nFragments == 0 ) return 0;
	if ( nLength == 0 ) return 0;
	
	QWORD nOverlap = 0;
	
	for ( CFileFragment* pFragment = m_pFirst ; pFragment ; pFragment = pFragment->m_pNext )
	{
		if ( nOffset <= pFragment->m_nOffset &&
			 nOffset + nLength >= pFragment->m_nOffset + pFragment->m_nLength )
		{
			nOverlap += pFragment->m_nLength;
		}
		else if (	nOffset > pFragment->m_nOffset &&
					nOffset + nLength < pFragment->m_nOffset + pFragment->m_nLength )
		{
			nOverlap += nLength;
			break;
		}
		else if (	nOffset + nLength > pFragment->m_nOffset &&
					nOffset + nLength < pFragment->m_nOffset + pFragment->m_nLength )
		{
			nOverlap += nLength - ( pFragment->m_nOffset - nOffset );
		}
		else if (	nOffset > pFragment->m_nOffset &&
					nOffset < pFragment->m_nOffset + pFragment->m_nLength )
		{
			nOverlap += pFragment->m_nOffset + pFragment->m_nLength - nOffset;
		}
	}
	
	return nOverlap;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile write some data to a range

BOOL CFragmentedFile::WriteRange(QWORD nOffset, LPCVOID pData, QWORD nLength)
{
	if ( m_pFile == NULL ) return FALSE;
	if ( m_nRemaining == 0 || m_nFragments == 0 ) return FALSE;
	if ( nLength == 0 ) return TRUE;
	
	QWORD nResult, nProcessed = 0;
	
	for ( CFileFragment* pFragment = m_pFirst ; pFragment ; )
	{
		CFileFragment* pNext = pFragment->m_pNext;

		if ( nOffset <= pFragment->m_nOffset &&
			 nOffset + nLength >= pFragment->m_nOffset + pFragment->m_nLength )
		{
			LPBYTE pSource = (LPBYTE)pData + ( pFragment->m_nOffset - nOffset );
			
			if ( ! m_pFile->Write( pFragment->m_nOffset, pSource, pFragment->m_nLength, &nResult ) ) return FALSE;
						
			nProcessed		+= pFragment->m_nLength;
			m_nRemaining	-= pFragment->m_nLength;
			
			if ( pFragment->m_pPrevious )
				pFragment->m_pPrevious->m_pNext = pNext;
			else
				m_pFirst = pNext;

			if ( pNext )
				pNext->m_pPrevious = pFragment->m_pPrevious;
			else
				m_pLast = pFragment->m_pPrevious;
			
			pFragment->DeleteThis();
			m_nFragments --;
		}
		else if (	nOffset > pFragment->m_nOffset &&
					nOffset + nLength < pFragment->m_nOffset + pFragment->m_nLength )
		{
			if ( ! m_pFile->Write( nOffset, pData, nLength, &nResult ) ) return FALSE;
			
			nProcessed		+= nLength;
			m_nRemaining	-= nLength;
			
			CFileFragment* pNew = CFileFragment::New( pFragment, pNext );
			pNew->m_nOffset	= nOffset + nLength;
			pNew->m_nLength	= pFragment->m_nOffset + pFragment->m_nLength - pNew->m_nOffset;
			
			pFragment->m_nLength	= nOffset - pFragment->m_nOffset;
			pFragment->m_pNext		= pNew;
			
			if ( pNext )
				pNext->m_pPrevious = pNew;
			else
				m_pLast = pNew;
			
			m_nFragments++;
			
			break;
		}
		else if (	nOffset + nLength > pFragment->m_nOffset &&
					nOffset + nLength < pFragment->m_nOffset + pFragment->m_nLength )
		{
			LPBYTE pSource	= (LPBYTE)pData + ( pFragment->m_nOffset - nOffset );
			QWORD nFragment	= nLength - ( pFragment->m_nOffset - nOffset );
			
			if ( ! m_pFile->Write( pFragment->m_nOffset, pSource, nFragment, &nResult ) ) return FALSE;
			
			nProcessed		+= nFragment;
			m_nRemaining	-= nFragment;
			
			pFragment->m_nOffset	+= nFragment;
			pFragment->m_nLength	-= nFragment;
		}
		else if (	nOffset > pFragment->m_nOffset &&
					nOffset < pFragment->m_nOffset + pFragment->m_nLength )
		{
			QWORD nFragment	= pFragment->m_nOffset + pFragment->m_nLength - nOffset;
			
			if ( ! m_pFile->Write( nOffset, pData, nFragment, &nResult ) ) return FALSE;
			
			nProcessed		+= nFragment;
			m_nRemaining	-= nFragment;
			
			pFragment->m_nLength	-= nFragment;
		}
		
		pFragment = pNext;
	}
	
	m_nUnflushed += nProcessed;
	return nProcessed > 0;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile read some data from a range

BOOL CFragmentedFile::ReadRange(QWORD nOffset, LPVOID pData, QWORD nLength)
{
	if ( m_pFile == NULL ) return FALSE;
	if ( nLength == 0 ) return TRUE;
	
	if ( DoesRangeOverlap( nOffset, nLength ) ) return FALSE;
	
	QWORD nRead = 0;
	m_pFile->Read( nOffset, pData, nLength, &nRead );
	
	return nRead == nLength;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile invalidate a range

QWORD CFragmentedFile::InvalidateRange(QWORD nOffset, QWORD nLength)
{
	CFileFragment* pFull = CopyFilledFragments();
	QWORD nCount = CFileFragment::Subtract( &pFull, nOffset, nLength );
	SetEmptyFragments( pFull->CreateInverse( m_nTotal ) );
	pFull->DeleteChain();
	
	return nCount;
}
