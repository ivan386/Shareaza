//
// DiscoveryServices.cpp
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
	m_tUpdated		= 0;
	m_tExecute		= 0;
	m_bFirstTime	= TRUE;
	m_bForG2		= TRUE;
	m_nCacheType	= NULL;
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

int CDiscoveryServices::GetCount(int nType) const
{
	if ( nType == CDiscoveryService::dsNull ) return m_pList.GetCount();

	int nCount = 0;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		if ( GetNext( pos )->m_nType == nType ) nCount++;
	}

	return nCount;
}

int CDiscoveryServices::GetGnutella2Count() const
{
	int nCount = 0;
	CDiscoveryService *pDiscovery;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		pDiscovery = GetNext( pos );
		if ( ( pDiscovery->m_nType == CDiscoveryService::dsWebCache ) && ( pDiscovery->m_bGnutella2  ) ) 
			nCount++;
	}

	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices list modification

CDiscoveryService* CDiscoveryServices::Add(LPCTSTR pszAddress, int nType, int nCacheType)
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return NULL;
	
	CString strAddress( pszAddress );
	
	if ( strAddress.GetLength() < 8 ) return NULL;

	//if ( strAddress.GetAt( strAddress.GetLength() - 1 ) == '/' )
		//strAddress = strAddress.Left( strAddress.GetLength() - 1 );
	
	CDiscoveryService* pService = GetByAddress( strAddress );
	
	if ( pService == NULL )
	{
		if ( nType == CDiscoveryService::dsWebCache )
		{
			if ( _tcsnicmp( strAddress, _T("http://"), 7 ) == 0 ||
				 _tcsnicmp( strAddress, _T("https://"), 8 ) == 0 )
			{
				if ( _tcschr( (LPCTSTR)strAddress + 8, '/' ) == NULL ) return NULL;
				pService = new CDiscoveryService( CDiscoveryService::dsWebCache, strAddress );
			}
		}
		else if ( nType == CDiscoveryService::dsServerMet )
		{
			if ( _tcsnicmp( strAddress, _T("http://"), 7 ) == 0 ||
				 _tcsnicmp( strAddress, _T("https://"), 8 ) == 0 )
			{
				if ( _tcschr( (LPCTSTR)strAddress + 8, '/' ) == NULL ) return NULL;
				pService = new CDiscoveryService( CDiscoveryService::dsServerMet, strAddress );
			}
		}
		else if ( nType == CDiscoveryService::dsGnutella )
		{
			if ( _tcschr( pszAddress, '.' ) != NULL )
			{
				pService = new CDiscoveryService( CDiscoveryService::dsGnutella, strAddress );
			}
		}
		
		if ( pService == NULL ) return NULL;
	}
	
	switch( nCacheType )
	{
	case wcForG2:
		pService->m_bGnutella2 = TRUE;
		pService->m_bGnutella1 = FALSE;
		break;
	case wcForG1:
		pService->m_bGnutella2 = FALSE;
		pService->m_bGnutella1 = TRUE;
		break;
	case wcForBoth:
		pService->m_bGnutella2 = TRUE;
		pService->m_bGnutella1 = TRUE;
		break;
	}

	return Add( pService );
}

CDiscoveryService* CDiscoveryServices::Add(CDiscoveryService* pService)
{
	if ( ( pService->m_bGnutella2 == FALSE ) && ( pService->m_bGnutella1 == FALSE ) )
	{		
		pService->m_bGnutella2 = TRUE;
		pService->m_bGnutella1 = TRUE;
	}
	if ( pService && m_pList.Find( pService ) == NULL ) m_pList.AddTail( pService );
	return pService;
}

void CDiscoveryServices::Remove(CDiscoveryService* pService)
{
	if ( POSITION pos = m_pList.Find( pService ) ) m_pList.RemoveAt( pos );
	delete pService;
	
	int nCount[4];
	ZeroMemory( nCount, sizeof(int) * 4 );
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDiscoveryService* pService = GetNext( pos );
		nCount[ pService->m_nType ] ++;
	}
	
	if ( ( nCount[ CDiscoveryService::dsWebCache ] < 4 ) ||
		 ( nCount[ CDiscoveryService::dsServerMet ] < 1 ) ||
		 ( GetGnutella2Count() < 3 ) )
	{
		AddDefaults();
	}
		
}

CDiscoveryService* CDiscoveryServices::GetByAddress(LPCTSTR pszAddress) const
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDiscoveryService* pService = GetNext( pos );
		if ( pService->m_sAddress.CompareNoCase( pszAddress ) == 0 )
			return pService;
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
	
	CString strFile = Settings.General.Path + _T("\\Data\\Discovery.dat");
	
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
	
	if ( ( GetCount() < 4 ) || ( GetGnutella2Count() < 3 ) )
	{
		AddDefaults();
		Save();
	}
	
	return TRUE;
}

BOOL CDiscoveryServices::Save()
{
	CSingleLock pLock( &Network.m_pSection, TRUE );
	CFile pFile;

	CString strFile = Settings.General.Path + _T("\\Data\\Discovery.dat");
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
		if ( nVersion != 6 ) return;
		
		for ( int nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			CDiscoveryService* pService = new CDiscoveryService();
			pService->Serialize( ar, nVersion );
			m_pList.AddTail( pService );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices defaults

void CDiscoveryServices::AddDefaults()
{
	CFile pFile;
	CString strFile = Settings.General.Path + _T("\\Data\\gwebcache.txt");

	if (  pFile.Open( strFile, CFile::modeRead ) ) //Load default list from file if possible
	{
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
				if ( strLine.GetLength() < 7 ) continue; //Blank comment line

				cType = strLine.GetAt( 0 );
				strService = strLine.Right( strLine.GetLength() - 2 );

				switch( cType )
				{
				case '1': Add( strService, CDiscoveryService::dsWebCache, wcForG1 );	//G1 service
					break;
				case '2': Add( strService, CDiscoveryService::dsWebCache, wcForG2 );	//G2 service
					break;
				case 'M': Add( strService, CDiscoveryService::dsWebCache, wcForBoth );	//Multinetwork service
					break;
				case 'D': Add( strService, CDiscoveryService::dsServerMet, wcNull );	//eDonkey service
					break;
				case '#': //Comment line
					break;
				}
			}
		}
		catch ( CException* pException )
		{
			if (pFile.m_hFile != CFile::hFileNull) pFile.Close(); //Check if file is still open, if yes close
			pException->Delete();
		}
	}
	else                //If file can't be used, drop back to the the in-built list
	{
		CString strServices;
		strServices.LoadString( IDS_DISCOVERY_DEFAULTS );
	
		for ( strServices += '\n' ; strServices.GetLength() ; )
		{
			CString strService = strServices.SpanExcluding( _T("\r\n") );
			strServices = strServices.Mid( strService.GetLength() + 1 );
		
			if ( strService.GetLength() > 0 )
			{
				Add( strService,
				( _tcsistr( strService, _T("server.met") ) == NULL ?
				CDiscoveryService::dsWebCache : CDiscoveryService::dsServerMet ),
				wcForBoth ); // ( _tcsistr( strService, _T("GWC2") ) != NULL || _tcsistr( strService, _T("g2cache") ) != NULL ) );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices update

BOOL CDiscoveryServices::Update()
{
	DWORD tNow = (DWORD)time( NULL );
	
	if ( tNow - m_tUpdated < Settings.Discovery.UpdatePeriod ) return FALSE;
	
	if ( m_hInternet ) return FALSE;
	StopWebRequest();
	
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;
	
	if ( Network.GetStableTime() < 7200 ) return FALSE;				// Up for two hours
//ToDo: ***** Need to add G1 stuff
	if ( ! Neighbours.IsG2Hub() ) return FALSE;						// Must be a hub now 
//
	if ( Neighbours.GetCount( -1, -1, ntNode ) < 4 ) return FALSE;	// Must have 4 peers
	
	CDiscoveryService* pService = GetRandomWebCache( TRUE, NULL, TRUE );
	
	if ( pService == NULL ) return FALSE;
	
	m_tUpdated = tNow;
	
	theApp.Message( MSG_DEFAULT, IDS_DISCOVERY_UPDATING, (LPCTSTR)pService->m_sAddress );

	return RequestWebCache( pService, wcmUpdate );
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices execute

BOOL CDiscoveryServices::Execute(BOOL bSecondary)
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;
	
	if ( bSecondary )
	{
		if ( m_hInternet ) return FALSE;
		if ( time( NULL ) - m_tQueried < 60 ) return FALSE;
		if ( time( NULL ) - m_tExecute < 10 ) return FALSE;
	}
	else
	{
		theApp.Message( MSG_SYSTEM, IDS_DISCOVERY_BOOTSTRAP );
	}
	
	m_tExecute = time( NULL );
	int nCount = 0;
	
	if ( ! bSecondary ) nCount += ExecuteBootstraps( Settings.Discovery.BootstrapCount );
	nCount += ExecuteWebCache();
	
	if ( ! bSecondary ) m_tUpdated = 0;
	
	return nCount > 0;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices execute eDonkey2000

BOOL CDiscoveryServices::ExecuteDonkey()
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CDiscoveryService* pService = GetNext( pos );
		
		if ( pService->m_nType == CDiscoveryService::dsServerMet )
		{
			if ( RequestWebCache( pService, wcmServerMet ) ) return TRUE;
		}
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
// CDiscoveryServices execute a random webcache service

int CDiscoveryServices::ExecuteWebCache()
{
	m_bForG2 = Neighbours.NeedMoreHubs( TS_TRUE );

	CDiscoveryService* pService = GetRandomWebCache( FALSE );
	if ( pService == NULL ) 
	{
		/*
		//This would be a way to ensure the user always has G2 services available-
		//However re-adding services in a section that's contantly run is risky.
		//See if the other changes fix the problem first.
		if ( GetGnutella2Count() < 3 )
		{
			theApp.Message( MSG_DEBUG, _T("Re-setting Discovery services") );
			AddDefaults();
		}
		*/
		
		return 0;
	}

	return RequestWebCache( pService, wcmHosts ) ? 1 : 0;
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices select a random webcache

CDiscoveryService* CDiscoveryServices::GetRandomWebCache(BOOL bWorkingOnly, CDiscoveryService* pExclude, BOOL bForUpdate)
{
	CPtrArray pVersion2, pVersion1;
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
						if ( pService->m_bGnutella1 && pService->m_bGnutella2 )
						{
							pVersion1.Add( pService );
							pVersion2.Add( pService );
						}
						else if ( pService->m_bGnutella2 )
						{
							pVersion2.Add( pService );
						}
						else
						{
							pVersion1.Add( pService );
						}
					}
				}
			}
		}
	}
	
	srand( GetTickCount() );
	
	if ( m_bForG2 && pVersion2.GetSize() > 0 )
	{
		return (CDiscoveryService*)pVersion2.GetAt( rand() % pVersion2.GetSize() );
	}
	else if ( ! m_bForG2 && pVersion1.GetSize() > 0 )
	{
		return (CDiscoveryService*)pVersion1.GetAt( rand() % pVersion1.GetSize() );
	}
	else
	{
		return NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryServices webcache request control

BOOL CDiscoveryServices::RequestWebCache(CDiscoveryService* pService, int nMode)
{
	StopWebRequest();
	
	if ( pService != NULL )
	{
		if ( time( NULL ) - pService->m_tAccessed < pService->m_nAccessPeriod ) return FALSE;
	}
	
	m_pWebCache	= pService;
	m_nWebCache	= nMode;
	m_hRequest	= NULL;
	
	if ( nMode == wcmSubmit )
	{
		m_pSubmit	= m_pWebCache;
		m_pWebCache	= GetRandomWebCache( FALSE, m_pSubmit, TRUE );
	}
	else if ( nMode == wcmUpdate )
	{
		m_pSubmit	= GetRandomWebCache( TRUE, m_pWebCache );
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
	
	for ( int nAttempt = 10 ; nAttempt > 0 ; nAttempt-- )
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
		if ( ! bSuccess ) m_tUpdated = 0;
	}
	else if ( m_nWebCache == wcmSubmit )
	{
		bSuccess = RunWebCacheUpdate();
	}
	
	if ( m_hInternet && ! bSuccess )
	{
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
	
	if ( m_bForG2 ) strURL += _T("&net=gnutella2");
	
	pLock.Unlock();
	if ( ! SendWebCacheRequest( strURL, strOutput ) ) return FALSE;
	pLock.Lock();
	
	if ( ! Check( m_pWebCache ) ) return FALSE;
	
	BOOL bSuccess = FALSE;
	int nIP[4], nIPs = 0;
	
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
				_stscanf( strLine.Mid( nBreak + 1 ), _T("%lu"), &nSeconds );
				nSeconds = max( 0, min( 18000, nSeconds ) );
				strLine = strLine.Left( nBreak );
				tSeen = time( NULL ) - nSeconds;
			}
			
			if ( m_bForG2 )
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
			Add( strLine.Mid( 2 ).SpanExcluding( _T("|") ), CDiscoveryService::dsWebCache, wcForBoth );

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
				_stscanf( (LPCTSTR)strLine + 16, _T("%lu"), &m_pWebCache->m_nAccessPeriod );
			}
			else if ( strLine == _T("i|force|remove") || _tcsnicmp( strLine, _T("i|update|warning|bad url"), 24 ) == 0 )
			{
				m_pWebCache->Remove();
				return FALSE;
			}
			
			if ( _tcsistr( strLine, _T("ERROR") ) != NULL )
			{
				// ERROR CONDITION
				if ( m_bForG2 )
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
		else if ( _tcsistr( strLine, _T("ERROR") ) != NULL )
		{
			// ERROR CONDITION
			if ( m_bForG2 )
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
			Add( strLine.SpanExcluding( _T(" ") ), CDiscoveryService::dsWebCache, wcForG1 );
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

		strURL.Format( _T("%s?update=1&ip=%s:%lu&x.leaves=%lu"),
			(LPCTSTR)m_pWebCache->m_sAddress,
			(LPCTSTR)CString( inet_ntoa( Network.m_pHost.sin_addr ) ),
			htons( Network.m_pHost.sin_port ),
			Neighbours.GetCount( -1, -1, ntLeaf ) );
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
	
	if ( Settings.Gnutella2.EnableToday ) strURL += _T("&net=gnutella2");
	
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
			_stscanf( (LPCTSTR)strLine + 16, _T("%lu"), &m_pWebCache->m_nAccessPeriod );
		}
		else if ( _tcsnicmp( strLine, _T("i|update|period|"), 16 ) == 0 )
		{
			_stscanf( (LPCTSTR)strLine + 16, _T("%lu"), &m_pWebCache->m_nUpdatePeriod );
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
	
	_stscanf( szStatusCode, _T("%lu"), &nStatusCode );
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
			DWORD nBuffer = min( nRemaining, 1024 );
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
	m_nAccessPeriod	= max( Settings.Discovery.UpdatePeriod, 1800 );
	m_nUpdatePeriod	= Settings.Discovery.DefaultUpdate;
	
	if ( pszAddress != NULL ) m_sAddress = pszAddress;
}

CDiscoveryService::~CDiscoveryService()
{
}

//////////////////////////////////////////////////////////////////////
// CDiscoveryService remove

void CDiscoveryService::Remove()
{
	DiscoveryServices.Remove( this );
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

BOOL CDiscoveryService::Execute(int nMode)
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;
	
	if ( nMode != CDiscoveryServices::wcmSubmit )
	{
		theApp.Message( MSG_SYSTEM, IDS_DISCOVERY_QUERY, (LPCTSTR)m_sAddress );
	}
	else
	{
		theApp.Message( MSG_SYSTEM, IDS_DISCOVERY_SUBMIT, (LPCTSTR)m_sAddress );
	}
	
	if ( m_nType == dsGnutella )
	{
		return ResolveGnutella();
	}
	else if ( m_nType == dsWebCache )
	{
		return DiscoveryServices.RequestWebCache( this, nMode );
	}
	else if ( m_nType == dsServerMet )
	{
		return DiscoveryServices.RequestWebCache( this, CDiscoveryServices::wcmServerMet );
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

