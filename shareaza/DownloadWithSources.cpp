//
// DownloadWithSources.cpp
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
#include "Shareaza.h"
#include "Settings.h"
#include "Downloads.h"
#include "DownloadWithSources.h"
#include "DownloadSource.h"
#include "Network.h"
#include "Neighbours.h"
#include "Transfer.h"
#include "QueryHit.h"
#include "SourceURL.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "XML.h"
#include "SHA.h"
#include "MD4.h"
#include "TigerTree.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadWithSources construction

CDownloadWithSources::CDownloadWithSources()
{
	m_pSourceFirst	= NULL;
	m_pSourceLast	= NULL;
	m_nSourceCount	= 0;
	m_pXML			= NULL;
}

CDownloadWithSources::~CDownloadWithSources()
{
	ClearSources();
	if ( m_pXML != NULL ) delete m_pXML;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources list access

int CDownloadWithSources::GetSourceCount(BOOL bNoPush, BOOL bSane) const
{
	if ( ! bNoPush && ! bSane ) return m_nSourceCount;
	
	DWORD tNow = GetTickCount();
	int nCount = 0;
	
	for ( CDownloadSource* pSource = m_pSourceFirst ; pSource ; pSource = pSource->m_pNext )
	{
		if ( ! bNoPush || ! pSource->m_bPushOnly )
		{
			if ( ! bSane ||
				 pSource->m_tAttempt < tNow ||
				 pSource->m_tAttempt - tNow <= 900000 )
			{
				nCount++;
			}
		}
	}
	
	return nCount;
}

int CDownloadWithSources::GetED2KCompleteSourceCount() const
{

	DWORD tNow = GetTickCount();
	int nCount = 0;
	
	for ( CDownloadSource* pSource = m_pSourceFirst ; pSource ; pSource = pSource->m_pNext )
	{
		if ( ( ! pSource->m_bPushOnly ) && //Push sources can't be counted since you often cannot reach them
			 ( pSource->m_tAttempt < tNow || pSource->m_tAttempt - tNow <= 900000 ) &&	//This source is probably dead
			 ( pSource->m_nProtocol == PROTOCOL_ED2K ) && //Only counting ed2k sources
             ( pSource->m_oAvailable.empty() ) ) //Only counting complete sources
			
		{
			nCount++;
		}
	}
	
	CString strT;
	strT.Format(_T("Complete ed2k sources: %i"), nCount);
	theApp.Message( MSG_ERROR, strT );
	return nCount;
}

BOOL CDownloadWithSources::CheckSource(CDownloadSource* pCheck) const
{
	for ( CDownloadSource* pSource = m_pSourceFirst ; pSource ; pSource = pSource->m_pNext )
	{
		if ( pSource == pCheck ) return TRUE;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources clear

void CDownloadWithSources::ClearSources()
{
	for ( CDownloadSource* pSource = GetFirstSource() ; pSource ; )
	{
		CDownloadSource* pNext = pSource->m_pNext;
		delete pSource;
		pSource = pNext;
	}
	
	m_pSourceFirst = m_pSourceLast = NULL;
	m_nSourceCount = 0;
	
	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources add a query-hit source

BOOL CDownloadWithSources::AddSourceHit(CQueryHit* pHit, BOOL bForce)
{
	BOOL bHash = FALSE;
	
	if ( ! bForce )
	{
		if ( m_bSHA1 && pHit->m_bSHA1 )
		{
			if ( m_pSHA1 != pHit->m_pSHA1 ) return FALSE;
			bHash = TRUE;
		}
		else if ( m_bTiger && pHit->m_bTiger )
		{
			if ( m_pTiger != pHit->m_pTiger ) return FALSE;
			bHash = TRUE;
		}
		if ( m_bED2K && pHit->m_bED2K )
		{
			if ( m_pED2K != pHit->m_pED2K ) return FALSE;
			bHash = TRUE;
		}
		if ( m_bBTH && pHit->m_bBTH )
		{
			if ( m_pBTH != pHit->m_pBTH ) return FALSE;
			bHash = TRUE;
		}
	}
	
	if ( ! bHash && ! bForce )
	{
		if ( Settings.General.HashIntegrity ) return FALSE;
		
		if ( m_sRemoteName.IsEmpty() || pHit->m_sName.IsEmpty() ) return FALSE;
		if ( m_nSize == SIZE_UNKNOWN || ! pHit->m_bSize ) return FALSE;
		
		if ( m_nSize != pHit->m_nSize ) return FALSE;
		if ( m_sRemoteName.CompareNoCase( pHit->m_sName ) ) return FALSE;
	}
	
	if ( ! m_bSHA1 && pHit->m_bSHA1 )
	{
		m_bSHA1 = TRUE;
		m_pSHA1 = pHit->m_pSHA1;
	}
	if ( ! m_bTiger && pHit->m_bTiger )
	{
		m_bTiger = TRUE;
		m_pTiger = pHit->m_pTiger;
	}
	if ( ! m_bED2K && pHit->m_bED2K )
	{
		m_bED2K = TRUE;
		m_pED2K = pHit->m_pED2K;
	}
	
	if ( m_nSize == SIZE_UNKNOWN && pHit->m_bSize )
	{
		m_nSize = pHit->m_nSize;
	}
	
	if ( m_sRemoteName.IsEmpty() && pHit->m_sName.GetLength() )
	{
		m_sRemoteName = pHit->m_sName;
	}
	
	if ( Settings.Downloads.Metadata && m_pXML == NULL )
	{
		if ( pHit->m_pXML != NULL && pHit->m_sSchemaPlural.GetLength() )
		{
			m_pXML = new CXMLElement( NULL, pHit->m_sSchemaPlural );
			m_pXML->AddAttribute( _T("xmlns:xsi"), CXMLAttribute::xmlnsInstance );
			m_pXML->AddAttribute( CXMLAttribute::schemaName, pHit->m_sSchemaURI );
			m_pXML->AddElement( pHit->m_pXML->Clone() );
			
			if ( CSchema* pSchema = SchemaCache.Get( pHit->m_sSchemaURI ) )
			{
				pSchema->Validate( m_pXML, TRUE );
			}
		}
	}

	/*
	if ( pHit->m_nProtocol == PROTOCOL_ED2K )
	{
		Neighbours.FindDonkeySources( &pHit->m_pED2K,
			(IN_ADDR*)pHit->m_pClientID.w, (WORD)pHit->m_pClientID.w[1] );
	}
	*/

	// No URL, stop now with success
	if ( pHit->m_sURL.IsEmpty() ) return TRUE;
	
	return AddSourceInternal( new CDownloadSource( (CDownload*)this, pHit ) );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources add miscellaneous sources

BOOL CDownloadWithSources::AddSourceED2K(DWORD nClientID, WORD nClientPort, DWORD nServerIP, WORD nServerPort, GGUID* pGUID)
{
	return AddSourceInternal( new CDownloadSource( (CDownload*)this, nClientID, nClientPort, nServerIP, nServerPort, pGUID ) );
}

BOOL CDownloadWithSources::AddSourceBT(SHA1* pGUID, IN_ADDR* pAddress, WORD nPort)
{
	return AddSourceInternal( new CDownloadSource( (CDownload*)this, pGUID, pAddress, nPort ) );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources add a single URL source

BOOL CDownloadWithSources::AddSourceURL(LPCTSTR pszURL, BOOL bURN, FILETIME* pLastSeen)
{
	if ( pszURL == NULL ) return FALSE;
	if ( *pszURL == 0 ) return FALSE;
	
	BOOL bHashAuth = FALSE;
	CSourceURL pURL;
	
	if ( *pszURL == '@' )
	{
		bHashAuth = TRUE;
		pszURL++;
	}
	
	if ( ! pURL.Parse( pszURL ) ) return FALSE;
	
	if ( bURN )
	{
		if ( pURL.m_pAddress.S_un.S_addr == Network.m_pHost.sin_addr.S_un.S_addr ) return FALSE;
		if ( Network.IsFirewalledAddress( &pURL.m_pAddress, TRUE ) ) return FALSE;
	}
	
	if ( m_pFailedSources.Find( pszURL ) != NULL ) return FALSE;
	
	if ( pURL.m_bSHA1 && m_bSHA1 )
	{
		if ( m_pSHA1 != pURL.m_pSHA1 ) return FALSE;
	}
	
	if ( m_sRemoteName.IsEmpty() && _tcslen( pszURL ) > 9 )
	{
		m_sRemoteName = &pszURL[8];
		
		int nPos = m_sRemoteName.ReverseFind( '/' );
		
		if ( nPos >= 0 )
		{
			m_sRemoteName = m_sRemoteName.Mid( nPos + 1 ).SpanExcluding( _T("?") );
			m_sRemoteName = CTransfer::URLDecode( m_sRemoteName );
		}
		else
		{
			m_sRemoteName.Empty();
		}
		
		if ( m_sRemoteName.IsEmpty() ) m_sRemoteName = _T("default.htm");
	}
	
	return AddSourceInternal( new CDownloadSource( (CDownload*)this, pszURL, bURN, bHashAuth, pLastSeen ) );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources add several URL sources

int CDownloadWithSources::AddSourceURLs(LPCTSTR pszURLs, BOOL bURN)
{
	CString strURLs( pszURLs );
	BOOL bQuote = FALSE;
	
	for ( int nScan = 0 ; nScan < strURLs.GetLength() ; nScan++ )
	{
		if ( strURLs[ nScan ] == '\"' )
		{
			bQuote = ! bQuote;
			strURLs.SetAt( nScan, ' ' );
		}
		else if ( strURLs[ nScan ] == ',' && bQuote )
		{
			strURLs.SetAt( nScan, '`' );
		}
	}
	
	strURLs += ',';
	
    int nCount = 0;
	for ( ; ; )
	{
		int nPos = strURLs.Find( ',' );
		if ( nPos < 0 ) break;
		
		CString strURL	= strURLs.Left( nPos );
		strURLs			= strURLs.Mid( nPos + 1 );
		strURL.TrimLeft();
		
		FILETIME tSeen = { 0, 0 };
		BOOL bSeen = FALSE;
		
		if ( _tcsistr( strURL, _T("://") ) != NULL )
		{
			nPos = strURL.ReverseFind( ' ' );
			
			if ( nPos > 0 )
			{
				CString strTime = strURL.Mid( nPos + 1 );
				strURL = strURL.Left( nPos );
				strURL.TrimRight();
				bSeen = TimeFromString( strTime, &tSeen );
			}
			
			for ( int nScan = 0 ; nScan < strURL.GetLength() ; nScan++ )
			{
				if ( strURL[ nScan ] == '`' ) strURL.SetAt( nScan, ',' );
			}
		}
		else
		{
			nPos = strURL.Find( ':' );
			if ( nPos < 1 ) continue;
			
			int nPort = 0;
			_stscanf( strURL.Mid( nPos + 1 ), _T("%i"), &nPort );
			strURL.Truncate( nPos );
			USES_CONVERSION;
			DWORD nAddress = inet_addr( T2CA( strURL ) );
			strURL.Empty();
			
			if ( ! Network.IsFirewalledAddress( &nAddress, TRUE ) && nPort != 0 && nAddress != INADDR_NONE )
			{
				if ( m_bSHA1 )
				{
					strURL.Format( _T("http://%s:%i/uri-res/N2R?%s"),
						(LPCTSTR)CString( inet_ntoa( *(IN_ADDR*)&nAddress ) ),
						nPort, (LPCTSTR)CSHA::HashToString( &m_pSHA1, TRUE ) );
				}
			}
		}
		
		if ( AddSourceURL( strURL, bURN, bSeen ? &tSeen : NULL ) ) nCount++;
	}
	
	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources internal source adder

BOOL CDownloadWithSources::AddSourceInternal(CDownloadSource* pSource)
{
	//Check/Reject if source is invalid
	if ( ! pSource->m_bPushOnly )
	{
		//Reject invalid IPs (Sometimes ed2k sends invalid 0.x.x.x sources)
		if ( pSource->m_pAddress.S_un.S_un_b.s_b1 == 0 )
		{
			delete pSource;
			return FALSE;
		}

		//Reject if source is the local IP/port
		if ( Network.m_pHost.sin_addr.S_un.S_addr == pSource->m_pAddress.S_un.S_addr )
		{
			if ( ( ( pSource->m_nServerPort == 0 ) && (Settings.Connection.InPort == pSource->m_nPort ) )
				|| ( Settings.Connection.IgnoreOwnIP ) )
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

	for ( CDownloadSource* pExisting = m_pSourceFirst ; pExisting ; pExisting = pExisting->m_pNext )
	{	
		if ( pExisting->Equals( pSource ) )
		{
			if ( pExisting->m_pTransfer != NULL ||
				 ( pExisting->m_nProtocol == PROTOCOL_HTTP && pSource->m_nProtocol != PROTOCOL_HTTP ) )
			{
				delete pSource;
				return FALSE;
			}
			else
			{
				pSource->m_tAttempt = pExisting->m_tAttempt;
				pExisting->Remove( TRUE, FALSE );
				break;
			}
		}
	}
	
	m_nSourceCount ++;

	pSource->m_pPrev = m_pSourceLast;
	pSource->m_pNext = NULL;
		
	if ( m_pSourceLast != NULL )
	{
		m_pSourceLast->m_pNext = pSource;
		m_pSourceLast = pSource;
	}
	else
	{
		m_pSourceFirst = m_pSourceLast = pSource;
	}
	
	SetModified();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources query for URLs

CString CDownloadWithSources::GetSourceURLs(CStringList* pState, int nMaximum, BOOL bHTTP, CDownloadSource* pExcept)
{
	CString strSources, strURL;
	
	for ( CDownloadSource* pSource = GetFirstSource() ; pSource ; pSource = pSource->m_pNext )
	{
		if ( pSource != pExcept && pSource->m_bPushOnly == FALSE &&
			 pSource->m_nFailures == 0 && pSource->m_bReadContent &&
			 ( pSource->m_bSHA1 || pSource->m_bED2K ) &&
			 ( pState == NULL || pState->Find( pSource->m_sURL ) == NULL ) )
		{
			if ( pState != NULL ) pState->AddTail( pSource->m_sURL );
			
			if ( bHTTP && pSource->m_nProtocol != PROTOCOL_HTTP ) continue;
			
			strURL = pSource->m_sURL;
			Replace( strURL, _T(","), _T("%2C") );
			
			if ( strSources.GetLength() > 0 ) strSources += _T(", ");
			strSources += strURL;
			strSources += ' ';
			strSources += TimeToString( &pSource->m_tLastSeen );
			
			if ( nMaximum == 1 ) break;
			else if ( nMaximum > 1 ) nMaximum --;
		}
	}
	
	if ( strSources.Find( _T("Zhttp://") ) >= 0 ) strSources.Empty();
	
	return strSources;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources query hit handler

BOOL CDownloadWithSources::OnQueryHits(CQueryHit* pHits)
{
	for ( ; pHits ; pHits = pHits->m_pNext )
	{
		if ( pHits->m_sURL.GetLength() ) AddSourceHit( pHits );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources remove overlapping sources

void CDownloadWithSources::RemoveOverlappingSources(QWORD nOffset, QWORD nLength)
{
	for ( CDownloadSource* pSource = GetFirstSource() ; pSource ; )
	{
		CDownloadSource* pNext = pSource->m_pNext;
		
		if ( pSource->TouchedRange( nOffset, nLength ) )
		{
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_VERIFY_DROP,
				(LPCTSTR)CString( inet_ntoa( pSource->m_pAddress ) ),
				(LPCTSTR)pSource->m_sServer, (LPCTSTR)m_sRemoteName,
				nOffset, nOffset + nLength - 1 );
			pSource->Remove( TRUE, TRUE );
		}
		
		pSource = pNext;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources remove a source

void CDownloadWithSources::RemoveSource(CDownloadSource* pSource, BOOL bBan)
{
	if ( bBan && pSource->m_sURL.GetLength() )
	{
		m_pFailedSources.AddTail( pSource->m_sURL );
	}
	
	ASSERT( m_nSourceCount > 0 );
	m_nSourceCount --;
	
	if ( pSource->m_pPrev != NULL )
		pSource->m_pPrev->m_pNext = pSource->m_pNext;
	else
		m_pSourceFirst = pSource->m_pNext;
	
	if ( pSource->m_pNext != NULL )
		pSource->m_pNext->m_pPrev = pSource->m_pPrev;
	else
		m_pSourceLast = pSource->m_pPrev;
	
	delete pSource;
	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources sort a source

void CDownloadWithSources::SortSource(CDownloadSource* pSource, BOOL bTop)
{
	ASSERT( m_nSourceCount > 0 );
	
	if ( pSource->m_pPrev != NULL )
		pSource->m_pPrev->m_pNext = pSource->m_pNext;
	else
		m_pSourceFirst = pSource->m_pNext;
	
	if ( pSource->m_pNext != NULL )
		pSource->m_pNext->m_pPrev = pSource->m_pPrev;
	else
		m_pSourceLast = pSource->m_pPrev;
	
	if ( ! bTop )
	{
		pSource->m_pPrev = m_pSourceLast;
		pSource->m_pNext = NULL;
		
		if ( m_pSourceLast != NULL )
		{
			m_pSourceLast->m_pNext = pSource;
			m_pSourceLast = pSource;
		}
		else
		{
			m_pSourceFirst = m_pSourceLast = pSource;
		}
	}
	else
	{
		pSource->m_pPrev = NULL;
		pSource->m_pNext = m_pSourceFirst;
		
		if ( m_pSourceFirst != NULL )
		{
			m_pSourceFirst->m_pPrev = pSource;
			m_pSourceFirst = pSource;
		}
		else
		{
			m_pSourceFirst = m_pSourceLast = pSource;
		}
	}
}


//////////////////////////////////////////////////////////////////////
// CDownloadWithSources sort a source by state (Downloading, etc...)

void CDownloadWithSources::SortSource(CDownloadSource* pSource)
{
	ASSERT( m_nSourceCount > 0 );

	//Remove source from current position. (It's unsorted, and would interfere with sort)
	if ( pSource->m_pPrev != NULL )
		pSource->m_pPrev->m_pNext = pSource->m_pNext;
	else
		m_pSourceFirst = pSource->m_pNext;
	
	if ( pSource->m_pNext != NULL )
		pSource->m_pNext->m_pPrev = pSource->m_pPrev;
	else
		m_pSourceLast = pSource->m_pPrev;
	


	if ( ( m_pSourceFirst == NULL ) || ( m_pSourceLast == NULL ) )
	{	//Only one source
		m_pSourceFirst = m_pSourceLast = pSource;
		pSource->m_pNext = pSource->m_pPrev = NULL;
	}
	else
	{	//Sort sources
		CDownloadSource* pCompare = m_pSourceFirst;

		while ( ( pCompare != NULL ) && (pCompare->m_nSortOrder < pSource->m_nSortOrder) )
			pCompare = pCompare->m_pNext; //Run through the sources to the correct position

		

		if ( pCompare == NULL )
		{	//Source is last on list
			m_pSourceLast->m_pNext = pSource;
			pSource->m_pPrev = m_pSourceLast;
			pSource->m_pNext = NULL;
			m_pSourceLast = pSource;
		}
		else
		{	//Insert source in front of current compare source
			if ( pCompare->m_pPrev == NULL )
				m_pSourceFirst = pSource;
			else
				pCompare->m_pPrev->m_pNext = pSource;

			pSource->m_pNext = pCompare;
			pSource->m_pPrev = pCompare->m_pPrev;
			pCompare->m_pPrev= pSource;
		}

	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources source colour selector

#define SRC_COLOURS 6

int CDownloadWithSources::GetSourceColour()
{
	BOOL bTaken[SRC_COLOURS];
	ZeroMemory( bTaken, sizeof(BOOL) * SRC_COLOURS );
	int nFree = SRC_COLOURS;
	
	for ( CDownloadSource* pSource = GetFirstSource() ; pSource ; pSource = pSource->m_pNext )
	{
		if ( pSource->m_nColour >= 0 )
		{
			if ( bTaken[ pSource->m_nColour ] == FALSE )
			{
				bTaken[ pSource->m_nColour ] = TRUE;
				nFree--;
			}
		}
	}
	
	srand( GetTickCount() + (DWORD)this );
	
	if ( nFree == 0 ) return rand() % SRC_COLOURS;
	
	nFree = rand() % nFree;
	
	for ( int nColour = 0 ; nColour < SRC_COLOURS ; nColour++ )
	{
		if ( bTaken[ nColour ] == FALSE )
		{
			if ( nFree-- == 0 ) return nColour;
		}
	}
	
	return rand() % SRC_COLOURS;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithSources serialize

void CDownloadWithSources::Serialize(CArchive& ar, int nVersion)
{
	CDownloadBase::Serialize( ar, nVersion );
	
	if ( ar.IsStoring() )
	{
		ar.WriteCount( GetSourceCount() );
		
		for ( CDownloadSource* pSource = GetFirstSource() ; pSource ; pSource = pSource->m_pNext )
		{
			pSource->Serialize( ar, nVersion );
		}
		
		ar.WriteCount( m_pXML != NULL ? 1 : 0 );
		if ( m_pXML ) m_pXML->Serialize( ar );
	}
	else
	{
		for ( int nSources = ar.ReadCount() ; nSources ; nSources-- )
		{
			// Create new source
			CDownloadSource* pSource = new CDownloadSource( (CDownload*)this );
			
			// Add to the list
			m_nSourceCount ++;
			pSource->m_pPrev = m_pSourceLast;
			pSource->m_pNext = NULL;
			
			if ( m_pSourceLast != NULL )
			{
				m_pSourceLast->m_pNext = pSource;
				m_pSourceLast = pSource;
			}
			else
			{
				m_pSourceFirst = m_pSourceLast = pSource;
			}

			// Load details from disk
			pSource->Serialize( ar, nVersion );

			// Extract ed2k client ID from url (m_pAddress) because it wasn't saved
			if ( ( !pSource->m_nPort ) && ( _tcsnicmp( pSource->m_sURL, _T("ed2kftp://"), 10 ) == 0 )  )
			{
				CString strURL = pSource->m_sURL.Mid(10);
				if ( strURL.GetLength())
					_stscanf( strURL, _T("%lu"), &pSource->m_pAddress.S_un.S_addr );
			}
		}
		
		if ( ar.ReadCount() )
		{
			m_pXML = new CXMLElement();
			m_pXML->Serialize( ar );
		}
	}
}
