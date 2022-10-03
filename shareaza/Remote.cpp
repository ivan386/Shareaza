//
// Remote.cpp
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
#include "Remote.h"

#include "Network.h"
#include "MatchObjects.h"
#include "QuerySearch.h"
#include "QueryHit.h"
#include "VendorCache.h"
#include "SchemaCache.h"
#include "Schema.h"
#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"
#include "DownloadGroups.h"
#include "DownloadGroup.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "Uploads.h"
#include "UploadQueues.h"
#include "UploadQueue.h"
#include "UploadFile.h"
#include "UploadTransfer.h"
#include "UploadTransferBT.h"
#include "Neighbours.h"
#include "G1Neighbour.h"
#include "G2Neighbour.h"
#include "EDNeighbour.h"
#include "EDPacket.h"
#include "GProfile.h"
#include "ShareazaURL.h"
#include "Skin.h"

#include "WndMain.h"
#include "WndSearch.h"
#include "CtrlDownloads.h"
#include "CtrlUploads.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CList<int> CRemote::m_pCookies;

/////////////////////////////////////////////////////////////////////////////
// CRemote construction

CRemote::CRemote(CConnection* pConnection)
{
	CTransfer::AttachTo( pConnection );
	m_mInput.pLimit = m_mOutput.pLimit = NULL;
	OnRead();
}

CRemote::~CRemote()
{
}

/////////////////////////////////////////////////////////////////////////////
// CRemote run event

BOOL CRemote::OnRun()
{
	DWORD tNow = GetTickCount();
	
	// 3 minute timeout
	if ( ( tNow - m_mOutput.tLast > 3 * 60 * 1000 ) || ( ! Network.IsConnected() ) )
	{
		Close();
		delete this;
		return FALSE;
	}
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CRemote dropped event

void CRemote::OnDropped()
{
	Close();
	delete this;
}

/////////////////////////////////////////////////////////////////////////////
// CRemote read event

BOOL CRemote::OnRead()
{
	if ( ! CTransfer::OnRead() ) return FALSE;
	
	if ( m_sHandshake.IsEmpty() )
	{
		if ( GetInputLength() > 4096 || ! Settings.Remote.Enable )
		{
			Close();
			return FALSE;
		}
		
		Read( m_sHandshake );
	}
	
	if ( ! m_sHandshake.IsEmpty() )
	{
		theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, _T("%s >> REMOTE REQUEST: %s"), (LPCTSTR)m_sAddress, (LPCTSTR)m_sHandshake );

		return ReadHeaders();
	}
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CRemote headers complete event

BOOL CRemote::OnHeadersComplete()
{
	if ( m_sHandshake.Find( _T("GET /") ) != 0 )
	{
		Close();
		delete this;
		return FALSE;
	}
	
	m_sHandshake = m_sHandshake.Mid( 4 ).SpanExcluding( _T(" \t") );
	
	const CString strPath = m_sHandshake.SpanExcluding( _T("?&") );
	
	m_sRedirect.Empty();
	m_sHeader.Empty();
	m_sResponse.Empty();
	m_pResponse.Clear();
	
	if ( strPath == _T("/") )
	{
		SendHTML( IDR_HTML_ABOUT );
	}
	else
		PageSwitch( strPath );

	if ( ! m_sRedirect.IsEmpty() )
	{
		Write( _P("HTTP/1.1 302 Found\r\n") );
		Write( _P("Content-Length: 0\r\n") );
		if ( ! m_sHeader.IsEmpty() ) Write( m_sHeader );
		Write( _P("Location: ") );
		Write( m_sRedirect );
		Write( _P("\r\n\r\n") );
		LogOutgoing();
	}
	else if ( ! m_sResponse.IsEmpty() )
	{
		CString strLength;
		Prepare();
		Output( _T("commonFooter") );
		int nBytes = WideCharToMultiByte( CP_UTF8, 0, m_sResponse, m_sResponse.GetLength(), NULL, 0, NULL, NULL );
		strLength.Format( _T("Content-Length: %i\r\n"), nBytes );
		Write( _P("HTTP/1.1 200 OK\r\n") );
		Write( _P("Content-Type: text/html; charset=UTF-8\r\n") );
		Write( strLength );
		if ( ! m_sHeader.IsEmpty() ) Write( m_sHeader );
		Write( _P("\r\n") );
		LogOutgoing();
		Write( m_sResponse, CP_UTF8 );
	}
	else if ( m_pResponse.m_nLength )
	{
		Write( _P("HTTP/1.1 200 OK\r\n") );
		CString strLength;
		strLength.Format( _T("Content-Length: %u\r\n"), m_pResponse.m_nLength );
		Write( strLength );
		if ( ! m_sHeader.IsEmpty() ) Write( m_sHeader );
		Write( _P("\r\n") );
		LogOutgoing();
		Write( &m_pResponse );
	}
	else
	{
		SendHTML( IDR_HTML_BROWSER );
	}

	m_sHandshake.Empty();

	ClearHeaders();

	OnWrite();
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CRemote get a query string key

CString CRemote::GetKey(LPCTSTR pszName)
{
	int nStart = 0;
	CString strPair = m_sHandshake.Tokenize( _T("&?"), nStart );
	
	while ( ! strPair.IsEmpty() )
	{
		CString strName = strPair.SpanExcluding( _T("=") );
		
		if ( strName.CompareNoCase( pszName ) == 0 )
		{
			strName = strPair.Mid( strName.GetLength() + 1 );
			return URLDecode( strName );
		}
		
		strPair = m_sHandshake.Tokenize( _T("&?"), nStart );
	}
	
	strPair.Empty();
	return strPair;
}

/////////////////////////////////////////////////////////////////////////////
// CRemote check access cookie helper

BOOL CRemote::CheckCookie()
{
	for ( INT_PTR nHeader = 0 ; nHeader < m_pHeaderName.GetSize() ; nHeader ++ )
	{
		if ( m_pHeaderName.GetAt( nHeader ).CompareNoCase( _T("Cookie") ) == 0 )
		{
			if ( LPCTSTR szPos = _tcsistr( m_pHeaderValue.GetAt( nHeader ), _T("shareazaremote=") ) )
			{
				int nCookie = 0;
				_stscanf( szPos + 15, _T("%i"), &nCookie );
				if ( m_pCookies.Find( nCookie ) != NULL ) return FALSE;
			}
		}
	}
	
	m_sRedirect = _T("/remote/");
	return TRUE;
}

// Determines what session ID is currently being used by the logged in user
// and removes it from the cookie list.
BOOL CRemote::RemoveCookie()
{
	for ( INT_PTR nHeader = 0 ; nHeader < m_pHeaderName.GetSize() ; nHeader ++ )
	{
		if ( m_pHeaderName.GetAt( nHeader ).CompareNoCase( _T("Cookie") ) == 0 )
		{
			if ( LPCTSTR szPos = _tcsistr( m_pHeaderValue.GetAt( nHeader ), _T("shareazaremote=") ) )
			{
				int nCookie = 0;
				_stscanf( szPos + 15, _T("%i"), &nCookie );
				POSITION pos = m_pCookies.Find( nCookie );
				if ( pos != NULL ) 
				{
					m_pCookies.RemoveAt( pos );
					return FALSE;
				}
			}
		}
	}
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CRemote prepare to output a HTML segment

void CRemote::Prepare(LPCTSTR pszPrefix)
{
	if ( pszPrefix == NULL )
	{
		m_pKeys.RemoveAll();
		if ( m_sResponse.IsEmpty() ) Output( _T("commonHeader") );
	}
	else
	{
		for ( POSITION pos = m_pKeys.GetStartPosition() ; pos != NULL ; )
		{
			CString strKey, strValue;
			m_pKeys.GetNextAssoc( pos, strKey, strValue );
			strKey.MakeLower();
			if ( strKey.Find( pszPrefix ) == 0 ) m_pKeys.RemoveKey( strKey );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRemote add a substitution key for the next HTML segment

void CRemote::Add(LPCTSTR pszKey, LPCTSTR pszValue)
{
	m_pKeys.SetAt( pszKey, pszValue );
}

/////////////////////////////////////////////////////////////////////////////
// CRemote output a HTML segment

void CRemote::Output(LPCTSTR pszName)
{
	CString strBody, strValue;
	CFile hFile;
	
	if ( _tcsstr( pszName, _T("..") ) || _tcschr( pszName, '/' ) ) return;
	strValue = Settings.General.Path + _T("\\Remote\\") + pszName + _T(".htm");
	if ( ! hFile.Open( strValue, CFile::modeRead ) ) return;
	
	int nBytes		= (int)hFile.GetLength();
	CAutoVectorPtr< BYTE > pBytes ( new BYTE[ nBytes ] );
	hFile.Read( pBytes, nBytes );
	hFile.Close();
	LPCSTR pBody = (LPCSTR)(BYTE*)pBytes;
	if ( nBytes > 3 && pBytes[0] == 0xEF && pBytes[1] == 0xBB && pBytes[2] == 0xBF )
	{
		// Skip BOM
		pBody += 3;
		nBytes -= 3;
	}
	strBody = UTF8Decode( pBody, nBytes );

	CList<BOOL> pDisplayStack;
	
	for ( BOOL bDisplay = TRUE ; ; )
	{
		int nStart = strBody.Find( _T("<%") );
		
		if ( nStart < 0 )
		{
			if ( bDisplay ) m_sResponse += strBody;
			break;
		}
		else if ( nStart >= 0 )
		{
			if ( bDisplay && nStart > 0 ) m_sResponse += strBody.Left( nStart );
			strBody = strBody.Mid( nStart + 2 );
		}
		
		int nEnd = strBody.Find( _T("%>") );
		if ( nEnd < 0 ) break;
		
		CString strKey = strBody.Left( nEnd );
		strBody = strBody.Mid( nEnd + 2 );
		
		strKey.TrimLeft();
		strKey.TrimRight();
		
		if ( strKey.IsEmpty() )
		{
		}
		else if ( strKey.GetAt( 0 ) == '=' && bDisplay )
		{
			strKey = strKey.Mid( 1 );
			strKey.Trim();
			if ( m_pKeys.Lookup( strKey, strValue ) ) m_sResponse += strValue;
		}
		else if ( strKey.GetAt( 0 ) == '?' )
		{
			strKey = strKey.Mid( 1 );
			strKey.Trim();
			
			if ( strKey.IsEmpty() )
			{
				if ( ! pDisplayStack.IsEmpty() ) bDisplay = pDisplayStack.RemoveTail();
			}
			else
			{
				if ( strKey.GetAt( 0 ) == '!' )
				{
					strKey = strKey.Mid( 1 );
					strKey.Trim();
					if ( ! m_pKeys.Lookup( strKey, strValue ) ) strValue.Empty();
					pDisplayStack.AddTail( bDisplay );
					bDisplay = bDisplay && strValue.IsEmpty();
				}
				else
				{
					if ( ! m_pKeys.Lookup( strKey, strValue ) ) strValue.Empty();
					pDisplayStack.AddTail( bDisplay );
					bDisplay = bDisplay && ! strValue.IsEmpty();
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page switch

void CRemote::PageSwitch(const CString& strPath)
{
	if ( strPath.CompareNoCase( _T("/remote") ) == 0 )
	{
		m_sRedirect = _T("/remote/");
	}
	else if ( strPath.CompareNoCase( _T("/remote/") ) == 0 )
	{
		PageLogin();
	}
	else if ( strPath.CompareNoCase( _T("/remote/logout") ) == 0 )
	{
		PageLogout();
	}
	else if ( strPath.CompareNoCase( _T("/remote/home") ) == 0 )
	{
		PageHome();
	}
	else if ( strPath.CompareNoCase( _T("/remote/search") ) == 0 )
	{
		PageSearch();
	}
	else if ( strPath.CompareNoCase( _T("/remote/newsearch") ) == 0 )
	{
		PageNewSearch();
	}
	else if ( strPath.CompareNoCase( _T("/remote/downloads") ) == 0 )
	{
		PageDownloads();
	}
	else if ( strPath.CompareNoCase( _T("/remote/newdownload") ) == 0 )
	{
		PageNewDownload();
	}
	else if ( strPath.CompareNoCase( _T("/remote/uploads") ) == 0 )
	{
		PageUploads();
	}
	else if ( strPath.CompareNoCase( _T("/remote/network") ) == 0 )
	{
		PageNetwork();
	}
	else if ( strPath.Left( 15 ).CompareNoCase( _T("/remote/images/") ) == 0 )
	{
		PageImage( strPath );
	}
	else
	{
		PageBanner( strPath );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page : login

void CRemote::PageLogin()
{
	CString strPassword = GetKey( _T("password") );
	
	if ( ! strPassword.IsEmpty() )
	{
		CSHA pSHA1;
		pSHA1.Add( (LPCTSTR)strPassword, strPassword.GetLength() * sizeof(TCHAR) );
		pSHA1.Finish();
        Hashes::Sha1Hash tmp;
        pSHA1.GetHash( &tmp[ 0 ] );
		tmp.validate();
        strPassword = tmp.toString();
	}
	
	if ( GetKey( _T("username") ) == Settings.Remote.Username &&
					  strPassword == Settings.Remote.Password &&
		 Settings.Remote.Username.GetLength() > 0 &&
		 Settings.Remote.Password.GetLength() > 0 )
	{
		__int32 nCookie = GetRandomNum( 0i32, _I32_MAX );
		m_pCookies.AddTail( nCookie );
		m_sHeader.Format( _T("Set-Cookie: ShareazaRemote=%i; path=/remote\r\n"), nCookie );
		m_sRedirect.Format( _T("/remote/home?%i"), GetRandomNum( 0i32, _I32_MAX ) );
	}
	else
	{
		Prepare();
		if ( GetKey( _T("submit") ).GetLength() > 0 ) Add( _T("failure"), _T("true") );
		Output( _T("login") );
	}
}

void CRemote::PageLogout()
{
	// Clear the server-side session cookie
	RemoveCookie();
	// Clear the client-side session cookie
	m_sHeader.Format( _T("Set-Cookie: ShareazaRemote=0; path=/remote; Max-Age=0\r\n") );
	m_sRedirect.Format( _T("/remote/") );
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page : home

void CRemote::PageHome()
{
	if ( CheckCookie() ) return;
	
	Prepare();
	Output( _T("home") );
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page : search

void CRemote::PageSearch()
{
	if ( CheckCookie() ) return;
	
	CSingleLock pLock( &theApp.m_pSection );
	if ( ! pLock.Lock( 1000 ) )
		return;
	CMainWnd* pMainWnd = static_cast< CMainWnd* >( theApp.m_pMainWnd );
	if ( pMainWnd == NULL || ! pMainWnd->IsKindOf( RUNTIME_CLASS(CMainWnd) ) )
		return;
	
	INT_PTR nSearchID = NULL;
	INT_PTR nCloseID = NULL;
	CSearchWnd* pSearchWnd = NULL;
	CString str;
	
	_stscanf( GetKey( _T("id") ), _T("%Ii"), &nSearchID );
	_stscanf( GetKey( _T("close") ), _T("%Ii"), &nCloseID );
	
	Prepare();
	Output( _T("searchHeader") );
	
	for ( CSearchWnd* pFindWnd = NULL ; ( pFindWnd = static_cast< CSearchWnd* >( pMainWnd->m_pWindows.Find( RUNTIME_CLASS(CSearchWnd), pFindWnd ) ) ) != NULL ; )
	{
		Prepare();
		INT_PTR nFindWnd = reinterpret_cast< INT_PTR >( pFindWnd );
		if ( nCloseID == nFindWnd )
		{
			pFindWnd->PostMessage( WM_CLOSE );
			continue;
		}
		else if ( nSearchID == nFindWnd )
		{
			pSearchWnd = pFindWnd;
			Add( _T("search_selected"), _T("true") );
		}
		
		str.Format( _T("%Ii"), nFindWnd );
		Add( _T("search_id"), str );
		str = pFindWnd->GetCaption();
		if ( str.Find( _T("Search : ") ) == 0 ) str = str.Mid( 9 ).SpanExcluding( _T("[") );
		Add( _T("search_caption"), str );
		Output( _T("searchTab") );
	}
	
	if ( pSearchWnd == NULL )
	{
		str.Empty();

		for ( POSITION pos = SchemaCache.GetIterator() ; pos != NULL ; )
		{
			CSchemaPtr pSchema = SchemaCache.GetNext( pos );
			if ( ! pSchema->m_bPrivate && pSchema->m_nType == CSchema::stFile )
			{
				str += _T("<option value=\"") + pSchema->GetURI();
				str += _T("\">") + pSchema->m_sTitle;
				str += _T("</option>\r\n");
			}
		}
		
		Prepare();
		Add( _T("schema_option_list"), str );
		Output( _T("searchNew") );
		Output( _T("searchFooter") );
		return;
	}
	
	if ( ! GetKey( _T("stop") ).IsEmpty() )
	{
		pSearchWnd->PostMessage( WM_COMMAND, ID_SEARCH_STOP );
		Sleep( 500 );
	}

	CLockedMatchList pMatches( pSearchWnd->GetMatches() );

	str = GetKey( _T("sort") );
	if ( ! str.IsEmpty() )
	{
		int nColumn = 0;
		_stscanf( str, _T("%i"), &nColumn );
		
		if ( pMatches->m_nSortColumn == nColumn )
		{
			if ( pMatches->m_bSortDir == 1 )
			{
				pMatches->SetSortColumn( nColumn, TRUE );
			}
			else
			{
				pMatches->SetSortColumn( nColumn, FALSE );
			}
		}
		else
		{
			pMatches->SetSortColumn( nColumn, TRUE );
		}
		
		pSearchWnd->PostMessage( WM_TIMER, 7 );
	}
	
	str = GetKey( _T("expcol") );
	if ( ! str.IsEmpty() )
	{
		CMatchFile** pLoop = pMatches->m_pFiles;
		for ( DWORD nCount = 0 ; nCount < pMatches->m_nFiles ; nCount++, pLoop++ )
		{
			if ( (*pLoop)->GetURN() == str )
			{
				(*pLoop)->Expand( GetKey( _T("collapse") ).IsEmpty() );
				pSearchWnd->PostMessage( WM_TIMER, 7 );
				break;
			}
		}
	}
	
	str = GetKey( _T("download") );
	if ( ! str.IsEmpty() )
	{
		CMatchFile** pLoop = pMatches->m_pFiles;
		for ( DWORD nCount = 0 ; nCount < pMatches->m_nFiles ; nCount++, pLoop++ )
		{
			if ( (*pLoop)->GetURN() == str )
			{
				Downloads.Add( *pLoop );
				pSearchWnd->PostMessage( WM_TIMER, 7 );
				m_sResponse.Empty();
				m_sRedirect = _T("downloads?group_reveal=all");
				return;
			}
		}
	}
	
	if ( ! GetKey( _T("setfilter") ).IsEmpty() )
	{
		pMatches->m_sFilter = GetKey( _T("filter") );
		pMatches->Filter();
		pSearchWnd->PostMessage( WM_TIMER, 7 );
	}
	
	Prepare();
	str.Format( _T("%Ii"), nSearchID );
	Add( _T("search_id"), str );
	str.Format( _T("%i"), GetRandomNum( 0i32, _I32_MAX ) );
	Add( _T("random"), str );
	if ( ! pSearchWnd->IsPaused() ) Add( _T("searching"), _T("true") );
	Add( _T("search_filter"), pMatches->m_sFilter );
	Output( _T("searchTop") );
	
	PageSearchHeaderColumn( MATCH_COL_NAME, Skin.GetHeaderTranslation( L"CMatchCtrl", L"File" ), L"left" );
	PageSearchHeaderColumn( MATCH_COL_SIZE, Skin.GetHeaderTranslation( L"CMatchCtrl", L"Size" ), L"center" );
	PageSearchHeaderColumn( MATCH_COL_RATING, Skin.GetHeaderTranslation( L"CMatchCtrl", L"Rating" ), L"center" );
	PageSearchHeaderColumn( MATCH_COL_STATUS, Skin.GetHeaderTranslation( L"CMatchCtrl", L"Status" ), L"center" );
	PageSearchHeaderColumn( MATCH_COL_COUNT, Skin.GetHeaderTranslation( L"CMatchCtrl", L"Host/Count" ), L"center" );
	PageSearchHeaderColumn( MATCH_COL_SPEED, Skin.GetHeaderTranslation( L"CMatchCtrl", L"Speed" ), L"center" );
	PageSearchHeaderColumn( MATCH_COL_CLIENT, Skin.GetHeaderTranslation( L"CMatchCtrl", L"Client" ), L"center" );
	
	Output( _T("searchMiddle") );
	
	CMatchFile** pLoop = pMatches->m_pFiles;
	
	for ( DWORD nCount = 0 ; nCount < pMatches->m_nFiles ; nCount++, pLoop++ )
	{
		CMatchFile* pFile = *pLoop;
		if ( pFile->GetFilteredCount() == 0 ) continue;
		
		Add( _T("row_urn"), pFile->GetURN() );
		Add( _T("row_filename"), pFile->m_sName );
		if ( pFile->GetFilteredCount() > 1 )
		{
			if ( pFile->m_bExpanded )
				Add( _T("row_expanded"), _T("true") );
			else
				Add( _T("row_collapsed"), _T("true") );
		}
		else
		{
			Add( _T("row_single"), _T("true") );
		}
		Output( _T("searchRowStart") );
		
		PageSearchRowColumn( MATCH_COL_SIZE, pFile, Settings.SmartVolume( pFile->m_nSize ) );
		
		str.Empty();
		for ( INT_PTR nStar = pFile->m_nRating / max( 1, pFile->m_nRated ) ; nStar > 1 ; nStar -- ) str += _T("*");
		PageSearchRowColumn( MATCH_COL_RATING, pFile, str );
		
		str.Empty();
		if ( pFile->m_bBusy == TRI_TRUE ) str += 'B'; else str += '-';
		if ( pFile->m_bPush == TRI_TRUE ) str += 'F'; else str += '-';
		if ( pFile->m_bStable == TRI_FALSE ) str += 'U'; else str += '-';
		PageSearchRowColumn( MATCH_COL_STATUS, pFile, str );
		
		if ( pFile->GetFilteredCount() > 1 )
		{
			str.Format( _T("(%u sources)"), pFile->GetFilteredCount() );
			PageSearchRowColumn( MATCH_COL_COUNT, pFile, str );
		}
		else
		{
			PageSearchRowColumn( MATCH_COL_COUNT, pFile,
				CString( inet_ntoa( pFile->GetBestAddress() ) ) );
		}
		
		PageSearchRowColumn( MATCH_COL_SPEED, pFile, pFile->m_sSpeed );
		PageSearchRowColumn( MATCH_COL_CLIENT, pFile, pFile->GetFilteredCount() == 1 ? pFile->GetBestVendorName() : _T("") );
		
		Output( _T("searchRowEnd") );
		Prepare( _T("column_") );
		Prepare( _T("row_") );
		
		if ( pFile->m_bExpanded )
		{
			for ( CQueryHit* pHit = pFile->GetHits() ; pHit != NULL ; pHit = pHit->m_pNext )
			{
				if ( ! pHit->m_bFiltered ) continue;
				
				Add( _T("row_urn"), pFile->GetURN() );
				Add( _T("row_filename"), pHit->m_sName );
				Add( _T("row_source"), _T("true") );
				Output( _T("searchRowStart") );
				
				PageSearchRowColumn( MATCH_COL_SIZE, pFile, Settings.SmartVolume( pHit->m_nSize ) );
				str.Empty();
				for ( int nStar = pHit->m_nRating ; nStar > 1 ; nStar -- ) str += _T("*");
				PageSearchRowColumn( MATCH_COL_RATING, pFile, str );
				str.Empty();
				if ( pHit->m_bBusy == TRI_TRUE ) str += 'B'; else str += '-';
				if ( pHit->m_bPush == TRI_TRUE ) str += 'F'; else str += '-';
				if ( pHit->m_bStable == TRI_FALSE ) str += 'U'; else str += '-';
				PageSearchRowColumn( MATCH_COL_STATUS, pFile, str );
				PageSearchRowColumn( MATCH_COL_COUNT, pFile,
					CString( inet_ntoa( pHit->m_pAddress ) ) );
				PageSearchRowColumn( MATCH_COL_SPEED, pFile, pHit->m_sSpeed );
				PageSearchRowColumn( MATCH_COL_CLIENT, pFile, pHit->m_pVendor->m_sName );
				
				Output( _T("searchRowEnd") );
				Prepare( _T("column_") );
				Prepare( _T("row_") );
			}
		}
	}
	
	Output( _T("searchBottom") );
	Prepare();
	Output( _T("searchFooter") );
}

void CRemote::PageSearchHeaderColumn(int nColumnID, LPCTSTR pszCaption, LPCTSTR pszAlign)
{
	CString str;
	str.Format( _T("%i"), nColumnID );
	Add( _T("column_id"), str );
	Add( _T("column_align"), pszAlign );
	Add( _T("column_caption"), pszCaption );
	Output( _T("searchColumn") );
	Prepare( _T("column_") );
}

void CRemote::PageSearchRowColumn(int nColumnID, CMatchFile* pFile, LPCTSTR pszValue, LPCTSTR pszAlign)
{
	CString str;
	str.Format( _T("%i"), nColumnID );
	Add( _T("column_id"), str );
	Add( _T("column_align"), pszAlign );
	Add( _T("row_urn"), pFile->GetURN() );
	Add( _T("row_value"), pszValue );
	Output( _T("searchRowColumn") );
	Prepare( _T("column_") );
	Prepare( _T("row_") );
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page : newsearch

void CRemote::PageNewSearch()
{
	CString strURI;
	if ( CheckCookie() ) return;
	
	CSingleLock pLock( &theApp.m_pSection );
	if ( ! pLock.Lock( 1000 ) ) return;
	CMainWnd* pMainWnd = (CMainWnd*)theApp.m_pMainWnd;
	if ( pMainWnd == NULL || ! pMainWnd->IsKindOf( RUNTIME_CLASS(CMainWnd) ) ) return;
	
	CString strSearch	= GetKey( _T("search") );
	CString strSchema	= GetKey( _T("schema") );
	
	if ( strSearch.IsEmpty() || ( ! strSchema.IsEmpty() && SchemaCache.Get( strSchema ) == NULL ) )
	{
		m_sRedirect = _T("home");
		return;
	}
	
	CQuerySearchPtr pSearch	= new CQuerySearch();
	pSearch->m_sSearch		= strSearch;
	pSearch->m_pSchema		= SchemaCache.Get( strSchema );

	if ( pSearch->m_pSchema != NULL ) strURI = pSearch->m_pSchema->GetURI();
	
	Settings.Search.LastSchemaURI = strURI;
	
	pMainWnd->PostMessage( WM_OPENSEARCH, (WPARAM)pSearch.Detach() );
	pLock.Unlock();
	Sleep( 500 );
	
	m_sRedirect = _T("search");
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page : downloads

void CRemote::PageDownloads()
{
	if ( CheckCookie() ) return;
	
	CSingleLock pLock( &DownloadGroups.m_pSection, TRUE );
	CString str;
	
	Prepare();
	str.Format( _T("%i"), GetRandomNum( 0i32, _I32_MAX ) );
	Add( _T("random"), str );
	Output( _T("downloadsHeader") );
	
	BOOL bExclusive = ! GetKey( _T("group_exclusive") ).IsEmpty();
	BOOL bReveal = ! GetKey( _T("group_reveal") ).IsEmpty();
	
	for ( POSITION posGroup = DownloadGroups.GetIterator() ; posGroup != NULL ; )
	{
		CDownloadGroup* pGroup = DownloadGroups.GetNext( posGroup );
		
		CString group_id;
		group_id.Format( _T("%p"), pGroup );
		Add( _T("group_id"), group_id );
		
		if ( bExclusive )
		{
			pGroup->m_bRemoteSelected = ( GetKey( _T("group_exclusive") ) == group_id );
		}
		else
		{
			if ( bReveal )
				pGroup->m_bRemoteSelected = TRUE;
			else if ( GetKey( _T("group_select") ) == group_id )
				pGroup->m_bRemoteSelected = TRUE;
			else if ( GetKey( _T("group_deselect") ) == group_id )
				pGroup->m_bRemoteSelected = FALSE;
		}
		
		Add( _T("group_caption"), pGroup->m_sName );
		if ( pGroup->m_bRemoteSelected ) Add( _T("group_selected"), _T("true") );
		Output( _T("downloadsTab") );
		Prepare( _T("group_") );
	}
	
	if ( ! GetKey( _T("filter_set") ).IsEmpty() )
	{
		Settings.Downloads.FilterMask &= ~( DLF_ACTIVE | DLF_QUEUED | DLF_SOURCES | DLF_PAUSED );
		if ( GetKey( _T("filter_active") ) == _T("1") ) Settings.Downloads.FilterMask |= DLF_ACTIVE;
		if ( GetKey( _T("filter_queued") ) == _T("1") ) Settings.Downloads.FilterMask |= DLF_QUEUED;
		if ( GetKey( _T("filter_sources") ) == _T("1") ) Settings.Downloads.FilterMask |= DLF_SOURCES;
		if ( GetKey( _T("filter_paused") ) == _T("1") ) Settings.Downloads.FilterMask |= DLF_PAUSED;
		Settings.Downloads.ShowSources = ( GetKey( _T("filter_show_all") ) == _T("1") );
	}
	
	Add( _T("filter_active"), ( Settings.Downloads.FilterMask & DLF_ACTIVE ) ? _T("checked=\"checked\"") : _T("") );
	Add( _T("filter_queued"), ( Settings.Downloads.FilterMask & DLF_QUEUED ) ? _T("checked=\"checked\"") : _T("") );
	Add( _T("filter_sources"), ( Settings.Downloads.FilterMask & DLF_SOURCES ) ? _T("checked=\"checked\"") : _T("") );
	Add( _T("filter_paused"), ( Settings.Downloads.FilterMask & DLF_PAUSED ) ? _T("checked=\"checked\"") : _T("") );
	Add( _T("filter_show_all"), Settings.Downloads.ShowSources ? _T("checked=\"checked\"") : _T("") );
	Output( _T("downloadsTop") );
	
	for ( POSITION posDownload = Downloads.GetIterator() ; posDownload != NULL ; )
	{
		CDownload* pDownload = Downloads.GetNext( posDownload );

		CString download_id;
		download_id.Format( _T("%p"), pDownload );
		
		if ( GetKey( _T("modify_id") ) == download_id )
		{
			const CString action = GetKey( _T("modify_action") );
			
			if ( action.CompareNoCase( _T("expand") ) == 0 && CDownloadsCtrl::IsExpandable( pDownload ) )
			{
				pDownload->m_bExpanded = TRUE;
			}
			else if ( action.CompareNoCase( _T("collapse") ) == 0 && CDownloadsCtrl::IsExpandable( pDownload ) )
			{
				pDownload->m_bExpanded = FALSE;
			}
			else if ( action.CompareNoCase( _T("resume") ) == 0 )
			{
				pDownload->Resume();
			}
			else if ( action.CompareNoCase( _T("pause") ) == 0 )
			{
				if ( ! pDownload->IsPaused() && ! pDownload->IsTasking() )
					pDownload->Pause();
			}
			else if ( action.CompareNoCase( _T("cancel") ) == 0 )
			{
				if ( ! pDownload->IsTasking() )
					pDownload->Remove();
				continue;
			}
			else if ( action.CompareNoCase( _T("clear") ) == 0 )
			{
				if ( pDownload->IsCompleted() && ! pDownload->IsPreviewVisible() )
				{
					pDownload->Remove();
					continue;
				}
			}
			// roo_koo_too improvement
			else if ( action.CompareNoCase( _T("more_sources") ) == 0 )
			{ 
				pDownload->FindMoreSources();
			}
		}
		
		if ( CDownloadsCtrl::IsFiltered( pDownload ) ) continue;
		
		CDownloadGroup* pGroup = NULL;
		
		for ( POSITION posGroup = DownloadGroups.GetIterator() ; posGroup != NULL ; )
		{
			pGroup = DownloadGroups.GetNext( posGroup );
			if ( pGroup->m_bRemoteSelected && pGroup->Contains( pDownload ) ) break;
			pGroup = NULL;
		}
		
		if ( pGroup == NULL ) continue;
		
		Add( _T("download_id"), download_id );
		Add( _T("download_filename"), pDownload->GetDisplayName() );
		Add( _T("download_size"), ( pDownload->m_nSize == SIZE_UNKNOWN ) ?
			LoadString( IDS_STATUS_UNKNOWN ) : Settings.SmartVolume( pDownload->m_nSize ) );
		int nProgress = int( pDownload->GetProgress() );
		str.Format( _T("%i"), nProgress );
		Add( _T("download_percent"), str );
		str.Format( _T("%i"), 100 - nProgress );
		Add( _T("download_percent_inverse"), str );
		Add( _T("download_speed"), Settings.SmartSpeed( pDownload->GetMeasuredSpeed() ) );
		if ( CDownloadsCtrl::IsExpandable( pDownload ) )
		{
			if ( pDownload->m_bExpanded ) Add( _T("download_is_expanded"), _T("true") );
			else Add( _T("download_is_collapsed"), _T("true") );
		}

		if ( pDownload->IsCompleted() )
		{
			Add( _T("download_is_complete"), _T("true") );
		}
		else if ( pDownload->IsPaused() )
		{
			Add( _T("download_is_paused"), _T("true") );
		}

		Add( _T("download_status"), pDownload->GetDownloadStatus() );
		Add( _T("download_sources"), pDownload->GetDownloadSources() );
		Output( _T("downloadsDownload") );
		
		if ( pDownload->m_bExpanded && CDownloadsCtrl::IsExpandable( pDownload ) )
		{
			for ( POSITION posSource = pDownload->GetIterator(); posSource ; )
			{
				CDownloadSource* pSource = pDownload->GetNext( posSource );

				ASSERT( pSource->m_pDownload == pDownload );

				CString source_id;
				source_id.Format( _T("%p"), pSource );

				if ( GetKey( _T("modify_id") ) == source_id )
				{
					const CString modify_action = GetKey( _T("modify_action") );

					if ( modify_action.CompareNoCase( _T("access") ) == 0 )
					{
						// Only create a new Transfer if there isn't already one
						if ( pSource->IsIdle()
							&& pSource->m_nProtocol != PROTOCOL_ED2K )
						{
							if ( pDownload->IsPaused() )
								pDownload->Resume();

							pDownload->Resume();

							if ( pSource->m_bPushOnly )
								pSource->PushRequest();
							else
							{
								CDownloadTransfer* pTransfer = pSource->CreateTransfer();
								if ( pTransfer )
									pTransfer->Initiate();
							}
						}
					}
					else if ( modify_action.CompareNoCase( _T("forget") ) == 0 )
					{
						pSource->Remove( TRUE, TRUE );
						continue;
					}
				}
				
				if ( Settings.Downloads.ShowSources || pSource->IsConnected() )
				{
					Add( _T("source_id"), source_id );
					Add( _T("source_agent"), pSource->m_sServer );
					Add( _T("source_nick"), pSource->m_sNick );
					
					if ( ! pSource->IsIdle() )
					{
						Add( _T("source_status"), pSource->GetState( FALSE ) );
						Add( _T("source_volume"), Settings.SmartVolume( pSource->GetDownloaded() ) );
						DWORD nSpeed = pSource->GetMeasuredSpeed();
						if ( nSpeed )
							Add( _T("source_speed"), Settings.SmartSpeed( nSpeed ) );
						Add( _T("source_address"), pSource->GetAddress() );
						Add( _T("source_caption"), pSource->GetAddress() + _T(" - ") + pSource->m_sNick );
					}
					else
					{
						Add( _T("source_address"), CString( inet_ntoa( pSource->m_pAddress ) ) );
						Add( _T("source_caption"), CString( inet_ntoa( pSource->m_pAddress ) ) + _T(" - ") + pSource->m_sNick );
						
						if ( pSource->m_tAttempt > 0 )
						{
							DWORD tNow = GetTickCount();
							
							if ( pSource->m_tAttempt >= tNow )
							{
								tNow = ( pSource->m_tAttempt - tNow ) / 1000;
								CString source_status;
								source_status.Format( _T("%.2u:%.2u"), tNow / 60, tNow % 60 );
								Add( _T("source_status"), source_status );
							}
						}
					}
					
					Output( _T("downloadsSource") );
					Prepare( _T("source_") );
				}
			}
		}
		
		Prepare( _T("download_") );
	}
	
	Output( _T("downloadsBottom") );
	Output( _T("downloadsFooter") );
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page : newDownload

void CRemote::PageNewDownload()
{
	if ( CheckCookie() ) return;
	
	CShareazaURL pURI;
	if ( pURI.Parse( GetKey( _T("uri") ) ) ) Downloads.Add( pURI );
	
	m_sRedirect = _T("downloads?group_reveal=all");
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page : uploads

void CRemote::PageUploads()
{
	if ( CheckCookie() ) return;

	CSingleLock pLock( &UploadQueues.m_pSection, FALSE );
	if ( ! pLock.Lock( 1000 ) )
		return;
	
	Prepare();

	CString random;
	random.Format( _T("%i"), GetRandomNum( 0i32, _I32_MAX ) );
	Add( _T("random"), random );

	Output( _T("uploadsHeader") );
	
	for ( POSITION posQueue = CUploadsCtrl::GetQueueIterator() ; posQueue != NULL ; )
	{
		CUploadQueue* pQueue = CUploadsCtrl::GetNextQueue( posQueue );
		
		CString queue_id;
		queue_id.Format( _T("%p"), pQueue );
		
		if ( GetKey( _T("queue_expand") ) == queue_id ) pQueue->m_bExpanded = TRUE;
		else if ( GetKey( _T("queue_collapse") ) == queue_id ) pQueue->m_bExpanded = FALSE;
		
		POSITION posFile = CUploadsCtrl::GetFileIterator( pQueue );
		if ( posFile == NULL ) continue;
		
		Prepare();
		Add( _T("queue_id"), queue_id );
		Add( _T("queue_caption"), pQueue->m_sName );
		if ( pQueue->m_bExpanded ) Add( _T("queue_expanded"), _T("true") );
		
		if ( pQueue != UploadQueues.m_pTorrentQueue && pQueue != UploadQueues.m_pHistoryQueue )
		{
			CString str;
			str.Format( _T("%u"), pQueue->GetTransferCount() );
			Add( _T("queue_transfers"), str );
			str.Format( _T("%u"), pQueue->GetQueuedCount() );
			Add( _T("queue_queued"), str );
			Add( _T("queue_bandwidth"), Settings.SmartSpeed( pQueue->GetMeasuredSpeed() ) );
		}
		
		Output( _T("uploadsQueueStart") );
		
		if ( pQueue->m_bExpanded )
		{
			while ( posFile != NULL )
			{
				int nPosition;
				CUploadFile* pFile = CUploadsCtrl::GetNextFile( pQueue, posFile, &nPosition );
				if ( pFile == NULL ) continue;
				CUploadTransfer* pTransfer = pFile->GetActive();
				
				CString file_id;
				file_id.Format( _T("%p"), pFile );
				
				if ( GetKey( _T("drop") ) == file_id )
				{
					pFile->Remove();
					continue;
				}
				
				Add( _T("file_id"), file_id );
				Add( _T("file_filename"), pFile->m_sName );
				Add( _T("file_size"), Settings.SmartVolume( pFile->m_nSize ) );
				
				if ( pTransfer != NULL )
				{
					Add( _T("file_address"), pTransfer->m_sAddress );
					Add( _T("file_nick"), pTransfer->m_sRemoteNick );
					Add( _T("file_user"), pTransfer->m_sAddress + _T(" - ") + pTransfer->m_sRemoteNick );
					Add( _T("file_agent"), pTransfer->m_sUserAgent );
				}
				
				CString str;
				if ( pTransfer == NULL || pTransfer->m_nState == upsNull )
				{
					LoadString( str, IDS_STATUS_COMPLETED );
				}
				else if ( pTransfer->m_nProtocol == PROTOCOL_BT )
				{
					CUploadTransferBT* pBT = (CUploadTransferBT*)pTransfer;
					
					if ( ! pBT->m_bInterested )
						LoadString( str, IDS_STATUS_UNINTERESTED );
					else if ( pBT->m_bChoked )
						LoadString( str, IDS_STATUS_CHOKED );
					else
					{
						DWORD nSpeed = pTransfer->GetMeasuredSpeed();
						if ( nSpeed )
							str = Settings.SmartSpeed( nSpeed );
					}
				}
				else if ( nPosition > 0 )
				{
					LoadString( str, IDS_STATUS_Q );
					str.Format( _T("%s %i"), (LPCTSTR)str, nPosition );
				}
				else
				{
					DWORD nSpeed = pTransfer->GetMeasuredSpeed();
					if ( nSpeed )
						str = Settings.SmartSpeed( nSpeed );
					else
						LoadString( str, IDS_STATUS_NEXT );
				}
				Add( _T("file_speed"), str );
				Add( _T("file_status"), str );
				
				Output( _T("uploadsFile") );
				Prepare( _T("file_") );
			}
		}
		
		Output( _T("uploadsQueueEnd") );
		Prepare( _T("queue_") );
	}
	
	Prepare();
	Output( _T("uploadsFooter") );
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page : network

void CRemote::PageNetwork()
{
	if ( CheckCookie() ) return;
	
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 1000 ) ) return;
	
	DWORD nNeighbourID = 0;
	_stscanf( GetKey( _T("drop") ), _T("%lu"), &nNeighbourID );
	
	if ( nNeighbourID != 0 )
	{
		if ( CNeighbour* pNeighbour = Neighbours.Get( nNeighbourID ) )
		{
			pNeighbour->Close( IDS_CONNECTION_CLOSED );
		}
	}
	
	CString str;
	
	Prepare();
	str.Format( _T("%i"), GetRandomNum( 0i32, _I32_MAX ) );
	Add( _T("random"), str );
	Output( _T("networkHeader") );
	
	PageNetworkNetwork( PROTOCOL_G2, &Settings.Gnutella2.EnableToday, protocolNames[ PROTOCOL_G2 ] );
	PageNetworkNetwork( PROTOCOL_G1, &Settings.Gnutella1.EnableToday, protocolNames[ PROTOCOL_G1 ] );
	PageNetworkNetwork( PROTOCOL_ED2K, &Settings.eDonkey.EnableToday, protocolNames[ PROTOCOL_ED2K ] );
	PageNetworkNetwork( PROTOCOL_BT, &Settings.BitTorrent.EnableToday, protocolNames[ PROTOCOL_BT ] );
	PageNetworkNetwork( PROTOCOL_DC, &Settings.DC.EnableToday, protocolNames[ PROTOCOL_DC ] );
	
	Output( _T("networkFooter") );
}

void CRemote::PageNetworkNetwork(int nID, bool* pbConnect, LPCTSTR pszName)
{
	CString str;
	
	str.Format( _T("%i"), nID );
	
	if ( GetKey( _T("connect") ) == str )
	{
		*pbConnect = TRUE;
		Network.Connect( TRUE );
	}
	else if ( GetKey( _T("disconnect") ) == str )
	{
		*pbConnect = FALSE;
		
		for ( POSITION pos = Neighbours.GetIterator() ; pos != NULL ; )
		{
			CNeighbour* pNeighbour = Neighbours.GetNext( pos );
			if ( pNeighbour->m_nProtocol == PROTOCOL_NULL ||
				 pNeighbour->m_nProtocol == nID )
				 pNeighbour->Close( IDS_CONNECTION_CLOSED );
		}
	}
	
	Add( _T("network_id"), str );
	Add( _T("network_caption"), pszName );
	if ( *pbConnect ) Add( _T("network_connected"), _T("true") );
	Output( _T("networkNetStart") );
	
	for ( POSITION pos = Neighbours.GetIterator() ; pos != NULL ; )
	{
		CNeighbour* pNeighbour = Neighbours.GetNext( pos );
		if ( pNeighbour->m_nProtocol != nID ) continue;
		pNeighbour->Measure();
		
		str.Format( _T("%p"), pNeighbour );
		Add( _T("row_id"), str );
		Add( _T("row_address"), pNeighbour->m_sAddress );
		Add( _T("row_mode"), Neighbours.GetName( pNeighbour ) );
		Add( _T("row_agent"), Neighbours.GetAgent( pNeighbour ) );
		Add( _T("row_nick"), Neighbours.GetNick( pNeighbour ) );
		str.Format( _T("%u -/- %u"), pNeighbour->m_nInputCount, pNeighbour->m_nOutputCount );
		Add( _T("row_packets"), str );
		str.Format( _T("%s -/- %s"),
			(LPCTSTR)Settings.SmartSpeed( pNeighbour->m_mInput.nMeasure ),
			(LPCTSTR)Settings.SmartSpeed( pNeighbour->m_mOutput.nMeasure ) );
		Add( _T("row_bandwidth"), str );
		str.Format( _T("%s -/- %s"),
			(LPCTSTR)Settings.SmartVolume( pNeighbour->m_mInput.nTotal ),
			(LPCTSTR)Settings.SmartVolume( pNeighbour->m_mOutput.nTotal ) );
		Add( _T("row_total"), str );
		
		switch ( pNeighbour->m_nState )
		{
		case nrsConnecting:
			LoadString( str, IDS_NEIGHBOUR_CONNECTING );
			break;
		case nrsHandshake1:
		case nrsHandshake2:
		case nrsHandshake3:
			LoadString( str, IDS_NEIGHBOUR_HANDSHAKING );
			break;
		case nrsRejected:
			LoadString( str, IDS_NEIGHBOUR_REJECTED );
			break;
		case nrsClosing:
			LoadString( str, IDS_NEIGHBOUR_CLOSING );
			break;
		case nrsConnected:
			{
				DWORD tNow = ( GetTickCount() - pNeighbour->m_tConnected ) / 1000;
				if ( tNow > 86400 )
					str.Format( _T("%u:%.2u:%.2u:%.2u"), tNow / 86400, ( tNow / 3600 ) % 24, ( tNow / 60 ) % 60, tNow % 60 );
				else
					str.Format( _T("%u:%.2u:%.2u"), tNow / 3600, ( tNow / 60 ) % 60, tNow % 60 );
			}
			break;
		case nrsNull:
		default:
			LoadString( str, IDS_NEIGHBOUR_UNKNOWN );
			break;
		}
		Add( _T("row_time"), str );

		if ( pNeighbour->GetUserCount() )
		{
			if ( pNeighbour->GetUserLimit() )
			{
				str.Format( _T("%u/%u"), pNeighbour->GetUserCount(), pNeighbour->GetUserLimit() );
			}
			else
			{
				str.Format( _T("%u"), pNeighbour->GetUserCount() );
			}				
			Add( _T("row_leaves"), str );
		}

		Add( _T("row_caption"), pNeighbour->m_sAddress + _T(" - ") + Neighbours.GetNick( pNeighbour ) );
		
		Output( _T("networkRow") );
		Prepare( _T("row_") );
	}
	
	Output( _T("networkNetEnd") );
	Prepare( _T("network_") );
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page : banner

void CRemote::PageBanner(const CString& strPath)
{
	ResourceRequest( strPath, m_pResponse, m_sHeader );
}

/////////////////////////////////////////////////////////////////////////////
// CRemote page : image server

void CRemote::PageImage(const CString& strPath)
{
	if ( CheckCookie() ) return;
	
	CFile hFile;
	if ( hFile.Open( Settings.General.Path + _T("\\Remote\\images\\") + SafeFilename( strPath.Mid( 15 ) ), CFile::modeRead ) )
	{
		m_pResponse.EnsureBuffer( (DWORD)hFile.GetLength() );
		hFile.Read( m_pResponse.m_pBuffer, (UINT)hFile.GetLength() );
		m_pResponse.m_nLength += (DWORD)hFile.GetLength();
		hFile.Close();
	}
}
