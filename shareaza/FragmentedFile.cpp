//
// FragmentedFile.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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

IMPLEMENT_DYNCREATE( CFragmentedFile, CObject )

//////////////////////////////////////////////////////////////////////
// CFragmentedFile construction

CFragmentedFile::CFragmentedFile() :
	m_nUnflushed	( 0 )
,	m_oFList		( 0 )
{
}

CFragmentedFile::~CFragmentedFile()
{
	Clear();
}

#ifdef _DEBUG

void CFragmentedFile::AssertValid() const
{
	CObject::AssertValid();

	ASSERT( m_oFile.size() != 0 );
	if ( m_oFile.size() != 0 )
	{
		ASSERT( m_oFile.front().m_nOffset == 0 );
		CVirtualFile::const_iterator j;
		for ( CVirtualFile::const_iterator i = m_oFile.begin(); i != m_oFile.end(); ++i )
		{
			ASSERT( (*i).m_nLength != 0 && (*i).m_nLength != SIZE_UNKNOWN );
			if ( i != m_oFile.begin() )
				ASSERT( (*j).m_nOffset + (*j).m_nLength == (*i).m_nOffset );
			j = i;
		}
	}
}

void CFragmentedFile::Dump(CDumpContext& dc) const
{
	CObject::Dump( dc );

	int n = 1;
	for ( CVirtualFile::const_iterator i = m_oFile.begin(); i != m_oFile.end(); ++i, ++n )
		dc << n << _T(". File offset ") << (*i).m_nOffset << _T(", ")
			<< (*i).m_nLength << _T(" bytes, ")
			<< ( (*i).m_bWrite ? _T("RW") : _T("RO") )
			<< _T(" \"") << (*i).m_sPath << _T("\"\n");
}

#endif

//////////////////////////////////////////////////////////////////////
// CFragmentedFile open

BOOL CFragmentedFile::Open(LPCTSTR pszFile, QWORD nOffset, QWORD nLength, BOOL bWrite, BOOL bCreate)
{
	ASSERT( pszFile && AfxIsValidString( pszFile ) );
	ASSERT( nLength != 0 && nLength != SIZE_UNKNOWN );
	ASSERT( ! bCreate || bWrite );

	CQuickLock oLock( m_pSection );

	if ( ! pszFile || ! *pszFile )
		// Bad file name
		return FALSE;

	CVirtualFile::const_iterator i = std::find( m_oFile.begin(), m_oFile.end(), pszFile );
	if ( i != m_oFile.end() )
		// Already opened
		return FALSE;

	CTransferFile* pFile = TransferFiles.Open( pszFile, bWrite, bCreate );
	if ( pFile == NULL )
	{
		if ( bWrite && ! bCreate &&
			GetFileAttributes( pszFile ) == INVALID_FILE_ATTRIBUTES )
		{
			// Recreate file
			pFile = TransferFiles.Open( pszFile, bWrite, TRUE );
		}
		if ( pFile == NULL )
		{
			// Failed to open
			Close();
			return FALSE;
		}
	}

	if ( ! bWrite && pFile->GetSize() != nLength )
	{
		// Wrong file
		Close();
		return FALSE;
	}

	CVirtualFilePart part;
	part.m_sPath = pszFile;
	part.m_pFile = pFile;
	part.m_nOffset = nOffset;
	part.m_nLength = nLength;
	part.m_bWrite = bWrite;
	m_oFile.push_back( part );
	m_oFile.sort();

	m_oFList.ensure( m_oFile.back().m_nOffset + m_oFile.back().m_nLength );

	if ( ! pFile->IsExists() )
	{
		// Add empty fragment
		InvalidateRange( nOffset, nLength );
	}

	ASSERT_VALID( this );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile flush

BOOL CFragmentedFile::Flush()
{
	CQuickLock oLock( m_pSection );

	if ( m_nUnflushed == 0 )
		// No unflushed data left
		return FALSE;

	if ( ! IsOpen() )
		// File not opened
		return FALSE;

	ASSERT_VALID( this );

	std::for_each( m_oFile.begin(), m_oFile.end(), Flusher() );

	m_nUnflushed = 0;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile close

void CFragmentedFile::Close()
{
	CQuickLock oLock( m_pSection );

	std::for_each( m_oFile.begin(), m_oFile.end(), Releaser() );
	m_oFile.clear();

	m_nUnflushed = 0;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile clear

void CFragmentedFile::Clear()
{
	CQuickLock oLock( m_pSection );

	Close();

	Fragments::List oNewList( 0 );
	m_oFList.swap( oNewList );
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile make complete

BOOL CFragmentedFile::MakeComplete()
{
	ASSERT_VALID( this );
	ASSERT( m_oFile.back().m_nOffset + m_oFile.back().m_nLength == m_oFList.limit() );

	CQuickLock oLock( m_pSection );

	if ( m_oFList.empty() )
		// No incomplete parts left
		return TRUE;

	if ( ! IsOpen() )
		// File is not opened
		return FALSE;

	m_oFList.clear();

	if ( m_oFList.limit() == SIZE_UNKNOWN )
		// Unknown size
		return TRUE;

	std::for_each( m_oFile.begin(), m_oFile.end(), Completer() );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile serialize

void CFragmentedFile::Serialize(CArchive& ar, int nVersion)
{
	CQuickLock oLock( m_pSection );

	if ( ar.IsStoring() )
	{
		ASSERT_VALID( this );

		SerializeOut1( ar, m_oFList );

		if ( nVersion >= 40 )
		{
			ar << m_oFile.size();
			for ( CVirtualFile::const_iterator i = m_oFile.begin();
				i != m_oFile.end(); ++i )
			{
				ar << (*i).m_sPath;
				ar << (*i).m_nOffset;
				ar << (*i).m_nLength;
				ar << (*i).m_bWrite;
			}
		}
	}
	else
	{
		SerializeIn1( ar, m_oFList, nVersion );

		if ( nVersion >= 40 )
		{
			size_t count = 0;
			ar >> count;
			for ( size_t i = 0; i < count; ++i )
			{
				CString sPath;
				ar >> sPath;
				QWORD nOffset = 0;
				ar >> nOffset;
				QWORD nLength = 0;
				ar >> nLength;
				BOOL bWrite = FALSE;
				ar >> bWrite;
				if ( ! Open( sPath, nOffset, nLength, bWrite, FALSE ) )
					AfxThrowArchiveException( CArchiveException::genericException );
			}
			ASSERT_VALID( this );
			ASSERT( m_oFile.back().m_nOffset + m_oFile.back().m_nLength == m_oFList.limit() );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile simple intersections

BOOL CFragmentedFile::IsPositionRemaining(QWORD nOffset) const
{
	CQuickLock oLock( m_pSection );

	return m_oFList.has_position( nOffset );
}

BOOL CFragmentedFile::DoesRangeOverlap(QWORD nOffset, QWORD nLength) const
{
	CQuickLock oLock( m_pSection );

	return m_oFList.overlaps( Fragments::Fragment( nOffset, nOffset + nLength ) );
}

QWORD CFragmentedFile::GetRangeOverlap(QWORD nOffset, QWORD nLength) const
{
	CQuickLock oLock( m_pSection );

	return m_oFList.overlapping_sum( Fragments::Fragment( nOffset, nOffset + nLength ) );
}

QWORD CFragmentedFile::GetCompleted(QWORD nOffset, QWORD nLength) const
{
	CQuickLock oLock( m_pSection );

	// TODO: Optimize this
	Fragments::List oList( m_oFList );	
	oList.insert( Fragments::Fragment( 0, nOffset ) );
	oList.insert( Fragments::Fragment( nOffset + nLength, m_oFList.limit() ) );

	return oList.missing();
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile write some data to a range

BOOL CFragmentedFile::WriteRange(QWORD nOffset, LPCVOID pData, QWORD nLength)
{
	ASSERT_VALID( this );
	ASSERT( m_oFile.back().m_nOffset + m_oFile.back().m_nLength == m_oFList.limit() );

	if ( nLength == 0 )
		// No data to write
		return TRUE;

	CQuickLock oLock( m_pSection );

	if ( ! IsOpen() )
		// File is not opened
		return FALSE;

	Fragments::Fragment oMatch( nOffset, nOffset + nLength );
	Fragments::List::const_iterator_pair pMatches = m_oFList.equal_range( oMatch );
	if ( pMatches.first == pMatches.second )
		// Empty range
		return FALSE;

	QWORD nResult, nProcessed = 0;

	for ( ; pMatches.first != pMatches.second; ++pMatches.first )
	{
		QWORD nStart = max( pMatches.first->begin(), oMatch.begin() );
		nResult = min( pMatches.first->end(), oMatch.end() ) - nStart;

		const char* pSource
			= static_cast< const char* >( pData ) + ( nStart - oMatch.begin() );

		if ( ! VirtualWrite( nStart, pSource, nResult, &nResult ) )
			// Write error
			return FALSE;

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
	ASSERT_VALID( this );
	ASSERT( m_oFile.back().m_nOffset + m_oFile.back().m_nLength == m_oFList.limit() );

	if ( nLength == 0 )
		// No data to read
		return TRUE;

	CQuickLock oLock( m_pSection );

	if ( ! IsOpen() )
		// File is not opened
		return FALSE;

	if ( DoesRangeOverlap( nOffset, nLength ) )
		// No data available yet
		return FALSE;

	return VirtualRead( nOffset, (char*)pData, nLength, NULL );
}

BOOL CFragmentedFile::VirtualRead(QWORD nOffset, char* pBuffer, QWORD nBuffer, QWORD* pnRead)
{
	ASSERT( nOffset < m_oFile.back().m_nOffset + m_oFile.back().m_nLength );
	ASSERT( nBuffer != 0 && nBuffer != SIZE_UNKNOWN );
	ASSERT( pBuffer != NULL && AfxIsValidAddress( pBuffer, nBuffer ) );

	// Find first file 
	CVirtualFile::const_iterator i = std::find_if( m_oFile.begin(), m_oFile.end(),
		bind2nd( Greater(), nOffset ) );
	ASSERT( i != m_oFile.begin() );
	--i;

	while( nBuffer )
	{
		ASSERT( i != m_oFile.end() );
		ASSERT( (*i).m_nOffset <= nOffset );
		QWORD nPartOffset = ( nOffset - (*i).m_nOffset );
		ASSERT( (*i).m_nLength > nPartOffset );
		QWORD nPartLength = min( nBuffer, (*i).m_nLength - nPartOffset );

		QWORD nRead = 0;
		if ( ! (*i).m_pFile->Read( nPartOffset, pBuffer, nPartLength, &nRead ) )
			// Read error
			return FALSE;

		pBuffer += nRead;
		nOffset += nRead;
		nBuffer -= nRead;
		if ( pnRead )
			*pnRead += nRead;

		if ( nRead != nPartLength )
			// EOF
			return FALSE;

		// Next part
		++i;
	}

	return TRUE;
}

BOOL CFragmentedFile::VirtualWrite(QWORD nOffset, const char* pBuffer, QWORD nBuffer, QWORD* pnWritten)
{
	ASSERT( nOffset < m_oFile.back().m_nOffset + m_oFile.back().m_nLength );
	ASSERT( nBuffer != 0 && nBuffer != SIZE_UNKNOWN );
	ASSERT( pBuffer != NULL && AfxIsValidAddress( pBuffer, nBuffer ) );

	// Find first file 
	CVirtualFile::const_iterator i = std::find_if( m_oFile.begin(), m_oFile.end(),
		bind2nd( Greater(), nOffset ) );
	ASSERT( i != m_oFile.begin() );
	--i;

	while( nBuffer )
	{
		ASSERT( i != m_oFile.end() );
		ASSERT( (*i).m_nOffset <= nOffset );
		QWORD nPartOffset = ( nOffset - (*i).m_nOffset );
		ASSERT( (*i).m_nLength > nPartOffset );
		QWORD nPartLength = min( nBuffer, (*i).m_nLength - nPartOffset );

		QWORD nWritten = 0;
		if ( !  (*i).m_pFile->Write( nPartOffset, pBuffer, nPartLength, &nWritten ) )
			// Write error
			return FALSE;

		pBuffer += nWritten;
		nOffset += nWritten;
		nBuffer -= nWritten;
		if ( pnWritten )
			*pnWritten += nWritten;

		if ( nWritten != nPartLength )
			// EOF
			return FALSE;

		// Next part
		++i;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile invalidate a range

QWORD CFragmentedFile::InvalidateRange(QWORD nOffset, QWORD nLength)
{
	CQuickLock oLock( m_pSection );

	return m_oFList.insert( Fragments::Fragment( nOffset, nOffset + nLength ) );
}
