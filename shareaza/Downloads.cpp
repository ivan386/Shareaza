//
// Downloads.cpp
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
#include "Downloads.h"
#include "Download.h"
#include "Transfers.h"
#include "Transfer.h"
#include "DownloadGroups.h"
#include "DownloadTransfer.h"
#include "DownloadTransferED2K.h"
#include "DownloadTransferBT.h"
#include "UploadQueues.h"

#include "Buffer.h"
#include "EDClient.h"
#include "QueryHit.h"
#include "MatchObjects.h"
#include "ShareazaURL.h"

#include "SHA.h"
#include "ED2K.h"
#include "TigerTree.h"

#include "WndMain.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDownloads Downloads;


//////////////////////////////////////////////////////////////////////
// CDownloads construction

CDownloads::CDownloads() :
	m_tBandwidthLastCalc	( 0 ),
	m_tBandwidthAtMax		( 0 ),
	m_tBandwidthAtMaxED2K	( 0 ),
//	m_nLimitNew				( 0 ),
	m_nLimitGeneric			( 0 ),
	m_nLimitDonkey			( 0 ),
	m_nTransfers			( 0 ),
	m_nBandwidth			( 0 ),
	m_nValidation			( 0 ),
	m_bAllowMoreDownloads	( TRUE ),
	m_bAllowMoreTransfers	( TRUE ),
	m_bClosing				( FALSE ),
	m_tLastConnect			( 0 ),
	m_nRunCookie			( 0 )
{
}

CDownloads::~CDownloads()
{
}

//////////////////////////////////////////////////////////////////////
// CDownloads add an empty download (privileged)

CDownload* CDownloads::Add()
{
	CDownload* pDownload = new CDownload();
	m_pList.AddTail( pDownload );
	return pDownload;
}

//////////////////////////////////////////////////////////////////////
// CDownloads add download from a hit or from a file

CDownload* CDownloads::Add(CQueryHit* pHit, BOOL bAddToHead)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	CDownload* pDownload = NULL;
	
	if ( pDownload == NULL && pHit->m_oSHA1 )
		pDownload = FindBySHA1( pHit->m_oSHA1 );
	if ( pDownload == NULL && pHit->m_oTiger )
		pDownload = FindByTiger( pHit->m_oTiger );
	if ( pDownload == NULL && pHit->m_oED2K )
		pDownload = FindByED2K( pHit->m_oED2K );
	if ( pDownload == NULL && pHit->m_oBTH )
		pDownload = FindByBTH( pHit->m_oBTH );
	if ( pDownload == NULL && pHit->m_oMD5 )
		pDownload = FindByMD5( pHit->m_oMD5 );

	if ( pDownload != NULL )
	{
		theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_ALREADY, (LPCTSTR)pHit->m_sName );
		
		pDownload->AddSourceHit( pHit );
		pDownload->Resume();
	}
	else
	{
		pDownload = new CDownload();
		pDownload->AddSourceHit( pHit, TRUE );

		if ( bAddToHead ) m_pList.AddHead( pDownload );
		else m_pList.AddTail( pDownload );
		
		theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_ADDED,
			(LPCTSTR)pDownload->GetDisplayName(), pDownload->GetSourceCount() );

        if ( pDownload->m_oSHA1 ) pDownload->m_oSHA1.signalTrusted();
        if ( pDownload->m_oED2K ) pDownload->m_oED2K.signalTrusted();
		if ( pDownload->m_oBTH ) pDownload->m_oBTH.signalTrusted();
		if ( pDownload->m_oMD5 ) pDownload->m_oMD5.signalTrusted();
	}

	pHit->m_bDownload = TRUE;
	
	DownloadGroups.Link( pDownload );
	Transfers.StartThread();

	if ( ( GetTryingCount() < Settings.Downloads.MaxFiles ) || ( bAddToHead ) )
	{
		pDownload->SetStartTimer();
	}
	
	
	return pDownload;
}

CDownload* CDownloads::Add(CMatchFile* pFile, BOOL bAddToHead)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	CDownload* pDownload = NULL;
	
	if ( pDownload == NULL && pFile->m_oSHA1 )
		pDownload = FindBySHA1( pFile->m_oSHA1 );
	if ( pDownload == NULL && pFile->m_oTiger )
		pDownload = FindByTiger( pFile->m_oTiger );
	if ( pDownload == NULL && pFile->m_oED2K )
		pDownload = FindByED2K( pFile->m_oED2K );
	if ( pDownload == NULL && pFile->m_oBTH )
		pDownload = FindByBTH( pFile->m_oBTH );
	if ( pDownload == NULL && pFile->m_oMD5 )
		pDownload = FindByMD5( pFile->m_oMD5 );

	if ( pDownload != NULL )
	{
		theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_ALREADY, (LPCTSTR)pFile->m_sName );
		
		pFile->AddHitsToDownload( pDownload );
		
		pDownload->Resume();
	}
	else
	{
		pDownload = new CDownload();
		if ( bAddToHead ) m_pList.AddHead( pDownload );
		else m_pList.AddTail( pDownload );
		
		pFile->AddHitsToDownload( pDownload, TRUE );
		
		theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_ADDED,
			(LPCTSTR)pDownload->GetDisplayName(), pDownload->GetSourceCount() );

        if ( pDownload->m_oSHA1 ) pDownload->m_oSHA1.signalTrusted();
        if ( pDownload->m_oED2K ) pDownload->m_oED2K.signalTrusted();
		if ( pDownload->m_oBTH ) pDownload->m_oBTH.signalTrusted();
		if ( pDownload->m_oMD5 ) pDownload->m_oMD5.signalTrusted();
	}
	
	pFile->m_bDownload = TRUE;
	
	DownloadGroups.Link( pDownload );
	Transfers.StartThread();
	
	if ( ( GetTryingCount() < Settings.Downloads.MaxFiles ) || ( bAddToHead ) )
	{
		pDownload->SetStartTimer();

		if ( ( (pDownload->GetEffectiveSourceCount() <= 1 ) ||
			( ( pDownload->m_oED2K || pDownload->m_oBTH || pDownload->m_oMD5 ) &&
				! pDownload->m_oSHA1 )) )
		{
			pDownload->FindMoreSources();
		}
	}
	
	return pDownload;
}

//////////////////////////////////////////////////////////////////////
// CDownloads add download from a URL

CDownload* CDownloads::Add(CShareazaURL* pURL)
{
	if ( pURL->m_nAction != CShareazaURL::uriDownload &&
		 pURL->m_nAction != CShareazaURL::uriSource ) return NULL;
	
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	CDownload* pDownload = NULL;
	BOOL bNew = TRUE;
	
	if ( pDownload == NULL && pURL->m_oSHA1 )
		pDownload = FindBySHA1( pURL->m_oSHA1 );
	if ( pDownload == NULL && pURL->m_oTiger )
		pDownload = FindByTiger( pURL->m_oTiger );
	if ( pDownload == NULL && pURL->m_oED2K )
		pDownload = FindByED2K( pURL->m_oED2K );
	if ( pDownload == NULL && pURL->m_oBTH )
		pDownload = FindByBTH( pURL->m_oBTH );
	if ( pDownload == NULL && pURL->m_oMD5 )
		pDownload = FindByMD5( pURL->m_oMD5 );
	
	if ( pDownload != NULL )
	{
		theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_ALREADY,
			(LPCTSTR)pDownload->GetDisplayName() );
		
		bNew = FALSE;
	}
	else
		pDownload = new CDownload();
	
	if ( ! pDownload->m_oSHA1 && pURL->m_oSHA1 )
	{
		pDownload->m_oSHA1			= pURL->m_oSHA1;
        pDownload->m_oSHA1.signalTrusted();
	}
	if ( ! pDownload->m_oTiger && pURL->m_oTiger )
	{
		pDownload->m_oTiger			= pURL->m_oTiger;
        pDownload->m_oTiger.signalTrusted();
	}
	if ( ! pDownload->m_oED2K && pURL->m_oED2K )
	{
		pDownload->m_oED2K			= pURL->m_oED2K;
        pDownload->m_oED2K.signalTrusted();
		pDownload->Share( TRUE );
	}
	if ( ! pDownload->m_oBTH && pURL->m_oBTH )
	{
		pDownload->m_oBTH			= pURL->m_oBTH;
		pDownload->m_oBTH.signalTrusted();
		pDownload->Share( TRUE );
	}
	if ( ! pDownload->m_oMD5 && pURL->m_oMD5 )
	{
		pDownload->m_oMD5			= pURL->m_oMD5;
		pDownload->m_oMD5.signalTrusted();
		pDownload->Share( TRUE );
	}
	
	if ( ! pDownload->m_sDisplayName.GetLength() && pURL->m_sName.GetLength() )
	{
		pDownload->m_sDisplayName = pURL->m_sName;
	}
	
	if ( pDownload->m_nSize == SIZE_UNKNOWN && pURL->m_bSize )
	{
		pDownload->m_nSize = pURL->m_nSize;
	}
	
	if ( pURL->m_sURL.GetLength() )
	{
		if ( ! pDownload->AddSourceURLs( pURL->m_sURL, FALSE ) && bNew )
		{
			if ( pURL->m_nAction == CShareazaURL::uriSource )
			{
				delete pDownload;
				return NULL;
			}
		}
	}
	
	// Add sources from torrents - DWK
	pDownload->SetTorrent( pURL->m_pTorrent );
	if ( pURL->m_pTorrent && pURL->m_pTorrent->m_sURLs.GetCount() > 0 )
	{
		for ( POSITION pos = pURL->m_pTorrent->m_sURLs.GetHeadPosition() ; pos ; )
		{
			CString pCurrentUrl = pURL->m_pTorrent->m_sURLs.GetNext( pos );
			pDownload->AddSourceURLs( (LPCTSTR)pCurrentUrl , FALSE  );
		}
		pURL->m_pTorrent->m_sURLs.RemoveAll();
	}
	
	if ( bNew )
	{
		m_pList.AddTail( pDownload );
		
		theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_ADDED,
			(LPCTSTR)pDownload->GetDisplayName(), pDownload->GetEffectiveSourceCount() );
		
		if ( (  pDownload->IsTorrent() && ( GetTryingCount(TRUE)  < Settings.BitTorrent.DownloadTorrents ) ) ||
			( !pDownload->IsTorrent() && ( GetTryingCount(FALSE) < Settings.Downloads.MaxFiles ) ) )
		{
			pDownload->SetStartTimer();
			if ( pDownload->GetEffectiveSourceCount() <= 1 ) 
				pDownload->FindMoreSources();
		}
		
		DownloadGroups.Link( pDownload );
		Transfers.StartThread();
	}

	return pDownload;
}

//////////////////////////////////////////////////////////////////////
// CDownloads commands

void CDownloads::PauseAll()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		GetNext( pos )->Pause( FALSE );
	}
}

void CDownloads::ClearCompleted()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		if ( ( pDownload->IsCompleted() ) && ( !pDownload->IsSeeding() ) ) pDownload->Remove();
	}
}

void CDownloads::ClearPaused()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		if ( pDownload->IsPaused() ) pDownload->Remove();
	}
}

void CDownloads::Clear(BOOL bShutdown)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	m_bClosing = TRUE;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		Remove( GetNext( pos ) );
	}
	
	m_pList.RemoveAll();
	m_bClosing = bShutdown;
}

void CDownloads::CloseTransfers()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	m_bClosing = TRUE;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		GetNext( pos )->CloseTransfers();
	}
	
	m_bClosing = FALSE;
	m_nTransfers = 0;
	m_nBandwidth = 0;
}

//////////////////////////////////////////////////////////////////////
// CDownloads list access

int CDownloads::GetSeedCount() const
{
	int nCount = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		
		if ( pDownload->IsSeeding() ) nCount++;
	}
	
	return nCount;
}

int CDownloads::GetActiveTorrentCount() const
{
	int nCount = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		
		if ( pDownload->IsDownloading() && pDownload->IsTorrent() &&
			! pDownload->IsSeeding()	&& ! pDownload->IsCompleted() &&
			! pDownload->IsMoving()		&& ! pDownload->IsPaused() )
				nCount++;
	}
	
	return nCount;
}

DWORD CDownloads::GetCount(BOOL bActiveOnly) const
{
	if ( ! bActiveOnly ) return (DWORD)m_pList.GetCount();
	
	DWORD nCount = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		
		if ( ! pDownload->IsMoving() && ! pDownload->IsPaused() &&
			 pDownload->GetEffectiveSourceCount() > 0 )
				nCount++;
	}
	
	return nCount;
}

DWORD CDownloads::GetTransferCount() const
{
	DWORD nCount = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		nCount += GetNext( pos )->GetTransferCount();
	}
	
	return nCount;
}

DWORD CDownloads::GetTryingCount(BOOL bTorrentsOnly) const
{
	DWORD nCount = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		
		if ( !bTorrentsOnly || pDownload->IsTorrent() )
		{
			if ( !pDownload->IsCompleted() && pDownload->IsTrying() && !pDownload->IsPaused() )
				nCount++;
		}
	}
	
	return nCount;
}

DWORD CDownloads::GetConnectingTransferCount() const
{
	DWORD nCount = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		
		nCount += pDownload->GetTransferCount( dtsConnecting );
	}
	
	return nCount;
}

void CDownloads::Remove(CDownload* pDownload)
{
	POSITION pos = m_pList.Find( pDownload );
	if ( pos != NULL ) m_pList.RemoveAt( pos );
	delete pDownload;
}

//////////////////////////////////////////////////////////////////////
// CDownloads find by pointer or hash

BOOL CDownloads::Check(CDownloadSource* pSource) const
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		if ( GetNext( pos )->CheckSource( pSource ) ) return TRUE;
	}
	return FALSE;
}

BOOL CDownloads::CheckActive(CDownload* pDownload, int nScope) const
{
	for ( POSITION pos = GetReverseIterator() ; pos && nScope > 0 ; )
	{
		CDownload* pTest = GetPrevious( pos );
		BOOL bActive = pTest->IsPaused() == FALSE && pTest->IsCompleted() == FALSE;
		
		if ( pDownload == pTest ) return bActive;
		if ( bActive ) nScope--;
	}
	
	return FALSE;
}

CDownload* CDownloads::FindByPath(LPCTSTR szPath) const
{
	if ( szPath && *szPath )
	{
		for ( POSITION pos = GetIterator() ; pos ; )
		{
			CDownload* pDownload = GetNext( pos );
			if ( pDownload->m_sDiskName.CompareNoCase( szPath ) == 0 )
				return pDownload;
		}
	}
	return NULL;
}

CDownload* CDownloads::FindByURN(LPCTSTR pszURN, BOOL bSharedOnly) const
{
	CDownload* pDownload;
    Hashes::TigerHash oTiger;
    Hashes::Sha1Hash oSHA1;
    Hashes::Ed2kHash oED2K;
	Hashes::BtHash oBTH;
	Hashes::Md5Hash oMD5;
	
    if ( oSHA1.fromUrn( pszURN ) )
	{
		if ( ( pDownload = FindBySHA1( oSHA1, bSharedOnly ) ) != NULL ) return pDownload;
	}
	
	if ( oTiger.fromUrn( pszURN ) )
	{
		if ( ( pDownload = FindByTiger( oTiger, bSharedOnly ) ) != NULL ) return pDownload;
	}
	
	if ( oED2K.fromUrn( pszURN ) )
	{
		if ( ( pDownload = FindByED2K( oED2K, bSharedOnly ) ) != NULL ) return pDownload;
	}

	if ( oBTH.fromUrn( pszURN ) )
	{
		if ( ( pDownload = FindByBTH( oBTH, bSharedOnly ) ) != NULL ) return pDownload;
	}

	if ( oMD5.fromUrn( pszURN ) )
	{
		if ( ( pDownload = FindByMD5( oMD5, bSharedOnly ) ) != NULL ) return pDownload;
	}
	
	return NULL;
}

CDownload* CDownloads::FindBySHA1(const Hashes::Sha1Hash& oSHA1, BOOL bSharedOnly) const
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		if ( validAndEqual( pDownload->m_oSHA1, oSHA1 ) )
		{
			if ( ! bSharedOnly || ( pDownload->IsShared() && pDownload->IsStarted() ) )
				return pDownload;
		}
	}
	
	return NULL;
}

CDownload* CDownloads::FindByTiger(const Hashes::TigerHash& oTiger, BOOL bSharedOnly) const
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		if ( validAndEqual( pDownload->m_oTiger, oTiger ) )
		{
			if ( ! bSharedOnly || ( pDownload->IsShared() && pDownload->IsStarted() ) )
				return pDownload;
		}
	}
	
	return NULL;
}

CDownload* CDownloads::FindByED2K(const Hashes::Ed2kHash& oED2K, BOOL bSharedOnly) const
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		if ( validAndEqual( pDownload->m_oED2K, oED2K ) )
		{
			if ( ! bSharedOnly || ( pDownload->IsShared() && pDownload->IsStarted() )
				&& ( pDownload->m_nSize > ED2K_PART_SIZE || pDownload->IsCompleted() ) )
				return pDownload;
		}
	}
	
	return NULL;
}

CDownload* CDownloads::FindByBTH(const Hashes::BtHash& oBTH, BOOL bSharedOnly) const
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		if ( validAndEqual( pDownload->m_oBTH, oBTH ) )
		{
			if ( ! bSharedOnly || ( pDownload->IsShared() ) )
				return pDownload;
		}
	}
	
	return NULL;
}

CDownload* CDownloads::FindByMD5(const Hashes::Md5Hash& oMD5, BOOL bSharedOnly) const
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		if ( validAndEqual( pDownload->m_oMD5, oMD5) )
		{
			if ( ! bSharedOnly || ( pDownload->IsShared() ) )
				return pDownload;
		}
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////
// CDownloads serialization ID

CDownload* CDownloads::FindBySID(DWORD nSerID) const
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		if ( pDownload->m_nSerID == nSerID ) return pDownload;
	}
	
	return NULL;
}

DWORD CDownloads::GetFreeSID()
{
	for ( ;; )
	{
		DWORD nSerID	= ( rand() & 0xFF ) + ( ( rand() & 0xFF ) << 8 )
						+ ( ( rand() & 0xFF ) << 16 ) + ( ( rand() & 0xFF ) << 24 );
		
		for ( POSITION pos = GetIterator() ; pos ; )
		{
			CDownload* pDownload = GetNext( pos );
			if ( pDownload->m_nSerID == nSerID ) { nSerID = 0; break; }
		}
		
		if ( nSerID ) return nSerID;
	}
	
//	ASSERT( FALSE );
//	return 0;
}

//////////////////////////////////////////////////////////////////////
// CDownloads ordering

BOOL CDownloads::Move(CDownload* pDownload, int nDelta)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	POSITION posMe = m_pList.Find( pDownload );
	if ( posMe == NULL ) return FALSE;
	
	POSITION posOther = posMe;
	
	if ( nDelta < 0 )
		m_pList.GetPrev( posOther );
	else
		m_pList.GetNext( posOther );
	
	if ( posOther == NULL) return FALSE;
	
	if ( nDelta < 0 )
		m_pList.InsertBefore( posOther, pDownload );
	else
		m_pList.InsertAfter( posOther, pDownload );
	m_pList.RemoveAt( posMe );
	
	DownloadGroups.IncBaseCookie();
	
	return TRUE;
}

BOOL CDownloads::Swap(CDownload* p1, CDownload*p2)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	POSITION pos1 = m_pList.Find( p1 );
	if (pos1 == NULL) return FALSE;
	
	POSITION pos2 = m_pList.Find( p2 );	
	if (pos2 == NULL) return FALSE;
	
	m_pList.InsertAfter(pos2, p1 );
	m_pList.RemoveAt( pos2);	
	m_pList.InsertAfter(pos1, p2 );
	m_pList.RemoveAt( pos1 );	
	return TRUE;
}

BOOL CDownloads::Reorder(CDownload* pDownload, CDownload* pBefore)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	POSITION pos1 = m_pList.Find( pDownload );
	if ( pos1 == NULL ) return FALSE;
	
	if ( pBefore != NULL )
	{
		POSITION pos2 = m_pList.Find( pBefore );
		if ( pos2 == NULL || pos1 == pos2 ) return FALSE;
		m_pList.RemoveAt( pos1 );
		m_pList.InsertBefore( pos2, pDownload );
	}
	else
	{
		m_pList.RemoveAt( pos1 );
		m_pList.AddTail( pDownload );
	}
	
	DownloadGroups.IncBaseCookie();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloads find amount downloaded from a user

QWORD CDownloads::GetAmountDownloadedFrom(IN_ADDR* pAddress)
{
	QWORD nTotal = 0;

	if ( pAddress == NULL ) return 0;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );

		nTotal += pDownload->GetAmountDownloadedFrom(pAddress);
	}
	return nTotal;
}

//////////////////////////////////////////////////////////////////////
// CDownloads bandwidth

DWORD CDownloads::GetBandwidth() const
{
	DWORD nTotal = 0;
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		nTotal += GetNext( pos )->GetMeasuredSpeed();
	}
	return nTotal;
}

//////////////////////////////////////////////////////////////////////
// CDownloads limiting tests

void CDownloads::UpdateAllows(BOOL bNew)
{
	DWORD nDownloads	= 0;
	DWORD nTransfers	= 0;
	
	if ( bNew ) m_tLastConnect = GetTickCount();
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		int nTemp = pDownload->GetTransferCount();
		
		if ( nTemp )
		{
			nDownloads ++;
			nTransfers += nTemp;
		}
	}
	
	m_bAllowMoreDownloads = nDownloads < Settings.Downloads.MaxFiles;
	m_bAllowMoreTransfers = nTransfers < Settings.Downloads.MaxTransfers;
}

BOOL CDownloads::AllowMoreDownloads() const
{
	DWORD nCount = 0;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		if ( GetNext( pos )->HasActiveTransfers() )
			nCount++;
	}
	
	return nCount < Settings.Downloads.MaxFiles;
}

BOOL CDownloads::AllowMoreTransfers(IN_ADDR* pAddress) const
{
	DWORD nCount = 0, nLimit = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		nCount += GetNext( pos )->GetTransferCount( dtsCountAll, pAddress );
	}
	
	if ( pAddress == NULL ) return nCount < Settings.Downloads.MaxTransfers;
	
	if ( m_pHostLimits.Lookup( pAddress->S_un.S_addr, nLimit ) )
	{
		return ( nCount < nLimit );
	}
	else
	{
		return ( nCount == 0 );
	}
}

void CDownloads::SetPerHostLimit(IN_ADDR* pAddress, DWORD nLimit)
{
	m_pHostLimits.SetAt( pAddress->S_un.S_addr, nLimit );
}

//////////////////////////////////////////////////////////////////////
// CDownloads disk space helper

BOOL CDownloads::IsSpaceAvailable(QWORD nVolume, int nPath)
{
	QWORD nMargin = 10485760;

	if ( theApp.m_pfnGetDiskFreeSpaceExW != NULL )
	{
		ULARGE_INTEGER nFree, nNull;

		if ( ( ! nPath || nPath == dlPathIncomplete ) && theApp.m_pfnGetDiskFreeSpaceExW( Settings.Downloads.IncompletePath, &nFree, &nNull, &nNull ) )
		{
			if ( nFree.QuadPart < nVolume + nMargin ) return FALSE;
		}

		if ( ( ! nPath || nPath == dlPathComplete ) && theApp.m_pfnGetDiskFreeSpaceExW( Settings.Downloads.CompletePath, &nFree, &nNull, &nNull ) )
		{
			if ( nFree.QuadPart < nVolume + nMargin ) return FALSE;
		}

		return TRUE;
	}

	DWORD nSPC, nBPS, nFree, nTotal;
	if ( ! nPath || nPath == dlPathIncomplete )
	{
		CString str = Settings.Downloads.IncompletePath.SpanExcluding( _T("\\") ) + '\\';

		if ( GetDiskFreeSpace( str, &nSPC, &nBPS, &nFree, &nTotal ) )
		{
			QWORD nBytes = (QWORD)nSPC * (QWORD)nBPS * (QWORD)nFree;
			if ( nBytes < nVolume + nMargin ) return FALSE;
		}
	}

	if ( ! nPath || nPath == dlPathComplete )
	{
		CString str = Settings.Downloads.CompletePath.SpanExcluding( _T("\\") ) + '\\';
	
		if ( GetDiskFreeSpace( str, &nSPC, &nBPS, &nFree, &nTotal ) )
		{
			QWORD nBytes = (QWORD)nSPC * (QWORD)nBPS * (QWORD)nFree;
			if ( nBytes < nVolume + nMargin ) return FALSE;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloads run callback (threaded)

void CDownloads::OnRun()
{
	DWORD tNow = GetTickCount();


	// Re-calculating bandwidth may be a little CPU heavy if there are a lot of transfers- limit
	// it to 4 times per second
	if ( ( ( tNow - m_tBandwidthLastCalc ) < 250 ) && ( tNow > m_tBandwidthLastCalc ) )
	{
		// Just run the downloads, don't bother re-calulating bandwidth
		CQuickLock oLock( Transfers.m_pSection );

		m_nValidation = 0;
		++m_nRunCookie;
			
		for ( POSITION pos = GetIterator(); pos; )
		{
			CDownload* pDownload = GetNext( pos );
			pDownload->m_nRunCookie = m_nRunCookie;
			pDownload->OnRun();
		}
	}
	else
	{
		// Run downloads, update bandwidth settings and allows
		m_tBandwidthLastCalc = tNow;

		DWORD nActiveDownloads		= 0;	// Number of downloads that are doing something
		DWORD nActiveTransfers		= 0;	// Number of transfers that are in the downloading state
		DWORD nTotalTransfers		= 0;	// Total transfers
		DWORD nTotalBandwidth		= 0;	// Total bandwidth in use
		DWORD nRunningTransfers		= 0;	// Number of transfers that are downloading and transfering data
		DWORD nRunningTransfersED2K	= 0;	// Number of ed2k transfers that are downloading and transfering data
		DWORD nTotalBandwidthED2K	= 0;	// Total ed2k bandwidth in use.

		DWORD nBandwidthAvailable	= Settings.Bandwidth.Downloads;
		DWORD nBandwidthAvailableED2K = 0;
		BOOL bDonkeyRatioActive		= FALSE;

		{	// Lock transfers section
			CQuickLock oLock( Transfers.m_pSection );
			CList<CDownloadTransfer*> pTransfersToLimit;
			m_nValidation = 0;
			++m_nRunCookie;
				
			// Run all the downloads, select the transfers that need bandwidth limiting
			for ( POSITION pos = GetIterator(); pos; )
			{
				CDownload* pDownload = GetNext( pos );
				pDownload->m_nRunCookie = m_nRunCookie;
				pDownload->OnRun();
					
				int nTemp = 0;
					
				for ( CDownloadTransfer* pTransfer = pDownload->GetFirstTransfer() ; pTransfer ; pTransfer = pTransfer->m_pDlNext )
				{
					if ( pTransfer->m_nProtocol == PROTOCOL_ED2K )					
					{
						CDownloadTransferED2K* pED2K = (CDownloadTransferED2K*)pTransfer;
						if ( pED2K->m_pClient == NULL || pED2K->m_pClient->m_bConnected == FALSE ) continue;
						if ( pTransfer->m_nState == dtsQueued ) continue;
					}
					else if ( pTransfer->m_nProtocol == PROTOCOL_BT )
					{						
						CDownloadTransferBT* pBT = (CDownloadTransferBT*)pTransfer;
						if ( pBT->m_nState == dtsTorrent && pBT->m_bChoked ) continue;
					}
						
					nTemp ++;
						
					if ( pTransfer->m_nState == dtsDownloading )
					{
						DWORD nSpeed = pTransfer->GetMeasuredSpeed();
						nTotalBandwidth += nSpeed;
						nActiveTransfers ++;
						if ( nSpeed > 32 ) 
						{
							nRunningTransfers ++; 
							pTransfersToLimit.AddTail( pTransfer );
							if ( pTransfer->m_nProtocol == PROTOCOL_ED2K ) 
							{
								nTotalBandwidthED2K += nSpeed;
								nRunningTransfersED2K ++;
							}
							// Limit will be set below, once all data is collected
						}
						else
						{
							// This download is 'stalled'. It's probably not going to transfer 
							// anything, so we don't want to reserve bandwidth for it.
							pTransfer->m_nBandwidth = 0;
							// Don't bother limiting. If it starts to transfer then it
							// will be limited properly. For now, ignore since it's inactive.
						}
					}
				}
					
				if ( nTemp )
				{
					nActiveDownloads ++;
					nTotalTransfers += nTemp;
				}
			}

			// Set the ed2k ratio, bandwidth available
			if ( UploadQueues.IsDonkeyRatioActive() )
			{
				nBandwidthAvailableED2K = max( UploadQueues.GetMinimumDonkeyBandwidth(), UploadQueues.GetCurrentDonkeyBandwidth() );
				if ( nBandwidthAvailableED2K < 10240 )
				{
					bDonkeyRatioActive = TRUE;
					// ED2K 3:1 ratio if you aren't uploading at 10KB/s
					nBandwidthAvailableED2K *= 3;	
				}
				else
					nBandwidthAvailableED2K = 0;
			}

			// Limit the transfers. Faster transfers get a bigger share of the bandwidth
			for ( POSITION pos = pTransfersToLimit.GetHeadPosition() ; pos ; )
			{
				CDownloadTransfer* pTransfer = pTransfersToLimit.GetNext( pos );

				DWORD nCurrentSpeed = pTransfer->GetMeasuredSpeed();
				DWORD nLimit;

				if ( nTotalBandwidth == 0 )
				{
					nLimit = nBandwidthAvailable / (DWORD)pTransfersToLimit.GetCount();
				}
				else
				{
					double nPercentageUsed = (double)nCurrentSpeed / (double)nTotalBandwidth;
					nPercentageUsed = min(nPercentageUsed, 0.90);
					nLimit = (DWORD)(nBandwidthAvailable * nPercentageUsed);
				}
					
				// eDonkey ratio
				if ( bDonkeyRatioActive && ( pTransfer->m_nProtocol == PROTOCOL_ED2K ) )
				{
					DWORD nLimitED2K;
					if ( nRunningTransfersED2K == 0 )
					{
						nLimitED2K = nBandwidthAvailableED2K;
					}
					else if ( nTotalBandwidthED2K == 0 )
					{
						nLimitED2K = nBandwidthAvailableED2K / nRunningTransfersED2K;
					}
					else
					{
						double nPercentageUsed = (double)nCurrentSpeed / (double)nTotalBandwidthED2K;
						nPercentageUsed = min(nPercentageUsed, 0.90);
						nLimitED2K = (DWORD)(nBandwidthAvailableED2K * nPercentageUsed );
					}

					if ( nLimit ) nLimit = min( nLimit, nLimitED2K );
					else nLimit = nLimitED2K;
				}

				// Minimum allocation- 64 bytes / second to prevent time-outs.
				if ( ( nLimit ) && ( nLimit < 64 ) ) nLimit = 64;

				pTransfer->m_nBandwidth = nLimit;
			}

		} 	// End of transfers section lock

		// Update limit assigned to new transfers
//		if ( nBandwidthAvailable > nTotalBandwidth )
//			m_nLimitNew = nBandwidthAvailable - nTotalBandwidth;
//		else
//			m_nLimitNew = Settings.Bandwidth.Request;

		// Save bandwidth stats, Update allows
		m_nTransfers = nActiveTransfers;
		m_nBandwidth = nTotalBandwidth;
				
		m_bAllowMoreDownloads = nActiveDownloads < (DWORD)Settings.Downloads.MaxFiles;
		m_bAllowMoreTransfers = nTotalTransfers < (DWORD)Settings.Downloads.MaxTransfers;

		// Set "bandwidth use is near maximum" timers
		// beware that the MAX setting uses a limit of 0 internally,
		// so we need to catch that case first
		if ( nBandwidthAvailable != 0 && ( nTotalBandwidth * 1.1 ) >= nBandwidthAvailable )
			m_tBandwidthAtMax = tNow;
		if ( nBandwidthAvailableED2K != 0 && ( nTotalBandwidthED2K * 1.1 ) >= nBandwidthAvailableED2K )
			m_tBandwidthAtMaxED2K = tNow;

		DownloadGroups.Save( FALSE );
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloads query hit handler

void CDownloads::OnQueryHits(CQueryHit* pHits)
{
	CSingleLock pLock( &Transfers.m_pSection );
	
	if ( ! pLock.Lock( 50 ) ) return;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		if ( pDownload->IsMoving() == FALSE ) pDownload->OnQueryHits( pHits );
	}	
}

//////////////////////////////////////////////////////////////////////
// CDownloads push handler

BOOL CDownloads::OnPush(const Hashes::Guid& oGUID, CConnection* pConnection)
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		if ( pDownload->OnAcceptPush( oGUID, pConnection ) ) return TRUE;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloads eDonkey2000 callback handler

BOOL CDownloads::OnDonkeyCallback(CEDClient* pClient, CDownloadSource* pExcept)
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;
	
	if ( m_bClosing ) return FALSE;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		if ( pDownload->OnDonkeyCallback( pClient, pExcept ) ) return TRUE;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloads verification handler

void CDownloads::OnVerify(LPCTSTR pszPath, BOOL bVerified)
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 500 ) ) return;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		if ( GetNext( pos )->OnVerify( pszPath, bVerified ) ) break;
	}	
}

//////////////////////////////////////////////////////////////////////
// CDownloads load and save

void CDownloads::Load()
{
//	m_nLimitNew = Settings.Bandwidth.Downloads;
	m_nLimitGeneric = Settings.Bandwidth.Downloads;
	m_nLimitDonkey = Settings.Bandwidth.Downloads;

	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	WIN32_FIND_DATA pFind;
	CString strPath;
	HANDLE hSearch;
	
	PurgeDeletes();
	PurgePreviews();
	
	DownloadGroups.CreateDefault();
	LoadFromCompoundFiles();
	
	strPath = Settings.Downloads.IncompletePath + _T("\\*.sd");
	hSearch = FindFirstFile( strPath, &pFind );
	
	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		do
		{
			CDownload* pDownload = new CDownload();
			
			strPath.Format( _T("%s\\%s"), (LPCTSTR)Settings.Downloads.IncompletePath, pFind.cFileName );
			
			if ( pDownload->Load( strPath ) )
			{
				if ( pDownload->m_bSeeding )
				{
					if ( !Settings.BitTorrent.AutoSeed || 
						 GetFileAttributes( pDownload->m_sServingFileName ) == INVALID_FILE_ATTRIBUTES )
					{
						::DeleteFile( strPath );
						delete pDownload;
						continue;
					}
					pDownload->m_bComplete = TRUE;
					pDownload->m_bVerify = TRI_TRUE;
				}
				m_pList.AddTail( pDownload );
			}
			else
			{
				theApp.Message( MSG_ERROR, _T("Error loading %s"), strPath );
				pDownload->ClearSources();
				delete pDownload;
			}
		}
		while ( FindNextFile( hSearch, &pFind ) );
		
		FindClose( hSearch );
	}
	
	Save( FALSE );
	DownloadGroups.Load();
	Transfers.StartThread();
}

void CDownloads::Save(BOOL bForce)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		if ( bForce || pDownload->m_nCookie != pDownload->m_nSaveCookie ) pDownload->Save( TRUE );
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloads load all the old compound file formats

void CDownloads::LoadFromCompoundFiles()
{
	if ( LoadFromCompoundFile( Settings.Downloads.IncompletePath + _T("\\Shareaza Downloads.dat") ) )
	{
		// Good
	}
	else if ( LoadFromCompoundFile( Settings.Downloads.IncompletePath + _T("\\Shareaza Downloads.bak") ) )
	{
		// Good
	}
	else if ( LoadFromTimePair() )
	{
		// Good
	}
	
	DeleteFile( Settings.Downloads.IncompletePath + _T("\\Shareaza Downloads.dat") );
	DeleteFile( Settings.Downloads.IncompletePath + _T("\\Shareaza Downloads.bak") );
	DeleteFile( Settings.Downloads.IncompletePath + _T("\\Shareaza.dat") );
	DeleteFile( Settings.Downloads.IncompletePath + _T("\\Shareaza1.dat") );
	DeleteFile( Settings.Downloads.IncompletePath + _T("\\Shareaza2.dat") );
}

BOOL CDownloads::LoadFromCompoundFile(LPCTSTR pszFile)
{
	CFile pFile;
	
	if ( ! pFile.Open( pszFile, CFile::modeRead ) ) return FALSE;
	
	CArchive ar( &pFile, CArchive::load );
	
	try
	{
		SerializeCompound( ar );
	}
	catch ( CException* pException )
	{
		pException->Delete();
		Clear();
		return FALSE;
	}
	
	return TRUE;
}

BOOL CDownloads::LoadFromTimePair()
{
	FILETIME pFileTime1 = { 0, 0 }, pFileTime2 = { 0, 0 };
	CFile pFile1, pFile2;
	BOOL bFile1, bFile2;
	CString strFile;
	
	strFile	= Settings.Downloads.IncompletePath + _T("\\Shareaza");
	bFile1	= pFile1.Open( strFile + _T("1.dat"), CFile::modeRead );
	bFile2	= pFile2.Open( strFile + _T("2.dat"), CFile::modeRead );
	
	if ( bFile1 || bFile2 )
	{
		if ( bFile1 ) bFile1 = pFile1.Read( &pFileTime1, sizeof(FILETIME) ) == sizeof(FILETIME);
		if ( bFile2 ) bFile2 = pFile2.Read( &pFileTime2, sizeof(FILETIME) ) == sizeof(FILETIME);
	}
	else
	{
		if ( ! pFile1.Open( strFile + _T(".dat"), CFile::modeRead ) ) return FALSE;
		pFileTime1.dwHighDateTime++;
	}
	
	CFile* pNewest = ( CompareFileTime( &pFileTime1, &pFileTime2 ) >= 0 ) ? &pFile1 : &pFile2;
	
	try
	{
		CArchive ar( pNewest, CArchive::load );
		SerializeCompound( ar );
		ar.Close();
	}
	catch ( CException* pException )
	{
		pException->Delete();
		Clear();
		
		if ( pNewest == &pFile1 && bFile2 )
			pNewest = &pFile2;
		else if ( pNewest == &pFile2 && bFile1 )
			pNewest = &pFile1;
		else
			pNewest = NULL;

		if ( pNewest != NULL )
		{
			try
			{
				CArchive ar( pNewest, CArchive::load );
				SerializeCompound( ar );
				ar.Close();
			}
			catch ( CException* pException )
			{
				pException->Delete();
				Clear();
				return FALSE;
			}
		}
	}
	
	return TRUE;
}

void CDownloads::SerializeCompound(CArchive& ar)
{
	ASSERT( ar.IsLoading() );
	
	int nVersion;
	ar >> nVersion;
	if ( nVersion < 4 ) return;
	
	for ( DWORD_PTR nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
	{
		CDownload* pDownload = new CDownload();
		m_pList.AddTail( pDownload );
		pDownload->Serialize( ar, nVersion );
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloads left over file purge operations

void CDownloads::PurgeDeletes()
{
	CList< CString > pRemove;
	HKEY hKey = NULL;
	
	if ( ERROR_SUCCESS != RegOpenKeyEx( HKEY_CURRENT_USER,
		_T("Software\\Shareaza\\Shareaza\\Delete"), 0, KEY_ALL_ACCESS, &hKey ) ) return;
	
	for ( DWORD nIndex = 0 ; nIndex < 1000 ; nIndex ++ )
	{
		DWORD nPath = MAX_PATH*2;
		TCHAR szPath[MAX_PATH*2];
		
		if ( ERROR_SUCCESS != RegEnumValue( hKey, nIndex, szPath, &nPath, NULL,
			NULL, NULL, NULL ) ) break;
		
		if ( GetFileAttributes( szPath ) == 0xFFFFFFFF || DeleteFile( szPath ) )
		{
			pRemove.AddTail( szPath );
		}
	}
	
	while ( ! pRemove.IsEmpty() )
	{
		RegDeleteValue( hKey, pRemove.RemoveHead() );
	}
	
	RegCloseKey( hKey );
}

void CDownloads::PurgePreviews()
{
	WIN32_FIND_DATA pFind;
	HANDLE hSearch;
	CString strPath;
	
	strPath = Settings.Downloads.IncompletePath + _T("\\Preview of *.*");
	hSearch = FindFirstFile( strPath, &pFind );
	if ( hSearch == INVALID_HANDLE_VALUE ) return;
	
	do
	{
		if ( _tcsnicmp( pFind.cFileName, _T("Preview of "), 11 ) == 0 )
		{
			strPath = Settings.Downloads.IncompletePath + '\\' + pFind.cFileName;
			DeleteFile( strPath );
		}
	}
	while ( FindNextFile( hSearch, &pFind ) );
	
	FindClose( hSearch );
}
