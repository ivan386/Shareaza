//
// UploadTransferBT.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
#include <algorithm>
#include "Shareaza.h"
#include "Settings.h"
#include "BTClient.h"
#include "BTPacket.h"
#include "Download.h"
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

CUploadTransferBT::CUploadTransferBT(CBTClient* pClient, CDownload* pDownload) : CUploadTransfer( PROTOCOL_BT )
{
	ASSERT( pClient != NULL );
	ASSERT( pDownload != NULL );
	
	m_pDownload			= pDownload;
	m_pClient			= pClient;
	m_pHost				= pClient->m_pHost;
	m_sAddress			= pClient->m_sAddress;
	m_sUserAgent		= _T("BitTorrent");
	
	m_nState			= upsReady;
	m_bInterested		= FALSE;
	m_bChoked			= TRUE;
	m_nRandomUnchoke	= 0;
	
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
	if ( m_bChoked == bChoke ) return;
	m_bChoked = bChoke;
	
	m_oRequested.clear();
	m_oServed.clear();
	
	if ( bChoke )
	{
		m_nState = upsReady;
		UploadFiles.MoveToTail( this );
	}
	else
	{
		UploadFiles.MoveToHead( this );
	}
		
	
	m_pClient->Send( CBTPacket::New( bChoke ? BT_PACKET_CHOKE : BT_PACKET_UNCHOKE ) );
	
	theApp.Message( MSG_DEBUG, _T("%s upload to %s"),
		bChoke ? _T("Choking") : _T("Unchoking"), (LPCTSTR)m_sAddress );
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferBT close

void CUploadTransferBT::Close(BOOL bMessage)
{
	if ( m_pClient != NULL )
	{
		m_pClient->m_pUpload = NULL;
		m_pClient->Close();
		m_pClient = NULL;
	}
	
	if ( m_pDiskFile != NULL ) CloseFile();
	
	if ( m_pDownload != NULL ) m_pDownload->RemoveUpload( this );
	m_pDownload = NULL;
	
	m_oRequested.clear();
	m_oServed.clear();
	
	CUploadTransfer::Close( bMessage );
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferBT bandwidth

DWORD CUploadTransferBT::GetMeasuredSpeed()
{
	if ( m_pClient == NULL ) return 0;
	m_pClient->Measure();
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

BOOL CUploadTransferBT::OnInterested(CBTPacket* pPacket)
{
	if ( m_bInterested ) return TRUE;
	m_bInterested = TRUE;
	return TRUE;
}

BOOL CUploadTransferBT::OnUninterested(CBTPacket* pPacket)
{
	if ( ! m_bInterested ) return TRUE;
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
	
	if ( nOffset + nLength > m_nFileSize )
	{
		// error
		theApp.Message( MSG_DEBUG, _T("CUploadTransferBT::OnRequest(): Request through %I64i > %I64i"), nLength, m_nFileSize );
		Close();
		return FALSE;
	}
	
    if ( ::std::find_first_of( m_oRequested.begin(), m_oRequested.end(), m_oServed.begin(), m_oServed.end() )
        != m_oRequested.end() ) return TRUE;
	
    m_oRequested.pushBack( FF::SimpleFragment( nOffset, nOffset + nLength ) );
	
	if ( m_nState == upsReady )
	{
		m_nState = upsRequest;
		AllocateBaseFile();
		theApp.Message( MSG_SYSTEM, IDS_UPLOAD_FILE,
			(LPCTSTR)m_sFileName, (LPCTSTR)m_sAddress );
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
	
    m_oRequested.erase( FF::SimpleFragment( nOffset, nOffset + nLength ) );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferBT file access

BOOL CUploadTransferBT::OpenFile()
{
	ASSERT( m_nState == upsRequest || m_nState == upsUploading );
	ASSERT( m_pBaseFile != NULL );
	
	if ( m_pDiskFile != NULL ) return TRUE;
	m_pDiskFile = TransferFiles.Open( m_sFilePath, FALSE, FALSE );
	if ( m_pDiskFile != NULL ) return TRUE;
	
	theApp.Message( MSG_ERROR, IDS_UPLOAD_CANTOPEN, (LPCTSTR)m_sAddress, (LPCTSTR)m_sFileName );
	
	Close();
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferBT serving

BOOL CUploadTransferBT::ServeRequests()
{
	ASSERT( m_nState == upsRequest || m_nState == upsUploading );
	ASSERT( m_pBaseFile != NULL );
	ASSERT( m_nLength == SIZE_UNKNOWN );
	
	if ( m_bChoked ) return TRUE;
	if ( m_pClient->m_pOutput->m_nLength > Settings.BitTorrent.RequestSize / 3 ) return TRUE;
	
	while ( !m_oRequested.empty() && m_nLength == SIZE_UNKNOWN )
	{
        if ( ::std::find( m_oServed.begin(), m_oServed.end(), *m_oRequested.begin() )
            == m_oServed.end()
            // This should be redundant (Camper)
            && m_oRequested.begin()->begin() < m_nFileSize
            && m_oRequested.begin()->end() <= m_nFileSize )
        {
            m_nOffset = m_oRequested.begin()->begin();
            m_nLength = m_oRequested.begin()->length();
            m_nPosition = 0;
        }
        m_oRequested.popFront();
	}
	
	if ( m_nLength < SIZE_UNKNOWN )
	{
		if ( ! OpenFile() ) return FALSE;
		
		theApp.Message( MSG_DEBUG, IDS_UPLOAD_CONTENT,
			m_nOffset, m_nOffset + m_nLength - 1,
			(LPCTSTR)m_sFileName, (LPCTSTR)m_sAddress, _T("BT") );
		
		CBuffer* pBuffer = m_pClient->m_pOutput;
		pBuffer->EnsureBuffer( sizeof(BT_PIECE_HEADER) + (DWORD)m_nLength );
		
		BT_PIECE_HEADER* pHeader = (BT_PIECE_HEADER*)( pBuffer->m_pBuffer + pBuffer->m_nLength );
		
		if ( ! m_pDiskFile->Read( m_nOffset + m_nPosition, &pHeader[1], m_nLength, &m_nLength ) ) return FALSE;

		pHeader->nLength	= SWAP_LONG( 1 + 8 + (DWORD)m_nLength );
		pHeader->nType		= BT_PACKET_PIECE;
		pHeader->nPiece		= (DWORD)( m_nOffset / m_pDownload->m_pTorrent.m_nBlockSize );
		pHeader->nOffset	= (DWORD)( m_nOffset % m_pDownload->m_pTorrent.m_nBlockSize );
		pHeader->nPiece		= SWAP_LONG( pHeader->nPiece );
		pHeader->nOffset	= SWAP_LONG( pHeader->nOffset );
		
		pBuffer->m_nLength += sizeof(BT_PIECE_HEADER) + (DWORD)m_nLength;
		m_pClient->Send( NULL );
		
		m_nPosition += m_nLength;
		m_nUploaded += m_nLength;
		m_pDownload->m_nTorrentUploaded += m_nLength;
		Statistics.Current.Uploads.Volume += ( m_nLength / 1024 );
		
        m_oServed.pushBack( FF::SimpleFragment( m_nOffset, m_nOffset + m_nLength ) );
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

