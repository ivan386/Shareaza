//
// DownloadWithTiger.cpp
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
#include "DownloadWithTiger.h"
#include "FragmentedFile.h"

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

CDownloadWithTiger::CDownloadWithTiger()
{
	m_pTigerBlock		= NULL;
	m_nTigerBlock		= 0;
	m_nTigerSuccess		= 0;
	
	m_pHashsetBlock		= NULL;
	m_nHashsetBlock		= 0;
	m_nHashsetSuccess	= 0;
	
	m_nVerifyCookie		= 0;
	m_nVerifyHash		= HASH_NULL;
	m_nVerifyBlock		= 0xFFFFFFFF;
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
	return m_nVerifyCookie;
}

QWORD CDownloadWithTiger::GetVerifyLength(int nHash) const
{
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
			TRISTATE nBase	= pBlockPtr[ nBlock ];
			bSuccess		= nBase == TS_TRUE;
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
	if ( m_nTorrentBlock > 0 && m_nTorrentSuccess >= m_nTorrentBlock ) return TRUE;
	if ( m_nTigerBlock > 0 && m_nTigerSuccess >= m_nTigerBlock ) return TRUE;
	if ( m_nHashsetBlock > 0 && m_nHashsetSuccess >= m_nHashsetBlock ) return TRUE;
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger tiger-tree access

BOOL CDownloadWithTiger::NeedTigerTree() const
{
	return ( m_nSize < SIZE_UNKNOWN && m_pTigerTree.IsAvailable() == FALSE );
}

BOOL CDownloadWithTiger::SetTigerTree(BYTE* pTiger, DWORD nTiger)
{
	if ( m_nSize == SIZE_UNKNOWN ) return FALSE;
	if ( m_pTigerTree.IsAvailable() ) return TRUE;
	
	if ( ! m_pTigerTree.FromBytes( pTiger, nTiger,
			Settings.Library.TigerHeight, m_nSize ) )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_TIGER_CORRUPT,
			(LPCTSTR)GetDisplayName() );
		return FALSE;
	}
	
	TIGEROOT pRoot;
	m_pTigerTree.GetRoot( &pRoot );
	
	if ( m_bTiger && m_pTiger != pRoot )
	{
		m_pTigerTree.Clear();
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_TIGER_MISMATCH,
			(LPCTSTR)GetDisplayName() );
		return FALSE;
	}
	else if ( ! m_bTiger )
	{
		m_bTiger = TRUE;
		m_pTiger = pRoot;
	}
	
	m_nTigerSize	= m_pTigerTree.GetBlockLength();
	m_nTigerBlock	= m_pTigerTree.GetBlockCount();
	m_pTigerBlock	= new BYTE[ m_nTigerBlock ];
	
	ZeroMemory( m_pTigerBlock, sizeof(BYTE) * m_nTigerBlock );
	
	SetModified();
	
	theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_TIGER_READY,
		(LPCTSTR)GetDisplayName(), m_pTigerTree.GetHeight(),
		(LPCTSTR)Settings.SmartVolume( m_nTigerSize, FALSE ) );
	
	return TRUE;
}

CTigerTree* CDownloadWithTiger::GetTigerTree()
{
	return m_pTigerTree.IsAvailable() ? &m_pTigerTree : NULL;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger eDonkey2000 hashset access

BOOL CDownloadWithTiger::NeedHashset() const
{
	return ( m_nSize < SIZE_UNKNOWN && m_pHashset.IsAvailable() == FALSE );
}

BOOL CDownloadWithTiger::SetHashset(BYTE* pSource, DWORD nSource)
{
	if ( m_nSize == SIZE_UNKNOWN ) return FALSE;
	if ( m_pHashset.IsAvailable() ) return TRUE;
	
	if ( nSource == 0 && m_bED2K )
	{
		m_pHashset.FromRoot( &m_pED2K );
	}
	else if ( m_pHashset.FromBytes( pSource, nSource, m_nSize ) )
	{
		MD4 pRoot;
		m_pHashset.GetRoot( &pRoot );
		
		if ( m_bED2K && m_pED2K != pRoot )
		{
			m_pHashset.Clear();
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_HASHSET_CORRUPT,
				(LPCTSTR)GetDisplayName() );
			return FALSE;
		}
		else if ( ! m_bED2K )
		{
			m_bED2K = TRUE;
			m_pED2K = pRoot;
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
	
	theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_HASHSET_READY,
		(LPCTSTR)GetDisplayName(),
		(LPCTSTR)Settings.SmartVolume( ED2K_PART_SIZE, FALSE ) );
	
	Neighbours.SendDonkeyDownload( reinterpret_cast<CDownload*>( this ) );
	
	return TRUE;
}

CED2K* CDownloadWithTiger::GetHashset()
{
	return m_pHashset.IsAvailable() ? &m_pHashset : NULL;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger test if the file can finish

BOOL CDownloadWithTiger::ValidationCanFinish() const
{
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
		if ( m_nHashsetSuccess >= m_nHashsetBlock ) return TRUE;
		bAvailable = TRUE;
	}
	
	return ! bAvailable;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger run validation

void CDownloadWithTiger::RunValidation(BOOL bSeeding)
{
	if ( m_pTigerBlock == NULL && m_pHashsetBlock == NULL && m_pTorrentBlock == NULL ) return;
	if ( m_sLocalName.IsEmpty() ) return;
	
	if ( ! bSeeding )
	{
		if ( m_pFile == NULL || ! OpenFile() ) return;
	}
	
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
		for ( TRISTATE nState = TS_UNKNOWN ; nState < TS_TRUE ; nState++ )
		{
			for ( DWORD nBlock = 0 ; nBlock < nBlockCount ; nBlock ++ )
			{
				if ( pBlockPtr[ nBlock ] == nState )
				{
					nTarget = nBlock;
					break;
				}
			}
			
			if ( nTarget != 0xFFFFFFFF ) break;
		}
	}
	else
	{
		DWORD nRetry = 0xFFFFFFFF;
		QWORD nPrevious = 0;
		
		for ( CFileFragment* pFragment = m_pFile->GetFirstEmptyFragment() ; pFragment ; pFragment = pFragment->m_pNext )
		{
			if ( pFragment->m_nOffset - nPrevious >= nBlockSize )
			{
				DWORD nBlock = (DWORD)( ( nPrevious + nBlockSize - 1 ) / nBlockSize );
				nPrevious = nBlockSize * (QWORD)nBlock + nBlockSize;
				
				for ( ; nPrevious <= pFragment->m_nOffset ; nBlock ++, nPrevious += nBlockSize )
				{
					if ( pBlockPtr[ nBlock ] == TS_UNKNOWN )
					{
						nTarget = nBlock;
						break;
					}
					else if ( pBlockPtr[ nBlock ] == TS_FALSE && nRetry == 0xFFFFFFFF )
					{
						nRetry = nBlock;
					}
				}
				
				if ( nTarget != 0xFFFFFFFF ) break;
			}
			
			nPrevious = pFragment->m_nOffset + pFragment->m_nLength;
		}
		
		if ( m_nSize > nPrevious && nTarget == 0xFFFFFFFF )
		{
			DWORD nBlock = (DWORD)( ( nPrevious + nBlockSize - 1 ) / nBlockSize );
			nPrevious = nBlockSize * (QWORD)nBlock;
			
			for ( ; nPrevious < m_nSize ; nBlock ++, nPrevious += nBlockSize )
			{
				if ( pBlockPtr[ nBlock ] == TS_UNKNOWN )
				{
					nTarget = nBlock;
					break;
				}
				else if ( pBlockPtr[ nBlock ] == TS_FALSE && nRetry == 0xFFFFFFFF )
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
	ASSERT( m_nVerifyHash > HASH_NULL );
	ASSERT( m_nVerifyBlock < 0xFFFFFFFF );
	
	BOOL bDone = ( m_pFile == NULL ) || ( m_pFile->GetRemaining() == 0 );
	HANDLE hComplete = INVALID_HANDLE_VALUE;
	
	if ( m_pFile == NULL )
	{
		hComplete = CreateFile( m_sLocalName, GENERIC_READ,
			FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL );
		if ( hComplete == INVALID_HANDLE_VALUE ) return;
	}
	
	for ( int nRound = bDone ? 10 : 2 ; nRound > 0 && m_nVerifyLength > 0 ; nRound-- )
	{
		DWORD nChunk	= (DWORD)min( m_nVerifyLength, Transfers.m_nBuffer );
		LPBYTE pChunk	= Transfers.m_pBuffer;
		
		if ( m_pFile != NULL )
		{
			m_pFile->ReadRange( m_nVerifyOffset, pChunk, nChunk );
		}
		else
		{
			LONG nOffsetHigh = (LONG)( m_nVerifyOffset >> 32 );
			SetFilePointer( hComplete, (DWORD)( m_nVerifyOffset & 0xFFFFFFFF ), &nOffsetHigh, FILE_BEGIN );
			ReadFile( hComplete, pChunk, nChunk, &nChunk, NULL );
		}
		
		if ( m_nVerifyHash == HASH_TIGERTREE )
			m_pTigerTree.AddToTest( pChunk, (DWORD)nChunk );
		else if ( m_nVerifyHash == HASH_ED2K )
			m_pHashset.AddToTest( pChunk, (DWORD)nChunk );
		else if ( m_nVerifyHash == HASH_TORRENT )
			m_pTorrent.AddToTest( pChunk, (DWORD)nChunk );
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
	CFileFragment* pCorrupted = NULL;
	
	if ( m_nVerifyHash == HASH_TIGERTREE )
	{
		if ( m_pTigerTree.FinishBlockTest( m_nVerifyBlock ) )
		{
			m_pTigerBlock[ m_nVerifyBlock ] = TS_TRUE;
			m_nTigerSuccess ++;
		}
		else
		{
			m_pTigerBlock[ m_nVerifyBlock ] = TS_FALSE;
			
			pCorrupted = CFileFragment::New();
			pCorrupted->m_nOffset	= (QWORD)m_nVerifyBlock * (QWORD)m_nTigerSize;
			pCorrupted->m_nLength	= min( m_nTigerSize, m_nSize - pCorrupted->m_nOffset );
		}
	}
	else if ( m_nVerifyHash == HASH_ED2K )
	{
		if ( m_pHashset.FinishBlockTest( m_nVerifyBlock ) )
		{
			m_pHashsetBlock[ m_nVerifyBlock ] = TS_TRUE;
			m_nHashsetSuccess ++;
		}
		else
		{
			m_pHashsetBlock[ m_nVerifyBlock ] = TS_FALSE;
			
			pCorrupted = CFileFragment::New();
			pCorrupted->m_nOffset	= (QWORD)m_nVerifyBlock * ED2K_PART_SIZE;
			pCorrupted->m_nLength	= min( ED2K_PART_SIZE, m_nSize - pCorrupted->m_nOffset );
		}
	}
	else if ( m_nVerifyHash == HASH_TORRENT )
	{
		if ( m_pTorrent.FinishBlockTest( m_nVerifyBlock ) )
		{
			m_pTorrentBlock[ m_nVerifyBlock ] = TS_TRUE;
			m_nTorrentSuccess ++;
			
			OnFinishedTorrentBlock( m_nVerifyBlock );
		}
		else
		{
			m_pTorrentBlock[ m_nVerifyBlock ] = TS_FALSE;
			
			pCorrupted = CFileFragment::New();
			pCorrupted->m_nOffset	= (QWORD)m_nVerifyBlock * (QWORD)m_nTorrentSize;
			pCorrupted->m_nLength	= min( m_nTorrentSize, m_nSize - pCorrupted->m_nOffset );
		}
	}
	
	if ( pCorrupted != NULL && m_pFile != NULL )
	{
		if ( m_pTigerBlock != NULL )
			SubtractHelper( &pCorrupted, m_pTigerBlock, m_nTigerBlock, m_nTigerSize );
		if ( m_pHashsetBlock != NULL )
			SubtractHelper( &pCorrupted, m_pHashsetBlock, m_nHashsetBlock, ED2K_PART_SIZE );
		if ( m_pTorrentBlock != NULL )
			SubtractHelper( &pCorrupted, m_pTorrentBlock, m_nTorrentBlock, m_nTorrentSize );
		
		for ( CFileFragment* pRange = pCorrupted ; pRange ; pRange = pRange->m_pNext )
		{
			m_pFile->InvalidateRange( pRange->m_nOffset, pRange->m_nLength );
			RemoveOverlappingSources( pRange->m_nOffset, pRange->m_nLength );
		}
		
		pCorrupted->DeleteChain();
	}
	
	m_nVerifyHash	= HASH_NULL;
	m_nVerifyBlock	= 0xFFFFFFFF;
	m_nVerifyCookie++;
	
	SetModified();
}

void CDownloadWithTiger::SubtractHelper(CFileFragment** ppCorrupted, BYTE* pBlock, QWORD nBlock, QWORD nSize)
{
	QWORD nOffset = 0;
	
	while ( nBlock-- && *ppCorrupted )
	{
		if ( *pBlock++ == TS_TRUE )
		{
			CFileFragment::Subtract( ppCorrupted, nOffset, nSize );
		}
		
		nOffset += nSize;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger available ranges override

CString CDownloadWithTiger::GetAvailableRanges() const
{
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
	CDownloadWithTorrent::Serialize( ar, nVersion );
	
	m_pTigerTree.Serialize( ar );
	
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
			ar.Read( m_pTigerBlock, sizeof(BYTE) * m_nTigerBlock );
		}
	}
	
	if ( nVersion >= 19 )
	{
		m_pHashset.Serialize( ar );
		
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
				ar.Read( m_pHashsetBlock, sizeof(BYTE) * m_nHashsetBlock );
			}
		}
	}
	
	if ( nVersion < 30 && m_bBTH )
	{
		ClearVerification();
		m_bSHA1 = m_bTiger = m_bED2K = FALSE;
	}
}
