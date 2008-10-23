//
// DownloadWithFile.cpp
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
#include "Downloads.h"
#include "DownloadWithFile.h"
#include "DownloadWithTorrent.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadGroups.h"
#include "FragmentedFile.h"
#include "Uploads.h"

#include "ID3.h"
#include "XML.h"
#include "SchemaCache.h"
#include "LibraryBuilder.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadWithFile construction

CDownloadWithFile::CDownloadWithFile() :
	m_pFile		( new CFragmentedFile() )
,	m_tReceived	( GetTickCount() )
,	m_bDiskFull	( FALSE )
{
}

CDownloadWithFile::~CDownloadWithFile()
{
	if ( m_pFile != NULL ) delete m_pFile;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile open the file

BOOL CDownloadWithFile::OpenFile()
{
	if ( m_pFile == NULL || m_sName.IsEmpty() ) return FALSE;
	if ( m_pFile->IsOpen() ) return TRUE;

	SetModified();

	if ( m_pFile->IsValid() )
	{
		if ( m_pFile->Open( m_sPath, 0, m_nSize, TRUE, FALSE ) ) return TRUE;
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_FILE_OPEN_ERROR, (LPCTSTR)m_sPath );
	}
	else if ( m_nSize != SIZE_UNKNOWN && !Downloads.IsSpaceAvailable( m_nSize, Downloads.dlPathIncomplete ) )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_DISK_SPACE,
			m_sName,
			Settings.SmartVolume( m_nSize ) );
	}
	else
	{
		CString strLocalName = m_sPath;
		m_sPath.Empty();

		GenerateDiskName();

		for ( int nTry = 0 ; nTry < 5 ; nTry++ )
		{
			CString strName;

			if ( nTry == 0 )
				strName = m_sPath;
			else
				strName.Format( _T("%s.x%i"), (LPCTSTR)m_sPath, GetRandomNum( 0, 127 ) );

			theApp.Message( MSG_INFO, IDS_DOWNLOAD_FILE_CREATE, (LPCTSTR)strName );

			if ( m_pFile->Open( strName, 0, m_nSize, TRUE, TRUE ) )
			{
				theApp.WriteProfileString( _T("Delete"), strName, NULL );
				MoveFile( strLocalName + _T(".sd"), strName + _T(".sd") );
				m_sPath = strName;
				return TRUE;
			}

			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_FILE_CREATE_ERROR, (LPCTSTR)strName );
		}

		m_sPath = strLocalName;
	}

	m_bDiskFull = TRUE;

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile close the file

void CDownloadWithFile::CloseFile()
{
	if ( m_pFile != NULL ) m_pFile->Close();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile prepare file

BOOL CDownloadWithFile::PrepareFile()
{
	return OpenFile() && m_pFile->GetRemaining() > 0;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile delete the file

void CDownloadWithFile::DeleteFile(bool bForce)
{
	if ( m_pFile != NULL && m_pFile->IsValid() == FALSE ) return;

	// Close the file handle
	while( !Uploads.OnRename( m_sPath, NULL, bForce ) );

	if ( m_pFile != NULL )
	{
		if ( GetVolumeComplete() == 0 || ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 )
		{
			if ( !::DeleteFile( m_sPath ) )
				theApp.WriteProfileString( _T("Delete"), m_sPath, _T("") );
		}
		else
		{
			MoveFile( m_sPath, m_sPath + _T(".aborted") );
		}
	}
	else if ( bForce ) // be careful, do not delete completed BT seeding file
	{
		if ( !::DeleteFile( m_sPath ) )
			theApp.WriteProfileString( _T("Delete"), m_sPath, _T("") );
	}

	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile statistics

float CDownloadWithFile::GetProgress() const
{
	if ( m_nSize == 0 || m_nSize == SIZE_UNKNOWN ) return 0;
	if ( m_nSize == GetVolumeComplete() ) return 100.0f;
	return float( GetVolumeComplete() * 10000 / m_nSize ) / 100.0f;
}

QWORD CDownloadWithFile::GetVolumeComplete() const
{
	if ( m_pFile != NULL )
	{
		if ( m_pFile->IsValid() )
			return m_pFile->GetCompleted();
		else
			return 0;
	}
	return m_nSize;
}

QWORD CDownloadWithFile::GetVolumeRemaining() const
{
	if ( m_pFile != NULL )
	{
		if ( m_pFile->IsValid() )
			return m_pFile->GetRemaining();
		else
			return m_nSize;
	}
	return 0;
}

DWORD CDownloadWithFile::GetTimeRemaining() const
{
	QWORD nRemaining	= GetVolumeRemaining();
	DWORD nSpeed		= GetAverageSpeed();
	if ( nSpeed == 0 || nRemaining == SIZE_UNKNOWN ) return 0xFFFFFFFF;
	QWORD nTimeRemaining = nRemaining / nSpeed;
	return ( ( nTimeRemaining > 0xFFFFFFFF ) ? 0xFFFFFFFF : (DWORD)nTimeRemaining );
}

CString CDownloadWithFile::GetDisplayName() const
{
	if ( m_sName.GetLength() ) return m_sName;

	CString strName;

	if ( m_oSHA1 )
		strName = m_oSHA1.toShortUrn();
	else if ( m_oTiger )
		strName = m_oTiger.toShortUrn();
	else if ( m_oED2K )
		strName = m_oED2K.toShortUrn();
	else if ( m_oBTH )
		strName = m_oBTH.toShortUrn();
	else if ( m_oMD5 )
		strName = m_oMD5.toShortUrn();
	else
		strName = _T("Unknown File");

	return strName;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile get a list of possible download fragments

Fragments::List CDownloadWithFile::GetPossibleFragments(
	const Fragments::List& oAvailable, Fragments::Fragment& oLargest)
{
	if ( !PrepareFile() ) return Fragments::List( oAvailable.limit() );
	Fragments::List oPossible( oAvailable );

	if ( oAvailable.empty() )
	{
		oPossible = GetEmptyFragmentList();
	}
	else
	{
		Fragments::List tmp( inverse( GetEmptyFragmentList() ) );
		oPossible.erase( tmp.begin(), tmp.end() );
	}

	if ( oPossible.empty() ) return oPossible;

	oLargest = *oPossible.largest_range();

	for ( CDownloadTransfer* pTransfer = GetFirstTransfer();
		!oPossible.empty() && pTransfer;
		pTransfer = pTransfer->m_pDlNext )
	{
		pTransfer->SubtractRequested( oPossible );
	}

	return oPossible;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile select a fragment for a transfer

inline DWORD CalcChunkSize(QWORD nSize)
{
	if ( nSize <= 268435456 ) return 1024 * 1024; // try to keep chunk size reasonably large
	DWORD nChunk = DWORD( ( nSize - 1 ) / 256 ), nTemp; // default treeheight of 9
	while ( nTemp = nChunk & ( nChunk - 1 ) ) nChunk = nTemp;
	return nChunk * 2;
}

BOOL CDownloadWithFile::GetFragment(CDownloadTransfer* pTransfer)
{
	if ( !PrepareFile() )
		return NULL;

	Fragments::Fragment oLargest( SIZE_UNKNOWN, SIZE_UNKNOWN );

	Fragments::List oPossible = GetPossibleFragments(
		pTransfer->m_pSource->m_oAvailable, oLargest );

	if ( oLargest.begin() == SIZE_UNKNOWN )
	{
		ASSERT( oPossible.empty() );
		return FALSE;
	}

	if ( !oPossible.empty() )
	{
		Fragments::List::const_iterator pRandom = oPossible.begin()->begin() == 0
			? oPossible.begin()
			: oPossible.random_range();

		pTransfer->m_nOffset = pRandom->begin();
		pTransfer->m_nLength = pRandom->size();

		return TRUE;
	}
	else
	{
		CDownloadTransfer* pExisting = NULL;

		for ( CDownloadTransfer* pOther = GetFirstTransfer() ; pOther ; pOther = pOther->m_pDlNext )
		{
			if ( pOther->m_bRecvBackwards )
			{
				if ( pOther->m_nOffset + pOther->m_nLength - pOther->m_nPosition
					 != oLargest.end() ) continue;
			}
			else
			{
				if ( pOther->m_nOffset + pOther->m_nPosition != oLargest.begin() ) continue;
			}

			pExisting = pOther;
			break;
		}

		if ( pExisting == NULL )
		{
			pTransfer->m_nOffset = oLargest.begin();
			pTransfer->m_nLength = oLargest.size();
			return TRUE;
		}

		if ( oLargest.size() < 32 ) return FALSE;

		DWORD nOldSpeed	= pExisting->GetAverageSpeed();
		DWORD nNewSpeed	= pTransfer->GetAverageSpeed();
		QWORD nLength	= oLargest.size() / 2;

		if ( nOldSpeed > 5 && nNewSpeed > 5 )
		{
			nLength = oLargest.size() * nNewSpeed / ( nNewSpeed + nOldSpeed );

			if ( oLargest.size() > 102400 )
			{
				nLength = max( nLength, 51200ull );
				nLength = min( nLength, oLargest.size() - 51200ull );
			}
		}

		if ( pExisting->m_bRecvBackwards )
		{
			pTransfer->m_nOffset		= oLargest.begin();
			pTransfer->m_nLength		= nLength;
			pTransfer->m_bWantBackwards	= FALSE;
		}
		else
		{
			pTransfer->m_nOffset		= oLargest.end() - nLength;
			pTransfer->m_nLength		= nLength;
			pTransfer->m_bWantBackwards	= TRUE;
		}

		return TRUE;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile check if a byte position is empty

BOOL CDownloadWithFile::IsPositionEmpty(QWORD nOffset)
{
	if ( m_pFile == NULL || !m_pFile->IsValid() )
		return FALSE;

	return m_pFile->IsPositionRemaining( nOffset );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile check if a range would "help"

BOOL CDownloadWithFile::AreRangesUseful(const Fragments::List& oAvailable)
{
	if ( m_pFile == NULL || !m_pFile->IsValid() )
		return FALSE;

	return GetEmptyFragmentList().overlaps( oAvailable );
}

BOOL CDownloadWithFile::IsRangeUseful(QWORD nOffset, QWORD nLength)
{
	if ( m_pFile == NULL || !m_pFile->IsValid() )
		return FALSE;

	return GetEmptyFragmentList().overlaps( Fragments::Fragment( nOffset, nOffset + nLength ) );
}

// like IsRangeUseful( ) but take the amount of useful ranges relative to the amount of garbage
// and source speed into account
BOOL CDownloadWithFile::IsRangeUsefulEnough(CDownloadTransfer* pTransfer, QWORD nOffset, QWORD nLength)
{
	if ( m_pFile == NULL || !m_pFile->IsValid() )
		return FALSE;

	// range is useful if at least byte within the next amount of data transferable within the next 5 seconds
	// is useful
	DWORD nLength2 = 5 * pTransfer->GetAverageSpeed();
	if ( nLength2 < nLength )
	{
		if ( !pTransfer->m_bRecvBackwards ) nOffset += nLength - nLength2;
		nLength = nLength2;
	}
	return GetEmptyFragmentList().overlaps( Fragments::Fragment( nOffset, nOffset + nLength ) );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile get a string of available ranges

CString CDownloadWithFile::GetAvailableRanges() const
{
	CString strRange, strRanges;

	if ( m_pFile == NULL || !m_pFile->IsValid() )
		return strRanges;

	Fragments::List oAvailable( inverse( GetEmptyFragmentList() ) );

	for ( Fragments::List::const_iterator pFragment = oAvailable.begin();
		pFragment != oAvailable.end(); ++pFragment )
	{
		if ( strRanges.IsEmpty() )
		{
			strRanges = _T("bytes ");
		}
		else
		{
			strRanges += ',';
		}

		strRange.Format( _T("%I64i-%I64i"), pFragment->begin(), pFragment->end() - 1 );
		strRanges += strRange;
	}

	return strRanges;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile clip a range to valid portions

BOOL CDownloadWithFile::ClipUploadRange(QWORD nOffset, QWORD& nLength) const
{
	if ( m_pFile == NULL || !m_pFile->IsValid() )
		return FALSE;

	if ( nOffset >= m_nSize ) return FALSE;

	if ( m_pFile->IsPositionRemaining( nOffset ) ) return FALSE;

	if ( nOffset + nLength > m_nSize ) nLength = m_nSize - nOffset;

	Fragments::List::const_iterator_pair match( GetEmptyFragmentList().equal_range(
		Fragments::Fragment( nOffset, nOffset + nLength ) ) );

	if ( match.first != match.second )
	{
		if ( match.first->begin() <= nOffset ) return ( nLength = 0 ) > 0;
		nLength = match.first->end() - nOffset;
		return TRUE;
	}

	return nLength > 0;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile select a random range of available data

BOOL CDownloadWithFile::GetRandomRange(QWORD& nOffset, QWORD& nLength) const
{
	if ( m_pFile == NULL || !m_pFile->IsValid() )
		return FALSE;

	if ( m_pFile->GetCompleted() == 0 ) return FALSE;

	Fragments::List oFilled = inverse( GetEmptyFragmentList() );
	Fragments::List::const_iterator pRandom = oFilled.random_range();

	nOffset = pRandom->begin();
	nLength = pRandom->size();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile submit data

BOOL CDownloadWithFile::SubmitData(QWORD nOffset, LPBYTE pData, QWORD nLength)
{
	SetModified();
	m_tReceived = GetTickCount();

	if ( static_cast< CDownloadWithTorrent* >( this )->IsTorrent() )	// Hack: Only do this for BitTorrent
	{
		for ( CDownloadTransfer* pTransfer = GetFirstTransfer() ; pTransfer ; pTransfer = pTransfer->m_pDlNext )
		{
			if ( pTransfer->m_nProtocol == PROTOCOL_BT ) pTransfer->UnrequestRange( nOffset, nLength );
		}
	}

	return m_pFile != NULL && m_pFile->WriteRange( nOffset, pData, nLength );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile erase a range

QWORD CDownloadWithFile::EraseRange(QWORD nOffset, QWORD nLength)
{
	if ( m_pFile == NULL ) return 0;
	QWORD nCount = m_pFile->InvalidateRange( nOffset, nLength );
	if ( nCount > 0 ) SetModified();
	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile make the file appear complete

BOOL CDownloadWithFile::MakeComplete()
{
	return PrepareFile() && m_pFile->MakeComplete();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile run the file

BOOL CDownloadWithFile::RunFile(DWORD /*tNow*/)
{
	if ( m_pFile->IsOpen() )
	{
		if ( m_pFile->GetRemaining() == 0 ) return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile append intrinsic metadata

BOOL CDownloadWithFile::AppendMetadata()
{
	if ( !Settings.Library.VirtualFiles )
		return FALSE;

	if ( GetMetadata() == NULL ) return FALSE;
	CXMLElement* pXML = GetMetadata()->GetFirstElement();
	if ( pXML == NULL ) return FALSE;

	HANDLE hFile = CreateFile( m_sPath, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	VERIFY_FILE_ACCESS( hFile, m_sPath )
	if ( hFile == INVALID_HANDLE_VALUE ) return FALSE;

	CString strURI = GetMetadata()->GetAttributeValue( CXMLAttribute::schemaName );
	BOOL bSuccess = FALSE;

	if ( CheckURI( strURI, CSchema::uriAudio ) )
	{
		if ( _tcsistr( m_sPath, _T(".mp3") ) != NULL )
		{
			bSuccess |= AppendMetadataID3v1( hFile, pXML );
		}
	}

	CloseHandle( hFile );

	return bSuccess;
}

BOOL CDownloadWithFile::AppendMetadataID3v1(HANDLE hFile, CXMLElement* pXML)
{
	DWORD nBytes;
	CString str;

	ID3V1 pID3 = {};
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );

	if ( !ReadFile( hFile, &pID3, 3, &nBytes, NULL ) )
		return FALSE;

	if ( memcmp( pID3.szTag, ID3V2_TAG, 3 ) == 0 )
		return FALSE;

	ZeroMemory( &pID3, sizeof(pID3) );
	SetFilePointer( hFile, -(int)sizeof(pID3), NULL, FILE_END );

	if ( !ReadFile( hFile, &pID3, sizeof(pID3), &nBytes, NULL ) )
		return FALSE;

	if ( memcmp( pID3.szTag, ID3V1_TAG, 3 ) == 0 )
		return FALSE;

	ZeroMemory( &pID3, sizeof(pID3) );
	std::memcpy( pID3.szTag, ID3V1_TAG, 3 );

	str = pXML->GetAttributeValue( _T("title") );
	if ( str.GetLength() )
		strncpy( pID3.szSongname, CT2CA( str ), 30 );

	str = pXML->GetAttributeValue( _T("artist") );
	if ( str.GetLength() )
		strncpy( pID3.szArtist, CT2CA( str ), 30 );

	str = pXML->GetAttributeValue( _T("album") );
	if ( str.GetLength() )
		strncpy( pID3.szAlbum, CT2CA( str ), 30 );

	str = pXML->GetAttributeValue( _T("year") );
	if ( str.GetLength() )
		strncpy( pID3.szYear, CT2CA( str ), 4 );

	str = pXML->GetAttributeValue( _T("genre") );
	if ( str.GetLength() )
	{
		int nGenre = LibraryBuilder.LookupID3v1Genre( str );
		if ( nGenre != -1 )
			pID3.nGenre = static_cast< BYTE >( nGenre );
	}

	SetFilePointer( hFile, 0, NULL, FILE_END );
	WriteFile( hFile, &pID3, sizeof(pID3), &nBytes, NULL );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile serialize

void CDownloadWithFile::Serialize(CArchive& ar, int nVersion)
{
	CDownloadWithTransfers::Serialize( ar, nVersion );

	if ( ar.IsStoring() )
	{
		ar.WriteCount( m_pFile != NULL );
		if ( m_pFile != NULL ) m_pFile->Serialize( ar, nVersion );
	}
	else
	{
		if ( nVersion < 28 )
		{
			CString strLocalName;
			ar >> strLocalName;

			if ( strLocalName.GetLength() )
			{
				if ( m_sPath.GetLength() )
					MoveFile( m_sPath + _T(".sd"), strLocalName + _T(".sd") );
				m_sPath = strLocalName;
			}
			else
			{
				GenerateDiskName();
			}
		}

		if ( nVersion < 25 || ar.ReadCount() )
		{
			m_pFile->Serialize( ar, nVersion );
		}
		else
		{
			delete m_pFile;
			m_pFile = NULL;
		}
	}
}
