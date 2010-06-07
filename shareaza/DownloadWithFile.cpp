//
// DownloadWithFile.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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
#include "DownloadSource.h"
#include "DownloadWithFile.h"
#include "Downloads.h"
#include "Library.h"
#include "LibraryBuilder.h"
#include "LibraryHistory.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadWithFile construction

CDownloadWithFile::CDownloadWithFile() :
	m_bVerify		( TRI_UNKNOWN )
,	m_tReceived		( GetTickCount() )
,	m_pFile			( new CFragmentedFile )
,	m_nFileError	( ERROR_SUCCESS )
{
	m_pFile->SetDownload( static_cast< CDownload*>( this ) );
}

CDownloadWithFile::~CDownloadWithFile()
{
}

BOOL CDownloadWithFile::IsValid() const
{
	return m_pFile.get() && m_pFile->IsValid();
}

BOOL CDownloadWithFile::IsFileOpen() const
{
	return m_pFile.get() && m_pFile->IsOpen();
}

Fragments::List CDownloadWithFile::GetFullFragmentList() const
{
	return m_pFile.get() ? m_pFile->GetFullFragmentList() : Fragments::List( 0 );
}

Fragments::List CDownloadWithFile::GetEmptyFragmentList() const
{
	return m_pFile.get() ? m_pFile->GetEmptyFragmentList() : Fragments::List( 0 );
}

CFragmentedFile* CDownloadWithFile::GetFile()
{
	m_pFile->AddRef();
	return m_pFile.get();
}

BOOL CDownloadWithFile::FindByPath(const CString& sPath) const
{
	return m_pFile.get() && m_pFile->FindByPath( sPath );
}

// Get amount of subfiles
DWORD CDownloadWithFile::GetFileCount() const
{
	return m_pFile.get() ? m_pFile->GetCount() : 0;
}

// Get subfile offset
QWORD CDownloadWithFile::GetOffset(DWORD nIndex) const
{
	return m_pFile.get() ? m_pFile->GetOffset( nIndex ) : 0;
}

// Get subfile length
QWORD CDownloadWithFile::GetLength(DWORD nIndex) const
{
	return m_pFile.get() ? m_pFile->GetLength( nIndex ) : SIZE_UNKNOWN;
}

// Get path of subfile
CString CDownloadWithFile::GetPath(DWORD nIndex) const
{
	return m_pFile.get() ? m_pFile->GetPath( nIndex ) : CString();
}

// Get original name of subfile
CString CDownloadWithFile::GetName(DWORD nIndex) const
{
	return m_pFile.get() ? m_pFile->GetName( nIndex ) : CString();
}

// Get completed size of subfile (in bytes)
QWORD CDownloadWithFile::GetCompleted(DWORD nIndex) const
{
	return m_pFile.get() ? m_pFile->GetCompleted( nIndex ) : 0;
}

// Select subfile (with user interaction)
int CDownloadWithFile::SelectFile(CSingleLock* pLock) const
{
	return m_pFile.get() ? m_pFile->SelectFile( pLock ) : -1;
}

// Get last file/disk operation error
DWORD CDownloadWithFile::GetFileError() const
{
	return m_nFileError;
}

// Set file/disk error status
void CDownloadWithFile::SetFileError(DWORD nFileError)
{
	m_nFileError = nFileError;
}

// Clear file/disk error status
void CDownloadWithFile::ClearFileError()
{
	m_nFileError = ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile open the file

BOOL CDownloadWithFile::OpenFile()
{
	if ( m_sName.IsEmpty() )
		return TRUE;

	if ( IsFileOpen() )
		return TRUE;

	SetModified();

	CDownload* pThis = static_cast< CDownload* >( this );	// TODO: Fix bad inheritance
	if ( m_pFile.get() )
	{
		ClearFileError();

		if ( pThis->IsTorrent() )
		{
			CString sFoo;
			if ( m_pFile->Open( pThis->m_pTorrent, ! IsCompleted(), sFoo ) )
				return TRUE;
		}
		else
		{
			// TODO: Refactor m_sTorrentTrackerError
			pThis->m_sTorrentTrackerError.Empty();

			if ( m_pFile->Open( *this, ! IsCompleted() ) )
				return TRUE;
		}

		m_nFileError = m_pFile->GetFileError();
	}
	else if ( m_nSize != SIZE_UNKNOWN &&
		! Downloads.IsSpaceAvailable( m_nSize, Downloads.dlPathIncomplete ) )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_DISK_SPACE,
			m_sName, Settings.SmartVolume( m_nSize ) );

		m_nFileError = ERROR_DISK_FULL;
	}

	// TODO: Refactor m_sTorrentTrackerError
	if ( m_nFileError != ERROR_SUCCESS )
		pThis->m_sTorrentTrackerError = GetErrorString( m_nFileError );

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile close the file

void CDownloadWithFile::CloseFile()
{
	if ( m_pFile.get() )
		m_pFile->Close();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile clear the file

void CDownloadWithFile::ClearFile()
{
	if ( m_pFile.get() )
		m_pFile->Clear();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile prepare file

BOOL CDownloadWithFile::PrepareFile()
{
	return OpenFile() && m_pFile->GetRemaining() > 0;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile attach the file

void CDownloadWithFile::AttachFile(auto_ptr< CFragmentedFile >& pFile)
{
	m_pFile = pFile;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile delete the file

void CDownloadWithFile::DeleteFile()
{
	ASSERT( ! IsTasking() );

	if ( m_pFile.get() )
	{
		m_pFile->Delete();
		m_pFile.reset();
	}

	SetModified();
}

// Move file(s) to destination. Returns 0 on success or file error number.
DWORD CDownloadWithFile::MoveFile(LPCTSTR pszDestination, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData)
{
	ASSERT( IsMoving() );

	if ( ! m_pFile.get() )
		return ERROR_FILE_NOT_FOUND;

	const DWORD nCount = m_pFile->GetCount();
	for( DWORD nIndex = 0; nIndex < nCount; ++nIndex )
	{
		DWORD dwError = m_pFile->Move( nIndex, pszDestination, lpProgressRoutine, lpData );

		if ( dwError != ERROR_SUCCESS )
		{
			CString strMessage;
			strMessage.Format( LoadString( IDS_DOWNLOAD_CANT_MOVE ),
				GetDisplayName(), pszDestination );
			theApp.Message( MSG_ERROR | MSG_TRAY, _T("%s %s"),
				strMessage, GetErrorString( dwError ) );
			return dwError;
		}

		// Save download every move
		static_cast< CDownload* >( this )->Save();

		const CString sPath = m_pFile->GetPath( nIndex );

		MarkFileAsDownload( sPath );

		LibraryBuilder.RequestPriority( sPath );

		// TODO: Get hashes for all files of download
		if ( nCount == 1 )
		{
			// Update with download hashes single-file download only
			Hashes::Sha1ManagedHash		oSHA1( m_oSHA1 );
			if ( m_bSHA1Trusted )		oSHA1.signalTrusted();
			Hashes::TigerManagedHash	oTiger( m_oTiger );
			if ( m_bTigerTrusted )		oTiger.signalTrusted();
			Hashes::Ed2kManagedHash		oED2K( m_oED2K );
			if ( m_bED2KTrusted )		oED2K.signalTrusted();
			Hashes::BtManagedHash		oBTH( m_oBTH );
			if ( m_bBTHTrusted )		oBTH.signalTrusted();
			Hashes::Md5ManagedHash		oMD5( m_oMD5 );
			if ( m_bMD5Trusted )		oMD5.signalTrusted();
			LibraryHistory.Add( sPath, oSHA1, oTiger, oED2K, oBTH, oMD5,
				GetSourceURLs( NULL, 0, PROTOCOL_NULL, NULL ) );
		}
		else
		{
			Hashes::Sha1ManagedHash		oSHA1;
			Hashes::TigerManagedHash	oTiger;
			Hashes::Ed2kManagedHash		oED2K;
			Hashes::BtManagedHash		oBTH;
			Hashes::Md5ManagedHash		oMD5;
			LibraryHistory.Add( sPath, oSHA1, oTiger, oED2K, oBTH, oMD5 );
		}

		// Early metadata update
		CSingleLock oLibraryLock( &Library.m_pSection, FALSE );
		if ( oLibraryLock.Lock( 100 ) )
		{
			if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( sPath ) )
				pFile->UpdateMetadata( static_cast< CDownload* >( this ) );
		}
	}

	theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_MOVED,
		(LPCTSTR)GetDisplayName(), (LPCTSTR)pszDestination );

	return ERROR_SUCCESS;
}

BOOL CDownloadWithFile::FlushFile()
{
	return m_pFile.get() && m_pFile->Flush();
}

BOOL CDownloadWithFile::IsComplete() const
{
	return m_pFile.get() && m_pFile->IsComplete();
}

BOOL CDownloadWithFile::ReadFile(QWORD nOffset, LPVOID pData, QWORD nLength, QWORD* pnRead)
{
	return m_pFile.get() && m_pFile->Read( nOffset, pData, nLength, pnRead );
}

BOOL CDownloadWithFile::WriteFile(QWORD nOffset, LPCVOID pData, QWORD nLength, QWORD* pnWritten)
{
	return m_pFile.get() && m_pFile->Write( nOffset, pData, nLength, pnWritten );
}

QWORD CDownloadWithFile::InvalidateFileRange(QWORD nOffset, QWORD nLength)
{
	return m_pFile.get() && m_pFile->InvalidateRange( nOffset, nLength );
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
	if ( m_pFile.get() )
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
	if ( m_pFile.get() )
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
// CDownloadWithFile check if a byte position is empty

BOOL CDownloadWithFile::IsPositionEmpty(QWORD nOffset)
{
	return m_pFile.get() && m_pFile->IsValid() &&
		m_pFile->IsPositionRemaining( nOffset );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile get a string of available ranges

CString CDownloadWithFile::GetAvailableRanges() const
{
	CString strRange, strRanges;

	if ( ! m_pFile.get() || ! m_pFile->IsValid() )
		return strRanges;

	Fragments::List oAvailable( inverse( GetEmptyFragmentList() ) );
	Fragments::List::const_iterator pItr = oAvailable.begin();
	const Fragments::List::const_iterator pEnd = oAvailable.end();
	for ( ; pItr != pEnd ; ++pItr )
	{
		if ( strRanges.IsEmpty() )
		{
			strRanges = _T("bytes ");
		}
		else
		{
			strRanges += ',';
		}

		strRange.Format( _T("%I64i-%I64i"), pItr->begin(), pItr->end() - 1 );
		strRanges += strRange;

		// Prevent too long line
		if ( strRanges.GetLength() > HTTP_HEADER_MAX_LINE - 256 )
			break;
	}

	return strRanges;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile clip a range to valid portions

BOOL CDownloadWithFile::ClipUploadRange(QWORD nOffset, QWORD& nLength) const
{
	if ( ! m_pFile.get() || ! m_pFile->IsValid() )
		return FALSE;

	if ( nOffset >= m_nSize )
		return FALSE;

	if ( m_pFile->IsPositionRemaining( nOffset ) )
		return FALSE;

	if ( nOffset + nLength > m_nSize )
		nLength = m_nSize - nOffset;

	Fragments::Fragment oMatch( nOffset, nOffset + nLength );
	Fragments::List oList( GetEmptyFragmentList() );
	Fragments::List::const_iterator_pair pMatches = oList.equal_range( oMatch );

	if ( pMatches.first != pMatches.second )
	{
		if ( pMatches.first->begin() <= nOffset )
			nLength = 0;
		else
			nLength = pMatches.first->end() - nOffset;
	}

	return nLength > 0;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile select a random range of available data

BOOL CDownloadWithFile::GetRandomRange(QWORD& nOffset, QWORD& nLength) const
{
	if ( ! m_pFile.get() || ! m_pFile->IsValid() )
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

	if ( static_cast< CDownload* >( this )->IsTorrent() )	// HACK: Only do this for BitTorrent
															// TODO: Fix bad inheritance
	{
		for ( CDownloadTransfer* pTransfer = GetFirstTransfer() ; pTransfer ; pTransfer = pTransfer->m_pDlNext )
		{
			if ( pTransfer->m_nProtocol == PROTOCOL_BT )
				pTransfer->UnrequestRange( nOffset, nLength );
		}
	}

	return ( m_pFile.get() && m_pFile->Write( nOffset, pData, nLength ) );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile erase a range

QWORD CDownloadWithFile::EraseRange(QWORD nOffset, QWORD nLength)
{
	if ( ! m_pFile.get() ) return 0;
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
// CDownloadWithFile append intrinsic metadata

/*BOOL CDownloadWithFile::AppendMetadata()
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

	if ( !::ReadFile( hFile, &pID3, 3, &nBytes, NULL ) )
		return FALSE;

	if ( memcmp( pID3.szTag, ID3V2_TAG, 3 ) == 0 )
		return FALSE;

	ZeroMemory( &pID3, sizeof(pID3) );
	SetFilePointer( hFile, -(int)sizeof(pID3), NULL, FILE_END );

	if ( !::ReadFile( hFile, &pID3, sizeof(pID3), &nBytes, NULL ) )
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
	::WriteFile( hFile, &pID3, sizeof(pID3), &nBytes, NULL );

	return TRUE;
}*/

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile serialize

void CDownloadWithFile::Serialize(CArchive& ar, int nVersion)
{
	CDownloadWithTransfers::Serialize( ar, nVersion );

	if ( ar.IsStoring() )
	{
		ar.WriteCount( m_pFile.get() != NULL );

		// Restore original filename added in nVersion == 41
		{
			DWORD nIndex = 0;
			if ( static_cast< CDownload* >( this )->IsTorrent() )
			{
				CBTInfo& oInfo = static_cast< CDownload* >( this )->m_pTorrent;
				for ( POSITION pos = oInfo.m_pFiles.GetHeadPosition() ; pos ; ++nIndex )
				{
					CBTInfo::CBTFile* pBTFile = oInfo.m_pFiles.GetNext( pos );
					if ( m_pFile->GetName( nIndex ).IsEmpty() )
						m_pFile->SetName( nIndex, pBTFile->m_sPath );
				}
			}
			else
				if ( m_pFile->GetName( nIndex ).IsEmpty() )
					m_pFile->SetName( nIndex, m_sName );
		}

		SerializeFile( ar, nVersion );
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
					::MoveFile( m_sPath, strLocalName + _T(".sd") );
				m_sPath = strLocalName + _T(".sd");
			}
		}

		if ( nVersion < 25 || ar.ReadCount() )
		{
			SerializeFile( ar, nVersion );
		}
		else
		{
			CloseFile();
		}
	}
}

void CDownloadWithFile::SerializeFile(CArchive& ar, int nVersion)
{
	if ( m_pFile.get() )
		m_pFile->Serialize( ar, nVersion );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile verification handler

BOOL CDownloadWithFile::OnVerify(LPCTSTR pszPath, BOOL bVerified)
{
	if ( ! m_pFile.get() || ! m_pFile->FindByPath( pszPath ) )
		return FALSE;

	m_bVerify = bVerified ? TRI_TRUE : TRI_FALSE;

	SetModified();

	return TRUE;
}
