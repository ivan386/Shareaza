//
// DiscoveryServices.cpp
//
// Copyright © Shareaza Development Team, 2002-2009.
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
#include "DiscoveryServices.h"
#include "Network.h"
#include "HostCache.h"
#include "Neighbours.h"
#include "Neighbour.h"
#include "Packet.h"
#include "G2Packet.h"
#include "Buffer.h"
#include "Datagrams.h"
#include "Kademlia.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDiscoveryServices DiscoveryServices;

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices construction

CDiscoveryServices::CDiscoveryServices() :
	m_hInternet		( NULL ),
	m_hRequest		( NULL ),
	m_pWebCache		( NULL ),
	m_nWebCache		( wcmHosts ),
	m_pSubmit		( NULL ),
	m_nLastQueryProtocol ( PROTOCOL_NULL ),
	m_tUpdated		( 0 ),
	m_nLastUpdateProtocol ( PROTOCOL_NULL ),
	m_bFirstTime	( TRUE ),
	m_tExecute		( 0 ),
	m_tQueried		( 0 ),
	m_tMetQueried	( 0 )
{
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
	return m_pList.GetNext( pos );
}

BOOL CDiscoveryServices::Check(CDiscoveryService* pService, CDiscoveryService::Type nType) const
{
	if ( pService == NULL ) return FALSE;
	if ( m_pList.Find( pService ) == NULL ) return FALSE;
	return ( nType == CDiscoveryService::dsNull ) || ( pService->m_nType == nType );
}

DWORD CDiscoveryServices::GetCount(int nType, PROTOCOLID nProtocol) const
{
	DWORD nCount = 0;
	CDiscoveryService* ptr;

	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		ptr = m_pList.GetNext( pos );
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
	// Trim garbage on the end- sometimes you get "//", "./", "./." etc. (Bad caches)
	bool bEndSlash = strAddress.GetAt( strAddress.GetLength() - 1 ) == '/';
	strAddress.TrimLeft();
	strAddress.TrimRight();
	strAddress.TrimRight( L"./" );

	if ( bEndSlash ) strAddress.Append( L"/" );

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
		if ( CheckWebCacheValid( pszAddress ) )
		{
			pService = new CDiscoveryService( CDiscoveryService::dsGnutella, strAddress );

			if ( _tcsnicmp( strAddress, _T("gnutella1:host:"),  15 ) == 0 )
			{
				nProtocol = PROTOCOL_G1;
				pService->m_nSubType = CDiscoveryService::dsGnutellaTCP;
			}
			else if ( _tcsnicmp( strAddress, _T("gnutella2:host:"), 15 ) == 0 )
			{
				nProtocol = PROTOCOL_G2;
				pService->m_nSubType = CDiscoveryService::dsGnutella2TCP;
			}
			else if ( _tcsnicmp( strAddress, _T("uhc:"), 4 )  == 0 )
			{
				nProtocol = PROTOCOL_G1;
				pService->m_nSubType = CDiscoveryService::dsGnutellaUDPHC;
			}
			else if ( _tcsnicmp( strAddress, _T("ukhl:"), 5 )  == 0 )
			{
				nProtocol = PROTOCOL_G2;
				pService->m_nSubType = CDiscoveryService::dsGnutella2UDPKHL;
			}
		}
		break;

	case CDiscoveryService::dsBlocked:
		pService = new CDiscoveryService( CDiscoveryService::dsBlocked, strAddress );
		for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
		{
			CDiscoveryService* pItem = m_pList.GetNext( pos );

			if ( _tcsistr( pItem->m_sAddress, pService->m_sAddress ) != NULL )
			{
				if ( pItem->m_nType != CDiscoveryService::dsBlocked )
				{
					pItem->m_nType = CDiscoveryService::dsBlocked;
					delete pService;
					return NULL;
				}
			}
		}

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
	if ( ( pService->m_bGnutella2 && ( GetCount( pService->m_nType, PROTOCOL_G2 ) >= Settings.Discovery.CacheCount ) ) ||
		 ( pService->m_bGnutella1 && ( GetCount( pService->m_nType, PROTOCOL_G1 ) >= Settings.Discovery.CacheCount ) ) )
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
	if ( m_pList.Find( pService ) == NULL )
	{
		m_pList.AddTail( pService );
		MergeURLs();
	}
	return pService;
}

void CDiscoveryServices::Remove(CDiscoveryService* pService, BOOL bCheck)
{
	if ( POSITION pos = m_pList.Find( pService ) ) m_pList.RemoveAt( pos );
	delete pService;

	if ( bCheck )
		CheckMinimumServices();
}


BOOL CDiscoveryServices::CheckWebCacheValid(LPCTSTR pszAddress)
{
	// Check it's long enough
	if ( _tcsclen( pszAddress ) < 12 ) return FALSE;

	// Check it's not blocked
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CDiscoveryService* pService = m_pList.GetNext( pos );

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
	else if ( _tcsnicmp( pszAddress, _T("gnutella1:host:"), 15 ) == 0 )
		return TRUE;
	else if ( _tcsnicmp( pszAddress, _T("gnutella2:host:"), 15 ) == 0 )
		return TRUE;
	else if ( _tcsnicmp( pszAddress, _T("uhc:"), 4 ) == 0 )
		return TRUE;
	else if ( _tcsnicmp( pszAddress, _T("ukhl:"), 5 ) == 0 )
		return TRUE;
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

//DWORD CDiscoveryServices::MetQueried() const
//{
//	return m_tMetQueried;
//}

DWORD CDiscoveryServices::LastExecute() const
{
	return m_tExecute;
}

CDiscoveryService* CDiscoveryServices::GetByAddress(LPCTSTR pszAddress) const
{
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CDiscoveryService* pService = m_pList.GetNext( pos );

		int nLen = pService->m_sAddress.GetLength();
		if ( nLen > 45 )
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

CDiscoveryService* CDiscoveryServices::GetByAddress( IN_ADDR* pAddress, WORD nPort, CDiscoveryService::SubType nSubType )
{
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CDiscoveryService* pService = m_pList.GetNext( pos );

			if ( pService->m_nSubType == nSubType && pService->m_pAddress.S_un.S_addr == pAddress->S_un.S_addr &&
				pService->m_nPort == nPort )
				return pService;
	}

	return NULL;
}

void CDiscoveryServices::Clear()
{
	Stop();

	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		delete m_pList.GetNext( pos );
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

		for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
		{
			m_pList.GetNext( pos )->Serialize( ar, nVersion );
		}
	}
	else
	{
		Clear();

		ar >> nVersion;
		if ( nVersion < 6 ) return;

		for ( DWORD_PTR nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			auto_ptr< CDiscoveryService > pService( new CDiscoveryService() );
			try
			{
				pService->Serialize( ar, nVersion );
				m_pList.AddTail( pService.release() );
			}
			catch ( CException* pException )
			{
				pException->Delete();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices Check we have the minimum number of services
// Returns TRUE if there are enough services, or FALSE if there are not.

BOOL CDiscoveryServices::EnoughServices() const
{
	int nWebCacheCount = 0, nServerMetCount = 0;	// Types of services
	int nG1Count = 0, nG2Count = 0;					// Protocols

	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CDiscoveryService* pService = m_pList.GetNext( pos );
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
		else if ( pService->m_nType == CDiscoveryService::dsGnutella )
		{
			if ( pService->m_nSubType == CDiscoveryService::dsGnutellaUDPHC ) nG1Count++;
			if ( pService->m_nSubType == CDiscoveryService::dsGnutella2UDPKHL ) nG2Count++;
		}
	}

	return ( ( nWebCacheCount   >= 1 ) &&	// At least 1 webcache
			 ( nG2Count			>= 5 ) &&	// At least 5 G2 services
			 ( nG1Count			>= 3 ) &&	// At least 3 G1 services
			 ( nServerMetCount  >= 2 ) );	// At least 2 server.met
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices defaults

void CDiscoveryServices::AddDefaults()
{
	CFile pFile;
	CString strFile = Settings.General.Path + _T("\\Data\\DefaultServices.dat");

	if (  pFile.Open( strFile, CFile::modeRead ) )			// Load default list from file if possible
	{
		theApp.Message( MSG_NOTICE, _T("Loading default discovery service list") );

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
				case 'M': Add( strService, CDiscoveryService::dsWebCache );					// Multi-network service
					break;
				case 'D': Add( strService, CDiscoveryService::dsServerMet, PROTOCOL_ED2K );	// eDonkey service
					break;
				case 'U': Add( strService, CDiscoveryService::dsGnutella );					// Bootstrap and UDP Discovery Service
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
	if ( !EnoughServices() )
	{
		theApp.Message( MSG_ERROR, _T("Default discovery service load failed") );
		/*
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
		*/
	}
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices Merge URLs

void CDiscoveryServices::MergeURLs()
{
	CArray< CDiscoveryService* > G1URLs, G2URLs, MultiURLs, OtherURLs;
	theApp.Message( MSG_DEBUG, _T("CDiscoveryServices::MergeURLs(): Checking the discovery service WebCache URLs") );
	//Building the arrays...
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CDiscoveryService* pService = m_pList.GetNext( pos );
		if ( pService->m_nType == CDiscoveryService::dsWebCache )
		{
			if ( pService->m_bGnutella1 && pService->m_bGnutella2 )
			{
				MultiURLs.Add( pService );	//Multi-network array
			}
			else if ( pService->m_bGnutella1 )
			{
				G1URLs.Add( pService );	//Gnutella array
			}
			else if ( pService->m_bGnutella2 )
			{
				G2URLs.Add( pService );	//Gnutella2 array
			}
		}
		else
		{
			OtherURLs.Add( pService );	//Ignore anything other than a GWC and pass off into the 'otherURLs array'.
		}
	}
	if ( !MultiURLs.IsEmpty() )
	{
		for ( int index = 0; index < MultiURLs.GetCount(); index++ )
		{
			for ( int dup_index = 0; dup_index < MultiURLs.GetCount(); dup_index++ )
			{
				//Checking for identical duplicate.
				if ( MultiURLs.GetAt( index )->m_sAddress == MultiURLs.GetAt( dup_index )->m_sAddress && index != dup_index )
				{
					theApp.Message( MSG_NOTICE, _T("CDiscoveryServices::MergeURLs(): Removed %s from the multi list"), (LPCTSTR)MultiURLs.GetAt( dup_index )->m_sAddress );
					MultiURLs.RemoveAt( dup_index );
				}
			}
			if ( !G1URLs.IsEmpty() )
			{
				//Remove G1 service if it matches that of a multi.
				for ( int index2 = 0; index2 < G1URLs.GetCount(); index2++ )
				{
					if ( MultiURLs.GetAt( index )->m_sAddress == G1URLs.GetAt( index2 )->m_sAddress )
					{
						theApp.Message( MSG_NOTICE, _T("CDiscoveryServices::MergeURLs(): Removed %s from the Gnutella list"), (LPCTSTR)G1URLs.GetAt( index2 )->m_sAddress );
						G1URLs.RemoveAt( index2 );
					}
				}
			}
			if ( !G2URLs.IsEmpty() )
			{
				//Remove G2 service if it matches that of a multi.
				for ( int index3 = 0; index3 < G2URLs.GetCount(); index3++ )
				{
					if ( MultiURLs.GetAt( index )->m_sAddress == G2URLs.GetAt( index3 )->m_sAddress )
					{
						theApp.Message( MSG_NOTICE, _T("CDiscoveryServices::MergeURLs(): Removed %s from the Gnutella2 list"), (LPCTSTR)G2URLs.GetAt( index3 )->m_sAddress );
						G2URLs.RemoveAt( index3 );
					}
				}
			}
		}
	}
	if ( !G1URLs.IsEmpty() )
	{
		for (int index4 = 0; index4 < G1URLs.GetCount(); index4++)
		{
			for ( int dup_index2 = 0; dup_index2 < G1URLs.GetCount(); dup_index2++ )
			{
				//Checking for identical duplicate.
				if ( G1URLs.GetAt( index4 )->m_sAddress == G1URLs.GetAt( dup_index2 )->m_sAddress && index4 != dup_index2 )
				{
					theApp.Message( MSG_NOTICE, _T("CDiscoveryServices::MergeURLs(): Removed %s from the Gnutella list"), (LPCTSTR)G1URLs.GetAt( dup_index2 )->m_sAddress );
					G1URLs.RemoveAt( dup_index2 );
				}
			}
			if ( !G2URLs.IsEmpty() )
			{
				//If G1 and G2 of the same URL exist, drop one and upgrade the other to multi status.
				for ( int index5 = 0; index5 < G2URLs.GetCount(); index5++ )
				{
					if ( G1URLs.GetAt( index4 )->m_sAddress == G2URLs.GetAt( index5 )->m_sAddress )
					{
						CDiscoveryService* pService = G1URLs[index4];
						pService->m_bGnutella2 = true;
						G1URLs[index4] = pService;
						theApp.Message( MSG_NOTICE, _T("CDiscoveryServices::MergeURLs(): Merged %s into the multi list"), (LPCTSTR)G2URLs.GetAt( index5 )->m_sAddress );
						G2URLs.RemoveAt( index5 );
					}
				}
			}
		}
	}
	if ( !G2URLs.IsEmpty() )
	{
		for (int index6 = 0; index6 < G2URLs.GetCount(); index6++)
		{
			for ( int dup_index3 = 0; dup_index3 < G2URLs.GetCount(); dup_index3++ )
			{
				//Checking for identical duplicate
				if ( G2URLs.GetAt( index6 )->m_sAddress == G2URLs.GetAt( dup_index3 )->m_sAddress && index6 != dup_index3 )
				{
					theApp.Message( MSG_NOTICE, _T("CDiscoveryServices::MergeURLs(): Removed %s from the Gnutella2 list"), (LPCTSTR)G2URLs.GetAt( dup_index3 )->m_sAddress );
					G2URLs.RemoveAt( dup_index3 );
				}
			}
		}
	}
	if ( !G1URLs.IsEmpty() || !G2URLs.IsEmpty() || !MultiURLs.IsEmpty() || !OtherURLs.IsEmpty() )
	{
		//Updating the list...
		m_pList.RemoveAll();
		if ( !G1URLs.IsEmpty() )
		{
			for ( int g1_index = 0; g1_index < G1URLs.GetCount(); g1_index++ )
			{
				m_pList.AddTail( G1URLs.GetAt(  g1_index ) );
			}
		}
		if ( !G2URLs.IsEmpty() )
		{
			for ( int g2_index = 0; g2_index < G2URLs.GetCount(); g2_index++ )
			{
				m_pList.AddTail( G2URLs.GetAt(  g2_index ) );
			}
		}
		if ( !MultiURLs.IsEmpty() )
		{
			for ( int multi_index = 0; multi_index < MultiURLs.GetCount(); multi_index++ )
			{
				m_pList.AddTail( MultiURLs.GetAt(  multi_index ) );
			}
		}
		if ( !OtherURLs.IsEmpty() )
		{
			for ( int other_index = 0; other_index < OtherURLs.GetCount(); other_index++ )
			{
				m_pList.AddTail( OtherURLs.GetAt(  other_index ) );
			}
		}
		//The Discovery Service list is now rebuilt with the updated listing.
		theApp.Message( MSG_DEBUG, _T("CDiscoveryServices::MergeURLs(): Completed Successfully") );
	}
	else
	{
		//The Discovery Service list is empty.
		theApp.Message( MSG_ERROR, _T("CDiscoveryServices::MergeURLs(): detected an empty discovery service list") );
	}
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices update

BOOL CDiscoveryServices::Update()
{
	DWORD tNow = static_cast< DWORD >( time( NULL ) );

	// Don't update too frequently
	if ( tNow - m_tUpdated < Settings.Discovery.UpdatePeriod ) return FALSE;

	if ( m_hInternet )
		// Don't run concurrent request
		return FALSE;

	StopWebRequest();

	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;

	// Must be up for two hours
	if ( Network.GetStableTime() < 7200 ) return FALSE;

	// Determine which network/protocol to update
	PROTOCOLID nProtocol;
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

	//*** ToDo: Ultrapeer mode hasn't been updated or tested in a long time

	ASSERT ( ( nProtocol == PROTOCOL_G1 ) || ( nProtocol == PROTOCOL_G2 ) );

	// Must have at least 4 peers
	if ( Neighbours.GetCount( nProtocol, -1, ntNode ) < 4 ) return FALSE;

	// Select a random webcache of the appropriate sort
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

BOOL CDiscoveryServices::Execute(BOOL bDiscovery, PROTOCOLID nProtocol, USHORT nForceDiscovery)
{
	/*
		bDiscovery:
			TRUE	- Want Discovery(GEC, UHC, MET)
			FALSE	- Want Bootstrap connection.
		nProtocol:
			PROTOCOL_NULL	- Auto Detection
			PROTOCOL_G1		- Execute entry for G1
			PROTOCOL_G2		- Execute entry for G2
			PROTOCOL_ED2K	- Execute entry for ED2K
		nForceDiscovery:
			FALSE - Normal discovery. There is a time limit and a check if it is needed
			1 - Forced discovery. Partial time limit and withOUT check if it is needed ( Used inside CNeighboursWithConnect::Maintain() )
			2 - Unlimited discovery. No time limit but there is the check if it is needed ( Only from QuickStart Wizard )
	*/

	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;

	DWORD tNow = static_cast< DWORD >( time( NULL ) );	// The time (in seconds)

	if ( bDiscovery ) // If this is a user-initiated manual query, or AutoStart with Cache empty
	{
		if ( m_hInternet ) return FALSE;
		if ( m_tExecute != 0 && tNow - m_tExecute < 5 && nForceDiscovery < 2 ) return FALSE;
		if ( m_tQueried != 0 && tNow - m_tQueried < 60 && nForceDiscovery == 0 ) return FALSE;
		if ( nForceDiscovery > 0 && nProtocol == PROTOCOL_NULL ) return FALSE;

		m_tExecute = tNow;
		BOOL	bG1Required = Settings.Gnutella1.EnableToday && ( nProtocol == PROTOCOL_NULL || nProtocol == PROTOCOL_G1) && ( nForceDiscovery == 1 || HostCache.Gnutella1.CountHosts(TRUE) < 20 );
		BOOL	bG2Required = Settings.Gnutella2.EnableToday && ( nProtocol == PROTOCOL_NULL || nProtocol == PROTOCOL_G2) && ( nForceDiscovery == 1 || HostCache.Gnutella2.CountHosts(TRUE) < 25 );
		BOOL	bEdRequired = Settings.eDonkey.EnableToday && ( nProtocol == PROTOCOL_NULL || nProtocol == PROTOCOL_ED2K ) && Settings.eDonkey.MetAutoQuery && ( m_tMetQueried == 0 || tNow - m_tMetQueried >= 60 * 60 ) && ( nForceDiscovery == 1 || !HostCache.EnoughED2KServers() );

		// Broadcast discovery
		if ( bG2Required && Neighbours.NeedMoreHubs( PROTOCOL_G2 ) )
		{
			SOCKADDR_IN addr;
			CopyMemory( &addr, &(Network.m_pHost), sizeof( SOCKADDR_IN ) );
			addr.sin_family = AF_INET;
			addr.sin_addr.S_un.S_addr = INADDR_NONE;
			Datagrams.Send( &addr, CG2Packet::New( G2_PACKET_DISCOVERY ), TRUE, 0, FALSE );
		}

		if ( bEdRequired )
			m_tMetQueried = tNow;	// Execute this maximum one time each 60 min only when the number of eDonkey servers is too low (Very important).

		pLock.Unlock();

		if ( nProtocol == PROTOCOL_NULL )	// G1 + G2 + Ed hosts are wanted
			return  ( ! bG1Required || RequestRandomService( PROTOCOL_G1 ) ) &&
					( ! bG2Required || RequestRandomService( PROTOCOL_G2 ) ) &&
					( ! bEdRequired || RequestRandomService( PROTOCOL_ED2K ) );
		else if ( bG1Required )	// Only G1
			return RequestRandomService( PROTOCOL_G1 );
		else if ( bG2Required )	// Only G2
			return RequestRandomService( PROTOCOL_G2 );
		else if ( bEdRequired )	// Only Ed
			return RequestRandomService( PROTOCOL_ED2K );
		else
			return TRUE;	// No Discovery needed
	}
	else
	{
		theApp.Message( MSG_NOTICE, IDS_DISCOVERY_BOOTSTRAP );
		// TCP bootstraps
		if ( Settings.Gnutella1.EnableToday && Settings.Gnutella2.EnableToday && nProtocol == PROTOCOL_NULL )
			ExecuteBootstraps( Settings.Discovery.BootstrapCount, FALSE, PROTOCOL_NULL );
		else if ( Settings.Gnutella2.EnableToday && nProtocol == PROTOCOL_G2 )
			ExecuteBootstraps( Settings.Discovery.BootstrapCount, FALSE, PROTOCOL_G2 );
		else if ( Settings.Gnutella1.EnableToday && nProtocol == PROTOCOL_G1 )
			ExecuteBootstraps( Settings.Discovery.BootstrapCount, FALSE, PROTOCOL_G1 );
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices resolve N gnutella bootstraps

int CDiscoveryServices::ExecuteBootstraps(int nCount, BOOL bUDP, PROTOCOLID nProtocol)
{
	CArray< CDiscoveryService* > pRandom;
	int nSuccess;
	BOOL bGnutella1, bGnutella2;

	switch(nProtocol)
	{
		case PROTOCOL_NULL:
			bGnutella1 = TRUE;
			bGnutella2 = TRUE;
			break;
		case PROTOCOL_G1:
			bGnutella1 = TRUE;
			bGnutella2 = FALSE;
			break;
		case PROTOCOL_G2:
			bGnutella1 = FALSE;
			bGnutella2 = TRUE;
			break;
		default:
			bGnutella1 = FALSE;
			bGnutella2 = FALSE;
			break;
	}

	if ( nCount < 1 ) return 0;

	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CDiscoveryService* pService = m_pList.GetNext( pos );
		if ( pService->m_nType == CDiscoveryService::dsGnutella &&
			( ( bGnutella1 && bGnutella2 ) || ( bGnutella1 == pService->m_bGnutella1 && bGnutella2 == pService->m_bGnutella2 ) ) &&
			( ( ( pService->m_nSubType == CDiscoveryService::dsGnutellaUDPHC || pService->m_nSubType == CDiscoveryService::dsGnutella2UDPKHL ) && bUDP && ( time( NULL ) - pService->m_tAccessed >= 300 ) ) ||
			( ( pService->m_nSubType == CDiscoveryService::dsOldBootStrap || pService->m_nSubType == CDiscoveryService::dsGnutellaTCP || pService->m_nSubType == CDiscoveryService::dsGnutella2TCP ) && ! bUDP ) ) )
				pRandom.Add( pService );
	}

	for ( nSuccess = 0 ; nCount > 0 && pRandom.GetSize() > 0 ; )
	{
		INT_PTR nRandom( GetRandomNum< INT_PTR >( 0, pRandom.GetSize() - 1 ) );
		CDiscoveryService* pService( pRandom.GetAt( nRandom ) );
		pRandom.RemoveAt( nRandom );

		if ( pService->ResolveGnutella() )
		{
			++nSuccess;
			--nCount;
		}
	}

	return nSuccess;
}

void CDiscoveryServices::OnGnutellaAdded(IN_ADDR* /*pAddress*/, int /*nCount*/)
{
	// Find this host somehow and add to m_nHosts
}

void CDiscoveryServices::OnGnutellaFailed(IN_ADDR* /*pAddress*/)
{
	// Find this host and add to m_nFailures, and delete if excessive
}


//////////////////////////////////////////////////////////////////////
// CDiscoveryServices RequestRandomService

// Execute a random service (of any type) for any given protocol. Used to find more
// hosts (to connect to a network).

BOOL CDiscoveryServices::RequestRandomService(PROTOCOLID nProtocol)
{
	if ( CDiscoveryService* pService = GetRandomService( nProtocol ) )
	{
		if ( pService->m_nType == CDiscoveryService::dsGnutella )
		{
			return pService->ResolveGnutella();
		}
		else if ( pService->m_nType == CDiscoveryService::dsServerMet )
		{
			return RequestWebCache( pService, wcmServerMet, nProtocol );
		}
		else
		{
			return RequestWebCache( pService, wcmHosts, nProtocol );
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
	CArray< CDiscoveryService* > pServices;
	DWORD tNow = static_cast< DWORD >( time( NULL ) );

	// Loops through all services
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CDiscoveryService* pService = m_pList.GetNext( pos );

		switch ( nProtocol )
		{
		case PROTOCOL_G1:
			if ( Settings.Discovery.EnableG1GWC &&
				( pService->m_nType == CDiscoveryService::dsWebCache ) && ( pService->m_bGnutella1 ) &&
				( tNow - pService->m_tAccessed > pService->m_nAccessPeriod ) )
				pServices.Add( pService );
			else if ( ( pService->m_nType == CDiscoveryService::dsGnutella ) && ( pService->m_nSubType == CDiscoveryService::dsGnutellaUDPHC ) &&
				time( NULL ) - pService->m_tAccessed >= 300 )
				pServices.Add( pService );
			break;
		case PROTOCOL_G2:
			if ( ( pService->m_nType == CDiscoveryService::dsWebCache ) && ( pService->m_bGnutella2 ) &&
				( tNow - pService->m_tAccessed > pService->m_nAccessPeriod ) )
				pServices.Add( pService );
			else if ( ( pService->m_nType == CDiscoveryService::dsGnutella ) && ( pService->m_nSubType == CDiscoveryService::dsGnutella2UDPKHL ) &&
				time( NULL ) - pService->m_tAccessed >= 300 )
				pServices.Add( pService );
			break;
		case PROTOCOL_ED2K:
			if ( pService->m_nType == CDiscoveryService::dsServerMet  &&
				( tNow - pService->m_tAccessed > pService->m_nAccessPeriod ) )
				pServices.Add( pService );
			break;
		default:
			break;
		}
	}

	// Select a random service from the list of possible ones.
	if ( pServices.GetSize() > 0 )	// If the list of possible ones isn't empty
	{
		// return a random service
		return pServices.GetAt( GetRandomNum< INT_PTR >( 0, pServices.GetSize() - 1 ) );
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
	CArray< CDiscoveryService* > pWebCaches;
	DWORD tNow = static_cast< DWORD >( time( NULL ) );

	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CDiscoveryService* pService = m_pList.GetNext( pos );

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
		return pWebCaches.GetAt( GetRandomNum< INT_PTR >( 0, pWebCaches.GetSize() - 1 ) );
	}
	else
	{
		// return null to indicate none available
		return NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices webcache request control

BOOL CDiscoveryServices::RequestWebCache(CDiscoveryService* pService, Mode nMode, PROTOCOLID nProtocol)
{
	StopWebRequest();

	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;

	DWORD tNow = (DWORD)time( NULL );
	DWORD nHosts = 0;

	switch ( nProtocol )
	{
	case PROTOCOL_G1:
		nHosts = HostCache.Gnutella1.GetCount();
		break;
	case PROTOCOL_G2:
		nHosts = HostCache.Gnutella2.GetCount();
		break;
	case PROTOCOL_ED2K:
		nHosts = HostCache.eDonkey.GetCount();
		break;
	default:
		return FALSE;
	}

	if ( pService != NULL )
	{
		if ( time( NULL ) - pService->m_tAccessed < pService->m_nAccessPeriod &&
			 nHosts ) return FALSE;
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
			theApp.Message( MSG_NOTICE, IDS_DISCOVERY_QUERY, (LPCTSTR)m_pWebCache->m_sAddress );
			// Update the 'last queried' settings
			m_tQueried = tNow;
			m_nLastQueryProtocol = nProtocol;
		}
		break;

	case wcmUpdate:
		m_pSubmit	= GetRandomWebCache( nProtocol, TRUE, m_pWebCache, FALSE );
		if ( m_pWebCache != NULL )
		{
			theApp.Message( MSG_NOTICE, IDS_DISCOVERY_UPDATING, (LPCTSTR)m_pWebCache->m_sAddress );
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
			theApp.Message( MSG_NOTICE, IDS_DISCOVERY_SUBMIT, (LPCTSTR)m_pSubmit->m_sAddress );
			// Update the 'last updated' settings
			m_tUpdated = tNow;
			m_nLastUpdateProtocol = nProtocol;
		}
		break;

	case wcmServerMet:
		if ( nProtocol != PROTOCOL_ED2K )
		{
			ASSERT ( FALSE );
			return FALSE;
		}
		else if ( m_pWebCache != NULL )
		{
			theApp.Message( MSG_NOTICE, IDS_DISCOVERY_QUERY, (LPCTSTR)m_pWebCache->m_sAddress );
			// Update the 'last queried' settings
			m_tQueried = tNow;
			m_nLastQueryProtocol = nProtocol;
		}
		break;
	default:
		ASSERT ( FALSE );
		return FALSE;
	}

	if ( m_pWebCache == NULL ) return FALSE;

	return BeginThread( "Discovery" );
}

void CDiscoveryServices::StopWebRequest()
{
	if ( m_hInternet ) InternetCloseHandle( m_hInternet );
	m_hInternet = NULL;

	CloseThread();
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices thread run

void CDiscoveryServices::OnRun()
{
	if ( m_hInternet ) return;

	m_hInternet = InternetOpen( Settings.SmartAgent(),
		INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
	if ( ! m_hInternet ) return;

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
// CDiscoveryServices URL (specification 2) Pre-Add:

BOOL CDiscoveryServices::WebCacheStore(CString Line, LPCTSTR ID, int SubmitType, PROTOCOLID AddrProtocol)
{
	CString Marker = ID;
	int len_marker = Marker.GetLength();
	Marker += _T("||");
	// Make sure the address of the gwebcache isn't empty
	if ( _tcsnicmp( Line, (LPCTSTR)Marker, len_marker + 2 ) == 0 )
	{
		return false;
	}

	Add( Line.Mid( len_marker + 1 ).SpanExcluding( _T("|") ), SubmitType, AddrProtocol );
		
	return true;
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

	//strURL += _T("&support=1");	// GWC network and status - todo : Use this parameter's output to check GWCs for self-network support relay.
	//strURL += _T("&info=1");		// Maintainer Info - todo : Use this parameter's output to add info (about the maintainer, etc.) into new columns inside the Discovery window.

	if ( m_nLastQueryProtocol == PROTOCOL_G2 )
	{
		strURL += _T("&net=gnutella2&ping=1&pv=6");
		strURL += _T("&client=")_T(VENDOR_CODE);	//Version number is combined with client parameter for spec2
		strURL += theApp.m_sVersion;
	}
	else
	{
		//gnutella assumed
		strURL += _T("&net=gnutella");	//Some gnutella GWCs that serve specification 1 will not serve right on host/url requests combined with the ping request.
		strURL += _T("&client=")_T(VENDOR_CODE)_T("&version=");	//Version parameter is spec1
		strURL += theApp.m_sVersion;
	}

	pLock.Unlock();

	if ( ! SendWebCacheRequest( strURL, strOutput ) ) return FALSE;
	pLock.Lock();

	if ( ! Check( m_pWebCache ) ) return FALSE;

	BOOL bSuccess = FALSE;
	int nIP[4], nIPs = 0;
	
	////GWC-Output Network Validation Variables (Coolg1026's network validation update):
	CString lineline;
	int TokenNumber;
	BOOL IsNetwork, URLCheck;

	for ( strOutput += '\n' ; strOutput.GetLength() ; )
	{
		CString strLine	= strOutput.SpanExcluding( _T("\r\n") );
		strOutput		= strOutput.Mid( strLine.GetLength() + 1 );

		strLine.Trim();
		if ( strLine.IsEmpty() ) continue;

		theApp.Message( MSG_DEBUG, _T("GWebCache %s : %s"), (LPCTSTR)m_pWebCache->m_sAddress, (LPCTSTR)strLine );

		if ( _tcsistr( strLine, _T("http") ) == NULL )
		{
			//Many checks were added under this category to prevent exploitation or tripping when introduced as part of a URL.
			if ( ( _tcsistr( strOutput, _T("<") ) != NULL ) )
			{
				// Error- getting HMTL response.
				return FALSE;
			}
			else if ( _tcsistr( strLine, _T("early") ) != NULL )
			{
				// Did we flood?
				theApp.Message( MSG_ERROR, _T("GWebCache %s : Too many connection attempts"), (LPCTSTR)m_pWebCache->m_sAddress );
				return FALSE;
			}
			else if ( ( _tcsistr( strLine, _T("not supported") ) != NULL ) ||
			 ( _tcsistr( strLine, _T("not-supported") ) != NULL ) )
			{
				// ERROR CONDITION
				theApp.Message( MSG_ERROR, _T("GWebCache %s : Not supported"), (LPCTSTR)m_pWebCache->m_sAddress );
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
				tSeen = static_cast< DWORD >( time( NULL ) ) - nSeconds;
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
			URLCheck = false;
			if ( _tcsnicmp( strLine, _T("u|http://"), 9 ) == 0 )
			{
				URLCheck = WebCacheStore( strLine, _T("u"), CDiscoveryService::dsWebCache, m_nLastQueryProtocol );
			}
			else if ( _tcsnicmp( strLine, _T("u|uhc://"), 8 ) == 0 )
			{
				if ( m_nLastQueryProtocol != PROTOCOL_G2 )
				{
					URLCheck = WebCacheStore( strLine, _T("u"), CDiscoveryService::dsGnutella, m_nLastQueryProtocol );
				}
			}
			else if ( _tcsnicmp( strLine, _T("u|ukhl://"), 9 ) == 0 )
			{
				if ( m_nLastQueryProtocol == PROTOCOL_G2 )
				{
					URLCheck = WebCacheStore( strLine, _T("u"), CDiscoveryService::dsGnutella, m_nLastQueryProtocol );
				}
			}
			else if ( _tcsistr( strLine, _T("u||") ) == NULL )
			{
				theApp.Message( MSG_ERROR, _T("GWebCache %s : Unknown URL %s was given"), (LPCTSTR)m_pWebCache->m_sAddress, (LPCTSTR)strLine );
				return false;
			}
			if ( !URLCheck )
			{
				continue;
			}
			else
			{
				m_bFirstTime = FALSE;
				bSuccess = TRUE;
			}
		}
		else if ( _tcsnicmp( strLine, _T("UHC|"), 4 ) == 0 && m_nLastQueryProtocol != PROTOCOL_G2 )
		{
			//UDP Host Cache URL (For Gnutella1 ONLY):
			if ( !WebCacheStore( strLine, _T("UHC"), CDiscoveryService::dsGnutella, m_nLastQueryProtocol ) )
			{
				continue;
			}
			else
			{
				m_bFirstTime = FALSE;
				bSuccess = TRUE;
			}
		}
		else if ( _tcsnicmp( strLine, _T("UKHL|"), 5 ) == 0 && m_nLastQueryProtocol == PROTOCOL_G2 )
		{
			//UDP Known Hub List URL (For Gnutella2 ONLY):
			if ( !WebCacheStore( strLine, _T("UKHL"), CDiscoveryService::dsGnutella, m_nLastQueryProtocol ) )
			{
				continue;
			}
			else
			{
				m_bFirstTime = FALSE;
				bSuccess = TRUE;
			}
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
			else if ( ( _tcsnicmp( strLine, _T("i|force|remove"), 14 ) == 0 ) ||
					  ( _tcsnicmp( strLine, _T("i|update|warning|bad url"), 24 ) == 0 ) )
			{
				m_pWebCache->Remove();
				return FALSE;
			}
			else if ( _tcsnicmp( strLine, _T("i|pong|"), 7 ) == 0 )
			{
				// pong v2 (Skulls-type PONG network extension usage)
				//Usage here: Used to check if cache supports requested network.
				if ( m_nLastQueryProtocol != PROTOCOL_G2 )
				{
					//Mystery pong received - possibly a hosted static webpage.
					theApp.Message( MSG_ERROR, _T("GWebCache %s : PONG received when no ping was given"), (LPCTSTR)m_pWebCache->m_sAddress );
					return FALSE;
				}
				TokenNumber = 7;
				IsNetwork = FALSE;
				if ( strLine.Find( _T("|"), TokenNumber ) > 1 )
				{
					TokenNumber = strLine.Find( _T("|"), TokenNumber ) + 1;
					lineline = strLine.Tokenize( _T("-"), TokenNumber );
					while ( lineline != "" )
					{
						if ( ( _tcsnicmp( lineline, _T("gnutella2"), 9 ) == 0 && m_nLastQueryProtocol == PROTOCOL_G2 ) ||
						( _tcsnicmp( lineline, _T("gnutella"), 8 ) == 0 && m_nLastQueryProtocol != PROTOCOL_G2 && _tcsnicmp( lineline, _T("gnutella2"), 9 ) == -1 ) )
						{
							IsNetwork = TRUE;
							theApp.Message( MSG_DEBUG, _T("GWebCache %s : Correct Network : %s"), (LPCTSTR)m_pWebCache->m_sAddress, (LPCTSTR)strLine );
						}
						lineline = strLine.Tokenize( _T("-"), TokenNumber );
					};
					if ( !IsNetwork )
					{
						theApp.Message( MSG_ERROR, _T("GWebCache %s : Wrong Network : %s"), (LPCTSTR)m_pWebCache->m_sAddress, (LPCTSTR)strLine );
						return false;
					}
				}
			}
			else if ( _tcsnicmp( strLine, _T("i|networks|"), 11 ) == 0 )
			{
				//Beacon Cache type output
				//Used to check if cache supports requested network.
				TokenNumber = 11;
				IsNetwork = FALSE;
				lineline = strLine.Tokenize( _T("|"), TokenNumber );
				while ( lineline != "" )
				{
					if ( ( _tcsnicmp( lineline, _T("gnutella2"), 9 ) == 0 && m_nLastQueryProtocol == PROTOCOL_G2 ) ||
					( _tcsnicmp( lineline, _T("gnutella"), 8 ) == 0 && m_nLastQueryProtocol != PROTOCOL_G2 && _tcsnicmp( lineline, _T("gnutella2"), 9 ) == -1 ) )
					{
						IsNetwork = TRUE;
						theApp.Message( MSG_DEBUG, _T("GWebCache %s : Correct Network : %s"), (LPCTSTR)m_pWebCache->m_sAddress, (LPCTSTR)strLine );
					}
					lineline = strLine.Tokenize( _T("|"), TokenNumber );
				};
				if ( !IsNetwork )
				{
					theApp.Message( MSG_ERROR, _T("GWebCache %s : Wrong Network : %s"), (LPCTSTR)m_pWebCache->m_sAddress, (LPCTSTR)strLine );
					return false;
				}
			}
			else if ( _tcsnicmp( strLine, _T("i|nets|"), 7 ) == 0 )
			{
				//Skulls type output
				//Used to check if cache supports requested network.
				TokenNumber = 7;
				IsNetwork = FALSE;
				lineline = strLine.Tokenize( _T("-"), TokenNumber );
				while ( lineline != "" )
				{
					if ( ( _tcsnicmp( lineline, _T("gnutella2"), 9 ) == 0 && m_nLastQueryProtocol == PROTOCOL_G2 ) ||
					( _tcsnicmp( lineline, _T("gnutella"), 8 ) == 0 && m_nLastQueryProtocol != PROTOCOL_G2 && _tcsnicmp( lineline, _T("gnutella2"), 9 ) == -1 ) )
					{
						IsNetwork = TRUE;
						theApp.Message( MSG_DEBUG, _T("GWebCache %s : Correct Network : %s"), (LPCTSTR)m_pWebCache->m_sAddress, (LPCTSTR)strLine );
					}
					lineline = strLine.Tokenize( _T("-"), TokenNumber );
				};
				if ( !IsNetwork )
				{
					theApp.Message( MSG_ERROR, _T("GWebCache %s : Wrong Network : %s"), (LPCTSTR)m_pWebCache->m_sAddress, (LPCTSTR)strLine );
					return false;
				}
			}
			else if ( _tcsnicmp( strLine, _T("i|warning|"), 10 ) == 0 )
			{
				// Something wrong
				theApp.Message( MSG_INFO, _T("GWebCache %s : Warning : %s"), (LPCTSTR)m_pWebCache->m_sAddress, (LPCTSTR)strLine.Mid( 10 ) );
			}
			else if ( _tcsnicmp( strLine, _T("i|NO-URL-NO-HOSTS"), 17) == 0 )
			{
				// GWC is totally empty!
				theApp.Message( MSG_DEBUG, _T("GWebCache %s : Totally empty : %s"), (LPCTSTR)m_pWebCache->m_sAddress, (LPCTSTR)strLine );
			}
			else if ( _tcsnicmp( strLine, _T("i|NO-URL"), 8) == 0)
			{
				// GWC doesn't have URLs!
				theApp.Message( MSG_DEBUG, _T("GWebCache %s : No alternative URLs : %s"), (LPCTSTR)m_pWebCache->m_sAddress, (LPCTSTR)strLine );
			}
			else if ( _tcsnicmp( strLine, _T("i|NO-HOSTS"), 10) == 0)
			{
				// GWC doesn't have hosts!
				theApp.Message( MSG_DEBUG, _T("GWebCache %s : No hosts : %s"), (LPCTSTR)m_pWebCache->m_sAddress, (LPCTSTR)strLine );
			}
		}
		else if ( _tcsnicmp( strLine, _T("PONG"), 4 ) == 0 )
		{
			// pong v1
			if ( m_nLastQueryProtocol != PROTOCOL_G2 )
			{
				//Mystery pong received - possibly a hosted static webpage.
				theApp.Message( MSG_ERROR, _T("GWebCache %s : PONG received when no ping was given"), (LPCTSTR)m_pWebCache->m_sAddress );
				return FALSE;
			}
		}
		else if ( _tcsistr( strLine, _T("ERROR") ) != NULL )
		{
			if ( _tcsistr( strLine, _T("Something Failed") ) != NULL )
			{
				// Some Bazooka GWCs are bugged but ok.
				theApp.Message( MSG_INFO, _T("GWebCache %s : Warning : %s"), (LPCTSTR)m_pWebCache->m_sAddress, (LPCTSTR)strLine );
			}
			else
			{
				// Misc error. (Often CGI limits error)
				theApp.Message( MSG_ERROR, _T("GWebCache %s : Error : %s"), (LPCTSTR)m_pWebCache->m_sAddress, (LPCTSTR)strLine );
				return FALSE;
			}
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
			Add( strLine.SpanExcluding( _T(" ") ),
				CDiscoveryService::dsWebCache, PROTOCOL_G1 );
			m_pWebCache->m_bGnutella2 = FALSE;
			m_pWebCache->m_bGnutella1 = TRUE;
			m_bFirstTime = FALSE;
		}
	}

	if ( bSuccess )
	{
		m_pWebCache->OnSuccess();
		if ( HostCache.Gnutella2.GetNewest() != NULL && nIPs > 0 )
			m_tQueried = static_cast< DWORD >( time( NULL ) );
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

		strURL.Format( _T("%s?ip=%s:%hu&x.leaves=%i"),
			(LPCTSTR)m_pWebCache->m_sAddress,
			(LPCTSTR)CString( inet_ntoa( Network.m_pHost.sin_addr ) ),
			htons( Network.m_pHost.sin_port ),
			Neighbours.GetCount( PROTOCOL_ANY, -1, ntLeaf ) );		//ToDo: Check this
	}

	if ( m_pSubmit != NULL && Check( m_pSubmit, CDiscoveryService::dsWebCache ) )
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

		strURL += URLEncode( strSubmit );
	}

	strURL += _T("&update=1");	//Spec2 'update' parameter required...
	if ( m_nLastUpdateProtocol == PROTOCOL_G2 )
	{
		strURL += _T("&net=gnutella2");
		strURL += _T("&client=")_T(VENDOR_CODE);	//Version number is combined with client parameter for spec2
		strURL += theApp.m_sVersion;
	}
	else
	{
		//gnutella assumed
		strURL += _T("&net=gnutella");	//Some gnutella GWCs that serve specification 1 will not serve right on host/url requests combined with the ping request.
		strURL += _T("&client=")_T(VENDOR_CODE)_T("&version=");	//Version parameter is spec1
		strURL += theApp.m_sVersion;
	}

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

		if ( _tcsstr( strLine, _T("OK") ) != NULL )
		{
			if ( _tcsistr( strLine, _T("i|update|OK|IP already updated") ) != NULL || _tcsistr( strLine, _T("i|update|OK|Host already updated") ) != NULL )
			{
				theApp.Message( MSG_INFO, _T("GWebCache(v2) IP Updated Already: %s"), (LPCTSTR)strLine );
			}
			else if ( _tcsistr( strLine, _T("i|update|OK|URL already updated") ) != NULL || _tcsistr( strLine, _T("i|update|OK|Updated URL timestamp") ) != NULL )
			{
				theApp.Message( MSG_INFO, _T("GWebCache(v2) URL Updated Already: %s"), (LPCTSTR)strLine );
			}
			else if ( _tcsistr( strLine, _T("i|update|OK") ) != NULL )
			{
				if ( _tcsistr( strLine, _T("i|update|OK|URL|WARNING") ) != NULL )
				{
					theApp.Message( MSG_INFO, _T("GWebCache(v2) URL Update Warning: %s"), (LPCTSTR)strLine );
				}
				else if ( _tcsistr( strLine, _T("i|update|OK|HOST|WARNING") ) != NULL )
				{
					theApp.Message( MSG_INFO, _T("GWebCache(v2) IP Update Warning: %s"), (LPCTSTR)strLine );
				}
				else
				{
					theApp.Message( MSG_INFO, _T("GWebCache(v2) Update Passed: %s"), (LPCTSTR)strLine );
				}
			}
			else if ( _tcsstr( strLine, _T("OK") ) != NULL )
			{
				theApp.Message( MSG_INFO, _T("GWebCache(v1) Update Passed: %s"), (LPCTSTR)strLine );
				
			}
			m_pWebCache->m_tUpdated = (DWORD)time( NULL );
			m_pWebCache->m_nUpdates++;
			m_pWebCache->OnSuccess();
			return TRUE;
		}
		else if ( _tcsistr( strLine, _T("i|warning|client|early") ) != NULL || _tcsistr( strLine, _T("i|warning|You came back too early") ) != NULL || _tcsistr( strLine, _T("WARNING: You came back too early") ) != NULL )
		{
			theApp.Message( MSG_ERROR, _T("GWebCache(update) Too many connection attempts") );
			//Did we flood a gwc? It's always nice to know. :)
			return FALSE;
		}
		else if ( _tcsistr( strLine, _T("ERROR") ) != NULL )
		{
			if ( _tcsistr( strLine, _T("ERROR: Client returned too early") ) != NULL )
			{
				//GhostWhiteCrab type flood warning.
				theApp.Message( MSG_ERROR, _T("GWebCache(update) Too many connection attempts") );
				//Did we flood a gwc? It's always nice to know. :)
				return FALSE;
			}
			else
			{
			// Misc error. (Often CGI limits error)
			return FALSE;
			}
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

	theApp.Message( MSG_DEBUG, _T("DiscoveryService URL: %s"), (LPCTSTR)strURL );

	if ( m_hRequest ) InternetCloseHandle( m_hRequest );

	m_hRequest = InternetOpenUrl( m_hInternet, strURL, _T("Connection: close"), ~0u,
		INTERNET_FLAG_RELOAD|INTERNET_FLAG_DONT_CACHE|INTERNET_FLAG_NO_COOKIES, 0 );
	if ( m_hRequest == NULL ) return FALSE;

	DWORD nStatusCode = 0, nStatusLen = 32;
	TCHAR szStatusCode[32];

	if ( ! HttpQueryInfo( m_hRequest, HTTP_QUERY_STATUS_CODE, szStatusCode,
		&nStatusLen, NULL ) ) return FALSE;

	if ( _stscanf( szStatusCode, _T("%u"), &nStatusCode ) != 1 &&
		nStatusCode < 200 || nStatusCode > 299 ) return FALSE;

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
			DWORD nBuffer = min( nRemaining, 1024ul );
			InternetReadFile( m_hRequest, pBuffer, nBuffer, &nBuffer );
			pFile.Write( pBuffer, nBuffer );
			nRemaining -= nBuffer;
		}
	}

	pFile.Seek( 0, CFile::begin );

	pLock.Lock();

	if ( ! Check( m_pWebCache, CDiscoveryService::dsServerMet ) ) return FALSE;

	int nCount = HostCache.ImportMET( &pFile );

	if ( ! nCount ) return FALSE;

	HostCache.Save();
	m_pWebCache->OnHostAdd( nCount );
	m_pWebCache->OnSuccess();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices execute

// Note: This is used by wndDiscovery only
BOOL CDiscoveryServices::Execute(CDiscoveryService* pService, Mode nMode)
{
	if ( pService->m_nType == CDiscoveryService::dsGnutella )
	{
		return pService->ResolveGnutella();
	}
	else if ( pService->m_nType == CDiscoveryService::dsWebCache )
	{
		return RequestWebCache( pService,
			nMode, pService->m_bGnutella2 ? PROTOCOL_G2 : PROTOCOL_G1 );
	}
	else if ( pService->m_nType == CDiscoveryService::dsServerMet )
	{
		return RequestWebCache( pService,
			CDiscoveryServices::wcmServerMet, PROTOCOL_ED2K );
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////
// CDiscoveryService construction

CDiscoveryService::CDiscoveryService(Type nType, LPCTSTR pszAddress) :
	m_nType			( nType ),
	m_sAddress		( pszAddress ? pszAddress : _T("") ),
	m_bGnutella2	( FALSE ),
	m_bGnutella1	( FALSE ),
	m_tCreated		( (DWORD)time( NULL ) ),
	m_tAccessed		( 0 ),
	m_nAccesses		( 0 ),
	m_tUpdated		( 0 ),
	m_nUpdates		( 0 ),
	m_nHosts		( 0 ),
	m_nFailures		( 0 ),
	m_nAccessPeriod	( max( Settings.Discovery.UpdatePeriod, 1800ul ) ),
	m_nUpdatePeriod	( Settings.Discovery.DefaultUpdate ),
	m_nSubType		( dsOldBootStrap ),
	m_pAddress		(),
	m_nPort			( 0 )
{
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

void CDiscoveryService::Serialize(CArchive& ar, int /*nVersion*/)
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
		ar >> (int&)m_nType;
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

		// Check it has a valid protocol
		if ( _tcsnicmp( m_sAddress, _T("gnutella1:host:"),  15 ) == 0 )
		{
			m_bGnutella1 = TRUE;
			m_bGnutella2 = FALSE;
			m_nSubType = dsGnutellaTCP;
		}
		else if ( _tcsnicmp( m_sAddress, _T("gnutella2:host:"), 15 ) == 0 )
		{
			m_bGnutella1 = FALSE;
			m_bGnutella2 = TRUE;
			m_nSubType = dsGnutella2TCP;
		}
		else if ( _tcsnicmp( m_sAddress, _T("uhc:"), 4 ) == 0 )
		{
			m_bGnutella1 = TRUE;
			m_bGnutella2 = FALSE;
			m_nSubType = dsGnutellaUDPHC;
		}
		else if ( _tcsnicmp( m_sAddress, _T("ukhl:"), 5 ) == 0 )
		{
			m_bGnutella1 = FALSE;
			m_bGnutella2 = TRUE;
			m_nSubType = dsGnutella2UDPKHL;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryService resolve a gnutella node

BOOL CDiscoveryService::ResolveGnutella()
{
	if ( ! Network.Connect( FALSE ) ) return FALSE;

	if ( time( NULL ) - m_tAccessed < 300 ) return FALSE;

	CString strHost	= m_sAddress;
	int nSkip = 0;
	int nPort = 0;
	m_nHosts = 0;

	// Check it has a valid protocol
	if ( _tcsnicmp( strHost, _T("gnutella1:host:"),  15 ) == 0 )
	{
		m_nSubType = dsGnutellaTCP;
		nSkip = 15;
		m_bGnutella1 = TRUE;
		m_bGnutella2 = FALSE;
		nPort = GNUTELLA_DEFAULT_PORT;
	}
	else if ( _tcsnicmp( strHost, _T("gnutella2:host:"), 15 ) == 0 )
	{
		m_nSubType = dsGnutella2TCP;
		nSkip = 15;
		m_bGnutella1 = FALSE;
		m_bGnutella2 = TRUE;
		nPort = GNUTELLA_DEFAULT_PORT;
	}
	else if ( _tcsnicmp( strHost, _T("uhc:"), 4 ) == 0 )
	{
		m_nSubType = dsGnutellaUDPHC;
		nSkip = 4;
		m_bGnutella1 = TRUE;
		m_bGnutella2 = FALSE;
		nPort = 9999;
	}
	else if ( _tcsnicmp( strHost, _T("ukhl:"), 5 ) == 0 )
	{
		m_nSubType = dsGnutella2UDPKHL;
		nSkip = 5;
		m_bGnutella1 = FALSE;
		m_bGnutella2 = TRUE;
		nPort = GNUTELLA_DEFAULT_PORT;
	}

	if (m_nSubType == dsOldBootStrap)
	{
		int nPos = strHost.Find( ':' );
		if ( nPos >= 0 && _stscanf( strHost.Mid( nPos + 1 ), _T("%i"), &nPort ) == 1 )
			strHost = strHost.Left( nPos );

		if ( Network.AsyncResolve( strHost, (WORD)nPort, PROTOCOL_G1, 0 ) )
		{
			OnSuccess();
			return TRUE;
		}
	}
	else if (m_nSubType == dsGnutellaTCP)
	{
		strHost = strHost.Mid( nSkip );
		int nPos		= strHost.Find( ':');
		if ( nPos >= 0 && _stscanf( strHost.Mid( nPos + 1 ), _T("%i"), &nPort ) == 1 )
			strHost = strHost.Left( nPos );

		if ( Network.AsyncResolve( strHost, (WORD)nPort, PROTOCOL_G1, 1 ) )
		{
			OnAccess();
			return TRUE;
		}
	}
	else if (m_nSubType == dsGnutella2TCP)
	{
		strHost = strHost.Mid( nSkip );
		int nPos		= strHost.Find( ':');
		if ( nPos >= 0 && _stscanf( strHost.Mid( nPos + 1 ), _T("%i"), &nPort ) == 1 )
			strHost = strHost.Left( nPos );

		if ( Network.AsyncResolve( strHost, (WORD)nPort, PROTOCOL_G2, 1 ) )
		{
			OnAccess();
			return TRUE;
		}
	}
	else if (m_nSubType == dsGnutellaUDPHC)
	{
		strHost = strHost.Mid( nSkip );
		int nPos		= strHost.Find( ':');
		if ( nPos >= 0 && _stscanf( strHost.Mid( nPos + 1 ), _T("%i"), &nPort ) == 1 )
			strHost = strHost.Left( nPos );

		if ( Network.AsyncResolve( strHost, (WORD)nPort, PROTOCOL_G1, 3 ) )
		{
			OnAccess();
			return TRUE;
		}
	}
	else if (m_nSubType == dsGnutella2UDPKHL)
	{
		strHost = strHost.Mid( nSkip );
		int nPos		= strHost.Find( ':');
		if ( nPos >= 0 && _stscanf( strHost.Mid( nPos + 1 ), _T("%i"), &nPort ) == 1 )
			strHost = strHost.Left( nPos );

		if ( Network.AsyncResolve( strHost, (WORD)nPort, PROTOCOL_G2, 3 ) )
		{
			OnAccess();
			return TRUE;
		}
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
		theApp.Message( MSG_INFO, IDS_DISCOVERY_WEB_SUCCESS,
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
