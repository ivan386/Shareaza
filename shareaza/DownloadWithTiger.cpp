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
	m_nTigerBlock = 0;
	m_nTigerSize = 0;
	m_pTigerVerificationQueue = NULL;
	m_pTigerVerificationCandidates = NULL;
	
	m_nHashsetBlock = 0;
	m_pHashsetVerificationQueue = NULL;
	m_pHashsetVerificationCandidates = NULL;
	
	m_nVerifyCookie = 0;
	m_nVerifyHash = HASH_NULL;
}

CDownloadWithTiger::~CDownloadWithTiger()
{
	delete [] m_pTigerVerificationQueue;
	delete [] m_pTigerVerificationCandidates;
	delete [] m_pHashsetVerificationQueue;
	delete [] m_pHashsetVerificationCandidates;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger informational access

DWORD CDownloadWithTiger::GetValidationCookie() const
{
	return m_nVerifyCookie;
}

QWORD CDownloadWithTiger::GetVerifyLength(HASHID nHash) const
{
	if ( nHash == HASH_NULL )
	{
		if ( m_oBTH.IsTrusted() ) return m_nTorrentSize;
		if ( m_oTigerTree.IsAvailable() && m_oTiger.IsTrusted() ) return m_nTigerSize;
		if ( m_oHashset.IsAvailable() && m_oED2K.IsTrusted() ) return ED2K_PART_SIZE;
	}
	else if ( nHash == HASH_TIGERTREE && m_oTigerTree.IsAvailable() && m_oTiger.IsTrusted() ) return m_nTigerSize;
	else if ( nHash == HASH_ED2K && m_oHashset.IsAvailable() && m_oED2K.IsTrusted() ) return ED2K_PART_SIZE;
	else if ( nHash == HASH_TORRENT && m_oBTH.IsTrusted() ) return m_nTorrentSize;
	return 0;
}

BOOL CDownloadWithTiger::GetNextVerifyRange(QWORD& nOffset, QWORD& nLength, DWORD& nVerifyState) const
{
	CFileFragment *pNextVerified, *pNextInvalid;
	if ( m_nVerifyHash == HASH_SHA1 || m_nVerifyHash == HASH_MD5 )
	{
		if ( !nOffset && nLength )
		{
			nLength = m_nVerifyOffset;
			nVerifyState = 3;
			return TRUE;
		}
		if ( nOffset == m_nSize || !m_bVerifySpeculative ) return FALSE;
		nLength = m_nSize - m_nVerifyOffset;
		nVerifyState = 1;
		return TRUE;
	}
	if ( pNextVerified = m_oVerified.FindNextFragment( nOffset ) )
	{
		if ( ( pNextInvalid = m_oInvalid.FindNextFragment( nOffset ) )
			&& ( pNextInvalid->Offset() < pNextVerified->Offset() ) )
		{
			nOffset = pNextInvalid->Offset();
			nLength = pNextInvalid->Length();
			nVerifyState = 1;
			return TRUE;
		}
		else
		{
			nOffset = pNextVerified->Offset();
			nLength = pNextVerified->Length();
			nVerifyState = 2;
			return TRUE;
		}
	}
	else if ( pNextInvalid = m_oInvalid.FindNextFragment( nOffset ) )
	{
		nOffset = pNextInvalid->Offset();
		nLength = pNextInvalid->Length();
		nVerifyState = 1;
		return TRUE;
	}
	return FALSE;
}

BOOL CDownloadWithTiger::IsFullyVerified()
{
	return ( m_oVerified.GetSize() == m_nSize ) || ( m_pFile && m_pFile->m_oFree.IsEmpty()
		&& ! m_oSHA1.IsTrusted() && ! m_oMD5.IsTrusted()
		&& ( ! m_oBTH.IsTrusted() || ! m_pBTHVerificationQueue )
		&& ( ! m_oTiger.IsTrusted() || ! m_pTigerVerificationQueue )
		&& ( ! m_oED2K.IsTrusted() || ! m_pHashsetVerificationQueue ) );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger tiger-tree access

BOOL CDownloadWithTiger::NeedTigerTree() const
{
	return ( m_nSize < SIZE_UNKNOWN && ! m_oTigerTree.IsAvailable() );
}

BOOL CDownloadWithTiger::SetTigerTree(BYTE* pTiger, DWORD nTiger)
{
	if ( m_nSize == SIZE_UNKNOWN ) return FALSE;
	if ( m_oTigerTree.IsAvailable() ) return TRUE;
	if ( ! m_oTigerTree.FromBytes( pTiger, nTiger, Settings.Library.TigerHeight, m_nSize ) )
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_TIGER_CORRUPT, (LPCTSTR)GetDisplayName() );
		return FALSE;
	}
	if ( m_oTiger.IsValid()  )
	{
		if ( m_oTiger != m_oTigerTree )
		{
			m_oTigerTree.Clear();
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_TIGER_MISMATCH, (LPCTSTR)GetDisplayName() );
			return FALSE;
		}
	}
	else
	{
		m_oTiger = m_oTigerTree;
	}
	m_nTigerSize	= m_oTigerTree.GetBlockLength();
	m_nTigerBlock	= m_oTigerTree.GetBlockCount();
	SetModified();
	theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_TIGER_READY, (LPCTSTR)GetDisplayName(), m_oTigerTree.GetHeight(),
		(LPCTSTR)Settings.SmartVolume( m_nTigerSize, FALSE ) );
	m_pTigerVerificationQueue = new DWORD[ m_nTigerBlock + 1 ];
	m_nTigerVerificationEnd = m_nTigerVerificationStart = m_nTigerBlock;
	m_pTigerVerificationCandidates = new BYTE[ m_nTigerBlock ];
	ZeroMemory( m_pTigerVerificationCandidates, sizeof(BYTE) * m_nTigerBlock );
	return TRUE;
}

CTigerTree* CDownloadWithTiger::GetTigerTree()
{
	return m_oTigerTree.IsAvailable() ? &m_oTigerTree : NULL;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger eDonkey2000 hashset access

BOOL CDownloadWithTiger::NeedHashset() const
{
	return ( m_nSize < SIZE_UNKNOWN && ! m_oHashset.IsAvailable() );
}

BOOL CDownloadWithTiger::SetHashset(BYTE* pSource, DWORD nSource)
{
	if ( m_nSize == SIZE_UNKNOWN ) return FALSE;
	if ( m_oHashset.IsAvailable() ) return TRUE;
	if ( ! nSource && m_oED2K.IsValid() ) m_oHashset.FromRoot( m_oED2K );
	else if ( m_oHashset.FromBytes( pSource, nSource, m_nSize ) )
	{
		if ( m_oED2K.IsValid() )
		{
			if ( m_oED2K != m_oHashset )
			{
				m_oHashset.Clear();
				theApp.Message( MSG_ERROR, IDS_DOWNLOAD_HASHSET_CORRUPT, (LPCTSTR)GetDisplayName() );
				return FALSE;
			}
		}
		else
		{
			m_oED2K = m_oHashset;
		}
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_HASHSET_CORRUPT, (LPCTSTR)GetDisplayName() );
		return FALSE;
	}
	m_nHashsetBlock	= m_oHashset.GetBlockCount();
	SetModified();
	theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_HASHSET_READY, (LPCTSTR)GetDisplayName(),
		(LPCTSTR)Settings.SmartVolume( ED2K_PART_SIZE, FALSE ) );
	Neighbours.SendDonkeyDownload( reinterpret_cast<CDownload*>( this ) );
	m_pHashsetVerificationQueue = new DWORD[ m_nHashsetBlock + 1 ];
	m_nHashsetVerificationEnd = m_nHashsetVerificationStart = m_nHashsetBlock;
	m_pHashsetVerificationCandidates = new BYTE[ m_nHashsetBlock ];
	ZeroMemory( m_pHashsetVerificationCandidates, sizeof(BYTE) * m_nHashsetBlock );
	return TRUE;
}

CED2K* CDownloadWithTiger::GetHashset()
{
	return m_oHashset.IsAvailable() ? &m_oHashset : NULL;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger test if the file can finish

BOOL CDownloadWithTiger::ValidationCanFinish() const
{
	return ( m_oVerified.GetSize() == m_nSize ) || ( m_pFile && m_pFile->m_oFree.IsEmpty()
		&& ! m_oSHA1.IsTrusted() && ! m_oMD5.IsTrusted()
		&& ( ! m_oBTH.IsTrusted() || ! m_pBTHVerificationQueue )
		&& ( ! m_oTiger.IsTrusted() || ! m_pTigerVerificationQueue )
		&& ( ! m_oED2K.IsTrusted() || ! m_pHashsetVerificationQueue ) );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger run validation

void CDownloadWithTiger::RunValidation(BOOL bSeeding)
{
	if ( m_sLocalName.IsEmpty() || ( !bSeeding && ( !m_pFile || !OpenFile() ) ) ) return;
	if ( m_nVerifyHash != HASH_NULL )
	{
		Downloads.m_nValidation++;
		ContinueValidation();
		return;
	}
	if ( m_oBTH.IsTrusted() && m_pBTHVerificationQueue && m_nBTHVerificationStart != m_nBTHVerificationEnd )
	{
		m_nVerifyHash = HASH_TORRENT;
		m_nVerifyBlock = m_pBTHVerificationQueue[ m_nBTHVerificationStart ];
		ASSERT( m_pBTHVerificationCandidates[ m_nVerifyBlock ] );
		m_nVerifyOffset = (QWORD)( m_nVerifyBlock ) * m_nTorrentSize;
		m_nVerifyLength = min( m_nTorrentSize, m_nSize - m_nVerifyOffset );
		if ( m_oVerified.ContainsRange( m_nVerifyOffset, m_nVerifyOffset + m_nVerifyLength ) )
		{
			m_pBTHVerificationCandidates[ m_nVerifyBlock ] = FALSE;
			if ( ! m_nBTHVerificationStart-- ) m_nBTHVerificationStart = m_nTorrentBlock;
			m_nVerifyHash = HASH_NULL;
		}
		else
		{
			if ( ! ( m_bVerifySpeculative = ( m_pFile
				&& m_pFile->m_oFree.OverlapsRange( m_nVerifyOffset, m_nVerifyOffset + m_nVerifyLength ) ) ) )
			{
				m_pBTHVerificationCandidates[ m_nVerifyBlock ] = FALSE;
				if ( ! m_nBTHVerificationStart-- ) m_nBTHVerificationStart = m_nTorrentBlock;
				m_tVerifyLast = GetTickCount();
				m_pTorrent.BeginBlockTest();
				Downloads.m_nValidation++;
				ContinueValidation();
				return;
			}
		}
		
	}
	if ( m_oTiger.IsTrusted() && m_pTigerVerificationQueue && m_nTigerVerificationStart != m_nTigerVerificationEnd )
	{
		m_nVerifyHash = HASH_TIGERTREE;
		m_nVerifyBlock = m_pTigerVerificationQueue[ m_nTigerVerificationStart ];
		ASSERT( m_pTigerVerificationCandidates[ m_nVerifyBlock ] );
		m_nVerifyOffset = (QWORD)( m_nVerifyBlock ) * m_nTigerSize;
		m_nVerifyLength = min( m_nTigerSize, m_nSize - m_nVerifyOffset );
		if ( m_oVerified.ContainsRange( m_nVerifyOffset, m_nVerifyOffset + m_nVerifyLength ) )
		{
			m_pTigerVerificationCandidates[ m_nVerifyBlock ] = FALSE;
			if ( ! m_nTigerVerificationStart-- ) m_nTigerVerificationStart = m_nTigerBlock;
			m_nVerifyHash = HASH_NULL;
		}
		else
		{
			if ( ! ( m_bVerifySpeculative = ( m_pFile
				&& m_pFile->m_oFree.OverlapsRange( m_nVerifyOffset, m_nVerifyOffset + m_nVerifyLength ) ) ) )
			{
				m_pTigerVerificationCandidates[ m_nVerifyBlock ] = FALSE;
				if ( ! m_nTigerVerificationStart-- ) m_nTigerVerificationStart = m_nTigerBlock;
				m_tVerifyLast = GetTickCount();
				m_oTigerTree.BeginBlockTest();
				Downloads.m_nValidation++;
				ContinueValidation();
				return;
			}
		}
	}
	if ( m_oED2K.IsTrusted() && m_pHashsetVerificationQueue && m_nHashsetVerificationStart != m_nHashsetVerificationEnd )
	{
		m_nVerifyHash = HASH_ED2K;
		m_nVerifyBlock = m_pHashsetVerificationQueue[ m_nHashsetVerificationStart ];
		ASSERT( m_pHashsetVerificationCandidates[ m_nVerifyBlock ] );
		m_nVerifyOffset = (QWORD)( m_nVerifyBlock ) * ED2K_PART_SIZE;
		m_nVerifyLength = min( ED2K_PART_SIZE, m_nSize - m_nVerifyOffset );
		if ( m_oVerified.ContainsRange( m_nVerifyOffset, m_nVerifyOffset + m_nVerifyLength ) )
		{
			m_pHashsetVerificationCandidates[ m_nVerifyBlock ] = FALSE;
			if ( ! m_nHashsetVerificationStart-- ) m_nHashsetVerificationStart = m_nHashsetBlock;
			m_nVerifyHash = HASH_NULL;
		}
		else
		{
			if ( ! ( m_bVerifySpeculative = ( m_pFile
				&& m_pFile->m_oFree.OverlapsRange( m_nVerifyOffset, m_nVerifyOffset + m_nVerifyLength ) ) ) )
			{
				m_pHashsetVerificationCandidates[ m_nVerifyBlock ] = FALSE;
				if ( ! m_nHashsetVerificationStart-- ) m_nHashsetVerificationStart = m_nHashsetBlock;
				m_tVerifyLast = GetTickCount();
				m_oHashset.BeginBlockTest();
				Downloads.m_nValidation++;
				ContinueValidation();
				return;
			}
		}
	}
	if ( m_oBTH.IsValid() && ! m_oBTH.IsTrusted() && m_pBTHVerificationQueue
		&& m_nBTHVerificationStart != m_nBTHVerificationEnd )
	{
		m_nVerifyHash = HASH_TORRENT;
		m_nVerifyBlock = m_pBTHVerificationQueue[ m_nBTHVerificationStart ];
		if ( ! m_nBTHVerificationStart-- ) m_nBTHVerificationStart = m_nTorrentBlock;
		ASSERT( m_pBTHVerificationCandidates[ m_nVerifyBlock ] );
		m_pBTHVerificationCandidates[ m_nVerifyBlock ] = FALSE;
		m_nVerifyOffset = (QWORD)( m_nVerifyBlock ) * m_nTorrentSize;
		m_nVerifyLength = min( m_nTorrentSize, m_nSize - m_nVerifyOffset );
		if ( m_oVerified.ContainsRange( m_nVerifyOffset, m_nVerifyOffset + m_nVerifyLength ) )
		{
			m_bVerifySpeculative = FALSE;
			m_tVerifyLast = GetTickCount();
			m_pTorrent.BeginBlockTest();
			Downloads.m_nValidation++;
			ContinueValidation();
			return;
		}
		else
		{
			m_nVerifyHash = HASH_NULL;
		}
	}
	if ( m_oTiger.IsValid() && ! m_oTiger.IsTrusted() && m_pTigerVerificationQueue
		&& m_nTigerVerificationStart != m_nTigerVerificationEnd )
	{
		m_nVerifyHash = HASH_TIGERTREE;
		m_nVerifyBlock = m_pTigerVerificationQueue[ m_nTigerVerificationStart ];
		if ( ! m_nTigerVerificationStart-- ) m_nTigerVerificationStart = m_nTigerBlock;
		ASSERT( m_pTigerVerificationCandidates[ m_nVerifyBlock ] );
		m_pTigerVerificationCandidates[ m_nVerifyBlock ] = FALSE;
		m_nVerifyOffset = (QWORD)( m_nVerifyBlock ) * m_nTigerSize;
		m_nVerifyLength = min( m_nTigerSize, m_nSize - m_nVerifyOffset );
		if ( m_oVerified.ContainsRange( m_nVerifyOffset, m_nVerifyOffset + m_nVerifyLength ) )
		{
			m_bVerifySpeculative = FALSE;
			m_tVerifyLast = GetTickCount();
			m_oTigerTree.BeginBlockTest();
			Downloads.m_nValidation++;
			ContinueValidation();
			return;
		}
		else
		{
			m_nVerifyHash = HASH_NULL;
		}
	}
	if ( m_oED2K.IsValid() && ! m_oED2K.IsTrusted() && m_pHashsetVerificationQueue
		&& m_nHashsetVerificationStart != m_nHashsetVerificationEnd )
	{
		m_nVerifyHash = HASH_ED2K;
		m_nVerifyBlock = m_pHashsetVerificationQueue[ m_nHashsetVerificationStart ];
		if ( ! m_nHashsetVerificationStart--) m_nHashsetVerificationStart = m_nHashsetBlock;
		ASSERT( m_pHashsetVerificationCandidates[ m_nVerifyBlock ] );
		m_pHashsetVerificationCandidates[ m_nVerifyBlock ] = FALSE;
		m_nVerifyOffset = (QWORD)( m_nVerifyBlock ) * ED2K_PART_SIZE;
		m_nVerifyLength = min( ED2K_PART_SIZE, m_nSize - m_nVerifyOffset );
		if ( m_oVerified.ContainsRange( m_nVerifyOffset, m_nVerifyOffset + m_nVerifyLength ) )
		{
			m_bVerifySpeculative = FALSE;
			m_tVerifyLast = GetTickCount();
			m_oHashset.BeginBlockTest();
			Downloads.m_nValidation++;
			ContinueValidation();
			return;
		}
		else
		{
			m_nVerifyHash = HASH_NULL;
		}
	}
	if ( m_nVerifyHash != HASH_NULL )					// speculative
	{
		if ( m_nVerifyHash == HASH_TORRENT )
		{
			m_pBTHVerificationCandidates[ m_nVerifyBlock ] = FALSE;
			if ( ! m_nBTHVerificationStart-- ) m_nBTHVerificationStart = m_nTorrentBlock;
			if ( m_pFile->m_oFree.LessOrEqualMatch( m_oInvalid, m_nVerifyOffset, m_nVerifyOffset + m_nVerifyLength ) )
			{
				m_tVerifyLast = GetTickCount();
				m_pTorrent.BeginBlockTest();
				Downloads.m_nValidation++;
				ContinueValidation();
				return;
			}
		}
		else if ( m_nVerifyHash == HASH_TIGERTREE )
		{
			m_pTigerVerificationCandidates[ m_nVerifyBlock ] = FALSE;
			if ( ! m_nTigerVerificationStart-- ) m_nTigerVerificationStart = m_nTigerBlock;
			if ( m_pFile->m_oFree.LessOrEqualMatch( m_oInvalid, m_nVerifyOffset, m_nVerifyOffset + m_nVerifyLength ) )
			{
				m_tVerifyLast = GetTickCount();
				m_oTigerTree.BeginBlockTest();
				Downloads.m_nValidation++;
				ContinueValidation();
				return;
			}
		}
		else if ( m_nVerifyHash == HASH_ED2K )
		{
			m_pHashsetVerificationCandidates[ m_nVerifyBlock ] = FALSE;
			if ( ! m_nHashsetVerificationStart-- ) m_nHashsetVerificationStart = m_nHashsetBlock;
			if ( m_pFile->m_oFree.LessOrEqualMatch( m_oInvalid, m_nVerifyOffset, m_nVerifyOffset + m_nVerifyLength ) )
			{
				m_tVerifyLast = GetTickCount();
				m_oHashset.BeginBlockTest();
				Downloads.m_nValidation++;
				ContinueValidation();
				return;
			}
		}
		else ASSERT( FALSE );
	}
	else
	{		// == HASH_NULL
		if ( ( m_oSHA1.IsTrusted() || m_oMD5.IsTrusted() ) && m_pFile )
		{
			if ( m_pFile->m_oFree.IsEmpty() )
			{
				m_bVerifySpeculative = FALSE;
				m_nVerifyBlock = 0;
				m_nVerifyOffset = 0;
				m_nVerifyLength = m_nSize;
				m_tVerifyLast = GetTickCount();
				if ( m_oSHA1.IsTrusted() )
				{
					m_nVerifyHash = HASH_SHA1;
					BeginSHA1Test();
				}
				else
				{
					m_nVerifyHash = HASH_SHA1;
					BeginMD5Test();
				}
				Downloads.m_nValidation++;
				ContinueValidation();
				return;
			}
			else if ( ( m_oInvalid.GetSize() == m_nSize )
				&& ( ! m_oBTH.IsTrusted() || ! m_pBTHVerificationQueue )
				&& ( ! m_oTiger.IsTrusted() || ! m_pTigerVerificationQueue )
				&& ( ! m_oED2K.IsTrusted() || ! m_pHashsetVerificationQueue ) )
			{
				m_bVerifySpeculative = TRUE;
				m_nVerifyBlock = 0;
				m_nVerifyOffset = 0;
				m_nVerifyLength = m_nSize;
				m_tVerifyLast = GetTickCount();
				if ( m_oSHA1.IsTrusted() )
				{
					m_nVerifyHash = HASH_SHA1;
					BeginSHA1Test();
				}
				else
				{
					m_nVerifyHash = HASH_SHA1;
					BeginMD5Test();
				}
				Downloads.m_nValidation++;
				ContinueValidation();
				return;
			}
		}
	}
	m_nVerifyHash = HASH_NULL;
}

void CDownloadWithTiger::BeginSHA1Test()
{
	m_oTestSHA1.Reset();
}

void CDownloadWithTiger::BeginMD5Test()
{
	m_oTestMD5.Reset();
}

BOOL CDownloadWithTiger::FinishSHA1Test()
{
	m_oTestSHA1.Finish();
	return m_oSHA1 == m_oTestSHA1;
}

BOOL CDownloadWithTiger::FinishMD5Test()
{
	m_oTestMD5.Finish();
	return m_oMD5 == m_oTestMD5;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger validation process

void CDownloadWithTiger::ContinueValidation()
{
	ASSERT( m_nVerifyHash > HASH_NULL );
	BOOL bDone = ( !m_pFile || !m_pFile->GetRemaining() );
	HANDLE hComplete = INVALID_HANDLE_VALUE;
	if ( !m_pFile )
	{
		hComplete = CreateFile( m_sLocalName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL );
		if ( hComplete == INVALID_HANDLE_VALUE ) return;
	}
	for ( int nRound = bDone ? 20 : 4 ; nRound && m_nVerifyLength ; nRound-- )
	{
		DWORD nChunk	= (DWORD)min( m_nVerifyLength, Transfers.m_nBuffer );
		LPBYTE pChunk	= Transfers.m_pBuffer;
		if ( m_pFile ) m_pFile->ReadRangeUnlimited( m_nVerifyOffset, pChunk, nChunk );
		else
		{
			LONG nOffsetHigh = (LONG)( m_nVerifyOffset >> 32 );
			SetFilePointer( hComplete, (DWORD)m_nVerifyOffset, &nOffsetHigh, FILE_BEGIN );
			ReadFile( hComplete, pChunk, nChunk, &nChunk, NULL );
		}
		if ( m_nVerifyHash == HASH_TORRENT ) m_pTorrent.AddToTest( pChunk, nChunk );
		else if ( m_nVerifyHash == HASH_TIGERTREE ) m_oTigerTree.AddToTest( pChunk, nChunk );
		else if ( m_nVerifyHash == HASH_ED2K ) m_oHashset.AddToTest( pChunk, nChunk );
		else if ( m_nVerifyHash == HASH_SHA1 )
		{
			m_oTestSHA1.Add( pChunk, nChunk );
			SetModified();
		}
		else if ( m_nVerifyHash == HASH_MD5 )
		{
			m_oTestMD5.Add( pChunk, nChunk );
			SetModified();
		}
		else ASSERT( FALSE );
		m_nVerifyOffset += nChunk;
		m_nVerifyLength -= nChunk;
	}
	if ( hComplete != INVALID_HANDLE_VALUE ) CloseHandle( hComplete );
	if ( !m_nVerifyLength ) FinishValidation();
}

void CDownloadWithTiger::FinishValidation()
{
	QWORD nOffset, nNext;
	BOOL bSuccess = FALSE;
	BOOL bInvalidate = FALSE;
	if ( m_nVerifyHash == HASH_TORRENT )
	{
		nOffset = (QWORD)m_nVerifyBlock * m_nTorrentSize;
		nNext = min( m_nSize, nOffset + m_nTorrentSize );
		if ( m_pTorrent.FinishBlockTest( m_nVerifyBlock ) )
		{
			m_oBTH.SetSuccess();
			if ( m_oBTH.IsTrusted() )
			{
				if ( m_bVerifySpeculative )
				{
					if ( m_pFile ) m_pFile->m_oFree.Subtract( nOffset, nNext );
					if ( ! m_pBTHVerificationCandidates[ m_nVerifyBlock ] )
					{
						m_pBTHVerificationCandidates[ m_nVerifyBlock ] = TRUE;
						m_pBTHVerificationQueue[ m_nBTHVerificationEnd ] = m_nVerifyBlock;
						if ( ! m_nBTHVerificationEnd-- ) m_nBTHVerificationEnd = m_nTorrentBlock;
					}
				}
				else
				{
					bSuccess = TRUE;
					OnFinishedTorrentBlock( m_nVerifyBlock );
				}
			}
		}
		else
		{
			m_oBTH.SetFailure();
			bInvalidate = m_oBTH.IsTrusted() && ! m_bVerifySpeculative;
		}
	}
	else if ( m_nVerifyHash == HASH_TIGERTREE )
	{
		nOffset = (QWORD)m_nVerifyBlock * m_nTigerSize;
		nNext = min( m_nSize, nOffset + m_nTigerSize );
		if ( m_oTigerTree.FinishBlockTest( m_nVerifyBlock ) )
		{
			m_oTiger.SetSuccess();
			if ( m_oTiger.IsTrusted() )
			{
				if ( m_bVerifySpeculative )
				{
					if ( m_pFile ) m_pFile->m_oFree.Subtract( nOffset, nNext );
					if ( ! m_pTigerVerificationCandidates[ m_nVerifyBlock ] )
					{
						m_pTigerVerificationCandidates[ m_nVerifyBlock ] = TRUE;
						m_pTigerVerificationQueue[ m_nTigerVerificationEnd ] = m_nVerifyBlock;
						if ( ! m_nTigerVerificationEnd-- ) m_nTigerVerificationEnd = m_nTigerBlock;
					}
				}
				else bSuccess = TRUE;
			}
		}
		else
		{
			m_oTiger.SetFailure();
			bInvalidate = m_oTiger.IsTrusted() && ! m_bVerifySpeculative;
		}
	}
	else if ( m_nVerifyHash == HASH_ED2K )
	{
		nOffset = (QWORD)m_nVerifyBlock * ED2K_PART_SIZE;
		nNext = min( m_nSize, nOffset + ED2K_PART_SIZE );
		if ( m_oHashset.FinishBlockTest( m_nVerifyBlock ) )
		{
			m_oED2K.SetSuccess();
			if ( m_oED2K.IsTrusted() )
			{
				if ( m_bVerifySpeculative )
				{
					if ( m_pFile ) m_pFile->m_oFree.Subtract( nOffset, nNext );
					if ( ! m_pHashsetVerificationCandidates[ m_nVerifyBlock ] )
					{
						m_pHashsetVerificationCandidates[ m_nVerifyBlock ] = TRUE;
						m_pHashsetVerificationQueue[ m_nHashsetVerificationEnd ] = m_nVerifyBlock;
						if ( ! m_nHashsetVerificationEnd-- ) m_nHashsetVerificationEnd = m_nHashsetBlock;
					}
				}
				else bSuccess = TRUE;
			}
		}
		else
		{
			m_oED2K.SetFailure();
			bInvalidate = m_oED2K.IsTrusted() && ! m_bVerifySpeculative;
		}
	}
	else if ( m_nVerifyHash == HASH_SHA1 )
	{
		nOffset = 0;
		nNext = m_nSize;
		if ( FinishSHA1Test() )
		{
			if ( m_bVerifySpeculative )
			{
				m_pFile->m_oFree.Delete();
			}
			else bSuccess = TRUE;
		}
		else
		{
			bInvalidate = ! m_bVerifySpeculative;
		}
	}
	else if ( m_nVerifyHash == HASH_MD5 )
	{
		nOffset = 0;
		nNext = m_nSize;
		if ( FinishMD5Test() )
		{
			if ( m_bVerifySpeculative )
			{
				m_pFile->m_oFree.Delete();
			}
			else bSuccess = TRUE;
		}
		else
		{
			bInvalidate = ! m_bVerifySpeculative;
		}
	}
	else ASSERT( FALSE );
	if ( bSuccess )
	{
		m_oVerified.Add( nOffset, nNext );
		( m_oInvalid.Subtract( nOffset, nNext ) && m_pFile );
		m_pFile->m_oFree.Subtract( nOffset, nNext );
		SetModified();
	}
	else if ( bInvalidate && m_pFile )
	{
		CFileFragmentList Corrupted;
		Corrupted.Add( nOffset, nNext );
		Corrupted.Subtract( m_oVerified );
		m_pFile->InvalidateRange( Corrupted );
		m_oInvalid.Merge( Corrupted );
		SetModified();
	}
	m_nVerifyHash = HASH_NULL;
	m_nVerifyCookie++;
}

void CDownloadWithTiger::AddVerificationBlocks(const QWORD nOffset, const QWORD nNext)
{
	QWORD nBlockOffset, nBlockNext;
	DWORD nBlock;
	if ( m_pBTHVerificationQueue )
	{
		nBlock = (DWORD)( nOffset / m_nTorrentSize );
		nBlockOffset = (QWORD)nBlock * m_nTorrentSize;
		nBlockNext = nBlockOffset + m_nTorrentSize;
		while ( nBlockOffset < nNext )
		{
			if ( !m_pBTHVerificationCandidates[ nBlock ] )
			{
				m_pBTHVerificationCandidates[ nBlock ] = TRUE;
				m_pBTHVerificationQueue[ m_nBTHVerificationEnd ] = nBlock;
				if ( ! m_nBTHVerificationEnd-- ) m_nBTHVerificationEnd = m_nTorrentBlock;
			}
			nBlock++;
			nBlockOffset = nBlockNext;
			nBlockNext += m_nTorrentSize;
		}
	}
	if ( m_pTigerVerificationQueue )
	{
		nBlock = (DWORD)( nOffset / m_nTigerSize );
		nBlockOffset = (QWORD)nBlock * m_nTigerSize;
		nBlockNext = nBlockOffset + m_nTigerSize;
		while ( nBlockOffset < nNext )
		{
			if ( !m_pTigerVerificationCandidates[ nBlock ] )
			{
				m_pTigerVerificationCandidates[ nBlock ] = TRUE;
				m_pTigerVerificationQueue[ m_nTigerVerificationEnd ] = nBlock;
				if ( ! m_nTigerVerificationEnd-- ) m_nTigerVerificationEnd = m_nTigerBlock;
			}
			nBlock++;
			nBlockOffset = nBlockNext;
			nBlockNext += m_nTigerSize;
		}
	}
	if ( m_pHashsetVerificationQueue )
	{
		nBlock = (DWORD)( nOffset / ED2K_PART_SIZE );
		nBlockOffset = (QWORD)nBlock * ED2K_PART_SIZE;
		nBlockNext = nBlockOffset + ED2K_PART_SIZE;
		while ( nBlockOffset < nNext )
		{
			if ( !m_pHashsetVerificationCandidates[ nBlock ] )
			{
				m_pHashsetVerificationCandidates[ nBlock ] = TRUE;
				m_pHashsetVerificationQueue[ m_nHashsetVerificationEnd ] = nBlock;
				if ( ! m_nHashsetVerificationEnd-- ) m_nHashsetVerificationEnd = m_nHashsetBlock;
			}
			nBlock++;
			nBlockOffset = nBlockNext;
			nBlockNext += ED2K_PART_SIZE;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger available ranges override

CString CDownloadWithTiger::GetAvailableRanges() const
{
	CString strRanges, strRange;
	CFileFragment *pFragment;
	if ( pFragment = m_oVerified.GetFirst() )
	{
		strRange.Format( _T("%I64i-%I64i"), pFragment->Offset(), pFragment->Next() - 1 );
		strRanges = _T("bytes ") + strRange;
		while ( pFragment = pFragment->GetNext() )
		{
			strRange.Format( _T("%I64i-%I64i"), pFragment->Offset(), pFragment->Next() - 1 );
			strRanges += ',' + strRange;
		}
	}
	return strRanges;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger clear data

void CDownloadWithTiger::ResetVerification()
{
	if ( m_nVerifyHash == HASH_TIGERTREE ) m_oTigerTree.FinishBlockTest( m_nVerifyBlock );
	else if ( m_nVerifyHash == HASH_ED2K ) m_oHashset.FinishBlockTest( m_nVerifyBlock );
	else if ( m_nVerifyHash == HASH_TORRENT ) m_pTorrent.FinishBlockTest( m_nVerifyBlock );
	if ( m_pBTHVerificationQueue )
	{
		m_nBTHVerificationEnd = m_nBTHVerificationStart = m_nTorrentBlock;
		ZeroMemory( m_pBTHVerificationCandidates, sizeof(BYTE) * m_nTorrentBlock );
	}
	if ( m_pTigerVerificationQueue )
	{
		m_nTigerVerificationEnd = m_nTigerVerificationStart = m_nTigerBlock;
		ZeroMemory( m_pTigerVerificationCandidates, sizeof(BYTE) * m_nTigerBlock );
	}
	if ( m_pHashsetVerificationQueue )
	{
		m_nHashsetVerificationEnd = m_nHashsetVerificationStart = m_nHashsetBlock;
		ZeroMemory( m_pHashsetVerificationCandidates, sizeof(BYTE) * m_nHashsetBlock );
	}
	m_nVerifyHash = HASH_NULL;
	m_oVerified.Delete();
	m_oInvalid.Delete();
	m_nVerifyCookie++;
	CFileFragment *pFragment;
	QWORD nLast = 0;
	if ( m_pFile && ( pFragment = m_pFile->m_oFree.GetFirst() ) )
	{
		if ( pFragment->Offset() ) AddVerificationBlocks( 0, pFragment->Offset() );
		nLast = pFragment->Next();
		if ( pFragment = pFragment->GetNext() ) do
		{
			AddVerificationBlocks( nLast, pFragment->Offset() );
			nLast = pFragment->Next();
		}
		while ( pFragment = pFragment->GetNext() );
	}
	if ( nLast < m_nSize ) AddVerificationBlocks( nLast, m_nSize );
	SetModified();
}

void CDownloadWithTiger::ClearTiger()
{
	delete [] m_pTigerVerificationQueue;
	delete [] m_pTigerVerificationCandidates;
	m_pTigerVerificationQueue = NULL;
	m_pTigerVerificationCandidates = NULL;
	m_nTigerBlock = 0;
	m_nTigerSize = 0;
	m_oTigerTree.Clear();
	ResetVerification();
	m_nVerifyCookie++;
	SetModified();
}

void CDownloadWithTiger::ClearHashset()
{
	delete [] m_pHashsetVerificationQueue;
	delete [] m_pHashsetVerificationCandidates;
	m_pHashsetVerificationQueue = NULL;
	m_pHashsetVerificationCandidates = NULL;
	m_nHashsetBlock = 0;
	m_oHashset.Clear();
	ResetVerification();
	m_nVerifyCookie++;
	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTiger serialize

void CDownloadWithTiger::Serialize(CArchive& ar, int nVersion)
{
	CDownloadWithTorrent::Serialize( ar, nVersion );
	if ( nVersion >= 32 )
	{
		if ( ar.IsLoading() )
		{
			m_oTigerTree.SerializeLoad( ar, nVersion );
			if ( m_oTigerTree.IsAvailable() )
			{
				m_oTigerTree.SetupParameters( m_nSize );
				ar >> m_nTigerBlock;
				ar >> m_nTigerSize;
				m_pTigerVerificationQueue = new DWORD[ m_nTigerBlock + 1 ];
				m_nTigerVerificationEnd = m_nTigerVerificationStart = m_nTigerBlock;
				m_pTigerVerificationCandidates = new BYTE[ m_nTigerBlock ];
				ZeroMemory( m_pTigerVerificationCandidates, sizeof(BYTE) * m_nTigerBlock );
			}
			m_oHashset.SerializeLoad( ar, nVersion );
			if ( m_oHashset.IsAvailable() )
			{
				ar >> m_nHashsetBlock;
				m_pHashsetVerificationQueue = new DWORD[ m_nHashsetBlock + 1 ];
				m_nHashsetVerificationEnd = m_nHashsetVerificationStart = m_nHashsetBlock;
				m_pHashsetVerificationCandidates = new BYTE[ m_nHashsetBlock ];
				ZeroMemory( m_pHashsetVerificationCandidates, sizeof(BYTE) * m_nHashsetBlock );
			}
			m_oVerified.Serialize( ar, nVersion );
			m_oInvalid.Serialize( ar, nVersion );
			CFileFragment *pFragment;										// find Validation Blocks
			QWORD nLast = 0;
			if ( m_pFile && ( pFragment = m_pFile->m_oFree.GetFirst() ) )
			{
				if ( pFragment->Offset() ) AddVerificationBlocks( 0, pFragment->Offset() );
				nLast = pFragment->Next();
				if ( pFragment = pFragment->GetNext() ) do
				{
					AddVerificationBlocks( nLast, pFragment->Offset() );
					nLast = pFragment->Next();
				}
				while ( pFragment = pFragment->GetNext() );
			}
			if ( nLast < m_nSize ) AddVerificationBlocks( nLast, m_nSize );
		}
		else
		{
			m_oTigerTree.SerializeStore( ar, nVersion );
			if ( m_oTigerTree.IsAvailable() )
			{
				m_oTigerTree.SetupParameters( m_nSize );
				ar << m_nTigerBlock;
				ar << m_nTigerSize;
			}
			m_oHashset.SerializeStore( ar, nVersion );
			if ( m_oHashset.IsAvailable() ) ar << m_nHashsetBlock;
			m_oVerified.Serialize( ar, nVersion );
			m_oInvalid.Serialize( ar, nVersion );
		}
		return;
	}
	ASSERT( ar.IsLoading() );
	DWORD nTigerSuccess, nHashsetSuccess;
	m_oTigerTree.SerializeLoad( ar, nVersion );
	if ( m_oTigerTree.IsAvailable() )
	{
		m_oTigerTree.SetupParameters( m_nSize );
		ar >> m_nTigerBlock;
		ar >> m_nTigerSize;
		ar >> nTigerSuccess;
		BYTE nState;
		QWORD nOffset = 0, nNext = m_nTigerSize;
		while ( nNext < m_nSize )
		{
			ar >> nState;
			if ( nState == TS_TRUE ) m_oVerified.Add( nOffset, nNext );
			else if ( nState == TS_FALSE ) m_oInvalid.Add( nOffset, nNext );
			nOffset = nNext;
			nNext += m_nTigerSize;
		}
		ar >> nState;
		if ( nState == TS_TRUE ) m_oVerified.Add( nOffset, m_nSize );
		else if ( nState == TS_FALSE ) m_oInvalid.Add( nOffset, m_nSize );
		m_pTigerVerificationQueue = new DWORD[ m_nTigerBlock + 1 ];
		m_nTigerVerificationEnd = m_nTigerVerificationStart = m_nTigerBlock;
		m_pTigerVerificationCandidates = new BYTE[ m_nTigerBlock ];
		ZeroMemory( m_pTigerVerificationCandidates, sizeof(BYTE) * m_nTigerBlock );
	}
	if ( nVersion >= 19 )
	{
		m_oHashset.SerializeLoad( ar, nVersion );
		if ( m_oHashset.IsAvailable() )
		{
			ar >> m_nHashsetBlock;
			ar >> nHashsetSuccess;
			BYTE nState;
			QWORD nOffset = 0, nNext = ED2K_PART_SIZE;
			while ( nNext < m_nSize )
			{
				ar >> nState;
				if ( nState == TS_TRUE ) m_oVerified.Add( nOffset, nNext );
				else if ( nState == TS_FALSE ) m_oInvalid.Add( nOffset, nNext );
				nOffset = nNext;
				nNext += ED2K_PART_SIZE;
			}
			ar >> nState;
			if ( nState == TS_TRUE ) m_oVerified.Add( nOffset, m_nSize );
			else if ( nState == TS_FALSE ) m_oInvalid.Add( nOffset, m_nSize );
			m_pHashsetVerificationQueue = new DWORD[ m_nHashsetBlock + 1 ];
			m_nHashsetVerificationEnd = m_nHashsetVerificationStart = m_nHashsetBlock;
			m_pHashsetVerificationCandidates = new BYTE[ m_nHashsetBlock ];
			ZeroMemory( m_pHashsetVerificationCandidates, sizeof(BYTE) * m_nHashsetBlock );
		}
	}
	if ( nVersion < 30 && m_oBTH.IsValid() )
	{
		m_oED2K.Clear();
		m_oSHA1.Clear();
		m_oTiger.Clear();
	}
	m_oInvalid.Subtract( m_oVerified );
	CFileFragment *pFragment;
	QWORD nLast = 0;
	if ( m_pFile && ( pFragment = m_pFile->m_oFree.GetFirst() ) )
	{
		if ( pFragment->Offset() ) AddVerificationBlocks( 0, pFragment->Offset() );
		nLast = pFragment->Next();
		if ( pFragment = pFragment->GetNext() ) do
		{
			AddVerificationBlocks( nLast, pFragment->Offset() );
			nLast = pFragment->Next();
		}
		while ( pFragment = pFragment->GetNext() );
	}
	if ( nLast < m_nSize ) AddVerificationBlocks( nLast, m_nSize );
}