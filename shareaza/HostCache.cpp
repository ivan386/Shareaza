//
// HostCache.cpp
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
#include "Buffer.h"
#include "DiscoveryServices.h"
#include "EDPacket.h"
#include "HostCache.h"
#include "Neighbours.h"
#include "Network.h"
#include "Security.h"
#include "Settings.h"
#include "VendorCache.h"
#include "XML.h"

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
	DC( PROTOCOL_DC ),
	m_tLastPruneTime( 0 )
{
	m_pList.AddTail( &Gnutella1 );
	m_pList.AddTail( &Gnutella2 );
	m_pList.AddTail( &eDonkey );
	m_pList.AddTail( &G1DNA );
	m_pList.AddTail( &BitTorrent );
	m_pList.AddTail( &Kademlia );
	m_pList.AddTail( &DC );
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
	CString strFile = Settings.General.UserPath + _T("\\Data\\HostCache.dat");

	CFile pFile;
	if ( pFile.Open( strFile, CFile::modeRead | CFile::shareDenyWrite | CFile::osSequentialScan ) )
	{
		try
		{
			CArchive ar( &pFile, CArchive::load, 262144 );	// 256 KB buffer
			try
			{
				CQuickLock oLock( m_pSection );

				Serialize( ar );

				ar.Close();
			}
			catch ( CException* pException )
			{
				ar.Abort();
				pFile.Abort();
				pException->Delete();
				theApp.Message( MSG_ERROR, _T("Failed to load host cache: %s"), (LPCTSTR)strFile );
			}
			pFile.Close();
		}
		catch ( CException* pException )
		{
			pFile.Abort();
			pException->Delete();
			theApp.Message( MSG_ERROR, _T("Failed to load host cache: %s"), (LPCTSTR)strFile );
		}
	}
	else
		theApp.Message( MSG_ERROR, _T("Failed to load host cache: %s"), (LPCTSTR)strFile );

	if ( Gnutella2.IsEmpty() )	CheckMinimumServers( PROTOCOL_G2 );
	if ( Gnutella1.IsEmpty() )	CheckMinimumServers( PROTOCOL_G1 );
	if ( eDonkey.IsEmpty() )	CheckMinimumServers( PROTOCOL_ED2K );
	if ( BitTorrent.IsEmpty() )	CheckMinimumServers( PROTOCOL_BT );
	if ( Kademlia.IsEmpty() )	CheckMinimumServers( PROTOCOL_KAD );
	if ( DC.IsEmpty() )			CheckMinimumServers( PROTOCOL_DC );

	return TRUE;
}

BOOL CHostCache::Save()
{
	CString strTemp = Settings.General.UserPath + _T("\\Data\\HostCache.tmp");
	CString strFile = Settings.General.UserPath + _T("\\Data\\HostCache.dat");

	CFile pFile;
	if ( ! pFile.Open( strTemp, CFile::modeWrite | CFile::modeCreate | CFile::shareExclusive | CFile::osSequentialScan ) )
	{
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, _T("Failed to save host cache: %s"), (LPCTSTR)strTemp );
		return FALSE;
	}

	try
	{
		CArchive ar( &pFile, CArchive::store, 262144 );	// 256 KB buffer
		try
		{
			CQuickLock oLock( m_pSection );

			Serialize( ar );

			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
			DeleteFile( strTemp );
			theApp.Message( MSG_ERROR, _T("Failed to save host cache: %s"), (LPCTSTR)strTemp );
			return FALSE;
		}
		pFile.Close();
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, _T("Failed to save host cache: %s"), (LPCTSTR)strTemp );
		return FALSE;
	}

	if ( ! MoveFileEx( strTemp, strFile, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING ) )
	{
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, _T("Failed to save host cache: %s"), (LPCTSTR)strFile );
		return FALSE;
	}

	return TRUE;
}

void CHostCache::Serialize(CArchive& ar)
{
	int nVersion = HOSTCACHE_SER_VERSION;

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
// CHostCache prune old hosts

void CHostCache::PruneOldHosts()
{
	DWORD tNow = static_cast< DWORD >( time( NULL ) );
	if ( tNow  > m_tLastPruneTime + 60 ) // Every minute
	{
		for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
		{
			m_pList.GetNext( pos )->PruneOldHosts( tNow );
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

CHostCacheHostPtr CHostCache::Find(LPCTSTR szAddress) const
{
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CHostCacheList* pCache = m_pList.GetNext( pos );
		if ( CHostCacheHostPtr pHost = pCache->Find( szAddress ) ) return pHost;
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

void CHostCache::OnResolve(PROTOCOLID nProtocol, LPCTSTR szAddress, const IN_ADDR* pAddress, WORD nPort)
{
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CHostCacheList* pCache = m_pList.GetNext( pos );
		if ( nProtocol == PROTOCOL_NULL || nProtocol == pCache->m_nProtocol )
			pCache->OnResolve( szAddress, pAddress, nPort );
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

	for( CHostCacheMapItr i = m_Hosts.begin(); i != m_Hosts.end(); ++i )
	{
		delete (*i).second;
	}
	m_Hosts.clear();
	m_HostsTime.clear();

	m_nCookie++;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList host add

CHostCacheHostPtr CHostCacheList::Add(LPCTSTR pszHost, WORD nPort, DWORD tSeen, LPCTSTR pszVendor, DWORD nUptime, DWORD nCurrentLeaves, DWORD nLeafLimit)
{
	CString strHost( pszHost );
	strHost.Trim();
	int nPos = strHost.ReverseFind( _T(' ') );
	if ( nPos > 0 )
	{
		CString strTime = strHost.Mid( nPos + 1 );
		strHost = strHost.Left( nPos );
		strHost.TrimRight();
		tSeen = TimeFromString( strTime );
		if ( ! tSeen )
			return NULL;
	}

	return Add( NULL, nPort, tSeen, pszVendor, nUptime, nCurrentLeaves, nLeafLimit, strHost );
}

CHostCacheHostPtr CHostCacheList::Add(const IN_ADDR* pAddress, WORD nPort, DWORD tSeen, LPCTSTR pszVendor, DWORD nUptime, DWORD nCurrentLeaves, DWORD nLeafLimit, LPCTSTR szAddress)
{
	ASSERT( pAddress || szAddress );

	if ( ! nPort )
		// Use default port
		nPort = protocolPorts[ m_nProtocol ];

	SOCKADDR_IN saHost;
	if ( ! pAddress )
	{
		// Try to quick resolve dotted IP address
		if (  ! Network.Resolve( szAddress, nPort, &saHost, FALSE ) )
			// Bad address
			return NULL;

		pAddress = &saHost.sin_addr;
		nPort = ntohs( saHost.sin_port );

		if ( pAddress->s_addr != INADDR_ANY )
		{
			// It's dotted IP address
			szAddress = NULL;

			// Don't add invalid addresses
			if ( ! pAddress->S_un.S_un_b.s_b1 ||
			// Don't add own firewalled IPs
				Network.IsFirewalledAddress( pAddress, TRUE ) ||
			// check against IANA Reserved address.
				Network.IsReserved( pAddress ) ||
			// Check security settings, don't add blocked IPs
				Security.IsDenied( pAddress ) )
				// Bad IP
				return NULL;
		}
	}
	else
	{
		// Don't add invalid addresses
		if ( ! pAddress->S_un.S_un_b.s_b1 ||
		// Don't add own firewalled IPs
			Network.IsFirewalledAddress( pAddress, TRUE ) ||
		// check against IANA Reserved address.
			Network.IsReserved( pAddress ) ||
		// Check security settings, don't add blocked IPs
			Security.IsDenied( pAddress ) )
			// Bad IP
			return NULL;
	}

	CQuickLock oLock( m_pSection );

	// Check if we already have the host
	CHostCacheHostPtr pHost = Find( pAddress );
	if ( ! pHost && szAddress )
		pHost = Find( szAddress );
	if ( ! pHost )
	{
		// Create new host
		pHost = new CHostCacheHost( m_nProtocol );
		if ( pHost )
		{
			PruneHosts();

			pHost->m_pAddress = *pAddress;
			if ( szAddress ) pHost->m_sAddress = szAddress;
			pHost->m_sAddress = pHost->m_sAddress.SpanExcluding( _T(":") );

			pHost->Update( nPort, tSeen, pszVendor, nUptime, nCurrentLeaves, nLeafLimit );

			// Add host to map and index
			m_Hosts.insert( CHostCacheMapPair( pHost->m_pAddress, pHost ) );
			m_HostsTime.insert( pHost );

			m_nCookie++;
		}
	}
	else
	{
		if ( szAddress ) pHost->m_sAddress = szAddress;
		pHost->m_sAddress = pHost->m_sAddress.SpanExcluding( _T(":") );

		Update( pHost, nPort, tSeen, pszVendor, nUptime, nCurrentLeaves, nLeafLimit );
	}

	ASSERT( m_Hosts.size() == m_HostsTime.size() );

	return pHost;
}

void CHostCacheList::Update(CHostCacheHostPtr pHost, WORD nPort, DWORD tSeen, LPCTSTR pszVendor, DWORD nUptime, DWORD nCurrentLeaves, DWORD nLeafLimit)
{
	CQuickLock oLock( m_pSection );

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

CHostCacheMapItr CHostCacheList::Remove(CHostCacheHostPtr pHost)
{
	CQuickLock oLock( m_pSection );

	CHostCacheIterator j = std::find( m_HostsTime.begin(), m_HostsTime.end(), pHost );
	if ( j == m_HostsTime.end() )
		// Wrong cache
		return m_Hosts.end();
	m_HostsTime.erase( j );

	CHostCacheMapItr i = std::find_if( m_Hosts.begin(), m_Hosts.end(), std::bind2nd( is_host(), pHost ) );
	ASSERT( i != m_Hosts.end() );
	i = m_Hosts.erase( i );

	delete pHost;
	m_nCookie++;

	ASSERT( m_Hosts.size() == m_HostsTime.size() );

	return i;
}

CHostCacheMapItr CHostCacheList::Remove(const IN_ADDR* pAddress)
{
	CQuickLock oLock( m_pSection );

	CHostCacheMapItr i = m_Hosts.find( *pAddress );
	if ( i == m_Hosts.end() )
		// Wrong cache/address
		return m_Hosts.end();

	return Remove( (*i).second );
}

void CHostCacheList::SanityCheck()
{
	CQuickLock oLock( m_pSection );

	for( CHostCacheMapItr i = m_Hosts.begin(); i != m_Hosts.end(); )
	{
		CHostCacheHostPtr pHost = (*i).second;
		if ( Security.IsDenied( &pHost->m_pAddress ) ||
			( pHost->m_pVendor && Security.IsVendorBlocked( pHost->m_pVendor->m_sCode ) ) )
		{
			i = Remove( pHost );
		}
		else
			++i;
	}
}

void CHostCacheList::OnResolve(LPCTSTR szAddress, const IN_ADDR* pAddress, WORD nPort)
{
	if ( pAddress )
	{
		CQuickLock oLock( m_pSection );

		CHostCacheHostPtr pHost = Find( szAddress );

		// Don't add own firewalled IPs
		if ( Network.IsFirewalledAddress( pAddress, TRUE ) ||
		// check against IANA Reserved address.
			Network.IsReserved( pAddress ) ||
		// Check security settings, don't add blocked IPs
			Security.IsDenied( pAddress ) )
		{
			if ( pHost )
				Remove( pHost );
			return;
		}

		if ( pHost )
		{
			// Remove from old place
			m_Hosts.erase( std::find_if( m_Hosts.begin(), m_Hosts.end(),
				std::bind2nd( is_host(), pHost ) ) );

			pHost->m_pAddress = *pAddress;
			pHost->m_nPort = nPort;
			pHost->m_sCountry = theApp.GetCountryCode( pHost->m_pAddress );

			// Add to new place
			m_Hosts.insert( CHostCacheMapPair( pHost->m_pAddress, pHost ) );

			m_nCookie++;

			ASSERT( m_Hosts.size() == m_HostsTime.size() );
		}
		else
		{
			// New host
			pHost = Add( pAddress, nPort, 0, 0, 0, 0, 0, szAddress );
		}
	}
	else
	{
		OnFailure( szAddress, false );
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
		pHost->m_tFailure = static_cast< DWORD >( time( NULL ) );
		pHost->m_bCheckedLocally = TRUE;

		if ( ! pHost->m_sAddress.IsEmpty() )
			// Clear current IP address to re-resolve name later
			pHost->m_pAddress.s_addr = INADDR_ANY;

		if ( ! pHost->m_bPriority && ( bRemove || pHost->m_nFailures > Settings.Connection.FailureLimit ) )
			Remove( pHost );
	}
}

void CHostCacheList::OnFailure(LPCTSTR szAddress, bool bRemove)
{
	CQuickLock oLock( m_pSection );

	CHostCacheHostPtr pHost = Find( szAddress );

	if ( ! pHost )
		// New host (for resolver)
		pHost = Add( szAddress );

	if ( pHost )
	{
		m_nCookie++;
		pHost->m_nFailures++;
		pHost->m_tFailure = static_cast< DWORD >( time( NULL ) );

		if ( ! pHost->m_bPriority && ( bRemove || pHost->m_nFailures > Settings.Connection.FailureLimit ) )
			Remove( pHost );
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList failure processor

CHostCacheHostPtr CHostCacheList::OnSuccess(const IN_ADDR* pAddress, WORD nPort, bool bUpdate)
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
	return pHost;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheList prune old hosts

void CHostCacheList::PruneOldHosts(DWORD tNow)
{
	CQuickLock oLock( m_pSection );

	for( CHostCacheMapItr i = m_Hosts.begin(); i != m_Hosts.end(); )
	{
		CHostCacheHostPtr pHost = (*i).second;

		switch ( pHost->m_nProtocol )
		{
		case PROTOCOL_G2:
			if ( pHost->m_tAck && tNow > pHost->m_tAck + Settings.Gnutella2.QueryHostDeadline )
			{
				pHost->m_tAck = 0;

				m_nCookie++;
				pHost->m_nFailures++;
			}
			break;

		case PROTOCOL_BT:
			if ( pHost->m_tAck && tNow > pHost->m_tAck + Settings.BitTorrent.QueryHostDeadline )
			{
				pHost->m_tAck = 0;

				m_nCookie++;
				pHost->m_nFailures++;
			}
			break;

		case PROTOCOL_ED2K:
			if ( pHost->m_tAck && tNow > pHost->m_tAck + Settings.Connection.TimeoutHandshake )
			{
				pHost->m_tAck = 0;
				pHost->m_tFailure = pHost->m_tStats;

				m_nCookie++;
				pHost->m_nFailures++;
			}
			break;

		default:
			;
		}

		if ( ! pHost->m_bPriority &&
			 ( pHost->m_nFailures > Settings.Connection.FailureLimit ||
			   pHost->IsExpired( tNow ) ) )
		{
			i = Remove( pHost );
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
		CHostCacheHostPtr pHost = (*i);

		if ( ! pHost->m_bPriority )
		{
			i = m_HostsTime.erase( i );
			m_Hosts.erase( std::find_if( m_Hosts.begin(), m_Hosts.end(),
				std::bind2nd( is_host(), pHost ) ) );
			delete pHost;
			m_nCookie++;
		}
	}

	for( CHostCacheIndex::iterator i = m_HostsTime.end();
		m_Hosts.size() > Settings.Gnutella.HostCacheSize && i != m_HostsTime.begin(); )
	{
		--i;
		CHostCacheHostPtr pHost = (*i);

		i = m_HostsTime.erase( i );
		m_Hosts.erase( std::find_if( m_Hosts.begin(), m_Hosts.end(),
			std::bind2nd( is_host(), pHost ) ) );
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
		for( CHostCacheMapItr i = m_Hosts.begin(); i != m_Hosts.end(); ++i )
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
				if ( ! Security.IsDenied( &pHost->m_pAddress ) &&
					 ! Find( &pHost->m_pAddress ) &&
					 ! Find( pHost->m_sAddress ) )
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

int CHostCache::Import(LPCTSTR pszFile, BOOL bFreshOnly)
{
	int nImported = 0;

	theApp.Message( MSG_DEBUG, _T( "Looking for: %s" ), pszFile );

	CFileFind ff;
	BOOL res = ff.FindFile( pszFile );
	while ( res )
	{
		res = ff.FindNextFile();

		if ( ff.IsDirectory() )
			continue;

		const CString strPath = ff.GetFilePath();
		LPCTSTR szExt = PathFindExtension( strPath );

		// Ignore too old file
		if ( bFreshOnly && ! IsFileNewerThan( strPath, 90ull * 24 * 60 * 60 * 1000 ) ) // 90 days
		{
			theApp.Message( MSG_DEBUG, _T( "Ignoring too old file: %s" ), (LPCTSTR)strPath );
			continue;
		}

		CFile pFile;
		if ( ! pFile.Open( strPath, CFile::modeRead | CFile::shareDenyWrite | CFile::osSequentialScan ) )
		{
			theApp.Message( MSG_DEBUG, _T( "Failed to open file: %s" ), (LPCTSTR)strPath );
			continue;
		}

		if ( _tcsicmp( szExt, _T( ".bz2" ) ) == 0 )
		{
			theApp.Message( MSG_NOTICE, _T( "Importing DC++ Hub List file: %s" ), (LPCTSTR)strPath );

			nImported = ImportHubList( &pFile );
		}
		else if ( _tcsicmp( szExt, _T( ".met" ) ) == 0 )
		{
			theApp.Message( MSG_NOTICE, _T( "Importing Server.met file: %s" ), (LPCTSTR)strPath );

			nImported = ImportMET( &pFile );
		}
		else if ( _tcsicmp( szExt, _T( ".dat" ) ) == 0 )
		{
			theApp.Message( MSG_NOTICE, _T( "Importing Kademlia Nodes file: %s" ), (LPCTSTR)strPath );

			nImported = ImportNodes( &pFile );
		}
	}

	if ( nImported )
		Save();

	return nImported;
}

int CHostCache::ImportHubList(CFile* pFile)
{
	DWORD nSize = (DWORD)pFile->GetLength();

	CBuffer pBuffer;
	if ( ! pBuffer.EnsureBuffer( nSize ) )
		// Out of memory
		return 0;

	if ( pFile->Read( pBuffer.GetData(), nSize ) != nSize )
		// File error
		return 0;
	pBuffer.m_nLength = nSize;

	if ( ! pBuffer.UnBZip() )
		// Decompression error
		return 0;

	CString strEncoding;
	auto_ptr< CXMLElement > pHublist ( CXMLElement::FromString(
		pBuffer.ReadString( pBuffer.m_nLength ), TRUE, &strEncoding ) );
	if ( strEncoding.CompareNoCase( _T("utf-8") ) == 0 )
		// Reload as UTF-8
		pHublist.reset( CXMLElement::FromString(
			pBuffer.ReadString( pBuffer.m_nLength, CP_UTF8 ), TRUE ) );
	if ( ! pHublist.get() )
		// XML decoding error
		return FALSE;

	if ( ! pHublist->IsNamed( _T("Hublist") ) )
		// Invalid XML file format
		return FALSE;

	CXMLElement* pHubs = pHublist->GetFirstElement();
	if ( !  pHubs || ! pHubs->IsNamed( _T("Hubs") ) )
		// Invalid XML file format
		return FALSE;

	int nHubs = 0;
	for ( POSITION pos = pHubs->GetElementIterator() ; pos ; )
	{
		CXMLElement* pHub = pHubs->GetNextElement( pos );
		if ( pHub->IsNamed( _T("Hub") ) )
		{
			CString sAddress = pHub->GetAttributeValue( _T("Address") );
			if ( _tcsnicmp( sAddress, _T("dchub://"), 8 ) == 0 )
				sAddress = sAddress.Mid( 8 );
			else if ( _tcsnicmp( sAddress, _T("adc://"), 6 ) == 0 )
				// Skip ADC-hubs
				continue;
			else if ( _tcsnicmp( sAddress, _T("adcs://"), 7 ) == 0 )
				// Skip ADCS-hubs
				continue;

			int nUsers = _tstoi( pHub->GetAttributeValue( _T("Users") ) );
			int nMaxusers = _tstoi( pHub->GetAttributeValue( _T("Maxusers") ) );

			CQuickLock oLock( DC.m_pSection );
			CHostCacheHostPtr pServer = DC.Add( NULL, protocolPorts[ PROTOCOL_DC ], 0,
				protocolNames[ PROTOCOL_DC ], 0, nUsers, nMaxusers, sAddress );
			if ( pServer )
			{
				pServer->m_sName = pHub->GetAttributeValue( _T("Name") );
				pServer->m_sDescription = pHub->GetAttributeValue( _T("Description") );
				nHubs ++;
			}
		}
	}

	return nHubs;
}

int CHostCache::ImportMET(CFile* pFile)
{
	BYTE nVersion = 0;
	pFile->Read( &nVersion, sizeof(nVersion) );
	if ( nVersion != 0xE0 &&
		 nVersion != ED2K_MET &&
		 nVersion != ED2K_MET_I64TAGS ) return 0;

	int nServers = 0;
	DWORD nCount = 0;

	pFile->Read( &nCount, sizeof(nCount) );

	while ( nCount-- > 0 )
	{
		IN_ADDR pAddress;
		WORD nPort;
		DWORD nTags;

		if ( pFile->Read( &pAddress, sizeof(pAddress) ) != sizeof(pAddress) ) break;
		if ( pFile->Read( &nPort, sizeof(nPort) ) != sizeof(nPort) ) break;
		if ( pFile->Read( &nTags, sizeof(nTags) ) != sizeof(nTags) ) break;

		CQuickLock oLock( eDonkey.m_pSection );
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

int CHostCache::ImportNodes(CFile* pFile)
{
	int nServers = 0;
	DWORD nVersion = 0;

	DWORD nCount;
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
			CQuickLock oLock( Kademlia.m_pSection );
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

bool CHostCache::EnoughServers(PROTOCOLID nProtocol) const
{
	switch ( nProtocol )
	{
	case PROTOCOL_G1:
		return ! Settings.Gnutella1.EnableToday || Gnutella1.CountHosts( TRUE ) > 20;
	case PROTOCOL_G2:
		return ! Settings.Gnutella2.EnableToday || Gnutella2.CountHosts( TRUE ) > 25;
	case PROTOCOL_ED2K:
		return ! Settings.eDonkey.EnableToday || eDonkey.CountHosts( TRUE ) > 0;
	case PROTOCOL_DC:
		return ! Settings.DC.EnableToday || DC.CountHosts( TRUE ) > 0;
	case PROTOCOL_BT:
		return ! Settings.BitTorrent.EnableToday || BitTorrent.CountHosts( TRUE ) > 0;
	default:
		return true;
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCache Check Minimum Servers

bool CHostCache::CheckMinimumServers(PROTOCOLID nProtocol)
{
#ifdef LAN_MODE
	if ( nProtocol != PROTOCOL_G2 )
		// Skip everything except G2 in LAN_MODE
		return true;
#endif // LAN_MODE

	if ( EnoughServers( nProtocol ) )
		return true;

	// Load default server list (if necessary)
	int nImported = LoadDefaultServers( nProtocol );

	const static CString sRootPathes[] =
	{
		theApp.GetProgramFilesFolder().Left( 2 ),	// System disk
		theApp.GetProgramFilesFolder(),
		theApp.GetProgramFilesFolder64(),
		theApp.GetLocalAppDataFolder(),
		theApp.GetAppDataFolder()
	};

	// Get the server list from eMule (mods) if possible
	if ( nProtocol == PROTOCOL_ED2K )
	{
		const static LPCTSTR sServerMetPathes[] =
		{
			{ _T( "\\Neo Mule\\config\\server.met" ) },
			{ _T( "\\Neo Mule\\server.met" ) },
			{ _T( "\\aMule\\config\\server.met" ) },
			{ _T( "\\aMule\\server.met" ) },
			{ _T( "\\eMule\\config\\server.met" ) },
			{ _T( "\\eMule\\server.met" ) },
			{ _T( "\\hebMule\\config\\server.met" ) },
			{ _T( "\\hebMule\\server.met" ) }
		};

		for ( int i = 0; i < _countof( sRootPathes ); ++i )
			for ( int j = 0; j < _countof( sServerMetPathes ); ++j )
				nImported += Import( sRootPathes[ i ] + sServerMetPathes[ j ], TRUE );
	}
	// Get the hub list from DC++ (mods) if possible
	else if ( nProtocol == PROTOCOL_DC )
	{
		const static LPCTSTR sHubListPathes[] =
		{
			{ _T( "\\AirDC++\\HubLists\\*.xml.bz2" ) },
			{ _T( "\\ApexDC++\\HubLists\\*.xml.bz2" ) },
			{ _T( "\\DC++\\HubLists\\*.xml.bz2" ) },
			{ _T( "\\EiskaltDC++\\HubLists\\*.xml.bz2" ) },
			{ _T( "\\FlylinkDC++\\HubLists\\*.xml.bz2" ) },
			{ _T( "\\PeLinkDC\\Settings\\HubLists\\*.xml.bz2" ) }
		};

		for ( int i = 0; i < _countof( sRootPathes ); ++i )
			for ( int j = 0; j < _countof( sHubListPathes ); ++j )
				nImported += Import( sRootPathes[ i ] + sHubListPathes[ j ], TRUE );
	}

	theApp.Message( MSG_NOTICE, _T( "Imported %d new servers." ), nImported );

	return EnoughServers( nProtocol );
}

//////////////////////////////////////////////////////////////////////
// CHostCache Default servers import

int CHostCache::LoadDefaultServers(PROTOCOLID nProtocol)
{
	int nServers = 0;
	CString strFile = Settings.General.Path + _T("\\Data\\DefaultServers.dat");

	CStdioFile pFile;
	if ( pFile.Open( strFile, CFile::modeRead | CFile::shareDenyWrite | CFile::osSequentialScan ) )
	{
		theApp.Message( MSG_NOTICE, _T("Loading default server list for %s: %s"), protocolNames[ nProtocol ], (LPCTSTR)strFile );

		for (;;)
		{
			CString strLine;
			if ( ! pFile.ReadString( strLine ) )
				// End of file
				break;

			strLine.Trim( _T(" \t\r\n") );
			if ( strLine.GetLength() < 9 )
				 // Blank comment line
				continue;

			LPCTSTR szServer = strLine;
			if ( *szServer == _T('#') )
				// Comment line
				continue;

			BOOL bPriority = FALSE;
			if ( *szServer == _T('P') )
			{
				bPriority = TRUE;
				++szServer;
			}

			CHostCacheList* pCache = NULL;
			switch ( *szServer )
			{
			case _T('L'):
				pCache = &Gnutella1;
				++szServer;
				break;
			case _T('G'):
				pCache = &Gnutella2;
				++szServer;
				break;
			case _T(' '):	// compatibility
			case _T('E'):	// new way
				pCache = &eDonkey;
				++szServer;
				break;
			case _T('B'):
				pCache = &BitTorrent;
				++szServer;
				break;
			case _T('K'):
				pCache = &Kademlia;
				++szServer;
				break;
			case _T('D'):
				pCache = &DC;
				++szServer;
				break;
			}
			if ( ! pCache )
				// Unknown protocol
				continue;

			if ( ! ( nProtocol == PROTOCOL_ANY || nProtocol == pCache->m_nProtocol ) )
				// Unneeded protocol
				continue;

			for ( ; *szServer == _T(' '); ++szServer );

			CQuickLock oLock( pCache->m_pSection );
			if ( CHostCacheHostPtr pServer = pCache->Add( szServer ) )
			{
				pServer->m_bPriority = bPriority;
				nServers++;
			}
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
	m_pAddress.s_addr = INADDR_ANY;

	// 10 sec cooldown to avoid neighbor add-remove oscillation
	DWORD tNow = static_cast< DWORD >( time( NULL ) );
	switch ( m_nProtocol )
	{
	case PROTOCOL_G1:
	case PROTOCOL_G2:
		m_tConnect = tNow - Settings.Gnutella.ConnectThrottle + 10;
		break;
	case PROTOCOL_ED2K:
		m_tConnect = tNow - Settings.eDonkey.QueryThrottle + 10;
		break;
	default:
		;
	}
}

DWORD CHostCacheHost::Seen() const
{
	return m_tSeen;
}

CString CHostCacheHost::Address() const
{
	if ( m_pAddress.s_addr != INADDR_ANY )
		return CString( inet_ntoa( m_pAddress ) );
	else
		return m_sAddress;
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

		// Version 14
		ar << m_sCountry;

		// Version 15
		ar << m_bDHT;
		ar.Write( &m_oBtGUID[0], m_oBtGUID.byteCount );

		// Version 16
		ar << m_nUDPPort;
		ar.Write( &m_oGUID[0], m_oGUID.byteCount );
		ar << m_nKADVersion;

		// Version 17
		ar << m_tConnect;

		// Version 18
		ar << m_sUser;
		ar << m_sPass;

		// Version 18
		ar << m_sAddress;
	}
	else
	{
		ReadArchive( ar, &m_pAddress, sizeof(m_pAddress) );
		ar >> m_nPort;

		ar >> m_tAdded;

		DWORD tNow = static_cast< DWORD >( time( NULL ) );
		ar >> m_tSeen;
		if ( m_tSeen > tNow )
		{
			m_tSeen = tNow;
		}
		ar >> m_tRetryAfter;

		CHAR szaVendor[4] = { 0, 0, 0, 0 };
		ar >> szaVendor[0];

		if ( szaVendor[0] )
		{
			ReadArchive( ar, szaVendor + 1, 3 );
			TCHAR szVendor[5] = { (TCHAR)szaVendor[0], (TCHAR)szaVendor[1], (TCHAR)szaVendor[2], (TCHAR)szaVendor[3], 0 };
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

		if ( nVersion >= 18 )
		{
			ar >> m_sUser;
			ar >> m_sPass;
		}

		if ( nVersion >= 19 )
		{
			ar >> m_sAddress;
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

		if ( m_nProtocol == PROTOCOL_ED2K )
			m_nUDPPort += 4;
	}

	DWORD tNow = static_cast< DWORD >( time( NULL ) );
	if ( ! tSeen || tSeen > tNow )
	{
		tSeen = tNow;
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

bool CHostCacheHost::ConnectTo(BOOL bAutomatic)
{
	m_tConnect = static_cast< DWORD >( time( NULL ) );

	if ( m_pAddress.s_addr != INADDR_ANY )
		return Neighbours.ConnectTo( m_pAddress, m_nPort, m_nProtocol, bAutomatic ) != NULL;

	m_tConnect += 30; // Throttle for 30 seconds
	return Network.ConnectTo( m_sAddress, m_nPort, m_nProtocol ) != FALSE;
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost string

CString CHostCacheHost::ToString(const bool bLong) const
{
	CString str;
	if ( bLong )
	{
		time_t tSeen = m_tSeen;
		tm time = {};
		if ( gmtime_s( &time, &tSeen ) == 0 )
		{
			str.Format( _T("%s:%u %.4i-%.2i-%.2iT%.2i:%.2iZ"),
				(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort,
				time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
				time.tm_hour, time.tm_min ); // 2002-04-30T08:30Z
			return str;
		}
	}
	str.Format( _T("%s:%u"),
		(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort );
	return str;
}

bool CHostCacheHost::IsExpired(const DWORD tNow) const
{
	switch ( m_nProtocol )
	{
	case PROTOCOL_G1:
		return m_tSeen && ( tNow > m_tSeen + Settings.Gnutella1.HostExpire );
	case PROTOCOL_G2:
		return m_tSeen && ( tNow > m_tSeen + Settings.Gnutella2.HostExpire );
	case PROTOCOL_ED2K:
	case PROTOCOL_DC:
		return false;	// Never
	case PROTOCOL_BT:
		return m_tSeen && ( tNow > m_tSeen + Settings.BitTorrent.HostExpire );
	case PROTOCOL_KAD:
		return m_tSeen && ( tNow > m_tSeen + 24 * 60 * 60 ); // TODO: Add Kademlia setting
	default:
		return false;
	}
}

bool CHostCacheHost::IsThrottled(DWORD tNow) const
{
	// Don't overload network name resolver
	if ( m_pAddress.s_addr == INADDR_ANY && Network.GetResolveCount() > 3 )
		return true;

	if ( tNow < m_tConnect + Settings.Connection.ConnectThrottle / 1000 )
		return true;

	switch ( m_nProtocol )
	{
	case PROTOCOL_G1:
	case PROTOCOL_G2:
		return ( tNow < m_tConnect + Settings.Gnutella.ConnectThrottle );
	case PROTOCOL_ED2K:
		return ( tNow < m_tConnect + Settings.eDonkey.QueryThrottle );
	case PROTOCOL_BT:
		return ( tNow < m_tConnect + Settings.BitTorrent.ConnectThrottle );
	default:
		return false;
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost connection test

bool CHostCacheHost::CanConnect(DWORD tNow) const
{
	// Don't connect to self
	if ( Settings.Connection.IgnoreOwnIP && Network.IsSelfIP( m_pAddress ) ) return false;

	return
		// Let failed host rest some time...
		( ! m_tFailure || ( tNow >= m_tFailure + Settings.Connection.FailurePenalty ) ) &&
		// ...and we lost no hope on this host...
		( m_nFailures <= Settings.Connection.FailureLimit ) &&
		// ...and host isn't expired...
		( m_bPriority || ! IsExpired( tNow ) ) &&
		// ...and make sure we reconnect not too fast...
		( ! IsThrottled( tNow ) );
		// ...then we can connect!
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost quote test

bool CHostCacheHost::CanQuote(const DWORD tNow) const
{
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
bool CHostCacheHost::CanQuery(DWORD tNow) const
{
	switch ( m_nProtocol )
	{
	case PROTOCOL_G2:
		// Must support G2
		if ( ! Settings.Gnutella2.EnableToday ) return false;

		// Must not be waiting for an ack
		if ( 0 != m_tAck ) return false;

		// Must be a recently seen (current) host
		if ( tNow > m_tSeen + Settings.Gnutella2.HostCurrent ) return false;

		// Retry After
		if ( 0 != m_tRetryAfter && tNow < m_tRetryAfter ) return false;

		// Online
		if ( ! Network.IsConnected() ) return false;

		// If haven't queried yet, its ok
		if ( 0 == m_tQuery ) return true;

		// Don't query too fast
		return ( tNow >= m_tQuery + Settings.Gnutella2.QueryThrottle );

	case PROTOCOL_ED2K:
		// Must support ED2K
		if ( ! Settings.eDonkey.EnableToday ) return false;

		if ( ! Settings.eDonkey.ServerWalk ) return false;

		// Must not be waiting for an ack
		if ( 0 != m_tAck ) return false;

		// Retry After
		if ( 0 != m_tRetryAfter && tNow < m_tRetryAfter ) return false;

		// Online
		if ( ! Network.IsConnected() ) return false;

		// If haven't queried yet, its ok
		if ( 0 == m_tQuery ) return true;

		// Don't query too fast
		return ( tNow >= m_tQuery + Settings.eDonkey.QueryThrottle );

	default:
		return false;
	}
}

//////////////////////////////////////////////////////////////////////
// CHostCacheHost query key submission

void CHostCacheHost::SetKey(const DWORD nKey, const IN_ADDR* pHost)
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