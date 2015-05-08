//
// HttpRequest.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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
#include "HttpRequest.h"
#include "Network.h"
#include "Buffer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CHttpRequest construction

CHttpRequest::CHttpRequest() :
	m_hInternet( NULL ),
	m_nLimit( 0 ),
	m_nStatusCode( 0 ),
//	m_pPost( NULL ),
	m_pResponse( NULL ),
	m_hNotifyWnd( NULL ),
	m_nNotifyMsg( NULL ),
	m_nNotifyParam( NULL ),
	m_bUseCookie( true )
{
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
	
	m_sURL.Empty();
	m_sRequestHeaders.Empty();
	
	m_nLimit		= 0;
	m_nStatusCode	= 0;
	
	m_sStatusString.Empty();
	m_pResponseHeaders.RemoveAll();
	
//	if ( m_pPost != NULL ) delete m_pPost;
//	m_pPost = NULL;
	
	if ( m_pResponse != NULL ) delete m_pResponse;
	m_pResponse = NULL;
}

//////////////////////////////////////////////////////////////////////
// CHttpRequest request attributes

BOOL CHttpRequest::SetURL(LPCTSTR pszURL)
{
	if ( IsPending() ) 
		return FALSE;
	if ( pszURL == NULL || _tcsncmp( pszURL, _T("http"), 4 ) != 0 ) return FALSE;
	m_sURL = pszURL;
	return TRUE;
}

CString CHttpRequest::GetURL() const
{
	return m_sURL;
}

void CHttpRequest::AddHeader(LPCTSTR pszKey, LPCTSTR pszValue)
{
	if ( IsPending() ) return;
	
	m_sRequestHeaders += pszKey;
	m_sRequestHeaders += _T(": ");
	m_sRequestHeaders += pszValue;
	m_sRequestHeaders += _T("\r\n");
}

/*void CHttpRequest::SetPostData(LPCVOID pBody, DWORD nBody)
{
	if ( IsPending() ) return;
	
	if ( m_pPost != NULL ) delete m_pPost;
	m_pPost = NULL;
	
	if ( pBody != NULL && nBody > 0 )
	{
		m_pPost = new CBuffer();
		m_pPost->Add( pBody, nBody );
	}
}*/

void CHttpRequest::LimitContentLength(DWORD nLimit)
{
	if ( IsPending() ) return;
	m_nLimit = nLimit;
}

void CHttpRequest::SetNotify(HWND hWnd, UINT nMsg, WPARAM wParam)
{
	if ( IsPending() ) return;
	m_hNotifyWnd	= hWnd;
	m_nNotifyMsg	= nMsg;
	m_nNotifyParam	= wParam;
}

//////////////////////////////////////////////////////////////////////
// CHttpRequest response attributes

int CHttpRequest::GetStatusCode() const
{
	return IsPending() ? 0 : m_nStatusCode;
}

bool CHttpRequest::GetStatusSuccess() const
{
	return ! IsPending() && m_nStatusCode >= 200 && m_nStatusCode < 300;
}

CString CHttpRequest::GetStatusString() const
{
	return IsPending() ? _T("") : m_sStatusString;
}

CString CHttpRequest::GetHeader(LPCTSTR pszName) const
{
	CString strOut;
	return ( ! IsPending() && m_pResponseHeaders.Lookup( pszName, strOut ) ) ? strOut : _T("");
}

CString CHttpRequest::GetResponseString(UINT nCodePage) const
{
	return ( ! IsPending() && m_pResponse ) ?
		m_pResponse->ReadString( m_pResponse->m_nLength, nCodePage ) : _T("");
}

CBuffer* CHttpRequest::GetResponseBuffer() const
{
	return IsPending() ? NULL : m_pResponse;
}

BOOL CHttpRequest::InflateResponse()
{
	if ( IsPending() || m_pResponse == NULL )
		return FALSE;

	CString strEncoding( GetHeader( _T("Content-Encoding") ) );

	if ( strEncoding.CompareNoCase( _T("deflate") ) == 0 )
	{
		return m_pResponse->Inflate();
	}
	else if ( strEncoding.CompareNoCase( _T("gzip") ) == 0 )
	{
		return m_pResponse->Ungzip();
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHttpRequest process control

bool CHttpRequest::Execute(bool bBackground)
{
	if ( IsPending() || m_sURL.IsEmpty() )
		return false;

	m_hInternet = NULL;
	m_nStatusCode = 0;
	m_sStatusString.Empty();
	m_pResponseHeaders.RemoveAll();

	if ( m_pResponse )
		delete m_pResponse;
	m_pResponse = NULL;

	if ( ! BeginThread( "HTTPRequest" ) )
		return false;

	if ( bBackground )
	{
		return true;
	}
	else
	{
		Wait();
		return GetStatusSuccess();
	}
}

BOOL CHttpRequest::IsPending() const
{
	return IsThreadAlive();
}

BOOL CHttpRequest::IsFinished() const
{
	return ! IsPending() && m_nStatusCode;
}

void CHttpRequest::Cancel()
{
	if ( ! IsPending() ) return;

	if ( m_hInternet )
	{
		InternetCloseHandle( m_hInternet );
		m_hInternet = NULL;
	}

	CloseThread();
}

//////////////////////////////////////////////////////////////////////
// CHttpRequest thread run

void CHttpRequest::OnRun()
{
	ASSERT( m_sURL.GetLength() );
	ASSERT( m_pResponse == NULL );

	m_hInternet = CNetwork::InternetOpen();
	if ( m_hInternet )
	{
		HINTERNET hURL = CNetwork::InternetOpenUrl( m_hInternet, m_sURL,
			m_sRequestHeaders, m_sRequestHeaders.GetLength(),
			INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_RELOAD |
			INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE |
			( m_bUseCookie ? 0 : INTERNET_FLAG_NO_COOKIES ) );
		if ( hURL )
		{
			DWORD nLength = 255;
			BYTE nNull = 0;
			if ( ! IsThreadEnabled() || ! HttpQueryInfo( hURL, HTTP_QUERY_STATUS_TEXT,
				m_sStatusString.GetBuffer( nLength ), &nLength, 0 ) ) nLength = 0;
			m_sStatusString.ReleaseBuffer( nLength );
			if ( m_sStatusString.GetLength() )
			{
				m_pResponse = new CBuffer();
				DWORD nRemaining = 0;
				for ( ; IsThreadEnabled() &&
					InternetQueryDataAvailable( hURL, &nRemaining, 0, 0 ) &&
					nRemaining > 0 &&
					m_pResponse->EnsureBuffer( nRemaining ); )
				{
					if ( ! InternetReadFile( hURL, m_pResponse->m_pBuffer +
						m_pResponse->m_nLength, nRemaining, &nRemaining ) ) break;
					m_pResponse->m_nLength += nRemaining;
					if ( m_nLimit > 0 && m_pResponse->m_nLength > m_nLimit ) break;
				}
				if ( IsThreadEnabled() && nRemaining == 0 )
				{
					nLength = 0;
					HttpQueryInfo( hURL, HTTP_QUERY_RAW_HEADERS, &nNull, &nLength, 0 );
					if ( nLength )
					{
						LPTSTR pszHeaders = new TCHAR[ nLength + 1 ];
						pszHeaders[ 0 ] = pszHeaders[ 1 ] = 0;
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
								while ( m_pResponseHeaders.Lookup( strName, strValue ) )
									strName += _T('_');
								strValue = strHeader.Mid( nColon + 1 );
								strValue.Trim();
								m_pResponseHeaders.SetAt( strName, strValue );
							}
						}
						delete [] pszHeaders;

						nLength = 4;
						HttpQueryInfo( hURL, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
							&m_nStatusCode, &nLength, 0 );
					}
				}
			}
			InternetCloseHandle( hURL );
		}
		if ( m_hInternet )
		{
			InternetCloseHandle( m_hInternet );
			m_hInternet = NULL;
		}
	}

	if ( m_hNotifyWnd )
	{
		PostMessage( m_hNotifyWnd, m_nNotifyMsg, m_nNotifyParam, 0 );
	}
}

void CHttpRequest::EnableCookie(bool bEnable)
{
	m_bUseCookie = bEnable;
}
