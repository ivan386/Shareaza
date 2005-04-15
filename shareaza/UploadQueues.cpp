//
// UploadQueues.cpp
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
#include "UploadQueues.h"
#include "UploadQueue.h"
#include "UploadTransfer.h"
#include "Transfers.h"
#include "SharedFile.h"
#include "Download.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CUploadQueues UploadQueues;


//////////////////////////////////////////////////////////////////////
// CUploadQueues construction

CUploadQueues::CUploadQueues()
{
	m_pTorrentQueue = new CUploadQueue();
	m_pHistoryQueue = new CUploadQueue();
	m_pHistoryQueue->m_bExpanded = FALSE;
}

CUploadQueues::~CUploadQueues()
{
	Clear();
	delete m_pHistoryQueue;
	delete m_pTorrentQueue;
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues enqueue and dequeue

BOOL CUploadQueues::Enqueue(CUploadTransfer* pUpload, BOOL bForce)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	ASSERT( pUpload != NULL );
	
	Dequeue( pUpload );
	
	ASSERT( pUpload->m_pQueue == NULL );
	
	if ( pUpload->m_nFileSize == 0 ) return FALSE;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CUploadQueue* pQueue = GetNext( pos );
		
		if ( pQueue->CanAccept(	pUpload->m_nProtocol,
								pUpload->m_sFileName,
								pUpload->m_nFileSize,
								pUpload->m_bFilePartial,
								pUpload->m_sFileTags ) )
		{
			if ( pQueue->Enqueue( pUpload, bForce, bForce ) ) return TRUE;
		}
	}
	
	return FALSE;
}

BOOL CUploadQueues::Dequeue(CUploadTransfer* pUpload)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	ASSERT( pUpload != NULL );
	
	if ( Check( pUpload->m_pQueue ) )
	{
		return pUpload->m_pQueue->Dequeue( pUpload );
	}
	else
	{
		pUpload->m_pQueue = NULL;
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues position lookup and optional start

int CUploadQueues::GetPosition(CUploadTransfer* pUpload, BOOL bStart)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	ASSERT( pUpload != NULL );
	
	if ( Check( pUpload->m_pQueue ) )
	{
		return pUpload->m_pQueue->GetPosition( pUpload, bStart );
	}
	else
	{
		pUpload->m_pQueue = NULL;
		return -1;
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues position stealing

BOOL CUploadQueues::StealPosition(CUploadTransfer* pTarget, CUploadTransfer* pSource)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	ASSERT( pTarget != NULL );
	ASSERT( pSource != NULL );
	
	Dequeue( pTarget );
	
	if ( ! Check( pSource->m_pQueue ) ) return FALSE;
	
	return pSource->m_pQueue->StealPosition( pTarget, pSource );
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues create and delete queues

CUploadQueue* CUploadQueues::Create(LPCTSTR pszName, BOOL bTop)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	CUploadQueue* pQueue = new CUploadQueue();
	if ( pszName != NULL ) pQueue->m_sName = pszName;
	pQueue->m_bEnable = ! bTop;
	
	if ( bTop )
		m_pList.AddHead( pQueue );
	else
		m_pList.AddTail( pQueue );
	
	return pQueue;
}

void CUploadQueues::Delete(CUploadQueue* pQueue)
{
	CSingleLock pLock( &m_pSection, TRUE );
	if ( ! Check( pQueue ) ) return;
	if ( POSITION pos = m_pList.Find( pQueue ) ) m_pList.RemoveAt( pos );
	delete pQueue;
}

BOOL CUploadQueues::Reorder(CUploadQueue* pQueue, CUploadQueue* pBefore)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	POSITION pos1 = m_pList.Find( pQueue );
	if ( pos1 == NULL ) return FALSE;
	
	if ( pBefore != NULL )
	{
		POSITION pos2 = m_pList.Find( pBefore );
		if ( pos2 == NULL || pos1 == pos2 ) return FALSE;
		m_pList.RemoveAt( pos1 );
		m_pList.InsertBefore( pos2, pQueue );
	}
	else
	{
		m_pList.RemoveAt( pos1 );
		m_pList.AddTail( pQueue );
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues queue selection

CUploadQueue* CUploadQueues::SelectQueue(PROTOCOLID nProtocol, CLibraryFile* pFile)
{
	return SelectQueue( nProtocol, pFile->m_sName, pFile->GetSize(), FALSE, pFile->m_sShareTags );
}

CUploadQueue* CUploadQueues::SelectQueue(PROTOCOLID nProtocol, CDownload* pFile)
{
	return SelectQueue( nProtocol, pFile->m_sRemoteName, pFile->m_nSize, TRUE );
}

CUploadQueue* CUploadQueues::SelectQueue(PROTOCOLID nProtocol, LPCTSTR pszName, QWORD nSize, BOOL bPartial, LPCTSTR pszShareTags)
{
	CSingleLock pLock( &m_pSection, TRUE );
	int nIndex = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; nIndex++ )
	{
		CUploadQueue* pQueue = GetNext( pos );
		pQueue->m_nIndex = nIndex;
		if ( pQueue->CanAccept( nProtocol, pszName, nSize, bPartial, pszShareTags ) ) return pQueue;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues counting

int CUploadQueues::GetTotalBandwidthPoints( BOOL ActiveOnly )
{
	CSingleLock pLock( &m_pSection, TRUE );
	int nCount = 0;
	CUploadQueue *pQptr;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		pQptr=GetNext( pos );
		if ( ActiveOnly )
		{
			if ( pQptr->m_bEnable )
			{
				if ( ( ( pQptr->m_nProtocols & ( 1 << PROTOCOL_ED2K ) ) != 0 ) && ( Settings.Connection.RequireForTransfers ) )
				{
					if ( ! ( Settings.eDonkey.EnableAlways | Settings.eDonkey.EnableToday ) ) 
						continue;
				}
			}
			else
				continue;
		}
		nCount += pQptr->m_nBandwidthPoints;
	}
	
	return nCount;
}

int CUploadQueues::GetQueueCapacity()
{
	CSingleLock pLock( &m_pSection, TRUE );
	int nCount = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		nCount += GetNext( pos )->GetQueueCapacity();
	}
	
	return nCount;
}

int CUploadQueues::GetQueuedCount()
{
	CSingleLock pLock( &m_pSection, TRUE );
	int nCount = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		nCount += GetNext( pos )->GetQueuedCount();
	}
	
	return nCount;
}

int CUploadQueues::GetQueueRemaining()
{
	CSingleLock pLock( &m_pSection, TRUE );
	int nCount = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		nCount += GetNext( pos )->GetQueueRemaining();
	}
	
	return nCount;
}

int CUploadQueues::GetTransferCount()
{
	CSingleLock pLock( &m_pSection, TRUE );
	int nCount = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		nCount += GetNext( pos )->GetTransferCount();
	}
	
	return nCount;
}

BOOL CUploadQueues::IsTransferAvailable()
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		if ( GetNext( pos )->GetAvailableBandwidth() > 0 ) return TRUE;
	}
	
	return FALSE;
}

DWORD CUploadQueues::GetDonkeyBandwidth()
{
	CSingleLock pLock( &m_pSection, TRUE );
	DWORD nBandwidth = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CUploadQueue* pQueue = GetNext( pos );
		
		if ( pQueue->m_nProtocols == 0 || ( pQueue->m_nProtocols & ( 1 << PROTOCOL_ED2K ) ) != 0 )
			nBandwidth += pQueue->GetBandwidthLimit( pQueue->m_nMaxTransfers );
	}
	
	return nBandwidth;
}

BOOL CUploadQueues::CanUpload(PROTOCOLID nProtocol, CLibraryFile *pFile, BOOL bCanQueue )
{ 	// Can the specified file be uploaded with the current queue setup?

	// Don't bother with 0 byte files
	if ( pFile->m_nSize == 0 ) return FALSE;

	// Detect Ghosts
	if ( pFile->IsGhost() ) return FALSE;

	// G1 and G2 both use HTTP transfers, Sharaza doesn't consider them different.
	if ( ( nProtocol == PROTOCOL_G1 ) || ( nProtocol == PROTOCOL_G2 ) )
		nProtocol = PROTOCOL_HTTP;		

	CSingleLock pLock( &m_pSection, TRUE );
	
	//Check each queue
	for ( POSITION pos = GetIterator() ; pos ; )		
	{
		CUploadQueue* pQueue = GetNext( pos );
		
		if ( pQueue->CanAccept(	nProtocol, pFile->m_sName, pFile->m_nSize, FALSE, pFile->m_sShareTags ) )
		{	// If this queue will accept this file
			if ( ( ! bCanQueue ) || ( pQueue->GetQueueRemaining() > 0 ) ) 
			{	// And we don't care if there is space now, or the queue isn't full)
				return TRUE; // Then this file can be uploaded
			}
		}
	}

	return FALSE;	//This file is not uploadable with the current queue setup
}

int CUploadQueues::QueueRank(PROTOCOLID nProtocol, CLibraryFile *pFile )
{ 	// if the specified file was requested now, what queue position would it be in?
	// 0x7FFF (max int) indicates the file cannot be downloaded


	// Don't bother with 0 byte files
	if ( pFile->m_nSize == 0 ) return 0x7FFF;

	// Detect Ghosts
	if ( pFile->IsGhost() ) return 0x7FFF;

	// G1 and G2 both use HTTP transfers, Sharaza doesn't consider them different.
	if ( ( nProtocol == PROTOCOL_G1 ) || ( nProtocol == PROTOCOL_G2 ) )
		nProtocol = PROTOCOL_HTTP;		

	CSingleLock pLock( &m_pSection, TRUE );
	
	//Check each queue
	for ( POSITION pos = GetIterator() ; pos ; )		
	{
		CUploadQueue* pQueue = GetNext( pos );
		
		if ( pQueue->CanAccept(	nProtocol, pFile->m_sName, pFile->m_nSize, FALSE, pFile->m_sShareTags ) )
		{	// If this queue will accept this file

			if ( pQueue->GetQueueRemaining() > 0 )
				return pQueue->GetQueuedCount();
		}
	}

	return 0x7FFF;	//This file is not uploadable with the current queue setup
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues clear

void CUploadQueues::Clear()
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		delete GetNext( pos );
	}
	
	m_pList.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues load and save

BOOL CUploadQueues::Load()
{
	CSingleLock pLock( &m_pSection, TRUE );
	CFile pFile;
	
	LoadString( m_pTorrentQueue->m_sName, IDS_UPLOAD_QUEUE_TORRENT );
	LoadString( m_pHistoryQueue->m_sName, IDS_UPLOAD_QUEUE_HISTORY );

	CString strFile = Settings.General.UserPath + _T("\\Data\\UploadQueues.dat");
	
	if ( ! pFile.Open( strFile, CFile::modeRead ) )
	{
		CreateDefault();
		return FALSE;
	}
	
	try
	{
		CArchive ar( &pFile, CArchive::load );
		Serialize( ar );
		ar.Close();
	}
	catch ( CException* pException )
	{
		pFile.Close();
		pException->Delete();
		CreateDefault();
		return FALSE;
	}
	
	if ( GetCount() == 0 ) CreateDefault();

	Validate();
	
	return TRUE;
}

BOOL CUploadQueues::Save()
{
	CSingleLock pLock( &m_pSection, TRUE );
	CFile pFile;
	
	CString strFile = Settings.General.UserPath + _T("\\Data\\UploadQueues.dat");
	if ( !pFile.Open( strFile, CFile::modeWrite|CFile::modeCreate ) )
		return FALSE;
	
	CArchive ar( &pFile, CArchive::store );
	Serialize( ar );
	ar.Close();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues serialize

void CUploadQueues::Serialize(CArchive& ar)
{
	int nVersion = 5;
	
	if ( ar.IsStoring() )
	{
		ar << nVersion;
		
		ar.WriteCount( GetCount() );
		
		for ( POSITION pos = GetIterator() ; pos ; )
		{
			GetNext( pos )->Serialize( ar, nVersion );
		}
	}
	else
	{
		Clear();
		
		ar >> nVersion;
		if ( nVersion < 2 ) AfxThrowUserException();
		
		for ( int nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			Create()->Serialize( ar, nVersion );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues create default

void CUploadQueues::CreateDefault()
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	CUploadQueue* pQueue = NULL;
	
	Clear();
	CString strQueueName;
	
	if ( Settings.Connection.OutSpeed > 1200 )  // 1200 Kb/s (Massive connection)
	{
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_ED2K_PARTIALS );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 40;
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_bPartial			= TRUE;
		pQueue->m_nCapacity			= 2000;
		pQueue->m_nMinTransfers		= 3;
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 10*60;
		pQueue->m_bRewardUploaders	= TRUE;

		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_ED2K_CORE );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 20;
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_nCapacity			= 2000;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 10*60;
		pQueue->m_bRewardUploaders	= TRUE;
		
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_PARTIAL_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 50;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_bPartial			= TRUE;
		pQueue->m_nMinTransfers		= 3;
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 5*60;
		pQueue->m_bRewardUploaders	= TRUE;
		
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_SMALL_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 10;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nMaxSize			= 1 * 1024 * 1024;
		pQueue->m_nCapacity			= 10;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRewardUploaders	= FALSE;
		
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_MEDIUM_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 10;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nMinSize			= 1  * 1024 * 1024 + 1;
		pQueue->m_nMaxSize			= 10 * 1024 * 1024 - 1;
		pQueue->m_nCapacity			= 10;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRewardUploaders	= FALSE;
		
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_LARGE_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 20;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nMinSize			= 10 * 1024 * 1024;
		pQueue->m_nCapacity			= 10;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 60*60;
		pQueue->m_bRewardUploaders	= FALSE;
	}
	else if ( Settings.Connection.OutSpeed > 800 )  // 800 Kb/s (Fast Broadband)
	{
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_ED2K_PARTIALS );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 40;
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_bPartial			= TRUE;
		pQueue->m_nCapacity			= 2000;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 10*60;
		pQueue->m_bRewardUploaders	= TRUE;

		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_ED2K_CORE );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 20;
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_nCapacity			= 2000;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 10*60;
		pQueue->m_bRewardUploaders	= TRUE;
		
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_PARTIAL_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 50;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_bPartial			= TRUE;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 5*60;
		pQueue->m_bRewardUploaders	= TRUE;
		
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_SMALL_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 10;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nMaxSize			= 1 * 1024 * 1024;
		pQueue->m_nCapacity			= 10;
		pQueue->m_nMinTransfers		= 1;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRewardUploaders	= FALSE;
		
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_MEDIUM_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 10;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nMinSize			= 1  * 1024 * 1024 + 1;
		pQueue->m_nMaxSize			= 10 * 1024 * 1024 - 1;
		pQueue->m_nCapacity			= 10;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRewardUploaders	= FALSE;
		
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_LARGE_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 20;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nMinSize			= 10 * 1024 * 1024;
		pQueue->m_nCapacity			= 10;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 60*60;
		pQueue->m_bRewardUploaders	= FALSE;
	}
	else if ( Settings.Connection.OutSpeed > 200 )  // >200 Kb/s (Good Broadband)
	{
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_ED2K_PARTIALS );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 30;
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_bPartial			= TRUE;
		pQueue->m_nCapacity			= 2000;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 10*60;
		pQueue->m_bRewardUploaders	= TRUE;

		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_ED2K_CORE );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 10;
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_nCapacity			= 2000;
		pQueue->m_nMinTransfers		= 1;
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 10*60;
		pQueue->m_bRewardUploaders	= TRUE;

		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_PARTIAL_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 50;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_bPartial			= TRUE;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 5*60;
		pQueue->m_bRewardUploaders	= TRUE;
		
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_SMALL_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 10;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nMaxSize			= 1 * 1024 * 1024;
		pQueue->m_nCapacity			= 8;
		pQueue->m_nMinTransfers		= 1;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRewardUploaders	= FALSE;
		
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_MEDIUM_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 10;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nMinSize			= 1  * 1024 * 1024 + 1;
		pQueue->m_nMaxSize			= 10 * 1024 * 1024 - 1;
		pQueue->m_nCapacity			= 8;
		pQueue->m_nMinTransfers		= 1;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRewardUploaders	= FALSE;
		
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_LARGE_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 10;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nMinSize			= 10 * 1024 * 1024;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_nCapacity			= 10;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 60*60;
		pQueue->m_bRewardUploaders	= FALSE;
	}
	else if ( Settings.Connection.OutSpeed > 120 )  // >120 Kb/s (Average Broadband)
	{
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_ED2K_PARTIALS );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 30;
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_bPartial			= TRUE;
		pQueue->m_nCapacity			= 2000;
		pQueue->m_nMinTransfers		= 1;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 10*60;
		pQueue->m_bRewardUploaders	= TRUE;

		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_ED2K_CORE );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 10;
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_nCapacity			= 1000;
		pQueue->m_nMinTransfers		= 1;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 10*60;
		pQueue->m_bRewardUploaders	= TRUE;
		
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_PARTIAL_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 50;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_bPartial			= TRUE;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 5*60;
		pQueue->m_bRewardUploaders	= TRUE;
	
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_SMALL_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 10;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nMaxSize			= 10 * 1024 * 1024 - 1;
		pQueue->m_nCapacity			= 10;
		pQueue->m_nMinTransfers		= 1;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRewardUploaders	= FALSE;
		
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_LARGE_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 10;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nMinSize			= 10 * 1024 * 1024;
		pQueue->m_nMinTransfers		= 2;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_nCapacity			= 10;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 60*60;
		pQueue->m_bRewardUploaders	= FALSE;
	}
	else if ( Settings.Connection.OutSpeed > 40 ) // >40 Kb/s (Slow Broadband/ISDN)
	{
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_ED2K_CORE );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 20;
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_nCapacity			= 500;
		pQueue->m_nMinTransfers		= 1;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 30*60;
		pQueue->m_bRewardUploaders	= TRUE;
		
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_PARTIAL_FILES );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 20;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_bPartial			= TRUE;
		pQueue->m_nCapacity			= 8;
		pQueue->m_nMinTransfers		= 1;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 20*60;
		pQueue->m_bRewardUploaders	= TRUE;
		
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_COMPLETE );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 15;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nCapacity			= 8;
		pQueue->m_nMinTransfers		= 1;
		pQueue->m_nMaxTransfers		= 4;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 20*60;
		pQueue->m_bRewardUploaders	= FALSE;
	}
	else  // <40 Kb/s (Dial up modem)
	{
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_ED2K_CORE );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 20;
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_bPartial			= TRUE;
		pQueue->m_nCapacity			= 500;
		pQueue->m_nMinTransfers		= 1;
		pQueue->m_nMaxTransfers		= 2;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 30*60;
		pQueue->m_bRewardUploaders	= TRUE;
		
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_QUEUE );
		pQueue						= Create( strQueueName );
		pQueue->m_nBandwidthPoints	= 30;
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nCapacity			= 5;
		pQueue->m_nMinTransfers		= 1;
		pQueue->m_nMaxTransfers		= 2;
		pQueue->m_bRotate			= TRUE;
		pQueue->m_nRotateTime		= 20*60;
		pQueue->m_bRewardUploaders	= FALSE;
	}
	
	Save();
}

//////////////////////////////////////////////////////////////////////
// CUploadQueues validate

void CUploadQueues::Validate()
{
	CString strQueueName;
	if ( SelectQueue( PROTOCOL_ED2K, _T("Filename"), 0x00A00000, TRUE ) == NULL &&
		 SelectQueue( PROTOCOL_ED2K, _T("Filename"), 0x03200000, TRUE ) == NULL &&
		 SelectQueue( PROTOCOL_ED2K, _T("Filename"), 0x1F400000, TRUE ) == NULL )
	{
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_ED2K_GUARD );
		CUploadQueue* pQueue		= Create( strQueueName );
		pQueue->m_nProtocols		= (1<<PROTOCOL_ED2K);
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRotate			= TRUE;
		
		if ( Settings.Connection.OutSpeed > 100 )
		{
			pQueue->m_nMinTransfers		= 2;
			pQueue->m_nBandwidthPoints	= 30;
			pQueue->m_nCapacity			= 2000;
			pQueue->m_nRotateTime		= 10*60;
			pQueue->m_bRewardUploaders	= TRUE;
		}
		else
		{
			pQueue->m_nMinTransfers		= 1;
			pQueue->m_nBandwidthPoints	= 20;
			pQueue->m_nCapacity			= 500;
			pQueue->m_nRotateTime		= 30*60;
			pQueue->m_bPartial			= TRUE;
			pQueue->m_bRewardUploaders	= TRUE;
		}
	}

	if ( SelectQueue( PROTOCOL_HTTP, _T("Filename"), 0x00A00000, TRUE ) == NULL &&
		 SelectQueue( PROTOCOL_HTTP, _T("Filename"), 0x03200000, TRUE ) == NULL &&
		 SelectQueue( PROTOCOL_HTTP, _T("Filename"), 0x1F400000, TRUE ) == NULL )
	{
		LoadString ( strQueueName, IDS_UPLOAD_QUEUE_HTTP_GUARD );
		CUploadQueue* pQueue		= Create( strQueueName );
		pQueue->m_nProtocols		= (1<<PROTOCOL_HTTP);
		pQueue->m_nMaxTransfers		= 5;
		pQueue->m_bRotate			= TRUE;
		
		if ( Settings.Connection.OutSpeed > 100 )
		{
			pQueue->m_nMinTransfers		= 2;
			pQueue->m_nBandwidthPoints	= 30;
			pQueue->m_nCapacity			= 10;
			pQueue->m_nRotateTime		= 10*60;
		}
		else
		{
			pQueue->m_nMinTransfers		= 1;
			pQueue->m_nBandwidthPoints	= 20;
			pQueue->m_nCapacity			= 5;
			pQueue->m_nRotateTime		= 30*60;
		}
	}
}

