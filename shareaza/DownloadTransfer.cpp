//
// DownloadTransfer.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "Download.h"
#include "Downloads.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "FragmentedFile.h"
#include "Network.h"
#include "Buffer.h"
#include "SHA.h"
#include "ED2K.h"
#include "XML.h"
#include "DownloadTransferBT.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadTransfer construction

CDownloadTransfer::CDownloadTransfer(CDownloadSource* pSource, PROTOCOLID nProtocol)
{
	m_nProtocol		= nProtocol;
	m_pDownload		= pSource->m_pDownload;
	m_pDlPrev		= NULL;
	m_pDlNext		= NULL;
	m_pSource		= pSource;

	m_nState		= dtsNull;

	m_nQueuePos		= 0;
	m_nQueueLen		= 0;

	m_nBandwidth	= 0;

	m_nOffset		= SIZE_UNKNOWN;
	m_nLength		= 0;
	m_nPosition		= 0;
	m_nDownloaded	= 0;

	m_bWantBackwards = m_bRecvBackwards = FALSE;

	m_pDownload->AddTransfer( this );
}

CDownloadTransfer::~CDownloadTransfer()
{
	ASSERT( m_pSource == NULL );
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
	SetState( dtsNull );

	CTransfer::Close();

	if ( m_pSource != NULL )
	{
		switch ( bKeepSource )
		{
		case TRI_TRUE:
			if ( m_pSource->m_bCloseConn && m_pSource->m_nGnutella )
			{
				m_pSource->OnResumeClosed();
			}
			else
			{
				m_pSource->OnFailure( TRUE, nRetryAfter );
			}
			break;
		case TRI_UNKNOWN:
			m_pSource->OnFailure( FALSE );
			break;
		case TRI_FALSE:
			m_pSource->Remove( FALSE, TRUE );
			break;
		}

		m_pSource = NULL;
	}

	ASSERT( m_pDownload != NULL );
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
	return GetMeasuredSpeed();
}

DWORD CDownloadTransfer::GetMeasuredSpeed()
{
	// Calculate Input
	MeasureIn();

	// Return calculated speed
	return m_mInput.nMeasure;
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
			str.Format( m_nQueueLen ? _T("%s %i %s %i") : _T("%s #%i"),
				strQ, m_nQueuePos, strOf, m_nQueueLen );
		}
		else if ( m_sQueueName.GetLength() )
		{
			str.Format( _T("%s: %s: %i %s %i"), strQueued,
				(LPCTSTR)m_sQueueName, m_nQueuePos, strOf, m_nQueueLen );
		}
		else
		{
			str.Format( m_nQueueLen ? _T("%s: %i %s %i") : _T("%s: #%i"), strQueued,
				m_nQueuePos, strOf, m_nQueueLen );
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
	if ( m_pDownload != NULL )
	{
		if ( Settings.Downloads.SortSources )
		{	//Proper sort

			static BYTE StateSortOrder[13]={ 13 ,12 ,10 ,4 ,0 ,4 ,1 ,2 ,3 ,12 ,8 ,6 ,9};
				//dtsNull, dtsConnecting, dtsRequesting, dtsHeaders, dtsDownloading, dtsFlushing,
				//dtsTiger, dtsHashset, dtsMetadata, dtsBusy, dtsEnqueue, dtsQueued, dtsTorrent


			//Assemble the sort order DWORD
			m_pSource->m_nSortOrder = StateSortOrder[ min( nState, 13 ) ];          //Get state sort order

			if ( m_pSource->m_nSortOrder >= 13 )
			{	//Don't bother wasting CPU sorting 'dead' sources- Simply send to bottom.
				m_pDownload->SortSource( m_pSource, FALSE );
				m_pSource->m_nSortOrder = ~0u;
			}
			else
			{	//All other sources should be properly sorted

				if ( ( nState == dtsTorrent ) && ( m_pSource->m_pTransfer ) )    //Torrent states
				{       //Choked torrents after queued, requesting = requesting, uninterested near end
					CDownloadTransferBT* pBT = (CDownloadTransferBT*)m_pSource->m_pTransfer;
					if ( ! pBT->m_bInterested ) m_pSource->m_nSortOrder = 11;
					else if ( pBT->m_bChoked ) m_pSource->m_nSortOrder = 7;
					else m_pSource->m_nSortOrder = 10;
				}
				m_pSource->m_nSortOrder <<=  8;									//Sort by state

				if ( m_nProtocol != PROTOCOL_HTTP )
					m_pSource->m_nSortOrder += ( m_nProtocol & 0xFF );
				m_pSource->m_nSortOrder <<=  16;								//Then protocol

				if ( nState == dtsQueued )										//Then queue postion
					m_pSource->m_nSortOrder += ( min( m_nQueuePos, 10000u ) & 0xFFFF );
				else															// or IP
					m_pSource->m_nSortOrder += ( ( m_pSource->m_pAddress.S_un.S_un_b.s_b1 << 8 ) |
												 ( m_pSource->m_pAddress.S_un.S_un_b.s_b2      ) );

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

void CDownloadTransfer::ChunkifyRequest(QWORD* pnOffset, QWORD* pnLength, QWORD nChunk, BOOL bVerifyLock)
{
	ASSERT( pnOffset != NULL && pnLength != NULL );

	if ( m_pSource->m_bCloseConn ) return;

	nChunk = min( nChunk, Settings.Downloads.ChunkSize );

	if ( bVerifyLock )
	{
		if ( QWORD nVerify = m_pDownload->GetVerifyLength() )
		{
			nVerify = nVerify * 3 / 2;
			nChunk = max( nChunk, nVerify );
		}

		if ( Settings.Downloads.ChunkStrap > 0 && m_nDownloaded == 0 )
		{
			nChunk = Settings.Downloads.ChunkStrap;
		}
	}

	if ( nChunk == 0 || *pnLength <= nChunk ) return;

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
		nCount = rand() % nCount;

		QWORD nStart = *pnOffset + nChunk * nCount;
		*pnLength = min( nChunk, *pnOffset + *pnLength - nStart );
		*pnOffset = nStart;
	}
}
