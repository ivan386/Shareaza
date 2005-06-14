//
// DiscoveryServices.cpp
//
//	Date:			"$Date: 2005/06/14 17:09:16 $"
//	Revision:		"$Revision: 1.35 $"
//  Last change by:	"$Author: mogthecat $"
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
#include "DiscoveryServices.h"
#include "Network.h"
#include "HostCache.h"
#include "Neighbours.h"
#include "Neighbour.h"
#include "Packet.h"
#include "Buffer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDiscoveryServices DiscoveryServices;

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices construction

CDiscoveryServices::CDiscoveryServices()
{
	m_hThread		= NULL;
	m_hInternet		= NULL;
	m_hRequest		= NULL;
	m_pWebCache		= NULL;
	m_nWebCache		= 0;
	m_tQueried		= 0;
	m_nLastQueryProtocol = PROTOCOL_NULL;
	m_tUpdated		= 0;
	m_nLastUpdateProtocol = PROTOCOL_NULL;
	m_tExecute		= 0;
	m_bFirstTime	= TRUE;
	m_tMetQueried	= 0;
}

CDiscoveryServices::~CDiscoveryServices()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices list access

POSITION CDiscoveryServices::GetIterator() const
{
	return m_pList.GetHeadPosition();
}

CDiscoveryService* CDiscoveryServices::GetNext(POSITION& pos) const
{
	return (CDiscoveryService*)m_pList.GetNext( pos );
}

BOOL CDiscoveryServices::Check(CDiscoveryService* pService, int nType) const
{
	if ( pService == NULL ) return FALSE;
	if ( m_pList.Find( pService ) == NULL ) return FALSE;
	return ( nType < 0 ) || ( pService->m_nType == nType );
}

int CDiscoveryServices::GetCount(int nType, PROTOCOLID nProtocol) const
{
	int nCount = 0;
	CDiscoveryService* ptr;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		ptr = GetNext( pos );
		if ( ( nType == CDiscoveryService::dsNull ) || ( ptr->m_nType == nType ) )	// If we're counting all types, or it matches
		{
			if ( ( nProtocol == PROTOCOL_NULL ) ||									// If we're counting all protocols
			   ( ( nProtocol == PROTOCOL_G1   ) && ptr->m_bGnutella1 ) ||			// Or we're counting G1 and it matches
			   ( ( nProtocol == PROTOCOL_G2   ) && ptr->m_bGnutella2 ) ||			// Or we're counting G2 and it matches
			   ( ( nProtocol == PROTOCOL_ED2K ) && ptr->m_nType == CDiscoveryService::dsServerMet ) ) // Or we're counting ED2K
			{
			   nCount++;
			}
		}
	}
	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices list modification

CDiscoveryService* CDiscoveryServices::Add(LPCTSTR pszAddress, int nType, PROTOCOLID nProtocol)
{
	CString strAddress( pszAddress );

	// Trim any excess whitespace.
	strAddress.TrimLeft();
	strAddress.TrimRight();

	// Trim garbage on the end- sometimes you get "//", "./", "./." etc. (Bad caches)
	while ( strAddress.GetLength() >= 8 )
	{
		if ( strAddress.Right( 2 ) == _T("//") )
			strAddress = strAddress.Left( strAddress.GetLength() - 1 );
		else if ( strAddress.Right( 2 ) == _T("./") )
			strAddress = strAddress.Left( strAddress.GetLength() - 2 );
		else if ( strAddress.GetAt( strAddress.GetLength() - 1 ) == '.' )
			strAddress = strAddress.Left( strAddress.GetLength() - 1 );
		else break;

	}

	/*
	// Although this is part of the spec, it was removed at the request of the GDF.
	// Trim trailing '/'
	if ( strAddress.GetAt( strAddress.GetLength() - 1 ) == '/' )
		strAddress = strAddress.Left( strAddress.GetLength() - 1 );
	*/

	// Reject impossibly short services
	if ( strAddress.GetLength() < 8 ) return NULL;

	CSingleLock pNetworkLock( &Network.m_pSection );
	if ( ! pNetworkLock.Lock( 250 ) ) return NULL;
	
	if ( GetByAddress( strAddress ) != NULL ) return NULL;

	CDiscoveryService* pService = NULL;

	switch ( nType )
	{
	case CDiscoveryService::dsWebCache:
		if ( CheckWebCacheValid( pszAddress ) )
			pService = new CDiscoveryService( CDiscoveryService::dsWebCache, strAddress );
		break;

	case CDiscoveryService::dsServerMet:
		if ( CheckWebCacheValid( pszAddress ) )
			pService = new CDiscoveryService( CDiscoveryService::dsServerMet, strAddress );
		break;

	case CDiscoveryService::dsGnutella:
		if ( _tcschr( pszAddress, '.' ) != NULL )
			pService = new CDiscoveryService( CDiscoveryService::dsGnutella, strAddress );
		break;

	case CDiscoveryService::dsBlocked:
		pService = new CDiscoveryService( CDiscoveryService::dsBlocked, strAddress );
		break;
	}
		
	if ( pService == NULL ) return NULL;
	
	// Set the appropriate protocol flags
	switch( nProtocol )
	{
	case PROTOCOL_ED2K:
		pService->m_bGnutella2 = FALSE;
		pService->m_bGnutella1 = FALSE;
		break;
	case PROTOCOL_G2:
		pService->m_bGnutella2 = TRUE;
		pService->m_bGnutella1 = FALSE;
		break;
	case PROTOCOL_G1:
		pService->m_bGnutella2 = FALSE;
		pService->m_bGnutella1 = TRUE;
		break;
	default:
		pService->m_bGnutella2 = TRUE;
		pService->m_bGnutella1 = TRUE;
		break;
	}

	return Add( pService );
}

CDiscoveryService* CDiscoveryServices::Add(CDiscoveryService* pService)
{
	if ( pService == NULL ) return NULL; // Can't add a null

	// If it's a webcache with no protocols set, assume it's for both.
	if ( ( pService->m_bGnutella2 == FALSE ) && ( pService->m_bGnutella1 == FALSE ) && ( pService->m_nType == CDiscoveryService::dsWebCache ) )
	{		
		pService->m_bGnutella2 = TRUE;
		pService->m_bGnutella1 = TRUE;
	}

	// Stop if we already have enough caches
	if ( ( pService->m_bGnutella2 && ( GetCount( PROTOCOL_G2 ) >= Settings.Discovery.CacheCount ) ) ||
		 ( pService->m_bGnutella1 && ( GetCount( PROTOCOL_G1 ) >= Settings.Discovery.CacheCount ) ) )
	{
		// Check if the service is already in the list.
		if ( m_pList.Find( pService ) == NULL )
		{
			// It's a new service, but we don't want more. We should delete it.
			theApp.Message( MSG_DEBUG, _T("Maximum discovery service count reached- %s not added"), pService->m_sAddress );
			delete pService;
			return NULL;
		}
		else
		{
			// We already had this service on the list. Do nothing.
			return pService;
		}
	}

	// Add the service to the list if it's not there already
	if ( m_pList.Find( pService ) == NULL ) m_pList.AddTail( pService );
	return pService;
}

void CDiscoveryServices::Remove(CDiscoveryService* pService, BOOL bCheck)
{
	if ( POSITION pos = m_pList.Find( pService ) ) m_pList.RemoveAt( pos );
	delete pService;
	
	if ( bCheck && ! EnoughServices() )
	{
		AddDefaults();
	}
}

BOOL CDiscoveryServices::CheckWebCacheValid(LPCTSTR pszAddress)
{
	// Check it's long enough
	if ( _tcsclen( pszAddress ) < 12 ) return FALSE;

	// Check it's not blocked
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDiscoveryService* pService = GetNext( pos );
		
		if ( pService->m_nType == CDiscoveryService::dsBlocked )
		{
			if ( _tcsistr( pszAddress, pService->m_sAddress ) != NULL )
				return FALSE;
		}
	}

	// Check it has a valid protocol
	if ( _tcsnicmp( pszAddress, _T("http://"),  7 ) == 0 ) 
		pszAddress += 7;
	else if ( _tcsnicmp( pszAddress, _T("https://"), 8 ) == 0 ) 
		pszAddress += 8;
	else
		 return FALSE;

	// Scan through, make sure there are some '.' in there.
	pszAddress = _tcschr( pszAddress, '.' );
	if ( pszAddress == NULL ) return FALSE;

	// And check we have a '/' as well
	pszAddress = _tcschr( pszAddress, '/' );
	if ( pszAddress == NULL ) return FALSE;

	// Probably okay
	return TRUE;
}

BOOL CDiscoveryServices::CheckMinimumServices()
{
	// Add the default services if we don't have enough
	if ( ! EnoughServices() )
	{
		AddDefaults();
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices execute a service to get hosts

// WARNING: Way too agressive for general use- Be very careful where this is called!
// This is a public function, and should be called once when setting up/installing the program. 
// IE: In the Quickstart Wizard *only*
// Using this for general querying would overload services and get you blacklisted.
BOOL CDiscoveryServices::QueryForHosts( PROTOCOLID nProtocol )
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;

	for ( int nLoop = 0 ; nLoop < 3 ; nLoop ++ )
	{
		if ( RequestRandomService( nProtocol ) )
			return TRUE;
	}
	
	return FALSE;
}

CDiscoveryService* CDiscoveryServices::GetByAddress(LPCTSTR pszAddress) const
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDiscoveryService* pService = GetNext( pos );
		
		int nLen = pService->m_sAddress.GetLength();
		if ( nLen > 20 )
		{
			// If it's a long webcache address, ignore the last few characters when checking
			if ( _tcsnicmp( pService->m_sAddress, pszAddress, nLen - 2 ) == 0 )
				return pService;
		}
		else
		{
			if ( pService->m_sAddress.CompareNoCase( pszAddress ) == 0 )
				return pService;
		}
	}

	return NULL;
}

void CDiscoveryServices::Clear()
{
	Stop();

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		delete GetNext( pos );
	}

	m_pList.RemoveAll();
}

void CDiscoveryServices::Stop()
{
	StopWebRequest();
}


//////////////////////////////////////////////////////////////////////
// CDiscoveryServices load and save

BOOL CDiscoveryServices::Load()
{
	CSingleLock pLock( &Network.m_pSection, TRUE );
	CFile pFile;
	
	CString strFile = Settings.General.UserPath + _T("\\Data\\Discovery.dat");
	
	// Load the services from disk
	if ( ! pFile.Open( strFile, CFile::modeRead ) )
	{
		AddDefaults();
		Save();
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
		pException->Delete();
		pFile.Close();
		Clear();
		AddDefaults();
		Save();
		return FALSE;
	}
	
	pFile.Close();
	
	// Check we have the minimum number of services (in case of file corruption, etc)
	if ( ! EnoughServices() )
	{
		AddDefaults();	// Re-add the default list
		Save();			// And save it
	}
	
	return TRUE;
}

BOOL CDiscoveryServices::Save()
{
	CSingleLock pLock( &Network.m_pSection, TRUE );
	CFile pFile;

	CString strFile = Settings.General.UserPath + _T("\\Data\\Discovery.dat");
	if ( !pFile.Open( strFile, CFile::modeWrite|CFile::modeCreate ) )
		return FALSE;

	CArchive ar( &pFile, CArchive::store );
	Serialize( ar );
	ar.Close();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices serialize

void CDiscoveryServices::Serialize(CArchive& ar)
{
	int nVersion = 6;
	
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
		if ( nVersion < 6 ) return;
		
		for ( int nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			CDiscoveryService* pService = new CDiscoveryService();
			pService->Serialize( ar, nVersion );
			m_pList.AddTail( pService );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices Check we have the minimum number of services

BOOL CDiscoveryServices::EnoughServices() const
{
	int nWebCacheCount = 0, nServerMetCount = 0;	// Types of services
	int nG1Count = 0, nG2Count = 0;					// Protocols
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDiscoveryService* pService = GetNext( pos );
		if ( pService->m_nType == CDiscoveryService::dsWebCache )
		{
			nWebCacheCount++;

			if ( pService->m_bGnutella1 ) nG1Count++;
			if ( pService->m_bGnutella2 ) nG2Count++;
		}
		else if ( pService->m_nType == CDiscoveryService::dsServerMet )
		{
			nServerMetCount ++;
		}
	}

	return ( ( nWebCacheCount   > 4 ) &&	// At least 5 webcaches
		     ( nG2Count			> 2 ) &&	// At least 3 G2 services
			 ( nG1Count			> 0 ) &&	// At least 1 G1 service
			 ( nServerMetCount  > 0 ) );	// At least 1 server.met
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices defaults

void CDiscoveryServices::AddDefaults()
{
	CFile pFile;
	CString strFile = Settings.General.Path + _T("\\Data\\DefaultServices.dat");

	if (  pFile.Open( strFile, CFile::modeRead ) )			// Load default list from file if possible
	{
		theApp.Message( MSG_DEFAULT, _T("Loading default discovery service list") );

		try
		{
			CString strService;
			CString strLine;
			CBuffer pBuffer;
			TCHAR cType;

			pBuffer.EnsureBuffer( (DWORD)pFile.GetLength() );
			pBuffer.m_nLength = (DWORD)pFile.GetLength();
			pFile.Read( pBuffer.m_pBuffer, pBuffer.m_nLength );
			pFile.Close();

			while ( pBuffer.ReadLine( strLine ) )
			{
				if ( strLine.GetLength() < 7 ) continue;									// Blank comment line

				cType = strLine.GetAt( 0 );
				strService = strLine.Right( strLine.GetLength() - 2 );

				switch( cType )
				{
				case '1': Add( strService, CDiscoveryService::dsWebCache, PROTOCOL_G1 );	// G1 service
					break;
				case '2': Add( strService, CDiscoveryService::dsWebCache, PROTOCOL_G2 );	// G2 service
					break;
				case 'M': Add( strService, CDiscoveryService::dsWebCache );					// Multinetwork service
					break;
				case 'D': Add( strService, CDiscoveryService::dsServerMet, PROTOCOL_ED2K );	// eDonkey service
					break;
				case 'X': Add( strService, CDiscoveryService::dsBlocked );					// Blocked service
					break;
				case '#':																	// Comment line
					break;
				}
			}
		}
		catch ( CException* pException )
		{
			if (pFile.m_hFile != CFile::hFileNull) pFile.Close(); // Check if file is still open, if yes close
			pException->Delete();
		}
	}
	
	// If file can't be used or didn't have enough services, drop back to the the in-built list
	if ( ! EnoughServices() )
	{
		theApp.Message( MSG_ERROR, _T("Default discovery service load failed- using application defined list.") );
		CString strServices;
		strServices.LoadString( IDS_DISCOVERY_DEFAULTS );
	
		for ( strServices += '\n' ; strServices.GetLength() ; )
		{
			CString strService = strServices.SpanExcluding( _T("\r\n") );
			strServices = strServices.Mid( strService.GetLength() + 1 );
		
			if ( strService.GetLength() > 0 )
			{
				if ( _tcsistr( strService, _T("server.met") ) == NULL )
					Add( strService, CDiscoveryService::dsWebCache );
				else
					Add( strService, CDiscoveryService::dsServerMet, PROTOCOL_ED2K );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices update

BOOL CDiscoveryServices::Update()
{
	PROTOCOLID nProtocol;
	DWORD tNow = (DWORD)time( NULL );
	
	// Don't update too frequently
	if ( tNow - m_tUpdated < Settings.Discovery.UpdatePeriod ) return FALSE;
	
	if ( m_hInternet ) return FALSE;
	StopWebRequest();
	
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;
	
	// Must be up for two hours
	if ( Network.GetStableTime() < 7200 ) return FALSE;				

	// Determine which network/protocol to update
	if ( Neighbours.IsG2Hub() )						// G2 hub mode is active
	{
		if ( Neighbours.IsG1Ultrapeer() )			// G2 and G1 are active
		{	
			// Update the one we didn't update last time
			if ( m_nLastUpdateProtocol == PROTOCOL_G2 ) nProtocol = PROTOCOL_G1;
			else nProtocol = PROTOCOL_G2;
		}
		else										// Only G2 is active
			nProtocol = PROTOCOL_G2;
	}
	else if ( Neighbours.IsG1Ultrapeer() )			// Only G1 active
		nProtocol = PROTOCOL_G1;
	else											// No protocols active- no updates
		return FALSE;

//*** ToDo: If you don't have leafs, you aren't an UP. If you aren't an UP, you don't advertise 
// for leafs! This means Neighbours.IsG1Ultrapeer() will never be true...
	ASSERT ( ( nProtocol == PROTOCOL_G1 ) || ( nProtocol == PROTOCOL_G2 ) );

	// Must have at least 4 peers
	if ( Neighbours.GetCount( nProtocol, -1, ntNode ) < 4 ) return FALSE;	
		
	// Select a random webcache of the approprate sort
	CDiscoveryService* pService = GetRandomWebCache(nProtocol, TRUE, NULL, TRUE );
	if ( pService == NULL ) return FALSE;
		
	// Make the update request
	return RequestWebCache( pService, wcmUpdate, nProtocol );

}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices execute

// This is called when trying to connect to a network, and at intervals while connected. 
// Makes sure you have hosts available to connect to. Be very careful not to be agressive here. 
// You should never query server.met files, because of the load it would create.
// This is public, and will be called quite regularly.

BOOL CDiscoveryServices::Execute(BOOL bSecondary)
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;
	DWORD tNow = time( NULL );

	if ( bSecondary )
	{
		if ( m_hInternet ) return FALSE;
		if ( tNow - m_tQueried < 60 ) return FALSE;
		if ( tNow - m_tExecute < 10 ) return FALSE;
	}
	else
	{
		theApp.Message( MSG_SYSTEM, IDS_DISCOVERY_BOOTSTRAP );
	}
	
	m_tExecute = tNow;
	DWORD	nG1Hosts = HostCache.Gnutella1.CountHosts();
	DWORD	nG2Hosts = HostCache.Gnutella2.CountHosts();
	BOOL	bG1Required = Settings.Gnutella1.EnableToday && ( nG1Hosts < 200 );
	BOOL	bG2Required = Settings.Gnutella2.EnableToday && ( nG2Hosts < 800 );

	if ( ! bSecondary ) // If this is a user-initiated manual query (Or the 'on-startup' query)
	{
		ExecuteBootstraps( Settings.Discovery.BootstrapCount );
		
		if ( ( bG2Required ) && ( nG2Hosts < 25 ) && RequestRandomService( PROTOCOL_G2 ) )
			return TRUE;
		
		if ( ( bG1Required ) && ( nG1Hosts < 15 ) && RequestRandomService( PROTOCOL_G1 ) )
			return TRUE;

		/*
		// Note: Do not enable until we have a MET file set up!
		if ( ( Settings.eDonkey.EnableToday ) && ( Settings.eDonkey.MetAutoQuery ) &&
			 ( HostCache.eDonkey.CountHosts() < 3 ) && ( m_tMetQueried == 0 ) )
		{	// Execute this once only! It's not a webcache...
			m_tMetQueried = tNow;
			if ( RequestRandomService( PROTOCOL_ED2K ) ) return TRUE;
		}
		*/
		
		if ( ( bG1Required ) && ( m_nLastQueryProtocol == PROTOCOL_G2 ) )
			return RequestRandomService( PROTOCOL_G1 );
		else
			return RequestRandomService( PROTOCOL_G2 );
	}

	// A general request. (We have to be very careful not to be too agressive here.)

	// Select the network protocol we want to request hosts from
	if ( bG1Required && bG2Required )					// G1 + G2 hosts are wanted
	{
		// Select which protocol should be requested
		if ( nG2Hosts < 25 )
			return RequestRandomService( PROTOCOL_G2 );
		else if ( nG1Hosts < 15 )
			return RequestRandomService( PROTOCOL_G1 );
		else if ( Neighbours.NeedMoreHubs( PROTOCOL_G2 ) || ( nG2Hosts <= nG1Hosts ) )
			return RequestRandomService( PROTOCOL_G2 );
		else
			return RequestRandomService( PROTOCOL_G1 );
	}
	else if ( bG2Required )								// Only G2
	{
		return RequestRandomService( PROTOCOL_G2 );	
	}
	else if ( bG1Required )								// Only G1
	{
		return RequestRandomService( PROTOCOL_G1 );	
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices resolve N gnutella bootstraps

int CDiscoveryServices::ExecuteBootstraps(int nCount)
{
	CPtrArray pRandom;
	int nSuccess;

	if ( nCount < 1 ) return 0;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDiscoveryService* pService = GetNext( pos );
		if ( pService->m_nType == CDiscoveryService::dsGnutella )
			pRandom.Add( pService );
	}

	srand( GetTickCount() );

	for ( nSuccess = 0 ; nCount > 0 && pRandom.GetSize() > 0 ; )
	{
		int nRandom = rand() % pRandom.GetSize();
		CDiscoveryService* pService = (CDiscoveryService*)pRandom.GetAt( nRandom );
		pRandom.RemoveAt( nRandom );

		if ( pService->ResolveGnutella() )
		{
			nSuccess++;
			nCount--;
		}
	}

	return nSuccess;
}

void CDiscoveryServices::OnGnutellaAdded(IN_ADDR* pAddress, int nCount)
{
	// Find this host somehow and add to m_nHosts
}

void CDiscoveryServices::OnGnutellaFailed(IN_ADDR* pAddress)
{
	// Find this host and add to m_nFailures, and delete if excessive
}


//////////////////////////////////////////////////////////////////////
// CDiscoveryServices RequestRandomService

// Execute a random service (of any type) for any given protocol. Used to find more
// hosts (to connect to a network).

BOOL CDiscoveryServices::RequestRandomService(PROTOCOLID nProtocol)
{
	//CSingleLock pLock( &Network.m_pSection );	// Note: This shouldn't be necessary, since the
	//if ( ! pLock.Lock( 250 ) ) return FALSE;	// calling functions should lock...

	CDiscoveryService* pService = GetRandomService( nProtocol );

	if ( pService )
	{
		if ( pService->m_nType == CDiscoveryService::dsServerMet )
		{
			if ( RequestWebCache( pService, wcmServerMet, nProtocol ) ) return TRUE;
		}
		else
		{
			if ( RequestWebCache( pService, wcmHosts, nProtocol ) ) return TRUE;
		}
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices GetRandomService 

// Selects a random discovery service of any type for any given protocol. Called by 
// RequestRandomService(), and used when finding more hosts (to connect to a network).

CDiscoveryService* CDiscoveryServices::GetRandomService(PROTOCOLID nProtocol)
{
	CPtrArray pServices;
	DWORD tNow = time( NULL );

	// Loops through all services
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDiscoveryService* pService = GetNext( pos );

		// If this one hasn't been recently accessed
		if ( tNow - pService->m_tAccessed > pService->m_nAccessPeriod )
		{
			// Then add it to the possible list (if it's of the right sort)
			switch ( nProtocol )
			{
				case PROTOCOL_G1:
					if ( ( pService->m_nType == CDiscoveryService::dsWebCache ) && ( pService->m_bGnutella1 ) )
						pServices.Add( pService );
					break;
				case PROTOCOL_G2:
					if ( ( pService->m_nType == CDiscoveryService::dsWebCache ) && ( pService->m_bGnutella2 ) )
						pServices.Add( pService );
					break;
				case PROTOCOL_ED2K:
					if ( pService->m_nType == CDiscoveryService::dsServerMet )
						pServices.Add( pService );
					break;
				default:
					break;
			}
		}
	}

	// Select a random service from the list of possible ones.
	if ( pServices.GetSize() > 0 )	// If the list of possible ones isn't empty
	{
		// return a random service
		srand( GetTickCount() );
		return (CDiscoveryService*)pServices.GetAt( rand() % pServices.GetSize() );
	}
	else							// else (No services available)
	{
		// return NULL to indicate none available
		return NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices select a random webcache (For updates, etc)

CDiscoveryService* CDiscoveryServices::GetRandomWebCache(PROTOCOLID nProtocol, BOOL bWorkingOnly, CDiscoveryService* pExclude, BOOL bForUpdate)
{	// Select a random webcache (G1/G2 only)
	CPtrArray pWebCaches;
	DWORD tNow = time( NULL );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDiscoveryService* pService = GetNext( pos );

		if ( pService->m_nType == CDiscoveryService::dsWebCache && pService != pExclude )
		{
			if ( ! bWorkingOnly || ( pService->m_nAccesses > 0 && pService->m_nFailures == 0 && pService->m_nHosts > 0 ) )
			{
				if ( tNow - pService->m_tAccessed > pService->m_nAccessPeriod )
				{
					if ( ! bForUpdate || tNow - pService->m_tUpdated > pService->m_nUpdatePeriod )
					{
						switch ( nProtocol )
						{
							case PROTOCOL_G1:
								if ( ( pService->m_nType == CDiscoveryService::dsWebCache ) && ( pService->m_bGnutella1 ) )
									pWebCaches.Add( pService );
								break;
							case PROTOCOL_G2:
								if ( ( pService->m_nType == CDiscoveryService::dsWebCache ) && ( pService->m_bGnutella2 ) )
									pWebCaches.Add( pService );
								break;
							default:
								theApp.Message( MSG_ERROR, _T("CDiscoveryServices::GetRandomWebCache() was passed an invalid protocol") );
								ASSERT( FALSE );
								return NULL;
						}
					}
				}
			}
		}
	}
	
	// If there are any available web caches
	if ( pWebCaches.GetSize() > 0 )
	{
		// Select a random one
		srand( GetTickCount() );
		return (CDiscoveryService*)pWebCaches.GetAt( rand() % pWebCaches.GetSize() );
	}
	else
	{
		// return null to indicate none available
		return NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices webcache request control

BOOL CDiscoveryServices::RequestWebCache(CDiscoveryService* pService, int nMode, PROTOCOLID nProtocol)
{
	DWORD tNow = (DWORD)time( NULL );
	StopWebRequest();
	
	if ( pService != NULL )
	{
		if ( time( NULL ) - pService->m_tAccessed < pService->m_nAccessPeriod ) return FALSE;
	}

	switch ( nProtocol )
	{
	case PROTOCOL_G1:
		theApp.Message( MSG_DEBUG, _T("CDiscoveryServices::RequestWebCache() seeking gnutella hosts") );
		break;
	case PROTOCOL_G2:
		theApp.Message( MSG_DEBUG, _T("CDiscoveryServices::RequestWebCache() seeking G2 hosts") );
		break;
	case PROTOCOL_ED2K:
		theApp.Message( MSG_DEBUG, _T("CDiscoveryServices::RequestWebCache() seeking ed2k hosts") );
		break;
	default:
		theApp.Message( MSG_ERROR, _T("ERROR: CDiscoveryServices::RequestWebCache() was passed an invalid protocol") );
		return FALSE;
	}
	
	m_pWebCache	= pService;
	m_nWebCache	= nMode;
	m_hRequest	= NULL;
	
	switch ( nMode )
	{
	case wcmHosts:
	case wcmCaches:
		if ( m_pWebCache != NULL ) 
		{	
			theApp.Message( MSG_SYSTEM, IDS_DISCOVERY_QUERY, (LPCTSTR)m_pWebCache->m_sAddress );
			// Update the 'last queried' settings
			m_tQueried = tNow;
			m_nLastQueryProtocol = nProtocol;
		}
		break;

	case wcmUpdate:
		m_pSubmit	= GetRandomWebCache( nProtocol, TRUE, m_pWebCache, FALSE );
		if ( m_pWebCache != NULL ) 
		{
			theApp.Message( MSG_SYSTEM, IDS_DISCOVERY_UPDATING, (LPCTSTR)m_pWebCache->m_sAddress );
			// Update the 'last updated' settings
			m_tUpdated = tNow;
			m_nLastUpdateProtocol = nProtocol;
		}
		break;

	case wcmSubmit:
		m_pSubmit	= m_pWebCache;
		m_pWebCache	= GetRandomWebCache( nProtocol, FALSE, m_pSubmit, TRUE );
		if ( m_pWebCache != NULL ) 
		{		
			theApp.Message( MSG_SYSTEM, IDS_DISCOVERY_SUBMIT, (LPCTSTR)m_pWebCache->m_sAddress );
			// Update the 'last queried' settings
			m_tQueried = tNow;
			m_nLastQueryProtocol = nProtocol;
		}
		break;

	case wcmServerMet:
		if ( nProtocol != PROTOCOL_ED2K )
		{
			theApp.Message( MSG_ERROR, _T("ERROR: CDiscoveryServices::RequestWebCache() was passed wcmServerMet with non-ed2k protocol") );
			ASSERT ( FALSE );
			return FALSE;
		}
		else if ( m_pWebCache != NULL ) 
		{	
			theApp.Message( MSG_SYSTEM, IDS_DISCOVERY_QUERY, (LPCTSTR)m_pWebCache->m_sAddress );
			// Update the 'last queried' settings
			m_tQueried = tNow;
			m_nLastQueryProtocol = nProtocol;
		}
		break;
	default:
		theApp.Message( MSG_ERROR, _T("ERROR: CDiscoveryServices::RequestWebCache() was passed an invalid mode") );
		ASSERT ( FALSE );
		return FALSE;
	}
	
	if ( m_pWebCache == NULL ) return FALSE;
	
	CString strAgent = Settings.SmartAgent( Settings.General.UserAgent );
	
	m_hInternet = InternetOpen( strAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
	if ( ! m_hInternet ) return FALSE;
	
	CWinThread* pThread = AfxBeginThread( ThreadStart, this, THREAD_PRIORITY_NORMAL );
	m_hThread = pThread->m_hThread;
	
	return TRUE;
}

void CDiscoveryServices::StopWebRequest()
{
	if ( m_hInternet ) InternetCloseHandle( m_hInternet );
	m_hInternet = NULL;
	
	if ( m_hThread == NULL ) return;
	
    int nAttempt = 10;
	for ( ; nAttempt > 0 ; nAttempt-- )
	{
		DWORD nCode;

		if ( ! GetExitCodeThread( m_hThread, &nCode ) ) break;
		if ( nCode != STILL_ACTIVE ) break;
		Sleep( 100 );
	}

	if ( nAttempt == 0 )
	{
		TerminateThread( m_hThread, 0 );
		theApp.Message( MSG_DEBUG, _T("WARNING: Terminating CDiscoveryServices thread.") );
		Sleep( 100 );
	}

	m_hThread = NULL;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices thread bootstrap

UINT CDiscoveryServices::ThreadStart(LPVOID pParam)
{
	CDiscoveryServices* pClass = (CDiscoveryServices*)pParam;
	pClass->OnRun();
	return 0;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices thread run

void CDiscoveryServices::OnRun()
{
	BOOL bSuccess = TRUE;
	
	if ( m_nWebCache == wcmServerMet )
	{
		bSuccess = RunServerMet();
	}
	else if ( m_nWebCache == wcmHosts )
	{
		bSuccess = RunWebCacheGet( FALSE );
		
		if ( bSuccess && m_hInternet )
		{
			CSingleLock pLock( &Network.m_pSection, TRUE );
			
			if ( m_bFirstTime || ( GetCount( CDiscoveryService::dsWebCache ) < (int)Settings.Discovery.Lowpoint ) )
			{
				m_bFirstTime = FALSE;
				pLock.Unlock();
				bSuccess = RunWebCacheGet( TRUE );
			}
		}
	}
	else if ( m_nWebCache == wcmCaches )
	{
		bSuccess = RunWebCacheGet( TRUE );
	}
	else if ( m_nWebCache == wcmUpdate )
	{
		bSuccess = RunWebCacheUpdate();
	}
	else if ( m_nWebCache == wcmSubmit )
	{
		bSuccess = RunWebCacheUpdate();
	}
	
	if ( m_hInternet && ! bSuccess )
	{
		if ( m_nWebCache == wcmUpdate )
		{
			m_tUpdated = 0;
			m_nLastUpdateProtocol = PROTOCOL_NULL;
		}
		else
		{
			m_tQueried = 0;
			m_nLastQueryProtocol = PROTOCOL_NULL;
		}

		CSingleLock pLock( &Network.m_pSection );
		if ( pLock.Lock( 250 ) && Check( m_pWebCache ) ) m_pWebCache->OnFailure();
	}
	
	if ( m_hInternet != NULL )
	{
		if ( m_hRequest != NULL ) InternetCloseHandle( m_hRequest );
		InternetCloseHandle( m_hInternet );
		m_hInternet	= NULL;
		m_hRequest	= NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices execute hosts request

BOOL CDiscoveryServices::RunWebCacheGet(BOOL bCaches)
{
	CSingleLock pLock( &Network.m_pSection, TRUE );
	CString strURL, strOutput;
	
	if ( ! Check( m_pWebCache ) ) return FALSE;
	m_pWebCache->OnAccess();
	
	if ( bCaches )
		strURL = m_pWebCache->m_sAddress + _T("?get=1&urlfile=1");
	else
		strURL = m_pWebCache->m_sAddress + _T("?get=1&hostfile=1");
	
	if ( m_nLastQueryProtocol == PROTOCOL_G2 ) strURL += _T("&net=gnutella2");
	
	pLock.Unlock();

	if ( ! SendWebCacheRequest( strURL, strOutput ) ) return FALSE;
	pLock.Lock();
	
	if ( ! Check( m_pWebCache ) ) return FALSE;
	
	BOOL bSuccess = FALSE;
	int nIP[4], nIPs = 0;

	if ( _tcsistr( strOutput, _T("<html>") ) != NULL )
	{
		// Error- getting HMTL response.
		return FALSE;
	}
	else if ( _tcsistr( strOutput, _T("<font>") ) != NULL )
	{
		// Error- getting HMTL response.
		return FALSE;
	}

	
	for ( strOutput += '\n' ; strOutput.GetLength() ; )
	{
		CString strLine	= strOutput.SpanExcluding( _T("\r\n") );
		strOutput		= strOutput.Mid( strLine.GetLength() + 1 );
		
		strLine.TrimLeft();
		strLine.TrimRight();
		if ( strLine.IsEmpty() ) continue;

		theApp.Message( MSG_DEBUG, _T("GWebCache(get): %s"), (LPCTSTR)strLine );
		
		if ( _tcsnicmp( strLine, _T("h|"), 2 ) == 0 )
		{
			// IP ADDRESS AT: strLine.Mid( 2 )
			// CORRECT (REQUESTED) NETWORK
			
			strLine = strLine.Mid( 2 );
			int nBreak	= strLine.Find( '|' );
			DWORD tSeen	= 0;

			if ( nBreak > 0 )
			{
				int nSeconds = 0;
				_stscanf( strLine.Mid( nBreak + 1 ), _T("%i"), &nSeconds );
				nSeconds = max( 0, min( 18000, nSeconds ) );
				strLine = strLine.Left( nBreak );
				tSeen = time( NULL ) - nSeconds;
			}
			
			if ( m_nLastQueryProtocol == PROTOCOL_G2 )
				HostCache.Gnutella2.Add( strLine, tSeen );
			else
				HostCache.Gnutella1.Add( strLine, tSeen );
			
			m_pWebCache->OnHostAdd();
			bSuccess = TRUE;
			nIPs ++;
		}
		else if ( _tcsnicmp( strLine, _T("u|"), 2 ) == 0 )
		{
			// URL ADDRESS AT: strLine.Mid( 2 )
			// CORRECT (REQUESTED) NETWORK
			Add( strLine.Mid( 2 ).SpanExcluding( _T("|") ), CDiscoveryService::dsWebCache, m_nLastQueryProtocol );
			
			m_bFirstTime = FALSE;
			bSuccess = TRUE;
		}
		else if ( _tcsnicmp( strLine, _T("i|"), 2 ) == 0 )
		{
			// INFORMATIONAL
			
			// Don't count as success if it's only informational
			// bSuccess = TRUE; 
			
			if ( _tcsnicmp( strLine, _T("i|access|period|"), 16 ) == 0 )
			{
				_stscanf( (LPCTSTR)strLine + 16, _T("%u"), &m_pWebCache->m_nAccessPeriod );
			}
			else if ( strLine == _T("i|force|remove") || _tcsnicmp( strLine, _T("i|update|warning|bad url"), 24 ) == 0 )
			{
				m_pWebCache->Remove();
				return FALSE;
			}
			
			if ( _tcsistr( strLine, _T("ERROR: Network not supported") ) != NULL )
			{
				// ERROR CONDITION
				if ( m_nLastQueryProtocol == PROTOCOL_G2 )
				{
					m_pWebCache->m_bGnutella1 = TRUE;
					m_pWebCache->m_bGnutella2 = FALSE;
				}
				else
				{
					m_pWebCache->m_bGnutella1 = FALSE;
					m_pWebCache->m_bGnutella2 = TRUE;
				}

				return FALSE;
			}
		}
		else if ( _tcsistr( strLine, _T("ERROR: Network not supported") ) != NULL )
		{
			// Wrong network (Whoops!)
			if ( m_nLastQueryProtocol == PROTOCOL_G2 )
			{
				m_pWebCache->m_bGnutella1 = TRUE;
				m_pWebCache->m_bGnutella2 = FALSE;
			}
			else
			{
				m_pWebCache->m_bGnutella1 = FALSE;
				m_pWebCache->m_bGnutella2 = TRUE;
			}

			return FALSE;
		}
		else if ( _tcsistr( strLine, _T("ERROR") ) != NULL )
		{
			// Misc error. (Often CGI limits error)
			return FALSE;
		}
		else if ( _stscanf( strLine, _T("%i.%i.%i.%i"), &nIP[0], &nIP[1], &nIP[2], &nIP[3] ) == 4 )
		{
			// Plain IP, NOT G2
			HostCache.Gnutella1.Add( strLine );
			m_pWebCache->OnHostAdd();
			m_pWebCache->m_bGnutella2 = FALSE;
			m_pWebCache->m_bGnutella1 = TRUE;
			bSuccess = TRUE;
		}
		else
		{
			// Plain URL, WRONG NETWORK
			Add( strLine.SpanExcluding( _T(" ") ), CDiscoveryService::dsWebCache, PROTOCOL_G1 );
			m_pWebCache->m_bGnutella2 = FALSE;
			m_pWebCache->m_bGnutella1 = TRUE;
			m_bFirstTime = FALSE;
		}
	}
	
	if ( bSuccess )
	{
		m_pWebCache->OnSuccess();
		if ( HostCache.Gnutella2.GetNewest() != NULL && nIPs > 0 ) m_tQueried = time( NULL );
		return TRUE;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices execute update request

BOOL CDiscoveryServices::RunWebCacheUpdate()
{
	CSingleLock pLock( &Network.m_pSection, TRUE );
	CString strURL, strOutput;

	if ( ! Check( m_pWebCache, CDiscoveryService::dsWebCache ) ) return FALSE;
	m_pWebCache->OnAccess();
	
	if ( m_nWebCache == wcmUpdate )
	{
		if ( ! Network.IsListening() ) return TRUE;

		strURL.Format( _T("%s?update=1&ip=%s:%hu&x.leaves=%i"),
			(LPCTSTR)m_pWebCache->m_sAddress,
			(LPCTSTR)CString( inet_ntoa( Network.m_pHost.sin_addr ) ),
			htons( Network.m_pHost.sin_port ),
			Neighbours.GetCount( -1, -1, ntLeaf ) );		//ToDo: Check this
	}
	
	if ( m_pSubmit != NULL && Check( m_pSubmit, CDiscoveryService::dsWebCache ) &&
		 m_pWebCache->m_bGnutella2 == m_pSubmit->m_bGnutella2 )
	{
		if ( strURL.IsEmpty() )
		{
			strURL.Format( _T("%s?url="), (LPCTSTR)m_pWebCache->m_sAddress );
		}
		else
		{
			strURL += _T("&url=");
		}
		
		CString strSubmit( m_pSubmit->m_sAddress );
		
		for ( int nSubmit = 0 ; nSubmit < strSubmit.GetLength() ; nSubmit ++ )
		{
			if ( (WORD)strSubmit.GetAt( nSubmit ) > 127 )
			{
				strSubmit = strSubmit.Left( nSubmit );
				break;
			}
		}
		
		strURL += CConnection::URLEncode( strSubmit );
	}
	
	if ( m_nLastUpdateProtocol == PROTOCOL_G2 ) strURL += _T("&net=gnutella2");
	
	pLock.Unlock();
	
	if ( strURL.IsEmpty() ) return FALSE;
	
	if ( ! SendWebCacheRequest( strURL, strOutput ) ) return FALSE;
	
	pLock.Lock();
	if ( ! Check( m_pWebCache, CDiscoveryService::dsWebCache ) ) return FALSE;
	
	for ( strOutput += '\n' ; strOutput.GetLength() ; )
	{
		CString strLine	= strOutput.SpanExcluding( _T("\r\n") );
		strOutput		= strOutput.Mid( strLine.GetLength() + 1 );
		
		strLine.TrimLeft();
		strLine.TrimRight();
		
		if ( strLine.IsEmpty() ) continue;
		
		theApp.Message( MSG_DEBUG, _T("GWebCache(update): %s"), (LPCTSTR)strLine );
		
		if ( _tcsstr( strLine, _T("OK") ) )
		{
			m_pWebCache->m_tUpdated = (DWORD)time( NULL );
			m_pWebCache->m_nUpdates++;
			m_pWebCache->OnSuccess();
			return TRUE;
		}
		else if ( _tcsstr( strLine, _T("ERROR") ) )
		{
			return FALSE;
		}
		else if ( _tcsnicmp( strLine, _T("i|access|period|"), 16 ) == 0 )
		{
			_stscanf( (LPCTSTR)strLine + 16, _T("%u"), &m_pWebCache->m_nAccessPeriod );
		}
		else if ( _tcsnicmp( strLine, _T("i|update|period|"), 16 ) == 0 )
		{
			_stscanf( (LPCTSTR)strLine + 16, _T("%u"), &m_pWebCache->m_nUpdatePeriod );
		}
		else if ( strLine == _T("i|force|remove") )
		{
			m_pWebCache->Remove();
			break;
		}
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices HTTP request controllor

BOOL CDiscoveryServices::SendWebCacheRequest(CString strURL, CString& strOutput)
{
	strOutput.Empty();
	
	strURL += _T("&client=RAZA&version=");
	strURL += theApp.m_sVersion;
	
	theApp.Message( MSG_DEBUG, _T("DiscoveryService URL: %s"), (LPCTSTR)strURL );
	
	if ( m_hRequest ) InternetCloseHandle( m_hRequest );
	
	m_hRequest = InternetOpenUrl( m_hInternet, strURL, _T("Connection: close"), -1,
		INTERNET_FLAG_RELOAD|INTERNET_FLAG_DONT_CACHE|INTERNET_FLAG_NO_COOKIES, 0 );
	if ( m_hRequest == NULL ) return FALSE;
	
	DWORD nStatusCode = 0, nStatusLen = 32;
	TCHAR szStatusCode[32];
	
	if ( ! HttpQueryInfo( m_hRequest, HTTP_QUERY_STATUS_CODE, szStatusCode,
		&nStatusLen, NULL ) ) return FALSE;
	
	_stscanf( szStatusCode, _T("%u"), &nStatusCode );
	if ( nStatusCode < 200 || nStatusCode > 299 ) return FALSE;
	
	DWORD nRemaining, nResponse = 0;
	LPBYTE pResponse = NULL;
	
	while ( InternetQueryDataAvailable( m_hRequest, &nRemaining, 0, 0 ) && nRemaining > 0 )
	{
		pResponse = (LPBYTE)realloc( pResponse, nResponse + nRemaining );
		InternetReadFile( m_hRequest, pResponse + nResponse, nRemaining, &nRemaining );
		nResponse += nRemaining;
	}
	
	if ( nRemaining )
	{
		free( pResponse );
		return FALSE;
	}
	
	LPTSTR pszResponse = strOutput.GetBuffer( nResponse );
	for ( nStatusCode = 0 ; nStatusCode < nResponse ; nStatusCode++ )
		pszResponse[ nStatusCode ] = (TCHAR)pResponse[ nStatusCode ];
	strOutput.ReleaseBuffer( nResponse );
	
	free( pResponse );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices execute server.met request

BOOL CDiscoveryServices::RunServerMet()
{
	CSingleLock pLock( &Network.m_pSection, TRUE );
	CString strURL;
	
	if ( ! Check( m_pWebCache, CDiscoveryService::dsServerMet ) ) return FALSE;
	
	m_pWebCache->OnAccess();
	strURL = m_pWebCache->m_sAddress;
	
	pLock.Unlock();
	
	if ( m_hRequest != NULL ) InternetCloseHandle( m_hRequest );
	
	m_hRequest = InternetOpenUrl( m_hInternet, strURL, NULL, 0,
		INTERNET_FLAG_RELOAD|INTERNET_FLAG_DONT_CACHE, 0 );
	
	if ( m_hRequest == NULL ) return FALSE;
	
	DWORD nRemaining = 0;
	BYTE pBuffer[1024];
	CMemFile pFile;
	
	while ( InternetQueryDataAvailable( m_hRequest, &nRemaining, 0, 0 ) && nRemaining > 0 )
	{
		while ( nRemaining > 0 )
		{
			DWORD nBuffer = min( nRemaining, DWORD(1024) );
			InternetReadFile( m_hRequest, pBuffer, nBuffer, &nBuffer );
			pFile.Write( pBuffer, nBuffer );
			nRemaining -= nBuffer;
		}
	}
	
	pFile.Seek( 0, CFile::begin );
	
	pLock.Lock();
	
	if ( ! Check( m_pWebCache, CDiscoveryService::dsServerMet ) ) return FALSE;
	
	int nCount = HostCache.eDonkey.ImportMET( &pFile );
	
	if ( ! nCount ) return FALSE;
	
	HostCache.Save();
	m_pWebCache->OnHostAdd( nCount );
	m_pWebCache->OnSuccess();
	
	return TRUE;
}


//////////////////////////////////////////////////////////////////////
// CDiscoveryService construction

CDiscoveryService::CDiscoveryService(int nType, LPCTSTR pszAddress)
{
	m_nType			= nType;
	m_bGnutella2	= FALSE;
	m_bGnutella1	= FALSE;
	m_tCreated		= (DWORD)time( NULL );
	m_tAccessed		= 0;
	m_nAccesses		= 0;
	m_tUpdated		= 0;
	m_nUpdates		= 0;
	m_nFailures		= 0;
	m_nHosts		= 0;
	m_nAccessPeriod	= max( Settings.Discovery.UpdatePeriod, DWORD(1800) );
	m_nUpdatePeriod	= Settings.Discovery.DefaultUpdate;
	
	if ( pszAddress != NULL ) m_sAddress = pszAddress;
}

CDiscoveryService::~CDiscoveryService()
{
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryService remove

void CDiscoveryService::Remove(BOOL bCheck)
{
	DiscoveryServices.Remove( this, bCheck );
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryService serialize

void CDiscoveryService::Serialize(CArchive& ar, int nVersion)
{

	if ( ar.IsStoring() )
	{
		ar << m_nType;
		ar << m_sAddress;
		ar << m_bGnutella2;
		ar << m_bGnutella1;
		ar << m_tCreated;
		ar << m_tAccessed;
		ar << m_nAccesses;
		ar << m_tUpdated;
		ar << m_nUpdates;
		ar << m_nFailures;
		ar << m_nHosts;
		ar << m_nAccessPeriod;
		ar << m_nUpdatePeriod;
	}
	else
	{
		ar >> m_nType;
		ar >> m_sAddress;
		ar >> m_bGnutella2;
		ar >> m_bGnutella1;
		ar >> m_tCreated;
		ar >> m_tAccessed;
		ar >> m_nAccesses;
		ar >> m_tUpdated;
		ar >> m_nUpdates;
		ar >> m_nFailures;
		ar >> m_nHosts;
		ar >> m_nAccessPeriod;
		ar >> m_nUpdatePeriod;
	}
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryService execute

// Note: This is used by wndDiscovery only
BOOL CDiscoveryService::Execute(int nMode)
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;
	
	if ( m_nType == dsGnutella )
	{
		return ResolveGnutella();
	}
	else if ( m_nType == dsWebCache )
	{
		return DiscoveryServices.RequestWebCache( this, nMode, m_bGnutella2 ? PROTOCOL_G2 : PROTOCOL_G1 );
	}
	else if ( m_nType == dsServerMet )
	{
		return DiscoveryServices.RequestWebCache( this, CDiscoveryServices::wcmServerMet, PROTOCOL_ED2K );
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryService resolve a gnutella node

BOOL CDiscoveryService::ResolveGnutella()
{
	CString strHost	= m_sAddress;
	int nPort		= GNUTELLA_DEFAULT_PORT;
	int nPos		= strHost.Find( ':' );
	
	if ( nPos >= 0 && _stscanf( strHost.Mid( nPos + 1 ), _T("%i"), &nPort ) == 1 )
		strHost = strHost.Left( nPos );
	
	if ( ! Network.Connect( FALSE ) ) return FALSE;
	
	if ( Network.AsyncResolve( strHost, (WORD)nPort, PROTOCOL_G1, 0 ) )
	{
		OnSuccess();
		return TRUE;
	}
	
	OnFailure();
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryService events

void CDiscoveryService::OnAccess()
{
	m_tAccessed = (DWORD)time( NULL );
	m_nAccesses ++;
}

void CDiscoveryService::OnHostAdd(int nCount)
{
	m_nHosts += nCount;
	m_nFailures = 0;
}

void CDiscoveryService::OnSuccess()
{
	m_nFailures = 0;

	if ( m_nType == dsWebCache || m_nType == dsServerMet )
	{
		theApp.Message( MSG_DEFAULT, IDS_DISCOVERY_WEB_SUCCESS,
			(LPCTSTR)m_sAddress );
	}
}

void CDiscoveryService::OnFailure()
{
	m_nFailures++;
	
	theApp.Message( MSG_ERROR, IDS_DISCOVERY_FAILED,
		(LPCTSTR)m_sAddress, m_nFailures );
	
	if ( m_nFailures >= Settings.Discovery.FailureLimit )
	{
		theApp.Message( MSG_ERROR, IDS_DISCOVERY_FAIL_REMOVE,
			(LPCTSTR)m_sAddress, m_nFailures );
		Remove();
	}
}

