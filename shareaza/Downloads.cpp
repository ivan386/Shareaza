//
// Downloads.cpp
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

CDownloads::CDownloads()
{
	m_nLimitGeneric	= Settings.Bandwidth.Downloads;
	m_nLimitDonkey	= Settings.Bandwidth.Downloads;
	m_nTransfers	= 0;
	m_nBandwidth	= 0;
	m_nRunCookie	= 0;
	m_bClosing		= FALSE;
	m_tLastConnect	= 0;
}

CDownloads::~CDownloads()
{
}

//////////////////////////////////////////////////////////////////////
// CDownloads add an empty download (privilaged)

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
	
	if ( pDownload == NULL && pHit->m_bSHA1 )
		pDownload = FindBySHA1( &pHit->m_pSHA1 );
	if ( pDownload == NULL && pHit->m_bTiger )
		pDownload = FindByTiger( &pHit->m_pTiger );
	if ( pDownload == NULL && pHit->m_bED2K )
		pDownload = FindByED2K( &pHit->m_pED2K );
	
	if ( pDownload != NULL )
	{
		theApp.Message( MSG_DOWNLOAD, IDS_DOWNLOAD_ALREADY, (LPCTSTR)pHit->m_sName );
		
		pDownload->AddSourceHit( pHit );
		pDownload->Resume();
	}
	else
	{
		pDownload = new CDownload();
		pDownload->AddSourceHit( pHit, TRUE );

		if ( bAddToHead ) m_pList.AddHead( pDownload );
		else m_pList.AddTail( pDownload );
		
		theApp.Message( MSG_DOWNLOAD, IDS_DOWNLOAD_ADDED,
			(LPCTSTR)pDownload->GetDisplayName(), pDownload->GetSourceCount() );

		if( pDownload->m_bSHA1 ) pDownload->m_bSHA1Trusted = TRUE;
		else if( pDownload->m_bED2K ) pDownload->m_bED2KTrusted = TRUE;
	}

	pHit->m_bDownload = TRUE;
	
	DownloadGroups.Link( pDownload );
	Transfers.StartThread();

	if ( ( (pDownload->GetSourceCount() == 0 ) || ( pDownload->m_bED2K && ! pDownload->m_bSHA1 ) ) 
	 &&( (GetTryingCount() < Settings.Downloads.MaxFiles ) || ( bAddToHead ) ) )
	{
		pDownload->SetStartTimer();
	}
	
	
	return pDownload;
}

CDownload* CDownloads::Add(CMatchFile* pFile, BOOL bAddToHead)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	CDownload* pDownload = NULL;
	
	if ( pDownload == NULL && pFile->m_bSHA1 )
		pDownload = FindBySHA1( &pFile->m_pSHA1 );
	if ( pDownload == NULL && pFile->m_bTiger )
		pDownload = FindByTiger( &pFile->m_pTiger );
	if ( pDownload == NULL && pFile->m_bED2K )
		pDownload = FindByED2K( &pFile->m_pED2K );
	
	if ( pDownload != NULL )
	{
		theApp.Message( MSG_DOWNLOAD, IDS_DOWNLOAD_ALREADY, (LPCTSTR)pFile->m_pHits->m_sName );
		
		for ( CQueryHit* pHit = pFile->m_pHits ; pHit ; pHit = pHit->m_pNext )
		{
			pDownload->AddSourceHit( pHit );
		}
		
		pDownload->Resume();
	}
	else
	{
		pDownload = new CDownload();
		if ( bAddToHead ) m_pList.AddHead( pDownload );
		else m_pList.AddTail( pDownload );
		
		if ( pFile->m_pBest != NULL )
		{
			pDownload->AddSourceHit( pFile->m_pBest, TRUE );
		}
		
		for ( CQueryHit* pHit = pFile->m_pHits ; pHit ; pHit = pHit->m_pNext )
		{
			if ( pHit != pFile->m_pBest )
			{
				pDownload->AddSourceHit( pHit, TRUE );
			}
		}
		
		theApp.Message( MSG_DOWNLOAD, IDS_DOWNLOAD_ADDED,
			(LPCTSTR)pDownload->GetDisplayName(), pDownload->GetSourceCount() );

		if( pDownload->m_bSHA1 ) pDownload->m_bSHA1Trusted = TRUE;
		else if( pDownload->m_bED2K ) pDownload->m_bED2KTrusted = TRUE;
	}
	
	pFile->m_bDownload = TRUE;
	
	DownloadGroups.Link( pDownload );
	Transfers.StartThread();
	
	if ( ( (pDownload->GetSourceCount() == 0 ) ||
		   ( pDownload->m_bED2K && ! pDownload->m_bSHA1 )) &&
		   (GetTryingCount() < Settings.Downloads.MaxFiles ) )
	{
		pDownload->FindMoreSources();
		pDownload->SetStartTimer();
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
	
	if ( pDownload == NULL && pURL->m_bSHA1 )
		pDownload = FindBySHA1( &pURL->m_pSHA1 );
	if ( pDownload == NULL && pURL->m_bTiger )
		pDownload = FindByTiger( &pURL->m_pTiger );
	if ( pDownload == NULL && pURL->m_bED2K )
		pDownload = FindByED2K( &pURL->m_pED2K );
	if ( pDownload == NULL && pURL->m_bBTH )
		pDownload = FindByBTH( &pURL->m_pBTH );
	
	if ( pDownload != NULL )
	{
		theApp.Message( MSG_DOWNLOAD, IDS_DOWNLOAD_ALREADY,
			(LPCTSTR)pDownload->GetDisplayName() );
		
		if ( pURL->m_sURL.GetLength() ) pDownload->AddSourceURLs( pURL->m_sURL, FALSE );
		
		return pDownload;
	}
	
	pDownload = new CDownload();
	
	if ( pURL->m_bSHA1 )
	{
		pDownload->m_bSHA1			= TRUE;
		pDownload->m_pSHA1			= pURL->m_pSHA1;
		pDownload->m_bSHA1Trusted	= TRUE;
	}
	if ( pURL->m_bTiger )
	{
		pDownload->m_bTiger			= TRUE;
		pDownload->m_pTiger			= pURL->m_pTiger;
		pDownload->m_bTigerTrusted	= TRUE;
	}
	if ( pURL->m_bMD5 )
	{
		pDownload->m_bMD5			= TRUE;
		pDownload->m_pMD5			= pURL->m_pMD5;
		pDownload->m_bMD5Trusted	= TRUE;
	}
	if ( pURL->m_bED2K )
	{
		pDownload->m_bED2K			= TRUE;
		pDownload->m_pED2K			= pURL->m_pED2K;
		pDownload->m_bED2KTrusted	= TRUE;
		pDownload->Share( TRUE );
	}
	if ( pURL->m_bBTH )
	{
		pDownload->m_bBTH			= TRUE;
		pDownload->m_pBTH			= pURL->m_pBTH;
		pDownload->m_bBTHTrusted	= TRUE;
		pDownload->Share( TRUE );
	}
	
	if ( pURL->m_sName.GetLength() )
	{
		pDownload->m_sRemoteName = pURL->m_sName;
	}
	
	if ( pURL->m_bSize )
	{
		pDownload->m_nSize = pURL->m_nSize;
	}
	
	if ( pURL->m_sURL.GetLength() )
	{
		if ( ! pDownload->AddSourceURLs( pURL->m_sURL, FALSE ) )
		{
			if ( pURL->m_nAction == CShareazaURL::uriSource )
			{
				delete pDownload;
				return NULL;
			}
		}
	}
	
	pDownload->SetTorrent( pURL->m_pTorrent );
	
	m_pList.AddTail( pDownload );
	
	theApp.Message( MSG_DOWNLOAD, IDS_DOWNLOAD_ADDED,
		(LPCTSTR)pDownload->GetDisplayName(), pDownload->GetSourceCount() );
	
	if( (  pDownload->m_bBTH && ( GetTryingCount(TRUE)  < Settings.BitTorrent.DownloadTorrents ) ) ||
		( !pDownload->m_bBTH && ( GetTryingCount(FALSE) < Settings.Downloads.MaxFiles ) ) )
	{
		pDownload->SetStartTimer();
		if ( pURL->m_nAction != CShareazaURL::uriSource )
			pDownload->FindMoreSources();
	}
	
	DownloadGroups.Link( pDownload );
	Transfers.StartThread();
	
	return pDownload;
}

//////////////////////////////////////////////////////////////////////
// CDownloads commands

void CDownloads::PauseAll()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		GetNext( pos )->Pause();
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
		
		if ( pDownload->IsSeeding() )
			nCount++;		//Manually seeded Torrent
		else if ( pDownload->IsCompleted() && pDownload->m_bBTH && pDownload->IsFullyVerified() )
			nCount++;		//Torrent that has completed
	}
	
	return nCount;
}

int CDownloads::GetActiveTorrentCount() const
{
	int nCount = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		
		if ( pDownload->IsDownloading() && pDownload->m_bBTH &&
			! pDownload->IsSeeding()	&& ! pDownload->IsCompleted() &&
			! pDownload->IsMoving()		&& ! pDownload->IsPaused() )
				nCount++;
	}
	
	return nCount;
}

int CDownloads::GetCount(BOOL bActiveOnly) const
{
	if ( ! bActiveOnly ) return m_pList.GetCount();
	
	int nCount = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		
		if ( ! pDownload->IsMoving() && ! pDownload->IsPaused() &&
			 pDownload->GetSourceCount( TRUE ) > 0 )
				nCount++;
	}
	
	return nCount;
}

int CDownloads::GetTransferCount() const
{
	int nCount = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		nCount += GetNext( pos )->GetTransferCount();
	}
	
	return nCount;
}

int CDownloads::GetTryingCount(BOOL bTorrentsOnly) const
{
	int nCount = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		
		if ( ( pDownload->IsTrying() ) && ( ! pDownload->IsCompleted() ) && ( ! pDownload->IsPaused() ) )
		{
			if ( ( pDownload->m_bBTH ) || ( ! bTorrentsOnly ) )
				nCount++;
		}
	}
	
	return nCount;
}

int CDownloads::GetConnectingTransferCount() const
{
	int nCount = 0;
	
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

CDownload* CDownloads::FindByURN(LPCTSTR pszURN, BOOL bSharedOnly) const
{
	CDownload* pDownload;
	TIGEROOT pTiger;
	SHA1 pSHA1;
	MD4 pED2K;
	
	if ( CSHA::HashFromURN( pszURN, &pSHA1 ) )
	{
		if ( pDownload = FindBySHA1( &pSHA1, bSharedOnly ) ) return pDownload;
	}
	
	if ( CTigerNode::HashFromURN( pszURN, &pTiger ) )
	{
		if ( pDownload = FindByTiger( &pTiger, bSharedOnly ) ) return pDownload;
	}
	
	if ( CED2K::HashFromURN( pszURN, &pED2K ) )
	{
		if ( pDownload = FindByED2K( &pED2K, bSharedOnly ) ) return pDownload;
	}
	
	return NULL;
}

CDownload* CDownloads::FindBySHA1(const SHA1* pSHA1, BOOL bSharedOnly) const
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		if ( pDownload->m_bSHA1 && pDownload->m_pSHA1 == *pSHA1 )
		{
			if ( ! bSharedOnly || ( pDownload->IsShared() && pDownload->IsStarted() ) )
				return pDownload;
		}
	}
	
	return NULL;
}

CDownload* CDownloads::FindByTiger(const TIGEROOT* pTiger, BOOL bSharedOnly) const
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		if ( pDownload->m_bTiger && pDownload->m_pTiger == *pTiger )
		{
			if ( ! bSharedOnly || ( pDownload->IsShared() && pDownload->IsStarted() ) )
				return pDownload;
		}
	}
	
	return NULL;
}

CDownload* CDownloads::FindByED2K(const MD4* pED2K, BOOL bSharedOnly) const
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		if ( pDownload->m_bED2K && pDownload->m_pED2K == *pED2K )
		{
			if ( ! bSharedOnly || ( pDownload->IsShared() && pDownload->IsStarted() ) )
				return pDownload;
		}
	}
	
	return NULL;
}

CDownload* CDownloads::FindByBTH(const SHA1* pBTH, BOOL bSharedOnly) const
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		if ( pDownload->m_bBTH && pDownload->m_pBTH == *pBTH )
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
	
	ASSERT( FALSE );
	return 0;
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
	int nDownloads	= 0;
	int nTransfers	= 0;
	
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
	int nCount = 0;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		if ( GetNext( pos )->GetTransferCount() ) nCount++;
	}
	
	return nCount < Settings.Downloads.MaxFiles;
}

BOOL CDownloads::AllowMoreTransfers(IN_ADDR* pAddress) const
{
	int nCount = 0, nLimit = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		nCount += GetNext( pos )->GetTransferCount( dtsCountAll, pAddress );
	}
	
	if ( pAddress == NULL ) return nCount < Settings.Downloads.MaxTransfers;
	
	if ( m_pHostLimits.Lookup( (LPVOID)pAddress->S_un.S_addr, (void*&)nLimit ) )
	{
		return ( nCount < nLimit );
	}
	else
	{
		return ( nCount == 0 );
	}
}

void CDownloads::SetPerHostLimit(IN_ADDR* pAddress, int nLimit)
{
	m_pHostLimits.SetAt( (LPVOID)pAddress->S_un.S_addr, (LPVOID)nLimit );
}

//////////////////////////////////////////////////////////////////////
// CDownloads disk space helper

BOOL CDownloads::IsSpaceAvailable(QWORD nVolume)
{
	QWORD nMargin = 10485760;
	
	if ( HINSTANCE hKernel = GetModuleHandle( _T("KERNEL32.DLL") ) )
	{
		 BOOL (WINAPI *pfnGetDiskFreeSpaceEx)(LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER); 
#ifdef UNICODE
		(FARPROC&)pfnGetDiskFreeSpaceEx = GetProcAddress( hKernel, "GetDiskFreeSpaceExW" );
#else
 		(FARPROC&)pfnGetDiskFreeSpaceEx = GetProcAddress( hKernel, "GetDiskFreeSpaceExA" );
#endif	
		if ( pfnGetDiskFreeSpaceEx != NULL )
		{
			ULARGE_INTEGER nFree, nNull;
			
			if ( (*pfnGetDiskFreeSpaceEx)( Settings.Downloads.IncompletePath, &nFree, &nNull, &nNull ) )
			{
				if ( nFree.QuadPart < nVolume + nMargin ) return FALSE;
				
				if ( (*pfnGetDiskFreeSpaceEx)( Settings.Downloads.CompletePath, &nFree, &nNull, &nNull ) )
				{
					if ( nFree.QuadPart < nVolume + nMargin ) return FALSE;
				}
				
				return TRUE;
			}
		}
	}
	
	CString str = Settings.Downloads.IncompletePath.SpanExcluding( _T("\\") ) + '\\';
	DWORD nSPC, nBPS, nFree, nTotal;
	
	if ( GetDiskFreeSpace( str, &nSPC, &nBPS, &nFree, &nTotal ) )
	{
		QWORD nBytes = (QWORD)nSPC * (QWORD)nBPS * (QWORD)nFree;
		if ( nBytes < nVolume + nMargin ) return FALSE;
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloads run callback (threaded)

void CDownloads::OnRun()
{
	CSingleLock pLock( &Transfers.m_pSection );
	
	DWORD nActiveDownloads	= 0;
	DWORD nActiveTransfers	= 0;
	DWORD nTotalTransfers	= 0;
	DWORD nTotalBandwidth	= 0;
	
	m_nValidation = 0;
	m_nRunCookie ++;
	
	while ( TRUE )
	{
		BOOL bWorked = FALSE;
		pLock.Lock();
		
		for ( POSITION pos = GetIterator() ; pos ; )
		{
			CDownload* pDownload = GetNext( pos );
			if ( pDownload->m_nRunCookie == m_nRunCookie ) continue;
			
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
					nTotalBandwidth += pTransfer->GetMeasuredSpeed();
					nActiveTransfers ++;
				}
			}
			
			if ( nTemp )
			{
				nActiveDownloads ++;
				nTotalTransfers += nTemp;
			}
			
			bWorked = TRUE;
			break;
		}
		
		pLock.Unlock();
		if ( ! bWorked ) break;
	}
	
	m_nTransfers = nActiveTransfers;
	m_nBandwidth = nTotalBandwidth;
	
	m_bAllowMoreDownloads = nActiveDownloads < (DWORD)Settings.Downloads.MaxFiles;
	m_bAllowMoreTransfers = nTotalTransfers < (DWORD)Settings.Downloads.MaxTransfers;
	
	if ( nActiveTransfers > 0 )
	{
		m_nLimitGeneric	= Settings.Bandwidth.Downloads / nActiveTransfers;
		m_nLimitDonkey	= UploadQueues.GetDonkeyBandwidth();
		
		if ( m_nLimitDonkey < 10240 )
			m_nLimitDonkey	= min( m_nLimitGeneric, m_nLimitDonkey * 3 / nActiveTransfers );
		else
			m_nLimitDonkey = m_nLimitGeneric;
	}
	else
	{
		m_nLimitGeneric = m_nLimitDonkey = Settings.Bandwidth.Downloads;
	}
	
	DownloadGroups.Save( FALSE );
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

BOOL CDownloads::OnPush(GGUID* pGUID, CConnection* pConnection)
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDownload* pDownload = GetNext( pos );
		if ( pDownload->OnAcceptPush( pGUID, pConnection ) ) return TRUE;
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
				m_pList.AddTail( pDownload );
			}
			else
			{
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
	
	for ( int nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
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
	CStringList pRemove;
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
