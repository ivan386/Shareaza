//
// DownloadWithTorrent.cpp
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
#include "Network.h"
#include "BTPacket.h"
#include "BTClient.h"
#include "BTClients.h"
#include "Download.h"
#include "DownloadTask.h"
#include "DownloadSource.h"
#include "DownloadWithTorrent.h"
#include "DownloadTransferBT.h"
#include "UploadTransferBT.h"
#include "BTTrackerRequest.h"
#include "FragmentedFile.h"
#include "Buffer.h"
#include "SHA.h"
#include "LibraryFolders.h"
#include "GProfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent construction

CDownloadWithTorrent::CDownloadWithTorrent()
{
	m_bTorrentRequested		= FALSE;
	m_bTorrentStarted		= FALSE;
	m_tTorrentTracker		= 0;
	m_nTorrentUploaded		= 0;
	m_nTorrentDownloaded	= 0;
	m_bTorrentEndgame		= FALSE;
	m_bTorrentTrackerError	= FALSE;
	
	m_nTorrentBlock			= 0;
	m_nTorrentSize			= 0;
	m_pBTHVerificationQueue = NULL;
	m_pBTHVerificationCandidates = NULL;
	m_bSeeding				= FALSE;
	
	m_tTorrentChoke			= 0;
	m_tTorrentSources		= 0;
	ZeroMemory( &m_oPeerID, 20);
}

CDownloadWithTorrent::~CDownloadWithTorrent()
{
	if ( m_bTorrentRequested ) CBTTrackerRequest::SendStopped( this );
	CloseTorrentUploads();
	delete [] m_pBTHVerificationQueue;
	delete [] m_pBTHVerificationCandidates;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent serialize

void CDownloadWithTorrent::Serialize(CArchive& ar, int nVersion)
{
	DWORD nTorrentSuccess = 0;
	CDownloadWithFile::Serialize( ar, nVersion );
	if ( nVersion < 22 ) return;
	m_pTorrent.Serialize( ar );
	if ( nVersion >= 32 )
	{
		m_oBTH = m_pTorrent.m_oInfoBTH;
		m_oBTH.SetTrusted();
		if ( ar.IsLoading() )
		{
			m_nTorrentSize = m_pTorrent.m_nBlockSize;
			m_nTorrentBlock	= m_pTorrent.m_nBlockCount;
			if ( m_pTorrent.IsAvailable() )
			{
				m_oBTH = m_pTorrent.m_oInfoBTH;
				m_pBTHVerificationQueue = new DWORD[ m_nTorrentBlock + 1 ];
				m_nBTHVerificationEnd = m_nBTHVerificationStart = m_nTorrentBlock;
				m_pBTHVerificationCandidates = new BYTE[ m_nTorrentBlock ];
				ZeroMemory( m_pBTHVerificationCandidates, sizeof(BYTE) * m_nTorrentBlock );
			}
		}
		return;
	}
	ASSERT( ar.IsLoading() );
	if ( m_pTorrent.IsAvailable() )
	{
		m_oBTH = m_pTorrent.m_oInfoBTH;
		m_oBTH.SetTrusted();
		if ( nVersion >= 23 )
		{
			m_nTorrentSize	= m_pTorrent.m_nBlockSize;
			m_nTorrentBlock	= m_pTorrent.m_nBlockCount;
			ar >> nTorrentSuccess;
			BYTE *pTorrentBlock = new BYTE[ m_nTorrentBlock ];
			ar.Read( pTorrentBlock, sizeof(BYTE) * m_nTorrentBlock );
			BYTE nState;
			DWORD i = 0;
			QWORD nOffset = 0, nNext = m_nTorrentSize;
			while ( nNext < m_nSize )
			{
				nState = pTorrentBlock[ i++ ];
				if ( nState == TS_TRUE ) m_oVerified.Add( nOffset, nNext );
				else if ( nState == TS_FALSE ) m_oInvalid.Add( nOffset, nNext );
				nOffset = nNext;
				nNext += m_nTorrentSize;
			}
			nState = pTorrentBlock[ i++ ];
			if ( nState == TS_TRUE ) m_oVerified.Add( nOffset, m_nSize );
			else if ( nState == TS_FALSE ) m_oInvalid.Add( nOffset, m_nSize );
			ASSERT( i == m_nTorrentBlock );
			delete [] pTorrentBlock;
			m_pBTHVerificationQueue = new DWORD[ m_nTorrentBlock + 1 ];
			m_nBTHVerificationEnd = m_nBTHVerificationStart = m_nTorrentBlock;
			m_pBTHVerificationCandidates = new BYTE[ m_nTorrentBlock ];
			ZeroMemory( m_pBTHVerificationCandidates, sizeof(BYTE) * m_nTorrentBlock );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent set torrent

BOOL CDownloadWithTorrent::SetTorrent(CBTInfo* pTorrent)
{
	if ( pTorrent == NULL ) return FALSE;
	if ( m_pTorrent.IsAvailable() ) return FALSE;
	if ( ! pTorrent->IsAvailable() ) return FALSE;
	
	m_pTorrent.Copy( pTorrent );

	if ( m_oBTH.IsTrusted() )
	{
        m_oBTH = m_pTorrent.m_oInfoBTH;
		m_oBTH.SetTrusted();
	}
	else
	{
        m_oBTH = m_pTorrent.m_oInfoBTH;
	}
	
	m_nTorrentSize	= m_pTorrent.m_nBlockSize;
	m_nTorrentBlock	= m_pTorrent.m_nBlockCount;
	m_pBTHVerificationQueue = new DWORD[ m_nTorrentBlock + 1 ];
	m_nBTHVerificationEnd = m_nBTHVerificationStart = m_nTorrentBlock;
	m_pBTHVerificationCandidates = new BYTE[ m_nTorrentBlock ];
	ZeroMemory( m_pBTHVerificationCandidates, sizeof(BYTE) * m_nTorrentBlock );
	SetModified();
	
	CreateDirectory( Settings.Downloads.TorrentPath, NULL );//Create/set up torrents folder
	LibraryFolders.AddFolder( Settings.Downloads.TorrentPath, FALSE );
	pTorrent->SaveTorrentFile( Settings.Downloads.TorrentPath );
	Settings.BitTorrent.AdvancedInterface = TRUE;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent run

BOOL CDownloadWithTorrent::RunTorrent(DWORD tNow)
{
	if ( ! m_pTorrent.IsAvailable() ) return TRUE;
	if ( m_bDiskFull ) return FALSE;
	
	if ( tNow > m_tTorrentChoke && tNow - m_tTorrentChoke >= 10000 ) ChokeTorrent( tNow );
	
	if ( m_pFile != NULL && m_pFile->IsOpen() == FALSE )
	{
		BOOL bCreated = ( m_sLocalName.IsEmpty() ||
						GetFileAttributes( m_sLocalName ) == 0xFFFFFFFF );
		
		if ( ! PrepareFile() ) return FALSE;
		
		ASSERT( m_pTask == NULL );
		if ( bCreated ) m_pTask = new CDownloadTask( (CDownload*)this, CDownloadTask::dtaskAllocate );
	}
	
	if ( m_pTask != NULL ) return FALSE;
	
	BOOL bLive = ( ! IsPaused() ) && ( Network.IsConnected() );
	
	if ( bLive && ! m_bTorrentStarted )
	{
		if ( ! m_bTorrentRequested || tNow > m_tTorrentTracker )
		{
			GenerateTorrentDownloadID();

			m_bTorrentRequested	= TRUE;
			m_bTorrentStarted	= FALSE;
			m_tTorrentTracker	= tNow + Settings.BitTorrent.DefaultTrackerPeriod;
			
			CBTTrackerRequest::SendStarted( this );
		}
	}
	else if ( ! bLive && m_bTorrentRequested )
	{
		CBTTrackerRequest::SendStopped( this );
		
		m_bTorrentRequested = m_bTorrentStarted = FALSE;
		m_tTorrentTracker = 0;
		ZeroMemory( &m_oPeerID, 20);
	}
	
	if ( m_bTorrentStarted && tNow > m_tTorrentTracker )
	{
		m_tTorrentTracker = tNow + Settings.BitTorrent.DefaultTrackerPeriod;
		CBTTrackerRequest::SendUpdate( this );
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent GenerateTorrentDownloadID (Called 'Peer ID', but seperate for each transfer)

BOOL CDownloadWithTorrent::GenerateTorrentDownloadID()
{
	theApp.Message( MSG_DEBUG, _T("Creating Peer ID") );

	if ( ! m_oPeerID.IsEmpty() )
	{
		theApp.Message( MSG_ERROR, _T("Attempted to re-create an in-use peer ID") );
		return FALSE;
	}

	m_oPeerID = MyProfile.GUID;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent tracker event handler

void CDownloadWithTorrent::OnTrackerEvent(BOOL bSuccess, LPCTSTR pszReason)
{
	m_bTorrentTrackerError = ! bSuccess;
	m_sTorrentTrackerError.Empty();
	
	if ( pszReason != NULL )
	{
		m_sTorrentTrackerError = pszReason;
	}
	else if ( m_bTorrentTrackerError )
	{
		m_sTorrentTrackerError = _T("Unable to communicate with tracker");
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent download transfer linking

CDownloadTransferBT* CDownloadWithTorrent::CreateTorrentTransfer(CBTClient* pClient)
{
	if ( IsMoving() || IsPaused() ) return NULL;
	
	CDownloadSource* pSource = NULL;
	
	for ( pSource = GetFirstSource() ; pSource ; pSource = pSource->m_pNext )
	{
		if ( pSource->m_nProtocol == PROTOCOL_BT &&
			 memcmp( &pSource->m_oGUID, &pClient->m_oGUIDBT, 16 ) == 0 ) break;
	}
	
	if ( pSource == NULL )
	{
		pSource = new CDownloadSource( (CDownload*)this, &pClient->m_oGUIDBT,
			&pClient->m_pHost.sin_addr, htons( pClient->m_pHost.sin_port ) );
		pSource->m_bPushOnly = TRUE;
		
		if ( ! AddSourceInternal( pSource ) ) return NULL;
	}
	
	if ( pSource->m_pTransfer != NULL ) return NULL;
	
	pSource->m_pTransfer = new CDownloadTransferBT( pSource, pClient );
	
	return (CDownloadTransferBT*)pSource->m_pTransfer;
}

void CDownloadWithTorrent::OnFinishedTorrentBlock(DWORD nBlock)
{
	for ( CDownloadTransferBT* pTransfer = (CDownloadTransferBT*)GetFirstTransfer() ; pTransfer ; pTransfer = (CDownloadTransferBT*)pTransfer->m_pDlNext )
	{
		if ( pTransfer->m_nProtocol == PROTOCOL_BT )
		{
			pTransfer->SendFinishedBlock( nBlock );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent create bitfield

CBTPacket* CDownloadWithTorrent::CreateBitfieldPacket()
{
	ASSERT( m_pTorrent.IsAvailable() );
	CBTPacket* pPacket = CBTPacket::New( BT_PACKET_BITFIELD );
	BYTE nByte = 0, nBit = 0x80;
	QWORD nOffset = 0, nNext = m_nTorrentSize;
	CFileFragment *pFragment;
	if ( pFragment = m_oVerified.GetFirst() ) do
	{
		if ( pFragment->Next() < nOffset )
		{
			while ( ( pFragment = pFragment->GetNext() ) && ( pFragment->Next() < nOffset ) );
			if ( ! pFragment ) break;
		}
		if ( nOffset >= pFragment->Offset() && nNext <= pFragment->Next() ) nByte |= nBit;
		if ( ! ( nBit >>= 1 ) )
		{
			pPacket->WriteByte( nByte );
			nBit = 0x80;
			nByte = 0;
		}
		nOffset = nNext;
	}
	while ( ( nNext += m_nTorrentSize ) < m_nSize );
	if ( pFragment )
	{
		if ( nOffset >= pFragment->Offset() && m_nSize <= pFragment->Next() ) nByte |= nBit;
		pPacket->WriteByte( nByte );
	}
	else
	{
		do
		{
			nOffset += m_nTorrentSize;
		}
		while ( nBit >>= 1 );
		pPacket->WriteByte( nByte );
		QWORD nTorrentSize8 = 8 * (QWORD)m_nTorrentSize;
		while ( nOffset < m_nSize )
		{
			nOffset += nTorrentSize8;
			pPacket->WriteByte( 0 );
		}
	}
	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent upload linking

void CDownloadWithTorrent::AddUpload(CUploadTransferBT* pUpload)
{
	if ( m_pTorrentUploads.Find( pUpload ) == NULL )
		m_pTorrentUploads.AddTail( pUpload );
}

void CDownloadWithTorrent::RemoveUpload(CUploadTransferBT* pUpload)
{
	if ( POSITION pos = m_pTorrentUploads.Find( pUpload ) )
		m_pTorrentUploads.RemoveAt( pos );
}

void CDownloadWithTorrent::CloseTorrentUploads()
{
	for ( POSITION pos = m_pTorrentUploads.GetHeadPosition() ; pos ; )
	{
		CUploadTransferBT* pUpload = (CUploadTransferBT*)m_pTorrentUploads.GetNext( pos );
		pUpload->Close();
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent choking

void CDownloadWithTorrent::ChokeTorrent(DWORD tNow)
{
	BOOL bChooseRandom = TRUE;
	int nTotalRandom = 0;
	CPtrList pSelected;
	
	if ( ! tNow ) tNow = GetTickCount();
	if ( tNow > m_tTorrentChoke && tNow - m_tTorrentChoke < 2000 ) return;
	m_tTorrentChoke = tNow;
	
	for ( POSITION pos = m_pTorrentUploads.GetHeadPosition() ; pos ; )
	{
		CUploadTransferBT* pTransfer = (CUploadTransferBT*)m_pTorrentUploads.GetNext( pos );
		if ( pTransfer->m_nProtocol != PROTOCOL_BT ) continue;
		
		if ( pTransfer->m_nRandomUnchoke == 2 )
		{
			if ( tNow - pTransfer->m_tRandomUnchoke >= Settings.BitTorrent.RandomPeriod )
			{
				pTransfer->m_nRandomUnchoke = 1;
			}
			else
			{
				bChooseRandom = FALSE;
			}
		}
		
		if ( pTransfer->m_bInterested )
			nTotalRandom += ( pTransfer->m_nRandomUnchoke == 0 ) ? 3 : 1;
	}
	
	if ( bChooseRandom && nTotalRandom > 0 )
	{
		nTotalRandom = rand() % nTotalRandom;
		
		for ( pos = m_pTorrentUploads.GetHeadPosition() ; pos ; )
		{
			CUploadTransferBT* pTransfer = (CUploadTransferBT*)m_pTorrentUploads.GetNext( pos );
			if ( pTransfer->m_nProtocol != PROTOCOL_BT ) continue;
			if ( pTransfer->m_bInterested == FALSE ) continue;
			
			int nWeight = ( pTransfer->m_nRandomUnchoke == 0 ) ? 3 : 1;
			
			if ( nTotalRandom < nWeight )
			{
				pTransfer->m_nRandomUnchoke = 2;
				pTransfer->m_tRandomUnchoke = tNow;
				pSelected.AddTail( pTransfer );
				break;
			}
			else
			{
				nTotalRandom -= nWeight;
			}
		}
	}
	
	while ( pSelected.GetCount() < Settings.BitTorrent.UploadCount )
	{
		CUploadTransferBT* pBest = NULL;
		DWORD nBest = 0;
		
		for ( POSITION pos = m_pTorrentUploads.GetHeadPosition() ; pos ; )
		{
			CUploadTransferBT* pTransfer = (CUploadTransferBT*)m_pTorrentUploads.GetNext( pos );
			
			if (	pTransfer->m_nProtocol == PROTOCOL_BT &&
					pTransfer->m_bInterested &&
					pSelected.Find( pTransfer->m_pClient ) == NULL &&
					pTransfer->GetAverageSpeed() >= nBest )
			{
				pBest = pTransfer;
				nBest = pTransfer->GetAverageSpeed();
			}
		}
		
		if ( pBest == NULL ) break;
		pSelected.AddTail( pBest->m_pClient );
	}
	
	while ( pSelected.GetCount() < Settings.BitTorrent.UploadCount )
	{
		CDownloadTransferBT* pBest = NULL;
		DWORD nBest = 0;
		
		for ( CDownloadTransferBT* pTransfer = (CDownloadTransferBT*)GetFirstTransfer()
				; pTransfer ; pTransfer = (CDownloadTransferBT*)pTransfer->m_pDlNext )
		{
			if (	pTransfer->m_nProtocol == PROTOCOL_BT &&
					pSelected.Find( pTransfer->m_pClient ) == NULL &&
					pTransfer->m_nState == dtsDownloading &&
					pTransfer->m_pClient->m_pUpload->m_bInterested &&
					pTransfer->GetAverageSpeed() >= nBest )
			{
				pBest = pTransfer;
				nBest = pTransfer->GetAverageSpeed();
			}
		}
		
		if ( pBest == NULL ) break;
		pSelected.AddTail( pBest->m_pClient );
	}
	
	for ( pos = m_pTorrentUploads.GetHeadPosition() ; pos ; )
	{
		CUploadTransferBT* pTransfer = (CUploadTransferBT*)m_pTorrentUploads.GetNext( pos );
		if ( pTransfer->m_nProtocol != PROTOCOL_BT ) continue;
		
		pTransfer->SetChoke(	pTransfer->m_bInterested == TRUE &&
								pSelected.Find( pTransfer->m_pClient ) == NULL );
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent search -> tracker link

BOOL CDownloadWithTorrent::FindMoreSources()
{
	if ( m_pFile != NULL && m_bTorrentRequested )
	{
		ASSERT( m_pTorrent.IsAvailable() );
		
		if ( GetTickCount() - m_tTorrentSources > 15000 )
		{
			m_tTorrentTracker = GetTickCount() + Settings.BitTorrent.DefaultTrackerPeriod;
			m_tTorrentSources = GetTickCount();
			CBTTrackerRequest::SendUpdate( this );
			return TRUE;
		}
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent seed

BOOL CDownloadWithTorrent::SeedTorrent(LPCTSTR pszTarget)
{
	
	CDownload* pDownload = reinterpret_cast<CDownload*>(this);
	
	if ( IsMoving() || IsCompleted() ) return FALSE;
	if ( m_sLocalName == pszTarget ) return FALSE;
	
	ASSERT( m_pFile != NULL );
	if ( m_pFile == NULL ) return FALSE;
	ASSERT( m_pFile->IsOpen() == FALSE );
	if ( m_pFile->IsOpen() ) return FALSE;
	delete m_pFile;
	m_pFile = NULL;

	GenerateTorrentDownloadID();
	
	pDownload->m_bSeeding	= TRUE;
	pDownload->m_bComplete	= TRUE;
	pDownload->m_tCompleted	= GetTickCount();
	
	m_oVerified.Add( 0, m_nSize );
	
	if ( m_sLocalName.GetLength() > 0 )
	{
		ASSERT( FALSE );
		::DeleteFile( m_sLocalName );
		::DeleteFile( m_sLocalName + _T(".sd") );
	}
	
	m_sLocalName = pszTarget;
	SetModified();
	
	m_tTorrentTracker	= GetTickCount() + ( 30 * 1000 ); //Give tracker 30 seconds to respond
	m_bTorrentRequested	= TRUE;
	m_bTorrentStarted	= FALSE;
	CBTTrackerRequest::SendStarted( this );	
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent Close

void CDownloadWithTorrent::CloseTorrent()
{
	if ( m_bTorrentRequested ) CBTTrackerRequest::SendStopped( this );
	m_bTorrentRequested		= FALSE;
	m_bTorrentStarted		= FALSE;
	CloseTorrentUploads();
	ZeroMemory( &m_oPeerID, 20);
}


//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent stats

float CDownloadWithTorrent::GetRatio() const
{
	if ( m_nTorrentUploaded == 0 || m_nTorrentDownloaded == 0 ) return 0;
	return (float)m_nTorrentUploaded / (float)m_nTorrentDownloaded;
}