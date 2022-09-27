//
// IEProtocol.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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
#include "Buffer.h"
#include "CoolInterface.h"
#include "IEProtocol.h"
#include "Library.h"
#include "SharedFile.h"
#include "ShellIcons.h"
#include "AlbumFolder.h"
#include "LibraryFolders.h"
#include "LibraryHistory.h"
#include "CollectionFile.h"
#include "XML.h"
#include "ZIPFile.h"
#include "ShellIcons.h"
#include "Connection.h"
#include "ImageFile.h"
#include "Skin.h"
#include "ThumbCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// System

IMPLEMENT_DYNAMIC(CIEProtocol, CComObject)

IMPLEMENT_DYNAMIC(CIEProtocolRequest, CComObject)

// {18D11ED9-1264-48A1-9E14-20F2C633242B}
IMPLEMENT_OLECREATE_FLAGS(CIEProtocol, CLIENT_NAME _T(".IEProtocol"),
	afxRegFreeThreading|afxRegApartmentThreading,
	0x18d11ed9, 0x1264, 0x48a1, 0x9e, 0x14, 0x20, 0xf2, 0xc6, 0x33, 0x24, 0x2b)

// {E1A67AE5-7041-4AE1-94F7-DE03EF759E27}
IMPLEMENT_OLECREATE_FLAGS(CIEProtocolRequest, CLIENT_NAME _T(".IEProtocolRequest"),
	afxRegFreeThreading|afxRegApartmentThreading,
	0xe1a67ae5, 0x7041, 0x4ae1, 0x94, 0xf7, 0xde, 0x03, 0xef, 0x75, 0x9e, 0x27)

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

LPCWSTR	CIEProtocol::pszProtocols[]	= { L"p2p-col", L"p2p-file", L"p2p-app", NULL };

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
		if ( FAILED( pSession->RegisterNameSpace( &m_xClassFactory, CLSID_ShareazaIEProtocol,
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
	HRESULT hr = IEProtocol.OnRequest( pszURL, m_oBuffer, m_strMimeType, ( dwFlags & PI_PARSE_URL ) != 0 );

	if ( ( dwFlags & PI_PARSE_URL ) || ( hr == INET_E_INVALID_URL ) )
	{
		return hr;
	}

	m_pSink = pSink;

	if ( SUCCEEDED( hr ) )
	{
		if ( m_strMimeType.GetLength() > 0 )
		{
			hr = m_pSink->ReportProgress( BINDSTATUS_MIMETYPEAVAILABLE, CComBSTR( m_strMimeType ) );
			ASSERT( SUCCEEDED( hr ) );
			hr = m_pSink->ReportProgress( BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE, CComBSTR( m_strMimeType ) );
			ASSERT( SUCCEEDED( hr ) );
		}

		hr = m_pSink->ReportData( BSCF_FIRSTDATANOTIFICATION, 0, m_oBuffer.m_nLength );
		ASSERT( SUCCEEDED( hr ) );

		hr = m_pSink->ReportData( BSCF_LASTDATANOTIFICATION, m_oBuffer.m_nLength, m_oBuffer.m_nLength );
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
	return pThis->OnStart( (LPCTSTR)CW2T( szUrl ), pOIProtSink, pOIBindInfo, grfPI );
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
		_tcscpy_s( pwzResult, cchResult, _T(WEB_SITE) );
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

	if ( _tcsnicmp( pszURL, _PT("p2p-col:") ) == 0 )
	{
		return OnRequestCollection( SkipSlashes( pszURL, 8 ), oBuffer, sMimeType, bParseOnly );
	}
	else if ( _tcsnicmp( pszURL, _PT("p2p-file:") ) == 0 )
	{
		return OnRequestFile( SkipSlashes( pszURL, 9 ), oBuffer, sMimeType, bParseOnly );
	}
	else if ( _tcsnicmp( pszURL, _PT("p2p-app:") ) == 0 )
	{
		return OnRequestApplication( SkipSlashes( pszURL, 8 ), oBuffer, sMimeType, bParseOnly );
	}
	else
	{
		return INET_E_INVALID_URL;
	}
}

HRESULT CIEProtocol::OnRequestCollection(LPCTSTR pszURL, CBuffer& oBuffer, CString& sMimeType, BOOL bParseOnly)
{
	CString strURL = pszURL;

	CString strURN = strURL.SpanExcluding( _T("/") );
	if ( strURN.IsEmpty() )
		return INET_E_INVALID_URL;

	Hashes::Sha1Hash oSHA1;
	oSHA1.fromString( strURN );

	CSingleLock oLock( &Library.m_pSection, FALSE );
	if ( ! oLock.Lock( 500 ) )
		return INET_E_OBJECT_NOT_FOUND;

	if ( oSHA1 )
	{
		// Render simple collection as HTML
		CAlbumFolder* pCollAlbum = LibraryFolders.GetCollection( oSHA1 );
		if ( pCollAlbum )
		{
			CCollectionFile* pCollFile = pCollAlbum->GetCollection();
			if ( pCollFile && pCollFile->IsType( CCollectionFile::SimpleCollection ) )
			{
				if ( ! bParseOnly )
				{
					if (strURL.Find(_T("/shareaza_filelist_style.xsl")) > 0){
						oBuffer.Print( LoadHTML( GetModuleHandle( NULL ), IDR_XSL_ShareazaFileListStyle ) );
						sMimeType = _T("text/xsl");
						return S_OK;
					}

					pCollFile->Render( oBuffer );
					sMimeType = _T("text/xml");
				}
				return S_OK;
			}
			else if ( pCollFile && pCollFile->IsType( CCollectionFile::DCCollection ) )
			{
				if ( ! bParseOnly ){
					if (strURL.Find(_T("/dc_filelist_style.xsl")) > 0){
						oBuffer.Print( LoadHTML( GetModuleHandle( NULL ), IDR_XSL_DCFileListStyle ) );
						sMimeType = _T("text/xsl");
						return S_OK;
					}

					// Try to load as urn first
					CLibraryFile* pDCCollFile = LibraryMaps.LookupFileByURN( strURN, FALSE, TRUE );
					if ( ! pDCCollFile )
					{
						// else load as sha1
						if ( ! oSHA1 )
							return INET_E_INVALID_URL;
						pDCCollFile = LibraryMaps.LookupFileBySHA1( oSHA1, FALSE, TRUE );
						if ( ! pDCCollFile )
							return INET_E_INVALID_URL;
					}

					CString strDCCollPath = pDCCollFile->GetPath();
					CFile pDCFile(strDCCollPath, CFile::modeRead);
					ULONGLONG nInSize = pDCFile.GetLength();
					
					CBuffer pDCBuffer;

					if ( ! pDCBuffer.EnsureBuffer( nInSize ) )
						// Out of memory
						return INET_E_CANNOT_LOAD_DATA;

					if ( pDCFile.Read( pDCBuffer.GetData(), nInSize ) != nInSize )
						// File read error
						return INET_E_CANNOT_LOAD_DATA;
					pDCBuffer.m_nLength = nInSize;

					pDCFile.Close();

					if ( ! pDCBuffer.UnBZip() )
						// Decompression error
						return INET_E_CANNOT_LOAD_DATA;

					CString strTemp;
					strTemp.Format(	_T("<?xml-stylesheet type=\"text/xsl\" href=\"dc_filelist_style.xsl\"?>") );
					oBuffer.Print( strTemp , CP_UTF8 );
					oBuffer.AddBuffer( &pDCBuffer );
					sMimeType = _T("text/xml");
					return S_OK;
				}
			}
		}
	}

	// Load file directly from ZIP

	// Try to load as urn first
	CLibraryFile* pCollFile = LibraryMaps.LookupFileByURN( strURN, FALSE, TRUE );
	if ( ! pCollFile )
	{
		// else load as sha1
		if ( ! oSHA1 )
			return INET_E_INVALID_URL;
		pCollFile = LibraryMaps.LookupFileBySHA1( oSHA1, FALSE, TRUE );
		if ( ! pCollFile )
			return INET_E_INVALID_URL;
	}

	CString strCollPath = pCollFile->GetPath();

	oLock.Unlock();

	CZIPFile oCollZIP;
	if ( ! oCollZIP.Open( strCollPath ) )
		return INET_E_OBJECT_NOT_FOUND;

	CString strPath = URLDecode( strURL.Mid( strURN.GetLength() + 1 ) );
	bool bDir = strPath.IsEmpty() || ( strPath.GetAt( strPath.GetLength() - 1 ) == _T('/') );

	CString strFile = ( bDir ? ( strPath + _T("index.htm") ) : strPath );
	CZIPFile::File* pFile = oCollZIP.GetFile( strFile, TRUE );
	if ( ! pFile )
	{
		if ( bDir )
		{
			strFile = strPath + _T("collection.xml");
			pFile = oCollZIP.GetFile( strFile, TRUE );
			if ( ! pFile )
				return INET_E_OBJECT_NOT_FOUND;
		}
		else
			return INET_E_OBJECT_NOT_FOUND;
	}

	CAutoPtr< CBuffer > pSource ( pFile->Decompress() );
	if ( ! pSource )
		return INET_E_OBJECT_NOT_FOUND;

	if ( ! bParseOnly )
	{
		oBuffer.AddBuffer( pSource );
		sMimeType = ShellIcons.GetMIME( PathFindExtension( strFile ) );
	}
	return S_OK;
}

HRESULT CIEProtocol::OnRequestFile(LPCTSTR pszURL, CBuffer& oBuffer, CString& sMimeType, BOOL bParseOnly)
{
	CString strURL = pszURL;

	CString strURN = strURL.SpanExcluding( _T("/") );
	if ( strURN.IsEmpty() )
		return INET_E_INVALID_URL;

	CString strVerb = URLDecode( strURL.Mid( strURN.GetLength() + 1 ) );
	if ( strVerb.IsEmpty() )
		return INET_E_INVALID_URL;

	CSingleLock oLock( &Library.m_pSection, FALSE );
	if ( ! oLock.Lock( 500 ) )
		return INET_E_OBJECT_NOT_FOUND;

	// Try to load as urn first
	CLibraryFile* pFile = LibraryMaps.LookupFileByURN( strURN, FALSE, TRUE );
	if ( ! pFile )
	{
		// else load as sha1
		Hashes::Sha1Hash oSHA1;
		if ( ! oSHA1.fromString( strURN ) )
			return INET_E_INVALID_URL;
		pFile = LibraryMaps.LookupFileBySHA1( oSHA1, FALSE, TRUE );
		if ( ! pFile )
			return INET_E_INVALID_URL;
	}

	HRESULT ret = INET_E_INVALID_URL;
	if ( strVerb.CompareNoCase( _T("preview") ) == 0 )
	{
		if ( bParseOnly )
			return S_OK;

		CImageFile pImage;
		if ( CThumbCache::Cache( pFile->GetPath(), &pImage ) )
		{
			CAutoVectorPtr< BYTE > pBuffer;
			DWORD nImageSize = 0;
			if ( pImage.SaveToMemory( _T(".jpg"), 90, &pBuffer.m_p, &nImageSize ) )
			{
				oBuffer.Add( pBuffer, nImageSize );
				sMimeType = _T("image/jpeg");
				ret = S_OK;
			}
		}
	}
	else if ( strVerb.CompareNoCase( _T("meta") ) == 0 )
	{
		if ( bParseOnly )
			return S_OK;

		CString strXML;
		if ( pFile->m_pMetadata )
			strXML = pFile->m_pMetadata->ToString( TRUE, FALSE, TRUE );
		else
			strXML = _T("<?xml version=\"1.0\"?>");
		oBuffer.Print( strXML, CP_UTF8 );
		sMimeType = _T("text/xml");
		ret = S_OK;
	}
	else if ( strVerb.Left( 4 ).CompareNoCase( _T("icon") ) == 0 )
	{
		if ( bParseOnly )
			return S_OK;

		int cx = max( min( _tstoi( strVerb.Mid( 4 ) ), 256 ), 16 );
		if ( HICON hIcon = ShellIcons.ExtractIcon( ShellIcons.Get( pFile->GetPath(), cx ), cx ) )
		{
			if ( SaveIcon( hIcon, oBuffer ) )
			{
				sMimeType = _T("image/x-icon");
				ret = S_OK;
			}
			DeleteObject( hIcon );
		}
	}
	return ret;
}

CString ToCSSColor(COLORREF rgb)
{
	CString strColor;
	strColor.Format( _T("#%02x%02x%02x"), GetRValue( rgb ), GetGValue( rgb ), GetBValue( rgb ) );
	return strColor;
}

HRESULT CIEProtocol::OnRequestApplication(LPCTSTR pszURL, CBuffer& oBuffer, CString& sMimeType, BOOL bParseOnly)
{
	if ( _tcsnicmp( pszURL, _PT("history") ) == 0 )
	{
		if ( bParseOnly )
			return S_OK;

		CString strXML;
		strXML.Format( _T("<html>\n<head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n<style type=\"text/css\">\n")
			_T("body { font-family: %s; font-size: %upx; margin: 0; padding: 0; background-color: %s; color: %s; }\n")
			_T("h1 { font-size: 120%%; font-weight: bold; color: %s; background-color: %s; margin: 0; }\n")
			_T("table { width: 100%%; font-size: 100%%; margin: 0; padding: 0; table-layout: fixed; }\n")
			_T(".name0 { width: 41%%; background-color: %s; color: %s; cursor: hand; }\n")
			_T(".time0 { width: 8%%; background-color: %s; text-align: right; }\n")
			_T(".name1 { width: 41%%; background-color: %s; color: %s; cursor: hand; }\n")
			_T(".time1 { width: 8%%; background-color: %s; text-align: right; }\n")
			_T(".icon { width: 16px; height: 16px; border-style: none; }\n")
			_T("</style>\n</head>\n<body onmousemove=\"window.external.hover(''); event.cancel\">\n<h1> %s </h1>\n<table>\n"),
			/* body */ (LPCTSTR)Settings.Fonts.DefaultFont, Settings.Fonts.FontSize,
			(LPCTSTR)ToCSSColor( CoolInterface.m_crWindow ), (LPCTSTR)ToCSSColor( CoolInterface.m_crDisabled ),
			/* h1 */ (LPCTSTR)ToCSSColor( Skin.m_crBannerText ) , (LPCTSTR)ToCSSColor( Skin.m_crBannerBack ),
			/* .name0 */ (LPCTSTR)ToCSSColor( Skin.m_crSchemaRow[ 0 ] ), (LPCTSTR)ToCSSColor( CoolInterface.m_crTextLink ),
			/* .time0 */ (LPCTSTR)ToCSSColor( Skin.m_crSchemaRow[ 0 ] ),
			/* .name1 */ (LPCTSTR)ToCSSColor( Skin.m_crSchemaRow[ 1 ] ), (LPCTSTR)ToCSSColor( CoolInterface.m_crTextLink ),
			/* .time1 */ (LPCTSTR)ToCSSColor( Skin.m_crSchemaRow[ 1 ] ),
			/* h1 */ (LPCTSTR)Escape( LoadString( IDS_LIBPANEL_RECENT_ADDITIONS ) ) );

		CSingleLock oLock( &Library.m_pSection, FALSE );
		if ( ! oLock.Lock( 500 ) )
			return INET_E_OBJECT_NOT_FOUND;

		int nCount = 0;
		for ( POSITION pos = LibraryHistory.GetIterator() ; pos ; )
		{
			const CLibraryRecent* pRecent = LibraryHistory.GetNext( pos );
			if ( ! pRecent->m_pFile )
				continue;

			CString strURN = pRecent->m_pFile->GetURN();
			if ( strURN.IsEmpty() )
				continue;

			CString sTime;
			SYSTEMTIME tAdded;
			FileTimeToSystemTime( &pRecent->m_tAdded, &tAdded );
			SystemTimeToTzSpecificLocalTime( NULL, &tAdded, &tAdded );
			GetDateFormat( LOCALE_USER_DEFAULT, NULL, &tAdded, _T("ddd',' MMM dd"), sTime.GetBuffer( 64 ), 64 );
			sTime.ReleaseBuffer();

			if ( ( nCount & 1 ) == 0 )
				strXML += _T("<tr>");

			strXML.AppendFormat(
				_T("<td class=\"name%d\" onclick=\"window.external.display('%s');\" onmousemove=\"window.external.hover('%s'); window.event.cancelBubble = true;\">")
				_T("<img class=\"icon\" src=\"p2p-file://%s/icon16\"> %s </a></td>")
				_T("<td class=\"time%d\"> %s </td>"),
				( nCount & 2 ) >> 1, (LPCTSTR)Escape( strURN ), (LPCTSTR)Escape( strURN ), (LPCTSTR)Escape( strURN ), (LPCTSTR)Escape( pRecent->m_pFile->m_sName ),
				( nCount & 2 ) >> 1, (LPCTSTR)Escape( sTime ) );

			if ( ( nCount & 1 ) != 0 )
				strXML += _T("</tr>\n");

			nCount++;
		}

		if ( nCount && ( nCount & 1 ) != 0 )
			strXML += _T("<td></td><td></td></tr>\n");

		strXML += _T("</table>\n</body>\n</html>");

		oBuffer.Print( strXML, CP_UTF8 );
		sMimeType = _T("text/html");

		return S_OK;
	}

	return INET_E_INVALID_URL;
}
