//
// DownloadSource.cpp
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
#include "Download.h"
#include "Downloads.h"
#include "DownloadSource.h"
#include "DownloadTransferHTTP.h"
//#include "DownloadTransferFTP.h"
#include "DownloadTransferED2K.h"
#include "DownloadTransferBT.h"
#include "FragmentedFile.h"

#include "Neighbours.h"
#include "QueryHit.h"
#include "Network.h"
#include "VendorCache.h"
#include "EDClients.h"
#include "EDClient.h"
#include "EDPacket.h"
#include "SourceURL.h"
#include "SHA.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadSource construction

CDownloadSource::CDownloadSource(CDownload* pDownload)
{
	Construct( pDownload );
}

void CDownloadSource::Construct(CDownload* pDownload)
{
	ASSERT( pDownload != NULL );
	
	m_pDownload		= pDownload;
	m_pPrev			= NULL;
	m_pNext			= NULL;
	m_pTransfer		= NULL;
	m_bSelected		= FALSE;
	
	m_nProtocol		= PROTOCOL_NULL;
	m_bGUID			= FALSE;
	m_nPort			= 0;
	m_nServerPort	= 0;
	
	m_nIndex		= 0;
	m_bHashAuth		= FALSE;
	m_bSHA1			= FALSE;
	m_bTiger		= FALSE;
	m_bED2K			= FALSE;
	
	m_nSpeed		= 0;
	m_bPushOnly		= FALSE;
	m_bCloseConn	= FALSE;
	m_bReadContent	= FALSE;
	m_nGnutella		= 0;
	m_bClientExtended=FALSE;
	
	m_nSortOrder	= 0xFFFFFFFF;
	m_nColour		= -1;
	m_tAttempt		= 0;
	m_nFailures		= 0;

	m_pPastFragment	= NULL;
	m_pAvailable	= NULL;
	
	SYSTEMTIME pTime;
	GetSystemTime( &pTime );
	SystemTimeToFileTime( &pTime, &m_tLastSeen );
}

CDownloadSource::~CDownloadSource()
{
	m_pPastFragment->DeleteChain();
	m_pAvailable->DeleteChain();
}

//////////////////////////////////////////////////////////////////////
// CDownloadSource construction from a query hit

CDownloadSource::CDownloadSource(CDownload* pDownload, CQueryHit* pHit)
{
	Construct( pDownload );
	
	m_bPushOnly	= pHit->m_bPush == TS_TRUE ? TRUE : FALSE;
	
	m_sURL		= pHit->m_sURL;
	m_pAddress	= pHit->m_pAddress;	// Not needed?
	m_nPort		= pHit->m_nPort;	// Not needed?
	m_nSpeed	= pHit->m_bMeasured == TS_TRUE ? ( pHit->m_nSpeed * 128 ) : 0;
	m_sServer	= pHit->m_pVendor->m_sName;
	m_sName		= pHit->m_sName;
	m_nIndex	= pHit->m_nIndex;
	m_bSHA1		= pHit->m_bSHA1;
	m_bTiger	= pHit->m_bTiger;
	m_bED2K		= pHit->m_bED2K;
	
	if ( pHit->m_nProtocol == PROTOCOL_G1 || pHit->m_nProtocol == PROTOCOL_G2 )
	{
		m_bGUID = TRUE;
		m_pGUID = pHit->m_pClientID;
		m_bClientExtended = TRUE;
	}
	else if ( pHit->m_nProtocol == PROTOCOL_ED2K )
	{
		if ( ( m_sURL.Right( 3 ) == _T("/0/") ) && ( pDownload->m_nSize ) )
		{	//Add the size if it was missing.
			CString strTemp =  m_sURL.Left( m_sURL.GetLength() - 2 );
			m_sURL.Format( _T("%s%I64i/"), strTemp, pDownload->m_nSize );
		}
	}
	
	ResolveURL();
}

//////////////////////////////////////////////////////////////////////
// CDownloadSource construction from eDonkey source transfer

CDownloadSource::CDownloadSource(CDownload* pDownload, DWORD nClientID, WORD nClientPort, DWORD nServerIP, WORD nServerPort, GGUID* pGUID)
{
	Construct( pDownload );
	
	if ( m_bPushOnly = CEDPacket::IsLowID( nClientID ) )
	{
		m_sURL.Format( _T("ed2kftp://%lu@%s:%i/%s/%I64i/"),
			nClientID,
			(LPCTSTR)CString( inet_ntoa( (IN_ADDR&)nServerIP ) ), nServerPort,
			(LPCTSTR)CED2K::HashToString( &m_pDownload->m_pED2K ), m_pDownload->m_nSize );
	}
	else
	{
		m_sURL.Format( _T("ed2kftp://%s:%i/%s/%I64i/"),
			(LPCTSTR)CString( inet_ntoa( (IN_ADDR&)nClientID ) ), nClientPort,
			(LPCTSTR)CED2K::HashToString( &m_pDownload->m_pED2K ), m_pDownload->m_nSize );
	}
	
	if ( m_bGUID = ( pGUID != NULL ) ) m_pGUID = *pGUID;
	
	m_bED2K		= TRUE;
	m_sServer	= _T("eDonkey2000");
	
	ResolveURL();
}

//////////////////////////////////////////////////////////////////////
// CDownloadSource construction from BitTorrent

CDownloadSource::CDownloadSource(CDownload* pDownload, SHA1* pGUID, IN_ADDR* pAddress, WORD nPort)
{
	Construct( pDownload );
	
	if ( pGUID != NULL )
	{
		m_sURL.Format( _T("btc://%s:%i/%s/%s/"),
			(LPCTSTR)CString( inet_ntoa( *pAddress ) ), nPort,
			(LPCTSTR)CSHA::HashToString( pGUID ),
			(LPCTSTR)CSHA::HashToString( &pDownload->m_pBTH ) );
	}
	else
	{
		m_sURL.Format( _T("btc://%s:%i//%s/"),
			(LPCTSTR)CString( inet_ntoa( *pAddress ) ), nPort,
			(LPCTSTR)CSHA::HashToString( &pDownload->m_pBTH ) );
	}
	
	m_bGUID		= pGUID != NULL;
	m_sServer	= _T("BitTorrent");
	
	ResolveURL();
}

//////////////////////////////////////////////////////////////////////
// CDownloadSource construction from URL

CDownloadSource::CDownloadSource(CDownload* pDownload, LPCTSTR pszURL, BOOL bSHA1, BOOL bHashAuth, FILETIME* pLastSeen)
{
	Construct( pDownload );
	
	ASSERT( pszURL != NULL );
	m_sURL = pszURL;
	
	if ( ! ResolveURL() ) return;
	
	//m_bSHA1			= bSHA1; //Done in ResolveURL now
	m_bHashAuth		= bHashAuth;
	
	if ( pLastSeen != NULL )
	{
		FILETIME tNow = m_tLastSeen;
		(LONGLONG&)tNow += 10000000;
		if ( CompareFileTime( pLastSeen, &tNow ) <= 0 ) m_tLastSeen = *pLastSeen;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadSource URL resolver

BOOL CDownloadSource::ResolveURL()
{
	CSourceURL pURL;
	
	if ( ! pURL.Parse( m_sURL ) )
	{
		theApp.Message( MSG_ERROR, _T("Unable to parse URL: %s"), (LPCTSTR)m_sURL );
		return FALSE;
	}
	
	m_bSHA1		|= pURL.m_bSHA1;
	m_bED2K		|= pURL.m_bED2K;

	m_nProtocol	= pURL.m_nProtocol;
	m_pAddress	= pURL.m_pAddress;
	m_nPort		= pURL.m_nPort;
	
	if ( m_nProtocol == PROTOCOL_ED2K )
	{
		m_pServerAddress	= pURL.m_pServerAddress;
		m_nServerPort		= pURL.m_nServerPort;
		if ( m_nServerPort ) m_bPushOnly = TRUE;
	}
	else if ( m_nProtocol == PROTOCOL_BT )
	{
		if ( m_bGUID = pURL.m_bBTC ) CopyMemory( &m_pGUID, &pURL.m_pBTC, 16 );
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadSource serialize

void CDownloadSource::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar << m_sURL;
		ar << m_nProtocol;
		
		ar << m_bGUID;
		if ( m_bGUID ) ar.Write( &m_pGUID, sizeof(GGUID) );
		
		ar << m_nPort;
		if ( m_nPort ) ar.Write( &m_pAddress, sizeof(m_pAddress) );
		ar << m_nServerPort;
		if ( m_nServerPort ) ar.Write( &m_pServerAddress, sizeof(m_pServerAddress) );
		
		ar << m_sName;
		ar << m_nIndex;
		ar << m_bHashAuth;
		ar << m_bSHA1;
		ar << m_bTiger;
		ar << m_bED2K;
		
		ar << m_sServer;
		ar << m_sNick;
		ar << m_nSpeed;
		ar << m_bPushOnly;
		ar << m_bCloseConn;
		ar << m_bReadContent;
		ar.Write( &m_tLastSeen, sizeof(FILETIME) );
		
		ar.WriteCount( m_pPastFragment->GetCount() );
		
		for ( CFileFragment* pFragment = m_pPastFragment ; pFragment ; pFragment = pFragment->m_pNext )
		{
			pFragment->Serialize( ar );
		}
	}
	else if ( nVersion >= 21 )
	{
		ar >> m_sURL;
		ar >> m_nProtocol;
		
		ar >> m_bGUID;
		if ( m_bGUID ) ar.Read( &m_pGUID, sizeof(GGUID) );
		
		ar >> m_nPort;
		if ( m_nPort ) ar.Read( &m_pAddress, sizeof(m_pAddress) );
		ar >> m_nServerPort;
		if ( m_nServerPort ) ar.Read( &m_pServerAddress, sizeof(m_pServerAddress) );
		
		ar >> m_sName;
		ar >> m_nIndex;
		ar >> m_bHashAuth;
		ar >> m_bSHA1;
		ar >> m_bTiger;
		ar >> m_bED2K;
		
		ar >> m_sServer;
		if ( nVersion >= 24 ) ar >> m_sNick;
		ar >> m_nSpeed;
		ar >> m_bPushOnly;
		ar >> m_bCloseConn;
		ar >> m_bReadContent;
		ar.Read( &m_tLastSeen, sizeof(FILETIME) );
		
		for ( int nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			CFileFragment* pNew = CFileFragment::New( NULL, m_pPastFragment );
			pNew->Serialize( ar, nVersion >= 29 );
			if ( m_pPastFragment != NULL ) m_pPastFragment->m_pPrevious = pNew;
			m_pPastFragment = pNew;
		}

		// Should probably save this instead...
		if ( _tcsncmp( m_sServer, _T("Shareaza"), 8 ) == 0 )
			m_bClientExtended = TRUE;
		if ( _tcsncmp( m_sServer, _T("RAZA"), 4 ) == 0 )
			m_bClientExtended = TRUE;
		//
	}
	else
	{
		DWORD nIndex;
		ar.Read( &m_pAddress, sizeof(m_pAddress) );
		ar >> m_nPort;
		ar >> m_nSpeed;
		ar >> nIndex;
		ar >> m_sName;
		if ( nVersion >= 4 ) ar >> m_sURL;
		if ( nVersion >= 21 ) ar >> m_nProtocol;
		ar >> m_bSHA1;
		if ( nVersion >= 13 ) ar >> m_bTiger;
		if ( nVersion >= 13 ) ar >> m_bED2K;
		if ( nVersion >= 10 ) ar >> m_bHashAuth;
		
		if ( nVersion == 8 )
		{
			DWORD nV;
			ar >> nV;
			m_sServer.Format( _T("%c%c%c%c"), nV & 0xFF, ( nV >> 8 ) & 0xFF, ( nV >> 16 ) & 0xFF, nV >> 24 );
		}
		else if ( nVersion >= 9 )
		{
			ar >> m_sServer;
		}
		
		ar >> m_bPushOnly;
		ar >> m_bReadContent;
		if ( nVersion >= 7 ) ar >> m_bCloseConn;
		if ( nVersion >= 12 ) ar.Read( &m_tLastSeen, sizeof(FILETIME) );
		
		ar.Read( &m_pGUID, sizeof(GGUID) );
		ar.Read( &m_pGUID, sizeof(GGUID) );
		m_bGUID = m_pGUID != (GGUID&)GUID_NULL;
		
		if ( nVersion >= 20 )
		{
			for ( int nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
			{
				CFileFragment* pNew = CFileFragment::New( NULL, m_pPastFragment );
				pNew->Serialize( ar, FALSE );
				if ( m_pPastFragment != NULL ) m_pPastFragment->m_pPrevious = pNew;
				m_pPastFragment = pNew;
			}
		}
		else if ( nVersion >= 5 )
		{
			while ( ar.ReadCount() )
			{
				CFileFragment* pNew = CFileFragment::New( NULL, m_pPastFragment );
				pNew->Serialize( ar, FALSE );
				if ( m_pPastFragment != NULL ) m_pPastFragment->m_pPrevious = pNew;
				m_pPastFragment = pNew;
			}
		}
		
		ResolveURL();
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadSource create transfer

CDownloadTransfer* CDownloadSource::CreateTransfer()
{
	ASSERT( m_pTransfer == NULL );
	
	if ( m_nProtocol == PROTOCOL_HTTP )
	{
		return ( m_pTransfer = new CDownloadTransferHTTP( this ) );
	}
	if ( m_nProtocol == PROTOCOL_FTP )
	{
		return NULL; //( m_pTransfer = new CDownloadTransferFTP( this ) );
	}
	else if ( m_nProtocol == PROTOCOL_ED2K )
	{
		return ( m_pTransfer = new CDownloadTransferED2K( this ) );
	}
	else if ( m_nProtocol == PROTOCOL_BT )
	{
		return ( m_pTransfer = new CDownloadTransferBT( this, NULL ) );
	}
	else
	{
		return NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadSource remove

void CDownloadSource::Remove(BOOL bCloseTransfer, BOOL bBan)
{
	if ( m_pTransfer != NULL )
	{
		if ( bCloseTransfer )
		{
			m_pTransfer->Close( TS_TRUE );
			ASSERT( m_pTransfer == NULL );
		}
		else
		{
			m_pTransfer->m_pSource = NULL;
			m_pTransfer = NULL;
		}
	}
	
	m_pDownload->RemoveSource( this, bBan );
}

//////////////////////////////////////////////////////////////////////
// CDownloadSource failure handler

void CDownloadSource::OnFailure(BOOL bNondestructive)
{
	if ( m_pTransfer != NULL )
	{
		m_pTransfer->SetState(dtsNull);
		m_pTransfer->m_pSource = NULL;
		m_pTransfer = NULL;
	}
	
	DWORD nDelay = Settings.Downloads.RetryDelay * (DWORD)pow( 2, m_nFailures );
	
	if ( m_nFailures < 20 )
	{
		if ( nDelay > 3600000 ) nDelay = 3600000;
	}
	else
	{
		if ( nDelay > 86400000 ) nDelay = 86400000;
	}
	
	nDelay += GetTickCount();
	
	int nMaxFailures = ( m_bReadContent ? 40 : 3 );
	if ( nMaxFailures < 20 && m_pDownload->GetSourceCount() > 20 ) nMaxFailures = 0;
	
	m_pDownload->SetModified();
	
	if ( bNondestructive || ( ++m_nFailures < nMaxFailures ) )
	{
		m_tAttempt = max( m_tAttempt, nDelay );
	}
	else
	{
		if ( Settings.Downloads.NeverDrop )
		{
			m_tAttempt = nDelay;
		}
		else
		{
			m_pDownload->RemoveSource( this, TRUE );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadSource resume handler

void CDownloadSource::OnResume()
{
	m_tAttempt = 0;
}

//////////////////////////////////////////////////////////////////////
// CDownloadSource status

void CDownloadSource::SetValid()
{
	m_bReadContent = TRUE;
	m_nFailures = 0;
	m_pDownload->SetModified();
}

void CDownloadSource::SetLastSeen()
{
	SYSTEMTIME pTime;
	GetSystemTime( &pTime );
	SystemTimeToFileTime( &pTime, &m_tLastSeen );
	m_pDownload->SetModified();
}

void CDownloadSource::SetGnutella(int nGnutella)
{
	m_nGnutella |= nGnutella;
	m_pDownload->SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownloadSource hash check and learn

BOOL CDownloadSource::CheckHash(const SHA1* pSHA1)
{
	if ( m_pDownload->m_bSHA1 && ! m_bHashAuth )
	{
		if ( m_pDownload->m_pSHA1 != *pSHA1 ) return FALSE;
	}
	else
	{
		if ( m_pDownload->m_pTorrent.IsAvailable() ) return TRUE;
		
		m_pDownload->m_bSHA1 = TRUE;
		m_pDownload->m_pSHA1 = *pSHA1;
	}
	
	m_bSHA1 = TRUE;
	m_pDownload->SetModified();
	
	return TRUE;
}

BOOL CDownloadSource::CheckHash(const TIGEROOT* pTiger)
{
	if ( m_pDownload->m_bTiger && ! m_bHashAuth )
	{
		if ( m_pDownload->m_pTiger != *pTiger ) return FALSE;
	}
	else
	{
		if ( m_pDownload->m_pTorrent.IsAvailable() ) return TRUE;
		
		m_pDownload->m_bTiger = TRUE;
		m_pDownload->m_pTiger = *pTiger;
	}
	
	m_bTiger = TRUE;
	m_pDownload->SetModified();
	
	return TRUE;
}

BOOL CDownloadSource::CheckHash(const MD4* pED2K)
{
	if ( m_pDownload->m_bED2K && ! m_bHashAuth )
	{
		if ( memcmp( &m_pDownload->m_pED2K, pED2K, sizeof(MD4) ) ) return FALSE;
	}
	else
	{
		if ( m_pDownload->m_pTorrent.IsAvailable() ) return TRUE;
		
		m_pDownload->m_bED2K = TRUE;
		m_pDownload->m_pED2K = *pED2K;
	}
	
	m_bED2K = TRUE;
	m_pDownload->SetModified();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadSource push request

BOOL CDownloadSource::PushRequest()
{
	if ( m_nProtocol == PROTOCOL_BT )
	{
		return FALSE;
	}
	else if ( m_nProtocol == PROTOCOL_ED2K )
	{
		if ( m_nServerPort == 0 ) return FALSE;
		if ( EDClients.IsFull() ) return TRUE;
		
		CEDClient* pClient = EDClients.Connect( m_pAddress.S_un.S_addr, m_nPort,
			&m_pServerAddress, m_nServerPort, m_bGUID ? &m_pGUID : NULL );
		
		if ( pClient != NULL && pClient->m_bConnected )
		{
			pClient->SeekNewDownload();
			return TRUE;
		}
		
		if ( Neighbours.PushDonkey( m_pAddress.S_un.S_addr, &m_pServerAddress, m_nServerPort ) )
		{
			theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_PUSH_SENT, (LPCTSTR)m_pDownload->m_sRemoteName );
			m_tAttempt = GetTickCount() + Settings.Downloads.PushTimeout;
			return TRUE;
		}
	}
	else
	{
		if ( ! m_bGUID ) return FALSE;
		
		if ( Network.SendPush( &m_pGUID, m_nIndex ) )
		{
			theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_PUSH_SENT, (LPCTSTR)m_pDownload->m_sRemoteName );
			m_tAttempt = GetTickCount() + Settings.Downloads.PushTimeout;
			return TRUE;
		}
	}
	
	return FALSE;
}

BOOL CDownloadSource::CheckPush(GGUID* pClientID)
{
	return m_bGUID && ( m_pGUID == *pClientID );
}

BOOL CDownloadSource::CheckDonkey(CEDClient* pClient)
{
	if ( m_nProtocol != PROTOCOL_ED2K ) return FALSE;
	
	if ( m_bGUID && pClient->m_bGUID ) return m_pGUID == pClient->m_pGUID;
	
	if ( m_bPushOnly )
	{
		return	m_pServerAddress.S_un.S_addr == pClient->m_pServer.sin_addr.S_un.S_addr &&
				m_pAddress.S_un.S_addr == pClient->m_nClientID;
	}
	else
	{
		return m_pAddress.S_un.S_addr == pClient->m_pHost.sin_addr.S_un.S_addr;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadSource past fragments

void CDownloadSource::AddFragment(QWORD nOffset, QWORD nLength, BOOL bMerge)
{
	m_bReadContent = TRUE;
	CFileFragment::AddMerge( &m_pPastFragment, nOffset, nLength );
	m_pDownload->SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownloadSource available ranges

void CDownloadSource::SetAvailableRanges(LPCTSTR pszRanges)
{
	if ( m_pAvailable != NULL )
	{
		m_pAvailable->DeleteChain();
		m_pAvailable = NULL;
	}
	
	if ( ! pszRanges || ! *pszRanges ) return;
	if ( _tcsnicmp( pszRanges, _T("bytes"), 5 ) ) return;
	
	CFileFragment* pPrevious = NULL;
	CString strRanges( pszRanges + 6 );
	
	for ( strRanges += ',' ; strRanges.GetLength() ; )
	{
		CString strRange = strRanges.SpanExcluding( _T(", \t") );
		strRanges = strRanges.Mid( strRange.GetLength() + 1 );
		
		strRange.TrimLeft();
		strRange.TrimRight();
		if ( strRange.Find( '-' ) < 0 ) continue;
		
		QWORD nFirst = 0, nLast = 0;
		
		if ( _stscanf( strRange, _T("%I64i-%I64i"), &nFirst, &nLast ) == 2 && nLast > nFirst )
		{
			CFileFragment* pFragment = CFileFragment::New( pPrevious, NULL, nFirst, nLast + 1 - nFirst );
			if ( ! m_pAvailable ) m_pAvailable = pFragment;
			if ( pPrevious ) pPrevious->m_pNext = pFragment;
			pPrevious = pFragment;
		}
	}
	
	m_pDownload->SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownloadSource range intersection test

BOOL CDownloadSource::HasUsefulRanges() const
{
	if ( m_pAvailable == NULL ) return m_pDownload->IsRangeUseful( 0, m_pDownload->m_nSize );
	
	for ( CFileFragment* pFragment = m_pAvailable ; pFragment ; pFragment = pFragment->m_pNext )
	{
		if ( m_pDownload->IsRangeUseful( pFragment->m_nOffset, pFragment->m_nLength ) ) return TRUE;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadSource range intersection

BOOL CDownloadSource::TouchedRange(QWORD nOffset, QWORD nLength) const
{
	if ( m_pTransfer != NULL && m_pTransfer->m_nState == dtsDownloading )
	{
		if ( m_pTransfer->m_nOffset + m_pTransfer->m_nLength > nOffset &&
			 m_pTransfer->m_nOffset < nOffset + nLength )
		{
			return TRUE;
		}
	}
	
	for (	CFileFragment* pFragment = m_pPastFragment ; pFragment ;
			pFragment = pFragment->m_pNext )
	{
		if ( pFragment->m_nOffset >= nOffset + nLength ) continue;
		if ( pFragment->m_nOffset + pFragment->m_nLength <= nOffset ) continue;
		return TRUE;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadSource colour

int CDownloadSource::GetColour()
{
	if ( m_nColour >= 0 ) return m_nColour;
	m_nColour = m_pDownload->GetSourceColour();
	return m_nColour;
}

