//
// HttpRequest.cpp
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
#include "HttpRequest.h"
#include "Network.h"
#include "Buffer.h"
#include "SourceURL.h"


//////////////////////////////////////////////////////////////////////
// CHttpRequest construction

CHttpRequest::CHttpRequest()
{
	m_sUserAgent	= Settings.SmartAgent( Settings.General.UserAgent );
	m_hThread		= NULL;
	m_hInternet		= NULL;
	m_nLimit		= 0;
	m_nStatusCode	= 0;
	m_pPost			= NULL;
	m_pResponse		= NULL;
	m_hNotifyWnd	= NULL;
}

CHttpRequest::~CHttpRequest()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CHttpRequest clear

void CHttpRequest::Clear()
{
	Cancel();
	
	CSingleLock pLock( &m_pSection );
	
	m_sURL.Empty();
	m_sRequestHeaders.Empty();
	
	m_nLimit		= 0;
	m_nStatusCode	= 0;
	
	m_sStatusString.Empty();
	m_pResponseHeaders.RemoveAll();
	
	if ( m_pPost != NULL ) delete m_pPost;
	m_pPost = NULL;
	
	if ( m_pResponse != NULL ) delete m_pResponse;
	m_pResponse = NULL;
}

//////////////////////////////////////////////////////////////////////
// CHttpRequest request attributes

BOOL CHttpRequest::SetURL(LPCTSTR pszURL)
{
	CSingleLock pLock( &m_pSection );
	if ( IsPending() ) return FALSE;
	if ( pszURL == NULL || _tcsncmp( pszURL, _T("http"), 4 ) ) return FALSE;
	m_sURL = pszURL;
	return TRUE;
}

CString CHttpRequest::GetURL()
{
	CSingleLock pLock( &m_pSection );
	CString strURL = m_sURL;
	return strURL;
}

void CHttpRequest::AddHeader(LPCTSTR pszKey, LPCTSTR pszValue)
{
	CSingleLock pLock( &m_pSection );
	if ( IsPending() ) return;
	
	m_sRequestHeaders += pszKey;
	m_sRequestHeaders += _T(": ");
	m_sRequestHeaders += pszValue;
	m_sRequestHeaders += _T("\r\n");
}

void CHttpRequest::SetPostData(LPCVOID pBody, DWORD nBody)
{
	CSingleLock pLock( &m_pSection );
	if ( IsPending() ) return;
	
	if ( m_pPost != NULL ) delete m_pPost;
	m_pPost = NULL;
	
	if ( pBody != NULL && nBody > 0 )
	{
		m_pPost = new CBuffer();
		m_pPost->Add( pBody, nBody );
	}
}

void CHttpRequest::SetUserAgent(LPCTSTR pszUserAgent)
{
	CSingleLock pLock( &m_pSection );
	if ( IsPending() ) return;
	m_sUserAgent = pszUserAgent;
}

void CHttpRequest::LimitContentLength(DWORD nLimit)
{
	CSingleLock pLock( &m_pSection );
	if ( IsPending() ) return;
	m_nLimit = nLimit;
}

void CHttpRequest::SetNotify(HWND hWnd, UINT nMsg, WPARAM wParam)
{
	CSingleLock pLock( &m_pSection );
	m_hNotifyWnd	= hWnd;
	m_nNotifyMsg	= nMsg;
	m_nNotifyParam	= wParam;
}

//////////////////////////////////////////////////////////////////////
// CHttpRequest response attributes

int CHttpRequest::GetStatusCode()
{
	CSingleLock pLock( &m_pSection );
	return IsPending() ? 0 : m_nStatusCode;
}

BOOL CHttpRequest::GetStatusSuccess()
{
	CSingleLock pLock( &m_pSection );
	if ( IsPending() ) return FALSE;
	return m_nStatusCode >= 200 && m_nStatusCode < 300;
}

CString CHttpRequest::GetStatusString()
{
	CSingleLock pLock( &m_pSection );
	return m_sStatusString;
}

CString CHttpRequest::GetHeader(LPCTSTR pszName)
{
	CSingleLock pLock( &m_pSection );
	CString strIn, strOut;
	
	if ( IsPending() ) return strOut;
	
	strIn = pszName;
	strIn = CharLower( strIn.GetBuffer() );
	m_pResponseHeaders.Lookup( strIn, strOut );
	
	return strOut;
}

CString CHttpRequest::GetResponseString(UINT nCodePage)
{
	CSingleLock pLock( &m_pSection );
	CString str;
	
	if ( ! IsPending() && m_pResponse != NULL )
	{
		str = m_pResponse->ReadString( m_pResponse->m_nLength, nCodePage );
	}
	
	return str;
}

CBuffer* CHttpRequest::GetResponseBuffer()
{
	CSingleLock pLock( &m_pSection );
	if ( IsPending() ) return NULL;
	return m_pResponse;
}

BOOL CHttpRequest::InflateResponse()
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	if ( IsPending() || m_pResponse == NULL ) return FALSE;
	
	CString strEncoding = GetHeader( _T("Content-Encoding") );
	
	if ( strEncoding.CompareNoCase( _T("deflate") ) == 0 )
	{
		return m_pResponse->Inflate();
	}
	
	if ( strEncoding.CompareNoCase( _T("gzip") ) == 0 )
	{
		return m_pResponse->Ungzip();
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHttpRequest process control

BOOL CHttpRequest::Execute(BOOL bBackground)
{
	if ( IsPending() ) return FALSE;
	Cancel();
	
	ASSERT( m_sURL.GetLength() > 0 );
	ASSERT( m_pPost == NULL );
	
	m_bCancel = FALSE;
	m_nStatusCode = 0;
	m_pResponseHeaders.RemoveAll();
	if ( m_pResponse != NULL ) delete m_pResponse;
	m_pResponse = NULL;
	
	if ( bBackground )
	{
		m_hThread = AfxBeginThread( (AFX_THREADPROC)ThreadStart, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL )->m_hThread;
		return TRUE;
	}
	else
	{
		Run();
		ASSERT( ! IsPending() );
		return ( m_nStatusCode >= 200 && m_nStatusCode < 300 );
	}
}

BOOL CHttpRequest::IsPending()
{
	return ( m_hInternet != NULL );
}

BOOL CHttpRequest::IsFinished()
{
	return ( m_hInternet == NULL ) && ( m_nStatusCode != 0 );
}

void CHttpRequest::Cancel()
{
	m_bCancel = TRUE;
	
	m_pSection.Lock();
	HINTERNET hInternet = m_hInternet;
	m_hInternet = NULL;
	m_pSection.Unlock();
	
	if ( hInternet != NULL ) InternetCloseHandle( hInternet );
	
	if ( m_hThread != NULL )
	{
		ASSERT( GetCurrentThread() != m_hThread );
		
        int nAttempt = 100;
		for ( ; nAttempt > 0 ; nAttempt-- )
		{
			DWORD nCode = 0;
			if ( ! GetExitCodeThread( m_hThread, &nCode ) ) break;
			if ( nCode != STILL_ACTIVE ) break;
			Sleep( 100 );
		}
		
		if ( nAttempt == 0 )
		{
			TerminateThread( m_hThread, 0 );
			Sleep( 100 );
		}
	}
	
	m_hThread = NULL;
	m_bCancel = FALSE;
}

//////////////////////////////////////////////////////////////////////
// CHttpRequest thread run

UINT CHttpRequest::ThreadStart(LPVOID lpParameter)
{
	CHttpRequest* pRequest = reinterpret_cast<CHttpRequest*>(lpParameter);
	return (DWORD)pRequest->Run();
}

int CHttpRequest::Run()
{
	CSingleLock pLock( &m_pSection, FALSE );
	
	HINTERNET hInternet = InternetOpen( m_sUserAgent, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0 );
	
	if ( hInternet != NULL )
	{
		m_hInternet = hInternet;
		
		RunRequest();
		// RunDebugRequest();
		
		pLock.Lock();
		if ( m_bCancel ) m_nStatusCode = 0;
		hInternet = m_hInternet;
		m_hInternet = NULL;
		pLock.Unlock();
		
		if ( hInternet != NULL ) InternetCloseHandle( hInternet );
	}
	
	pLock.Lock();
	if ( m_hNotifyWnd != NULL ) PostMessage( m_hNotifyWnd, m_nNotifyMsg, m_nNotifyParam, 0 );
	pLock.Unlock();
	
	return 0;
}

void CHttpRequest::RunRequest()
{
	HINTERNET hURL = InternetOpenUrl( m_hInternet, m_sURL, m_sRequestHeaders,
		m_sRequestHeaders.GetLength(), INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_RELOAD | 
		INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE , NULL );
	
	if ( hURL != NULL )
	{
		RunResponse( hURL );
		InternetCloseHandle( hURL );
	}
}

void CHttpRequest::RunResponse(HINTERNET hURL)
{
	DWORD nLength = 255;
	BYTE nNull = 0;
	
	if ( ! HttpQueryInfo( hURL, HTTP_QUERY_STATUS_TEXT,
		m_sStatusString.GetBuffer( nLength ), &nLength, 0 ) ) nLength = 0;
	m_sStatusString.ReleaseBuffer( nLength );
	if ( m_sStatusString.IsEmpty() ) return;
	
	if ( m_pResponse != NULL ) delete m_pResponse;
	m_pResponse = new CBuffer();
	
    DWORD nRemaining;
	for ( ; InternetQueryDataAvailable( hURL, &nRemaining, 0, 0 ) && nRemaining > 0 && ! m_bCancel ; )
	{
		m_pResponse->EnsureBuffer( nRemaining );
		if ( ! InternetReadFile( hURL, m_pResponse->m_pBuffer + m_pResponse->m_nLength,
			nRemaining, &nRemaining ) ) break;
		m_pResponse->m_nLength += nRemaining;
		if ( m_nLimit > 0 && m_pResponse->m_nLength > m_nLimit ) break;
	}
	
	if ( nRemaining > 0 ) return;
	
	nLength = 0;
	HttpQueryInfo( hURL, HTTP_QUERY_RAW_HEADERS, &nNull, &nLength, 0 );
	if ( nLength == 0 ) return;
	
	LPTSTR pszHeaders = new TCHAR[ nLength + 1 ];
	pszHeaders[0] = pszHeaders[1] = 0;
	HttpQueryInfo( hURL, HTTP_QUERY_RAW_HEADERS, pszHeaders, &nLength, 0 );
	
	for ( LPTSTR pszHeader = pszHeaders ; *pszHeader ; )
	{
		CString strHeader( pszHeader );
		pszHeader += strHeader.GetLength() + 1;
		
		int nColon = strHeader.Find( ':' );
		
		if ( nColon > 0 )
		{
			CString strValue, strName = strHeader.Left( nColon );
			strName.Trim(); 
			strName = CharLower( strName.GetBuffer() );
			
			while ( m_pResponseHeaders.Lookup( strName, strValue ) ) strName += _T('_');

			strValue = strHeader.Mid( nColon + 1 );
			strValue.Trim();
			m_pResponseHeaders.SetAt( strName, strValue );
		}
	}
	
	delete [] pszHeaders;
	
	nLength = 4;
	m_nStatusCode = 0;
	HttpQueryInfo( hURL, HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER,
		&m_nStatusCode, &nLength, 0 );
}

//////////////////////////////////////////////////////////////////////
// CHttpRequest clear

void CHttpRequest::CloseThread(HANDLE* phThread, LPCTSTR pszName)
{
	if ( *phThread == NULL ) return;
	
    int nAttempt = 100;
	for ( ; nAttempt > 0 ; nAttempt-- )
	{
		DWORD nCode;
		if ( ! GetExitCodeThread( *phThread, &nCode ) ) break;
		if ( nCode != STILL_ACTIVE ) break;
		Sleep( 100 );
	}
	
	if ( nAttempt == 0 )
	{
		TerminateThread( *phThread, 0 );
		if ( pszName != NULL )
		{
			theApp.Message( MSG_DEBUG,
				_T("WARNING: Terminating %s thread."), pszName );
		}
		Sleep( 100 );
	}
	
	*phThread = NULL;
}
