//
// DownloadWithFile.cpp
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
#include "Downloads.h"
#include "DownloadWithFile.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadGroups.h"
#include "FragmentedFile.h"
#include "Uploads.h"

#include "ID3.h"
#include "SHA.h"
#include "XML.h"
#include "Schema.h"
#include "LibraryBuilderInternals.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadWithFile construction

CDownloadWithFile::CDownloadWithFile()
{
	m_pFile		= new CFragmentedFile();
	m_tReceived	= GetTickCount();
	m_bDiskFull	= FALSE;
}

CDownloadWithFile::~CDownloadWithFile()
{
	if ( m_pFile != NULL ) delete m_pFile;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile open the file

BOOL CDownloadWithFile::OpenFile()
{
	if ( m_pFile == NULL || m_sRemoteName.IsEmpty() || m_nSize == SIZE_UNKNOWN ) return FALSE;
	if ( m_pFile->IsOpen() ) return TRUE;
	
	SetModified();
	
	if ( m_pFile->IsValid() )
	{
		if ( m_pFile->Open( m_sLocalName ) ) return TRUE;
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_FILE_OPEN_ERROR, (LPCTSTR)m_sLocalName );
	}
	else if ( ! Downloads.IsSpaceAvailable( m_nSize, Downloads.dlPathIncomplete ) )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_DISK_SPACE,
			(LPCTSTR)m_sRemoteName,
			(LPCTSTR)Settings.SmartVolume( m_nSize, FALSE ) );
	}
	else
	{
		CString strLocalName = m_sLocalName;
		m_sLocalName.Empty();
		
		GenerateLocalName();
		
		for ( int nTry = 0 ; nTry < 5 ; nTry++ )
		{
			CString strName;

			if ( nTry == 0 )
				strName = m_sLocalName;
			else
				strName.Format( _T("%s.x%i"), (LPCTSTR)m_sLocalName, rand() % 128 );
			
            theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_FILE_CREATE, (LPCTSTR)strName );
			
			if ( m_pFile->Create( strName, m_nSize ) )
			{
				theApp.WriteProfileString( _T("Delete"), strName, NULL );
				MoveFile( strLocalName + _T(".sd"), strName + _T(".sd") );
				m_sLocalName = strName;
				return TRUE;
			}
			
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_FILE_CREATE_ERROR, (LPCTSTR)strName );
		}
		
		m_sLocalName = strLocalName;
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

void CDownloadWithFile::DeleteFile(BOOL bForce)
{
	if ( m_pFile != NULL && m_pFile->IsValid() == FALSE ) return;
	
	Uploads.OnRename( m_sLocalName, NULL );
	
	int nPos = m_sLocalName.ReverseFind( '\\' );
	CString strMetadata;
	
	if ( nPos > 0 )
	{
		strMetadata = m_sLocalName.Left( nPos ) + _T("\\Metadata") + m_sLocalName.Mid( nPos ) + _T(".xml");
	}
	
	if ( m_pFile != NULL )
	{
		if ( GetVolumeComplete() == 0 || ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 )
		{
			if ( ! ::DeleteFile( m_sLocalName ) )
				theApp.WriteProfileString( _T("Delete"), m_sLocalName, _T("") );
			if ( strMetadata.GetLength() ) ::DeleteFile( strMetadata );
		}
		else
		{
			MoveFile( m_sLocalName, m_sLocalName + _T(".aborted") );
		}
	}
	else if ( bForce )
	{
		if ( ! ::DeleteFile( m_sLocalName ) )
			theApp.WriteProfileString( _T("Delete"), m_sLocalName, _T("") );
		if ( strMetadata.GetLength() ) ::DeleteFile( strMetadata );
	}
	
	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile statistics

float CDownloadWithFile::GetProgress() const
{
	if ( m_nSize == 0 || m_nSize == SIZE_UNKNOWN ) return 0;
	return (float)GetVolumeComplete() / (float)m_nSize;
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
	else
	{
		return m_nSize;
	}
}

QWORD CDownloadWithFile::GetVolumeRemaining() const
{
	if ( m_pFile != NULL )
	{
		if ( m_pFile->IsValid() )
			return m_pFile->GetRemaining();
		else if ( m_nSize != SIZE_UNKNOWN )
			return m_nSize;
	}
	
	return 0;
}

DWORD CDownloadWithFile::GetTimeRemaining() const
{
	QWORD nRemaining	= GetVolumeRemaining();
	DWORD nSpeed		= GetAverageSpeed();
	if ( nSpeed == 0 ) return 0xFFFFFFFF;
	return (DWORD)( nRemaining / nSpeed );
}

CString CDownloadWithFile::GetDisplayName() const
{
	if ( m_sRemoteName.GetLength() ) return m_sRemoteName;
	
	CString strName;
	
	if ( m_bSHA1 )
		strName = _T("sha1:") + CSHA::HashToString( &m_pSHA1 );
	else
		strName = _T("Unknown File");
	
	return strName;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile get the first empty fragment

const FF::SimpleFragmentList& CDownloadWithFile::GetEmptyFragmentList() const
{
    static const FF::SimpleFragmentList dummy( 0 );
    return m_pFile ? m_pFile->GetEmptyFragmentList() : dummy;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile get a list of possible download fragments

FF::SimpleFragmentList CDownloadWithFile::GetPossibleFragments(
    const FF::SimpleFragmentList& oAvailable, FF::SimpleFragment& oLargest)
{
	if ( !PrepareFile() ) return FF::SimpleFragmentList( oAvailable.limit() );
    FF::SimpleFragmentList oPossible( oAvailable );

    if( oAvailable.empty() )
    {
        oPossible = m_pFile->GetEmptyFragmentList();
    }
    else
    {
        // ToDo: add a function to FF::detail::List<...> to do that more efficiently
        FF::SimpleFragmentList tmp( inverse( m_pFile->GetEmptyFragmentList() ) );
        oPossible.erase( tmp.begin(), tmp.end() );
    }
	
    if ( oPossible.empty() ) return oPossible;

    oLargest = *largestFragment( oPossible );

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
    if( nSize <= 268435456 ) return 1024 * 1024; // try to keep chunk size reasonably large
    DWORD nChunk = DWORD( ( nSize - 1 ) / 256 ), nTemp; // default treeheight of 9
    while( nTemp = nChunk & ( nChunk - 1 ) ) nChunk = nTemp;
    return nChunk * 2;
}

BOOL CDownloadWithFile::GetFragment(CDownloadTransfer* pTransfer)
{
	if ( ! PrepareFile() ) return NULL;
	
    FF::SimpleFragment oLargest( SIZE_UNKNOWN, SIZE_UNKNOWN );

    FF::SimpleFragmentList oPossible = GetPossibleFragments(
        pTransfer->m_pSource->m_oAvailable, oLargest );
	
	if ( oLargest.begin() == SIZE_UNKNOWN )
	{
		ASSERT( oPossible.empty() );
		return FALSE;
	}
	
	if ( !oPossible.empty() )
	{
        // This is not optimal,
        // we assume the remote host only advertises aligned verified blocks
        FF::SimpleFragment oSelection( selectBlock( oPossible,
            CalcChunkSize( m_nSize ), static_cast< unsigned char* >( NULL ) ) );

		pTransfer->m_nOffset = oSelection.begin();
		pTransfer->m_nLength = oSelection.length();
		
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
			pTransfer->m_nLength = oLargest.length();
			return TRUE;
		}
		
		if ( oLargest.length() < 32 ) return FALSE;
		
		DWORD nOldSpeed	= pExisting->GetAverageSpeed();
		DWORD nNewSpeed	= pTransfer->GetAverageSpeed();
		QWORD nLength	= oLargest.length() / 2;
		
		if ( nOldSpeed > 5 && nNewSpeed > 5 )
		{
			nLength = (QWORD)( (double)oLargest.length() * nNewSpeed / ( nNewSpeed + nOldSpeed ) );
			nLength = min( nLength, oLargest.length() );
			
			if ( oLargest.length() > 102400 )
			{
				nLength = max( nLength, 51200ULL );
				nLength = min( nLength, oLargest.length() - 51200ULL );
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
	if ( m_pFile == NULL || ! m_pFile->IsValid() ) return FALSE;
	return m_pFile->IsPositionRemaining( nOffset );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile check if a range would "help"

BOOL CDownloadWithFile::AreRangesUseful(const FF::SimpleFragmentList& oAvailable)
{
	if ( m_pFile == NULL || ! m_pFile->IsValid() ) return FALSE;
    return overlaps( m_pFile->GetEmptyFragmentList(), oAvailable );
}

BOOL CDownloadWithFile::IsRangeUseful(QWORD nOffset, QWORD nLength)
{
	if ( m_pFile == NULL || ! m_pFile->IsValid() ) return FALSE;
    return overlaps( m_pFile->GetEmptyFragmentList(),
        FF::SimpleFragment( nOffset, nOffset + nLength ) );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile get a string of available ranges

CString CDownloadWithFile::GetAvailableRanges() const
{
	CString strRange, strRanges;
	
	if ( m_pFile == NULL || ! m_pFile->IsValid() ) return strRanges;
	
    const FF::SimpleFragmentList oAvailable = inverse( m_pFile->GetEmptyFragmentList() );

    for( FF::SimpleFragmentList::ConstIterator pFragment = oAvailable.begin();
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
	if ( m_pFile == NULL || ! m_pFile->IsValid() ) return FALSE;
	if ( nOffset >= m_nSize ) return FALSE;
	
	if ( m_pFile->IsPositionRemaining( nOffset ) ) return FALSE;
	
    if ( nOffset + nLength > m_nSize ) nLength = m_nSize - nOffset;

    FF::SimpleFragmentList::ConstIteratorPair match
        = m_pFile->GetEmptyFragmentList().overlappingRange(
            FF::SimpleFragment( nOffset, nOffset + nLength ) );

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
	if ( m_pFile == NULL || ! m_pFile->IsValid() ) return FALSE;
	
    if ( m_pFile->GetEmptyFragmentList().missing() == 0 ) return FALSE;

    FF::SimpleFragmentList oFilled = inverse( m_pFile->GetEmptyFragmentList() );
    FF::SimpleFragmentList::ConstIterator pRandom = randomFragment( oFilled );

    nOffset = pRandom->begin();
    nLength = pRandom->length();

    return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile submit data

BOOL CDownloadWithFile::SubmitData(QWORD nOffset, LPBYTE pData, QWORD nLength)
{
	SetModified();
	m_tReceived = GetTickCount();
	
	if ( m_bBTH )	// Hack: Only do this for BitTorrent
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
	if ( m_sLocalName.IsEmpty() ) return FALSE;
	if ( ! PrepareFile() ) return FALSE;
	return m_pFile->MakeComplete();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile run the file

BOOL CDownloadWithFile::RunFile(DWORD tNow)
{
	if ( m_pFile->IsOpen() )
	{
		if ( m_pFile->GetRemaining() == 0 ) return TRUE;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile write the metadata

BOOL CDownloadWithFile::WriteMetadata(LPCTSTR pszPath)
{
	ASSERT( m_pXML != NULL );
	
	CString strXML = m_pXML->ToString( TRUE, TRUE );
	delete m_pXML;
	m_pXML = NULL;
	
	CString strMetadata;
	
	strMetadata.Format( _T("%s\\Metadata"), pszPath );
	CreateDirectory( strMetadata, NULL );
	SetFileAttributes( strMetadata, FILE_ATTRIBUTE_HIDDEN );
	
	strMetadata += m_sLocalName.Mid( m_sLocalName.ReverseFind( '\\' ) );
	strMetadata += _T(".xml");
	
	HANDLE hFile = CreateFile( strMetadata, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
								FILE_ATTRIBUTE_NORMAL, NULL );
	
	if ( hFile == INVALID_HANDLE_VALUE ) return FALSE;
	
	DWORD nWritten;
	
#ifdef _UNICODE
	int nASCII = WideCharToMultiByte( CP_UTF8, 0, strXML, strXML.GetLength(), NULL, 0, NULL, NULL );
	LPSTR pszASCII = new CHAR[ nASCII ];
	WideCharToMultiByte( CP_UTF8, 0, strXML, strXML.GetLength(), pszASCII, nASCII, NULL, NULL );
	WriteFile( hFile, pszASCII, nASCII, &nWritten, NULL );
	delete [] pszASCII;
#else
	WriteFile( hFile, (LPCSTR)strXML, strXML.GetLength(), &nWritten, NULL );
#endif
	
	CloseHandle( hFile );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithFile append intrinsic metadata

BOOL CDownloadWithFile::AppendMetadata()
{
	if ( ! Settings.Library.VirtualFiles ) return FALSE;
	
	if ( m_pXML == NULL ) return FALSE;
	CXMLElement* pXML = m_pXML->GetFirstElement();
	if ( pXML == NULL ) return FALSE;
	
	HANDLE hFile = CreateFile( m_sLocalName, GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile == INVALID_HANDLE_VALUE ) return FALSE;
	
	CString strURI = m_pXML->GetAttributeValue( CXMLAttribute::schemaName );
	BOOL bSuccess = FALSE;
	
	if ( strURI == CSchema::uriAudio )
	{
		if ( _tcsistr( m_sLocalName, _T(".mp3") ) != NULL )
		{
			bSuccess |= AppendMetadataID3v1( hFile, pXML );
		}
	}
	
	CloseHandle( hFile );
	
	return bSuccess;
}

BOOL CDownloadWithFile::AppendMetadataID3v1(HANDLE hFile, CXMLElement* pXML)
{
	USES_CONVERSION;
	DWORD nBytes;
	CString str;
	ID3V1 pID3;
	
	ZeroMemory( &pID3, sizeof(pID3) );
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, &pID3, 3, &nBytes, NULL );
	if ( memcmp( pID3.szTag, ID3V2_TAG, 3 ) == 0 ) return FALSE;
	
	ZeroMemory( &pID3, sizeof(pID3) );
	SetFilePointer( hFile, -(int)sizeof(pID3), NULL, FILE_END );
	ReadFile( hFile, &pID3, sizeof(pID3), &nBytes, NULL );
	if ( memcmp( pID3.szTag, ID3V1_TAG, 3 ) == 0 ) return FALSE;
	
	ZeroMemory( &pID3, sizeof(pID3) );
	memcpy( pID3.szTag, ID3V1_TAG, 3 );
	
	str = pXML->GetAttributeValue( _T("title") );
	if ( str.GetLength() > 0 ) strncpy( pID3.szSongname, T2CA( (LPCTSTR)str ), 30 );
	str = pXML->GetAttributeValue( _T("artist") );
	if ( str.GetLength() > 0 ) strncpy( pID3.szArtist, T2CA( (LPCTSTR)str ), 30 );
	str = pXML->GetAttributeValue( _T("album") );
	if ( str.GetLength() > 0 ) strncpy( pID3.szAlbum, T2CA( (LPCTSTR)str ), 30 );
	str = pXML->GetAttributeValue( _T("year") );
	if ( str.GetLength() > 0 ) strncpy( pID3.szYear, T2CA( (LPCTSTR)str ), 4 );
	
	str = pXML->GetAttributeValue( _T("genre") );
	
	for ( int nGenre = 0 ; nGenre < ID3_GENRES ; nGenre ++ )
	{
		if ( str.CompareNoCase( CLibraryBuilderInternals::pszID3Genre[ nGenre ] ) == 0 )
		{
			pID3.nGenre = nGenre;
			break;
		}
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
				if ( m_sLocalName.GetLength() )
					MoveFile( m_sLocalName + _T(".sd"), strLocalName + _T(".sd") );
				m_sLocalName = strLocalName;
			}
			else
			{
				GenerateLocalName();
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
