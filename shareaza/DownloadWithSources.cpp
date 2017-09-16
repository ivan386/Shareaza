//
// DownloadWithSources.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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
#include "DownloadGroups.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "DownloadWithSources.h"
#include "Downloads.h"
#include "Library.h"
#include "MatchObjects.h"
#include "Neighbours.h"
#include "Network.h"
#include "QueryHashMaster.h"
#include "QueryHit.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "Security.h"
#include "ShareazaURL.h"
#include "SharedFile.h"
#include "Transfer.h"
#include "Transfers.h"
#include "VendorCache.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadWithSources construction

CDownloadWithSources::CDownloadWithSources()
	: m_nG1SourceCount	( 0 )
	, m_nG2SourceCount	( 0 )
	, m_nEdSourceCount	( 0 )
	, m_nHTTPSourceCount( 0 )
	, m_nBTSourceCount	( 0 )
	, m_nFTPSourceCount	( 0 )
	, m_nDCSourceCount	( 0 )
	, m_pXML			( NULL )
{
}

CDownloadWithSources::~CDownloadWithSources()
{
	ClearSources();

	delete m_pXML;

	for ( POSITION pos = m_pFailedSources.GetHeadPosition() ; pos ; )
		delete m_pFailedSources.GetNext( pos );
	m_pFailedSources.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources list access

POSITION CDownloadWithSources::GetIterator() const
{
	ASSUME_LOCK( Transfers.m_pSection );

	return m_pSources.GetHeadPosition();
}

CDownloadSource* CDownloadWithSources::GetNext(POSITION& rPosition) const
{
	ASSUME_LOCK( Transfers.m_pSection );

	return m_pSources.GetNext( rPosition );
}

INT_PTR CDownloadWithSources::GetCount() const
{
	ASSUME_LOCK( Transfers.m_pSection );

	return m_pSources.GetCount();
}

bool CDownloadWithSources::HasMetadata() const
{
	return ( m_pXML != NULL );
}

DWORD CDownloadWithSources::GetSourceCount(BOOL bNoPush, BOOL bSane) const
{
	CQuickLock pLock( Transfers.m_pSection );

	if ( ! bNoPush && ! bSane )
		return (DWORD)GetCount();

	DWORD tNow = GetTickCount();
	DWORD nCount = 0;

	for ( POSITION posSource = GetIterator() ; posSource ; )
	{
		CDownloadSource* pSource = GetNext( posSource );

		if ( ! bNoPush || ! pSource->m_bPushOnly )
		{
			if ( ! bSane || (
			   ( pSource->m_tAttempt < tNow ||
				 pSource->m_tAttempt - tNow <= 900000 ) &&
				 ! pSource->m_bKeep ) )
			{
				nCount++;
			}
		}
	}

	return nCount;
}

DWORD CDownloadWithSources::GetEffectiveSourceCount() const
{
	bool bIsG1Allowed = Settings.Gnutella1.EnableToday  || ! Settings.Connection.RequireForTransfers;
	bool bIsG2Allowed = Settings.Gnutella2.EnableToday  || ! Settings.Connection.RequireForTransfers;
	bool bIsEdAllowed = Settings.eDonkey.EnableToday    || ! Settings.Connection.RequireForTransfers;
	bool bIsBtAllowed = Settings.BitTorrent.EnableToday || ! Settings.Connection.RequireForTransfers;
	bool bIsDCAllowed = Settings.DC.EnableToday || ! Settings.Connection.RequireForTransfers;

	DWORD nResult = m_nFTPSourceCount;

	if ( bIsG1Allowed )
		nResult += m_nG1SourceCount;

	if ( bIsG2Allowed )
		nResult += m_nG2SourceCount;

	if ( bIsG1Allowed || bIsG2Allowed )
		nResult += m_nHTTPSourceCount;

	if ( bIsEdAllowed )
		nResult += m_nEdSourceCount;

	if ( bIsBtAllowed )
		nResult += m_nBTSourceCount;

	if ( bIsDCAllowed )
		nResult += m_nDCSourceCount;

	return nResult;
}

DWORD CDownloadWithSources::GetBTSourceCount(BOOL bNoPush) const
{
	CQuickLock pLock( Transfers.m_pSection );

	DWORD tNow = GetTickCount();
	DWORD nCount = 0;

	for ( POSITION posSource = GetIterator() ; posSource ; )
	{
		CDownloadSource* pSource = GetNext( posSource );

		if ( ( pSource->m_nProtocol == PROTOCOL_BT ) &&									// Only counting BT sources
			 ( pSource->m_tAttempt < tNow || pSource->m_tAttempt - tNow <= 900000 ) &&	// Don't count dead sources
			 ( ! pSource->m_bPushOnly || ! bNoPush ) )									// Push sources might not be counted
		{
			nCount++;
		}
	}

	return nCount;
}

DWORD CDownloadWithSources::GetED2KCompleteSourceCount() const
{
	CQuickLock pLock( Transfers.m_pSection );

	DWORD tNow = GetTickCount();
	DWORD nCount = 0;

	for ( POSITION posSource = GetIterator() ; posSource ; )
	{
		CDownloadSource* pSource = GetNext( posSource );

		if ( ( ! pSource->m_bPushOnly ) &&						// Push sources shouldn't be counted since you often cannot reach them
			 ( pSource->m_tAttempt < tNow || pSource->m_tAttempt - tNow <= 900000 ) &&	// Only count sources that are probably active
			 ( pSource->m_nProtocol == PROTOCOL_ED2K ) &&		// Only count ed2k sources
             ( pSource->m_oAvailable.empty() && pSource->IsOnline() ) )	// Only count complete sources

		{
			nCount++;
		}
	}

	return nCount;
}

BOOL CDownloadWithSources::CheckSource(CDownloadSource* pCheck) const
{
	CQuickLock pLock( Transfers.m_pSection );

	return ( m_pSources.Find( pCheck ) != NULL );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources clear

void CDownloadWithSources::ClearSources()
{
	CQuickLock pLock( Transfers.m_pSection );

	for ( POSITION posSource = GetIterator() ; posSource ; )
	{
		CDownloadSource* pSource = GetNext( posSource );

		pSource->Remove();
	}
	m_pSources.RemoveAll();

	m_nG1SourceCount	= 0;
	m_nG2SourceCount	= 0;
	m_nEdSourceCount	= 0;
	m_nHTTPSourceCount	= 0;
	m_nBTSourceCount	= 0;
	m_nFTPSourceCount	= 0;
	m_nDCSourceCount	= 0;

	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources add a query-hit source

BOOL CDownloadWithSources::AddSource(const CShareazaFile* pHit, BOOL bForce)
{
	ASSUME_LOCK( Transfers.m_pSection );

	const bool bHasHash = HasHash();
	const bool bHitHasName = ( pHit->m_sName.GetLength() != 0 );
	const bool bHitHasSize = ( pHit->m_nSize != 0 && pHit->m_nSize != SIZE_UNKNOWN );
	bool bHash = FALSE;
	bool bUpdated = FALSE;

	if ( ! bForce )
	{
		if ( m_oSHA1 && pHit->m_oSHA1 )
		{
			if ( m_oSHA1 != pHit->m_oSHA1 ) return FALSE;
			bHash = true;
		}
		if ( m_oTiger && pHit->m_oTiger )
		{
			if ( m_oTiger != pHit->m_oTiger ) return FALSE;
			bHash = true;
		}
        if ( m_oED2K && pHit->m_oED2K )
		{
			if ( m_oED2K != pHit->m_oED2K ) return FALSE;
			bHash = true;
		}
		if ( m_oMD5 && pHit->m_oMD5 )
		{
			if ( m_oMD5 != pHit->m_oMD5 ) return FALSE;
			bHash = true;
		}
		// BTH check is a last chance
		if ( ! bHash && m_oBTH && pHit->m_oBTH )
		{
			if ( m_oBTH != pHit->m_oBTH ) return FALSE;
			bHash = true;
		}
	}

	if ( ! bHash && ! bForce )
	{
		if ( Settings.General.HashIntegrity ) return FALSE;
		if ( m_sName.IsEmpty() || ! bHitHasName ) return FALSE;
		if ( m_nSize == SIZE_UNKNOWN || ! bHitHasSize ) return FALSE;
		if ( m_nSize != pHit->m_nSize ) return FALSE;
		if ( m_sName.CompareNoCase( pHit->m_sName ) ) return FALSE;
	}

	if ( m_nSize != SIZE_UNKNOWN && bHasHash && bHitHasSize && m_nSize != pHit->m_nSize )
		return FALSE;

	if ( !m_oSHA1 && pHit->m_oSHA1 )
	{
		m_oSHA1 = pHit->m_oSHA1;
		bUpdated = true;
	}
    if ( !m_oTiger && pHit->m_oTiger )
	{
		m_oTiger = pHit->m_oTiger;
		bUpdated = true;
	}
    if ( !m_oED2K && pHit->m_oED2K )
	{
		m_oED2K = pHit->m_oED2K;
		bUpdated = true;
	}
	if ( !m_oBTH && pHit->m_oBTH )
	{
		m_oBTH = pHit->m_oBTH;
		bUpdated = true;
	}
	if ( !m_oMD5 && pHit->m_oMD5 )
	{
		m_oMD5 = pHit->m_oMD5;
		bUpdated = true;
	}
	if ( ( m_sName.IsEmpty() || ! bHasHash ) && bHitHasName )
	{
		bUpdated = Rename( pHit->m_sName );
	}
	if ( ( m_nSize == SIZE_UNKNOWN || ! bHasHash ) && bHitHasSize )
	{
		bUpdated = Resize( pHit->m_nSize );
	}

	if ( bUpdated )
	{
		// Re-link
		DownloadGroups.Link( static_cast< CDownload* >( this ) );

		static_cast< CDownload* >( this )->m_bUpdateSearch = TRUE;

		QueryHashMaster.Invalidate();
	}

	return TRUE;
}

BOOL CDownloadWithSources::AddSourceHit(const CQueryHit* pHit, BOOL bForce)
{
	CQuickLock oLock( Transfers.m_pSection );

	if ( ! AddSource( pHit, bForce ) )
		return FALSE;

	if ( Settings.Downloads.Metadata && m_pXML == NULL && pHit->m_pXML && pHit->m_pSchema )
	{
		m_pXML = pHit->m_pSchema->Instantiate( TRUE );
		m_pXML->AddElement( pHit->m_pXML->Clone() );
		pHit->m_pSchema->Validate( m_pXML, TRUE );
	}

	/*
	if ( pHit->m_nProtocol == PROTOCOL_ED2K )
	{
		Neighbours.FindDonkeySources( pHit->m_oED2K,
			(IN_ADDR*)pHit->m_oClientID.begin(), (WORD)pHit->m_oClientID.begin()[1] );
	}
	*/

	// No URL, stop now with success
	if ( pHit->m_sURL.GetLength() )
	{
		if ( ! AddSourceInternal( new CDownloadSource( (CDownload*)this, pHit ) ) )
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CDownloadWithSources::AddSourceHit(const CMatchFile* pMatchFile, BOOL bForce)
{
	CQuickLock oLock( Transfers.m_pSection );

	BOOL bRet = AddSource( pMatchFile, bForce );

	// Best goes first if forced
	const CQueryHit* pBestHit = pMatchFile->GetBest();
	if ( bForce && pBestHit )
	{
		bRet = AddSourceHit( pBestHit, TRUE ) || bRet;
	}

	for ( const CQueryHit* pHit = pMatchFile->GetHits() ; pHit; pHit = pHit->m_pNext )
	{
		if ( bForce )
		{
			// Best already added
			if ( pHit != pBestHit )
			{
				bRet = AddSourceHit( pHit, TRUE ) || bRet;
			}
		}
		else
		{
			bRet = AddSourceHit( pHit, FALSE ) || bRet;
		}
	}

	return bRet;
}

BOOL CDownloadWithSources::AddSourceHit(const CShareazaURL& oURL, BOOL bForce, int nRedirectionCount)
{
	CQuickLock oLock( Transfers.m_pSection );

	if ( ! AddSource( &oURL, bForce ) )
		return FALSE;

	if ( oURL.m_pTorrent )
	{
		((CDownload*)this)->SetTorrent( oURL.m_pTorrent );
	}

	for ( CString sURLs = oURL.m_sURL; sURLs.GetLength(); )
	{
		CString sURL = sURLs.SpanExcluding( _T(",") );
		sURLs = sURLs.Mid( sURL.GetLength() + 1 );
		sURL.Trim();
		if ( sURL.GetLength() )
		{
			if ( ! AddSourceURL( sURL, NULL, nRedirectionCount, FALSE, bForce ) )
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources add miscellaneous sources

BOOL CDownloadWithSources::AddSourceED2K(DWORD nClientID, WORD nClientPort, DWORD nServerIP, WORD nServerPort, const Hashes::Guid& oGUID)
{
	return AddSourceInternal( new CDownloadSource( (CDownload*)this, nClientID, nClientPort, nServerIP, nServerPort, oGUID ) );
}

BOOL CDownloadWithSources::AddSourceBT(const Hashes::BtGuid& oGUID, const IN_ADDR* pAddress, WORD nPort, BOOL bIgnoreLocalIP)
{
	// Unreachable (Push) BT sources should never be added.
	if ( Network.IsFirewalledAddress( pAddress, Settings.Connection.IgnoreOwnIP, bIgnoreLocalIP ) )
		return FALSE;

	return AddSourceInternal( new CDownloadSource( (CDownload*)this, oGUID, pAddress, nPort ) );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources add a single URL source

BOOL CDownloadWithSources::AddSourceURL(LPCTSTR pszURL, FILETIME* pLastSeen, int nRedirectionCount, BOOL bFailed, BOOL bForce)
{
	if ( pszURL == NULL || *pszURL == 0 )
		return FALSE;

	if ( nRedirectionCount > 5 )
		return FALSE; // No more than 5 redirections

	BOOL bHashAuth = FALSE;
	CShareazaURL pURL;

	if ( *pszURL == '@' )
	{
		bHashAuth = TRUE;
		pszURL++;
	}

	if ( ! pURL.Parse( pszURL ) )
		return FALSE;	// Wrong URL

	if ( pURL.m_nAction == CShareazaURL::uriHost &&
		 pURL.m_nProtocol == PROTOCOL_DC )
	{
		// Connect to specified DC++ hub for future searches
		Network.ConnectTo( pURL.m_sName, pURL.m_nPort, PROTOCOL_DC );
		return FALSE;
	}

	if ( pURL.m_nAction != CShareazaURL::uriDownload &&
		 pURL.m_nAction != CShareazaURL::uriSource )
		return FALSE;	// Wrong URL type

	if ( pURL.m_pAddress.s_addr != INADDR_ANY && pURL.m_pAddress.s_addr != INADDR_NONE )
	{
		if ( Network.IsFirewalledAddress( &pURL.m_pAddress, TRUE ) ||
			 Network.IsReserved( &pURL.m_pAddress ) )
			 return FALSE;	// Unreachable URL
	}

	CQuickLock pLock( Transfers.m_pSection );

	if ( CFailedSource* pBadSource = LookupFailedSource( pszURL ) )
	{
		// Add a positive vote, add to the downloads if the negative votes compose
		// less than 2/3 of total.
		int nTotal = pBadSource->m_nPositiveVotes + pBadSource->m_nNegativeVotes + 1;
		if ( bFailed )
			pBadSource->m_nNegativeVotes++;
		else
			pBadSource->m_nPositiveVotes++;

		if ( nTotal > 30 && pBadSource->m_nNegativeVotes / nTotal > 2 / 3 )
			return FALSE;
	}
	else if ( bFailed )
	{
		AddFailedSource( pszURL, false );
		VoteSource( pszURL, false );
		return TRUE;
	}

	if ( ! AddSource( &pURL, bForce ) )
		// Not match
		return FALSE;

	return AddSourceInternal( new CDownloadSource( static_cast< const CDownload* >( this ), pszURL, bHashAuth, pLastSeen, nRedirectionCount ) );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources add several URL sources

int CDownloadWithSources::AddSourceURLs(LPCTSTR pszURLs, BOOL bFailed)
{
	int nCount = 0;

	CMapStringToFILETIME oUrls;
	SplitStringToURLs( pszURLs, oUrls );

	for ( POSITION pos = oUrls.GetStartPosition(); pos; )
	{
		CString strURL;
		FILETIME tSeen = {};
		oUrls.GetNextAssoc( pos, strURL, tSeen );

		if ( AddSourceURL( strURL, ( tSeen.dwLowDateTime | tSeen.dwHighDateTime ) ? &tSeen : NULL, 0, bFailed ) )
		{
			if ( bFailed )
			{
				theApp.Message( MSG_DEBUG, L"Adding X-NAlt: %s", (LPCTSTR)strURL );
			}
			else
			{
				theApp.Message( MSG_DEBUG, L"Adding X-Alt: %s", (LPCTSTR)strURL );
			}
			nCount++;
		}
	}

	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources internal source adder

BOOL CDownloadWithSources::AddSourceInternal(CDownloadSource* pSource)
{
	if ( ! pSource )
		// Out of memory
		return FALSE;

	//Check/Reject if source is invalid
	if ( ! pSource->m_bPushOnly )
	{
		//Reject invalid IPs (Sometimes ed2k sends invalid 0.x.x.x sources)
		if ( pSource->m_pAddress.S_un.S_un_b.s_b1 == 0 || pSource->m_nPort == 0 )
		{
			delete pSource;
			return FALSE;
		}

		//Reject if source is the local IP/port
		if ( Network.IsSelfIP( pSource->m_pAddress ) )
		{
			if ( ( pSource->m_nServerPort == 0 &&
				Settings.Connection.InPort == pSource->m_nPort ) ||
				Settings.Connection.IgnoreOwnIP )
			{
				delete pSource;
				return FALSE;
			}
		}
	}
	else if ( pSource->m_nProtocol == PROTOCOL_ED2K )
	{
		//Reject invalid server IPs (Sometimes ed2k sends invalid 0.x.x.x sources)
		if ( pSource->m_pServerAddress.S_un.S_un_b.s_b1 == 0 )
		{
			delete pSource;
			return FALSE;
		}
	}

	CQuickLock pLock( Transfers.m_pSection );

	if ( pSource->m_nRedirectionCount == 0 ) // Don't check for existing sources if source is a redirection
	{
		bool bDeleteSource = false;
		bool bHTTPSource = pSource->IsHTTPSource();
		bool bNeedHTTPSource = ! bHTTPSource &&
			Settings.Gnutella2.EnableToday &&
			VendorCache.IsExtended( pSource->m_sServer );

		// Remove unneeded sources
		for ( POSITION posSource = GetIterator() ; posSource ; )
		{
			CDownloadSource* pExisting = GetNext( posSource );

			ASSERT( pSource != pExisting );
			if ( pExisting->Equals( pSource ) ) // IPs and ports are equal
			{
				bool bExistingHTTPSource = pExisting->IsHTTPSource();
				pExisting->SetLastSeen();

				if ( bNeedHTTPSource && bExistingHTTPSource )
					bNeedHTTPSource = false;

				if ( ( pExisting->m_nProtocol == pSource->m_nProtocol ) ||
					 ( bExistingHTTPSource && bHTTPSource ) )
				{
					// Same protocol
					bDeleteSource = true;
				}
				else if ( ! pExisting->IsIdle() )
				{
					// We already downloading so we can remove new non-HTTP source
					if ( bExistingHTTPSource && ! bHTTPSource )
					{
						bDeleteSource = true;
					}
				}
				else
				{
					// We are not downloading so we can replace non-HTTP source with a new one
					if ( ! bExistingHTTPSource && bHTTPSource )
					{
						// Set connection delay the same as for the old source
						pSource->m_tAttempt = pExisting->m_tAttempt;
						pExisting->Remove( TRUE, FALSE );
					}
				}
			}
		}

		if ( bDeleteSource )
		{
			delete pSource;

			SetModified();

			return FALSE;
		}

		// Make additional G2 source from the existing non-HTTP Shareaza source
		if ( bNeedHTTPSource )
		{
			CString strURL = GetURL( pSource->m_pAddress, pSource->m_nPort );
			if ( strURL.GetLength() )
			{
				if ( CDownloadSource* pG2Source  = new CDownloadSource( (CDownload*)this, strURL ) )
				{
					pG2Source->m_sServer = pSource->m_sServer;		// Copy user-agent
					pG2Source->m_tAttempt = pSource->m_tAttempt;	// Set the same connection delay
					pG2Source->m_nProtocol = PROTOCOL_HTTP;

					AddSourceInternal( pG2Source );
				}
			}
		}
	}

	// Trimming extra sources - Idle and bad or busy
	DWORD nSourceCount = GetEffectiveSourceCount();
#ifdef _DEBUG
	FILETIME tNow;
	GetSystemTimeAsFileTime( &tNow );
#endif
	if ( nSourceCount >= Settings.Downloads.SourcesWanted )
	{
		CSortedSources oIdleSources;
		for ( POSITION posSource = m_pSources.GetTailPosition(); posSource && nSourceCount >= Settings.Downloads.SourcesWanted; )
		{
			CDownloadSource* pExisting = m_pSources.GetPrev( posSource );

			if ( pExisting->IsIdle() )
			{
				if ( pExisting->m_nFailures || pExisting->m_nBusyCount )
				{
#ifdef _DEBUG
					theApp.Message( MSG_DEBUG, _T( "Removed extra source %s %s (%I64d seconds old)" ), (LPCTSTR)CString( inet_ntoa( pExisting->m_pAddress ) ),
						(LPCTSTR)pExisting->m_oGUID.toString< Hashes::base16Encoding >().MakeUpper(),
						( *( (LONGLONG*)&tNow ) - *( (LONGLONG*)&pExisting->m_tLastSeen ) ) / 10000000 );
#endif
					RemoveSource( pExisting, FALSE );
					delete pExisting;
					nSourceCount = GetEffectiveSourceCount();
				}
				else
				{
					oIdleSources.insert( pExisting );
				}
			}
		}
		// Remove older sources first
		for ( CSortedSources::const_iterator i = oIdleSources.begin(); i != oIdleSources.end() && nSourceCount >= Settings.Downloads.SourcesWanted; ++i )
		{
			CDownloadSource* pExisting = (*i);
#ifdef _DEBUG
			theApp.Message( MSG_DEBUG, _T( "Removed extra source %s %s (%I64d seconds old)" ), (LPCTSTR)CString( inet_ntoa( pExisting->m_pAddress ) ),
				(LPCTSTR)pExisting->m_oGUID.toString< Hashes::base16Encoding >().MakeUpper(),
				( *( (LONGLONG*)&tNow ) - *( (LONGLONG*)&pExisting->m_tLastSeen ) ) / 10000000 );
#endif
			RemoveSource( pExisting, FALSE );
			delete pExisting;
			nSourceCount = GetEffectiveSourceCount();
		}
		if ( nSourceCount >= Settings.Downloads.SourcesWanted )
		{
			// Still too many sources
#ifdef _DEBUG
			theApp.Message( MSG_DEBUG, _T( "Ignored extra source %s %s due sources limit %u" ), (LPCTSTR)CString( inet_ntoa( pSource->m_pAddress ) ),
				(LPCTSTR)pSource->m_oGUID.toString< Hashes::base16Encoding >().MakeUpper(), Settings.Downloads.SourcesWanted );
#endif
			delete pSource;
			return FALSE;
		}
	}

	InternalAdd( pSource );

	SetModified();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources query for URLs

CString CDownloadWithSources::GetSourceURLs(CList< CString >* pState, int nMaximum, PROTOCOLID nProtocol, CDownloadSource* pExcept) const
{
	CQuickLock pLock( Transfers.m_pSection );

	CString strSources;

	for ( POSITION posSource = GetIterator() ; posSource ; )
	{
		CDownloadSource* pSource = GetNext( posSource );

		if ( pSource != pExcept && pSource->m_bPushOnly == FALSE &&
			 ( ( pSource->m_nFailures == 0 && pSource->m_bReadContent ) || nProtocol == PROTOCOL_NULL ) &&
			 ( pSource->m_bSHA1 || pSource->m_bTiger || pSource->m_bED2K || pSource->m_bBTH  || pSource->m_bMD5 ) &&
			 ( pState == NULL || pState->Find( pSource->m_sURL ) == NULL ) )
		{
			// Only return appropriate sources
			if ( ( nProtocol == PROTOCOL_HTTP ) && ( pSource->m_nProtocol != PROTOCOL_HTTP ) ) continue;
			if ( ( nProtocol == PROTOCOL_G1 ) && ( pSource->m_nGnutella != 1 ) ) continue;
			//if ( bHTTP && pSource->m_nProtocol != PROTOCOL_HTTP ) continue;

			if ( pState != NULL ) pState->AddTail( pSource->m_sURL );

			if ( nProtocol == PROTOCOL_G1 )
			{
				if ( strSources.GetLength() )
					strSources += ',';
				strSources += CString( inet_ntoa( pSource->m_pAddress ) );
				CString strURL;
				strURL.Format( _T("%hu"), pSource->m_nPort );
				strSources += ':' + strURL;
			}
			else if ( pSource->m_sURL.Find( _T("Zhttp://") ) >= 0 ||
				pSource->m_sURL.Find( _T("Z%2C http://") ) >= 0 )
			{
				// Ignore buggy URLs
				TRACE( "CDownloadWithSources::GetSourceURLs() Bad URL: %s\n", (LPCSTR)CT2A( pSource->m_sURL ) );
			}
			else
			{
				CString strURL = pSource->m_sURL;
				strURL.Replace( _T(","), _T("%2C") );

				if ( strSources.GetLength() ) strSources += _T(", ");
				strSources += strURL;
				strSources += ' ';
				strSources += TimeToString( &pSource->m_tLastSeen );
			}

			if ( nMaximum == 1 ) break;
			else if ( nMaximum > 1 ) nMaximum --;
		}
	}

	return strSources;
}

// Returns a string containing the most recent failed sources
CString	CDownloadWithSources::GetTopFailedSources(int nMaximum, PROTOCOLID nProtocol)
{
	// Currently we return only the string for G1, in X-NAlt format
	if ( nProtocol != PROTOCOL_G1 )
		return CString();

	CSingleLock oLock( &Transfers.m_pSection );
	if ( ! oLock.Lock( 100 ) )
		return CString();

	CString strSources, str;
	CFailedSource* pResult = NULL;

	for ( POSITION pos = m_pFailedSources.GetHeadPosition() ; pos ; )
	{
		pResult = m_pFailedSources.GetNext( pos );
		// Only return sources which we detected as failed
		if ( pResult && pResult->m_bLocal )
		{
			if ( _tcsistr( pResult->m_sURL, _T("http://") ) != NULL )
			{
				int nPos = pResult->m_sURL.Find( ':', 8 );
				if ( nPos < 0 ) continue;
				str = pResult->m_sURL.Mid( 7, nPos - 7 );
				int nPosSlash = pResult->m_sURL.Find( '/', nPos );
				if ( nPosSlash < 0 ) continue;

				if ( strSources.GetLength() )
					strSources += ',';

				strSources += str;
				str = pResult->m_sURL.Mid( nPos + 1, nPosSlash - nPos - 1 );
				strSources += ':';
				strSources += str;

				if ( nMaximum == 1 ) break;
				else if ( nMaximum > 1 ) nMaximum--;
			}
		}
	}
	return strSources;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources query hit handler

BOOL CDownloadWithSources::OnQueryHits(const CQueryHit* pHits)
{
	for ( const CQueryHit* pHit = pHits; pHit ; pHit = pHit->m_pNext )
	{
		AddSourceHit( pHit );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources remove overlapping sources

void CDownloadWithSources::RemoveOverlappingSources(QWORD nOffset, QWORD nLength)
{
	CSingleLock oLock( &Transfers.m_pSection );
	if ( ! oLock.Lock( 100 ) )
		return;

	for ( POSITION posSource = GetIterator() ; posSource ; )
	{
		CDownloadSource* pSource = GetNext( posSource );

		if ( pSource->TouchedRange( nOffset, nLength ) )
		{
			if ( GetTaskType() == dtaskMergeFile )
			{
				// Merging process can produce corrupted blocks
				// Just retry connection after 30 seconds
				pSource->m_nFailures = 0;
				pSource->Close( 30 );
			}
			else
			{
				theApp.Message( MSG_ERROR, IDS_DOWNLOAD_VERIFY_DROP,
					(LPCTSTR)CString( inet_ntoa( pSource->m_pAddress ) ),
					(LPCTSTR)pSource->m_sServer, (LPCTSTR)m_sName,
					nOffset, nOffset + nLength - 1 );
				pSource->Remove( TRUE, FALSE );
			}
		}
	}
}

// The function takes an URL and finds a failed source in the list;
// If bReliable is true, it checks only localy checked failed sources
// and those which have more than 30 votes from other users and negative
// votes compose 2/3 of the total number of votes.
CFailedSource* CDownloadWithSources::LookupFailedSource(LPCTSTR pszUrl, bool bReliable)
{
	CSingleLock oLock( &Transfers.m_pSection );
	if ( ! oLock.Lock( 100 ) )
		return NULL;

	CFailedSource* pResult = NULL;

	for ( POSITION pos = m_pFailedSources.GetHeadPosition() ; pos ; )
	{
		pResult = m_pFailedSources.GetNext( pos );
		if ( pResult && pResult->m_sURL.Compare( pszUrl ) == 0 )
		{
#ifndef NDEBUG
			theApp.Message( MSG_DEBUG, _T("Votes for file %s: negative - %i, positive - %i; offline status: %i"),
				pszUrl, pResult->m_nNegativeVotes,
				pResult->m_nPositiveVotes,
				pResult->m_bOffline );
#endif
			if ( pResult->m_bLocal )
				break;

			if ( bReliable ) // Not used at the moment anywhere, we check that explicitly
			{
				INT_PTR nTotalVotes = pResult->m_nNegativeVotes + pResult->m_nPositiveVotes;
				if ( nTotalVotes > 30 && pResult->m_nNegativeVotes / nTotalVotes > 2 / 3 )
					break;
			}
			break; // temp solution to have the same source not added more than once
				   // we should check IPs which add these sources though, since voting takes place
		}
		else
			pResult = NULL;
	}
	return pResult;
}

void CDownloadWithSources::AddFailedSource(const CDownloadSource* pSource, bool bLocal, bool bOffline)
{
	CString strURL;
	if ( pSource->m_nProtocol == PROTOCOL_BT && pSource->m_oGUID )
	{
		strURL.Format( _T("btc://%s/%s/"),
            (LPCTSTR)pSource->m_oGUID.toString(),
			(LPCTSTR)m_oBTH.toString() );
	}
	else
		strURL = pSource->m_sURL;

	AddFailedSource( (LPCTSTR)strURL, bLocal, bOffline );
}

void CDownloadWithSources::AddFailedSource(LPCTSTR pszUrl, bool bLocal, bool bOffline)
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( LookupFailedSource( pszUrl ) == NULL )
	{
		if ( CFailedSource* pBadSource = new CFailedSource( pszUrl, bLocal, bOffline ) )
		{
			m_pFailedSources.AddTail( pBadSource );
			theApp.Message( MSG_DEBUG, L"Bad sources count for \"%s\": %i. URL: %s", (LPCTSTR)m_sName, m_pFailedSources.GetCount(), pszUrl );
		}
	}
}

void CDownloadWithSources::VoteSource(LPCTSTR pszUrl, bool bPositively)
{
	if ( CFailedSource* pBadSource = LookupFailedSource( pszUrl ) )
	{
		if ( bPositively )
			pBadSource->m_nPositiveVotes++;
		else
			pBadSource->m_nNegativeVotes++;
	}
}

void CDownloadWithSources::ExpireFailedSources()
{
	ASSUME_LOCK( Transfers.m_pSection );

	DWORD tNow = GetTickCount();
	for ( POSITION pos = m_pFailedSources.GetHeadPosition() ; pos ; )
	{
		POSITION posThis = pos;
		CFailedSource* pBadSource = m_pFailedSources.GetNext( pos );
		if ( m_pFailedSources.GetAt( posThis ) == pBadSource )
		{
			// Expire bad sources added more than 2 hours ago
			if ( tNow - pBadSource->m_nTimeAdded > 2 * 3600 * 1000 )
			{
				delete pBadSource;
				m_pFailedSources.RemoveAt( posThis );
			}
			else break; // We appended to tail, so we do not need to move further
		}
	}
}

void CDownloadWithSources::ClearFailedSources()
{
	ASSUME_LOCK( Transfers.m_pSection );

	for ( POSITION pos = m_pFailedSources.GetHeadPosition() ; pos ; )
	{
		POSITION posThis = pos;
		CFailedSource* pBadSource = m_pFailedSources.GetNext( pos );
		if ( m_pFailedSources.GetAt( posThis ) == pBadSource )
		{
			delete pBadSource;
			m_pFailedSources.RemoveAt( posThis );
		}
	}
}

void CDownloadWithSources::InternalAdd(CDownloadSource* pSource)
{
	ASSUME_LOCK( Transfers.m_pSection );

	ASSERT( m_pSources.Find( pSource ) == NULL );
	m_pSources.AddTail( pSource );

	switch ( pSource->m_nProtocol )
	{
	case PROTOCOL_G1:
		m_nG1SourceCount++;
		break;
	case PROTOCOL_G2:
		m_nG2SourceCount++;
		break;
	case PROTOCOL_ED2K:
		m_nEdSourceCount++;
		break;
	case PROTOCOL_HTTP:
		m_nHTTPSourceCount++;
		break;
	case PROTOCOL_BT:
		m_nBTSourceCount++;
		break;
	case PROTOCOL_FTP:
		m_nFTPSourceCount++;
		break;
	case PROTOCOL_DC:
		m_nDCSourceCount++;
		break;
	default:
		ASSERT( FALSE );
	}
}

void CDownloadWithSources::InternalRemove(CDownloadSource* pSource)
{
	ASSUME_LOCK( Transfers.m_pSection );

	POSITION posSource = m_pSources.Find( pSource );
	ASSERT( posSource != NULL );
	m_pSources.RemoveAt( posSource );

	switch ( pSource->m_nProtocol )
	{
	case PROTOCOL_G1:
		m_nG1SourceCount--;
		break;
	case PROTOCOL_G2:
		m_nG2SourceCount--;
		break;
	case PROTOCOL_ED2K:
		m_nEdSourceCount--;
		break;
	case PROTOCOL_HTTP:
		m_nHTTPSourceCount--;
		break;
	case PROTOCOL_BT:
		m_nBTSourceCount--;
		break;
	case PROTOCOL_FTP:
		m_nFTPSourceCount--;
		break;
	case PROTOCOL_DC:
		m_nDCSourceCount--;
		break;
	default:
		;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources remove a source

void CDownloadWithSources::RemoveSource(CDownloadSource* pSource, BOOL bBan)
{
	ASSUME_LOCK( Transfers.m_pSection );

	InternalRemove( pSource );

	if ( bBan && pSource->m_sURL.GetLength() )
	{
		AddFailedSource( pSource );
	}

	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources sort a source

void CDownloadWithSources::SortSource(CDownloadSource* pSource, BOOL bTop)
{
	CQuickLock pLock( Transfers.m_pSection );

	if ( m_pSources.GetCount() == 1 )
		// No sorting
		return;

	POSITION posSource = m_pSources.Find( pSource );
	ASSERT( posSource );

	m_pSources.RemoveAt( posSource );

	if ( bTop )
		m_pSources.AddHead( pSource );
	else
		m_pSources.AddTail( pSource );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources sort a source by state (Downloading, etc...)

void CDownloadWithSources::SortSource(CDownloadSource* pSource)
{
	CQuickLock pLock( Transfers.m_pSection );

	if ( m_pSources.GetCount() == 1 )
		// No sorting
		return;

	POSITION posSource = m_pSources.Find( pSource );
	ASSERT( posSource );

	m_pSources.RemoveAt( posSource );

	// Run through the sources to the correct position
	for ( POSITION posCompare = GetIterator() ; posCompare ; )
	{
		posSource = posCompare;
		CDownloadSource* pCompare = GetNext( posCompare );

		if ( pCompare->m_nSortOrder >= pSource->m_nSortOrder )
			break;
	}

	// Insert source in front of current compare source
	m_pSources.InsertBefore( posSource, pSource );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources source colour selector

COLORREF CDownloadWithSources::GetSourceColour()
{
	int r = 0, g = 0, b = 0;

	BOOL bGood = FALSE;
	for ( int i = 0; ! bGood && i < 100; ++i )
	{
		bGood = TRUE;

		do
		{
			r = GetRandomNum( 0, 255 );
			g = GetRandomNum( 0, 255 );
			b = GetRandomNum( 0, 255 );
		}
		while ( r + g + b < 96 || r + g + b > 224 * 3 ||	// Too dark or too bright
			( abs( r - g ) < 24 && abs( r - b ) < 24 ) );	// Too gray
		int d = min( min( r, g ), b );

		CSingleLock oLock( &Transfers.m_pSection );
		if ( oLock.Lock( 100 ) )
		{
			for ( POSITION posSource = GetIterator() ; posSource ; )
			{
				CDownloadSource* pSource = GetNext( posSource );

				if ( pSource->m_crColour )
				{
					int sr = GetRValue( pSource->m_crColour );
					int sg = GetGValue( pSource->m_crColour );
					int sb = GetBValue( pSource->m_crColour );
					int sd = min( min( sr, sg ), sb );
					sr -= sd;
					sg -= sd;
					sb -= sd;
					if ( abs( r - d - sr ) < 24 &&
						 abs( g - d - sg ) < 24 &&
						 abs( b - d - sb ) < 24 )
					{
						// Too similar
						bGood = FALSE;
						break;
					}
				}
			}
		}
	}

	return RGB( r, g ,b );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources serialize

void CDownloadWithSources::Serialize(CArchive& ar, int nVersion /* DOWNLOAD_SER_VERSION */)
{
	CDownloadBase::Serialize( ar, nVersion );

	CQuickLock pLock( Transfers.m_pSection );

	if ( ar.IsStoring() )
	{
		// Don't save more than 500 sources
		DWORD_PTR nSources = min( (DWORD_PTR)GetCount(), (DWORD_PTR)Settings.Downloads.SourcesWanted );
		ar.WriteCount( nSources );

		for ( POSITION posSource = GetIterator() ; posSource && nSources ; nSources-- )
		{
			CDownloadSource* pSource = GetNext( posSource );

			pSource->Serialize( ar, nVersion );
		}

		ar.WriteCount( m_pXML != NULL ? 1 : 0 );
		if ( m_pXML ) m_pXML->Serialize( ar );
	}
	else
	{
		for ( DWORD_PTR nSources = ar.ReadCount() ; nSources ; nSources-- )
		{
			// Create new source
			CAutoPtr< CDownloadSource > pSource( new CDownloadSource( static_cast< CDownload* >( this ) ) );
			if ( ! pSource )
				AfxThrowMemoryException();

			// Load details from disk
			pSource->Serialize( ar, nVersion );

			// Extract ed2k client ID from url (m_pAddress) because it wasn't saved
			if ( ( !pSource->m_nPort ) && ( _tcsnicmp( pSource->m_sURL, _T("ed2kftp://"), 10 ) == 0 )  )
			{
				CString strURL = pSource->m_sURL.Mid(10);
				if ( strURL.GetLength())
					_stscanf( strURL, _T("%lu"), &pSource->m_pAddress.S_un.S_addr );
			}

			// Add to the list no more than 500 sources
			if ( nSources < (DWORD_PTR)Settings.Downloads.SourcesWanted )
				InternalAdd( pSource.Detach() );
		}

		if ( ar.ReadCount() )
		{
			m_pXML = new CXMLElement();
			if ( ! m_pXML )
				AfxThrowMemoryException();

			m_pXML->Serialize( ar );
		}
	}
}

void CDownloadWithSources::MergeMetadata(const CXMLElement* pXML)
{
	CQuickLock pLock( Transfers.m_pSection );

	if ( m_pXML )
	{
		const CXMLAttribute* pAttr1 = m_pXML->GetAttribute( CXMLAttribute::schemaName );
		const CXMLAttribute* pAttr2 = pXML->GetAttribute( CXMLAttribute::schemaName );
		if ( pAttr1 && pAttr2 && pAttr1->GetValue().CompareNoCase( pAttr2->GetValue() ) == 0 )
		{
			CXMLElement* pElement1 = m_pXML->GetFirstElement();
			const CXMLElement* pElement2 = pXML->GetFirstElement();
			if ( pElement1 && pElement2 )
			{
				pElement1->Merge( pElement2 );
			}
		}
	}
	else
	{
		m_pXML = pXML->Clone();
	}
}
