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
#include "BTInfo.h"
#include "Library.h"
#include "SharedFile.h"
#include "Uploads.h"

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
,	m_nRefCount		( 1 )
{
}

CFragmentedFile::~CFragmentedFile()
{
	ASSERT( m_nRefCount == 0 );

	Close();
}

ULONG CFragmentedFile::AddRef()
{
	return (ULONG)InterlockedIncrement( &m_nRefCount );
}

ULONG CFragmentedFile::Release()
{
	ULONG ref_count = (ULONG)InterlockedDecrement( &m_nRefCount );
	if ( ref_count )
		return ref_count;
	delete this;
	return 0;
}

#ifdef _DEBUG

void CFragmentedFile::AssertValid() const
{
	CObject::AssertValid();

	if ( m_oFile.size() != 0 )
	{
		ASSERT( m_oFile.front().m_nOffset == 0 );
		CVirtualFile::const_iterator j;
		for ( CVirtualFile::const_iterator i = m_oFile.begin(); i != m_oFile.end(); ++i )
		{
			ASSERT( (*i).m_nLength != 0 );
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
	ASSERT( nLength != 0 );

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

	QWORD nRealLength = pFile->GetSize();
	if ( pFile->IsExists() && nLength == SIZE_UNKNOWN )
	{
		nLength = nRealLength;
	}
	else if ( ! bWrite && nRealLength != nLength )
	{
		// Wrong file
		Close();
		return FALSE;
	}

	if ( nLength == SIZE_UNKNOWN )
	{
		// Unknown size
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

BOOL CFragmentedFile::Open(const CBTInfo& oInfo, BOOL bWrite, BOOL bCreate)
{
	QWORD nOffset = 0;
	for ( POSITION pos = oInfo.m_pFiles.GetHeadPosition() ; pos ; )
	{
		CBTInfo::CBTFile* pFile = oInfo.m_pFiles.GetNext( pos );

		CString strSource = pFile->FindFile();

		if ( ! Open( strSource, nOffset, pFile->m_nSize, bWrite, bCreate ) )
		{
			// Right file not found
			CString sErrorMessage, strFormat;
			LoadString( strFormat, IDS_BT_SEED_SOURCE_LOST );
			sErrorMessage.Format( strFormat, (LPCTSTR)pFile->m_sPath );
			return FALSE;
		}

		nOffset += pFile->m_nSize;
	}

	return TRUE;
}

BOOL CFragmentedFile::FindByPath(const CString& sPath) const
{
	CQuickLock oLock( m_pSection );

	for ( CVirtualFile::const_iterator i = m_oFile.begin(); i != m_oFile.end(); ++i )
		if ( ! (*i).m_sPath.CompareNoCase( sPath ) )
			// Our subfile
			return TRUE;

	return FALSE;
}

BOOL CFragmentedFile::IsOpen() const
{
	CQuickLock oLock( m_pSection );

	if ( m_oFile.empty() )
		// No subfiles
		return FALSE;

	for ( CVirtualFile::const_iterator i = m_oFile.begin(); i != m_oFile.end(); ++i )
		if ( ! (*i).m_pFile->IsOpen() )
			// Closed subfile
			return FALSE;

	return TRUE;
}

/*CFragmentedFile& CFragmentedFile::operator=(const CFragmentedFile& pFile)
{
	CQuickLock oLock1( pFile.m_pSection );
	CQuickLock oLock2( m_pSection );

	Close();

	// Copy fragments
	m_oFList = pFile.m_oFList;

	// Copy other data
	m_nUnflushed = pFile.m_nUnflushed;

	// Copy files
	for ( CVirtualFile::const_iterator i = pFile.m_oFile.begin();
		i != pFile.m_oFile.end(); ++i )
	{
		if ( ! Open( (*i).m_sPath, (*i).m_nOffset, (*i).m_nLength, (*i).m_bWrite, FALSE ) )
		{
			Close();
			return *this;
		}
	}

	ASSERT_VALID( this );

	return *this;
}*/

//////////////////////////////////////////////////////////////////////
// CFragmentedFile flush

BOOL CFragmentedFile::Flush()
{
	CQuickLock oLock( m_pSection );

	if ( m_nUnflushed == 0 )
		// No unflushed data left
		return FALSE;

	if ( m_oFile.empty() )
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
	if ( m_oFile.empty() )
		return;

	CQuickLock oLock( m_pSection );

	// Close own handles
	std::for_each( m_oFile.begin(), m_oFile.end(), Releaser() );

	m_nUnflushed = 0;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile make complete

BOOL CFragmentedFile::MakeComplete()
{
	CQuickLock oLock( m_pSection );

	ASSERT_VALID( this );

	if ( m_oFList.empty() )
		// No incomplete parts left
		return TRUE;

	if ( m_oFile.empty() )
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
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile write some data to a range

BOOL CFragmentedFile::Write(QWORD nOffset, LPCVOID pData, QWORD nLength, QWORD* pnWritten)
{
	ASSERT_VALID( this );

	if ( nLength == 0 )
		// No data to write
		return TRUE;

	CQuickLock oLock( m_pSection );

	Fragments::Fragment oMatch( nOffset, nOffset + nLength );
	Fragments::List::const_iterator_pair pMatches = m_oFList.equal_range( oMatch );
	if ( pMatches.first == pMatches.second )
		// Empty range
		return FALSE;

	QWORD nProcessed = 0;
	for ( ; pMatches.first != pMatches.second; ++pMatches.first )
	{
		QWORD nStart = max( pMatches.first->begin(), oMatch.begin() );
		QWORD nToWrite = min( pMatches.first->end(), oMatch.end() ) - nStart;

		const char* pSource
			= static_cast< const char* >( pData ) + ( nStart - oMatch.begin() );

		QWORD nWritten = 0;
		if ( ! VirtualWrite( nStart, pSource, nToWrite, &nWritten ) )
			// Write error
			return FALSE;

		if ( pnWritten )
			*pnWritten += nWritten;

		nProcessed += nWritten;
	}

	m_nUnflushed += nProcessed;
	m_oFList.erase( oMatch );
	return nProcessed > 0;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile read some data from a range

BOOL CFragmentedFile::Read(QWORD nOffset, LPVOID pData, QWORD nLength, QWORD* pnRead)
{
	ASSERT_VALID( this );

	if ( nLength == 0 )
		// No data to read
		return TRUE;

	CQuickLock oLock( m_pSection );

	if ( DoesRangeOverlap( nOffset, nLength ) )
		// No data available yet
		return FALSE;

	return VirtualRead( nOffset, (char*)pData, nLength, pnRead );
}

BOOL CFragmentedFile::VirtualRead(QWORD nOffset, char* pBuffer, QWORD nBuffer, QWORD* pnRead)
{
	ASSERT( nBuffer != 0 && nBuffer != SIZE_UNKNOWN );
	ASSERT( pBuffer != NULL && AfxIsValidAddress( pBuffer, nBuffer ) );

	// Find first file 
	CVirtualFile::const_iterator i = std::find_if( m_oFile.begin(), m_oFile.end(),
		bind2nd( Greater(), nOffset ) );
	ASSERT( i != m_oFile.begin() );
	--i;
	
	if ( pnRead )
		*pnRead = 0;

	while( nBuffer )
	{
		ASSERT( i != m_oFile.end() );
		ASSERT( (*i).m_nOffset <= nOffset );
		QWORD nPartOffset = ( nOffset - (*i).m_nOffset );
		ASSERT( (*i).m_nLength > nPartOffset );
		QWORD nPartLength = min( nBuffer, (*i).m_nLength - nPartOffset );

		QWORD nRead = 0;
		if ( ! (*i).m_pFile ||
			 ! (*i).m_pFile->Read( nPartOffset, pBuffer, nPartLength, &nRead ) )
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
	ASSERT( nBuffer != 0 && nBuffer != SIZE_UNKNOWN );
	ASSERT( pBuffer != NULL && AfxIsValidAddress( pBuffer, nBuffer ) );

	// Find first file 
	CVirtualFile::const_iterator i = std::find_if( m_oFile.begin(), m_oFile.end(),
		bind2nd( Greater(), nOffset ) );
	ASSERT( i != m_oFile.begin() );
	--i;

	if ( pnWritten )
		*pnWritten = 0;

	while( nBuffer )
	{
		ASSERT( i != m_oFile.end() );
		ASSERT( (*i).m_nOffset <= nOffset );
		QWORD nPartOffset = ( nOffset - (*i).m_nOffset );
		ASSERT( (*i).m_nLength > nPartOffset );
		QWORD nPartLength = min( nBuffer, (*i).m_nLength - nPartOffset );

		QWORD nWritten = 0;
		if ( ! (*i).m_pFile ||
			 ! (*i).m_pFile->Write( nPartOffset, pBuffer, nPartLength, &nWritten ) )
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

BOOL CFragmentedFile::EnsureWrite()
{
	CQuickLock oLock( m_pSection );

	return ( std::count_if( m_oFile.begin(), m_oFile.end(),
		EnsureWriter() ) == (int)m_oFile.size() );
}
