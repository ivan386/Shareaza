//
// ShareMonkeyData.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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
#include "ShareMonkeyData.h"
#include "Settings.h"
#include "Library.h"
#include "CtrlLibraryFileView.h"
#include "SharedFile.h"
#include "XML.h"
#include "Schema.h"
#include "SchemaCache.h"

CShareMonkeyData::CShareMonkeyData(INT_PTR nOffset, int nRequestType)
: m_nFileIndex( 0 )
, m_hInternet( NULL )
, m_hSession( NULL )
, m_hRequest( NULL )
, m_nDelay( 0 )
, m_nFailures( 0 )
, m_pXML( NULL )
, m_pRazaXML( NULL )
, m_pSchema( NULL )
, m_pFileView( NULL )
, m_nRequestType( nRequestType )
, m_nOffset( nOffset )
{
	int nLength = GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, NULL, 0 );
	LPTSTR pszCountry = m_sCountry.GetBuffer( nLength );
	VERIFY( GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, pszCountry, nLength ) );
	m_sCountry.ReleaseBuffer();
}

CShareMonkeyData::~CShareMonkeyData()
{
	Stop();
	Clear();
	if ( m_pChild )
	{
		delete m_pChild;
		m_pChild = NULL;
	}
	CMetaPanel::Clear();
}

void CShareMonkeyData::Clear()
{
	if ( m_pXML )
	{
		delete m_pXML;
		m_pXML = NULL;
	}
	if ( m_pRazaXML )
	{
		delete m_pRazaXML;
		m_pRazaXML = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// CShareMonkeyData start
BOOL CShareMonkeyData::Start(CLibraryFileView* pView, DWORD nFileIndex)
{
	if ( m_hInternet != NULL )
		return FALSE;

	CString strAgent = Settings.SmartAgent();

	m_hInternet = InternetOpen( strAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
	if ( ! m_hInternet ) return FALSE;

	m_hSession	= NULL;
	m_hRequest	= NULL;
	m_nDelay	= 0;
	m_nFailures	= 0;
	m_pFileView = pView;
	m_nFileIndex = nFileIndex;

	return BeginThread( "ShareMonkeyData" );
}

//////////////////////////////////////////////////////////////////////
// CShareMonkeyData stop
void CShareMonkeyData::Stop()
{
	if ( m_hSession != NULL )
		InternetCloseHandle( m_hSession );
	m_hSession = NULL;

	if ( m_hInternet )
		InternetCloseHandle( m_hInternet );
	m_hInternet = NULL;

	CloseThread();

	Clear();
}

//////////////////////////////////////////////////////////////////////
// CShareMonkeyData thread run
void CShareMonkeyData::OnRun()
{
	Sleep( 50 ); // Display "Please wait..." message
	while ( m_hInternet != NULL )
	{
		m_pSection.Lock();

		if ( BuildRequest() )
		{
			if ( ExecuteRequest() )
			{
				if ( DecodeResponse( m_sStatus ) )
				{
					NotifyWindow();
					m_sStatus.Empty();
				}
				else
				{
					if ( m_hInternet == NULL ) break;
					NotifyWindow( (LPCTSTR)m_sStatus );
				}
				break;
			}
			else if ( ++m_nFailures >= 3 )
			{
				if ( m_hInternet == NULL ) break;
				m_sStatus = L"Failed";
				NotifyWindow( (LPCTSTR)m_sStatus );
				break;
			}
			else
			{
				if ( m_hInternet == NULL ) break;
				if ( m_hRequest != NULL )
					InternetCloseHandle( m_hRequest );
				m_hRequest = NULL;
				m_sStatus = L"Failed. Retrying...";
				if ( !NotifyWindow( (LPCTSTR)m_sStatus ) ) break;
				Sleep( 1000 );
			}
		}

		if ( m_hRequest != NULL )
			InternetCloseHandle( m_hRequest );
		m_hRequest = NULL;
		m_sResponse.Empty();

		Sleep( min( m_nDelay, 500ul ) );
	}

	if ( m_hSession != NULL )
		InternetCloseHandle( m_hSession );
	m_hSession = NULL;

	if ( m_hInternet )
		InternetCloseHandle( m_hInternet );
	m_hInternet = NULL;
}

bool CShareMonkeyData::NotifyWindow(LPCTSTR pszMessage) const
{
	if ( m_pFileView && IsWindow( m_pFileView->m_hWnd ) )
	{
		 m_pFileView->SendMessage( WM_METADATA, (WPARAM)this, (LPARAM)pszMessage );
		 return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CShareMonkeyData::BuildRequest()
{
	int nCategory = 0;

	m_sURL = Settings.Search.ShareMonkeyBaseURL;
	m_pSchema = NULL;

	if ( m_nRequestType == stProductMatch )
	{
		CString str;
		str.Format( L"&offset=%i", m_nOffset );
		m_sURL += L"productMatch?v=latest&stores_amount=0&result_amount=1" + str;
	}
	else if ( m_nRequestType == stStoreMatch )
	{
		// storeMatch/<session_id>/<contributor_id>/<file_id>/<product_id>/COUNTRY
		CString str;
		str.Format( L"storeMatch/%s/%s/0/%s/%s", (LPCTSTR)m_sSessionID,
			(LPCTSTR)Settings.WebServices.ShareMonkeyCid,
			(LPCTSTR)m_sProductID, (LPCTSTR)m_sCountry );
		m_sURL += str;
	}
	else if ( m_nRequestType == stComparison )
		m_sURL += L"productMatch?v=latest&stores_amount=0&result_amount=0";

	if ( m_nRequestType == stProductMatch || m_nRequestType == stComparison )
	{
		if ( theApp.m_nUPnPExternalAddress.s_addr != INADDR_NONE )
		{
			m_sURL += L"&user_ip_address=";
			m_sURL += inet_ntoa( theApp.m_nUPnPExternalAddress );
		}
		else
		{
			m_sURL += L"&user_ip_address=" + m_sCountry;
		}

		{
			CQuickLock oLock( Library.m_pSection );
			CLibraryFile* pFile = Library.LookupFile( m_nFileIndex );

			if ( pFile == NULL )
				return FALSE;

			m_sURL += L"&n=" + pFile->m_sName;
			m_sURL += L"&sha1=" + pFile->m_oSHA1.toString();
			m_sURL += L"&ed2k=" + pFile->m_oED2K.toString();
			m_sURL += L"&tth=" + pFile->m_oTiger.toString();
			if ( pFile->m_nSize != 0 )
			{
				CString strSize;
				strSize.Format( L"&s=%I64u", pFile->m_nSize );
				m_sURL += strSize;
			}

			bool bTorrent = false;
			bool bAppOrGame = false;

			if ( pFile->m_pSchema )
			{
				m_pSchema = SchemaCache.Get( (LPCTSTR)pFile->m_pSchema->GetURI() );
				if ( pFile->m_pSchema->CheckURI( CSchema::uriAudio ) )
				{
					nCategory = 1;
					if ( pFile->m_pMetadata )
					{
						CXMLAttribute* pAttribute = pFile->m_pMetadata->GetAttribute( L"title" );
						if ( pAttribute )
							m_sURL += L"&audiotitle=" + pAttribute->GetValue();
						pAttribute = pFile->m_pMetadata->GetAttribute( L"artist" );
						if ( pAttribute )
							m_sURL += L"&audioartist=" + pAttribute->GetValue();
						pAttribute = pFile->m_pMetadata->GetAttribute( L"album" );
						if ( pAttribute )
							m_sURL += L"&audioalbum=" + pAttribute->GetValue();
						pAttribute = pFile->m_pMetadata->GetAttribute( L"track" );
						if ( pAttribute )
							m_sURL += L"&tn=" + pAttribute->GetValue();
					}
				}
				else if ( pFile->m_pSchema->CheckURI( CSchema::uriDocument ) ||
						pFile->m_pSchema->CheckURI( CSchema::uriBook ) )
					nCategory = 2;
				else if ( pFile->m_pSchema->CheckURI( CSchema::uriApplication ) )
					nCategory = 4;
				else if ( pFile->m_pSchema->CheckURI( CSchema::uriBitTorrent ) )
					bTorrent = true;
				else if ( pFile->m_pSchema->CheckURI( CSchema::uriROM ) ||
						pFile->m_pSchema->CheckURI( CSchema::uriArchive ) )
					bAppOrGame = true;

				if ( bAppOrGame && pFile->m_pMetadata )
				{
					CString strWords = pFile->m_pSchema->GetIndexedWords( pFile->m_pMetadata->GetFirstElement() );
					if ( _tcsistr( strWords, L"game" ) != NULL )
						nCategory = 3;
					else if ( _tcsistr( strWords, L"software" ) != NULL ||
							_tcsistr( strWords, L"application" ) != NULL )
						nCategory = 4;
				}
				else if ( bTorrent && pFile->m_pMetadata && pFile->m_bMetadataAuto )
				{
					CXMLAttribute* pInfoHash = pFile->m_pMetadata->GetAttribute( L"hash" );
					if ( pInfoHash )
						m_sURL += L"&info_hash=" + pInfoHash->GetValue();
				}

				if ( nCategory != 0 )
				{
					CString strCategory;
					strCategory.Format( L"&category_id=%i", nCategory );
					m_sURL += strCategory;
				}
			}

			m_sURL += L"&cid=";
			m_sURL += Settings.WebServices.ShareMonkeyCid;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CShareMonkeyData request executer

BOOL CShareMonkeyData::ExecuteRequest()
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
		m_hSession = InternetConnect( m_hInternet, strHost, INTERNET_PORT( nPort ),
			NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0 );
		if ( m_hSession == NULL ) return FALSE;
	}

	m_hRequest = HttpOpenRequest( m_hSession, _T("GET"), strPath, NULL, NULL, NULL,
		INTERNET_FLAG_KEEP_CONNECTION|INTERNET_FLAG_NO_COOKIES, 0 );

	if ( m_hRequest == NULL )
	{
		if ( m_hSession != NULL ) InternetCloseHandle( m_hSession );

		m_hSession = InternetConnect( m_hInternet, strHost, INTERNET_PORT( nPort ),
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

	if ( _stscanf( szStatusCode, _T("%u"), &nStatusCode ) != 1 ||
		nStatusCode < 200 || nStatusCode > 299 ) return FALSE;

	LPBYTE pResponse = NULL;
	DWORD nRemaining, nResponse = 0;

	while ( InternetQueryDataAvailable( m_hRequest, &nRemaining, 0, 0 ) && nRemaining > 0 )
	{
		BYTE* pNewResponse = (BYTE*)realloc( pResponse, nResponse + nRemaining );
		if ( ! pNewResponse )
		{
			free( pResponse );
			return FALSE;
		}
		pResponse = pNewResponse;
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

	if ( m_hRequest != NULL )
		InternetCloseHandle( m_hRequest );
	m_hRequest = NULL;

	m_nDelay = ( GetTickCount() - nTime ) * 2;

	return TRUE;
}

BOOL CShareMonkeyData::DecodeResponse(CString& strMessage)
{
	if ( m_pXML && m_nRequestType == stProductMatch )
		Clear();

	m_pXML = CXMLElement::FromString( m_sResponse, TRUE );
	if ( m_pXML == NULL )
	{
		strMessage = L"Invalid XML";
		return FALSE;
	}

	bool bFailed = false;
	CString strStatus, strWarnings;
	BOOL bResult = FALSE;

	for ( POSITION pos = m_pXML->GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = m_pXML->GetNextElement( pos );
		if ( pElement->IsNamed( L"Version" ) )
		{
			CString strVersion = pElement->GetValue();
			float nVersion = 0;
			if ( _stscanf( strVersion, L"%f", &nVersion ) != 1 || nVersion > 1.5f )
			{
				strMessage = L"Failed: Version mismatch.";
				Clear();
				return FALSE;
			}
		}
		else if ( pElement->IsNamed( L"Status" ) )
		{
			strStatus = pElement->GetValue();
			if ( strStatus.CompareNoCase( L"success" ) != NULL )
			{
				if ( m_nRequestType != stComparison )
					bFailed = true;
			}
		}
		else if ( pElement->IsNamed( L"ShareMonkeyComparisonURL" ) )
		{
			m_sComparisonURL = pElement->GetValue();
			if ( m_sComparisonURL.IsEmpty() && m_nRequestType == stComparison )
			{
				strMessage = L"Failed: No URL is available.";
				Clear();
				return FALSE;
			}

			int curPos = 0, currToken = 0;
			CString strToken;
			while ( curPos != -1 )
			{
				strToken = m_sComparisonURL.Tokenize( L"/", curPos );
				if ( ++currToken == 3 )
				{
					m_sSessionID = strToken;
					break;
				}
			}
		}
		else if ( pElement->IsNamed( L"Count" ) && m_nRequestType != stComparison )
		{
			if ( pElement->GetValue() == L"0" )
				bFailed = true;
		}
		else if ( pElement->IsNamed( L"Message" ) )
		{
			strWarnings = pElement->GetValue();
		}
		else if ( pElement->IsNamed( L"Product" ) && m_nRequestType == stProductMatch )
		{
			bResult = ImportData( pElement );
		}
		else if ( pElement->IsNamed( L"Store" ) && m_nRequestType == stStoreMatch )
		{
			bResult = ImportData( pElement ) || bResult;
		}
	}

	strMessage = strStatus + L". " + strWarnings;
	Clear();

	if ( bFailed )
		return FALSE;
	else if ( m_nRequestType == stComparison )
		return TRUE;
	else
		return bResult;
}

BOOL CShareMonkeyData::ImportData(CXMLElement* pRoot)
{
	if ( pRoot == NULL )
		return FALSE;

	if ( m_nRequestType == stProductMatch )
	{
		if ( m_pSchema == NULL ) // Get the schema from the product Category
		{
			CXMLElement* pCategory = pRoot->GetElementByName( L"CategoryID" );
			int nCategory = 0;
			CString strCategory = pCategory->GetValue();
			if ( pCategory && strCategory.GetLength() && _stscanf( strCategory, L"%i", &nCategory ) == 1 )
			{
				switch ( nCategory )
				{
					case 1:
						m_pSchema = SchemaCache.Get( CSchema::uriAudio );
						break;
					case 2:
						m_pSchema = SchemaCache.Get( CSchema::uriBook ); // For documents
						break;
					case 3:
						m_pSchema = SchemaCache.Get( CSchema::uriArchive ); // For games
						break;
					case 4:
						m_pSchema = SchemaCache.Get( CSchema::uriApplication ); // For software
						break;
					case 5:
						m_pSchema = SchemaCache.Get( CSchema::uriVideo );
						break;
					default: break;
				}
			}
		}

		if ( m_pSchema == NULL )
			return FALSE;

		m_pRazaXML = m_pSchema->Instantiate( TRUE );

		CXMLElement* pXML = m_pRazaXML->AddElement( m_pSchema->m_sSingular );
		Setup( m_pSchema, TRUE );

		for ( POSITION pos = pRoot->GetElementIterator() ; pos ; )
		{
			CXMLElement* pElement = pRoot->GetNextElement( pos );
			if ( pElement->IsNamed( L"ProductName" ) )
				m_sProductName = pElement->GetValue();
			else if ( pElement->IsNamed( L"ProductID" ) )
				m_sProductID = pElement->GetValue();
			else if ( pElement->IsNamed( L"ProductDescription" ) )
				m_sDescription = pElement->GetValue();
			else if ( pElement->IsNamed( L"ProductURLs" ) )
			{
				CXMLElement* pBuyURL = pElement->GetElementByName( L"ProductBuyURL" );
				if ( pBuyURL )
					m_sBuyURL = pBuyURL->GetValue();
			}
			else if ( pElement->IsNamed( L"ProductImages" ) )
			{
				CXMLElement* pImage = pElement->GetElementByName( L"LargeImage" );
				if ( pImage == NULL )
					pImage = pElement->GetElementByName( L"MediumImage" );
				if ( pImage == NULL )
					pImage = pElement->GetElementByName( L"SmallImage" );
				if ( pImage )
				{
					CXMLElement* pImageURL = pImage->GetElementByName( L"ImageURL" );
					if ( pImageURL )
						m_sThumbnailURL = pImageURL->GetValue();
				}
			}
		}

		CXMLAttribute* pAttribute = new CXMLAttribute( NULL, L"title" );
		pAttribute->SetValue( m_sProductName );
		pXML->AddAttribute( pAttribute );

		pAttribute = new CXMLAttribute( NULL, L"description" );
		pAttribute->SetValue( m_sDescription );
		pXML->AddAttribute( pAttribute );

/*
		if ( m_sBuyURL.GetLength() )
		{
			pAttribute = new CXMLAttribute( NULL, L"distributerLink" );
			pAttribute->SetValue( m_sBuyURL );
			pXML->AddAttribute( pAttribute );
		}
*/
		Combine( pXML );

		delete m_pRazaXML;
		m_pRazaXML = NULL;
	}
	else if ( m_nRequestType == stStoreMatch )
	{
		CXMLElement* pStore = pRoot->GetElementByName( L"StoreName" );
		if ( pStore == NULL )
			return FALSE;
		CString strName = pStore->GetValue();
		if ( strName.IsEmpty() )
			return FALSE;

		CXMLElement* pPrice = pRoot->GetElementByName( L"Price" );
		if ( pPrice == NULL )
			return FALSE;

		CXMLElement* pValue = pPrice->GetElementByName( L"PriceFormatted" );
		if ( pValue == NULL )
			return FALSE;

		CString strValue = pValue->GetValue();
		if ( strValue.IsEmpty() )
			return FALSE;

		CXMLElement* pLink = pRoot->GetElementByName( L"StoreURL" );
		if ( pLink )
		{
			CString strLink;
			strLink.Format( L"%s|%s",
				(LPCTSTR)pLink->GetValue(), (LPCTSTR)strValue );

			while ( Find( strName ) )
				strName += '\x00A0';

			CMetaItem* pItem = Add( strName, strLink );
			pItem->m_bValueDefined = TRUE;
		}
	}

	return TRUE;
}
