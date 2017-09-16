//
// UploadTransferBT.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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
#include "BTClient.h"
#include "BTPacket.h"
#include "Download.h"
#include "Downloads.h"
#include "DownloadTransferBT.h"
#include "Uploads.h"
#include "UploadFile.h"
#include "UploadFiles.h"
#include "UploadTransferBT.h"
#include "FragmentedFile.h"
#include "TransferFile.h"
#include "Statistics.h"
#include "Buffer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CUploadTransferBT construction

CUploadTransferBT::CUploadTransferBT(CBTClient* pClient, CDownload* pDownload)
	: CUploadTransfer( PROTOCOL_BT )
	, m_pClient			( pClient )
	, m_pDownload		( pDownload )
	, m_bInterested		( FALSE )
	, m_bChoked			( TRUE )
	, m_nRandomUnchoke	( 0 )
	, m_tRandomUnchoke	( 0 )
{
	ASSERT( pClient != NULL );
	ASSERT( pDownload != NULL );

	m_pHost				= pClient->m_pHost;
	m_sAddress			= pClient->m_sAddress;
	UpdateCountry();

	m_sUserAgent		= pClient->m_sUserAgent;
	m_bClientExtended	= pClient->m_bClientExtended;
	m_nState			= upsReady;

	RequestPartial( m_pDownload );
	m_pDownload->AddUpload( this );
}

CUploadTransferBT::~CUploadTransferBT()
{
	ASSERT( m_pClient == NULL );
	ASSERT( m_pDownload == NULL );
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferBT choking

void CUploadTransferBT::SetChoke(BOOL bChoke)
{
	// Sort transfers- keep active ones near the head (Top of the list in the uploads window)
	if ( ! bChoke ) UploadFiles.MoveToHead( this );

	// If we have not changed state, just return
	if ( m_bChoked == bChoke ) return;

	// Update state
	m_bChoked = bChoke;

	m_oRequested.clear();
	m_oServed.clear();

	if ( bChoke )
	{
		m_nState = upsReady;
		UploadFiles.MoveToTail( this );

		m_pClient->Choke();
	}
	else
	{
		m_pClient->UnChoke();
	}

	theApp.Message( MSG_DEBUG, _T("%s upload to %s"),
		bChoke ? _T("Choking") : _T("Unchoking"), (LPCTSTR)m_sAddress );
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferBT close

void CUploadTransferBT::Close(UINT nError)
{
	if ( m_pClient != NULL )
	{
		if ( Settings.General.DebugBTSources )
			theApp.Message( MSG_DEBUG, L"Closing BT upload connection: %s", (LPCTSTR)m_pClient->m_sAddress );
		m_pClient->m_pUploadTransfer = NULL;
		m_pClient->Close();
		m_pClient = NULL;
	}

	CloseFile();

	if ( m_pDownload != NULL ) m_pDownload->RemoveUpload( this );
	m_pDownload = NULL;

	m_oRequested.clear();
	m_oServed.clear();

	CUploadTransfer::Close( nError );
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferBT bandwidth

DWORD CUploadTransferBT::GetMeasuredSpeed()
{
	// Return if there is no client
	if ( m_pClient == NULL ) return 0;

	// Calculate Output
	m_pClient->MeasureOut();

	// Return calculated speed
	return m_pClient->m_mOutput.nMeasure;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferBT connection event

BOOL CUploadTransferBT::OnConnected()
{
	m_pClient->m_mOutput.pLimit = &Uploads.m_nTorrentSpeed;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferBT run event

BOOL CUploadTransferBT::OnRun()
{
	if ( m_nState >= upsRequest && ! m_bChoked ) return ServeRequests();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferBT interest flag

BOOL CUploadTransferBT::OnInterested(CBTPacket* /*pPacket*/)
{
	if ( m_bInterested ) return TRUE;
	m_bInterested = TRUE;
	return TRUE;
}

BOOL CUploadTransferBT::OnUninterested(CBTPacket* /*pPacket*/)
{
	if ( ! m_bInterested ) return TRUE;
	if ( m_pDownload->IsSeeding() )
	{
		Close(); // Remove from upload queue
		return FALSE; // Don't read the rest of buffer
	}
	m_bInterested = FALSE;
	m_nState = upsReady;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferBT request management

BOOL CUploadTransferBT::OnRequest(CBTPacket* pPacket)
{
	if ( pPacket->GetRemaining() < 4 * 3 ) return TRUE;
	if ( m_bChoked ) return TRUE;

	QWORD nIndex	= pPacket->ReadLongBE();
	QWORD nOffset	= pPacket->ReadLongBE();
	QWORD nLength	= pPacket->ReadLongBE();

	nOffset += nIndex * m_pDownload->m_pTorrent.m_nBlockSize;

	if ( nLength > Settings.BitTorrent.RequestLimit )
	{
		// error
		theApp.Message( MSG_DEBUG, _T("CUploadTransferBT::OnRequest(): Request size %I64i is too large"), nLength );
		Close();
		return FALSE;
	}

	if ( nOffset + nLength > m_nSize )
	{
		// error
		theApp.Message( MSG_DEBUG, _T("CUploadTransferBT::OnRequest(): Request through %I64i > %I64i"), nLength, m_nSize );
		Close();
		return FALSE;
	}

	if ( std::find_first_of( m_oRequested.begin(), m_oRequested.end(), m_oServed.begin(), m_oServed.end() )
		!= m_oRequested.end() ) return TRUE;

	m_oRequested.push_back( Fragments::Fragment( nOffset, nOffset + nLength ) );

	if ( m_nState == upsReady )
	{
		m_nState = upsRequest;

		AllocateBaseFile();

		if ( m_pBaseFile->m_nRequests++ == 0 )
		{
			theApp.Message( MSG_NOTICE, IDS_UPLOAD_FILE,
				(LPCTSTR)m_sName, (LPCTSTR)m_sAddress );

			PostMainWndMessage( WM_NOWUPLOADING, 0, (LPARAM)new CString( m_pBaseFile->m_sPath ) );
		}
	}

	return ServeRequests();
}

BOOL CUploadTransferBT::OnCancel(CBTPacket* pPacket)
{
	if ( pPacket->GetRemaining() < 4 * 3 ) return TRUE;

	QWORD nIndex	= pPacket->ReadLongBE();
	QWORD nOffset	= pPacket->ReadLongBE();
	QWORD nLength	= pPacket->ReadLongBE();

	nOffset += nIndex * m_pDownload->m_pTorrent.m_nBlockSize;

	m_oRequested.erase( Fragments::Fragment( nOffset, nOffset + nLength ) );

	if ( m_pDownload->IsSeeding() )
	{
		Close(); // Remove from upload queue
		return FALSE; // Don't read the rest of buffer
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferBT file access

BOOL CUploadTransferBT::OpenFile()
{
	ASSERT( m_nState == upsRequest || m_nState == upsUploading );
	ASSERT( m_pBaseFile != NULL );

	if ( m_pClient && Downloads.Check( m_pClient->m_pDownload ) )
	{
		// TODO: Change to CUploadTransfer::RequestPartial(CDownload* pDownload) ?
		auto_ptr< CFragmentedFile > pFile( m_pClient->m_pDownload->GetFile() );
		if ( pFile.get() )
		{
			AttachFile( pFile.release() );
			return TRUE;
		}

		// HACK: Open from disk (replace this with SeedTorrent in OnDownloadComplete)
		if ( m_pClient->m_pDownload->IsSeeding() )
		{
			auto_ptr< CFragmentedFile > pSeedingFile( new CFragmentedFile );
			if ( pSeedingFile.get() )
			{
				if ( pSeedingFile->Open( m_pClient->m_pDownload->m_pTorrent, FALSE ) )
				{
					AttachFile( pSeedingFile.release() );
					return TRUE;
				}
			}
		}
	}
	// else Something wrong...

	theApp.Message( MSG_ERROR, IDS_UPLOAD_CANTOPEN, (LPCTSTR)m_sName , (LPCTSTR)m_sAddress);

	Close();

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferBT serving

BOOL CUploadTransferBT::ServeRequests()
{
	ASSERT( m_nState == upsRequest || m_nState == upsUploading );
	ASSERT( m_pBaseFile != NULL );

	if ( m_bChoked ) return TRUE;
	if ( m_pClient->GetOutputLength() > Settings.BitTorrent.RequestSize / 3 ) return TRUE;

	while ( ! m_oRequested.empty() && ( m_nLength == SIZE_UNKNOWN || m_nLength == 0 ) )
	{
		if ( std::find( m_oServed.begin(), m_oServed.end(), *m_oRequested.begin() ) == m_oServed.end()
			// This should be redundant
			&& m_oRequested.begin()->begin() < m_nSize
			&& m_oRequested.begin()->end() <= m_nSize )
		{
			m_nOffset = m_oRequested.begin()->begin();
			m_nLength = m_oRequested.begin()->size();
			m_nPosition = 0;
		}
		m_oRequested.pop_front();
	}

	if ( m_nLength != SIZE_UNKNOWN && m_nLength != 0 && m_pDownload->m_pTorrent.m_nBlockSize != 0 )
	{
		if ( ! OpenFile() ) return FALSE;

		theApp.Message( MSG_DEBUG, IDS_UPLOAD_CONTENT,
			m_nOffset, m_nOffset + m_nLength - 1,
			(LPCTSTR)m_sName, (LPCTSTR)m_sAddress, _T("BT") );

		CBuffer pBuffer;
		pBuffer.EnsureBuffer( (size_t)m_nLength );

		QWORD nRead = 0;
		if ( ! ReadFile( m_nOffset + m_nPosition, pBuffer.m_pBuffer, m_nLength, &nRead ) )
			return FALSE;
		pBuffer.m_nLength = (DWORD)nRead;

		m_pClient->Piece(
			(DWORD)( m_nOffset / m_pDownload->m_pTorrent.m_nBlockSize ),
			(DWORD)( m_nOffset % m_pDownload->m_pTorrent.m_nBlockSize ),
			pBuffer.m_nLength, pBuffer.m_pBuffer );

		m_nPosition += m_nLength;
		m_nUploaded += m_nLength;
		m_pDownload->m_nTorrentUploaded += m_nLength;
		m_pDownload->m_pTorrent.m_nTotalUpload += m_nLength;
		Statistics.Current.Uploads.Volume += ( m_nLength / 1024 );

		m_oServed.push_back( Fragments::Fragment( m_nOffset, m_nOffset + m_nLength ) );
		m_pBaseFile->AddFragment( m_nOffset, m_nLength );

		m_nState	= upsUploading;
		m_nLength	= SIZE_UNKNOWN;
	}
	else
	{
		m_nState = upsRequest;
	}

	return TRUE;
}

