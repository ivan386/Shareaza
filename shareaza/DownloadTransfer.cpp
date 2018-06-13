//
// DownloadTransfer.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "FragmentBar.h"
#include "FragmentedFile.h"
#include "Network.h"
#include "Buffer.h"
#include "XML.h"
#include "DownloadTransferBT.h"
#include "Transfers.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadTransfer construction

CDownloadTransfer::CDownloadTransfer(CDownloadSource* pSource, PROTOCOLID nProtocol)
	: CTransfer			( nProtocol )
	, m_pDlPrev			( NULL )
	, m_pDlNext			( NULL )
	, m_nQueuePos		( 0ul )
	, m_nQueueLen		( 0ul )
	, m_nDownloaded		( 0ull )
	, m_bWantBackwards	( false )
	, m_bRecvBackwards	( false )
	, m_pDownload		( pSource->m_pDownload )
	, m_pSource			( pSource )
	, m_tSourceRequest	( 0ul )
{
	ASSUME_LOCK( Transfers.m_pSection );

	m_pDownload->AddTransfer( this );
}

CDownloadTransfer::~CDownloadTransfer()
{
	ASSUME_LOCK( Transfers.m_pSection );

	ASSERT( m_pSource == NULL );
}

void CDownloadTransfer::DrawStateBar(CDC* pDC, CRect* prcBar, COLORREF crFill, bool bTop) const
{
	CFragmentBar::DrawStateBar( pDC, prcBar, m_pDownload->m_nSize, m_nOffset, m_nLength, crFill, bTop );

	if ( m_nProtocol == PROTOCOL_ED2K || m_nProtocol == PROTOCOL_BT )
	{
		Fragments::Queue::const_iterator pItr = m_oRequested.begin();
		const Fragments::Queue::const_iterator pEnd = m_oRequested.end();

		if ( pItr != pEnd )
		{
			QWORD nOffset = pItr->begin();
			QWORD nLength = pItr->size();

			for ( ++pItr; pItr != pEnd; ++pItr )
			{
				if ( pItr->begin() == nOffset + nLength )
					nLength += pItr->size();
				else
				{
					CFragmentBar::DrawStateBar( pDC, prcBar, m_pDownload->m_nSize,
						nOffset, nLength, crFill, bTop );

					nOffset = pItr->begin();
					nLength = pItr->size();
				}
			}

			CFragmentBar::DrawStateBar( pDC, prcBar, m_pDownload->m_nSize,
									nOffset, nLength, crFill, bTop );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransfer close
// bKeepSource parameter:
// TRI_FALSE   - the source will be added to m_pFailedSources in CDownloadWithSources,
//			    removed from the sources and can be distributed in the Source Mesh as X-Nalt
// TRI_TRUE    - keeps the source and will be distributed as X-Alt
// TRI_UNKNOWN - keeps the source and will be dropped after several retries, will be
//            - added to m_pFailedSources when removed

void CDownloadTransfer::Close(TRISTATE bKeepSource, DWORD nRetryAfter)
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( bKeepSource == TRI_TRUE )
		SetState( dtsKeepSource );
	else
		SetState( dtsNull );

	CTransfer::Close();

	if ( CDownloadSource* pSource = m_pSource )
	{
		m_pSource = NULL;

		switch ( bKeepSource )
		{
		case TRI_TRUE:
			if ( pSource->m_bCloseConn && pSource->m_nGnutella )
			{
				pSource->OnResumeClosed();
			}
			else
			{
				pSource->OnFailure( TRUE, nRetryAfter );
			}
			break;

		case TRI_UNKNOWN:
			pSource->OnFailure( FALSE );
			break;

		case TRI_FALSE:
			pSource->Remove( FALSE, TRUE );
			break;
		}
	}

	if ( m_pDownload )
		m_pDownload->RemoveTransfer( this );
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransfer speed controls

void CDownloadTransfer::Boost()
{
	m_mInput.pLimit = m_mOutput.pLimit = NULL;
}

DWORD CDownloadTransfer::GetAverageSpeed()
{
	ASSUME_LOCK( Transfers.m_pSection );

	return m_AverageSpeed( m_pSource->m_nSpeed = GetMeasuredSpeed() );
}

DWORD CDownloadTransfer::GetMeasuredSpeed()
{
	// Calculate Input
	MeasureIn();

	// Return calculated speed
	return m_mInput.nMeasure;
}

CDownload* CDownloadTransfer::GetDownload() const
{
	ASSUME_LOCK( Transfers.m_pSection );
	ASSERT( Downloads.Check( m_pDownload ) );
	return m_pDownload;
}

CDownloadSource* CDownloadTransfer::GetSource() const
{
	ASSUME_LOCK( Transfers.m_pSection );
	ASSERT( ! m_pSource || GetDownload()->CheckSource( m_pSource ) );
	return m_pSource;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransfer state

CString CDownloadTransfer::GetStateText(BOOL bLong)
{
	CString str, strQ, strQueued, strOf;

	switch ( m_nState )
	{
	case dtsConnecting:
		LoadString( str, IDS_STATUS_CONNECTING );
		break;
	case dtsRequesting:
		LoadString( str, IDS_STATUS_REQUESTING );
		break;
	case dtsHeaders:
	case dtsFlushing:
		LoadString( str, IDS_STATUS_RESPONSE );
		break;
	case dtsDownloading:
		LoadString( str, IDS_STATUS_DOWNLOADING );
		break;
	case dtsTiger:
		LoadString( str, IDS_STATUS_TIGERTREE );
		break;
	case dtsHashset:
		LoadString( str, IDS_STATUS_HASHSET );
		break;
	case dtsMetadata:
		LoadString( str, IDS_STATUS_METADATA );
		break;
	case dtsBusy:
		LoadString( str, IDS_STATUS_BUSY );
		break;
	case dtsEnqueue:
		LoadString( str, IDS_STATUS_ENQUEUE );
		break;
	case dtsQueued:
		LoadString( strQ, IDS_STATUS_Q );
		LoadString( strQueued, IDS_STATUS_QUEUED );
		LoadString( strOf, IDS_GENERAL_OF );
		if ( ! bLong )
		{
			str.Format( ( m_nQueueLen ? _T("%s %i %s %i") : _T("%s #%i") ),
				(LPCTSTR)strQ, m_nQueuePos, (LPCTSTR)strOf, m_nQueueLen );
		}
		else if ( m_sQueueName.GetLength() )
		{
			str.Format( ( m_nQueueLen ? _T("%s %s %i %s %i") : _T("%s %s #%i") ),
				(LPCTSTR)strQueued, (LPCTSTR)m_sQueueName, m_nQueuePos, (LPCTSTR)strOf, m_nQueueLen );
		}
		else
		{
			str.Format( ( m_nQueueLen ? _T("%s %i %s %i") : _T("%s #%i") ),
				(LPCTSTR)strQueued, m_nQueuePos, (LPCTSTR)strOf, m_nQueueLen );
		}
		break;
	default:
		LoadString( str, IDS_STATUS_UNKNOWN );
		break;
	}

	return str;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransfer run handler

BOOL CDownloadTransfer::OnRun()
{
	return CTransfer::OnRun();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransfer state management

void CDownloadTransfer::SetState(int nState)
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( m_pSource->m_nSortOrder < ~0u )
	{
		switch (nState)
		{
		case dtsHeaders:
		case dtsRequesting:
		case dtsFlushing:
			m_nState = nState;
			return;
		}
	}

	if ( m_pDownload && m_pDownload->CheckSource( m_pSource ) )
	{
		if ( Settings.Downloads.SortSources )
		{	//Proper sort

			static BYTE StateSortOrder[14]={ 
				14  //dtsNull
				,13 //dtsKeepSource
				,12 //dtsConnecting
				,10 //dtsRequesting
				,4  //dtsHeaders
				,0  //dtsDownloading
				,4  //dtsFlushing
				,1  //dtsTiger
				,2  //dtsHashset
				,3  //dtsMetadata
				,12 //dtsBusy
				,8  //dtsEnqueue
				,6  //dtsQueued
				,9  //dtsTorrent
			};

			DWORD nOldSortOrder = m_pSource->m_nSortOrder;

			//Assemble the sort order DWORD
			m_pSource->m_nSortOrder = StateSortOrder[ min( nState, 14 ) ];          //Get state sort order

			if ( m_pSource->m_nSortOrder >= 14 )
			{	//Don't bother wasting CPU sorting 'dead' sources- Simply send to bottom.
				m_pDownload->SortSource( m_pSource, FALSE );
				m_pSource->m_nSortOrder = ~0u;
			}
			else
			{
				// All other sources should be properly sorted
				if ( nState == dtsTorrent && m_nProtocol == PROTOCOL_BT )    // Torrent states
				{
					// Choked torrents after queued, requesting = requesting, uninterested near end
					const CDownloadTransferBT* pBT =
						static_cast< const CDownloadTransferBT* >( this );
					if ( ! pBT->m_bInterested )
						m_pSource->m_nSortOrder = 11;
					else if ( pBT->m_bChoked )
						m_pSource->m_nSortOrder = 7;
					else
						m_pSource->m_nSortOrder = 10;
				}
				m_pSource->m_nSortOrder <<=  8;									//Sort by state

				if ( m_nProtocol != PROTOCOL_HTTP )
					m_pSource->m_nSortOrder += ( m_nProtocol & 0xFF );
				m_pSource->m_nSortOrder <<=  16;								//Then protocol

				if ( nState == dtsQueued )										//Then queue postion
					m_pSource->m_nSortOrder += min( m_nQueuePos, 10000lu ) & 0xFFFFlu;
				else															// or IP
					m_pSource->m_nSortOrder += ( ( m_pSource->m_pAddress.S_un.S_un_b.s_b1 << 8 ) |
												 ( m_pSource->m_pAddress.S_un.S_un_b.s_b2      ) );


				if ( nOldSortOrder != m_pSource->m_nSortOrder )
					//Do the sort
					m_pDownload->SortSource( m_pSource );
			}
		}
		else
		{	//Simple sort.
			if ( nState == dtsDownloading && m_nState != dtsDownloading )
			{	 //Downloading sources go to the top
				m_pDownload->SortSource( m_pSource, TRUE );
			}
			else if ( nState != dtsDownloading && m_nState == dtsDownloading )
			{	//Sources that have stopped downloading go to the bottom.
				m_pDownload->SortSource( m_pSource, FALSE );
			}
		}
	}

	m_nState = nState;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransfer fragment size management

bool CDownloadTransfer::ChunkifyRequest(QWORD* pnOffset, QWORD* pnLength, DWORD nChunk, BOOL bVerifyLock) const
{
	ASSUME_LOCK( Transfers.m_pSection );

	ASSERT( pnOffset != NULL && pnLength != NULL );

	if ( m_pSource->m_bCloseConn ) return m_bWantBackwards;

	nChunk = min( nChunk, Settings.Downloads.ChunkSize );

	if ( bVerifyLock )
	{
		if ( DWORD nVerify = m_pDownload->GetVerifyLength() )
		{
			nVerify = nVerify * 3 / 2;
			nChunk = max( nChunk, nVerify );
		}

		if ( Settings.Downloads.ChunkStrap > 0 && m_nDownloaded == 0 )
		{
			nChunk = Settings.Downloads.ChunkStrap;
		}
	}

	if ( nChunk == 0 || *pnLength <= nChunk ) return m_bWantBackwards;

	if ( m_pDownload->GetVolumeComplete() == 0 || *pnOffset == 0 )
	{
		*pnLength = nChunk;
	}
	else if ( m_bWantBackwards )
	{
		*pnOffset = *pnOffset + *pnLength - nChunk;
		*pnLength = nChunk;
	}
	else
	{
		QWORD nCount = *pnLength / nChunk;
		if ( *pnLength % nChunk ) nCount++;

		QWORD nNonRandomEnd = this->m_pDownload->GetNonRandomEnd();
		QWORD nStartFrom = this->m_pDownload->m_nStartFrom;
		QWORD nIndex = 0;
		QWORD nLength = *pnLength;
		QWORD nOffset = *pnOffset;

		if ( *pnOffset < nNonRandomEnd && nStartFrom < *pnOffset + *pnLength )
		{
			QWORD nStart = max( nStartFrom, *pnOffset );
			*pnLength -= nStart - *pnOffset;
			*pnOffset = nStart;
		}
		else
			nIndex = GetRandomNum( 0ui64, nCount - 1 );

		QWORD nStart = *pnOffset + nChunk * nIndex;
		*pnLength = min( (QWORD)nChunk, *pnOffset + *pnLength - nStart );
		*pnOffset = nStart;

		if ( *pnOffset - nOffset > nOffset + nLength - *pnOffset - *pnLength )
			return true;
	}
	return m_bWantBackwards;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransfer fragment selection
//
// Selects an available block, either unaligned blocks or if none is available
// a random aligned block

blockPair CDownloadTransfer::SelectBlock(const Fragments::List& oPossible, const std::vector< bool >& pAvailable, bool bEndGame) const
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( oPossible.empty() )
		return std::make_pair( 0ull, 0ull );

	Fragments::List::const_iterator pItr = oPossible.begin();
	const Fragments::List::const_iterator pEnd = oPossible.end();

	if ( bEndGame )
	{
		std::vector< blockPair > oPartials;
		for ( ; pItr != pEnd && oPartials.size() < oPartials.max_size()
			; ++pItr )
		{
			oPartials.push_back(
				std::make_pair( pItr->begin(), pItr->end() - pItr->begin() ) );
		}

		return oPartials[ GetRandomNum< size_t >( 0u, oPartials.size() - 1u ) ];
	}

	if ( pItr->begin() < Settings.Downloads.ChunkStrap )
	{
		return std::make_pair( pItr->begin(),
			min( pItr->end() - pItr->begin(), (QWORD)Settings.Downloads.ChunkStrap ) );
	}

	DWORD nBlockSize = m_pDownload->GetVerifyLength( m_nProtocol );
	if ( !nBlockSize )
		return std::make_pair( pItr->begin(), pItr->end() - pItr->begin() );

	QWORD nNonRandomEnd = this->m_pDownload->GetNonRandomEnd();
	QWORD nStartFrom = this->m_pDownload->m_nStartFrom;

	QWORD nRangeBlock = 0ull;
	QWORD nRange[3] = { 0ull, 0ull, 0ull };
	QWORD nBestRange[3] = { 0ull, 0ull, 0ull };

	Fragments::Fragment oBiggest( 0, 0, 0 ) ;

	for ( ; pItr != pEnd ; ++pItr )
	{
		QWORD nPart[2] = { pItr->begin(), 0ull };
		QWORD nBlockBegin = nPart[0] / nBlockSize;
		QWORD nBlockEnd = ( pItr->end() - 1ull ) / nBlockSize;

		if ( pItr->begin() < nNonRandomEnd && pItr->end() > nStartFrom )
		{
			
			for ( QWORD nBlock = nBlockBegin;
				  nBlock <= nBlockEnd && ( ( nBlock * nBlockSize ) < nNonRandomEnd );
				  ++nBlock )
			{
				QWORD nPos = max( pItr->begin(), nBlockSize * nBlock );

				if ( nPos >= nStartFrom && ( pAvailable.empty() || pAvailable[ nBlock ] ) )
				{
					QWORD nLength = min( pItr->end(), nBlockSize * ( nBlock + 1ull ) );
					nLength -=  nPos;
					return std::make_pair( nPos , nLength );
				}
			}
		}

		// The start of a block is complete, but part is missing
		if ( nPart[0] % nBlockSize
			&& ( nBlockBegin >= pAvailable.size() || pAvailable[ (DWORD)nBlockBegin ] ) )
		{
			nPart[1] = min( pItr->end(), nBlockSize * ( nBlockBegin + 1ull ) );
			nPart[1] -= nPart[0];
			CheckPart( nPart, nBlockBegin, nRange, nRangeBlock, nBestRange );
		}

		// The end of a block is complete, but part is missing
		if ( ( !nPart[1] || nBlockBegin != nBlockEnd )
			&& pItr->end() % nBlockSize
			&& ( nBlockEnd >= pAvailable.size() || pAvailable[ (DWORD)nBlockEnd ] ) )
		{
			nPart[0] = nBlockEnd * nBlockSize;
			nPart[1] = pItr->end() - nPart[0];
			CheckPart( nPart, nBlockEnd, nRange, nRangeBlock, nBestRange );
		}

		// This fragment contains one or more aligned empty blocks
		if ( oBiggest.size() < (*pItr).size() )
			oBiggest = (*pItr);
	}

	CheckRange( nRange, nBestRange );

	if ( !nBestRange[2] )
	{
		if ( oBiggest.size() )
		{
			QWORD nPartStart = oBiggest.begin();
			QWORD nPartEnd = oBiggest.end();

			QWORD nBlockBegin = nPartStart / nBlockSize;
			QWORD nBlockEnd = ( nPartEnd - 1ull ) / nBlockSize;
			QWORD nBlockCount = nBlockEnd - nBlockBegin + 1;
			
			for ( QWORD  nCount = 0 ; nCount < nBlockCount; nCount++ )
			{
				QWORD nBlock = GetRandomNum< size_t >( nBlockBegin, nBlockEnd );

				if ( nBlock >= pAvailable.size() || pAvailable[ (DWORD)nBlock ] )
				{
					nPartStart = max( nPartStart, nBlock * nBlockSize );

					return std::make_pair( nPartStart , min( (nPartEnd - nPartStart), nBlockSize ) );
				}
			}
		}

		Fragments::List::const_iterator pItr = oPossible.random_range();

		for ( ; pItr != pEnd ; ++pItr )
		{
			QWORD nPartStart = pItr->begin();
			QWORD nPartEnd = pItr->end();

			QWORD nBlockBegin = nPartStart / nBlockSize;
			QWORD nBlockEnd = ( nPartEnd - 1ull ) / nBlockSize;
			QWORD nBlockCount = nBlockEnd - nBlockBegin + 1;
			
			for ( QWORD  nCount = 0 ; nCount < nBlockCount; nCount++ )
			{
				QWORD nBlock = GetRandomNum< size_t >( nBlockBegin, nBlockEnd );

				if ( nBlock >= pAvailable.size() || pAvailable[ (DWORD)nBlock ] )
				{
					nPartStart = max( nPartStart, nBlock * nBlockSize );

					return std::make_pair( nPartStart , min( (nPartEnd - nPartStart), nBlockSize ) );
				}
			}
		}

		return std::make_pair( 0ull, 0ull );
	}

	return std::make_pair( nBestRange[0], nBestRange[1] );
}

void CDownloadTransfer::CheckPart(QWORD* nPart, QWORD nPartBlock,
	QWORD* nRange, QWORD& nRangeBlock, QWORD* nBestRange) const
{
	if ( nPartBlock == nRangeBlock )
	{
		if ( nPart[1] < nRange[1] || !nRange[1] )
		{
			nRange[0] = nPart[0];
			nRange[1] = nPart[1];
		}
		nRange[2] += nPart[1];
	}
	else
	{
		CheckRange( nRange, nBestRange );
		nRange[2] = nRange[1] = nPart[1];
		nRange[0] = nPart[0];
		nRangeBlock = nPartBlock;
	}
}

void CDownloadTransfer::CheckRange(QWORD* nRange, QWORD* nBestRange) const
{
	if ( nRange[2] < nBestRange[2]
		|| ( nRange[2] && !nBestRange[2] ) )
	{
		nBestRange[0] = nRange[0];
		nBestRange[1] = nRange[1];
		nBestRange[2] = nRange[2];
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransfer fragment selector

bool CDownloadTransfer::SelectFragment(const Fragments::List& oPossible,
	QWORD& nOffset, QWORD& nLength, bool bEndGame) const
{
	ASSUME_LOCK( Transfers.m_pSection );

	blockPair oSelection( SelectBlock( oPossible, m_pAvailable, bEndGame ) );

	nOffset = oSelection.first;
	nLength = oSelection.second;

	return nLength > 0ull;
}

bool CDownloadTransfer::UnrequestRange(QWORD /*nOffset*/, QWORD /*nLength*/, bool /* bSendCancel */)
{
	ASSUME_LOCK( Transfers.m_pSection );

	return false;
}
