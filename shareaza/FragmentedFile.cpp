//
// FragmentedFile.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
: m_pFile( NULL ), m_nUnflushed( 0 ), m_oFList( 0 )
{ }

CFragmentedFile::~CFragmentedFile()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile create

BOOL CFragmentedFile::Create(LPCTSTR pszFile, QWORD nLength)
{
	if ( m_pFile != NULL || m_oFList.limit() > 0 ) return FALSE;
	if ( nLength == 0 ) return FALSE;
	
	m_pFile = TransferFiles.Open( pszFile, TRUE, TRUE );
	if ( m_pFile == NULL ) return FALSE;

    m_oFList.swap( FF::SimpleFragmentList( nLength ) );

    m_oFList.insert( FF::SimpleFragment( 0, nLength ) );
	
	if ( Settings.Downloads.SparseThreshold > 0 && theApp.m_bNT &&
		 nLength >= Settings.Downloads.SparseThreshold * 1024 )
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
	if ( m_pFile != NULL || m_oFList.limit() == 0 ) return FALSE;
	
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

    m_oFList.swap( FF::SimpleFragmentList( 0 ) );

}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile make complete

BOOL CFragmentedFile::MakeComplete()
{
	if ( m_oFList.empty() ) return FALSE;

    m_oFList.clear();
	
	if ( m_pFile != NULL )
	{
		HANDLE hFile = m_pFile->GetHandle( TRUE );
		
		if ( hFile != INVALID_HANDLE_VALUE )
		{
			DWORD nSizeHigh	= (DWORD)( m_oFList.limit() >> 32 );
			DWORD nSizeLow	= (DWORD)( m_oFList.limit() & 0xFFFFFFFF );
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
        SerializeOut1( ar, m_oFList );
	}
	else
	{
		ASSERT( m_oFList.limit() == 0 );
		
        SerializeIn1( ar, m_oFList, nVersion );
	}
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile simple intersections

BOOL CFragmentedFile::IsPositionRemaining(QWORD nOffset) const
{
    return hasPosition( m_oFList, nOffset );
}

BOOL CFragmentedFile::DoesRangeOverlap(QWORD nOffset, QWORD nLength) const
{
    return overlaps( m_oFList, FF::SimpleFragment( nOffset, nOffset + nLength ) );
}

QWORD CFragmentedFile::GetRangeOverlap(QWORD nOffset, QWORD nLength) const
{
    return overlappingSum( m_oFList, FF::SimpleFragment( nOffset, nOffset + nLength ) );
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile write some data to a range

BOOL CFragmentedFile::WriteRange(QWORD nOffset, LPCVOID pData, QWORD nLength)
{
	if ( m_pFile == NULL ) return FALSE;
	if ( nLength == 0 ) return TRUE;

    FF::SimpleFragment oMatch( nOffset, nOffset + nLength );
    FF::SimpleFragmentList::ConstIteratorPair
        pMatches = m_oFList.overlappingRange( oMatch );
    if ( pMatches.first == pMatches.second ) return FALSE;

	QWORD nResult, nProcessed = 0;

    for ( ; pMatches.first != pMatches.second; ++pMatches.first )
    {
        QWORD nStart = std::max( pMatches.first->begin(), oMatch.begin() );
        nResult = std::min( pMatches.first->end(), oMatch.end() ) - nStart;

        const char* pSource
            = static_cast< const char* >( pData ) + nStart - oMatch.begin();

        if ( !m_pFile->Write( nStart, pSource, nResult, &nResult ) ) return FALSE;

        nProcessed += nResult;
	}
	
	m_nUnflushed += nProcessed;
    m_oFList.erase( oMatch );
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
    return m_oFList.insert( FF::SimpleFragment( nOffset, nOffset + nLength ) );
}
