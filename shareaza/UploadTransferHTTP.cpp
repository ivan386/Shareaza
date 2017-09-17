//
// UploadTransferHTTP.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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
#include "LibraryFolders.h"
#include "Uploads.h"
#include "UploadFile.h"
#include "UploadFiles.h"
#include "UploadQueue.h"
#include "UploadQueues.h"
#include "UploadTransferHTTP.h"
#include "TransferFile.h"
#include "Transfers.h"
#include "Remote.h"
#include "ShellIcons.h"
#include "Statistics.h"
#include "Buffer.h"
#include "Schema.h"
#include "XML.h"
#include "VendorCache.h"

#include "Network.h"
#include "Library.h"
#include "SharedFile.h"
#include "Downloads.h"
#include "Download.h"

#include "LocalSearch.h"
#include "ImageFile.h"
#include "ThumbCache.h"
#include "Neighbours.h"
#include "Neighbour.h"
#include "G2Packet.h"
#include "GProfile.h"
#include "Security.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP construction

CUploadTransferHTTP::CUploadTransferHTTP() :
	CUploadTransfer		( PROTOCOL_HTTP )
,	m_bHead				( FALSE )
,	m_bConnectHdr		( FALSE )
,	m_bKeepAlive		( TRUE )
,	m_nAccept			( 0 )
,	m_bDeflate			( FALSE )
,	m_bBackwards		( FALSE )
,	m_bRange			( FALSE )
,	m_bQueueMe			( FALSE )
,	m_bNotShareaza		( FALSE )
,	m_nGnutella			( 0 )
,	m_nReaskMultiplier	( 1 )
,	m_bTigerTree		( FALSE )
,	m_bHashset			( FALSE )
,	m_bMetadata			( FALSE )
{
}

CUploadTransferHTTP::~CUploadTransferHTTP()
{
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP attach to connection

void CUploadTransferHTTP::AttachTo(CConnection* pConnection)
{
	CUploadTransfer::AttachTo( pConnection );

	theApp.Message( MSG_INFO, IDS_UPLOAD_ACCEPTED, (LPCTSTR)m_sAddress );

	m_mInput.pLimit		= &Settings.Bandwidth.Request;
	m_mOutput.pLimit	= &m_nBandwidth;

	m_nState	= upsRequest;
	m_tRequest	= m_tConnected;

	OnRead();
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP read handler

BOOL CUploadTransferHTTP::OnRead()
{
	CUploadTransfer::OnRead();

	switch ( m_nState )
	{
	case upsRequest:
	case upsQueued:
		if ( ! ReadRequest() ) return FALSE;
		if ( m_nState != upsHeaders ) break;

	case upsHeaders:
		return ReadHeaders();

	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP read : request line

BOOL CUploadTransferHTTP::ReadRequest()
{
	CString strLine;

	if ( ! Read( strLine ) ) return TRUE;
	if ( strLine.GetLength() > HTTP_HEADER_MAX_LINE ) strLine = _T("#LINE_TOO_LONG#");

	theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, _T("%s >> UPLOAD REQUEST: %s"), (LPCTSTR)m_sAddress, (LPCTSTR)strLine );

	if ( m_nState == upsQueued && m_pQueue != NULL )
	{
		DWORD tLimit = Settings.Uploads.QueuePollMin;

		tLimit *= m_nReaskMultiplier;

		if ( GetTickCount() - m_tRequest < tLimit )
		{
			Close( IDS_UPLOAD_BUSY_FAST );
			return FALSE;
		}
	}

	int nChar = strLine.Find( _T(" HTTP/") );

	if ( strLine.GetLength() < 14 || nChar < 5 ||
		 ( strLine.Left( 4 ) != _T("GET ") && strLine.Left( 5 ) != _T("HEAD ") ) )
	{
		Close( IDS_UPLOAD_NOHTTP );
		return FALSE;
	}

	ClearRequest();

	m_bHead			= ( strLine.Left( 5 ) == _T("HEAD ") );
	m_bConnectHdr	= FALSE;
	m_bKeepAlive	= TRUE;
	m_nAccept		= 0;
	m_bDeflate		= FALSE;
	m_bBackwards	= FALSE;
	m_bRange		= FALSE;
	m_bQueueMe		= FALSE;
	m_bNotShareaza  = FALSE;
	m_bMetadata		= FALSE;
	m_bTigerTree	= FALSE;
	m_bHashset		= FALSE;

	m_sLocations.Empty();
	m_sRanges.Empty();

	CString strRequest = strLine.Mid( m_bHead ? 5 : 4, nChar - ( m_bHead ? 5 : 4 ) );

	if ( strRequest.GetLength() > 5 && strRequest.Right( 1 ) == _T("/") )
	{
		strRequest = strRequest.Left( strRequest.GetLength() - 1 );
	}

	strRequest = URLDecode( strRequest );

	if ( strRequest != m_sRequest )
	{
		if ( m_sRequest.Find( _T("/gnutella/tigertree/") ) < 0 &&
			 strRequest.Find( _T("/gnutella/tigertree/") ) < 0 &&
			 m_sRequest.Find( _T("/gnutella/thex/") ) < 0 &&
			 strRequest.Find( _T("/gnutella/thex/") ) < 0 &&
			 m_sRequest.Find( _T("/gnutella/metadata/") ) < 0 &&
			 strRequest.Find( _T("/gnutella/metadata/") ) < 0 )
		{
			UploadQueues.Dequeue( this );
		}

		m_sRequest = strRequest;
	}

	m_nState	= upsHeaders;
	m_tRequest	= GetTickCount();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP read : headers

BOOL CUploadTransferHTTP::OnHeaderLine(CString& strHeader, CString& strValue)
{
	if ( ! CUploadTransfer::OnHeaderLine( strHeader, strValue ) )
		return FALSE;

	if ( strHeader.CompareNoCase( _T("Connection") ) == 0 )
	{
		if ( strValue.CompareNoCase( _T("Keep-Alive") ) == 0 ) m_bKeepAlive = TRUE;
		if ( strValue.CompareNoCase( _T("close") ) == 0 ) m_bKeepAlive = FALSE;
		m_bConnectHdr = TRUE;
	}
	else if ( strHeader.CompareNoCase( _T("Accept") ) == 0 )
	{
		if ( _tcsistr( strValue, _T("application/x-gnutella-packets") ) ) m_nAccept = 1;
		if ( _tcsistr( strValue, _T("application/x-gnutella2") ) ) m_nAccept = 2;
		if ( _tcsistr( strValue, _T("application/x-shareaza") ) ) m_nAccept = 2;
	}
	else if ( strHeader.CompareNoCase( _T("Accept-Encoding") ) == 0 )
	{
		if ( _tcsistr( strValue, _T("deflate") ) ) m_bDeflate = TRUE;
		if ( Settings.Uploads.AllowBackwards && _tcsistr( strValue, _T("backwards") ) ) m_bBackwards = TRUE;
	}
	else if ( strHeader.CompareNoCase( _T("Range") ) == 0 )
	{
		QWORD nFrom = 0, nTo = 0;

		if ( _stscanf( strValue, _T("bytes=%I64u-%I64u"), &nFrom, &nTo ) == 2 )
		{
			m_nOffset	= nFrom;
			m_nLength	= nTo + 1 - nFrom;
			m_bRange	= TRUE;
		}
		else if ( _stscanf( strValue, _T("bytes=%I64u-"), &nFrom ) == 1 )
		{
			m_nOffset	= nFrom;
			m_nLength	= SIZE_UNKNOWN;
			m_bRange	= TRUE;
		}
	}
	else if (	strHeader.CompareNoCase( _T("X-Gnutella-Content-URN") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Content-URN") ) == 0 ||
				strHeader.CompareNoCase( _T("Content-URN") ) == 0 )
	{
		HashesFromURN( strValue );
		m_nGnutella |= 1;
	}
	else if (	strHeader.CompareNoCase( _T("X-Gnutella-Alternate-Location") ) == 0 ||
				strHeader.CompareNoCase( _T("Alt-Location") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Alt") ) == 0 )
	{
		if ( Settings.Library.SourceMesh )
			m_sLocations = strValue;
		m_nGnutella |= 1;
	}
	else if ( strHeader.CompareNoCase( _T("X-NAlt") ) == 0 )
	{
		// Dead alt-sources
		LPCTSTR pszURN = (LPCTSTR)m_sRequest + 13;

		if ( CDownload* pDownload = Downloads.FindByURN( pszURN ) )
		{
			if ( Settings.Library.SourceMesh )
			{
				if ( strValue.Find( _T("://") ) < 0 )
				{
					pDownload->AddSourceURLs( strValue, TRUE );
				}
			}
		}
		m_nGnutella |= 1;
	}
	else if ( strHeader.CompareNoCase( _T("X-Node") ) == 0 )
	{
		m_bNotShareaza = TRUE; // Shareaza doesn't send this header
	}
	else if ( strHeader.CompareNoCase( _T("X-Queue") ) == 0 )
	{
		m_bQueueMe = TRUE;
		m_nGnutella |= 1;
		if ( strValue == _T("1.0") ) m_bNotShareaza = TRUE;			// Shareaza doesn't send this value
	}
	else if (	strHeader.CompareNoCase( _T("X-Nick") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Name") ) == 0 ||
				strHeader.CompareNoCase( _T("X-UserName") ) == 0 )
	{
		m_sRemoteNick = URLDecode( strValue );
	}
	else if ( strHeader.CompareNoCase( _T("X-Features") ) == 0 )
	{
		if ( _tcsistr( strValue, _T("g2/") ) != NULL ) m_nGnutella |= 2;
		if ( _tcsistr( strValue, _T("gnet2/") ) != NULL ) m_nGnutella |= 2;
		if ( _tcsistr( strValue, _T("gnutella2/") ) != NULL ) m_nGnutella |= 2;
		if ( m_nGnutella == 0 ) m_nGnutella = 1;
	}
	else if ( strHeader.CompareNoCase( _T("X-PAlt") ) == 0 ||
			  strHeader.CompareNoCase( _T("FP-1a") ) == 0 ||
			  strHeader.CompareNoCase( _T("FP-Auth-Challenge") ) == 0 )
	{
		m_nGnutella |= 1;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP process request

BOOL CUploadTransferHTTP::OnHeadersComplete()
{
	BOOL bBusy = FALSE;

	if ( Uploads.EnforcePerHostLimit( this, TRUE ) )
	{
		// Too many connections from same host
		SendResponse( IDR_HTML_BUSY, TRUE );
		theApp.Message( MSG_ERROR, IDS_UPLOAD_BUSY_HOST, (LPCTSTR)m_sName, (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );
		return TRUE;
	}
	else if ( Security.IsClientBanned( m_sUserAgent ) )
	{
		SendResponse( IDR_HTML_BROWSER );
		Remove( FALSE, IDS_SECURITY_BANNED_USERAGENT );
		return FALSE;
	}
	else if ( m_bClientExtended )
	{
		// Check for non-shareaza clients spoofing a Shareaza user agent
		if ( m_bNotShareaza )
		{
			SendResponse( IDR_HTML_FILENOTFOUND );
			Remove( FALSE, IDS_SECURITY_BANNED_USERAGENT );
			return FALSE;
		}

		// Assume certain capabilities for various Shareaza versions
		m_nGnutella |= 3;
		if ( m_sUserAgent == _T("Shareaza 1.4.0.0") ) m_bQueueMe = TRUE;
	}
	else if ( _tcsistr( m_sUserAgent, _T("limewire") ) != NULL || // LimeWire(FrostWire)
			  _tcsistr( m_sUserAgent, _T("phex") ) != NULL )
	{
		// Assume Gnutella1 ONLY capability for certain user-agents
		m_nGnutella = 1;
	}
	else if ( _tcsistr( m_sUserAgent, _T("trustyfiles") ) != NULL ||
			  _tcsistr( m_sUserAgent, _T("gnucdna") ) != NULL ||
			  _tcsistr( m_sUserAgent, _T("adagio") ) != NULL )
	{
		// Assume Gnutella2 capability for certain user-agents
		m_nGnutella |= 3;
	}

	CBuffer pResponse;
	CString sHeader;
	if ( ResourceRequest( m_sRequest, pResponse, sHeader ) )
	{
		m_nBandwidth = 0;

		Write( _P("HTTP/1.1 200 OK\r\n") );
		SendDefaultHeaders();
		CString strLength;
		strLength.Format( _T("Content-Length: %u\r\n"), pResponse.m_nLength );
		Write( strLength );
		if ( ! sHeader.IsEmpty() )
			Write( sHeader );
		Write( _P("\r\n") );

		LogOutgoing();

		Write( &pResponse );

		StartSending( upsResponse );

		return TRUE;
	}
	else if ( m_sRequest == _T("/") || m_sRequest == _T("/gnutella/browse/v1") )
	{
		// Requests for "/" or the browse path are handled the same way

		if ( m_nAccept == 0 && m_sRequest == _T("/") )
		{
			SendResponse( IDR_HTML_ABOUT );
			theApp.Message( MSG_INFO, IDS_UPLOAD_ABOUT, (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );
		}
		else if ( ( m_nAccept == 0 && ! Settings.Community.ServeFiles ) ||
				  ( m_nAccept == 1 && ! Settings.Community.ServeFiles ) ||
				  ( m_nAccept == 2 && ! Settings.Community.ServeProfile && ! Settings.Community.ServeFiles ) ||
				  ( ! RequestHostBrowse() ) )
		{
			SendResponse( IDR_HTML_ABOUT );
			theApp.Message( MSG_ERROR, IDS_UPLOAD_BROWSE_DENIED, (LPCTSTR)m_sAddress );
		}

		return TRUE;
	}
	else if ( ::StartsWith( m_sRequest, _PT("/remote") ) )
	{
		// A web client can start requesting remote pages on the same keep-alive
		// connection after previously requesting other system objects

		if ( Settings.Remote.Enable )
		{
			Prefix( _P("GET /remote/ HTTP/1.1\r\n\r\n") );
			new CRemote( this );
			Remove( FALSE );
			return FALSE;
		}
	}
	else if ( ::StartsWith( m_sRequest, _PT("/gnutella/preview/v1?urn:") ) && Settings.Uploads.SharePreviews )
	{
		LPCTSTR pszURN = (LPCTSTR)m_sRequest + 21;

		CSingleLock oLock( &Library.m_pSection );
		if ( oLock.Lock( 1000 ) )
		{
			if ( CLibraryFile* pShared = LibraryMaps.LookupFileByURN( pszURN, TRUE, TRUE ) )
				return RequestPreview( pShared, oLock );
		}
		else
			bBusy = TRUE;
	}
	else if ( ::StartsWith( m_sRequest, _PT("/gnutella/metadata/v1?urn:") ) && Settings.Uploads.ShareMetadata )
	{
		LPCTSTR pszURN = (LPCTSTR)m_sRequest + 22;

		{
			CSingleLock oLock( &Library.m_pSection );
			if ( oLock.Lock( 1000 ) )
			{
				if ( CLibraryFile* pShared = LibraryMaps.LookupFileByURN( pszURN, TRUE, TRUE ) )
				{
					if ( pShared->m_pMetadata != NULL )
					{
						m_sName	= pShared->m_sName;
						if ( CXMLElement* pMetadata	= pShared->m_pSchema->Instantiate( TRUE ) )
						{
							pMetadata->AddElement( pShared->m_pMetadata->Clone() );
							return RequestMetadata( pMetadata );
						}
					}
				}
			}
			else
				bBusy = TRUE;
		}

		if ( CDownload* pDownload = Downloads.FindByURN( pszURN ) )
		{
			if ( pDownload->HasMetadata() )
			{
				m_sName	= pDownload->m_sName;
				if ( CXMLElement* pMetadata	= pDownload->m_pXML->Clone() )
					return RequestMetadata( pMetadata );
			}
		}
	}
	else if ( Security.IsAgentBlocked( m_sUserAgent ) )
	{
		if ( m_sName.IsEmpty() ) m_sName = _T("file");
		SendResponse( IDR_HTML_BROWSER );
		theApp.Message( MSG_ERROR, IDS_UPLOAD_BROWSER, (LPCTSTR)m_sAddress, (LPCTSTR)m_sName );
		Security.Complain( &m_pHost.sin_addr );
		if ( m_sUserAgent.Find( _T("Mozilla") ) >= 0 ) return TRUE;
		Remove( FALSE );
		return FALSE;
	}
	else if ( IsNetworkDisabled() )
	{
		// Network isn't active- Check if we should send 404 or 403

		if ( ::StartsWith( m_sRequest, _PT("/uri-res/N2R?urn:") ) )
		{
			LPCTSTR pszURN = (LPCTSTR)m_sRequest + 13;

			CSingleLock oLock( &Library.m_pSection );
			if ( oLock.Lock( 1000 ) )
			{
				if ( CLibraryFile* pFile = LibraryMaps.LookupFileByURN( pszURN, TRUE, TRUE ) )
					SendResponse( IDR_HTML_DISABLED );
				else
					// Network is disabled, but we don't have the file anyway.
					SendResponse( IDR_HTML_FILENOTFOUND );
			}
			else
				SendResponse( IDR_HTML_BUSY );
		}
		else
			SendResponse( IDR_HTML_DISABLED );

		theApp.Message( MSG_ERROR, IDS_UPLOAD_DISABLED, (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );
		Security.Ban( &m_pHost.sin_addr, ban30Mins, FALSE ); // Anti-hammer protection if client doesn't understand 403
		Remove( FALSE );
		return FALSE;
	}
	else if ( ::StartsWith( m_sRequest, _PT("/gnutella/tigertree/v3?urn:") ) && Settings.Uploads.ShareTiger )
	{
		LPCTSTR pszURN = (LPCTSTR)m_sRequest + 23;

		{
			CSingleLock oLock( &Library.m_pSection );
			if ( oLock.Lock( 1000 ) )
			{
				if ( CLibraryFile* pShared = LibraryMaps.LookupFileByURN( pszURN, TRUE, TRUE ) )
				{
					const CTigerTree* pTigerTree = pShared->GetTigerTree();
					m_sName = pShared->m_sName;
					return RequestTigerTreeRaw( pTigerTree, TRUE );
				}
			}
			else
				bBusy = TRUE;
		}

		if ( const CDownload* pDownload = Downloads.FindByURN( pszURN ) )
		{
			const CTigerTree* pTigerTree = pDownload->GetTigerTree();
			m_sName = pDownload->m_sName;
			return RequestTigerTreeRaw( pTigerTree, FALSE );
		}
	}
	else if ( ::StartsWith( m_sRequest, _PT("/gnutella/thex/v1?urn:") ) && ( Settings.Uploads.ShareTiger || Settings.Uploads.ShareHashset ) )
	{
		LPCTSTR pszURN	= (LPCTSTR)m_sRequest + 18;
		DWORD nDepth	= 0;

		if ( LPCTSTR pszDepth = _tcsistr( m_sRequest, _T("depth=") ) )
		{
			_stscanf( pszDepth + 6, _T("%lu"), &nDepth );
		}

		const bool bTigerTree = Settings.Uploads.ShareTiger;
		const bool bHashset = ( _tcsistr( m_sRequest, _T("ed2k=1") ) != NULL ) && Settings.Uploads.ShareHashset;

		{
			CSingleLock oLock( &Library.m_pSection );
			if ( oLock.Lock( 1000 ) )
			{
				if ( CLibraryFile* pShared = LibraryMaps.LookupFileByURN( pszURN, TRUE, TRUE ) )
				{
					const CTigerTree* pTigerTree = bTigerTree ? pShared->GetTigerTree() : NULL;
					const CED2K* pHashset = bHashset ? pShared->GetED2K() : NULL;
					m_sName = pShared->m_sName;
					m_nSize = pShared->GetSize();
					return RequestTigerTreeDIME( pTigerTree, nDepth, pHashset, TRUE );
				}
			}
			else
				bBusy = TRUE;
		}

		if ( const CDownload* pDownload = Downloads.FindByURN( pszURN ) )
		{
			const CTigerTree* pTigerTree = bTigerTree ? pDownload->GetTigerTree() : NULL;
			const CED2K* pHashset = bHashset ? pDownload->GetHashset() : NULL;
			m_sName = pDownload->m_sName;
			m_nSize = pDownload->m_nSize;
			return RequestTigerTreeDIME( pTigerTree, nDepth, pHashset, FALSE );
		}
	}
	else if ( ::StartsWith( m_sRequest, _PT("/uri-res/N2R?urn:") ) )
	{
		LPCTSTR pszURN = (LPCTSTR)m_sRequest + 13;

		{
			CSingleLock oLock( &Library.m_pSection );
			if ( oLock.Lock( 1000 ) )
			{
				if ( CLibraryFile* pShared = LibraryMaps.LookupFileByURN( pszURN, TRUE, TRUE ) )
					return RequestSharedFile( pShared, oLock );
			}
			else
				bBusy = TRUE;
		}

		CDownload* pDownload = Downloads.FindByURN( pszURN );
		if ( pDownload != NULL && pDownload->IsShared() && pDownload->IsStarted() )
			return RequestPartialFile( pDownload );
	}
	else if ( ::StartsWith( m_sRequest, _PT("/get/") ) )
	{
		DWORD nIndex = 0;
		CString strFile	= m_sRequest.Mid( 5 );
		int nChar = strFile.Find( '/' );
		bool bByIndex = ( _stscanf( strFile, _T("%lu/"), &nIndex ) == 1 &&
			nChar > 0 && nChar < strFile.GetLength() - 1 );
		strFile = strFile.Mid( nChar + 1 );

		CSingleLock oLock( &Library.m_pSection );
		if ( oLock.Lock( 1000 ) )
		{
			CLibraryFile* pFile = NULL;
			if ( bByIndex )
			{
				pFile = Library.LookupFile( nIndex, TRUE, TRUE );
				if ( pFile && pFile->m_sName.CompareNoCase( strFile ) )
					pFile = NULL;
			}
			if ( ! pFile )
				pFile = LibraryMaps.LookupFileByName( strFile, m_nSize, TRUE, TRUE );
			if ( pFile )
				return RequestSharedFile( pFile, oLock );
		}
		else
			bBusy = TRUE;
	}
	else
	{
		CString strFile = m_sRequest.Mid( 1 );

		CSingleLock oLock( &Library.m_pSection );
		if ( oLock.Lock( 1000 ) )
		{
			if ( CLibraryFile* pFile = LibraryMaps.LookupFileByName( strFile, m_nSize, TRUE, TRUE ) )
				return RequestSharedFile( pFile, oLock );
		}
		else
			bBusy = TRUE;
	}

	if ( m_sName.IsEmpty() )
	{
		if ( m_oSHA1 )
			m_sName = m_oSHA1.toUrn();
		else
			m_sName = m_sRequest;
	}

	if ( bBusy )
	{
		SendResponse( IDR_HTML_BUSY );
		theApp.Message( MSG_ERROR, _T("Refusing upload of \"%s\" to %s, Library is busy."), (LPCTSTR)m_sName , (LPCTSTR)m_sAddress);
	}
	else
	{
		SendResponse( IDR_HTML_FILENOTFOUND );
		theApp.Message( MSG_ERROR, IDS_UPLOAD_FILENOTFOUND, (LPCTSTR)m_sAddress, (LPCTSTR)m_sName );
	}

	return TRUE;
}

BOOL CUploadTransferHTTP::IsNetworkDisabled()
{
	if ( !Network.IsConnected() ) return TRUE;
	if ( Settings.Connection.RequireForTransfers == FALSE ) return FALSE;
	if ( ( m_nGnutella & 2 ) && Settings.Gnutella2.EnableToday ) return FALSE;
	if ( ( m_nGnutella & 1 ) && Settings.Gnutella1.EnableToday ) return FALSE;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP request a shared file

BOOL CUploadTransferHTTP::RequestSharedFile(CLibraryFile* pFile, CSingleLock& oLibraryLock)
{
	ASSERT( pFile != NULL );

	if ( ! RequestComplete( pFile ) )
	{
		oLibraryLock.Unlock();
		SendResponse( IDR_HTML_HASHMISMATCH );
		theApp.Message( MSG_ERROR, IDS_UPLOAD_HASH_MISMATCH, (LPCTSTR)m_sAddress, (LPCTSTR)m_sName );
		return TRUE;
	}

	if ( IsNetworkDisabled() )
	{
		oLibraryLock.Unlock();
		SendResponse( IDR_HTML_DISABLED );
		theApp.Message( MSG_ERROR, IDS_UPLOAD_DISABLED, (LPCTSTR)m_sAddress, (LPCTSTR)m_sName );
		return TRUE;
	}

	if ( ! UploadQueues.CanUpload( PROTOCOL_HTTP, pFile ) )
	{
		// File is not uploadable. (No queue, is a ghost, etc)
		if ( m_sName.IsEmpty() ) m_sName = m_oSHA1.toUrn();

		oLibraryLock.Unlock();
		SendResponse( IDR_HTML_FILENOTFOUND );
		theApp.Message( MSG_ERROR, IDS_UPLOAD_FILENOTFOUND, (LPCTSTR)m_sAddress, (LPCTSTR)m_sName );
		return TRUE;
	}

	m_bTigerTree	= bool( m_oTiger );
	m_bHashset		= bool( m_oED2K );
	m_bMetadata		= ( pFile->m_pMetadata != NULL && ( pFile->m_bMetadataAuto == FALSE || pFile->m_nVirtualSize > 0 ) );

	if ( ! HasHash() )
		m_sLocations.Empty();

	if ( m_nOffset == SIZE_UNKNOWN )
		m_nOffset = 0;
	if ( m_nLength == SIZE_UNKNOWN )
		m_nLength = m_nSize - m_nOffset;

	if ( m_nOffset >= m_nSize || m_nOffset + m_nLength > m_nSize )
	{
		oLibraryLock.Unlock();
		SendResponse( IDR_HTML_BADRANGE );
		theApp.Message( MSG_ERROR, IDS_UPLOAD_BAD_RANGE, (LPCTSTR)m_sAddress, (LPCTSTR)m_sName );
		return TRUE;
	}

	if ( m_sLocations.GetLength() )
	{
		pFile->AddAlternateSources( m_sLocations );
		m_sLocations.Empty();
	}

	if ( Settings.Library.SourceMesh && m_nGnutella > 1 )
	{
		m_sLocations = pFile->GetAlternateSources( &m_pSourcesSent, 15,
			PROTOCOL_HTTP );
	}

	oLibraryLock.Unlock();

	return QueueRequest();
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP request a partial file

BOOL CUploadTransferHTTP::RequestPartialFile(CDownload* pDownload)
{
	ASSERT( pDownload != NULL );
	ASSERT( pDownload->IsStarted() );

	if ( ! RequestPartial( pDownload ) )
	{
		SendResponse( IDR_HTML_HASHMISMATCH );
		theApp.Message( MSG_WARNING, IDS_UPLOAD_HASH_MISMATCH,
			(LPCTSTR)m_sAddress, (LPCTSTR)m_sName );
		return TRUE;
	}

	ASSERT( m_nFileBase == 0 );

	m_bTigerTree	= ( m_oTiger && pDownload->GetTigerTree() != NULL );
	m_bHashset		= ( m_oED2K && pDownload->GetHashset() != NULL );
	m_bMetadata		= pDownload->HasMetadata();

	if ( m_sLocations.GetLength() )
	{
		pDownload->AddSourceURLs( m_sLocations );
		m_sLocations.Empty();
	}

	if ( Settings.Library.SourceMesh )
	{
		m_sLocations = pDownload->GetSourceURLs( &m_pSourcesSent, 15,
			( m_nGnutella < 2 ) ? PROTOCOL_G1 : PROTOCOL_HTTP, NULL );
	}

	pDownload->GetAvailableRanges( m_sRanges );

	if ( ! m_bRange || ( m_nOffset == 0 && m_nLength == SIZE_UNKNOWN ) )
		pDownload->GetRandomRange( m_nOffset, m_nLength );

	if ( m_nOffset == SIZE_UNKNOWN )
		m_nOffset = 0;
	if ( m_nLength == SIZE_UNKNOWN )
		m_nLength = m_nSize - m_nOffset;

	if ( pDownload->ClipUploadRange( m_nOffset, m_nLength ) )
		return QueueRequest();

	if ( pDownload->IsMoving() )
	{
		if ( GetTickCount() - pDownload->m_tCompleted < 60 * 60 * 1000 ) // 1 hour
		{
			Write( _P("HTTP/1.1 503 Range Temporarily Unavailable\r\n") );
			Write( _P("Retry-After: 60\r\n") ); // 1 min
		}
		else
		{
			SendResponse( IDR_HTML_FILENOTFOUND );
			theApp.Message( MSG_INFO, IDS_UPLOAD_FILENOTFOUND,
				(LPCTSTR)m_sAddress, (LPCTSTR)m_sName );
			return TRUE;
		}
	}
	else if ( pDownload->HasActiveTransfers() )
	{
		Write( _P("HTTP/1.1 503 Range Temporarily Unavailable\r\n") );
	}
	else
	{
		Write( _P("HTTP/1.1 416 Requested Range Unavailable\r\n") );
	}

	SendDefaultHeaders();
	SendFileHeaders();

	Write( _P("Content-Length: 0\r\n") );
	Write( _P("\r\n") );

	LogOutgoing();

	StartSending( upsResponse );

	theApp.Message( MSG_INFO, IDS_UPLOAD_BAD_RANGE, (LPCTSTR)m_sAddress,
		(LPCTSTR)m_sName );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP queue the request if necessary

BOOL CUploadTransferHTTP::QueueRequest()
{
	if ( m_bHead ) return OpenFileSendHeaders();

	AllocateBaseFile();

	UINT nError		= 0;
	int nPosition	= 0;

	if ( m_bStopTransfer )
	{

		m_tRotateTime = 0;
		m_bStopTransfer	= FALSE;

		CUploadQueue* pQueue = m_pQueue;
		if ( pQueue ) pQueue->Dequeue( this );
	}


	if ( Uploads.CanUploadFileTo( &m_pHost.sin_addr, this ) )	//if ( Uploads.AllowMoreTo( &m_pHost.sin_addr ) )
	{
		if ( ( nPosition = UploadQueues.GetPosition( this, TRUE ) ) >= 0 )
		{
			ASSERT( m_pQueue != NULL );

			// If the queue can't accept this file
			if ( ! m_pQueue->CanAccept( m_nProtocol, m_sName, m_nSize,
				( m_bFilePartial ? CUploadQueue::ulqPartial: CUploadQueue::ulqLibrary ), m_sFileTags ) )
			{	// This is probably a partial that has completed
				theApp.Message( MSG_DEBUG, _T("File queue error- Partial may have recently completed") );

				// Might as well allow the upload... so don't do anything.
				//ASSERT( FALSE );
			}

			if ( nPosition == 0 )
			{
				// Queued, and ready to send
				return OpenFileSendHeaders();
			}
			else
			{
				// Queued, but must wait
			}
		}
		else if ( UploadQueues.Enqueue( this ) )
		{
			ASSERT( m_pQueue != NULL );
			ASSERT( m_pQueue->CanAccept( m_nProtocol, m_sName, m_nSize,
				( m_bFilePartial ? CUploadQueue::ulqPartial : CUploadQueue::ulqLibrary ), m_sFileTags ) );

			nPosition = UploadQueues.GetPosition( this, TRUE );
			if ( nPosition == 0 )
			{
				// Queued, and ready to send
				return OpenFileSendHeaders();
			}
			else if ( nPosition > 0 && m_bQueueMe )
			{
				// Queued, but must wait
			}
			else
			{
				// Client can't queue, so dequeue and return busy
				UploadQueues.Dequeue( this );
				ASSERT( m_pQueue == NULL );
			}
		}
		else
		{
			// Unable to queue anywhere
		}
	}
	else
	{
		// Too many from this host

		UploadQueues.Dequeue( this );
		ASSERT( m_pQueue == NULL );

		nError = IDS_UPLOAD_BUSY_HOST;
	}

	if ( m_pQueue != NULL )
	{
		CString strHeader, strName;

		Write( _P("HTTP/1.1 503 Busy Queued\r\n") );

		SendDefaultHeaders();
		SendFileHeaders();

		m_nReaskMultiplier=( nPosition <= 9 ) ? ( (nPosition+1) / 2 ) : 5;
		DWORD nTimeScale = 1000 / m_nReaskMultiplier;

		CSingleLock pLock( &UploadQueues.m_pSection, TRUE );

		if ( UploadQueues.Check( m_pQueue ) )
		{
			strName = m_pQueue->m_sName;
			strName.Replace( _T("\""), _T("'") );

			strHeader.Format( _T("X-Queue: position=%d,length=%u,limit=%u,pollMin=%lu,pollMax=%lu,id=\"%s\"\r\n"),
				nPosition,
				m_pQueue->GetQueuedCount(),
				m_pQueue->GetTransferCount( TRUE ),
				Settings.Uploads.QueuePollMin / nTimeScale,
				Settings.Uploads.QueuePollMax / nTimeScale,
				(LPCTSTR)strName );

			theApp.Message( MSG_INFO, IDS_UPLOAD_QUEUED, (LPCTSTR)m_sName,
				(LPCTSTR)m_sAddress, nPosition, m_pQueue->GetQueuedCount(),
				(LPCTSTR)strName );

		}

		pLock.Unlock();

		Write( strHeader );
		Write( _P("Content-Length: 0\r\n\r\n") );

		LogOutgoing();

		StartSending( upsPreQueue );
	}
	else
	{
		theApp.Message( MSG_ERROR, ( nError ? nError : ( m_bQueueMe ?
			IDS_UPLOAD_BUSY_QUEUE : IDS_UPLOAD_BUSY_OLD ) ),
			(LPCTSTR)m_sName, (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );
		SendResponse( IDR_HTML_BUSY, TRUE );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP default response headers

void CUploadTransferHTTP::SendDefaultHeaders()
{
	CString strLine = Settings.SmartAgent();

	if ( strLine.GetLength() )
	{
		strLine = _T("Server: ") + strLine + _T("\r\n");
		Write( strLine );
	}

	if ( ! m_bInitiated )
	{
		strLine.Format( _T("Remote-IP: %s\r\n"),
			(LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ) );
		Write( strLine );
	}

	if ( IsNetworkDisabled() )
	{
		// Ask to retry after some delay in seconds
		Write( _P("Retry-After: 3600\r\n") ); // 1 hour
	}

	if ( m_bKeepAlive )
	{
		Write( _P("Connection: Keep-Alive\r\n") );
	}
	else
	{
		Write( _P("Connection: close\r\n") );
	}

	Write( _P("Accept-Ranges: bytes\r\n") );

	if ( m_nRequests <= 1 )
	{
		if ( m_bInitiated ) SendMyAddress();
		strLine.Format( _T("X-PerHost: %lu\r\n"), Settings.Uploads.MaxPerHost );
		Write( strLine );

		strLine = MyProfile.GetNick().Left( 255 );

		if ( strLine.GetLength() > 0 )
		{
			strLine = _T("X-Nick: ") + URLEncode( strLine ) + _T("\r\n");
			Write( strLine );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP file response headers

void CUploadTransferHTTP::SendFileHeaders()
{
	if ( ! m_sName.IsEmpty() )
	{
		Write( _P("Content-Disposition: attachment; filename=\"") );
		Write( URLEncode( PathFindFileName( m_sName ) ) );
		Write( _P("\"\r\n") );
	}

	if ( m_oSHA1 || m_oTiger )
	{
		Write( _P("X-Content-URN: ") );
		Write( GetBitprint() );
		Write( _P("\r\n") );
	}

	if ( m_oED2K )
	{
		Write( _P("X-Content-URN: ") );
		Write( m_oED2K.toUrn() );
		Write( _P("\r\n") );
	}

	if ( m_oBTH )
	{
		Write( _P("X-Content-URN: ") );
		Write( m_oBTH.toUrn() );
		Write( _P("\r\n") );
	}

	if ( m_oMD5 )
	{
		Write( _P("X-Content-URN: ") );
		Write( m_oMD5.toUrn() );
		Write( _P("\r\n") );
	}

	if ( m_bTigerTree && Settings.Uploads.ShareTiger )
	{
		CString strTigerURL;
		strTigerURL.Format( _T("X-Thex-URI: /gnutella/thex/v1?%s&depth=%u&ed2k=%d;%s\r\n"),
			(LPCTSTR)m_oTiger.toUrn(), Settings.Library.TigerHeight, ( ( m_bHashset && Settings.Uploads.ShareHashset ) ? 1 : 0 ), (LPCTSTR)m_oTiger.toString() );
		Write( strTigerURL );
	}
	else if ( m_bHashset && Settings.Uploads.ShareHashset )
	{
		CString strTigerURL;
		strTigerURL.Format( _T("X-Thex-URI: /gnutella/thex/v1?%s&ed2k=1;%s\r\n"),
			(LPCTSTR)m_oED2K.toUrn(), (LPCTSTR)m_oED2K.toString() );
		Write( strTigerURL );
	}

	if ( m_bMetadata )
	{
		Write( _P("X-Metadata-Path: /gnutella/metadata/v1?") );
		Write( m_oTiger ? m_oTiger.toUrn() : m_oED2K.toUrn() );
		Write( _P("\r\n") );
	}

	if ( m_sRanges.GetLength() )
	{
		Write( _P("X-Available-Ranges: ") );
		Write( m_sRanges );
		Write( _P("\r\n") );
	}

	if ( m_sLocations.GetLength() )
	{
		if ( m_sLocations.Find( _T("://") ) < 0 )
			Write( _P("X-Alt: ") );
		else
			Write( _P("Alt-Location: ") );
		Write( m_sLocations );
		Write( _P("\r\n") );
	}

	if ( m_nGnutella < 2 )
	{
		LPCTSTR pszURN = (LPCTSTR)m_sRequest + 13;

		// Send X-NAlt for partial transfers only
		if ( CDownload* pDownload = Downloads.FindByURN( pszURN ) )
		{
			Write( _P("X-NAlt: ") );
			Write( pDownload->GetTopFailedSources( 15, PROTOCOL_G1 ) );
			Write( _P("\r\n") );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP open file and send headers

BOOL CUploadTransferHTTP::OpenFileSendHeaders()
{
	if ( ! OpenFile() )
	{
		SendResponse( IDR_HTML_FILENOTFOUND );

		return TRUE;
	}

	CSingleLock pLock( &UploadQueues.m_pSection, TRUE );

	if ( m_pQueue != NULL && UploadQueues.Check( m_pQueue ) && m_pQueue->m_bRotate )
	{
		DWORD nLimit = m_pQueue->m_nRotateChunk;
		if ( nLimit == 0 ) nLimit = Settings.Uploads.RotateChunkLimit;

		if ( nLimit > 0 && m_nLength > nLimit )
		{
			if ( m_bBackwards )
				m_nOffset += m_nLength - nLimit;

			m_nLength = nLimit;
		}
	}

	pLock.Unlock();

	if ( m_nLength != m_nSize )
		Write( _P("HTTP/1.1 206 OK\r\n") );
	else
		Write( _P("HTTP/1.1 200 OK\r\n") );

	SendDefaultHeaders();
	SendFileHeaders();

	Write( _T("Content-Type: ") + ShellIcons.GetMIME( PathFindExtension( m_sName ) ) + _T("\r\n") );

	CString strResponse;
	strResponse.Format( _T("Content-Length: %I64u\r\n"), m_nLength );
	Write( strResponse );

	if ( m_nLength != m_nSize )
	{
		strResponse.Format( _T("Content-Range: bytes=%I64u-%I64u/%I64u\r\n"), m_nOffset, m_nOffset + m_nLength - 1, m_nSize );
		Write( strResponse );
	}

	if ( ! m_bHead && m_bBackwards )
	{
		Write( _P("Content-Encoding: backwards\r\n") );
	}

	Write( _P("\r\n") );

	LogOutgoing();

	if ( m_bHead )
	{
		theApp.Message( MSG_INFO, IDS_UPLOAD_HEADERS, (LPCTSTR)m_sName,
			(LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );

		StartSending( upsResponse );
	}
	else
	{
		if ( m_pBaseFile->m_nRequests++ == 0 )
		{
			theApp.Message( MSG_NOTICE, IDS_UPLOAD_FILE,
				(LPCTSTR)m_sName, (LPCTSTR)m_sAddress );

			PostMainWndMessage( WM_NOWUPLOADING, 0, (LPARAM)new CString( m_pBaseFile->m_sPath ) );
		}

		theApp.Message( MSG_INFO,
			m_sRanges.GetLength() ? IDS_UPLOAD_PARTIAL_CONTENT : IDS_UPLOAD_CONTENT,
			m_nOffset, m_nOffset + m_nLength - 1, (LPCTSTR)m_sName,
			(LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );

		StartSending( upsUploading );
	}

	OnWrite();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP write handler

BOOL CUploadTransferHTTP::OnWrite()
{
	if ( m_nState == upsUploading && IsFileOpen() && GetOutputLength() == 0 )
	{
		if ( m_nPosition >= m_nLength )
		{
			OnCompleted();
			CUploadTransfer::OnWrite();
			return TRUE;
		}

		QWORD nPacket = min( m_nLength - m_nPosition, 1024000ull ); // 1000 KB
		auto_array< BYTE > pBuffer( new BYTE[ (size_t)nPacket ] );

		if ( m_bBackwards )
		{
			QWORD nRead = 0;
			if ( ! ReadFile( m_nFileBase + m_nOffset + m_nLength -
				m_nPosition - nPacket, pBuffer.get(), nPacket, &nRead ) ||
				nRead != nPacket )
				return TRUE;
			WriteReversed( pBuffer.get(), (DWORD)nPacket );
		}
		else
		{
			if ( ! ReadFile( m_nFileBase + m_nOffset + m_nPosition,
				pBuffer.get(), nPacket, &nPacket ) ||
				nPacket == 0 )
				return TRUE;
			Write( pBuffer.get(), (DWORD)nPacket );
		}

		m_nPosition += nPacket;
		m_nUploaded += nPacket;

		Statistics.Current.Uploads.Volume += ( nPacket / 1024 );
	}

	CUploadTransfer::OnWrite();

	if ( m_nState >= upsResponse && GetOutputLength() == 0 )
	{
		m_nState	= ( m_nState == upsPreQueue ) ? upsQueued : upsRequest;
		m_tRequest	= GetTickCount();
	}

	return TRUE;
}

void CUploadTransferHTTP::OnCompleted()
{
	Uploads.SetStable( GetAverageSpeed() );

	m_nState	= upsRequest;
	m_tRequest	= GetTickCount();

	m_pBaseFile->AddFragment( m_nOffset, m_nLength );
	// m_pBaseFile = NULL;

	theApp.Message( MSG_INFO, IDS_UPLOAD_FINISHED, (LPCTSTR)m_sName, (LPCTSTR)m_sAddress );
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP run handler

BOOL CUploadTransferHTTP::OnRun()
{
	CUploadTransfer::OnRun();

	DWORD tNow = GetTickCount();

	switch ( m_nState )
	{
	case upsRequest:
		if ( ! m_bKeepAlive && GetOutputLength() == 0 )
		{
			Close( IDS_UPLOAD_DROPPED );
			return FALSE;
		}

	case upsHeaders:
		if ( tNow - m_tRequest > Settings.Connection.TimeoutHandshake )
		{
			Close( IDS_UPLOAD_REQUEST_TIMEOUT );
			return FALSE;
		}
		break;

	case upsQueued:
		if ( tNow - m_tRequest > ( Settings.Uploads.QueuePollMax * m_nReaskMultiplier ) )
		{
			Close( IDS_UPLOAD_REQUEST_TIMEOUT );
			return FALSE;
		}
		break;

	case upsUploading:
	case upsResponse:
	case upsBrowse:
	case upsTigerTree:
	case upsMetadata:
	case upsPreview:
	case upsPreQueue:
		if ( tNow - m_mOutput.tLast > Settings.Connection.TimeoutTraffic )
		{
			Close( IDS_UPLOAD_TRAFFIC_TIMEOUT );
			return FALSE;
		}
		break;

	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP dropped handler

void CUploadTransferHTTP::OnDropped()
{
	if ( m_nState == upsUploading && m_pBaseFile != NULL )
	{
		if ( m_bBackwards )
		{
			m_pBaseFile->AddFragment( m_nOffset + m_nLength - m_nPosition, m_nPosition );
		}
		else
		{
			m_pBaseFile->AddFragment( m_nOffset, m_nPosition );
		}

		m_pBaseFile = NULL;
	}

	Close( IDS_UPLOAD_DROPPED );
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP request metadata

BOOL CUploadTransferHTTP::RequestMetadata(CXMLElement* pMetadata)
{
	ASSERT( pMetadata != NULL );
	CStringA strXML = UTF8Encode( pMetadata->ToString( TRUE, TRUE ) );
	delete pMetadata;

	Write( _P("HTTP/1.1 200 OK\r\n") );
	SendDefaultHeaders();
	Write( _P("Content-Type: text/xml\r\n") );

	CString strHeader;
	strHeader.Format( _T("Content-Length: %d\r\n"), strXML.GetLength() );
	Write( strHeader );
	Write( _P("\r\n") );

	LogOutgoing();

	if ( ! m_bHead ) Write( (LPCSTR)strXML, strXML.GetLength() );

	StartSending( upsMetadata );

	theApp.Message( MSG_INFO, IDS_UPLOAD_METADATA_SEND,
		(LPCTSTR)m_sName, (LPCTSTR)m_sAddress );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP request a tiger tree hash, raw format

BOOL CUploadTransferHTTP::RequestTigerTreeRaw(const CTigerTree* pTigerTree, BOOL bDelete)
{
	BYTE* pSerialTree = NULL;
	DWORD nSerialTree = 0;

	if ( ! pTigerTree || ! pTigerTree->ToBytes( &pSerialTree, &nSerialTree ) )
	{
		ClearHashes();
		m_sLocations.Empty();

		SendResponse( IDR_HTML_FILENOTFOUND, TRUE );
		theApp.Message( MSG_ERROR, IDS_UPLOAD_FILENOTFOUND, (LPCTSTR)m_sAddress, (LPCTSTR)m_sName );

		return TRUE;
	}

	if ( bDelete )
		delete pTigerTree;

	if ( m_bRange )
	{
		if ( m_nOffset >= nSerialTree ) m_nLength = SIZE_UNKNOWN;
		else m_nLength = min( m_nLength, nSerialTree - m_nOffset );
	}
	else
	{
		m_nOffset = 0;
		m_nLength = nSerialTree;
	}

	if ( m_nLength <= nSerialTree )
	{
		CString strHeader;

		if ( m_nLength != nSerialTree )
			Write( _P("HTTP/1.1 206 OK\r\n") );
		else
			Write( _P("HTTP/1.1 200 OK\r\n") );

		SendDefaultHeaders();

		Write( _P("Content-Type: application/tigertree-breadthfirst\r\n") );
		strHeader.Format( _T("Content-Length: %I64u\r\n"), m_nLength );
		Write( strHeader );

		if ( m_nLength != nSerialTree )
		{
			strHeader.Format( _T("Content-Range: %I64u-%I64u\r\n"), m_nOffset, m_nOffset + m_nLength - 1 );
			Write( strHeader );
		}

		Write( _P("\r\n") );

		LogOutgoing();

		if ( ! m_bHead ) Write( pSerialTree + m_nOffset, (DWORD)m_nLength );

		StartSending( upsTigerTree );

		theApp.Message( MSG_INFO, IDS_UPLOAD_TIGER_SEND,
			(LPCTSTR)m_sName, (LPCTSTR)m_sAddress );
	}
	else
	{
		m_sRanges.Format( _T("0-%I64u"), (QWORD)nSerialTree - 1 );
		ClearHashes();
		m_sLocations.Empty();

		SendResponse( IDR_HTML_BADRANGE, TRUE );
		theApp.Message( MSG_ERROR, IDS_UPLOAD_BAD_RANGE, (LPCTSTR)m_sAddress, (LPCTSTR)m_sName );
	}

	GlobalFree( pSerialTree );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP request a tiger tree hash, DIME format

BOOL CUploadTransferHTTP::RequestTigerTreeDIME(const CTigerTree* pTigerTree, int nDepth, const CED2K* pHashset, BOOL bDelete)
{
	DWORD nSerialTree = 0;
	BYTE* pSerialTree = NULL;
	BOOL  bSendDIME   = FALSE;
	CBuffer pDIME;

	if ( pTigerTree && pTigerTree->ToBytes( &pSerialTree, &nSerialTree, nDepth ) )
	{
		CString strUUID, strXML;
		Hashes::Guid oGUID;
		Network.CreateID( oGUID );
		GUID pUUID;
		std::memcpy( &pUUID, &oGUID[ 0 ], sizeof( pUUID ) );
		strUUID.Format( _T("uuid:%.8x-%.4x-%.4x-%.2x%.2x-%.2x%.2x%.2x%.2x%.2x%.2x"),
			pUUID.Data1, pUUID.Data2, pUUID.Data3,
			pUUID.Data4[0], pUUID.Data4[1], pUUID.Data4[2], pUUID.Data4[3],
			pUUID.Data4[4], pUUID.Data4[5], pUUID.Data4[6], pUUID.Data4[7] );

		if ( nDepth < 1 || nDepth > (int)pTigerTree->GetHeight() )
			nDepth = pTigerTree->GetHeight();

		strXML.Format(	_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n")
						_T("<!DOCTYPE hashtree SYSTEM \"http://open-content.net/spec/thex/thex.dtd\">\r\n")
						_T("<hashtree>\r\n")
						_T("\t<file size=\"%I64u\" segmentsize=\"1024\"/>\r\n")
						_T("\t<digest algorithm=\"http://open-content.net/spec/digest/tiger\" outputsize=\"24\"/>\r\n")
						_T("\t<serializedtree depth=\"%d\" type=\"http://open-content.net/spec/thex/breadthfirst\" uri=\"%s\"/>\r\n")
						_T("</hashtree>"),
						m_nSize, nDepth, (LPCTSTR)strUUID );

		CStringA strXMLUTF8 = UTF8Encode( strXML );

		int nUUID = WideCharToMultiByte( CP_ACP, 0, strUUID, -1, NULL, 0, NULL, NULL );
		LPSTR pszUUID = new CHAR[ nUUID ];
		WideCharToMultiByte( CP_ACP, 0, strUUID, -1, pszUUID, nUUID, NULL, NULL );

		pDIME.WriteDIME( 1, _P(""), _P("text/xml"), strXMLUTF8, strXMLUTF8.GetLength() );
		pDIME.WriteDIME( pHashset ? 0 : 2, pszUUID, nUUID - 1,
			_P("http://open-content.net/spec/thex/breadthfirst"), pSerialTree, nSerialTree );
		GlobalFree( pSerialTree );

		delete [] pszUUID;

		bSendDIME = TRUE;
	}

	if ( bDelete )
		delete pTigerTree;

	if ( pHashset && pHashset->ToBytes( &pSerialTree, &nSerialTree ) )
	{
		pDIME.WriteDIME( 2, _P(""),
			_P("http://edonkey2000.com/spec/md4-hashset"), pSerialTree, nSerialTree );
		GlobalFree( pSerialTree );

		bSendDIME = TRUE;
	}

	if ( bDelete )
		delete pHashset;

	if ( ! bSendDIME )
	{
		ClearHashes();
		m_sLocations.Empty();

		SendResponse( IDR_HTML_FILENOTFOUND, TRUE );
		theApp.Message( MSG_ERROR, IDS_UPLOAD_FILENOTFOUND, (LPCTSTR)m_sAddress, (LPCTSTR)m_sName );

		return TRUE;
	}

	if ( m_bRange )
	{
		if ( m_nOffset >= (QWORD)pDIME.m_nLength ) m_nLength = SIZE_UNKNOWN;
		else m_nLength = min( m_nLength, pDIME.m_nLength - m_nOffset );
	}
	else
	{
		m_nOffset = 0;
		m_nLength = (QWORD)pDIME.m_nLength;
	}

	if ( m_nLength <= pDIME.m_nLength )
	{
		CString strHeader;

		if ( m_nLength != pDIME.m_nLength )
			Write( _P("HTTP/1.1 206 OK\r\n") );
		else
			Write( _P("HTTP/1.1 200 OK\r\n") );

		SendDefaultHeaders();

		Write( _P("Content-Type: application/dime\r\n") );
		strHeader.Format( _T("Content-Length: %I64u\r\n"), m_nLength );
		Write( strHeader );

		if ( m_nLength != pDIME.m_nLength )
		{
			strHeader.Format( _T("Content-Range: %I64u-%I64u\r\n"), m_nOffset, m_nOffset + m_nLength - 1 );
			Write( strHeader );
		}

		Write( _P("\r\n") );

		LogOutgoing();

		if ( ! m_bHead )
		{
			Write( pDIME.m_pBuffer + m_nOffset, (DWORD)m_nLength );
		}

		StartSending( upsTigerTree );

		theApp.Message( MSG_INFO, IDS_UPLOAD_TIGER_SEND,
			(LPCTSTR)m_sName, (LPCTSTR)m_sAddress );
	}
	else
	{
		m_sRanges.Format( _T("0-%I64u"), (QWORD)pDIME.m_nLength - 1 );
		ClearHashes();
		m_sLocations.Empty();

		SendResponse( IDR_HTML_BADRANGE, TRUE );
		theApp.Message( MSG_ERROR, IDS_UPLOAD_BAD_RANGE, (LPCTSTR)m_sAddress, (LPCTSTR)m_sName );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP request preview

BOOL CUploadTransferHTTP::RequestPreview(CLibraryFile* pFile, CSingleLock& oLibraryLock)
{
	ASSERT( pFile != NULL );

	m_sName		= pFile->m_sName;
	m_sPath		= pFile->GetPath();
	m_oSHA1		= pFile->m_oSHA1;
	m_oTiger	= pFile->m_oTiger;
	m_oED2K		= pFile->m_oED2K;
	m_oBTH		= pFile->m_oBTH;
	m_oMD5		= pFile->m_oMD5;

	oLibraryLock.Unlock();

	DWORD nExisting = static_cast< DWORD >( Uploads.GetCount( this, upsPreview ) );

	if ( nExisting >= Settings.Uploads.PreviewTransfers )
	{
		theApp.Message( MSG_ERROR, IDS_UPLOAD_PREVIEW_BUSY, (LPCTSTR)m_sName, (LPCTSTR)m_sAddress );
		SendResponse( IDR_HTML_BUSY );
		return TRUE;
	}

	CImageFile pImage;
	if ( CThumbCache::Cache( m_sPath, &pImage, Settings.Uploads.DynamicPreviews ) )
	{
		theApp.Message( MSG_INFO, IDS_UPLOAD_PREVIEW_DYNAMIC, (LPCTSTR)m_sName, (LPCTSTR)m_sAddress );
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_UPLOAD_PREVIEW_EMPTY, (LPCTSTR)m_sAddress, (LPCTSTR)m_sName );
		SendResponse( IDR_HTML_FILENOTFOUND );
		return TRUE;
	}

	BYTE* pBuffer = NULL;
	DWORD nLength = 0;

	int nQuality = Settings.Uploads.PreviewQuality;

	if ( LPCTSTR pszQuality = _tcsistr( m_sRequest, _T("&quality=") ) )
	{
		_stscanf( pszQuality + 9, _T("%i"), &nQuality );
		nQuality = max( 1, min( 100, nQuality ) );
	}

	if ( ! pImage.SaveToMemory( _T(".jpg"), nQuality, &pBuffer, &nLength ) )
	{
		theApp.Message( MSG_ERROR, IDS_UPLOAD_PREVIEW_EMPTY, (LPCTSTR)m_sAddress, (LPCTSTR)m_sName );
		SendResponse( IDR_HTML_FILENOTFOUND );
		return TRUE;
	}

	Write( _P("HTTP/1.1 200 OK\r\n") );
	SendDefaultHeaders();

	CString strHeader;

	if ( m_oSHA1 )
	{
		strHeader.Format( _T("X-Previewed-URN: %s\r\n"),
			(LPCTSTR)m_oSHA1.toUrn() );
	}
	else if ( m_oTiger )
	{
		strHeader.Format( _T("X-Previewed-URN: %s\r\n"),
			(LPCTSTR)m_oTiger.toUrn() );
	}
	else if ( m_oED2K )
	{
		strHeader.Format( _T("X-Previewed-URN: %s\r\n"),
            (LPCTSTR)m_oED2K.toUrn() );
	}

	Write( strHeader );

	Write( _P("Content-Type: image/jpeg\r\n") );

	strHeader.Format( _T("Content-Length: %lu\r\n\r\n"), nLength );
	Write( strHeader );

	LogOutgoing();

	if ( ! m_bHead )
	{
		Write( pBuffer, nLength );
	}

	delete [] pBuffer;

	StartSending( upsPreview );

	theApp.Message( MSG_NOTICE, IDS_UPLOAD_PREVIEW_SEND, (LPCTSTR)m_sName,
		(LPCTSTR)m_sAddress );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP request host browse

BOOL CUploadTransferHTTP::RequestHostBrowse()
{
	CBuffer pBuffer;

	if ( m_nAccept == 0 )
	{
		// Create library file list
		CAutoPtr< CXMLElement > pXML( LibraryFolders.CreateXML( _T("/"), TRUE, xmlDefault ) );
		if ( ! pXML )
			// Out of memory
			return FALSE;

		pBuffer.Print( pXML->ToString( TRUE, TRUE, TRUE, TRI_TRUE ), CP_UTF8 );
	}
	else if ( m_nAccept == 1 )
	{
		if ( Settings.Community.ServeFiles )
		{
			CLocalSearch pSearch( &pBuffer, PROTOCOL_G1 );
			pSearch.Execute( 0 );
		}
	}
	else if ( m_nAccept == 2 )
	{
		if ( Settings.Community.ServeProfile && MyProfile.IsValid() )
		{
			CG2Packet* pProfile = CG2Packet::New( G2_PACKET_PROFILE_DELIVERY, TRUE );
			CString strXML = MyProfile.GetXML()->ToString( TRUE );
			pProfile->WritePacket( G2_PACKET_XML, pProfile->GetStringLen( strXML ) );
			pProfile->WriteString( strXML, FALSE );
			pProfile->ToBuffer( &pBuffer );
			pProfile->Release();
		}

		if ( Settings.Community.ServeFiles )
		{
			CLocalSearch pSearch( &pBuffer, PROTOCOL_G2 );
			pSearch.Execute( 0 );
		}

		if ( Settings.Community.ServeProfile && MyProfile.IsValid() )
		{
			if ( CG2Packet* pAvatar = MyProfile.CreateAvatar() )
			{
				pAvatar->ToBuffer( &pBuffer );
				pAvatar->Release();
			}
		}

		if ( Settings.Community.ChatEnable )
		{
			CG2Packet* pChat = CG2Packet::New( G2_PACKET_PEER_CHAT );
			pChat->ToBuffer( &pBuffer );
			pChat->Release();
		}
	}

	Write( _P("HTTP/1.1 200 OK\r\n") );
	SendDefaultHeaders();

	if ( m_nAccept == 0 )
	{
		Write( _P("Content-Type: text/xml\r\n") );
	}
	else if ( m_nAccept == 1 )
	{
		Write( _P("Content-Type: application/x-gnutella-packets\r\n") );
	}
	else if ( m_nAccept == 2 )
	{
		Write( _P("Content-Type: application/x-gnutella2\r\n") );
	}

	m_bDeflate = m_bDeflate && pBuffer.Deflate( TRUE );

	if ( m_bDeflate ) Write( _P("Content-Encoding: deflate\r\n") );

	CString strLength;
	strLength.Format( _T("Content-Length: %lu\r\n\r\n"), pBuffer.m_nLength );
	Write( strLength );

	LogOutgoing();

	CTransfer::OnWrite();

	if ( ! m_bHead ) Write( &pBuffer );

	StartSending( upsBrowse );

	theApp.Message( MSG_NOTICE, IDS_UPLOAD_BROWSE, (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP formatted response

void CUploadTransferHTTP::SendResponse(UINT nResourceID, BOOL bFileHeaders)
{
	m_nBandwidth = 0;

	CString strResponse;
	CString strBody = LoadRichHTML( nResourceID, strResponse, this );

	if ( strResponse.IsEmpty() )
		Write( _P("HTTP/1.1 200 OK\r\n") );
	else
		Write( _T("HTTP/1.1 ") + strResponse );

	SendDefaultHeaders();

	if ( bFileHeaders )
		SendFileHeaders();

	if ( nResourceID == IDR_HTML_BUSY )
		Write( _P("Retry-After: 30\r\n") );

	Write( _P("Content-Type: text/html\r\n") );

	CStringA strBodyUTF8 = UTF8Encode( strBody );

	CString strLength;
	strLength.Format( _T("Content-Length: %d\r\n\r\n"), strBodyUTF8.GetLength() );
	Write( strLength );

	LogOutgoing();

	if ( ! m_bHead ) Write( (LPCSTR)strBodyUTF8, strBodyUTF8.GetLength() );

	StartSending( upsResponse );
}
