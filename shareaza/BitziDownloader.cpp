//
// BitziDownloader.cpp
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
#include "Library.h"
#include "SharedFile.h"
#include "SHA.h"
#include "XML.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "BitziDownloader.h"
#include "DlgBitziDownload.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CBitziDownloader construction

CBitziDownloader::CBitziDownloader()
{
	m_pDlg			= NULL;
	m_hThread		= NULL;
	m_hInternet		= NULL;
	m_hSession		= NULL;
	m_hRequest		= NULL;
	m_bFinished		= FALSE;
	m_nDelay		= 0;
	m_nFailures		= 0;
	m_pXML			= NULL;
}

CBitziDownloader::~CBitziDownloader()
{
	Stop();

	if ( m_pXML ) delete m_pXML;
	m_pXML = NULL;
}

//////////////////////////////////////////////////////////////////////
// CBitziDownloader file list

void CBitziDownloader::AddFile(DWORD nIndex)
{
	CSingleLock pLock( &m_pSection, TRUE );
	m_pFiles.AddTail( (LPVOID)nIndex );
}

int CBitziDownloader::GetFileCount()
{
	CSingleLock pLock( &m_pSection, TRUE );
	return m_pFiles.GetCount();
}

//////////////////////////////////////////////////////////////////////
// CBitziDownloader start

BOOL CBitziDownloader::Start(CBitziDownloadDlg* pDlg)
{
	if ( m_hInternet != NULL ) return FALSE;

	CString strAgent = Settings.SmartAgent( Settings.Library.BitziAgent );

	m_hInternet = InternetOpen( strAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
	if ( ! m_hInternet ) return FALSE;

	m_hSession	= NULL;
	m_hRequest	= NULL;
	m_pDlg		= pDlg;
	m_bFinished	= FALSE;
	m_nDelay	= 0;
	m_nFailures	= 0;

	CWinThread* pThread = AfxBeginThread( ThreadStart, this, THREAD_PRIORITY_NORMAL );
	m_hThread = pThread->m_hThread;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBitziDownloader stop

void CBitziDownloader::Stop()
{
	if ( m_hSession != NULL ) InternetCloseHandle( m_hSession );
	m_hSession = NULL;

	if ( m_hInternet ) InternetCloseHandle( m_hInternet );
	m_hInternet = NULL;
	
	if ( m_hThread == NULL ) return;

	for ( int nAttempt = 5 ; nAttempt > 0 ; nAttempt-- )
	{
		DWORD nCode;

		if ( ! GetExitCodeThread( m_hThread, &nCode ) ) break;
		if ( nCode != STILL_ACTIVE ) break;
		Sleep( 100 );
	}

	if ( nAttempt == 0 )
	{
		TerminateThread( m_hThread, 0 );
		theApp.Message( MSG_DEBUG, _T("WARNING: Terminating CBitziDownloader thread.") );
		Sleep( 100 );
	}

	m_hThread	= NULL;
	m_pDlg		= NULL;
}

//////////////////////////////////////////////////////////////////////
// CBitziDownloader working flag

BOOL CBitziDownloader::IsWorking()
{
	return ( m_hThread != NULL ) && ! m_bFinished;
}

//////////////////////////////////////////////////////////////////////
// CBitziDownloader thread bootstrap

UINT CBitziDownloader::ThreadStart(LPVOID pParam)
{
	CBitziDownloader* pClass = (CBitziDownloader*)pParam;
	pClass->OnRun();
	return 0;
}

//////////////////////////////////////////////////////////////////////
// CBitziDownloader thread run

void CBitziDownloader::OnRun()
{
	while ( m_hInternet != NULL )
	{
		m_pSection.Lock();

		if ( m_pFiles.IsEmpty() )
		{
			m_pSection.Unlock();
			break;
		}

		m_nFileIndex = (DWORD)m_pFiles.RemoveHead();

		m_pSection.Unlock();

		m_pDlg->OnNextFile( m_nFileIndex );

		if ( BuildRequest() )
		{
			m_pDlg->OnRequesting( m_nFileIndex, m_sFileName );

			if ( ExecuteRequest() )
			{
				if ( DecodeResponse() )
				{
					m_pDlg->OnSuccess( m_nFileIndex );
				}
				else
				{
					if ( m_hInternet == NULL ) break;
					m_pDlg->OnFailure( m_nFileIndex, _T("Not Found") );
				}
			}
			else if ( ++m_nFailures >= 3 )
			{
				if ( m_hInternet == NULL ) break;
				m_pDlg->OnFailure( m_nFileIndex, _T("Aborting") );
				break;
			}
			else
			{
				if ( m_hInternet == NULL ) break;

				if ( m_hRequest != NULL ) InternetCloseHandle( m_hRequest );
				m_hRequest = NULL;
								
				m_pDlg->OnFailure( m_nFileIndex, _T("Failed") );

				Sleep( 1000 );
			}
		}

		m_pDlg->OnFinishedFile( m_nFileIndex );

		if ( m_hRequest != NULL ) InternetCloseHandle( m_hRequest );
		m_hRequest = NULL;

		m_sResponse.Empty();

		if ( m_pXML ) delete m_pXML;
		m_pXML = NULL;

		Sleep( min( m_nDelay, 500 ) );
	}

	if ( m_hSession != NULL && ! m_bFinished ) InternetCloseHandle( m_hSession );
	m_hSession = NULL;

	m_bFinished = TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBitziDownloader request builder

BOOL CBitziDownloader::BuildRequest()
{
	CLibraryFile* pFile = Library.LookupFile( m_nFileIndex, TRUE );

	if ( ! pFile ) return FALSE;

	m_sFileName = pFile->m_sName;
	m_sFileHash.Empty();

	if ( pFile->m_bSHA1 ) m_sFileHash = CSHA::HashToString( &pFile->m_pSHA1 );

	Library.Unlock();

	if ( m_sFileHash.IsEmpty() ) return FALSE;

	m_sURL = Settings.Library.BitziXML;
	Replace( m_sURL, _T("(SHA1)"), m_sFileHash );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBitziDownloader request executer

BOOL CBitziDownloader::ExecuteRequest()
{
	DWORD nTime = GetTickCount();

	int nPos, nPort = INTERNET_DEFAULT_HTTP_PORT;
	CString strHost, strPath;
	
	strHost = m_sURL;
	nPos = strHost.Find( _T("http://") );
	if ( nPos != 0 ) return FALSE;
	strHost = strHost.Mid( 7 );
	nPos = strHost.Find( '/' );
	if ( nPos < 0 ) return FALSE;
	strPath = strHost.Mid( nPos );
	strHost = strHost.Left( nPos );
	nPos = strHost.Find( ':' );
	
	if ( nPos > 0 )
	{
		_stscanf( strHost.Mid( nPos + 1 ), _T("%i"), &nPort );
		strHost = strHost.Left( nPos );
	}

	if ( m_hSession == NULL )
	{
		m_hSession = InternetConnect( m_hInternet, strHost, nPort,
			NULL, NULL, INTERNET_SERVICE_HTTP , 0, 0 );
		if ( m_hSession == NULL ) return FALSE;
	}

	m_hRequest = HttpOpenRequest( m_hSession, _T("GET"), strPath, NULL, NULL, NULL,
		INTERNET_FLAG_KEEP_CONNECTION|INTERNET_FLAG_NO_COOKIES, 0 );

	if ( m_hRequest == NULL )
	{
		if ( m_hSession != NULL ) InternetCloseHandle( m_hSession );

		m_hSession = InternetConnect( m_hInternet, strHost, nPort,
			NULL, NULL, INTERNET_SERVICE_HTTP , 0, 0 );

		if ( m_hSession == NULL ) return FALSE;

		m_hRequest = HttpOpenRequest( m_hSession, _T("GET"), strPath, NULL, NULL, NULL,
			INTERNET_FLAG_KEEP_CONNECTION|INTERNET_FLAG_NO_COOKIES, 0 );

		if ( m_hRequest == NULL ) return FALSE;
	}

	if ( ! HttpSendRequest( m_hRequest, NULL, 0, NULL, 0 ) ) return FALSE;

	TCHAR szStatusCode[32];
	DWORD nStatusCode = 0, nStatusLen = 32;
	
	if ( ! HttpQueryInfo( m_hRequest, HTTP_QUERY_STATUS_CODE, szStatusCode,
		&nStatusLen, NULL ) ) return FALSE;

	_stscanf( szStatusCode, _T("%lu"), &nStatusCode );
	if ( nStatusCode < 200 || nStatusCode > 299 ) return FALSE;

	LPBYTE pResponse = NULL;
	DWORD nRemaining, nResponse = 0;
	
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

	m_sResponse.Empty();

	LPTSTR pszResponse = m_sResponse.GetBuffer( nResponse );
	for ( nStatusCode = 0 ; nStatusCode < nResponse ; nStatusCode++ )
		pszResponse[ nStatusCode ] = (TCHAR)pResponse[ nStatusCode ];
	m_sResponse.ReleaseBuffer( nResponse );

	free( pResponse );

	if ( m_hRequest != NULL ) InternetCloseHandle( m_hRequest );
	m_hRequest = NULL;

	m_nDelay = ( GetTickCount() - nTime ) * 2;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBitziDownloader request decoder

BOOL CBitziDownloader::DecodeResponse()
{
	if ( m_pXML ) delete m_pXML;

	m_pXML = CXMLElement::FromString( m_sResponse, TRUE );
	if ( m_pXML == NULL ) return FALSE;

	for ( POSITION pos = SchemaCache.GetIterator() ; pos ; )
	{
		CSchema* pSchema = SchemaCache.GetNext( pos );

		if ( pSchema->m_sBitziTest.GetLength() && LookupValue( pSchema->m_sBitziTest ).GetLength() )
		{
			CXMLElement* pMetadata = ImportData( pSchema );
			
			if ( pMetadata == NULL ) return FALSE;
			
			return SubmitMetaData( pMetadata );
		}
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CBitziDownloader value lookup

CString CBitziDownloader::LookupValue(LPCTSTR pszPath)
{
	CString strName, strPath( pszPath );
	CXMLElement* pXML = m_pXML;
	BOOL bFirst = TRUE;
	
	while ( strPath.GetLength() )
	{
		strName = strPath.SpanExcluding( _T("/") );
		strPath = strPath.Mid( strName.GetLength() );

		if ( strPath.IsEmpty() )
		{
			return pXML->GetAttributeValue( strName, NULL );
		}

		if ( bFirst )
		{
			bFirst = FALSE;
			if ( strName.CompareNoCase( pXML->GetName() ) ) pXML = NULL;
		}
		else
		{
			pXML = pXML->GetElementByName( strName );
		}

		if ( ! pXML )
		{
			strName.Empty();
			return strName;
		}

		strPath = strPath.Mid( 1 );
	}
	
	strName.Empty();
	if ( pXML ) strName = pXML->GetValue();

	return strName;
}

//////////////////////////////////////////////////////////////////////
// CBitziDownloader import data

CXMLElement* CBitziDownloader::ImportData(CSchema* pSchema)
{
	CXMLElement* pRoot	= pSchema->Instantiate( TRUE );
	CXMLElement* pXML	= pRoot->AddElement( pSchema->m_sSingular );
	int nCount = 0;
	
	for ( POSITION pos = pSchema->m_pBitziMap.GetHeadPosition() ; pos ; )
	{
		CSchemaBitzi* pMap = (CSchemaBitzi*)pSchema->m_pBitziMap.GetNext( pos );
		
		CString strValue = LookupValue( pMap->m_sFrom );
		if ( strValue.IsEmpty() ) continue;

		if ( pMap->m_nFactor )
		{
			double nValue;

			if ( _stscanf( strValue, _T("%lf"), &nValue ) == 1 )
			{
				nValue *= pMap->m_nFactor;

				if ( nValue == (double)( (int)nValue ) )
				{
					strValue.Format( _T("%li"), (int)nValue );
				}
				else
				{
					strValue.Format( _T("%lf"), nValue );
				}
			}
		}

		pXML->AddAttribute( pMap->m_sTo, strValue );
		nCount++;
	}

	if ( nCount ) return pRoot;
	delete pRoot;

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CBitziDownloader submit metadata

BOOL CBitziDownloader::SubmitMetaData(CXMLElement* pXML)
{
	CLibraryFile* pFile = Library.LookupFile( m_nFileIndex, TRUE );
	
	if ( pFile == NULL )
	{
		delete pXML;
		return FALSE;
	}

	if ( pFile->m_pMetadata != NULL ) MergeMetaData( pXML, pFile->m_pMetadata );

	BOOL bSuccess = pFile->SetMetadata( pXML );

	delete pXML;

	Library.Unlock();

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CBitziDownloader metadata merge

BOOL CBitziDownloader::MergeMetaData(CXMLElement* pOutput, CXMLElement* pInput)
{
	if ( ! pOutput || ! pInput ) return FALSE;

	pOutput	= pOutput->GetFirstElement();

	if ( ! pOutput || pOutput->GetName() != pInput->GetName() ) return FALSE;

	for ( POSITION pos = pInput->GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement	= pInput->GetNextElement( pos );
		CXMLElement* pTarget	= pOutput->GetElementByName( pElement->GetName() );
		
		if ( pTarget == NULL ) pOutput->AddElement( pElement->Clone() );
	}

	for ( pos = pInput->GetAttributeIterator() ; pos ; )
	{
		CXMLAttribute* pAttribute	= pInput->GetNextAttribute( pos );
		CXMLAttribute* pTarget		= pOutput->GetAttribute( pAttribute->GetName() );
		
		if ( pTarget == NULL ) pOutput->AddAttribute( pAttribute->Clone() );
	}

	return TRUE;
}
