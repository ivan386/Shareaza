//
// UploadTransfer.cpp
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
#include "Uploads.h"
#include "UploadFile.h"
#include "UploadFiles.h"
#include "UploadQueue.h"
#include "UploadQueues.h"
#include "TransferFile.h"
#include "UploadTransfer.h"
#include "UploadTransferHTTP.h"

#include "SharedFile.h"
#include "Download.h"

#include "TigerTree.h"
#include "SHA.h"
#include "ED2K.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CUploadTransfer construction

CUploadTransfer::CUploadTransfer(PROTOCOLID nProtocol)
{
	ClearRequest();
	
	m_nProtocol		= nProtocol;
	m_nState		= upsNull;
	m_pQueue		= NULL;
	m_pBaseFile		= NULL;
	m_pDiskFile		= NULL;
	m_nBandwidth	= Settings.Bandwidth.Request;
	
	m_bLive			= TRUE;
	m_nRequests		= 0;
	m_nUploaded		= 0;
	
	m_tRotateTime	= 0;
	m_tAverageTime	= 0;
	m_nAveragePos	= 0;
	ZeroMemory( m_nAverageRate, sizeof(m_nAverageRate) );
	
	Uploads.Add( this );
}

CUploadTransfer::~CUploadTransfer()
{
	Close( FALSE );
	UploadFiles.Remove( this );
	Uploads.Remove( this );
}

//////////////////////////////////////////////////////////////////////
// CUploadTransfer remove record

void CUploadTransfer::Remove(BOOL bMessage)
{
	ASSERT( this != NULL );
	
	if ( bMessage && m_sFileName.GetLength() > 0 )
	{
		theApp.Message( MSG_SYSTEM, IDS_UPLOAD_REMOVE,
			(LPCTSTR)m_sFileName, (LPCTSTR)m_sAddress );
	}
	
	m_nUploaded = 1;
	Close( FALSE );
	
	delete this;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransfer close connection

void CUploadTransfer::Close(BOOL bMessage)
{
	if ( m_nState == upsNull ) return;
	m_nState = upsNull;
	
	CTransfer::Close();
	UploadQueues.Dequeue( this );
	CloseFile();
	
	if ( bMessage ) theApp.Message( MSG_SYSTEM, IDS_UPLOAD_DROPPED, (LPCTSTR)m_sAddress );
	if ( m_nUploaded == 0 ) Remove( FALSE );
}

//////////////////////////////////////////////////////////////////////
// CUploadTransfer promotion

BOOL CUploadTransfer::Promote()
{
	if ( m_nState != upsQueued ) return FALSE;
	UploadQueues.Dequeue( this );
	return UploadQueues.Enqueue( this, true );
}

//////////////////////////////////////////////////////////////////////
// CUploadTransfer rename handler

BOOL CUploadTransfer::OnRename(LPCTSTR pszSource, LPCTSTR pszTarget)
{
	if ( m_nState != upsUploading || _tcsicmp( m_sFilePath, pszSource ) ) return FALSE;
	
	if ( pszTarget == NULL )
	{
		theApp.Message( MSG_ERROR, IDS_UPLOAD_DELETED, (LPCTSTR)m_sFileName, (LPCTSTR)m_sAddress );
		Close();
		return TRUE;
	}
	
	if ( pszTarget == (LPCTSTR)1 )
	{
		if ( m_pDiskFile != NULL )
		{
			m_pDiskFile->Release( FALSE );
			m_pDiskFile = NULL;
		}
	}
	else if ( m_pDiskFile == NULL )
	{
		m_sFilePath = pszTarget;
		m_pDiskFile = TransferFiles.Open( m_sFilePath, FALSE, FALSE );
		
		if ( m_pDiskFile == NULL )
		{
			theApp.Message( MSG_ERROR, IDS_UPLOAD_DELETED, (LPCTSTR)m_sFileName, (LPCTSTR)m_sAddress );
			Close();
		}
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransfer statistics

float CUploadTransfer::GetProgress()
{
	if ( m_nState != upsUploading || m_nLength == 0 || m_nLength == SIZE_UNKNOWN ) return 0;
	return (float)m_nPosition / (float)m_nLength;
}

DWORD CUploadTransfer::GetAverageSpeed()
{
	if ( m_nState != upsUploading || m_nLength == 0 || m_nLength == SIZE_UNKNOWN ) return GetMeasuredSpeed();
	DWORD nTime = ( GetTickCount() - m_tContent ) / 1000;
	return nTime ? (DWORD)( m_nPosition / nTime ) : 0;
}

DWORD CUploadTransfer::GetMeasuredSpeed()
{
	Measure();
	return m_mOutput.nMeasure;
}

void CUploadTransfer::SetSpeedLimit(DWORD nLimit)
{
	ZeroMemory( m_nAverageRate, sizeof(DWORD) * ULA_SLOTS );
	m_nBandwidth	= nLimit;
	m_tAverageTime	= 0;
	m_nAveragePos	= 0;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransfer run handler

BOOL CUploadTransfer::OnRun()
{
	DWORD tNow = GetTickCount();
	LongTermAverage( tNow );
	RotatingQueue( tNow );
	return CTransfer::OnRun();
}

//////////////////////////////////////////////////////////////////////
// CUploadTransfer read and write handlers

BOOL CUploadTransfer::OnRead()
{
	DWORD tLastRead = m_mInput.tLast;
	
	if ( ! CTransfer::OnRead() ) return FALSE;
	
	if ( m_mInput.tLast != tLastRead && ! m_bLive )
	{
		m_bLive = TRUE;
		Uploads.EnforcePerHostLimit( this );
	}
	
	return TRUE;
}

BOOL CUploadTransfer::OnWrite()
{
	DWORD tLastSend = m_mOutput.tLast;
	
	if ( ! CTransfer::OnWrite() ) return FALSE;
	
	if ( m_mOutput.tLast != tLastSend && ! m_bLive )
	{
		m_bLive = TRUE;
		Uploads.EnforcePerHostLimit( this );
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransfer long term averaging

void CUploadTransfer::LongTermAverage(DWORD tNow)
{
	if ( m_nState != upsUploading || m_nLength == 0 || m_nLength == SIZE_UNKNOWN ) return;
	
	DWORD nSpeed = GetMeasuredSpeed();
	
	if ( Settings.Live.BandwidthScale < 100 )
	{
		nSpeed = nSpeed * 100 / max( 1, Settings.Live.BandwidthScale );
	}
	
	m_nAverageRate[ m_nAveragePos ] = max( m_nAverageRate[ m_nAveragePos ], nSpeed );
	
	if ( tNow - m_tAverageTime < 2000 || m_nAverageRate[ m_nAveragePos ] == 0 ) return;
	
	m_tAverageTime = tNow;
	m_nAveragePos = ( m_nAveragePos + 1 ) % ULA_SLOTS;
	
	DWORD nAverage = 0;
	
	for ( int nPos = 0 ; nPos < ULA_SLOTS ; nPos++ )
	{
		if ( m_nAverageRate[ nPos ] == 0 ) return;
		nAverage += m_nAverageRate[ nPos ];
	}
	
	m_nAverageRate[ m_nAveragePos ] = 0;
/*	nAverage = nAverage / ULA_SLOTS * 9 / 8;
	nAverage = max( nAverage, Settings.Uploads.ClampdownFloor );
	
	if ( nAverage < m_nBandwidth * ( 100 - Settings.Uploads.ClampdownFactor ) / 100 )
	{
		DWORD nOld = m_nBandwidth;	// Save
		
		m_nBandwidth = min( nAverage, m_nBandwidth );
		
		theApp.Message( MSG_DEBUG, _T("Changing upload throttle on %s from %s to %s"),
			(LPCTSTR)m_sAddress,
			(LPCTSTR)Settings.SmartVolume( nOld * 8, FALSE, TRUE ),
			(LPCTSTR)Settings.SmartVolume( m_nBandwidth * 8, FALSE, TRUE ) );
	}*/
}

//////////////////////////////////////////////////////////////////////
// CUploadTransfer rotating queue

void CUploadTransfer::RotatingQueue(DWORD tNow)
{
	CSingleLock pLock( &UploadQueues.m_pSection, TRUE );
	
	if ( m_pQueue != NULL && UploadQueues.Check( m_pQueue ) &&
		 m_pQueue->m_bRotate && m_pQueue->IsActive( this ) )
	{
		if ( m_tRotateTime == 0 )
		{
			if ( m_nState == upsUploading ) m_tRotateTime = tNow;
		}
		else if ( tNow - m_tRotateTime >= m_pQueue->m_nRotateTime * 1000 )
		{
			m_tRotateTime = 0;
			
			CUploadQueue* pQueue = m_pQueue;
			pQueue->Dequeue( this );
			pQueue->Enqueue( this, TRUE, FALSE );
			
			pLock.Unlock();
			OnQueueKick();
		}
	}
	else
	{
		m_tRotateTime = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadTransfer hash utilities

void CUploadTransfer::ClearHashes()
{
	m_oSHA1.Clear();
	m_oTiger.Clear();
	m_oED2K.Clear();
}

BOOL CUploadTransfer::HashesFromURN(LPCTSTR pszURN)
{
	CHashSHA1 oSHA1;
	CHashTiger oTiger;
	CHashED2K oED2K;
	if ( oSHA1.FromURN( pszURN ) ) m_oSHA1 = oSHA1;
	if ( oTiger.FromURN( pszURN ) ) m_oTiger = oTiger;
	if ( oED2K.FromURN( pszURN ) ) m_oED2K = oED2K;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransfer request utilities

void CUploadTransfer::ClearRequest()
{
	m_sFileName.Empty();
	m_sFilePath.Empty();
	m_sFileTags.Empty();
	
	m_nFileBase		= 0;
	m_nFileSize		= 0;
	m_bFilePartial	= FALSE;
	
	m_nOffset		= 0;
	m_nLength		= SIZE_UNKNOWN;
	m_nPosition		= 0;
	m_nRequests ++;
	
	ClearHashes();
	ClearHeaders();
}

BOOL CUploadTransfer::RequestComplete(CLibraryFile* pFile)
{
	ASSERT( pFile != NULL );
	
	if ( ( m_oSHA1 != pFile->m_oSHA1 ) || ( m_oTiger != pFile->m_oTiger )
		|| ( m_oED2K != pFile->m_oED2K ) ) return FALSE;
	
	m_sFileName	= pFile->m_sName;
	m_sFilePath	= pFile->GetPath();
	m_nFileBase	= pFile->m_nVirtualSize > 0 ? pFile->m_nVirtualBase : 0;
	m_nFileSize	= pFile->m_nVirtualSize > 0 ? pFile->m_nVirtualSize : pFile->m_nSize;
	m_sFileTags	= pFile->m_sShareTags;
	m_bFilePartial = FALSE;
	
	m_oSHA1 = pFile->m_oSHA1;
	m_oTiger = pFile->m_oTiger;
	m_oED2K = pFile->m_oED2K;
	
	return TRUE;
}

BOOL CUploadTransfer::RequestPartial(CDownload* pFile)
{
	ASSERT( pFile != NULL );
	
	if ( ( m_oSHA1 != pFile->m_oSHA1 ) || ( m_oTiger != pFile->m_oTiger )
		|| ( m_oED2K != pFile->m_oED2K ) ) return FALSE;
	
	m_sFileName	= pFile->m_sRemoteName;
	m_sFilePath	= pFile->m_sLocalName;
	m_nFileBase	= 0;
	m_nFileSize	= pFile->m_nSize;
	m_bFilePartial = TRUE;
	m_sFileTags.Empty();
	
	if ( m_oSHA1.IsValid() && ! pFile->m_oSHA1.IsValid() )
	{
		pFile->m_oSHA1 = m_oSHA1;
	}
	else
	{
		m_oSHA1 = pFile->m_oSHA1;
	}
	
	if ( m_oTiger.IsValid() && ! pFile->m_oTiger.IsValid() )
	{
		pFile->m_oTiger = m_oTiger;
	}
	else
	{
		m_oTiger = pFile->m_oTiger;
	}
	
	if ( m_oED2K.IsValid() && ! pFile->m_oED2K.IsValid() )
	{
		pFile->m_oED2K = m_oED2K;
	}
	else
	{
		m_oED2K = pFile->m_oED2K;
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransfer file utilities

void CUploadTransfer::StartSending(int nState)
{
	m_nState	= nState;
	m_nPosition	= 0;
	m_tContent	= m_mOutput.tLast = GetTickCount();
	CTransfer::OnWrite();
}

void CUploadTransfer::AllocateBaseFile()
{
	m_pBaseFile = UploadFiles.GetFile( this, m_oSHA1, m_sFileName, m_sFilePath, m_nFileSize );
}

void CUploadTransfer::CloseFile()
{
	if ( m_pDiskFile != NULL )
	{
		m_pDiskFile->Release( FALSE );
		m_pDiskFile = NULL;
	}
}
