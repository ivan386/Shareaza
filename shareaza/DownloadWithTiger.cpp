//
// DownloadWithTiger.cpp
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
#include "Download.h"
#include "Downloads.h"
#include "DownloadWithTiger.h"
#include "FragmentedFile.h"
#include "HashDatabase.h"

#include "Neighbours.h"
#include "Transfers.h"
#include "Library.h"
#include "SharedFile.h"
#include "BTInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger construction

CDownloadWithTiger::CDownloadWithTiger() :
	m_pTigerBlock		( NULL )
,	m_nTigerBlock		( 0ul )
,	m_nTigerSuccess		( 0ul )

,	m_pHashsetBlock		( NULL )
,	m_nHashsetBlock		( 0ul )
,	m_nHashsetSuccess	( 0ul )

,	m_nVerifyCookie		( 0ul )
,	m_nVerifyHash		( HASH_NULL )
,	m_nVerifyBlock		( ~0ul )
{
}

CDownloadWithTiger::~CDownloadWithTiger()
{
	if ( m_pHashsetBlock != NULL ) delete [] m_pHashsetBlock;
	if ( m_pTigerBlock != NULL ) delete [] m_pTigerBlock;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger informational access

DWORD CDownloadWithTiger::GetValidationCookie() const
{
	CQuickLock oLock( m_pTigerSection );

	return m_nVerifyCookie;
}

QWORD CDownloadWithTiger::GetVerifyLength(int nHash) const
{
	CQuickLock oLock( m_pTigerSection );

	if ( nHash == HASH_NULL )
	{
		if ( m_pTorrentBlock != NULL ) return m_nTorrentSize;
		else if ( m_pTigerBlock != NULL ) return m_nTigerSize;
		else if ( m_pHashsetBlock != NULL ) return ED2K_PART_SIZE;
	}
	else if ( nHash == HASH_TIGERTREE && m_pTigerBlock != NULL )
	{
		return m_nTigerSize;
	}
	else if ( nHash == HASH_ED2K && m_pHashsetBlock != NULL )
	{
		return ED2K_PART_SIZE;
	}
	else if ( nHash == HASH_TORRENT && m_pTorrentBlock != NULL )
	{
		return m_nTorrentSize;
	}

	return 0;
}

BOOL CDownloadWithTiger::GetNextVerifyRange(QWORD& nOffset, QWORD& nLength, BOOL& bSuccess, int nHash) const
{
	CQuickLock oLock( m_pTigerSection );

	if ( nOffset >= m_nSize ) return FALSE;
	if ( m_pTigerBlock == NULL && m_pHashsetBlock == NULL && m_pTorrentBlock == NULL ) return FALSE;

	if ( nHash == HASH_NULL )
	{
		if ( m_pTorrentBlock != NULL ) nHash = HASH_TORRENT;
		else if ( m_pTigerBlock != NULL ) nHash = HASH_TIGERTREE;
		else if ( m_pHashsetBlock != NULL ) nHash = HASH_ED2K;
	}

	QWORD nBlockCount, nBlockSize;
	BYTE* pBlockPtr;

	switch ( nHash )
	{
	case HASH_TIGERTREE:
		if ( m_pTigerBlock == NULL ) return FALSE;
		pBlockPtr	= m_pTigerBlock;
		nBlockCount	= m_nTigerBlock;
		nBlockSize	= m_nTigerSize;
		break;
	case HASH_ED2K:
		if ( m_pHashsetBlock == NULL ) return FALSE;
		pBlockPtr	= m_pHashsetBlock;
		nBlockCount	= m_nHashsetBlock;
		nBlockSize	= ED2K_PART_SIZE;
		break;
	case HASH_TORRENT:
		if ( m_pTorrentBlock == NULL ) return FALSE;
		pBlockPtr	= m_pTorrentBlock;
		nBlockCount	= m_nTorrentBlock;
		nBlockSize	= m_nTorrentSize;
		break;
	default:
		return FALSE;
	}

	if ( nBlockSize == 0 ) return FALSE;

	for ( QWORD nBlock = nOffset / nBlockSize ; nBlock < nBlockCount ; nBlock++ )
	{
		QWORD nThis = nBlock * nBlockSize;

		if ( nThis >= nOffset && pBlockPtr[ nBlock ] )
		{
			TRISTATE nBase	= static_cast< TRISTATE >( pBlockPtr[ nBlock ] );
			bSuccess		= nBase == TRI_TRUE;
			nOffset			= nThis;
			nLength			= 0;

			for ( ; nBlock < nBlockCount ; nBlock++ )
			{
				if ( nBase != pBlockPtr[ nBlock ] ) break;
				nLength += nBlockSize;
			}

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CDownloadWithTiger::IsFullyVerified()
{
	CQuickLock oLock( m_pTigerSection );

	if ( m_nTorrentBlock > 0 && m_nTorrentSuccess >= m_nTorrentBlock ) return TRUE;
	if ( m_nTigerBlock > 0 && m_nTigerSuccess >= m_nTigerBlock ) return TRUE;
	if ( m_nHashsetBlock > 0 && m_nHashsetSuccess >= m_nHashsetBlock ) return TRUE;
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger tiger-tree access

BOOL CDownloadWithTiger::NeedTigerTree() const
{
	CQuickLock oLock( m_pTigerSection );

	return ( m_nSize < SIZE_UNKNOWN && m_pTigerTree.IsAvailable() == FALSE );
}

BOOL CDownloadWithTiger::SetTigerTree(BYTE* pTiger, DWORD nTiger)
{
	CQuickLock oLock( m_pTigerSection );

	if ( m_nSize == SIZE_UNKNOWN ) return FALSE;
	if ( m_pTigerTree.IsAvailable() ) return TRUE;

	if ( ! m_pTigerTree.FromBytes( pTiger, nTiger,
			Settings.Library.TigerHeight, m_nSize ) )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_TIGER_CORRUPT,
			(LPCTSTR)GetDisplayName() );
		return FALSE;
	}

	Hashes::TigerHash oRoot;
	m_pTigerTree.GetRoot( &oRoot[ 0 ] );
	oRoot.validate();

	if ( validAndUnequal( m_oTiger, oRoot ) )
	{
		m_pTigerTree.Clear();
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_TIGER_MISMATCH,
			(LPCTSTR)GetDisplayName() );
		return FALSE;
	}
	else if ( ! m_oTiger )
	{
		m_oTiger = oRoot;
	}

	m_nTigerSize	= m_pTigerTree.GetBlockLength();
	m_nTigerBlock	= m_pTigerTree.GetBlockCount();
	m_pTigerBlock	= new BYTE[ m_nTigerBlock ];

	ZeroMemory( m_pTigerBlock, sizeof(BYTE) * m_nTigerBlock );

	SetModified();

	theApp.Message( MSG_INFO, IDS_DOWNLOAD_TIGER_READY,
		GetDisplayName(), m_pTigerTree.GetHeight(),
		Settings.SmartVolume( m_nTigerSize ) );

	return TRUE;
}

CTigerTree* CDownloadWithTiger::GetTigerTree()
{
	CQuickLock oLock( m_pTigerSection );

	return m_pTigerTree.IsAvailable() ? &m_pTigerTree : NULL;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger eDonkey2000 hashset access

BOOL CDownloadWithTiger::NeedHashset() const
{
	CQuickLock oLock( m_pTigerSection );

	return ( m_nSize < SIZE_UNKNOWN && m_pHashset.IsAvailable() == FALSE );
}

BOOL CDownloadWithTiger::SetHashset(BYTE* pSource, DWORD nSource)
{
	CQuickLock oLock( m_pTigerSection );

	if ( m_nSize == SIZE_UNKNOWN ) return FALSE;
	if ( m_pHashset.IsAvailable() ) return TRUE;

	if ( nSource == 0 && m_oED2K )
	{
		m_pHashset.FromRoot( &m_oED2K[ 0 ] );
	}
	else if ( m_pHashset.FromBytes( pSource, nSource, m_nSize ) )
	{
		Hashes::Ed2kHash oRoot;
		m_pHashset.GetRoot( &oRoot[ 0 ] );
		oRoot.validate();

		if ( validAndUnequal( m_oED2K, oRoot ) )
		{
			m_pHashset.Clear();
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_HASHSET_CORRUPT,
				(LPCTSTR)GetDisplayName() );
			return FALSE;
		}
		else if ( ! m_oED2K )
		{
			m_oED2K = oRoot;
		}
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_HASHSET_CORRUPT,
			(LPCTSTR)GetDisplayName() );
		return FALSE;
	}

	m_nHashsetBlock	= m_pHashset.GetBlockCount();
	m_pHashsetBlock	= new BYTE[ m_nHashsetBlock ];

	ZeroMemory( m_pHashsetBlock, sizeof(BYTE) * m_nHashsetBlock );

	SetModified();

	theApp.Message( MSG_INFO, IDS_DOWNLOAD_HASHSET_READY,
		GetDisplayName(), Settings.SmartVolume( ED2K_PART_SIZE ) );

	Neighbours.SendDonkeyDownload( static_cast< CDownload * >( this ) );

	return TRUE;
}

CED2K* CDownloadWithTiger::GetHashset()
{
	CQuickLock oLock( m_pTigerSection );

	return m_pHashset.IsAvailable() ? &m_pHashset : NULL;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger test if the file can finish

BOOL CDownloadWithTiger::ValidationCanFinish() const
{
	CQuickLock oLock( m_pTigerSection );

	BOOL bAvailable = FALSE;

	if ( m_pTorrentBlock != NULL )
	{
		if ( m_nTorrentSuccess >= m_nTorrentBlock ) return TRUE;
		bAvailable = TRUE;
	}

	if ( m_pTigerBlock != NULL && Settings.Downloads.VerifyTiger )
	{
		if ( m_nTigerSuccess >= m_nTigerBlock ) return TRUE;
		bAvailable = TRUE;
	}

	if ( m_pHashsetBlock != NULL && Settings.Downloads.VerifyED2K )
	{
		if ( m_nSize && m_nSize % ED2K_PART_SIZE == 0 &&
			 m_nHashsetSuccess >= m_nHashsetBlock - 1 )
			return TRUE;
		else if ( m_nHashsetSuccess >= m_nHashsetBlock )
			return TRUE;
		bAvailable = TRUE;
	}

	return ! bAvailable;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger run validation

void CDownloadWithTiger::RunValidation()
{
	CQuickLock oLock( m_pTigerSection );

	if ( m_pTigerBlock == NULL && m_pHashsetBlock == NULL && m_pTorrentBlock == NULL )
		return;

	if ( ! OpenFile() )
		return;

	if ( m_nVerifyHash > HASH_NULL && m_nVerifyBlock < 0xFFFFFFFF )
	{
		Downloads.m_nValidation ++;
		ContinueValidation();
	}
	else
	{
		if ( FindNewValidationBlock( HASH_TORRENT ) ||
			 FindNewValidationBlock( HASH_TIGERTREE ) ||
			 FindNewValidationBlock( HASH_ED2K ) )
		{
			Downloads.m_nValidation ++;
			ContinueValidation();
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger validation setup

BOOL CDownloadWithTiger::FindNewValidationBlock(int nHash)
{
	CQuickLock oLock( m_pTigerSection );

	if ( nHash == HASH_TIGERTREE && ! Settings.Downloads.VerifyTiger ) return FALSE;
	if ( nHash == HASH_ED2K && ! Settings.Downloads.VerifyED2K ) return FALSE;

	DWORD nBlockCount;
	QWORD nBlockSize;
	BYTE* pBlockPtr;

	switch ( nHash )
	{
	case HASH_TIGERTREE:
		if ( m_pTigerBlock == NULL ) return FALSE;
		pBlockPtr	= m_pTigerBlock;
		nBlockCount	= m_nTigerBlock;
		nBlockSize	= m_nTigerSize;
		break;
	case HASH_ED2K:
		if ( m_pHashsetBlock == NULL ) return FALSE;
		pBlockPtr	= m_pHashsetBlock;
		nBlockCount	= m_nHashsetBlock;
		nBlockSize	= ED2K_PART_SIZE;
		break;
	case HASH_TORRENT:
		if ( m_pTorrentBlock == NULL ) return FALSE;
		pBlockPtr	= m_pTorrentBlock;
		nBlockCount	= m_nTorrentBlock;
		nBlockSize	= m_nTorrentSize;
		break;
	default:
		return FALSE;
	}

	DWORD nTarget = 0xFFFFFFFF;

	if ( m_pFile == NULL )
	{
		for ( DWORD nBlock = 0 ; nBlock < nBlockCount ; nBlock ++ )
		{
			if ( static_cast< TRISTATE >( pBlockPtr[ nBlock ] ) == TRI_UNKNOWN )
			{
				nTarget = nBlock;
				break;
			}
		}
		if ( nTarget == 0xFFFFFFFF )
		{
			for ( DWORD nBlock = 0 ; nBlock < nBlockCount ; nBlock ++ )
			{
				if ( static_cast< TRISTATE >( pBlockPtr[ nBlock ] ) == TRI_FALSE )
				{
					nTarget = nBlock;
					break;
				}
			}
		}
	}
	else
	{
		DWORD nRetry = 0xFFFFFFFF;
		QWORD nPrevious = 0;

		Fragments::List oList( GetEmptyFragmentList() );
		for ( Fragments::List::const_iterator pFragment = oList.begin();
			pFragment != oList.end(); ++pFragment )
		{
			if ( pFragment->begin() - nPrevious >= nBlockSize )
			{
				DWORD nBlock = (DWORD)( ( nPrevious + nBlockSize - 1 ) / nBlockSize );
				nPrevious = nBlockSize * (QWORD)nBlock + nBlockSize;

				QWORD nFragmentBegin = pFragment->begin();
				for ( ; nPrevious <= nFragmentBegin ; nBlock ++, nPrevious += nBlockSize )
				{
					if ( static_cast< TRISTATE >( pBlockPtr[ nBlock ] ) == TRI_UNKNOWN )
					{
						nTarget = nBlock;
						break;
					}
					else if ( static_cast< TRISTATE >( pBlockPtr[ nBlock ] ) == TRI_FALSE &&
						nRetry == 0xFFFFFFFF )
					{
						nRetry = nBlock;
					}
				}

				if ( nTarget != 0xFFFFFFFF ) break;
			}

			nPrevious = pFragment->end();
		}

		if ( m_nSize > nPrevious && nTarget == 0xFFFFFFFF )
		{
			DWORD nBlock = (DWORD)( ( nPrevious + nBlockSize - 1 ) / nBlockSize );
			nPrevious = nBlockSize * (QWORD)nBlock;

			for ( ; nPrevious < m_nSize ; nBlock ++, nPrevious += nBlockSize )
			{
				if ( static_cast< TRISTATE >( pBlockPtr[ nBlock ] ) == TRI_UNKNOWN )
				{
					nTarget = nBlock;
					break;
				}
				else if ( static_cast< TRISTATE >( pBlockPtr[ nBlock ] ) == TRI_FALSE &&
					nRetry == 0xFFFFFFFF )
				{
					nRetry = nBlock;
				}
			}
		}

		if ( nTarget == 0xFFFFFFFF ) nTarget = nRetry;
	}

	if ( nTarget != 0xFFFFFFFF )
	{
		m_nVerifyHash	= nHash;
		m_nVerifyBlock	= nTarget;
		m_nVerifyOffset	= nTarget * nBlockSize;
		m_nVerifyLength	= min( nBlockSize, m_nSize - m_nVerifyOffset );
		m_tVerifyLast	= GetTickCount();

		if ( m_nVerifyHash == HASH_TIGERTREE )
			m_pTigerTree.BeginBlockTest();
		else if ( m_nVerifyHash == HASH_ED2K )
			m_pHashset.BeginBlockTest();
		else if ( m_nVerifyHash == HASH_TORRENT )
			m_pTorrent.BeginBlockTest();

		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger validation process

void CDownloadWithTiger::ContinueValidation()
{
	CQuickLock oLock( m_pTigerSection );

	ASSERT( m_nVerifyHash > HASH_NULL );
	ASSERT( m_nVerifyBlock < 0xFFFFFFFF );

	BOOL bDone = ( m_pFile == NULL ) || ( m_pFile->GetRemaining() == 0 );
	HANDLE hComplete = INVALID_HANDLE_VALUE;

	if ( m_pFile == NULL )
	{
		hComplete = CreateFile( m_sPath, GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		VERIFY_FILE_ACCESS( hComplete, m_sPath )
		if ( hComplete == INVALID_HANDLE_VALUE ) return;
	}

	auto_array< BYTE > pChunk( new BYTE[ 256 * 1024ull ] );

	for ( int nRound = bDone ? 10 : 2 ; nRound > 0 && m_nVerifyLength > 0 ; nRound-- )
	{
		DWORD nChunk	= (DWORD)min( m_nVerifyLength, 256 * 1024ull );

		if ( m_pFile != NULL )
		{
			if ( ! m_pFile->Read( m_nVerifyOffset, pChunk.get(), nChunk ) )
				break;
		}
		else
		{
			LONG nOffsetHigh = (LONG)( m_nVerifyOffset >> 32 );
			SetFilePointer( hComplete, (DWORD)( m_nVerifyOffset & 0xFFFFFFFF ), &nOffsetHigh, FILE_BEGIN );
			ReadFile( hComplete, pChunk.get(), nChunk, &nChunk, NULL );
		}

		if ( m_nVerifyHash == HASH_TIGERTREE )
			m_pTigerTree.AddToTest( pChunk.get(), (DWORD)nChunk );
		else if ( m_nVerifyHash == HASH_ED2K )
			m_pHashset.AddToTest( pChunk.get(), (DWORD)nChunk );
		else if ( m_nVerifyHash == HASH_TORRENT )
			m_pTorrent.AddToTest( pChunk.get(), (DWORD)nChunk );
		else
			ASSERT( FALSE );

		m_nVerifyOffset += nChunk;
		m_nVerifyLength -= nChunk;
	}

	if ( hComplete != INVALID_HANDLE_VALUE ) CloseHandle( hComplete );
	if ( m_nVerifyLength == 0 ) FinishValidation();
}

void CDownloadWithTiger::FinishValidation()
{
	CQuickLock oLock( m_pTigerSection );

	Fragments::List oCorrupted( m_nSize );

	if ( m_nVerifyHash == HASH_TIGERTREE )
	{
		if ( m_pTigerTree.FinishBlockTest( m_nVerifyBlock ) )
		{
			m_pTigerBlock[ m_nVerifyBlock ] = TRI_TRUE;
			m_nTigerSuccess ++;
		}
		else
		{
			m_pTigerBlock[ m_nVerifyBlock ] = TRI_FALSE;

			QWORD nOffset = QWORD(m_nVerifyBlock) * QWORD(m_nTigerSize);
			oCorrupted.insert( oCorrupted.end(), Fragments::Fragment( nOffset,
				min( nOffset + m_nTigerSize, m_nSize ) ) );
		}
	}
	else if ( m_nVerifyHash == HASH_ED2K )
	{
		if ( m_pHashset.FinishBlockTest( m_nVerifyBlock ) )
		{
			m_pHashsetBlock[ m_nVerifyBlock ] = TRI_TRUE;
			m_nHashsetSuccess ++;
		}
		else
		{
			m_pHashsetBlock[ m_nVerifyBlock ] = TRI_FALSE;

			QWORD nOffset = QWORD(m_nVerifyBlock) * QWORD(ED2K_PART_SIZE);
			oCorrupted.insert( oCorrupted.end(), Fragments::Fragment( nOffset,
				min( nOffset + ED2K_PART_SIZE, m_nSize ) ) );
		}
	}
	else if ( m_nVerifyHash == HASH_TORRENT )
	{
		if ( m_pTorrent.FinishBlockTest( m_nVerifyBlock ) )
		{
			m_pTorrentBlock[ m_nVerifyBlock ] = TRI_TRUE;
			m_nTorrentSuccess ++;

			OnFinishedTorrentBlock( m_nVerifyBlock );
		}
		else
		{
			m_pTorrentBlock[ m_nVerifyBlock ] = TRI_FALSE;

			QWORD nOffset = QWORD(m_nVerifyBlock) * QWORD(m_nTorrentSize);
			oCorrupted.insert( oCorrupted.end(), Fragments::Fragment( nOffset,
				min( nOffset + m_nTorrentSize, m_nSize ) ) );
		}
	}

	if ( !oCorrupted.empty() && m_pFile != NULL )
	{
		if ( m_pTigerBlock != NULL )
			SubtractHelper( oCorrupted, m_pTigerBlock, m_nTigerBlock, m_nTigerSize );
		if ( m_pHashsetBlock != NULL )
			SubtractHelper( oCorrupted, m_pHashsetBlock, m_nHashsetBlock, ED2K_PART_SIZE );
		if ( m_pTorrentBlock != NULL )
			SubtractHelper( oCorrupted, m_pTorrentBlock, m_nTorrentBlock, m_nTorrentSize );

		for ( Fragments::List::const_iterator pRange = oCorrupted.begin();
			pRange != oCorrupted.end(); ++pRange )
		{
			m_pFile->InvalidateRange( pRange->begin(), pRange->size() );
			RemoveOverlappingSources( pRange->begin(), pRange->size() );
		}

	}
	m_nVerifyHash	= HASH_NULL;
	m_nVerifyBlock	= 0xFFFFFFFF;
	m_nVerifyCookie++;

	SetModified();
}

void CDownloadWithTiger::SubtractHelper(Fragments::List& ppCorrupted, BYTE* pBlock, QWORD nBlock, QWORD nSize)
{
	CQuickLock oLock( m_pTigerSection );

	QWORD nOffset = 0;

	while ( nBlock-- && !ppCorrupted.empty() )
	{
		if ( *pBlock++ == TRI_TRUE )
		{
			ppCorrupted.erase( Fragments::Fragment( nOffset, min( nOffset + nSize, m_nSize ) ) );
		}

		nOffset += nSize;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger available ranges override

CString CDownloadWithTiger::GetAvailableRanges() const
{
	CQuickLock oLock( m_pTigerSection );

	CString strRanges, strRange;
	QWORD nOffset, nLength;
	BOOL bSuccess;

	for ( nOffset = 0 ; GetNextVerifyRange( nOffset, nLength, bSuccess ) ; )
	{
		if ( bSuccess )
		{
			if ( strRanges.IsEmpty() )
				strRanges = _T("bytes ");
			else
				strRanges += ',';

			strRange.Format( _T("%I64i-%I64i"), nOffset, nOffset + nLength - 1 );
			strRanges += strRange;
		}

		nOffset += nLength;
	}

	if ( strRanges.IsEmpty() ) strRanges = CDownloadWithTorrent::GetAvailableRanges();

	return strRanges;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger clear data

void CDownloadWithTiger::ResetVerification()
{
	CQuickLock oLock( m_pTigerSection );

	if ( m_nVerifyHash == HASH_TIGERTREE )
	{
		m_pTigerTree.FinishBlockTest( m_nVerifyBlock );
	}
	else if ( m_nVerifyHash == HASH_ED2K )
	{
		m_pHashset.FinishBlockTest( m_nVerifyBlock );
	}
	else if ( m_nVerifyHash == HASH_TORRENT )
	{
		m_pTorrent.FinishBlockTest( m_nVerifyBlock );
	}

	if ( m_pTigerBlock != NULL ) ZeroMemory( m_pTigerBlock, m_nTigerBlock );
	if ( m_pHashsetBlock != NULL ) ZeroMemory( m_pHashsetBlock, m_nHashsetBlock );
	if ( m_pTorrentBlock != NULL ) ZeroMemory( m_pTorrentBlock, m_nTorrentBlock );

	m_nTigerSuccess		= 0;
	m_nHashsetSuccess	= 0;
	m_nTorrentSuccess	= 0;

	m_nVerifyHash		= HASH_NULL;
	m_nVerifyBlock		= 0xFFFFFFFF;

	m_nVerifyCookie++;
	SetModified();
}

void CDownloadWithTiger::ClearVerification()
{
	CQuickLock oLock( m_pTigerSection );

	ResetVerification();

	if ( m_pTigerBlock != NULL ) delete [] m_pTigerBlock;
	if ( m_pHashsetBlock != NULL ) delete [] m_pHashsetBlock;

	m_pTigerBlock		= NULL;
	m_nTigerBlock		= 0;
	m_pHashsetBlock		= NULL;
	m_nHashsetBlock		= 0;

	m_pTigerTree.Clear();
	m_pHashset.Clear();

	m_nVerifyCookie++;
	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger serialize

void CDownloadWithTiger::Serialize(CArchive& ar, int nVersion)
{
	CQuickLock oLock( m_pTigerSection );

	CDownloadWithTorrent::Serialize( ar, nVersion );

	CHashDatabase::Serialize( ar, &m_pTigerTree );

	if ( m_pTigerTree.IsAvailable() )
	{
		if ( ar.IsStoring() )
		{
			ar << m_nTigerBlock;
			ar << m_nTigerSize;
			ar << m_nTigerSuccess;
			ar.Write( m_pTigerBlock, sizeof(BYTE) * m_nTigerBlock );
		}
		else
		{
			m_pTigerTree.SetupParameters( m_nSize );

			ar >> m_nTigerBlock;
			ar >> m_nTigerSize;
			ar >> m_nTigerSuccess;

			m_pTigerBlock = new BYTE[ m_nTigerBlock ];
			ReadArchive( ar, m_pTigerBlock, sizeof(BYTE) * m_nTigerBlock );
		}
	}

	if ( nVersion >= 19 )
	{
		CHashDatabase::Serialize( ar, &m_pHashset );

		if ( m_pHashset.IsAvailable() )
		{
			if ( ar.IsStoring() )
			{
				ar << m_nHashsetBlock;
				ar << m_nHashsetSuccess;
				ar.Write( m_pHashsetBlock, sizeof(BYTE) * m_nHashsetBlock );
			}
			else
			{
				ar >> m_nHashsetBlock;
				ar >> m_nHashsetSuccess;

				m_pHashsetBlock = new BYTE[ m_nHashsetBlock ];
				ReadArchive( ar, m_pHashsetBlock, sizeof(BYTE) * m_nHashsetBlock );
			}
		}
	}

	if ( nVersion < 30 && m_oBTH )
	{
		ClearVerification();
		m_oSHA1.clear();
		m_oTiger.clear();
		m_oED2K.clear();
	}
}
