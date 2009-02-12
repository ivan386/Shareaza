//
// IEProtocol.cpp
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
#include "IEProtocol.h"
#include "Buffer.h"
#include "Library.h"
#include "SharedFile.h"
#include "AlbumFolder.h"
#include "LibraryFolders.h"
#include "CollectionFile.h"
#include "XML.h"
#include "ZIPFile.h"
#include "ShellIcons.h"
#include "Connection.h"
#include "ImageServices.h"
#include "ImageFile.h"
#include "ThumbCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// System

IMPLEMENT_DYNCREATE(CIEProtocol, CComObject)

IMPLEMENT_DYNCREATE(CIEProtocolRequest, CComObject)

// {18D11ED9-1264-48A1-9E14-20F2C633242B}
IMPLEMENT_OLECREATE_FLAGS(CIEProtocol, "Shareaza.IEProtocol",
	afxRegFreeThreading|afxRegApartmentThreading,
	0x18d11ed9, 0x1264, 0x48a1, 0x9e, 0x14, 0x20, 0xf2, 0xc6, 0x33, 0x24, 0x2b);

// {E1A67AE5-7041-4AE1-94F7-DE03EF759E27}
IMPLEMENT_OLECREATE_FLAGS(CIEProtocolRequest, "Shareaza.IEProtocolRequest",
	afxRegFreeThreading|afxRegApartmentThreading,
	0xe1a67ae5, 0x7041, 0x4ae1, 0x94, 0xf7, 0xde, 0x03, 0xef, 0x75, 0x9e, 0x27);

BEGIN_INTERFACE_MAP(CIEProtocol, CComObject)
	INTERFACE_PART(CIEProtocol, IID_IClassFactory, ClassFactory)
END_INTERFACE_MAP()

BEGIN_INTERFACE_MAP(CIEProtocolRequest, CComObject)
	INTERFACE_PART(CIEProtocolRequest, IID_IInternet, InternetProtocol)
	INTERFACE_PART(CIEProtocolRequest, IID_IInternetProtocol, InternetProtocol)
	INTERFACE_PART(CIEProtocolRequest, IID_IInternetProtocolRoot, InternetProtocol)
	INTERFACE_PART(CIEProtocolRequest, IID_IInternetProtocolInfo, InternetProtocolInfo)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Global Instance

LPCWSTR	CIEProtocol::pszProtocols[]	= { L"p2p-col", L"p2p-file", NULL };

CIEProtocol IEProtocol;


/////////////////////////////////////////////////////////////////////////////
// CIEProtocol construction

CIEProtocol::CIEProtocol()
{
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
		if ( FAILED( pSession->RegisterNameSpace( &m_xClassFactory, CLSID_IEProtocol,
			 pszProtocols[ nProtocol ], 0, NULL, 0 ) ) ) return FALSE;
	}

	m_pSession = pSession;

	return TRUE;
}

void CIEProtocol::Close()
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( m_pSession )
	{
		for ( int nProtocol = 0 ; pszProtocols[ nProtocol ] != NULL ; nProtocol++ )
		{
			m_pSession->UnregisterNameSpace( &m_xClassFactory, pszProtocols[ nProtocol ] );
		}
		m_pSession.Release();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CIEProtocol IClassFactory implementation

IMPLEMENT_UNKNOWN(CIEProtocol, ClassFactory)

STDMETHODIMP CIEProtocol::XClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
{
	METHOD_PROLOGUE(CIEProtocol, ClassFactory)

	if ( pUnkOuter != NULL )
		return CLASS_E_NOAGGREGATION;

	CIEProtocolRequest* pRequest = new CIEProtocolRequest();
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

CIEProtocolRequest::CIEProtocolRequest()
{
}

CIEProtocolRequest::~CIEProtocolRequest()
{
}

/////////////////////////////////////////////////////////////////////////////
// CIEProtocolRequest transfer handler

HRESULT CIEProtocolRequest::OnStart(LPCTSTR pszURL, IInternetProtocolSink* pSink, IInternetBindInfo* /*pBindInfo*/, DWORD dwFlags)
{
	HRESULT hr = IEProtocol.OnRequest( pszURL, m_oBuffer, m_strMimeType,
		( dwFlags & PI_PARSE_URL ) != 0 );

	if ( ( dwFlags & PI_PARSE_URL ) || ( hr == INET_E_INVALID_URL ) )
	{
		return hr;
	}

	m_pSink = pSink;

	if ( SUCCEEDED( hr ) )
	{
		hr = m_pSink->ReportData( BSCF_FIRSTDATANOTIFICATION,
			0, m_oBuffer.m_nLength );
		ASSERT( SUCCEEDED( hr ) );

		if ( m_strMimeType.GetLength() > 0 )
		{
			hr = m_pSink->ReportProgress( BINDSTATUS_MIMETYPEAVAILABLE,
				CComBSTR( m_strMimeType ) );
			ASSERT( SUCCEEDED( hr ) );
		}

		hr = m_pSink->ReportData( BSCF_LASTDATANOTIFICATION,
			m_oBuffer.m_nLength, m_oBuffer.m_nLength );
		ASSERT( SUCCEEDED( hr ) );

		hr = m_pSink->ReportResult( S_OK, 200, NULL );
		ASSERT( SUCCEEDED( hr ) );
	}
	else
	{
		hr = m_pSink->ReportResult( INET_E_OBJECT_NOT_FOUND, 404, NULL );
		ASSERT( SUCCEEDED( hr ) );
	}

	return hr;
}

HRESULT CIEProtocolRequest::OnRead(void* pv, ULONG cb, ULONG* pcbRead)
{
	cb = min( cb, m_oBuffer.m_nLength );
	if ( pcbRead != NULL ) *pcbRead = cb;

	if ( cb > 0 )
	{
		CopyMemory( pv, m_oBuffer.m_pBuffer, cb );
		m_oBuffer.Remove( cb );
	}

	return ( cb > 0 || m_oBuffer.m_nLength > 0 ) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CIEProtocolRequest IInternetProtocol implementation

IMPLEMENT_UNKNOWN(CIEProtocolRequest, InternetProtocol)

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Abort(HRESULT /*hrReason*/, DWORD /*dwOptions*/)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	ASSERT_VALID( pThis );
	return S_OK;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Continue(PROTOCOLDATA* /*pProtocolData*/)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	ASSERT_VALID( pThis );
	return S_OK;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Resume()
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	ASSERT_VALID( pThis );
	return E_NOTIMPL;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Start(LPCWSTR szUrl, IInternetProtocolSink *pOIProtSink, IInternetBindInfo *pOIBindInfo, DWORD grfPI, HANDLE_PTR /*dwReserved*/)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	ASSERT_VALID( pThis );
	return pThis->OnStart( CW2CT( szUrl ), pOIProtSink, pOIBindInfo, grfPI );
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Suspend()
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	ASSERT_VALID( pThis );
	return E_NOTIMPL;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Terminate(DWORD /*dwOptions*/)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	ASSERT_VALID( pThis );
	return S_OK;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::LockRequest(DWORD /*dwOptions*/)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	ASSERT_VALID( pThis );
	return S_OK;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	ASSERT_VALID( pThis );
	return pThis->OnRead( pv, cb, pcbRead );
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::Seek(LARGE_INTEGER /*dlibMove*/, DWORD /*dwOrigin*/, ULARGE_INTEGER* /*plibNewPosition*/)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	ASSERT_VALID( pThis );
	return E_FAIL;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocol::UnlockRequest()
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocol)
	ASSERT_VALID( pThis );
	pThis->m_pSink = NULL;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CIEProtocolRequest IInternetProtocolInfo implementation

IMPLEMENT_UNKNOWN(CIEProtocolRequest, InternetProtocolInfo)

STDMETHODIMP CIEProtocolRequest::XInternetProtocolInfo::CombineUrl(LPCWSTR /*pwzBaseUrl*/, LPCWSTR /*pwzRelativeUrl*/, DWORD /*dwCombineFlags*/, LPWSTR /*pwzResult*/, DWORD /*cchResult*/, DWORD* /*pcchResult*/, DWORD /*dwReserved*/)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocolInfo)
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocolInfo::CompareUrl(LPCWSTR /*pwzUrl1*/, LPCWSTR /*pwzUrl2*/, DWORD /*dwCompareFlags*/)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocolInfo)
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocolInfo::ParseUrl(LPCWSTR pwzUrl, PARSEACTION ParseAction, DWORD /*dwParseFlags*/, LPWSTR pwzResult, DWORD cchResult, DWORD *pcchResult, DWORD /*dwReserved*/)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocolInfo)
	UNUSED_ALWAYS( pwzUrl );

	// HACK: Security bypass
	switch ( ParseAction )
	{
	case PARSE_SECURITY_URL:
	case PARSE_SECURITY_DOMAIN:
		*pcchResult = lstrlen( _T(WEB_SITE) ) + 1;
		if ( cchResult < *pcchResult || pwzResult == NULL ) return S_FALSE;
		lstrcpy( pwzResult, _T(WEB_SITE) );
		return S_OK;
	default:
		return INET_E_DEFAULT_ACTION;
	}
}

STDMETHODIMP CIEProtocolRequest::XInternetProtocolInfo::QueryInfo(LPCWSTR pwzUrl, QUERYOPTION OueryOption, DWORD /*dwQueryFlags*/, LPVOID pBuffer, DWORD cbBuffer, DWORD *pcbBuf, DWORD /*dwReserved*/)
{
	METHOD_PROLOGUE(CIEProtocolRequest, InternetProtocolInfo)
	UNUSED_ALWAYS( pwzUrl );

	switch ( OueryOption )
	{
	case QUERY_USES_NETWORK:
	case QUERY_IS_SECURE:
	case QUERY_IS_SAFE:
		*pcbBuf = sizeof( DWORD );
		if ( cbBuffer < *pcbBuf || pBuffer == NULL ) return S_FALSE;
		*(DWORD*)pBuffer = 0;
		return S_OK;
	default:
		return INET_E_DEFAULT_ACTION;
	}
}


/////////////////////////////////////////////////////////////////////////////
// CIEProtocol request handler

HRESULT CIEProtocol::OnRequest(LPCTSTR pszURL, CBuffer& oBuffer, CString& sMimeType, BOOL bParseOnly)
{
	CSingleLock pLock( &m_pSection, TRUE );

	TRACE( _T("Requested URL: %s\n"), pszURL );

	if ( _tcsnicmp( pszURL, _T("p2p-col://"), 10 ) == 0 )
	{
		// p2p-col://{SHA1}/{relative path inside zip}
		return OnRequestRAZACOL( pszURL + 10, oBuffer, sMimeType, bParseOnly );
	}
	else if ( _tcsnicmp( pszURL, _T("p2p-file://"), 11 ) == 0 )
	{
		// p2p-file://{SHA1}/{preview|meta}
		return OnRequestRAZAFILE( pszURL + 11, oBuffer, sMimeType, bParseOnly );
	}
	else
	{
		return INET_E_INVALID_URL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CIEProtocol request handler - "p2p-col"

HRESULT CIEProtocol::OnRequestRAZACOL(LPCTSTR pszURL, CBuffer& oBuffer, CString& sMimeType, BOOL /*bParseOnly*/)
{
	if ( _tcslen( pszURL ) < 32 + 1 || pszURL[32] != '/' )
		return INET_E_INVALID_URL;
	
    Hashes::Sha1Hash oSHA1;
	if ( ! oSHA1.fromString( pszURL ) )
		return INET_E_INVALID_URL;

	CSingleLock oLock( &Library.m_pSection, FALSE );
	if ( ! oLock.Lock( 500 ) )
		return INET_E_INVALID_URL;

	// Render simple collection as HTML
	CAlbumFolder* pCollAlbum = LibraryFolders.GetCollection( oSHA1 );
	if ( pCollAlbum )
	{
		CCollectionFile* pCollFile = pCollAlbum->GetCollection();
		if ( pCollFile && pCollFile->IsType( CCollectionFile::SimpleCollection ) )
		{
			CString strBuffer;
			pCollFile->Render( strBuffer );
			oBuffer.Print( strBuffer, CP_UTF8 );
			sMimeType = _T("text/html");
			return S_OK;
		}
	}

	// Load file directly from ZIP
	CLibraryFile* pCollFile = LibraryMaps.LookupFileBySHA1( oSHA1, FALSE, TRUE );
	if ( ! pCollFile )
		return INET_E_INVALID_URL;

	CString strCollPath = pCollFile->GetPath();

	oLock.Unlock();

	CZIPFile oCollZIP;
	if ( ! oCollZIP.Open( strCollPath ) )
		return INET_E_INVALID_URL;

	CString strPath = URLDecode( pszURL + 32 );
	bool bDir = ( strPath.GetAt( strPath.GetLength() - 1 ) == _T('/') );

	CZIPFile::File* pFile = oCollZIP.GetFile(
		( bDir ? ( strPath + _T("index.htm") ) : strPath ).Mid( 1 ), TRUE );
	if ( ! pFile )
	{
		if ( bDir )
		{
			pFile = oCollZIP.GetFile( ( strPath + _T("collection.xml") ).Mid( 1 ), TRUE );
			if ( ! pFile )
				return INET_E_OBJECT_NOT_FOUND;
		}
		else
			return INET_E_OBJECT_NOT_FOUND;
	}

	CBuffer* pSource = pFile->Decompress();
	if ( ! pSource )
		return INET_E_OBJECT_NOT_FOUND;

	oBuffer.AddBuffer( pSource );
	delete pSource;

	ShellIcons.Lookup( PathFindExtension( strPath ), NULL, NULL, NULL, &sMimeType );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CIEProtocol request handler - "p2p-file"

HRESULT CIEProtocol::OnRequestRAZAFILE(LPCTSTR pszURL, CBuffer& oBuffer, CString& sMimeType, BOOL /*bParseOnly*/)
{
	if ( _tcslen( pszURL ) < 32 + 1 || pszURL[32] != '/' )
		return INET_E_INVALID_URL;

	Hashes::Sha1Hash oSHA1;
	if ( ! oSHA1.fromString( pszURL ) )
		return INET_E_INVALID_URL;

	CSingleLock oLock( &Library.m_pSection, FALSE );
	if ( ! oLock.Lock( 500 ) )
		return INET_E_INVALID_URL;

	CLibraryFile* pFile = LibraryMaps.LookupFileBySHA1( oSHA1, FALSE, TRUE );
	if ( ! pFile )
		return INET_E_INVALID_URL;

	if ( _tcsicmp( pszURL + 33, _T("preview") ) == 0 )
	{
		CImageFile pImage;
		if ( CThumbCache::Cache( pFile->GetPath(), &pImage ) )
		{
			BYTE* pBuffer = NULL;
			DWORD nImageSize = 0;
			if ( pImage.SaveToMemory( _T(".jpg"), 90, &pBuffer, &nImageSize ) )
			{
				oBuffer.Add( pBuffer, nImageSize );
				sMimeType = _T("image/jpeg");
				return S_OK;
			}
		}
	}
	else if ( _tcsicmp( pszURL + 33, _T("meta") ) == 0 )
	{
		CString strXML( _T("<?xml version=\"1.0\"?>") );
		if ( pFile->m_pMetadata )
			strXML += pFile->m_pMetadata->ToString();
		oBuffer.Add( (LPCSTR)CT2A( strXML ), strXML.GetLength() );
		sMimeType = _T("text/xml");
		return S_OK;
	}

	return INET_E_INVALID_URL;
}
