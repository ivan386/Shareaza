//
// UploadTransfer.cpp
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
#include "Transfers.h"
#include "Uploads.h"
#include "UploadFile.h"
#include "UploadFiles.h"
#include "UploadQueue.h"
#include "UploadQueues.h"
#include "UploadTransfer.h"
#include "UploadTransferHTTP.h"

#include "SharedFile.h"
#include "Download.h"
#include "Downloads.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CUploadTransfer construction

CUploadTransfer::CUploadTransfer(PROTOCOLID nProtocol)
	: CTransfer			( nProtocol )
	, m_pQueue			( NULL )
	, m_pBaseFile		( NULL )
	, m_nUserRating		( urNew )
	, m_nFileBase		( 0 )
	, m_bFilePartial	( FALSE )
	, m_bLive			( TRUE )
	, m_nRequests		( 0 )
	, m_nUploaded		( 0 )
	, m_tContent		( 0 )
	, m_bStopTransfer	( FALSE )
	, m_tRotateTime		( 0 )
	, m_tAverageTime	( 0 )
	, m_nAveragePos		( 0 )
	, m_tRatingTime		( 0 )
	, m_nMaxRate		( 0 )
{
	m_nBandwidth		= Settings.Bandwidth.Request;
	m_nOffset			= 0; // ?
	ZeroMemory( m_nAverageRate, sizeof( m_nAverageRate ) );

	Uploads.Add( this );
}

CUploadTransfer::~CUploadTransfer()
{
	Close();
	UploadFiles.Remove( this );
	Uploads.Remove( this );
}

//////////////////////////////////////////////////////////////////////
// CUploadTransfer remove record

void CUploadTransfer::Remove(BOOL bMessage, UINT nError)
{
	ASSERT( this != NULL );

	if ( bMessage && m_sName.GetLength() > 0 )
	{
		theApp.Message( MSG_NOTICE, IDS_UPLOAD_REMOVE,
			(LPCTSTR)m_sName, (LPCTSTR)m_sAddress );
	}

	m_nUploaded = 1;
	Close( nError );

	delete this;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransfer close connection

void CUploadTransfer::Close(UINT nError)
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( m_nState == upsNull ) return;
	m_nState = upsNull;

	CTransfer::Close( nError );
	UploadQueues.Dequeue( this );
	CloseFile();

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
	if ( ! m_pFile.get() || ! m_pFile->FindByPath( pszSource ) )
		// Unknown name
		return FALSE;

	if ( pszTarget == NULL )
	{
		// Dequeue, close transfer and release a file
		theApp.Message( MSG_ERROR, IDS_UPLOAD_DELETED, (LPCTSTR)m_sName, (LPCTSTR)m_sAddress );
		Close();
		return TRUE;
	}
	else if ( pszTarget == (LPCTSTR)1 )
	{
		// Just release a file
		CloseFile();
		return TRUE;
	}
	else
	{
		// Reopen file
		m_sPath = pszTarget;
		if ( OpenFile() )
		{
			// Successfully reopened
			return TRUE;
		}

		theApp.Message( MSG_ERROR, IDS_UPLOAD_DELETED, (LPCTSTR)m_sName, (LPCTSTR)m_sAddress );
		Close();
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadTransfer statistics

float CUploadTransfer::GetProgress() const
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

DWORD CUploadTransfer::GetMaxSpeed() const
{
	return m_nMaxRate;
}

DWORD CUploadTransfer::GetMeasuredSpeed()
{
	// Calculate Output
	MeasureOut();

	// Return calculated speed
	return m_mOutput.nMeasure;
}

void CUploadTransfer::SetSpeedLimit(DWORD nLimit)
{
	ZeroMemory( m_nAverageRate, sizeof m_nAverageRate );
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
	CalculateRating( tNow );
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
	m_nMaxRate = max( m_nMaxRate, nSpeed );

	if ( Settings.Live.BandwidthScale < 100 )
	{
		nSpeed = nSpeed * 100 / max( 1ul, Settings.Live.BandwidthScale );
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
	nAverage = nAverage / ULA_SLOTS * 9 / 8;
	nAverage = max( nAverage, Settings.Uploads.ClampdownFloor );

	if ( nAverage < m_nBandwidth * ( 100 - Settings.Uploads.ClampdownFactor ) / 100 )
	{
		DWORD nOld = m_nBandwidth;	// Save

		m_nBandwidth = min( nAverage, m_nBandwidth );

		theApp.Message( MSG_DEBUG, _T("Changing upload throttle on %s from %s to %s"),
			(LPCTSTR)m_sAddress,
			(LPCTSTR)Settings.SmartSpeed( nOld ),
			(LPCTSTR)Settings.SmartSpeed( m_nBandwidth ) );
	}
	else if ( m_pQueue && m_pQueue->GetAvailableBandwidth() )
	{
		DWORD nOld = m_nBandwidth;	// Save
		ZeroMemory( m_nAverageRate, sizeof( m_nAverageRate ) );

		DWORD nIncrease = m_pQueue->GetAvailableBandwidth() / ( m_pQueue->GetTransferCount() + 1 );
		if ( nIncrease + m_nBandwidth < m_nMaxRate )
			m_nBandwidth += nIncrease;
		else
			m_nBandwidth = m_nMaxRate;

		theApp.Message( MSG_DEBUG, _T("Changing upload throttle on %s from %s to %s"),
			(LPCTSTR)m_sAddress,
			(LPCTSTR)Settings.SmartSpeed( nOld ),
			(LPCTSTR)Settings.SmartSpeed( m_nBandwidth ) );
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadTransfer rotating queue

void CUploadTransfer::RotatingQueue(DWORD tNow)
{
	CSingleLock pLock( &UploadQueues.m_pSection );
	if ( ! pLock.Lock( 250 ) )
		return;

	if ( m_pQueue != NULL && UploadQueues.Check( m_pQueue ) &&	//Is this queue able to rotate?
		 m_pQueue->m_bRotate && m_pQueue->IsActive( this ) && ! m_bStopTransfer )
	{
		DWORD tRotationLength = m_pQueue->m_nRotateTime * 1000;

		// High ranked users can get a longer rotate time
		if ( ( m_pQueue->m_bRewardUploaders ) && ( m_nUserRating == urCredit ) )
			tRotationLength <<= 1;

		pLock.Unlock();

		if ( m_tRotateTime == 0 )									//If the upload hasn't started yet
		{
			if ( m_nState == upsUploading ) m_tRotateTime = tNow;	//Set the upload as having started
		}
		else if ( tNow - m_tRotateTime >= tRotationLength )			//Otherwise check if it should rotate
		{
			m_bStopTransfer	= TRUE;
		}
	}
	else
	{
		m_tRotateTime = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadTransfer calculate rating

void CUploadTransfer::CalculateRating(DWORD tNow)
{	//calculate a download rating for this transfer / user
	if ( tNow > m_tRatingTime + 15000 ) //Recalculate rating every 15 seconds
	{
		QWORD nDownloaded = Downloads.GetAmountDownloadedFrom( &(m_pHost.sin_addr) );
		m_tRatingTime = tNow;
		if ( nDownloaded > 128 * 1024)	//They have uploaded to us. (Transfers < 128k are ignored)
		{
			if ( nDownloaded > m_nUploaded ) //If they have sent more to us than we have to them
				m_nUserRating = urCredit;			//They get the highest rating
			else
				m_nUserRating = urSharing;			//Otherwise, #2. (still known sharer)
		}
		else							//They have not uploaded to us.
		{
			if ( m_nUploaded < 4*1024*1024 ) //If they have not gotten at least 4MB
				m_nUserRating = urNew;			//They are a new user- give uncertain rating
			else
				m_nUserRating = urNotSharing;	//Else, probably not uploading to us.
		}
	}

	//ToDo: Maybe add a 'remote client' class to retain transfer stats for an hour or so?
}

//////////////////////////////////////////////////////////////////////
// CUploadTransfer hash utilities

void CUploadTransfer::ClearHashes()
{
	m_oSHA1.clear();
	m_oTiger.clear();
	m_oED2K.clear();
	m_oBTH.clear();
	m_oMD5.clear();
}

BOOL CUploadTransfer::HashesFromURN(LPCTSTR pszURN)
{
	if ( !m_oSHA1 ) m_oSHA1.fromUrn( pszURN );
	if ( !m_oTiger ) m_oTiger.fromUrn( pszURN );
	if ( !m_oED2K ) m_oED2K.fromUrn( pszURN );
	if ( !m_oMD5 ) m_oMD5.fromUrn( pszURN );
	if ( !m_oBTH ) m_oBTH.fromUrn( pszURN );
	if ( !m_oBTH ) m_oBTH.fromUrn< Hashes::base16Encoding >( pszURN );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransfer request utilities

void CUploadTransfer::ClearRequest()
{
	m_sName.Empty();
	m_sPath.Empty();
	m_sFileTags.Empty();

	m_nFileBase		= 0;
	m_nSize			= 0;
	m_bFilePartial	= FALSE;

	m_nOffset		= SIZE_UNKNOWN;
	m_nLength		= SIZE_UNKNOWN;
	m_nPosition		= 0;
	m_nRequests ++;

	ClearHashes();
	ClearHeaders();
}

BOOL CUploadTransfer::RequestComplete(const CLibraryFile* pFile)
{
	ASSERT( pFile != NULL );

	// Check all hashes except m_oBTH
	if ( validAndUnequal( m_oSHA1, pFile->m_oSHA1 ) ) return FALSE;
	if ( validAndUnequal( m_oTiger, pFile->m_oTiger ) ) return FALSE;
	if ( validAndUnequal( m_oED2K, pFile->m_oED2K ) ) return FALSE;
	if ( validAndUnequal( m_oMD5, pFile->m_oMD5 ) ) return FALSE;

	m_sName	= pFile->m_sName;
	m_sPath	= pFile->GetPath();
	m_nFileBase	= pFile->GetBase();
	m_nSize	= pFile->GetSize();
	m_sFileTags	= pFile->m_sShareTags;
	m_bFilePartial = FALSE;

	m_oSHA1 = pFile->m_oSHA1;
	m_oTiger = pFile->m_oTiger;
	m_oED2K = pFile->m_oED2K;
	m_oBTH = pFile->m_oBTH;
	m_oMD5 = pFile->m_oMD5;

	return TRUE;
}

BOOL CUploadTransfer::RequestPartial(CDownload* pDownload)
{
	ASSERT( pDownload != NULL );

	// Check all hashes except m_oBTH
	if ( validAndUnequal( m_oSHA1, pDownload->m_oSHA1 ) ) return FALSE;
	if ( validAndUnequal( m_oTiger, pDownload->m_oTiger ) ) return FALSE;
	if ( validAndUnequal( m_oED2K, pDownload->m_oED2K ) ) return FALSE;
	if ( validAndUnequal( m_oMD5, pDownload->m_oMD5 ) ) return FALSE;

	m_sName	= pDownload->m_sName;
    m_sPath = pDownload->GetPath( 0 );
	m_nFileBase	= 0;
	m_nSize	= pDownload->m_nSize;
	m_sFileTags.Empty();
	m_bFilePartial = TRUE;

	// Try to get existing file object from download
	auto_ptr< CFragmentedFile > pFile( pDownload->GetFile() );
	if ( ! pFile.get() )
		return FALSE;

	AttachFile( pFile.release() );

	if ( m_oSHA1 && !pDownload->m_oSHA1 )
	{
		pDownload->m_oSHA1 = m_oSHA1;
	}
	else
	{
		m_oSHA1 = pDownload->m_oSHA1;
	}

	if ( m_oTiger && ! pDownload->m_oTiger )
	{
		pDownload->m_oTiger = m_oTiger;
	}
	else
	{
		m_oTiger = pDownload->m_oTiger;
	}

	if ( m_oED2K && ! pDownload->m_oED2K )
	{
		pDownload->m_oED2K = m_oED2K;
	}
	else
	{
		m_oED2K = pDownload->m_oED2K;
	}

	if ( m_oBTH && ! pDownload->m_oBTH )
	{
		pDownload->m_oBTH = m_oBTH;
	}
	else
	{
		m_oBTH = pDownload->m_oBTH;
	}

	if ( m_oMD5 && ! pDownload->m_oMD5 )
	{
		pDownload->m_oMD5 = m_oMD5;
	}
	else
	{
		m_oMD5 = pDownload->m_oMD5;
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
	OnWrite();
}

void CUploadTransfer::AllocateBaseFile()
{
	m_pBaseFile = UploadFiles.GetFile( this );
}

BOOL CUploadTransfer::IsFileOpen() const
{
	return m_pFile.get() && m_pFile->IsOpen();
}

BOOL CUploadTransfer::OpenFile()
{
	if ( IsFileOpen() )
		return TRUE;

	auto_ptr< CFragmentedFile > pFile( new CFragmentedFile );
	if ( pFile.get() && pFile->Open( this, FALSE ) )
	{
		AttachFile( pFile.release() );
		return TRUE;
	}

	theApp.Message( MSG_ERROR, IDS_UPLOAD_CANTOPEN, (LPCTSTR)m_sName, (LPCTSTR)m_sAddress );

	return FALSE;
}

void CUploadTransfer::CloseFile()
{
	m_pFile.reset();
}

BOOL CUploadTransfer::WriteFile(QWORD nOffset, LPCVOID pData, QWORD nLength, QWORD* pnWritten)
{
	if ( ! IsFileOpen() )
		return FALSE;

	return m_pFile->Write( nOffset, pData, nLength, pnWritten);
}

BOOL CUploadTransfer::ReadFile(QWORD nOffset, LPVOID pData, QWORD nLength, QWORD* pnRead)
{
	if ( ! IsFileOpen() )
		return FALSE;

	return m_pFile->Read( nOffset, pData, nLength, pnRead );
}

void CUploadTransfer::AttachFile(CFragmentedFile* pFile)
{
	if ( pFile && m_pFile.get() == pFile )
		pFile->Release();
	else
		m_pFile.reset( pFile );
}
