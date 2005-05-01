//
// IEProtocol.cpp
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
#include "IEProtocol.h"
#include "Buffer.h"

#include "SHA.h"
#include "ZIPFile.h"
#include "ShellIcons.h"
#include "Connection.h"


/////////////////////////////////////////////////////////////////////////////
// System

IMPLEMENT_DYNAMIC(CIEProtocol, CCmdTarget)
IMPLEMENT_DYNAMIC(CIEProtocolRequest, CCmdTarget)

BEGIN_INTERFACE_MAP(CIEProtocol, CCmdTarget)
	INTERFACE_PART(CIEProtocol, IID_IClassFactory, ClassFactory)
END_INTERFACE_MAP()

BEGIN_INTERFACE_MAP(CIEProtocolRequest, CCmdTarget)
	INTERFACE_PART(CIEProtocolRequest, IID_IInternetProtocol, InternetProtocol)
	INTERFACE_PART(CIEProtocolRequest, IID_IInternetProtocolRoot, InternetProtocol)
	INTERFACE_PART(CIEProtocolRequest, IID_IInternetProtocolInfo, InternetProtocolInfo)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Global Instance

CLSID	CIEProtocol::clsidProtocol	= { 0x18d11ed9, 0x1264, 0x48a1, { 0x9e, 0x14, 0x20, 0xf2, 0xc6, 0x33, 0x24, 0x2b } };
LPCWSTR	CIEProtocol::pszProtocols[]	= { L"p2p-col", L"telnet", NULL };

CIEProtocol IEProtocol;


/////////////////////////////////////////////////////////////////////////////
// CIEProtocol construction

CIEProtocol::CIEProtocol()
{
	m_pShutdown = NULL;
	m_nRequests = 0;

	m_pCollZIP	= NULL;
}

CIEProtocol::~CIEProtocol()
{
	Close();
}

/////////////////////////////////////////////////////////////////////////////
// CIEProtocol session control

BOOL CIEProtocol::Create()
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( m_pSession ) return TRUE;

	CComPtr<IInternetSession> pSession;

	if ( FAILED( CoInternetGetSession( 0, &pSession, 0 ) ) ) return FALSE;

	for ( int nProtocol = 0 ; pszProtocols[ nProtocol ] != NULL ; nProtocol++ )
	{
		if ( FAILED( pSession->RegisterNameSpace( &m_xClassFactory, clsidProtocol,
			 pszProtocols[ nProtocol ], 0, NULL, 0 ) ) ) return FALSE;
	}

	m_pSession = pSession;

	return TRUE;
}

void CIEProtocol::Close()
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( m_nRequests > 0 )
	{
		m_pShutdown = new CEvent();

		pLock.Unlock();
		WaitForSingleObject( *m_pShutdown, 5000 );
		pLock.Lock();

		delete m_pShutdown;
		m_pShutdown = NULL;

		ASSERT( m_nRequests == 0 );
		m_nRequests = 0;
	}

	if ( m_pSession )
	{
		for ( int nProtocol = 0 ; pszProtocols[ nProtocol ] != NULL ; nProtocol++ )
		{
			m_pSession->UnregisterNameSpace( &m_xClassFactory, pszProtocols[ nProtocol ] );
		}

		m_pSession = NULL;
	}

	SetCollection( NULL, NULL );
}

/////////////////////////////////////////////////////////////////////////////
// CIEProtocol request management

CIEProtocolRequest* CIEProtocol::CreateRequest()
{
	CSingleLock pLock( &m_pSection, TRUE );
	return new CIEProtocolRequest( this );
}

void CIEProtocol::OnRequestConstruct(CIEProtocolRequest* pRequest)
{
	CSingleLock pLock( &m_pSection, TRUE );
	m_nRequests ++;
}

void CIEProtocol::OnRequestDestruct(CIEProtocolRequest* pRequest)
{
	CSingleLock pLock( &m_pSection, TRUE );

	ASSERT( m_nRequests > 0 );

	if ( --m_nRequests == 0 )
	{
		if ( m_pShutdown != NULL ) m_pShutdown->SetEvent();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CIEProtocol IClassFactory implementation

IMPLEMENT_UNKNOWN(CIEProtocol, ClassFactory)

STDMETHODIMP CIEProtocol::XClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
{
	METHOD_PROLOGUE(CIEProtocol, ClassFactory)

	if ( pUnkOuter != NULL ) return CLASS_E_NOAGGREGATION;

	CIEProtocolRequest* pRequest = pThis->CreateRequest();
	HRESULT hr = pRequest->ExternalQueryInterface( &riid, ppvObject );
	pRequest->ExternalRelease();

	return hr;
}

STDMETHODIMP CIEProtocol::XClassFactory::LockServer(BOOL fLock)
{
	METHOD_PROLOGUE(CIEProtocol, ClassFactory)

	if ( fLock )
		AfxOleLockApp();
	else
		AfxOleUnlockApp();

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// CIEProtocolRequest construction

CIEProtocolRequest::CIEProtocolRequest(CIEProtocol* pProtocol)
{
	ASSERT( pProtocol != NULL );

	m_pProtocol	= pProtocol;
	m_pBuffer	= new CBuffer();

	m_pProtocol->OnRequestConstruct( this );
}

CIEProtocolRequest::~CIEProtocolRequest()
{
	ASSERT( m_pSink == NULL );

	m_pProtocol->OnRequestDestruct( this );

	delete m_pBuffer;
}

/////////////////////////////////////////////////////////////////////////////
// CIEProtocolRequest transfer handler

HRESULT CIEProtocolRequest::OnStart(LPCTSTR pszURL, IInternetProtocolSink* pSink, IInternetBindInfo* pBindInfo, DWORD dwFlags)
{
	CSingleLock pLock( &m_pSection, TRUE );
	CString strMimeType;

	if ( m_pSink ) return E_UNEXPECTED;

	HRESULT hr = m_pProtocol->OnRequest( pszURL, m_pBuffer, &strMimeType,
					( dwFlags & PI_PARSE_URL ) != 0 );

	if ( ( dwFlags & PI_PARSE_URL ) || ( hr == INET_E_INVALID_URL ) ) return hr;

	m_pSink = pSink;

	if ( SUCCEEDED(hr) )
	{
		if ( strMimeType.GetLength() > 0 )
		{
			BSTR bsMimeType = strMimeType.AllocSysString();
			m_pSink->ReportProgress( BINDSTATUS_MIMETYPEAVAILABLE, bsMimeType );
			SysFreeString( bsMimeType );
		}

		m_pSink->ReportData( BSCF_LASTDATANOTIFICATION, m_pBuffer->m_nLength, m_pBuffer->m_nLength );
		m_pSink->ReportResult( S_OK, 200, L"OK" );
	}
	else
	{
		m_pSink->ReportResult( INET_E_OBJECT_NOT_FOUND, 404, L"OK" );
	}

	return hr;
}

HRESULT CIEProtocolRequest::OnRead(void* pv, ULONG cb, ULONG* pcbRead)
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( m_pSink == NULL ) return E_UNEXPECTED;

	cb = min( cb, m_pBuffer->m_nLength );
	if ( pcbRead != NULL ) *pcbRead = cb;

	if ( cb > 0 )
	{
		CopyMemory( pv, m_pBuffer->m_pBuffer, cb );
		m_pBuffer->Remove( cb );
	}

	return ( cb > 0 || m_pBuffer->m_nLength > 0 ) ? S_OK : S_FALSE;
}

HRESULT CIEProtocolRequest::OnTerminate()
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( m_pSink == NULL ) return E_UNEXPECTED;

	m_pSink = NULL;
	m_pBuffer->Clear();

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CIEProtocolRequest IInternetProtocol implementation

IMPLEMENT_UNKNOWN(CIEProtocolRequest, InternetProtocol)

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Abort(HRESULT hrReason, DWORD dwOptions)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	return S_OK;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Continue(PROTOCOLDATA *pProtocolData)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	return S_OK;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Resume()
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	return E_NOTIMPL;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Start(LPCWSTR szUrl, IInternetProtocolSink *pOIProtSink, IInternetBindInfo *pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	return pThis->OnStart( CString( szUrl ), pOIProtSink, pOIBindInfo, grfPI );
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Suspend()
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	return E_NOTIMPL;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Terminate(DWORD dwOptions)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	return pThis->OnTerminate();
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::LockRequest(DWORD dwOptions)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	return S_OK;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	return pThis->OnRead( pv, cb, pcbRead );
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	return E_FAIL;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::UnlockRequest()
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CIEProtocolRequest IInternetProtocolInfo implementation

IMPLEMENT_UNKNOWN(CIEProtocolRequest, InternetProtocolInfo)

STDMETHODIMP CIEProtocolRequest::XInternetProtocolInfo::CombineUrl(LPCWSTR pwzBaseUrl, LPCWSTR pwzRelativeUrl, DWORD dwCombineFlags, LPWSTR pwzResult, DWORD cchResult, DWORD *pcchResult, DWORD dwReserved)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocolInfo)
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocolInfo::CompareUrl(LPCWSTR pwzUrl1, LPCWSTR pwzUrl2, DWORD dwCompareFlags)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocolInfo)
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocolInfo::ParseUrl(LPCWSTR pwzUrl, PARSEACTION ParseAction, DWORD dwParseFlags, LPWSTR pwzResult, DWORD cchResult, DWORD *pcchResult, DWORD dwReserved)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocolInfo)
	switch ( ParseAction )
	{
	case PARSE_SECURITY_URL:
		// Substitute a known remote URL, to class this as an offsite/high-security zone
		*pcchResult = _tcslen( _T("http://p2p-col.shareaza.com/") ) + 1;
		if ( cchResult < *pcchResult ) return S_FALSE;
		wcscpy( pwzResult, L"http://p2p-col.shareaza.com/" );
		return S_OK;
	default:
		return INET_E_DEFAULT_ACTION;
	}
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocolInfo::QueryInfo(LPCWSTR pwzUrl, QUERYOPTION OueryOption, DWORD dwQueryFlags, LPVOID pBuffer, DWORD cbBuffer, DWORD *pcbBuf, DWORD dwReserved)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocolInfo)
	switch ( OueryOption )
	{
	case QUERY_USES_NETWORK:
	case QUERY_IS_SECURE:
	case QUERY_IS_SAFE:
		*pcbBuf = 4;
		if ( cbBuffer < 4 ) return S_FALSE;
		*(DWORD*)pBuffer = 0;
		return S_OK;
	default:
		return INET_E_DEFAULT_ACTION;
	}
}


/////////////////////////////////////////////////////////////////////////////
// CIEProtocol request handler

HRESULT CIEProtocol::OnRequest(LPCTSTR pszURL, CBuffer* pBuffer, CString* psMimeType, BOOL bParseOnly)
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( _tcsnicmp( pszURL, _T("p2p-col://"), 10 ) == 0 )
	{
		return OnRequestRAZACOL( pszURL + 10, pBuffer, psMimeType, bParseOnly );
	}
	else
	{
		return INET_E_INVALID_URL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CIEProtocol request handler - "p2p-col"

HRESULT CIEProtocol::OnRequestRAZACOL(LPCTSTR pszURL, CBuffer* pBuffer, CString* psMimeType, BOOL bParseOnly)
{
	if ( _tcslen( pszURL ) < 32 + 1 ) return INET_E_INVALID_URL;
	if ( pszURL[32] != '/' ) return INET_E_INVALID_URL;

	SHA1 pSHA1;
	if ( ! CSHA::HashFromString( pszURL, &pSHA1 ) ) return INET_E_INVALID_URL;

	if ( m_pCollZIP == NULL || ! m_pCollZIP->IsOpen() ) return INET_E_OBJECT_NOT_FOUND;
	if ( m_pCollSHA1 != pSHA1 ) return INET_E_OBJECT_NOT_FOUND;

	CString strPath( pszURL + 32 );
	strPath = CConnection::URLDecode( strPath );
	if ( strPath.Right( 1 ) == _T("/") ) strPath += _T("index.htm");

	CZIPFile::File* pFile = m_pCollZIP->GetFile( strPath.Mid( 1 ) );
	if ( pFile == NULL ) return INET_E_OBJECT_NOT_FOUND;

	CBuffer* pSource = pFile->Decompress();
	if ( pSource == NULL ) return INET_E_OBJECT_NOT_FOUND;

	pBuffer->AddBuffer( pSource );
	delete pSource;

	ShellIcons.Lookup( strPath, NULL, NULL, NULL, psMimeType );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CIEProtocol collection

BOOL CIEProtocol::SetCollection(SHA1* pSHA1, LPCTSTR pszPath, CString* psIndex)
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( m_pCollZIP != NULL )
	{
		delete m_pCollZIP;
		m_pCollZIP = NULL;
	}

	if ( pSHA1 == NULL || pszPath == NULL ) return TRUE;

	m_pCollZIP	= new CZIPFile();
	m_pCollSHA1	= *pSHA1;

	if ( m_pCollZIP->Open( pszPath ) )
	{
		if ( CZIPFile::File* pFile = m_pCollZIP->GetFile( _T("index.htm"), TRUE ) )
		{
			if ( psIndex != NULL ) *psIndex = pFile->m_sName;
		}

		return TRUE;
	}
	else
	{
		delete m_pCollZIP;
		m_pCollZIP = NULL;
		return FALSE;
	}
}
