//
// HostCache.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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
#include "Network.h"
#include "HostCache.h"
#include "Neighbours.h"
#include "Security.h"
#include "VendorCache.h"
#include "EDPacket.h"
#include "Buffer.h"
#include "Kademlia.h"
#include "DiscoveryServices.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CHostCache HostCache;


//////////////////////////////////////////////////////////////////////
// CHostCache construction

CHostCache::CHostCache() :
	Gnutella1( PROTOCOL_G1 ),
	Gnutella2( PROTOCOL_G2 ),
	eDonkey( PROTOCOL_ED2K ),
	G1DNA( PROTOCOL_G1 ),
	BitTorrent( PROTOCOL_BT ),
	Kademlia( PROTOCOL_KAD ),
	m_tLastPruneTime( 0 )
{
	m_pList.AddTail( &Gnutella1 );
	m_pList.AddTail( &Gnutella2 );
	m_pList.AddTail( &eDonkey );
	m_pList.AddTail( &G1DNA );
	m_pList.AddTail( &BitTorrent );
	m_pList.AddTail( &Kademlia );
}

//////////////////////////////////////////////////////////////////////
// CHostCache core operations

void CHostCache::Clear()
{
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CHostCacheList* pCache = m_pList.GetNext( pos );
		pCache->Clear();
	}
}

BOOL CHostCache::Load()
{
	CQuickLock oLock( m_pSection );

	CString strFile;
	CFile pFile;
	
	Clear();
	
	strFile.Format( _T("%s\\Data\\HostCache.dat"), (LPCTSTR)Settings.General.UserPath );
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

	if ( eDonkey.GetNewest() == NULL ) CheckMinimumED2KServers();

	return TRUE;
}

BOOL CHostCache::Save()
{
	CQuickLock oLock( m_pSection );

	CString strFile;
	CFile pFile;
	
	strFile.Format( _T("%s\\Data\\HostCache.dat"), (LPCTSTR)Settings.General.UserPath );
	
	if ( ! pFile.Open( strFile, CFile::modeWrite|CFile::modeCreate ) ) return FALSE;
	
	CArchive ar( &pFile, CArchive::store );
	Serialize( ar );
	ar.Close();
	
	return TRUE;
}

void CHostCache::Serialize(CArchive& ar)
{
	// History:
	// 14 - Added m_sCountry
	// 15 - Added m_bDHT and m_oBtGUID (Ryo-oh-ki)
	// 16 - Added m_nUDPPort, m_oGUID and m_nKADVersion (Ryo-oh-ki)
	// 17 - Added m_tConnect (Ryo-oh-ki)
	int nVersion = 17;

	if ( ar.IsStoring() )
	{
		ar << nVersion;
		ar.WriteCount( m_pList.GetCount() );
		
		for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
		{
			CHostCacheList* pCache = m_pList.GetNext( pos );
			ar << pCache->m_nProtocol;
			pCache->Serialize( ar, nVersion );
		}
	}
	else
	{
		ar >> nVersion;
		if ( nVersion < 6 ) return;
		
		for ( DWORD_PTR nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			PROTOCOLID nProtocol;
			ar >> nProtocol;
			
			for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
			{
				CHostCacheList* pCache = m_pList.GetNext( pos );
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
// CHostCache root import

int CHostCache::Import(LPCTSTR pszFile, BOOL bFreshOnly)
{
	// Ignore too old file
	if ( bFreshOnly && ! IsFileNewerThan( pszFile, 90ull * 24 * 60 * 60 * 1000 ) ) // 90 days
		return 0;

	CFile pFile;
	if ( ! pFile.Open( pszFile, CFile::modeRead ) )
		return 0;

	// server.met
	if ( _tcsicmp( PathFindExtension( pszFile ), _T(".met") ) == 0 )
	{
		theApp.Message( MSG_NOTICE, _T("Importing MET file: %s ..."), pszFile );
		return ImportMET( &pFile );
	}
	// nodes.dat
	else if ( _tcsicmp( PathFindExtension( pszFile ), _T(".dat") ) == 0 )
	{
		theApp.Message( MSG_NOTICE, _T("Importing Nodes file: %s ..."), pszFile );
		return ImportNodes( &pFile );
	}
	else
		return 0;
}

//////////////////////////////////////////////////////////////////////
// CHostCache prune old hosts

void CHostCache::PruneOldHosts()
{
	DWORD tNow = static_cast< DWORD >( time( NULL ) );
	if ( tNow - m_tLastPruneTime > 5 * 60 )
	{
		for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
		{
			m_pList.GetNext( pos )->PruneOldHosts();
		}
		m_tLastPruneTime = tNow;
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCache forwarding operations

CHostCacheHostPtr CHostCache::Find(const IN_ADDR* pAddress) const
{
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CHostCacheList* pCache = m_pList.GetNext( pos );
		if ( CHostCacheHostPtr pHost = pCache->Find( pAddress ) ) return pHost;
	}
	return NULL;
}

BOOL CHostCache::Check(const CHostCacheHostPtr pHost) const
{
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CHostCacheList* pCache = m_pList.GetNext( pos );
		if ( pCache->Check( pHost ) ) return TRUE;
	}
	return FALSE;
}

void CHostCache::Remove(CHostCacheHostPtr pHost)
{
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CHostCacheList* pCache = m_pList.GetNext( pos );
		pCache->Remove( pHost );
	}
}

void CHostCache::SanityCheck()
{
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CHostCacheList* pCache = m_pList.GetNext( pos );
		pCache->SanityCheck();
	}
}

void CHostCache::OnFailure(const IN_ADDR* pAddress, WORD nPort, PROTOCOLID nProtocol, bool bRemove)
{
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CHostCacheList* pCache = m_pList.GetNext( pos );
		if ( nProtocol == PROTOCOL_NULL || nProtocol == pCache->m_nProtocol )
			pCache->OnFailure( pAddress, nPort, bRemove );
	}
}

void CHostCache::OnSuccess(const IN_ADDR* pAddress, WORD nPort, PROTOCOLID nProtocol, bool bUpdate)
{
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CHostCacheList* pCache = m_pList.GetNext( pos );
		if ( nProtocol == PROTOCOL_NULL || nProtocol == pCache->m_nProtocol )
			pCache->OnSuccess( pAddress, nPort, bUpdate );
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList construction

CHostCacheList::CHostCacheList(PROTOCOLID nProtocol) :
	m_nProtocol( nProtocol ),
	m_nCookie( 0 )
{
}

CHostCacheList::~CHostCacheList()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList clear

void CHostCacheList::Clear()
{
	CQuickLock oLock( m_pSection );

	for( CHostCacheMap::const_iterator i = m_Hosts.begin(); i != m_Hosts.end(); ++i )
	{
		delete (*i).second;
	}
	m_Hosts.clear();
	m_HostsTime.clear();

	m_nCookie++;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList host add

CHostCacheHostPtr CHostCacheList::Add(IN_ADDR* pAddress, WORD nPort, DWORD tSeen, LPCTSTR pszVendor, DWORD nUptime, DWORD nCurrentLeaves, DWORD nLeafLimit)
{
	// Don't add invalid addresses
	if ( ! nPort ) 
		return NULL;
	if ( ! pAddress->S_un.S_un_b.s_b1 ) 
		return NULL;

	// Don't add own firewalled IPs
	if ( Network.IsFirewalledAddress( pAddress, TRUE ) ) 
		return NULL;

	// check against IANA Reserved address.
	if ( Network.IsReserved( pAddress ) )
		return NULL;

	// Check security settings, don't add blocked IPs
	if ( Security.IsDenied( pAddress ) )
		return NULL;

	// Try adding it to the cache. (duplicates will be rejected)
	return AddInternal( pAddress, nPort, tSeen, pszVendor, nUptime, nCurrentLeaves, nLeafLimit );
}

BOOL CHostCacheList::Add(LPCTSTR pszHost, DWORD tSeen, LPCTSTR pszVendor, DWORD nUptime, DWORD nCurrentLeaves, DWORD nLeafLimit)
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

		DWORD tNow = static_cast< DWORD >( time( NULL ) );
		if ( tNow < tSeen ) 
			tSeen = tNow;
	}

	nPos = strHost.Find( ':' );
	if ( nPos < 0 ) return FALSE;
	
	int nPort = GNUTELLA_DEFAULT_PORT;
	if ( _stscanf( strHost.Mid( nPos + 1 ), _T("%i"), &nPort ) != 1 ||
		nPort <= 0 || nPort >= 65536 ) return FALSE;
	strHost = strHost.Left( nPos );
	
	DWORD nAddress = inet_addr( CT2CA( (LPCTSTR)strHost ) );
	if ( nAddress == INADDR_NONE ) return FALSE;

	return ( Add( (IN_ADDR*)&nAddress, (WORD)nPort, tSeen, pszVendor, nUptime, nCurrentLeaves, nLeafLimit ) != NULL );
}

// This function actually add the remote client to the host cache. Private, but used by the public 
// functions. No security checking, etc.
CHostCacheHostPtr CHostCacheList::AddInternal(const IN_ADDR* pAddress, WORD nPort, 
											DWORD tSeen, LPCTSTR pszVendor, DWORD nUptime, DWORD nCurrentLeaves, DWORD nLeafLimit)
{
	CQuickLock oLock( m_pSection );

	ASSERT( pAddress );
	ASSERT( pAddress->s_addr != INADDR_NONE && pAddress->s_addr != INADDR_ANY );
	ASSERT( nPort );
	ASSERT( m_Hosts.size() == m_HostsTime.size() );

	// Check if we already have the host
	CHostCacheHostPtr pHost = Find( pAddress );
	if ( ! pHost )
	{
		// Create new host
		pHost = new CHostCacheHost( m_nProtocol );
		if ( pHost )
		{
			PruneHosts();

			// Add host to map and index
			pHost->m_pAddress = *pAddress;
			pHost->Update( nPort, tSeen, pszVendor, nUptime, nCurrentLeaves, nLeafLimit );
			m_Hosts.insert( CHostCacheMapPair( *pAddress, pHost ) );
			m_HostsTime.insert( pHost );

			m_nCookie++;
		}
	}
	else
	{
		Update( pHost, nPort, tSeen, pszVendor, nUptime, nCurrentLeaves, nLeafLimit );
	}
	return pHost;
}

void CHostCacheList::Update(CHostCacheHostPtr pHost, WORD nPort, DWORD tSeen, LPCTSTR pszVendor, DWORD nUptime, DWORD nCurrentLeaves, DWORD nLeafLimit)
{
	CQuickLock oLock( m_pSection );

	ASSERT( pHost );
	ASSERT( pHost->IsValid() );
	ASSERT( m_Hosts.size() == m_HostsTime.size() );

	// Update host
	if ( pHost->Update( nPort, tSeen, pszVendor, nUptime, nCurrentLeaves, nLeafLimit ) )
	{
		// Remove host from old and now invalid position
		m_HostsTime.erase(
			std::find( m_HostsTime.begin(), m_HostsTime.end(), pHost ) );

		// Add host to new sorted position
		m_HostsTime.insert( pHost );

		ASSERT( m_Hosts.size() == m_HostsTime.size() );
	}

	m_nCookie++;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList host remove

bool CHostCacheList::Remove(CHostCacheHostPtr pHost)
{
	CQuickLock oLock( m_pSection );

	for( CHostCacheMap::iterator i = m_Hosts.begin(); i != m_Hosts.end(); ++i )
	{
		if ( (*i).second == pHost )
		{
			m_HostsTime.erase(
				std::find( m_HostsTime.begin(), m_HostsTime.end(), pHost ) );
			m_Hosts.erase( i );
			delete pHost;
			m_nCookie++;
			return true;
		}
	}
	return false;
}

bool CHostCacheList::Remove(const IN_ADDR* pAddress)
{
	CQuickLock oLock( m_pSection );

	CHostCacheMap::iterator i = m_Hosts.find( *pAddress );
	if ( i != m_Hosts.end() )
	{
		CHostCacheHostPtr pHost = (*i).second;
		m_HostsTime.erase(
			std::find( m_HostsTime.begin(), m_HostsTime.end(), pHost ) );
		m_Hosts.erase( i );
		delete pHost;
		m_nCookie++;
		return true;
	}
	return false;
}

void CHostCacheList::SanityCheck()
{
	CQuickLock oLock( m_pSection );

	for( CHostCacheMap::iterator i = m_Hosts.begin(); i != m_Hosts.end(); )
	{
		CHostCacheHostPtr pHost = (*i).second;
		if ( Security.IsDenied( &pHost->m_pAddress ) )
		{
			m_HostsTime.erase(
				std::find( m_HostsTime.begin(), m_HostsTime.end(), pHost ) );
			i = m_Hosts.erase( i );
			delete pHost;
			m_nCookie++;
		}
		else
			++i;
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList failure processor

void CHostCacheList::OnFailure(const IN_ADDR* pAddress, WORD nPort, bool bRemove)
{
	CQuickLock oLock( m_pSection );

	CHostCacheHostPtr pHost = Find( pAddress );
	if ( pHost && ( ! nPort || pHost->m_nPort == nPort ) )
	{
		m_nCookie++;
		pHost->m_nFailures++;
		if ( pHost->m_bPriority )
			return;
		if ( bRemove || pHost->m_nFailures >= Settings.Connection.FailureLimit )
			Remove( pHost );
		else
		{
			pHost->m_tFailure = static_cast< DWORD >( time( NULL ) );
			pHost->m_bCheckedLocally = TRUE;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList failure processor

void CHostCacheList::OnSuccess(const IN_ADDR* pAddress, WORD nPort, bool bUpdate)
{
	CQuickLock oLock( m_pSection );

	CHostCacheHostPtr pHost = Add( const_cast< IN_ADDR* >( pAddress ), nPort );
	if ( pHost && ( ! nPort || pHost->m_nPort == nPort ) )
	{
		m_nCookie++;
		pHost->m_tFailure = 0;
		pHost->m_nFailures = 0;
		pHost->m_bCheckedLocally = TRUE;
		if ( bUpdate )
			Update( pHost, nPort );
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList query acknowledgment prune (G2)

void CHostCacheList::PruneByQueryAck()
{
	CQuickLock oLock( m_pSection );

	DWORD tNow = static_cast< DWORD >( time( NULL ) );

	for( CHostCacheMap::iterator i = m_Hosts.begin(); i != m_Hosts.end(); )
	{
		bool bRemoved = false;
		CHostCacheHostPtr pHost = (*i).second;
		if ( pHost->m_tAck )
		{
			if ( tNow - pHost->m_tAck > Settings.Gnutella2.QueryHostDeadline )
			{
				pHost->m_tAck = 0;
				if ( pHost->m_nFailures++ > Settings.Connection.FailureLimit )
				{
					m_HostsTime.erase(
						std::find( m_HostsTime.begin(), m_HostsTime.end(), pHost ) );
					i = m_Hosts.erase( i );
					delete pHost;
					bRemoved = true;
					m_nCookie++;
				}
			}
		}
		// Don't increment if host was removed
		if ( ! bRemoved )
			++i;
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList prune old hosts

void CHostCacheList::PruneOldHosts()
{
	CQuickLock oLock( m_pSection );

	DWORD tNow = static_cast< DWORD >( time( NULL ) );
	
	for( CHostCacheMap::iterator i = m_Hosts.begin(); i != m_Hosts.end(); )
	{
		CHostCacheHostPtr pHost = (*i).second;

		float nProbability = .0;

		if ( pHost->m_nProtocol == PROTOCOL_G1 )
		{
			// Calculate some kind of probability if we need to prune it
			float nProbability = (float)pHost->m_nDailyUptime / ( 24 * 60 * 60 );
			nProbability /= pHost->m_nFailures + 1;
		}

		// Since we discard hosts after 3 failures, it means that we will remove
		// hosts with the DU less than 8 hours without no failures when they expire;
		// hosts with the DU less than 16 hours with 1 failure;
		// hosts with the DU less than 24 hours with 2 failures;
		if ( ! pHost->m_bPriority && pHost->IsExpired( tNow ) && nProbability < .333 )
		{
			m_HostsTime.erase(
				std::find( m_HostsTime.begin(), m_HostsTime.end(), pHost ) );
			i = m_Hosts.erase( i );
			delete pHost;
			m_nCookie++;
		}
		else
			++i;
	}
}


//////////////////////////////////////////////////////////////////////
// Remove several oldest hosts

void CHostCacheList::PruneHosts()
{
	CQuickLock oLock( m_pSection );

	for( CHostCacheIndex::iterator i = m_HostsTime.end();
		m_Hosts.size() > Settings.Gnutella.HostCacheSize && i != m_HostsTime.begin(); )
	{
		--i;
		CHostCacheHost* pHost = (*i);
		if ( ! pHost->m_bPriority )
		{
			i = m_HostsTime.erase( i );
			m_Hosts.erase( pHost->m_pAddress );
			delete pHost;
			m_nCookie++;
		}
	}

	for( CHostCacheIndex::iterator i = m_HostsTime.end();
		m_Hosts.size() > Settings.Gnutella.HostCacheSize && i != m_HostsTime.begin(); )
	{
		--i;
		CHostCacheHost* pHost = (*i);
		i = m_HostsTime.erase( i );
		m_Hosts.erase( pHost->m_pAddress );
		delete pHost;
		m_nCookie++;
	}

	ASSERT( m_Hosts.size() == m_HostsTime.size() );
}


//////////////////////////////////////////////////////////////////////
// CHostCacheList serialize

void CHostCacheList::Serialize(CArchive& ar, int nVersion)
{
	CQuickLock oLock( m_pSection );

	if ( ar.IsStoring() )
	{
		ar.WriteCount( GetCount() );
		for( CHostCacheMap::const_iterator i = m_Hosts.begin(); i != m_Hosts.end(); ++i )
		{
			CHostCacheHostPtr pHost = (*i).second;
			pHost->Serialize( ar, nVersion );
		}
	}
	else
	{
		DWORD_PTR nCount = ar.ReadCount();
		for ( DWORD_PTR nItem = 0 ; nItem < nCount; nItem++ )
		{
			CHostCacheHostPtr pHost = new CHostCacheHost( m_nProtocol );
			if ( pHost )
			{
				pHost->Serialize( ar, nVersion );
				if ( pHost->IsValid() &&
					! Security.IsDenied( &pHost->m_pAddress ) &&
					! Find( &pHost->m_pAddress ) )
				{
					m_Hosts.insert( CHostCacheMapPair( pHost->m_pAddress, pHost ) );
					m_HostsTime.insert( pHost );
				}
				else
					// Remove bad or duplicated host
					delete pHost;
			}
		}

		PruneHosts();

		m_nCookie++;
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCache MET import

int CHostCache::ImportMET(CFile* pFile)
{
	BYTE nVersion = 0;
	pFile->Read( &nVersion, sizeof(nVersion) );
	if ( nVersion != 0xE0 &&
		 nVersion != ED2K_MET &&
		 nVersion != ED2K_MET_I64TAGS ) return FALSE;
	
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
		
		CHostCacheHostPtr pServer = eDonkey.Add( &pAddress, nPort );
		
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
				pServer->m_nUserLimit = (DWORD)pTag.m_nValue;
			}
			else if ( pTag.Check( ED2K_ST_MAXFILES, ED2K_TAG_INT ) )
			{
				pServer->m_nFileLimit = (DWORD)pTag.m_nValue;
			}
			else if ( pTag.Check( ED2K_ST_UDPFLAGS, ED2K_TAG_INT ) )
			{
				pServer->m_nUDPFlags = (DWORD)pTag.m_nValue;
			}
		}
		
		nServers++;
	}
	
	return nServers;
}

//////////////////////////////////////////////////////////////////////
// CHostCache Nodes import

int CHostCache::ImportNodes(CFile* pFile)
{
	int nServers = 0;
	UINT nVersion = 0;

	UINT nCount;
	if ( pFile->Read( &nCount, sizeof( nCount ) ) != sizeof( nCount ) )
		return 0;
	if ( nCount == 0 )
	{
		// New format
		if ( pFile->Read( &nVersion, sizeof( nVersion ) ) != sizeof( nVersion ) )
			return 0;
		if ( nVersion == 1 )
		{
			if ( pFile->Read( &nCount, sizeof( nCount ) ) != sizeof( nCount ) )
				return 0;
		}
		else
			// Unknown format
			return 0;
	}
	while ( nCount-- > 0 )
	{
		Hashes::Guid oGUID;
		if ( pFile->Read( &oGUID[0], oGUID.byteCount ) != oGUID.byteCount )
			break;
		oGUID.validate();
		IN_ADDR pAddress;
		if ( pFile->Read( &pAddress, sizeof( pAddress ) ) != sizeof( pAddress ) )
			break;
		pAddress.s_addr = ntohl( pAddress.s_addr );
		WORD nUDPPort;
		if ( pFile->Read( &nUDPPort, sizeof( nUDPPort ) ) != sizeof( nUDPPort ) )
			break;
		WORD nTCPPort;
		if ( pFile->Read( &nTCPPort, sizeof( nTCPPort ) ) != sizeof( nTCPPort ) )
			break;
		BYTE nKADVersion = 0;
		BYTE nType = 0;
		if ( nVersion == 1 )
		{
			if ( pFile->Read( &nKADVersion, sizeof( nKADVersion ) ) != sizeof( nKADVersion ) )
				break;
		}
		else
		{
			if ( pFile->Read( &nType, sizeof( nType ) ) != sizeof( nType ) )
				break;
		}
		if ( nType < 4 )
		{
			CHostCacheHostPtr pCache = Kademlia.Add( &pAddress, nTCPPort );
			if ( pCache )
			{
				pCache->m_oGUID = oGUID;
				pCache->m_sDescription = oGUID.toString();
				pCache->m_nUDPPort = nUDPPort;
				pCache->m_nKADVersion = nKADVersion;
				nServers++;
			}
		}
	}

	return nServers;
}

//////////////////////////////////////////////////////////////////////
// CHostCache Check Minimum ED2K Servers

bool CHostCache::CheckMinimumED2KServers()
{
#ifndef LAN_MODE
	if ( ! EnoughED2KServers() )
	{
		// Load default ed2k server list (if necessary)
		LoadDefaultED2KServers();

		// Get the server list from eMule (mods) if possible
		const LPCTSTR sServerMetPathes[ 8 ] = {
			{ _T("\\eMule\\config\\server.met") },
			{ _T("\\eMule\\server.met") },
			{ _T("\\Neo Mule\\config\\server.met") },
			{ _T("\\Neo Mule\\server.met") },
			{ _T("\\hebMule\\config\\server.met") },
			{ _T("\\hebMule\\server.met") },
			{ _T("\\aMule\\config\\server.met") },
			{ _T("\\aMule\\server.met") }
		};

		CString sRootPathes[ 3 ];
		sRootPathes[ 0 ] = theApp.GetProgramFilesFolder();
		sRootPathes[ 1 ] = theApp.GetLocalAppDataFolder();
		sRootPathes[ 2 ] = theApp.GetAppDataFolder();

		for ( int i = 0; i < _countof( sRootPathes ); i++ )
			for ( int j = 0; j < _countof( sServerMetPathes ); j++ )
				Import( sRootPathes[ i ] + sServerMetPathes[ j ], TRUE );

		// Get server list from Web
		if ( ! EnoughED2KServers() )
			DiscoveryServices.Execute( TRUE, PROTOCOL_ED2K, TRUE );

		return false;
	}
#endif // LAN_MODE
	return true;
}

//////////////////////////////////////////////////////////////////////
// CHostCache Default ED2K servers import

int CHostCache::LoadDefaultED2KServers()
{
	CFile pFile;
	int nServers = 0;
	CString strFile = Settings.General.Path + _T("\\Data\\DefaultServers.dat");

	// Ignore too old file
	if ( ! IsFileNewerThan( strFile, 90ull * 24 * 60 * 60 * 1000 ) ) // 90 days
		return 0;

	if ( pFile.Open( strFile, CFile::modeRead ) )			// Load default list from file if possible
	{
		theApp.Message( MSG_NOTICE, _T("Loading default ED2K server list") );

		try
		{
			CString strLine;
			CBuffer pBuffer;
			TCHAR cType;

			pBuffer.EnsureBuffer( (DWORD)pFile.GetLength() );
			pBuffer.m_nLength = (DWORD)pFile.GetLength();
			pFile.Read( pBuffer.m_pBuffer, pBuffer.m_nLength );
			pFile.Close();

			while ( pBuffer.ReadLine( strLine ) )
			{
				if ( strLine.GetLength() < 7 ) continue; // Blank comment line

				cType = strLine.GetAt( 0 );

				if ( cType != '#' )
				{
					CString strServer = strLine.Right( strLine.GetLength() - 2 );

					int nIP[4], nPort;

					if ( _stscanf( strServer, _T("%i.%i.%i.%i:%i"), &nIP[0], &nIP[1], &nIP[2], &nIP[3],	&nPort ) == 5 )
					{
						IN_ADDR pAddress;
						pAddress.S_un.S_un_b.s_b1 = (BYTE)nIP[0];
						pAddress.S_un.S_un_b.s_b2 = (BYTE)nIP[1];
						pAddress.S_un.S_un_b.s_b3 = (BYTE)nIP[2];
						pAddress.S_un.S_un_b.s_b4 = (BYTE)nIP[3];

						CHostCacheHostPtr pServer = eDonkey.Add( &pAddress, (WORD)nPort );

						if ( pServer )
						{
							if ( cType == 'P' )
								pServer->m_bPriority = TRUE;
							else
								pServer->m_bPriority = FALSE;

							nServers++;
						}
					}
				}
			}
		}
		catch ( CException* pException )
		{
			if (pFile.m_hFile != CFile::hFileNull) pFile.Close(); // Check if file is still open, if yes close
			pException->Delete();
		}
	}

	return nServers;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost construction

CHostCacheHost::CHostCacheHost(PROTOCOLID nProtocol) :
	m_nProtocol( nProtocol ),
	m_nPort(0),
	m_nUDPPort(0),
	m_pVendor(NULL),
	m_bPriority(FALSE),
	m_nUserCount(0),
	m_nUserLimit(0),
	m_nFileLimit(0),
	m_nTCPFlags(0),
	m_nUDPFlags(0),
	m_tAdded( GetTickCount() ),
	m_tSeen(0),
	m_tRetryAfter(0),
	m_tConnect(0),
	m_tQuery(0),
	m_tAck(0),
	m_tStats(0),
	m_tFailure(0),
	m_nFailures(0),
	m_nDailyUptime(0),
	m_tKeyTime(0),
	m_nKeyValue(0),
	m_nKeyHost(0),
	m_bCheckedLocally(FALSE),
	// Attributes: DHT
	m_bDHT(FALSE),
	// Attributes: Kademlia
	m_nKADVersion(0)
{
	m_pAddress.s_addr = 0;

	// 30 sec cooldown to avoid neighbor add-remove oscillation
	DWORD tNow = static_cast< DWORD >( time( NULL ) );
	switch ( m_nProtocol )
	{
	case PROTOCOL_G1:
	case PROTOCOL_G2:
		m_tConnect = tNow - Settings.Gnutella.ConnectThrottle + 30;
		break;
	case PROTOCOL_ED2K:
		m_tConnect = tNow - Settings.eDonkey.QueryServerThrottle + 30;
		break;
	default:
		break;
	}
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

		ar << m_tFailure;
		ar << m_nFailures;
		ar << m_bCheckedLocally;
		ar << m_nDailyUptime;
		ar << m_sCountry;

		ar << m_bDHT;
		ar.Write( &m_oBtGUID[0], m_oBtGUID.byteCount );

		ar << m_nUDPPort;
		ar.Write( &m_oGUID[0], m_oGUID.byteCount );
		ar << m_nKADVersion;

		ar << m_tConnect;
	}
	else
	{
		ReadArchive( ar, &m_pAddress, sizeof(m_pAddress) );
		ar >> m_nPort;
		
		ar >> m_tAdded;
		ar >> m_tSeen;
		ar >> m_tRetryAfter;
		
		CHAR szaVendor[4] = { 0, 0, 0, 0 };
		ar >> szaVendor[0];
		
		if ( szaVendor[0] )
		{
			ReadArchive( ar, szaVendor + 1, 3 );
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

		if ( nVersion >= 11 )
		{
			ar >> m_tFailure;
			ar >> m_nFailures;
		}

		if ( nVersion >= 12 )
		{
			ar >> m_bCheckedLocally;
			ar >> m_nDailyUptime;
		}
		if ( nVersion >= 14 )
		{
			ar >> m_sCountry;
		}
		else
			m_sCountry = theApp.GetCountryCode( m_pAddress );

		if ( nVersion >= 15 )
		{
			ar >> m_bDHT;
			ReadArchive( ar, &m_oBtGUID[0], m_oBtGUID.byteCount );
			m_oBtGUID.validate();
		}

		if ( nVersion >= 16 )
		{
			ar >> m_nUDPPort;
			ReadArchive( ar, &m_oGUID[0], m_oGUID.byteCount );
			m_oGUID.validate();
			ar >> m_nKADVersion;
		}

		if ( nVersion >= 17 )
		{
			ar >> m_tConnect;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost update

bool CHostCacheHost::Update(WORD nPort, DWORD tSeen, LPCTSTR pszVendor, DWORD nUptime, DWORD nCurrentLeaves, DWORD nLeafLimit)
{
	bool bChanged = FALSE;

	if ( nPort )
	{
		m_nUDPPort = m_nPort = nPort;
	}

	if ( ! tSeen )
	{
		tSeen = static_cast< DWORD >( time( NULL ) );
	}
	if ( m_tSeen < tSeen )
	{
		m_tSeen = tSeen;
		bChanged = true;
	}

	if ( nUptime )
	{
		m_nDailyUptime = nUptime;
	}

	if ( nCurrentLeaves )
	{
		m_nUserCount = nCurrentLeaves;
	}

	if ( nLeafLimit )
	{
		m_nUserLimit = nLeafLimit;
	}

	if ( pszVendor != NULL )
	{
		CString strVendorCode(pszVendor);
		strVendorCode.Trim();
		if ( ( m_pVendor == NULL || m_pVendor->m_sCode != strVendorCode ) &&
			strVendorCode.GetLength() != 0 )
		{
			m_pVendor = VendorCache.Lookup( (LPCTSTR)strVendorCode );
		}
	}

	if ( m_sCountry.IsEmpty() )
	{
		m_sCountry = theApp.GetCountryCode( m_pAddress );
	}

	return bChanged;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost connection setup

CNeighbour* CHostCacheHost::ConnectTo(BOOL bAutomatic)
{
	m_tConnect = static_cast< DWORD >( time( NULL ) );

	switch( m_nProtocol )
	{
	case PROTOCOL_G1:
	case PROTOCOL_G2:
	case PROTOCOL_ED2K:
		return Neighbours.ConnectTo( &m_pAddress, m_nPort, m_nProtocol, bAutomatic );
	case PROTOCOL_KAD:
		{
			SOCKADDR_IN pHost = { 0 };
			pHost.sin_family = AF_INET;
			pHost.sin_addr.s_addr = m_pAddress.s_addr;
			pHost.sin_port = htons( m_nUDPPort );
			Kademlia.Bootstrap( &pHost );
		}
		break;
	default:
		ASSERT( FALSE );
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost string

CString CHostCacheHost::ToString(bool bLong) const
{
	CString str;
	if ( bLong )
	{
		time_t tSeen = m_tSeen;
		tm* pTime = gmtime( &tSeen );
		str.Format( _T("%s:%i %.4i-%.2i-%.2iT%.2i:%.2iZ"),
			(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort,
			pTime->tm_year + 1900, pTime->tm_mon + 1, pTime->tm_mday,
			pTime->tm_hour, pTime->tm_min ); // 2002-04-30T08:30Z
	}
	else
	{
		str.Format( _T("%s:%i"),
			(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort );
	}
	return str;
}

bool CHostCacheHost::IsExpired(DWORD tNow) const
{
	switch ( m_nProtocol )
	{
	case PROTOCOL_G1:
		return m_tSeen && ( tNow - m_tSeen > Settings.Gnutella1.HostExpire );
	case PROTOCOL_G2:
		return m_tSeen && ( tNow - m_tSeen > Settings.Gnutella2.HostExpire );
	case PROTOCOL_ED2K:
		return false;	// Never
	case PROTOCOL_BT:
		return m_tSeen && ( tNow - m_tSeen > Settings.BitTorrent.DhtPruneTime );
	case PROTOCOL_KAD:
		return m_tSeen && ( tNow - m_tSeen > 24 * 60 * 60 ); // TODO: Add Kademlia setting
	default:
		return false;
	}
}

bool CHostCacheHost::IsThrottled(DWORD tNow) const
{
	switch ( m_nProtocol )
	{
	case PROTOCOL_G1:
	case PROTOCOL_G2:
		return ( tNow - m_tConnect < Settings.Gnutella.ConnectThrottle );
	case PROTOCOL_ED2K:
		return ( tNow - m_tConnect < Settings.eDonkey.QueryServerThrottle );
	default:
		return false;
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost connection test

BOOL CHostCacheHost::CanConnect(DWORD tNow) const
{
	// Don't connect to self
	if ( Settings.Connection.IgnoreOwnIP && Network.IsSelfIP( m_pAddress ) ) return FALSE;

	if ( ! tNow ) tNow = static_cast< DWORD >( time( NULL ) );

	return
		// Let failed host rest some time...
		( ! m_tFailure || ( tNow - m_tFailure >= Settings.Connection.FailurePenalty ) ) &&
		// ...and we lost no hope on this host...
		( m_nFailures <= Settings.Connection.FailureLimit ) &&
		// ...and host isn't expired...
		( ! IsExpired( tNow ) ) &&
		// ...and make sure we reconnect not too fast...
		( ! IsThrottled( tNow ) );
		// ...then we can connect!
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost quote test

BOOL CHostCacheHost::CanQuote(DWORD tNow) const
{
	if ( ! tNow ) tNow = static_cast< DWORD >( time( NULL ) );

	return
		// A host isn't dead...
		( m_nFailures == 0 ) &&
		// ...and host isn't expired...
		( ! IsExpired( tNow ) );
		// ...then we can tell about it to others!
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost query test

// Can we UDP query this host? (G2/ed2k)
BOOL CHostCacheHost::CanQuery(DWORD tNow) const
{
	// eDonkey2000 server
	if ( m_nProtocol == PROTOCOL_ED2K )
	{
		// Must support ED2K
		if ( !Network.IsConnected() || !Settings.eDonkey.EnableToday ) return FALSE;
		if ( !Settings.eDonkey.ServerWalk ) return FALSE;
		
		// Get the time if not supplied
		if ( 0 == tNow ) tNow = static_cast< DWORD >( time( NULL ) );
		
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
		if ( !Network.IsConnected() || !Settings.Gnutella2.EnableToday ) return FALSE;
		
		// Must not be waiting for an ack
		if ( 0 != m_tAck ) return FALSE;
		
		// Get the time if not supplied
		if ( 0 == tNow ) tNow = static_cast< DWORD >( time( NULL ) );
		
		// Must be a recently seen (current) host
		if ( ( tNow - m_tSeen ) > Settings.Gnutella2.HostCurrent ) return FALSE;
		
		// Retry After
		if ( 0 != m_tRetryAfter && tNow < m_tRetryAfter ) return FALSE;
		
		// If haven't queried yet, its ok
		if ( 0 == m_tQuery ) return TRUE;
		
		// Don't query too fast
		return ( tNow - m_tQuery ) >= Settings.Gnutella2.QueryHostThrottle;
	}
	else if ( m_nProtocol == PROTOCOL_BT )
	{
		// Must support BT
		if ( !Network.IsConnected() || !Settings.BitTorrent.EnableToday ) return FALSE;

		// Must not be waiting for an ack
		if ( 0 != m_tAck ) return FALSE;

		// Get the time if not supplied
		if ( 0 == tNow ) tNow = static_cast< DWORD >( time( NULL ) );

		// Retry After
		if ( 0 != m_tRetryAfter && tNow < m_tRetryAfter ) return FALSE;

		// If haven't queried yet, its ok
		if ( 0 == m_tQuery ) return TRUE;

		// Don't query too fast
		return ( tNow - m_tQuery ) >= 90u;
	}	
	else if ( m_nProtocol == PROTOCOL_KAD )
	{
		return TRUE; // TODO: Fix it
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost query key submission

void CHostCacheHost::SetKey(DWORD nKey, const IN_ADDR* pHost)
{
	if ( nKey )
	{
		m_tAck		= 0;
		m_nFailures	= 0;
		m_tFailure	= 0;
		m_tKeyTime	= static_cast< DWORD >( time( NULL ) );
		m_nKeyValue	= nKey;
		m_nKeyHost	= pHost ? pHost->S_un.S_addr : Network.m_pHost.sin_addr.S_un.S_addr;
	}
}
