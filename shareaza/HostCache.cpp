//
// HostCache.cpp
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
#include "HostCache.h"
#include "Neighbours.h"
#include "Security.h"
#include "VendorCache.h"
#include "G1Packet.h"
#include "EDPacket.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CHostCache HostCache;


//////////////////////////////////////////////////////////////////////
// CHostCache construction

CHostCache::CHostCache() :
		Gnutella1( PROTOCOL_G1 ), Gnutella2( PROTOCOL_G2 ),
		eDonkey( PROTOCOL_ED2K )
{
	m_pList.AddTail( &Gnutella1 );
	m_pList.AddTail( &Gnutella2 );
	m_pList.AddTail( &eDonkey );
}

//////////////////////////////////////////////////////////////////////
// CHostCache core operations

void CHostCache::Clear()
{
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CHostCacheList* pCache = (CHostCacheList*)m_pList.GetNext( pos );
		pCache->Clear();
	}
}

BOOL CHostCache::Load()
{
	CSingleLock pLock( &Network.m_pSection, TRUE );
	CString strFile;
	CFile pFile;
	
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CHostCacheList* pCache = (CHostCacheList*)m_pList.GetNext( pos );
		pCache->Clear();
	}
	
	strFile.Format( _T("%s\\Data\\HostCache.dat"), (LPCTSTR)Settings.General.Path );
	if ( ! pFile.Open( strFile, CFile::modeRead ) ) return FALSE;
	
	try
	{
		CArchive ar( &pFile, CArchive::load );
		Serialize( ar );
		ar.Close();
	}
	catch ( CException* pException )
	{
		pException->Delete();
	}
	
	if ( eDonkey.GetNewest() == NULL ) eDonkey.Import( _T("C:\\Program Files\\eMule\\server.met") );
	
	return TRUE;
}

BOOL CHostCache::Save()
{
	CSingleLock pLock( &Network.m_pSection, TRUE );
	CString strFile;
	CFile pFile;
	
	strFile.Format( _T("%s\\Data\\HostCache.dat"), (LPCTSTR)Settings.General.Path );
	
	if ( ! pFile.Open( strFile, CFile::modeWrite|CFile::modeCreate ) ) return FALSE;
	
	CArchive ar( &pFile, CArchive::store );
	Serialize( ar );
	ar.Close();
	
	return TRUE;
}

void CHostCache::Serialize(CArchive& ar)
{
	int nVersion = 10;
	
	if ( ar.IsStoring() )
	{
		ar << nVersion;
		ar.WriteCount( m_pList.GetCount() );
		
		for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
		{
			CHostCacheList* pCache = (CHostCacheList*)m_pList.GetNext( pos );
			ar << pCache->m_nProtocol;
			pCache->Serialize( ar, nVersion );
		}
	}
	else
	{
		ar >> nVersion;
		if ( nVersion < 6 ) return;
		
		for ( int nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			PROTOCOLID nProtocol;
			ar >> nProtocol;
			
			for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
			{
				CHostCacheList* pCache = (CHostCacheList*)m_pList.GetNext( pos );
				if ( pCache->m_nProtocol == nProtocol )
				{
					pCache->Serialize( ar, nVersion );
					break;
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCache forwarding operations

CHostCacheHost* CHostCache::Find(IN_ADDR* pAddress) const
{
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CHostCacheList* pCache = (CHostCacheList*)m_pList.GetNext( pos );
		if ( CHostCacheHost* pHost = pCache->Find( pAddress ) ) return pHost;
	}
	return NULL;
}

BOOL CHostCache::Check(CHostCacheHost* pHost) const
{
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CHostCacheList* pCache = (CHostCacheList*)m_pList.GetNext( pos );
		if ( pCache->Check( pHost ) ) return TRUE;
	}
	return FALSE;
}

void CHostCache::Remove(CHostCacheHost* pHost)
{
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CHostCacheList* pCache = (CHostCacheList*)m_pList.GetNext( pos );
		pCache->Remove( pHost );
	}
}

void CHostCache::OnFailure(IN_ADDR* pAddress, WORD nPort)
{
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CHostCacheList* pCache = (CHostCacheList*)m_pList.GetNext( pos );
		pCache->OnFailure( pAddress, nPort );
	}
}


//////////////////////////////////////////////////////////////////////
// CHostCacheList construction

CHostCacheList::CHostCacheList(PROTOCOLID nProtocol)
{
	m_nProtocol	= nProtocol;
	m_nBuffer	= 0;
	m_pBuffer	= NULL;
	m_nCookie	= 0;
}

CHostCacheList::~CHostCacheList()
{
	Clear();
	if ( m_pBuffer ) delete [] m_pBuffer;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList clear

void CHostCacheList::Clear()
{
	if ( m_pBuffer == NULL )
	{
		m_nBuffer	= Settings.Gnutella.HostCacheSize;
		m_pBuffer	= new CHostCacheHost[ m_nBuffer ];
	}
	
	CHostCacheHost* pHost = m_pBuffer;
	
	for ( DWORD nPos = m_nBuffer ; nPos ; nPos--, pHost++ )
	{
		pHost->m_pNextHash = ( nPos == 1 ) ? NULL : pHost + 1;
		pHost->m_nProtocol = m_nProtocol;
	}
	
	ZeroMemory( m_pHash, sizeof(CHostCacheHost*) * 256 );
	
	m_pFree		= m_pBuffer;
	m_pNewest	= NULL;
	m_pOldest	= NULL;
	m_nHosts	= 0;
	
	m_nCookie++;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList host add

CHostCacheHost* CHostCacheList::Add(IN_ADDR* pAddress, WORD nPort, DWORD tSeen, LPCTSTR pszVendor)
{
	if ( ! nPort ) return NULL;
	if ( ! pAddress->S_un.S_un_b.s_b1 ) return NULL;
	
	BYTE nHash	= pAddress->S_un.S_un_b.s_b1
				+ pAddress->S_un.S_un_b.s_b2
				+ pAddress->S_un.S_un_b.s_b3
				+ pAddress->S_un.S_un_b.s_b4;
	
	CHostCacheHost** pHash = m_pHash + nHash;
	CHostCacheHost* pHost = *pHash;
	
	for ( ; pHost ; pHost = pHost->m_pNextHash )
	{
		if ( pHost->m_pAddress.S_un.S_addr == pAddress->S_un.S_addr ) break;
	}
	
	BOOL bToNewest = TRUE;
	
	if ( pHost == NULL )
	{
		if ( m_nHosts == m_nBuffer ) RemoveOldest();
		if ( m_nHosts == m_nBuffer || ! m_pFree ) return NULL;
		
		pHost = m_pFree;
		m_pFree = m_pFree->m_pNextHash;
		m_nHosts++;
		
		pHost->m_pNextHash = *pHash;
		*pHash = pHost;
		
		pHost->Reset( pAddress );
	}
	else if ( time( NULL ) - pHost->m_tFailure >= 180 )
	{
		if ( pHost->m_pPrevTime )
			pHost->m_pPrevTime->m_pNextTime = pHost->m_pNextTime;
		else
			m_pOldest = pHost->m_pNextTime;
		
		if ( pHost->m_pNextTime )
			pHost->m_pNextTime->m_pPrevTime = pHost->m_pPrevTime;
		else
			m_pNewest = pHost->m_pPrevTime;
	}
	else
	{
		bToNewest = FALSE;
	}
	
	if ( bToNewest )
	{
		pHost->m_pNextTime = NULL;
		pHost->m_pPrevTime = m_pNewest;
		
		if ( m_pNewest )
			m_pNewest->m_pNextTime = pHost;
		else
			m_pOldest = pHost;
		
		m_pNewest = pHost;
	}
	
	pHost->Update( nPort, tSeen, pszVendor );
	m_nCookie++;
	
	return pHost;
}

BOOL CHostCacheList::Add(LPCTSTR pszHost, DWORD tSeen, LPCTSTR pszVendor)
{
	CString strHost( pszHost );
	
	strHost.TrimLeft();
	strHost.TrimRight();
	
	int nPos = strHost.ReverseFind( ' ' );
	
	if ( nPos > 0 ) 
	{
		CString strTime = strHost.Mid( nPos + 1 );
		strHost = strHost.Left( nPos );
		strHost.TrimRight();
		
		tSeen = TimeFromString( strTime );
	}
	
	nPos = strHost.Find( ':' );
	if ( nPos < 0 ) return FALSE;
	
	int nPort = GNUTELLA_DEFAULT_PORT;
	if ( _stscanf( strHost.Mid( nPos + 1 ), _T("%i"), &nPort ) != 1 ) return FALSE;
	strHost = strHost.Left( nPos );
	
	USES_CONVERSION;
	DWORD nAddress = inet_addr( T2CA( (LPCTSTR)strHost ) );
	
	if ( Security.IsDenied( (IN_ADDR*)&nAddress ) ) return TRUE;
	if ( Network.IsFirewalledAddress( &nAddress, TRUE ) ) return TRUE;
	
	Add( (IN_ADDR*)&nAddress, (WORD)nPort, tSeen, pszVendor );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList find

CHostCacheHost*	CHostCacheList::Find(IN_ADDR* pAddress) const
{
	if ( ! pAddress->S_un.S_un_b.s_b1 ) return NULL;
	
	BYTE nHash	= pAddress->S_un.S_un_b.s_b1
				+ pAddress->S_un.S_un_b.s_b2
				+ pAddress->S_un.S_un_b.s_b3
				+ pAddress->S_un.S_un_b.s_b4;
	
	CHostCacheHost** pHash = (class CHostCacheHost **)m_pHash + nHash;
	CHostCacheHost* pHost = *pHash;
	
	for ( ; pHost ; pHost = pHost->m_pNextHash )
	{
		if ( pHost->m_pAddress.S_un.S_addr == pAddress->S_un.S_addr ) return pHost;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList check host

BOOL CHostCacheList::Check(CHostCacheHost* pHost) const
{
	for ( CHostCacheHost* pTest = m_pOldest ; pTest ; pTest = pTest->m_pNextTime )
	{
		if ( pTest == pHost ) return TRUE;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList host remove

void CHostCacheList::Remove(CHostCacheHost* pHost)
{
	// NOTE: OPTIMIZE: Why the hell am I searching the list if I already have
	// the object pointer?
	
	for ( CHostCacheHost* pTest = m_pOldest ; pTest ; pTest = pTest->m_pNextTime )
	{
		if ( pTest == pHost )
		{
			if ( pHost->m_pPrevTime )
				pHost->m_pPrevTime->m_pNextTime = pHost->m_pNextTime;
			else
				m_pOldest = pHost->m_pNextTime;
			
			if ( pHost->m_pNextTime )
				pHost->m_pNextTime->m_pPrevTime = pHost->m_pPrevTime;
			else
				m_pNewest = pHost->m_pPrevTime;
			
			BYTE nHash	= pHost->m_pAddress.S_un.S_un_b.s_b1
						+ pHost->m_pAddress.S_un.S_un_b.s_b2
						+ pHost->m_pAddress.S_un.S_un_b.s_b3
						+ pHost->m_pAddress.S_un.S_un_b.s_b4;
			
			CHostCacheHost** pPrevious = m_pHash + nHash;
			
			for ( pTest = *pPrevious ; pTest ; pTest = pTest->m_pNextHash )
			{
				if ( pTest == pHost )
				{
					*pPrevious = pHost->m_pNextHash;
					break;
				}
				
				pPrevious = &pTest->m_pNextHash;
			}
			
			pHost->m_pNextHash = m_pFree;
			m_pFree = pHost;
			m_nHosts--;
			m_nCookie++;
			
			return;
		}
	}
}

void CHostCacheList::RemoveOldest()
{
	if ( m_pOldest ) Remove( m_pOldest );
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList failure processor

void CHostCacheList::OnFailure(IN_ADDR* pAddress, WORD nPort)
{
	BYTE nHash	= pAddress->S_un.S_un_b.s_b1
				+ pAddress->S_un.S_un_b.s_b2
				+ pAddress->S_un.S_un_b.s_b3
				+ pAddress->S_un.S_un_b.s_b4;
	
	CHostCacheHost** pHash = m_pHash + nHash;
	
	for ( CHostCacheHost* pHost = *pHash ; pHost ; pHost = pHost->m_pNextHash )
	{
		if ( pHost->m_pAddress.S_un.S_addr == pAddress->S_un.S_addr &&
			 ( ! nPort || pHost->m_nPort == nPort ) )
		{
			if ( pHost->m_bPriority ) return;
			
			/*
			if ( pHost->m_pPrevTime )
				pHost->m_pPrevTime->m_pNextTime = pHost->m_pNextTime;
			else
				m_pOldest = pHost->m_pNextTime;
			
			if ( pHost->m_pNextTime )
				pHost->m_pNextTime->m_pPrevTime = pHost->m_pPrevTime;
			else
				m_pNewest = pHost->m_pPrevTime;
			
			pHost->m_pPrevTime = NULL;
			pHost->m_pNextTime = m_pOldest;
			
			if ( m_pOldest )
				m_pOldest->m_pPrevTime = pHost;
			else
				m_pNewest = pHost;
			
			m_pOldest = pHost;
			
			pHost->m_tFailure = time( NULL );
			*/
			
			Remove( pHost );
			
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList count

DWORD CHostCacheList::CountHosts() const
{
	DWORD nCount = 0;

	for ( CHostCacheHost* pHost = GetNewest() ; pHost ; pHost = pHost->m_pPrevTime )
		nCount++;

	return ( nCount );
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList query acknowledgement prune

void CHostCacheList::PruneByQueryAck()
{
	DWORD tNow = time( NULL );
	
	for ( CHostCacheHost* pHost = m_pNewest ; pHost ; )
	{
		CHostCacheHost* pNext = pHost->m_pPrevTime;
		
		if ( pHost->m_tAck )
		{
			if ( tNow - pHost->m_tAck > Settings.Gnutella2.QueryHostDeadline )
			{
				pHost->m_tAck = 0;
				if ( pHost->m_nFailures++ > 3 ) Remove( pHost );
			}
		}
		
		pHost = pNext;
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList serialize

void CHostCacheList::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar.WriteCount( m_nHosts );
		
		for ( CHostCacheHost* pHost = m_pOldest ; pHost ; pHost = pHost->m_pNextTime )
		{
			pHost->Serialize( ar, nVersion );
		}
	}
	else
	{
		int nItem, nCount = ar.ReadCount();
		
		for ( nItem = 0 ; nItem < nCount && m_pFree ; nItem++ )
		{
			CHostCacheHost* pHost = m_pFree;
			m_pFree = m_pFree->m_pNextHash;
			m_nHosts++;

			pHost->Reset( NULL );
			pHost->Serialize( ar, nVersion );
			
			if ( m_pOldest == NULL ) m_pOldest = pHost;
			
			if ( m_pNewest )
			{
				m_pNewest->m_pNextTime = pHost;
				pHost->m_pPrevTime = m_pNewest;
				m_pNewest = pHost;
			}
			else m_pNewest = pHost;
			
			BYTE nHash	= pHost->m_pAddress.S_un.S_un_b.s_b1
						+ pHost->m_pAddress.S_un.S_un_b.s_b2
						+ pHost->m_pAddress.S_un.S_un_b.s_b3
						+ pHost->m_pAddress.S_un.S_un_b.s_b4;
			
			CHostCacheHost** pHash = m_pHash + nHash;
			pHost->m_pNextHash = *pHash;
			*pHash = pHost;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList root import

int CHostCacheList::Import(LPCTSTR pszFile)
{
	CFile pFile;
	
	if ( ! pFile.Open( pszFile, CFile::modeRead ) ) return 0;
	
	if ( _tcsistr( pszFile, _T(".met") ) != NULL )
	{
		return ImportMET( &pFile );
	}
	else
	{
		return 0;
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList MET import

int CHostCacheList::ImportMET(CFile* pFile)
{
	CSingleLock pLock( &Network.m_pSection, TRUE );
	
	BYTE nVersion = 0;
	pFile->Read( &nVersion, sizeof(nVersion) );
	if ( nVersion != 0xE0 && nVersion != 0x0E ) return FALSE;
	
	int nServers = 0;
	UINT nCount = 0;
	
	pFile->Read( &nCount, sizeof(nCount) );
	
	while ( nCount-- > 0 )
	{
		IN_ADDR pAddress;
		WORD nPort;
		UINT nTags;
		
		if ( pFile->Read( &pAddress, sizeof(pAddress) ) != sizeof(pAddress) ) break;
		if ( pFile->Read( &nPort, sizeof(nPort) ) != sizeof(nPort) ) break;
		if ( pFile->Read( &nTags, sizeof(nTags) ) != sizeof(nTags) ) break;
		
		CHostCacheHost* pServer = Add( &pAddress, nPort );
		
		while ( nTags-- > 0 )
		{
			CEDTag pTag;
			if ( ! pTag.Read( pFile ) ) break;
			if ( pServer == NULL ) continue;
			
			if ( pTag.Check( ED2K_ST_SERVERNAME, ED2K_TAG_STRING ) )
			{
				pServer->m_sName = pTag.m_sValue;
			}
			else if ( pTag.Check( ED2K_ST_DESCRIPTION, ED2K_TAG_STRING ) )
			{
				pServer->m_sDescription = pTag.m_sValue;
			}
			else if ( pTag.Check( ED2K_ST_MAXUSERS, ED2K_TAG_INT ) )
			{
				pServer->m_nUserLimit = pTag.m_nValue;
			}
			else if ( pTag.Check( ED2K_ST_MAXFILES, ED2K_TAG_INT ) )
			{
				pServer->m_nFileLimit = pTag.m_nValue;
			}
			else if ( pTag.Check( ED2K_ST_UDPFLAGS, ED2K_TAG_INT ) )
			{
				pServer->m_nUDPFlags = pTag.m_nValue;
			}
		}
		
		nServers++;
	}
	
	return nServers;
}


//////////////////////////////////////////////////////////////////////
// CHostCacheHost construction

CHostCacheHost::CHostCacheHost()
{
	m_pNextHash = m_pPrevTime = m_pNextTime = NULL;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost serialize

void CHostCacheHost::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar.Write( &m_pAddress, sizeof(m_pAddress) );
		ar << m_nPort;
		
		ar << m_tAdded;
		ar << m_tSeen;
		ar << m_tRetryAfter;
		
		if ( m_pVendor != NULL && m_pVendor->m_sCode.GetLength() == 4 )
		{
			ar << (CHAR)m_pVendor->m_sCode.GetAt( 0 );
			ar << (CHAR)m_pVendor->m_sCode.GetAt( 1 );
			ar << (CHAR)m_pVendor->m_sCode.GetAt( 2 );
			ar << (CHAR)m_pVendor->m_sCode.GetAt( 3 );
		}
		else
		{
			CHAR cZero = 0;
			ar << cZero;
		}
		
		ar << m_sName;
		if ( m_sName.GetLength() ) ar << m_sDescription;

		ar << m_nUserCount;
		ar << m_nUserLimit;
		ar << m_bPriority;

		ar << m_nFileLimit;
		ar << m_nTCPFlags;
		ar << m_nUDPFlags;
		ar << m_tStats;
		
		ar << m_nKeyValue;
		if ( m_nKeyValue != 0 )
		{
			ar << m_tKeyTime;
			ar << m_nKeyHost;
		}
	}
	else
	{
		ar.Read( &m_pAddress, sizeof(m_pAddress) );
		ar >> m_nPort;
		
		ar >> m_tAdded;
		ar >> m_tSeen;
		ar >> m_tRetryAfter;
		
		CHAR szaVendor[4] = { 0, 0, 0, 0 };
		ar >> szaVendor[0];
		
		if ( szaVendor[0] )
		{
			ar.Read( szaVendor + 1, 3 );
			TCHAR szVendor[5] = { szaVendor[0], szaVendor[1], szaVendor[2], szaVendor[3], 0 };
			m_pVendor = VendorCache.Lookup( szVendor );
		}
		
		if ( nVersion >= 10 )
		{
			ar >> m_sName;
			if ( m_sName.GetLength() ) ar >> m_sDescription;
			
			ar >> m_nUserCount;
			ar >> m_nUserLimit;
			ar >> m_bPriority;

			ar >> m_nFileLimit;
			ar >> m_nTCPFlags;
			ar >> m_nUDPFlags;
			ar >> m_tStats;
		}
		else if ( nVersion >= 7 )
		{
			ar >> m_sName;
			if ( m_sName.GetLength() )
			{
				ar >> m_sDescription;
				ar >> m_nUserCount;
				if ( nVersion >= 8 ) ar >> m_nUserLimit;
				if ( nVersion >= 9 ) ar >> m_bPriority;
				if ( nVersion >= 10 )
				{
					ar >> m_nFileLimit;
					ar >> m_nTCPFlags;
					ar >> m_nUDPFlags;
				}
			}
		}
		
		ar >> m_nKeyValue;
		if ( m_nKeyValue != 0 )
		{
			ar >> m_tKeyTime;
			ar >> m_nKeyHost;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost reset

void CHostCacheHost::Reset(IN_ADDR* pAddress)
{
	if ( pAddress ) m_pAddress = *pAddress;
	
	m_pVendor		= NULL;
	m_bPriority		= FALSE;
	m_nUserCount	= 0;
	m_nUserLimit	= 0;
	m_nFileLimit	= 0;
	m_sName.Empty();
	m_sDescription.Empty();

	m_nUDPFlags		= 0;
	m_nTCPFlags		= 0;
	
	m_tAdded		= GetTickCount();
	m_tSeen			= 0;
	m_tRetryAfter	= 0;
	m_tConnect		= 0;
	m_tQuery		= 0;
	m_tAck			= 0;
	m_tStats		= 0;
	m_tFailure		= 0;
	m_nFailures		= 0;
	
	m_tKeyTime		= 0;
	m_nKeyValue		= 0;
	m_nKeyHost		= 0;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost update

void CHostCacheHost::Update(WORD nPort, DWORD tSeen, LPCTSTR pszVendor)
{
	m_nPort		= nPort;
	m_tSeen		= tSeen > 1 ? tSeen : time( NULL );
	
	if ( pszVendor != NULL )
	{
		if ( m_pVendor == NULL || m_pVendor->m_sCode != pszVendor )
		{
			m_pVendor = VendorCache.Lookup( pszVendor );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost connection setup

CNeighbour* CHostCacheHost::ConnectTo(BOOL bAutomatic)
{
	m_tConnect = time( NULL );
	if ( bAutomatic && Network.IsFirewalledAddress( &m_pAddress, TRUE ) ) return NULL;
	return Neighbours.ConnectTo( &m_pAddress, m_nPort, m_nProtocol, bAutomatic );
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost packet conversion

CG1Packet* CHostCacheHost::ToG1Ping(int nTTL, GGUID* pGUID)
{
	CG1Packet* pPong = CG1Packet::New( G1_PACKET_PONG, nTTL, pGUID );
	
	pPong->WriteShortLE( m_nPort );
	pPong->WriteLongLE( *(DWORD*)&m_pAddress );
	pPong->WriteLongLE( 0 );
	pPong->WriteLongLE( 0 );
	
	return pPong;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost string

CString CHostCacheHost::ToString() const
{
	struct tm* pTime = gmtime( (time_t*)&m_tSeen );
	CString str; // 2002-04-30T08:30Z
	
	str.Format( _T("%s:%i %.4i-%.2i-%.2iT%.2i:%.2iZ"),
		(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort,
		pTime->tm_year + 1900, pTime->tm_mon + 1, pTime->tm_mday,
		pTime->tm_hour, pTime->tm_min );
	
	return str;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost connection test

BOOL CHostCacheHost::CanConnect(DWORD tNow) const
{
	if ( ! m_tConnect ) return TRUE;
	if ( m_pAddress.S_un.S_addr == Network.m_pHost.sin_addr.S_un.S_addr ) return FALSE;
	if ( ! tNow ) tNow = time( NULL );
	return tNow - m_tConnect >= Settings.Gnutella.ConnectThrottle;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost quote test

BOOL CHostCacheHost::CanQuote(DWORD tNow) const
{
	if ( ! tNow ) tNow = time( NULL );
	return tNow - m_tSeen < Settings.Gnutella.HostCacheExpire;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost query test

BOOL CHostCacheHost::CanQuery(DWORD tNow) const
{
	// eDonkey2000 server
	if ( m_nProtocol == PROTOCOL_ED2K )
	{
		// Must support ED2K
		if ( ! Settings.eDonkey.EnableToday ) return FALSE;
		if ( ! Settings.eDonkey.ServerWalk ) return FALSE;
		
		// Get the time if not supplied
		if ( 0 == tNow ) tNow = time( NULL );
		
		// Retry After
		if ( 0 != m_tRetryAfter && tNow < m_tRetryAfter ) return FALSE;
		
		// If haven't queried yet, its ok
		if ( 0 == m_tQuery ) return TRUE;
		
		// Don't query too fast
		return ( tNow - m_tQuery ) >= Settings.eDonkey.QueryServerThrottle;
	}
	else if ( m_nProtocol == PROTOCOL_G2 )
	{
		// Must support G2
		if ( ! Settings.Gnutella2.EnableToday ) return FALSE;
		
		// Must not be waiting for an ack
		if ( 0 != m_tAck ) return FALSE;
		
		// Get the time if not supplied
		if ( 0 == tNow ) tNow = time( NULL );
		
		// Must not have expired from host cache
		if ( ( tNow - m_tSeen ) > Settings.Gnutella.HostCacheExpire ) return FALSE;
		
		// Retry After
		if ( 0 != m_tRetryAfter && tNow < m_tRetryAfter ) return FALSE;
		
		// If haven't queried yet, its ok
		if ( 0 == m_tQuery ) return TRUE;
		
		// Don't query too fast
		return ( tNow - m_tQuery ) >= max( Settings.Gnutella2.QueryHostThrottle, DWORD(90) );
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost query key submission

void CHostCacheHost::SetKey(DWORD nKey, IN_ADDR* pHost)
{
	m_tAck		= 0;
	m_nFailures	= 0;
	m_tKeyTime	= nKey ? time( NULL ) : 0;
	m_nKeyValue	= nKey;
	m_nKeyHost	= pHost && nKey ? pHost->S_un.S_addr : Network.m_pHost.sin_addr.S_un.S_addr;
}
