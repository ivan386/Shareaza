//
// UploadTransferHTTP.cpp
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

#include "Network.h"
#include "Library.h"
#include "SharedFile.h"
#include "Downloads.h"
#include "Download.h"

#include "LocalSearch.h"
#include "ImageServices.h"
#include "ThumbCache.h"
#include "Neighbours.h"
#include "Neighbour.h"
#include "G2Packet.h"
#include "GProfile.h"
#include "Security.h"

#include "SHA.h"
#include "ED2K.h"
#include "TigerTree.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP construction

CUploadTransferHTTP::CUploadTransferHTTP() : CUploadTransfer( PROTOCOL_HTTP )
{
	m_bKeepAlive	= TRUE;
	m_nGnutella		= 0;
	m_nReaskMultiplier=1;
}

CUploadTransferHTTP::~CUploadTransferHTTP()
{
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP attach to connection

void CUploadTransferHTTP::AttachTo(CConnection* pConnection)
{
	CUploadTransfer::AttachTo( pConnection );
	
	theApp.Message( MSG_DEFAULT, IDS_UPLOAD_ACCEPTED, (LPCTSTR)m_sAddress );
	
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
	
	if ( ! m_pInput->ReadLine( strLine ) ) return TRUE;
	if ( strLine.GetLength() > 512 ) strLine = _T("#LINE_TOO_LONG#");
	
	if ( m_nState == upsQueued && m_pQueue != NULL )
	{
		DWORD tLimit = Settings.Uploads.QueuePollMin;

		tLimit *= m_nReaskMultiplier;
		
		if ( GetTickCount() - m_tRequest < tLimit )
		{
			theApp.Message( MSG_ERROR, IDS_UPLOAD_BUSY_FAST, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}
	}
	
	int nChar = strLine.Find( _T(" HTTP/") );
	
	if ( strLine.GetLength() < 14 || nChar < 5 ||
		 ( strLine.Left( 4 ) != _T("GET ") && strLine.Left( 5 ) != _T("HEAD ") ) )
	{
		theApp.Message( MSG_ERROR, IDS_UPLOAD_NOHTTP, (LPCTSTR)m_sAddress );
		Close();
		return FALSE;
	}
	
	ClearRequest();
	
	m_bHead			= ( strLine.Left( 5 ) == _T("HEAD ") );
	m_bConnectHdr	= FALSE;
	m_bKeepAlive	= TRUE;
	m_bHostBrowse	= FALSE;
	m_bDeflate		= FALSE;
	m_bBackwards	= FALSE;
	m_bRange		= FALSE;
	m_bQueueMe		= FALSE;
	
	m_bMetadata		= FALSE;
	m_bTigerTree	= FALSE;
	
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
	
	theApp.Message( MSG_DEBUG, _T("%s: UPLOAD PATH: %s"), (LPCTSTR)m_sAddress, (LPCTSTR)m_sRequest );
	
	m_nState	= upsHeaders;
	m_tRequest	= GetTickCount();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP read : headers

BOOL CUploadTransferHTTP::OnHeaderLine(CString& strHeader, CString& strValue)
{
	theApp.Message( MSG_DEBUG, _T("%s: UPLOAD HEADER: %s: %s"), (LPCTSTR)m_sAddress, (LPCTSTR)strHeader, strValue );
	
	if ( strHeader.CompareNoCase( _T("Connection") ) == 0 )
	{
		if ( strValue.CompareNoCase( _T("close") ) == 0 ) m_bKeepAlive = FALSE;
		m_bConnectHdr = TRUE;
	}
	else if ( strHeader.CompareNoCase( _T("Accept") ) == 0 )
	{
		strValue.MakeLower();
		if ( strValue.Find( _T("application/x-gnutella-packets") ) >= 0 ) m_bHostBrowse = 1;
		if ( strValue.Find( _T("application/x-gnutella2") ) >= 0 ) m_bHostBrowse = 2;
		if ( strValue.Find( _T("application/x-shareaza") ) >= 0 ) m_bHostBrowse = 2;
	}
	else if ( strHeader.CompareNoCase( _T("Accept-Encoding") ) == 0 )
	{
		if ( _tcsistr( strValue, _T("deflate") ) ) m_bDeflate = TRUE;
		if ( Settings.Uploads.AllowBackwards && _tcsistr( strValue, _T("backwards") ) ) m_bBackwards = TRUE;
	}
	else if ( strHeader.CompareNoCase( _T("Range") ) == 0 )
	{
		QWORD nFrom = 0, nTo = 0;
		
		if ( _stscanf( strValue, _T("bytes=%I64i-%I64i"), &nFrom, &nTo ) == 2 )
		{
			m_nOffset	= nFrom;
			m_nLength	= nTo + 1 - nFrom;
			m_bRange	= TRUE;
		}
		else if ( _stscanf( strValue, _T("bytes=%I64i-"), &nFrom ) == 1 )
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
		{
			if ( strValue.Find( _T("Zhttp://") ) < 0 ) m_sLocations = strValue;
		}
		m_nGnutella |= 1;
	}
	else if ( strHeader.CompareNoCase( _T("X-Queue") ) == 0 )
	{
		m_bQueueMe = TRUE;
		if ( strValue == _T("1.0") ) m_bQueueMe = (BOOL)2;
		m_nGnutella |= 1;
	}
	else if (	strHeader.CompareNoCase( _T("X-Nick") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Name") ) == 0 ||
				strHeader.CompareNoCase( _T("X-UserName") ) == 0 )
	{
		m_sNick = URLDecode( strValue );
	}
	else if ( strHeader.CompareNoCase( _T("X-Features") ) == 0 )
	{
		if ( _tcsistr( strValue, _T("g2/") ) != NULL ) m_nGnutella |= 2;
		if ( _tcsistr( strValue, _T("gnet2/") ) != NULL ) m_nGnutella |= 2;
		if ( _tcsistr( strValue, _T("gnutella2/") ) != NULL ) m_nGnutella |= 2;
		if ( m_nGnutella == 0 ) m_nGnutella = 1;
	}
	
	return CUploadTransfer::OnHeaderLine( strHeader, strValue );
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP process request

BOOL CUploadTransferHTTP::OnHeadersComplete()
{
	if ( Uploads.EnforcePerHostLimit( this, TRUE ) ) return FALSE;
	
	if ( _tcsistr( m_sUserAgent, _T("shareaza") ) != NULL )
	{
		// Assume certain capabilitites for various Shareaza versions
		m_nGnutella |= 3;
		if ( m_bQueueMe == (BOOL)2 ) m_nGnutella = 1;	// GTK
		if ( m_sUserAgent == _T("Shareaza 1.4.0.0") ) m_bQueueMe = TRUE;
	}
	else if ( _tcsistr( m_sUserAgent, _T("trustyfiles") ) != NULL ||
			  _tcsistr( m_sUserAgent, _T("gnucdna") ) != NULL ||
			  _tcsistr( m_sUserAgent, _T("adagio") ) != NULL )
	{
		// Assume Gnutella2 capability for certain user-agents
		m_nGnutella |= 3;
	}
	
	if ( m_sRequest == _T("/") || StartsWith( m_sRequest, _T("/gnutella/browse/v1") ) )
	{
		// Requests for "/" or the browse path are handled the same way
		
		if ( ( m_bHostBrowse == 1 && ! Settings.Community.ServeFiles ) ||
			 ( m_bHostBrowse == 2 && ! Settings.Community.ServeProfile && ! Settings.Community.ServeFiles ) )
		{
			theApp.Message( MSG_ERROR, IDS_UPLOAD_BROWSE_DENIED, (LPCTSTR)m_sAddress );
			m_bHostBrowse = FALSE;
		}
		
		if ( m_bHostBrowse )
		{
			RequestHostBrowse();
		}
		else
		{
			theApp.Message( MSG_DEFAULT, IDS_UPLOAD_ABOUT, (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );
			SendResponse( IDR_HTML_ABOUT );
		}
		
		return TRUE;
	}
	else if ( StartsWith( m_sRequest, _T("/remote") ) )
	{
		// A web client can start requesting remote pages on the same keep-alive
		// connection after previously requesting other system objects
		
		if ( Settings.Remote.Enable )
		{
			m_pInput->Prefix( "GET /remote/ HTTP/1.0\r\n\r\n" );
			new CRemote( this );
			Remove( FALSE );
			return FALSE;
		}
	}
	else if ( IsAgentBlocked() )
	{
		if ( m_sFileName.IsEmpty() ) m_sFileName = _T("file");
		SendResponse( IDR_HTML_BROWSER );
		theApp.Message( MSG_ERROR, IDS_UPLOAD_BROWSER, (LPCTSTR)m_sAddress, (LPCTSTR)m_sFileName );
		Security.TempBlock( &m_pHost.sin_addr ); //Anti-hammer protection if client doesn't understand 403 (Don't bother re-sending HTML every 5 seconds)
		if ( m_sUserAgent.Find( _T("Mozilla") ) >= 0 ) return TRUE;
		Remove( FALSE );
		return FALSE;
	}
	else if ( IsNetworkDisabled() )
	{
		SendResponse( IDR_HTML_DISABLED );
		theApp.Message( MSG_ERROR, IDS_UPLOAD_DISABLED, (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );
		Security.TempBlock( &m_pHost.sin_addr ); //Anti-hammer protection if client doesn't understand 403
		Remove( FALSE );
		return FALSE;
	}
	else if ( StartsWith( m_sRequest, _T("/gnutella/metadata/v1?urn:") ) && Settings.Uploads.ShareMetadata )
	{
		LPCTSTR pszURN = (LPCTSTR)m_sRequest + 22;
		CXMLElement* pMetadata = NULL;
		
		if ( CLibraryFile* pShared = LibraryMaps.LookupFileByURN( pszURN, TRUE, TRUE, TRUE ) )
		{
			if ( pShared->m_pMetadata != NULL )
			{
				m_sFileName	= pShared->m_sName;
				pMetadata	= pShared->m_pSchema->Instantiate( TRUE );
				pMetadata->AddElement( pShared->m_pMetadata->Clone() );
			}
			Library.Unlock();
		}
		else if ( CDownload* pDownload = Downloads.FindByURN( pszURN ) )
		{
			if ( pDownload->m_pXML != NULL )
			{
				m_sFileName	= pDownload->m_sRemoteName;
				pMetadata	= pDownload->m_pXML->Clone();
			}
		}
		
		if ( pMetadata != NULL ) return RequestMetadata( pMetadata );
	}
	else if ( StartsWith( m_sRequest, _T("/gnutella/tigertree/v3?urn:") ) && Settings.Uploads.ShareTiger )
	{
		LPCTSTR pszURN = (LPCTSTR)m_sRequest + 23;
		
		if ( CLibraryFile* pShared = LibraryMaps.LookupFileByURN( pszURN, TRUE, TRUE, TRUE ) )
		{
			CTigerTree* pTigerTree = pShared->GetTigerTree();
			m_sFileName = pShared->m_sName;
			Library.Unlock();
			return RequestTigerTreeRaw( pTigerTree, TRUE );
		}
		else if ( CDownload* pDownload = Downloads.FindByURN( pszURN ) )
		{
			if ( pDownload->GetTigerTree() != NULL )
			{
				m_sFileName = pDownload->m_sRemoteName;
				return RequestTigerTreeRaw( pDownload->GetTigerTree(), FALSE );
			}
		}
	}
	else if ( StartsWith( m_sRequest, _T("/gnutella/thex/v1?urn:") ) && Settings.Uploads.ShareTiger )
	{
		LPCTSTR pszURN	= (LPCTSTR)m_sRequest + 18;
		DWORD nDepth	= 0;
		
		if ( LPCTSTR pszDepth = _tcsistr( m_sRequest, _T("depth=") ) )
		{
			_stscanf( pszDepth + 6, _T("%i"), &nDepth );
		}
		
		BOOL bHashset = ( _tcsistr( m_sRequest, _T("ed2k=1") ) != NULL );
		
		if ( CLibraryFile* pShared = LibraryMaps.LookupFileByURN( pszURN, TRUE, TRUE, TRUE ) )
		{
			CTigerTree* pTigerTree	= pShared->GetTigerTree();
			CED2K* pHashset			= bHashset ? pShared->GetED2K() : NULL;
			m_sFileName = pShared->m_sName;
			m_nFileSize = pShared->GetSize();
			Library.Unlock();
			return RequestTigerTreeDIME( pTigerTree, nDepth, pHashset, TRUE );
		}
		else if ( CDownload* pDownload = Downloads.FindByURN( pszURN ) )
		{
			if ( pDownload->GetTigerTree() != NULL )
			{
				m_sFileName = pDownload->m_sRemoteName;
				m_nFileSize = pDownload->m_nSize;
				return RequestTigerTreeDIME( pDownload->GetTigerTree(), nDepth,
					bHashset ? pDownload->GetHashset() : NULL, FALSE );
			}
		}
	}
	else if ( StartsWith( m_sRequest, _T("/gnutella/preview/v1?urn:") ) && Settings.Uploads.SharePreviews )
	{
		LPCTSTR pszURN = (LPCTSTR)m_sRequest + 21;
		CLibraryFile* pShared = LibraryMaps.LookupFileByURN( pszURN, TRUE, TRUE, TRUE );
		if ( pShared != NULL ) return RequestPreview( pShared );
	}
	else if ( StartsWith( m_sRequest, _T("/uri-res/N2R?urn:") ) )
	{
		LPCTSTR pszURN = (LPCTSTR)m_sRequest + 13;
		
		if ( CLibraryFile* pShared = LibraryMaps.LookupFileByURN( pszURN, TRUE, TRUE, TRUE ) )
		{
			return RequestSharedFile( pShared );
		}
		
		CDownload* pDownload = Downloads.FindByURN( pszURN );
		
		if ( pDownload != NULL && pDownload->IsShared() && pDownload->IsStarted() )
		{
			return RequestPartialFile( pDownload );
		}
	}
	else if ( StartsWith( m_sRequest, _T("/get/") ) )
	{
		DWORD nIndex = 0;
		
		CString strFile	= m_sRequest.Mid( 5 );
		int nChar		= strFile.Find( '/' );
		
		if ( _stscanf( strFile, _T("%lu/"), &nIndex ) == 1 && nChar > 0 && nChar < strFile.GetLength() - 1 )
		{
			strFile = strFile.Mid( nChar + 1 );
			
			CLibraryFile* pFile = Library.LookupFile( nIndex, TRUE, TRUE, TRUE );
			
			if ( pFile != NULL && pFile->m_sName.CompareNoCase( strFile ) )
			{
				Library.Unlock();
				pFile = NULL;
			}
			
			if ( pFile == NULL )
			{
				pFile = LibraryMaps.LookupFileByName( strFile, TRUE, TRUE, TRUE );
			}
			
			if ( pFile != NULL ) return RequestSharedFile( pFile );
		}
		else
		{
			strFile = strFile.Mid( nChar + 1 );
			CLibraryFile* pFile = LibraryMaps.LookupFileByName( strFile, TRUE, TRUE, TRUE );
			if ( pFile != NULL ) return RequestSharedFile( pFile );
		}
	}
	else
	{
		CString strFile = m_sRequest.Mid( 1 );
		CLibraryFile* pFile = LibraryMaps.LookupFileByName( strFile, TRUE, TRUE, TRUE );
		if ( pFile != NULL ) return RequestSharedFile( pFile );
	}
	
	if ( m_sFileName.IsEmpty() )
	{
		m_sFileName = m_oSHA1.ToURN();
	}
	
	SendResponse( IDR_HTML_FILENOTFOUND );
	theApp.Message( MSG_ERROR, IDS_UPLOAD_FILENOTFOUND, (LPCTSTR)m_sAddress, (LPCTSTR)m_sFileName );
	
	return TRUE;
}

BOOL CUploadTransferHTTP::IsNetworkDisabled()
{
	if ( Settings.Connection.RequireForTransfers == FALSE ) return FALSE;
	
	if ( m_nGnutella == 2 )
	{
		if ( ! Settings.Gnutella2.EnableToday ) return TRUE;
	}
	else if ( m_nGnutella == 1 )
	{
		if ( ! Settings.Gnutella1.EnableToday ) return TRUE;
	}
	else
	{
		if ( ! Settings.Gnutella1.EnableToday &&
			 ! Settings.Gnutella2.EnableToday ) return TRUE;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP request a shared file

BOOL CUploadTransferHTTP::RequestSharedFile(CLibraryFile* pFile)
{
	ASSERT( pFile != NULL );
	
	if ( ! RequestComplete( pFile ) )
	{
		Library.Unlock();
		SendResponse( IDR_HTML_HASHMISMATCH );
		theApp.Message( MSG_ERROR, IDS_UPLOAD_HASH_MISMATCH, (LPCTSTR)m_sAddress, (LPCTSTR)m_sFileName );
		return TRUE;
	}
	
	m_bTigerTree	= m_oTiger.IsValid();
	m_bMetadata		= ( pFile->m_pMetadata != NULL && ( pFile->m_bMetadataAuto == FALSE || pFile->m_nVirtualSize > 0 ) );
	
	if ( ! m_oSHA1.IsValid() && ! m_oTiger.IsValid() && ! m_oED2K.IsValid() ) m_sLocations.Empty();
	
	if ( m_nLength == SIZE_UNKNOWN ) m_nLength = m_nFileSize - m_nOffset;
	
	if ( m_nOffset >= m_nFileSize || m_nOffset + m_nLength > m_nFileSize )
	{
		Library.Unlock();
		SendResponse( IDR_HTML_BADRANGE );
		theApp.Message( MSG_ERROR, IDS_UPLOAD_BAD_RANGE, (LPCTSTR)m_sAddress, (LPCTSTR)m_sFileName );
		return TRUE;
	}
	
	CString strLocations;
	if ( Settings.Library.SourceMesh ) strLocations = pFile->GetAlternateSources( &m_pSourcesSent, 15, TRUE );
	if ( m_sLocations.GetLength() ) pFile->AddAlternateSources( m_sLocations );
	m_sLocations = strLocations;
	
	Library.Unlock();
	
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
		theApp.Message( MSG_ERROR, IDS_UPLOAD_HASH_MISMATCH, (LPCTSTR)m_sAddress, (LPCTSTR)m_sFileName );
		return TRUE;
	}
	
	ASSERT( m_nFileBase == 0 );
	
	m_bTigerTree	= ( m_oTiger.IsValid() && pDownload->GetTigerTree() != NULL );
	m_bMetadata		= ( pDownload->m_pXML != NULL );
	
	if ( m_sLocations.GetLength() ) pDownload->AddSourceURLs( m_sLocations, TRUE );
	if ( Settings.Library.SourceMesh ) m_sLocations = pDownload->GetSourceURLs( &m_pSourcesSent, 15, TRUE, NULL );
	
	m_sRanges = pDownload->GetAvailableRanges();
	
	if ( m_bRange && m_nOffset == 0 && m_nLength == SIZE_UNKNOWN )
	{
		pDownload->GetRandomRange( m_nOffset, m_nLength );
	}
	
	if ( m_nLength == SIZE_UNKNOWN ) m_nLength = m_nFileSize - m_nOffset;
	
	if ( pDownload->ClipUploadRange( m_nOffset, m_nLength ) )
	{
		return QueueRequest();
	}
	
	if ( pDownload->IsMoving() )
	{
		if ( GetTickCount() - pDownload->m_tCompleted < 30000 )
		{
			m_pOutput->Print( "HTTP/1.1 503 Range Temporarily Unavailable\r\n" );
		}
		else
		{
			SendResponse( IDR_HTML_FILENOTFOUND );
			theApp.Message( MSG_ERROR, IDS_UPLOAD_FILENOTFOUND, (LPCTSTR)m_sAddress, (LPCTSTR)m_sFileName );
			return TRUE;
		}
	}
	else if ( pDownload->GetTransferCount() )
	{
		m_pOutput->Print( "HTTP/1.1 503 Range Temporarily Unavailable\r\n" );
	}
	else
	{
		m_pOutput->Print( "HTTP/1.1 416 Requested Range Unavailable\r\n" );
	}
	
	SendDefaultHeaders();
	SendFileHeaders();
	
	m_pOutput->Print( "Content-Length: 0\r\n" );
	m_pOutput->Print( "\r\n" );
	
	StartSending( upsResponse );
	
	theApp.Message( MSG_DEFAULT, IDS_UPLOAD_BAD_RANGE, (LPCTSTR)m_sAddress, (LPCTSTR)m_sFileName );
	
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
	
	if ( Uploads.AllowMoreTo( &m_pHost.sin_addr ) )
	{
		if ( ( nPosition = UploadQueues.GetPosition( this, TRUE ) ) >= 0 )
		{
			ASSERT( m_pQueue != NULL );
			ASSERT( m_pQueue->CanAccept( m_nProtocol, m_sFileName, m_nFileSize, m_bFilePartial, m_sFileTags ) );
			
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
			ASSERT( m_pQueue->CanAccept( m_nProtocol, m_sFileName, m_nFileSize, m_bFilePartial, m_sFileTags ) );
			
			nPosition = UploadQueues.GetPosition( this, TRUE );
			ASSERT( nPosition >= 0 );
			
			if ( nPosition == 0 )
			{
				// Queued, and ready to send
				return OpenFileSendHeaders();
			}
			else if ( m_bQueueMe )
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
		
		m_pOutput->Print( "HTTP/1.1 503 Busy Queued\r\n" );
		
		SendDefaultHeaders();
		SendFileHeaders();
		
		m_nReaskMultiplier=( nPosition <= 9 ) ? ( (nPosition+1) / 2 ) : 5;
		DWORD nTimeScale = 1000 / m_nReaskMultiplier;
		
		CSingleLock pLock( &UploadQueues.m_pSection, TRUE );
		
		if ( UploadQueues.Check( m_pQueue ) )
		{
			strName = m_pQueue->m_sName;
			Replace( strName, _T("\""), _T("'") );
			
			strHeader.Format( _T("X-Queue: position=%i,length=%i,limit=%i,pollMin=%lu,pollMax=%lu,id=\"%s\"\r\n"),
				nPosition,
				m_pQueue->GetQueuedCount(),
				m_pQueue->GetTransferCount( TRUE ),
				Settings.Uploads.QueuePollMin / nTimeScale,
				Settings.Uploads.QueuePollMax / nTimeScale,
				(LPCTSTR)strName );
			
			theApp.Message( MSG_DEFAULT, IDS_UPLOAD_QUEUED, (LPCTSTR)m_sFileName,
				(LPCTSTR)m_sAddress, nPosition, m_pQueue->GetQueuedCount(),
				(LPCTSTR)strName );

		}
		
		pLock.Unlock();
		
		m_pOutput->Print( strHeader );
		m_pOutput->Print( "Content-Length: 0\r\n" );
		m_pOutput->Print( "\r\n" );
		
		StartSending( upsPreQueue );
	}
	else
	{
		SendResponse( IDR_HTML_BUSY, TRUE );
		
		if ( ! nError ) nError = m_bQueueMe ? IDS_UPLOAD_BUSY_QUEUE : IDS_UPLOAD_BUSY_OLD;
		theApp.Message( MSG_ERROR, nError, (LPCTSTR)m_sFileName, (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP default response headers

void CUploadTransferHTTP::SendDefaultHeaders()
{
	CString strLine = Settings.SmartAgent( Settings.General.UserAgent );
	
	if ( strLine.GetLength() )
	{
		strLine = _T("Server: ") + strLine + _T("\r\n");
		m_pOutput->Print( strLine );
	}
	
	if ( ! m_bInitiated )
	{
		strLine.Format( _T("Remote-IP: %s\r\n"),
			(LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ) );
		m_pOutput->Print( strLine );
	}
	
	if ( m_bKeepAlive )
	{
		m_pOutput->Print( "Connection: Keep-Alive\r\n" );
	}
	else
	{
		m_pOutput->Print( "Connection: Close\r\n" );
	}
	
	m_pOutput->Print( "Accept-Ranges: bytes\r\n" );
	
	if ( m_nRequests <= 1 )
	{
		if ( m_bInitiated ) SendMyAddress();
		strLine.Format( _T("X-PerHost: %lu\r\n"), Settings.Uploads.MaxPerHost );
		m_pOutput->Print( strLine );
		
		strLine = MyProfile.GetNick();
		
		if ( strLine.GetLength() > 0 )
		{
			strLine = _T("X-Nick: ") + URLEncode( strLine ) + _T("\r\n");
			m_pOutput->Print( strLine );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP file response headers

void CUploadTransferHTTP::SendFileHeaders()
{
	CString strHeader;
	
	if ( m_oSHA1.IsValid() )
	{
		if ( m_oTiger.IsValid() )
		{
			strHeader	= _T("X-Content-URN: urn:bitprint:")
				+ m_oSHA1.ToString() + '.'
				+ m_oTiger.ToString() + _T("\r\n");
		}
		else
		{
			strHeader = _T("X-Content-URN: ") + m_oSHA1.ToURN() + _T("\r\n");
		}
		
		m_pOutput->Print( strHeader );
	}
	else if ( m_oTiger.IsValid() )
	{
		strHeader = _T("X-Content-URN: ") + m_oTiger.ToURN() + _T("\r\n");
		m_pOutput->Print( strHeader );
	}
	
	if ( m_oED2K.IsValid() )
	{
		strHeader = _T("X-Content-URN: ") + m_oED2K.ToURN() + _T("\r\n");
		m_pOutput->Print( strHeader );
	}
	
	if ( m_bTigerTree && Settings.Uploads.ShareTiger )
	{
		strHeader	= _T("X-Thex-URI: /gnutella/thex/v1?")
			+ m_oTiger.ToURN()
			+ _T("&depth=9&ed2k=0\r\n");
		m_pOutput->Print( strHeader );
	}
	
	if ( m_bMetadata )
	{
		strHeader	= _T("X-Metadata-Path: /gnutella/metadata/v1?")
			+ m_oTiger.ToURN()
			+ _T("\r\n");
		m_pOutput->Print( strHeader );
	}
	
	if ( m_sRanges.GetLength() )
	{
		strHeader = _T("X-Available-Ranges: ") + m_sRanges + _T("\r\n");
		m_pOutput->Print( strHeader );
	}
	
	if ( m_sLocations.GetLength() )
	{
		strHeader = _T("Alt-Location: ") + m_sLocations + _T("\r\n");
		m_pOutput->Print( strHeader );
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP open file and send headers

BOOL CUploadTransferHTTP::OpenFileSendHeaders()
{
	ASSERT( m_pDiskFile == NULL );
	
	m_pDiskFile = TransferFiles.Open( m_sFilePath, FALSE, FALSE );
	
	if ( m_pDiskFile == NULL )
	{
		SendResponse( IDR_HTML_FILENOTFOUND );
		theApp.Message( MSG_ERROR, IDS_UPLOAD_CANTOPEN, (LPCTSTR)m_sFileName, (LPCTSTR)m_sAddress );
		return TRUE;
	}
	
	CSingleLock pLock( &UploadQueues.m_pSection, TRUE );
	
	if ( m_pQueue != NULL && UploadQueues.Check( m_pQueue ) && m_pQueue->m_bRotate )
	{
		DWORD nLimit = m_pQueue->m_nRotateChunk;
		if ( nLimit == 0 ) nLimit = Settings.Uploads.RotateChunkLimit;
		if ( nLimit > 0 ) m_nLength = min( m_nLength, nLimit );
	}
	
	pLock.Unlock();
	
	if ( m_nLength != m_nFileSize )
		m_pOutput->Print( "HTTP/1.1 206 OK\r\n" );
	else
		m_pOutput->Print( "HTTP/1.1 200 OK\r\n" );
	
	SendDefaultHeaders();
	
	CString strExt, strResponse;
	
	int nType = m_sFileName.ReverseFind( '.' );
	if ( nType > 0 ) strExt = m_sFileName.Mid( nType );
	ShellIcons.Lookup( strExt, NULL, NULL, NULL, &strResponse );
	
	if ( strResponse.IsEmpty() )
	{
		m_pOutput->Print( "Content-Type: application/x-binary\r\n" );
	}
	else
	{
		strResponse = _T("Content-Type: ") + strResponse + _T("\r\n");
		m_pOutput->Print( strResponse );
	}
	
	strResponse.Format( _T("Content-Length: %I64i\r\n"), m_nLength );
	m_pOutput->Print( strResponse );
	
	if ( m_nLength != m_nFileSize )
	{
		strResponse.Format( _T("Content-Range: bytes=%I64i-%I64i/%I64i\r\n"), m_nOffset, m_nOffset + m_nLength - 1, m_nFileSize );
		m_pOutput->Print( strResponse );
	}
	
	if ( ! m_bHead && m_bBackwards )
	{
		m_pOutput->Print( "Content-Encoding: backwards\r\n" );
	}
	
	if ( m_oSHA1.IsValid() || m_oTiger.IsValid() || m_oED2K.IsValid() ) SendFileHeaders();
	
	m_pOutput->Print( "\r\n" );
	
	if ( m_bHead )
	{
		m_pDiskFile->Release( FALSE );
		m_pDiskFile = NULL;
		
		theApp.Message( MSG_DEFAULT, IDS_UPLOAD_HEADERS, (LPCTSTR)m_sFileName,
			(LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );
		
		StartSending( upsResponse );
	}
	else
	{
		if ( m_pBaseFile->m_nRequests++ == 0 )
		{
			theApp.Message( MSG_SYSTEM, IDS_UPLOAD_FILE,
				(LPCTSTR)m_sFileName, (LPCTSTR)m_sAddress );
			
			if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( m_sFilePath, TRUE, TRUE, TRUE ) )
			{
				pFile->m_nUploadsToday++;
				pFile->m_nUploadsTotal++;
				Library.Unlock();
			}
		}
		
		theApp.Message( MSG_DEFAULT,
			m_sRanges.GetLength() ? IDS_UPLOAD_PARTIAL_CONTENT : IDS_UPLOAD_CONTENT,
			m_nOffset, m_nOffset + m_nLength - 1, (LPCTSTR)m_sFileName,
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
	if ( m_nState == upsUploading && m_pDiskFile != NULL && m_pOutput->m_nLength == 0 )
	{
		if ( m_nPosition >= m_nLength )
		{
			OnCompleted();
			CUploadTransfer::OnWrite();
			return TRUE;
		}
		
		QWORD nPacket = min( m_nLength - m_nPosition, (QWORD)Transfers.m_nBuffer );
		BYTE* pBuffer = Transfers.m_pBuffer;
		
		if ( m_bBackwards )
		{
			QWORD nRead = 0;
			m_pDiskFile->Read( m_nFileBase + m_nOffset + m_nLength - m_nPosition - nPacket, pBuffer, nPacket, &nRead );
			if ( nRead != nPacket ) return TRUE;
			m_pOutput->AddReversed( pBuffer, (DWORD)nPacket );
		}
		else
		{
			m_pDiskFile->Read( m_nFileBase + m_nOffset + m_nPosition, pBuffer, nPacket, &nPacket );
			if ( nPacket == 0 ) return TRUE;
			m_pOutput->Add( pBuffer, (DWORD)nPacket );
		}
		
		m_nPosition += nPacket;
		m_nUploaded += nPacket;
		
		Statistics.Current.Uploads.Volume += nPacket;
	}
	
	CUploadTransfer::OnWrite();
	
	if ( m_nState >= upsResponse && m_pOutput->m_nLength == 0 )
	{
		m_nState	= ( m_nState == upsPreQueue ) ? upsQueued : upsRequest;
		m_tRequest	= GetTickCount();
	}
	
	return TRUE;
}

void CUploadTransferHTTP::OnCompleted()
{
	Uploads.SetStable( GetAverageSpeed() );
	
	m_pDiskFile->Release( FALSE );
	m_pDiskFile	= NULL;
	m_nState	= upsRequest;
	m_tRequest	= GetTickCount();
	
	m_pBaseFile->AddFragment( m_nOffset, m_nLength );
	// m_pBaseFile = NULL;
	
	theApp.Message( MSG_DEFAULT, IDS_UPLOAD_FINISHED, (LPCTSTR)m_sFileName, (LPCTSTR)m_sAddress );
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
		if ( ! m_bKeepAlive && m_pOutput->m_nLength == 0 )
		{
			theApp.Message( MSG_DEFAULT, IDS_UPLOAD_DROPPED, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}

	case upsHeaders:
		if ( tNow - m_tRequest > Settings.Connection.TimeoutHandshake )
		{
			theApp.Message( MSG_ERROR, IDS_UPLOAD_REQUEST_TIMEOUT, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}
		break;

	case upsQueued:
		if ( tNow - m_tRequest > ( Settings.Uploads.QueuePollMax * m_nReaskMultiplier ) )
		{
			theApp.Message( MSG_ERROR, IDS_UPLOAD_REQUEST_TIMEOUT, (LPCTSTR)m_sAddress );
			Close();
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
			theApp.Message( MSG_ERROR, IDS_UPLOAD_TRAFFIC_TIMEOUT, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}
		break;
		
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP dropped handler

void CUploadTransferHTTP::OnDropped(BOOL bError)
{
	theApp.Message( MSG_DEFAULT, IDS_UPLOAD_DROPPED, (LPCTSTR)m_sAddress );
	
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
	
	Close();
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP request metadata

BOOL CUploadTransferHTTP::RequestMetadata(CXMLElement* pMetadata)
{
	ASSERT( pMetadata != NULL );
	CString strXML = pMetadata->ToString( TRUE, TRUE );
	delete pMetadata;
	
#ifdef _UNICODE
	int nXML = WideCharToMultiByte( CP_UTF8, 0, strXML, strXML.GetLength(), NULL, 0, NULL, NULL );
	LPSTR pszXML = new CHAR[ nXML ];
	WideCharToMultiByte( CP_UTF8, 0, strXML, strXML.GetLength(), pszXML, nXML, NULL, NULL );
#else
	int nXML = strXML.GetLength();
	LPCSTR pszXML = (LPCSTR)strXML;
#endif
	
	m_pOutput->Print( "HTTP/1.1 200 OK\r\n" );
	SendDefaultHeaders();
	m_pOutput->Print( "Content-Type: text/xml\r\n" );
	
	CString strHeader;
	strHeader.Format( _T("Content-Length: %lu\r\n"), nXML );
	m_pOutput->Print( strHeader );
	m_pOutput->Print( "\r\n" );
	
	if ( ! m_bHead ) m_pOutput->Add( pszXML, nXML );
#ifdef _UNICODE
	delete [] pszXML;
#endif
	
	StartSending( upsMetadata );
	
	theApp.Message( MSG_DEFAULT, IDS_UPLOAD_METADATA_SEND,
		(LPCTSTR)m_sFileName, (LPCTSTR)m_sAddress );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP request a tiger tree hash, raw format

BOOL CUploadTransferHTTP::RequestTigerTreeRaw(CTigerTree* pTigerTree, BOOL bDelete)
{
	if ( pTigerTree == NULL )
	{
		ClearHashes();
		m_sLocations.Empty();
		
		SendResponse( IDR_HTML_FILENOTFOUND, TRUE );
		theApp.Message( MSG_ERROR, IDS_UPLOAD_FILENOTFOUND, (LPCTSTR)m_sAddress, (LPCTSTR)m_sFileName );
		
		return TRUE;
	}
	
	LPBYTE pSerialTree;
	DWORD nSerialTree;
	
	pTigerTree->ToBytes( pSerialTree, nSerialTree );
	if ( bDelete ) delete pTigerTree;
	
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
			m_pOutput->Print( "HTTP/1.1 206 OK\r\n" );
		else
			m_pOutput->Print( "HTTP/1.1 200 OK\r\n" );
		
		SendDefaultHeaders();
		
		m_pOutput->Print( "Content-Type: application/tigertree-breadthfirst\r\n" );
		strHeader.Format( _T("Content-Length: %I64i\r\n"), m_nLength );
		m_pOutput->Print( strHeader );
		
		if ( m_nLength != nSerialTree )
		{
			strHeader.Format( _T("Content-Range: %I64i-%I64i\r\n"), m_nOffset, m_nOffset + m_nLength - 1 );
			m_pOutput->Print( strHeader );
		}
		
		m_pOutput->Print( "\r\n" );
		
		if ( ! m_bHead ) m_pOutput->Add( pSerialTree + m_nOffset, (DWORD)m_nLength );
		
		StartSending( upsTigerTree );
		
		theApp.Message( MSG_DEFAULT, IDS_UPLOAD_TIGER_SEND,
			(LPCTSTR)m_sFileName, (LPCTSTR)m_sAddress );
	}
	else
	{
		m_sRanges.Format( _T("0-%I64i"), (QWORD)nSerialTree - 1 );
		ClearHashes();
		m_sLocations.Empty();
		
		SendResponse( IDR_HTML_BADRANGE, TRUE );
		theApp.Message( MSG_ERROR, IDS_UPLOAD_BAD_RANGE, (LPCTSTR)m_sAddress, (LPCTSTR)m_sFileName );
	}
	
	delete [] pSerialTree;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP request a tiger tree hash, DIME format

BOOL CUploadTransferHTTP::RequestTigerTreeDIME(CTigerTree* pTigerTree, int nDepth, CED2K* pHashset, BOOL bDelete)
{
	if ( pTigerTree == NULL )
	{
		ClearHashes();
		m_sLocations.Empty();
		
		SendResponse( IDR_HTML_FILENOTFOUND, TRUE );
		theApp.Message( MSG_ERROR, IDS_UPLOAD_FILENOTFOUND, (LPCTSTR)m_sAddress, (LPCTSTR)m_sFileName );
		
		if ( pHashset != NULL && bDelete ) delete pHashset;
		
		return TRUE;
	}
	
	DWORD nSerialTree;
	LPBYTE pSerialTree;
	CBuffer pDIME;
	
	if ( nDepth < 1 ) nDepth = pTigerTree->GetHeight();
	else if ( nDepth > (int)pTigerTree->GetHeight() ) nDepth = pTigerTree->GetHeight();
	
	pTigerTree->ToBytes( pSerialTree, nSerialTree, nDepth );
	if ( bDelete ) delete pTigerTree;
	
	CString strUUID, strXML;
	GUID pUUID;
	
	Network.CreateID( (CGUID*)&pUUID );
	strUUID.Format( _T("uuid:%.8x-%.4x-%.4x-%.2x%.2x-%.2x%.2x%.2x%.2x%.2x%.2x"),
		pUUID.Data1, pUUID.Data2, pUUID.Data3,
		pUUID.Data4[0], pUUID.Data4[1], pUUID.Data4[2], pUUID.Data4[3],
		pUUID.Data4[4], pUUID.Data4[5], pUUID.Data4[6], pUUID.Data4[7] );
	
	strXML.Format(	_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n")
					_T("<!DOCTYPE hashtree SYSTEM \"http://open-content.net/spec/thex/thex.dtd\">\r\n")
					_T("<hashtree>\r\n")
					_T("\t<file size=\"%I64i\" segmentsize=\"1024\"/>\r\n")
					_T("\t<digest algorithm=\"http://open-content.net/spec/digest/tiger\" outputsize=\"24\"/>\r\n")
					_T("\t<serializedtree depth=\"%i\" type=\"http://open-content.net/spec/thex/breadthfirst\" uri=\"%s\"/>\r\n")
					_T("</hashtree>"),
					m_nFileSize, nDepth, (LPCTSTR)strUUID );
	
#ifdef _UNICODE
	int nXML = WideCharToMultiByte( CP_UTF8, 0, strXML, -1, NULL, 0, NULL, NULL );
	LPSTR pszXML = new CHAR[ nXML ];
	WideCharToMultiByte( CP_UTF8, 0, strXML, -1, pszXML, nXML, NULL, NULL );
	int nUUID = WideCharToMultiByte( CP_ACP, 0, strUUID, -1, NULL, 0, NULL, NULL );
	LPSTR pszUUID = new CHAR[ nUUID ];
	WideCharToMultiByte( CP_ACP, 0, strUUID, -1, pszUUID, nUUID, NULL, NULL );
#else
	LPCSTR pszXML	= strXML;
	LPCSTR pszUUID	= strUUID;
#endif
	
	pDIME.WriteDIME( 1, "", "text/xml", pszXML, strlen(pszXML) );
	pDIME.WriteDIME( pHashset ? 0 : 2, pszUUID, "http://open-content.net/spec/thex/breadthfirst", pSerialTree, nSerialTree );
	delete [] pSerialTree;
	
#ifdef _UNICODE
	delete [] pszUUID;
	delete [] pszXML;
#endif
	
	if ( pHashset != NULL )
	{
		pHashset->ToBytes( pSerialTree, nSerialTree );
		if ( bDelete ) delete pHashset;
		
		pDIME.WriteDIME( 2, "", "http://edonkey2000.com/spec/md4-hashset", pSerialTree, nSerialTree );
		delete [] pSerialTree;
	}
	
	if ( m_bRange )
	{
		if ( m_nOffset >= (QWORD)pDIME.m_nLength ) m_nLength = SIZE_UNKNOWN;
		else m_nLength = min( m_nLength, (QWORD)pDIME.m_nLength - m_nOffset );
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
			m_pOutput->Print( "HTTP/1.1 206 OK\r\n" );
		else
			m_pOutput->Print( "HTTP/1.1 200 OK\r\n" );
		
		SendDefaultHeaders();
		
		m_pOutput->Print( "Content-Type: application/dime\r\n" );
		strHeader.Format( _T("Content-Length: %I64i\r\n"), m_nLength );
		m_pOutput->Print( strHeader );
		
		if ( m_nLength != pDIME.m_nLength )
		{
			strHeader.Format( _T("Content-Range: %I64i-%I64i\r\n"), m_nOffset, m_nOffset + m_nLength - 1 );
			m_pOutput->Print( strHeader );
		}
		
		m_pOutput->Print( "\r\n" );
		
		if ( ! m_bHead )
		{
			m_pOutput->Add( pDIME.m_pBuffer + m_nOffset, (DWORD)m_nLength );
		}
		
		StartSending( upsTigerTree );
		
		theApp.Message( MSG_DEFAULT, IDS_UPLOAD_TIGER_SEND,
			(LPCTSTR)m_sFileName, (LPCTSTR)m_sAddress );
	}
	else
	{
		m_sRanges.Format( _T("0-%I64i"), (QWORD)pDIME.m_nLength - 1 );
		ClearHashes();
		m_sLocations.Empty();
		
		SendResponse( IDR_HTML_BADRANGE, TRUE );
		theApp.Message( MSG_ERROR, IDS_UPLOAD_BAD_RANGE, (LPCTSTR)m_sAddress, (LPCTSTR)m_sFileName );
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP request preview

BOOL CUploadTransferHTTP::RequestPreview(CLibraryFile* pFile)
{
	ASSERT( pFile != NULL );
	
	m_sFileName		= pFile->m_sName;
	m_sFilePath		= pFile->GetPath();
	m_oSHA1			= pFile->m_oSHA1;
	m_oTiger		= pFile->m_oTiger;
	m_oED2K			= pFile->m_oED2K;
	DWORD nIndex	= pFile->m_nIndex;
	BOOL bCached	= pFile->m_bCachedPreview;
	
	Library.Unlock();
	
	int nExisting = Uploads.GetCount( this, upsPreview );
	
	if ( nExisting >= (int)Settings.Uploads.PreviewTransfers )
	{
		theApp.Message( MSG_ERROR, IDS_UPLOAD_PREVIEW_BUSY, (LPCTSTR)m_sFileName, (LPCTSTR)m_sAddress );
		m_pOutput->Print( "HTTP/1.1 503 Busy\r\n" );
		SendDefaultHeaders();
		StartSending( upsResponse );
		return TRUE;
	}
	
	CImageServices pServices;
	CImageFile pImage( &pServices );
	CThumbCache pCache;
	CSize szThumb( 0, 0 );
	
	if ( pCache.Load( m_sFilePath, &szThumb, nIndex, &pImage ) )
	{
		// Got a cached copy
	}
	else if ( Settings.Uploads.DynamicPreviews && pImage.LoadFromFile( m_sFilePath, FALSE, TRUE ) && pImage.EnsureRGB() )
	{
		theApp.Message( MSG_DEFAULT, IDS_UPLOAD_PREVIEW_DYNAMIC, (LPCTSTR)m_sFileName, (LPCTSTR)m_sAddress );
		
		int nSize = szThumb.cy * pImage.m_nWidth / pImage.m_nHeight;
		
		if ( nSize > szThumb.cx )
		{
			nSize = szThumb.cx * pImage.m_nHeight / pImage.m_nWidth;
			pImage.Resample( szThumb.cx, nSize );
		}
		else
		{
			pImage.Resample( nSize, szThumb.cy );
		}
		
		pCache.Store( m_sFilePath, &szThumb, nIndex, &pImage );
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_UPLOAD_PREVIEW_EMPTY, (LPCTSTR)m_sAddress, (LPCTSTR)m_sFileName );
		SendResponse( IDR_HTML_FILENOTFOUND );
		return TRUE;
	}
	
	if ( ! bCached )
	{
		if ( pFile = Library.LookupFile( nIndex, TRUE ) )
		{
			pFile->m_bCachedPreview = TRUE;
			Library.Unlock( TRUE );
		}
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
		theApp.Message( MSG_ERROR, IDS_UPLOAD_PREVIEW_EMPTY, (LPCTSTR)m_sAddress, (LPCTSTR)m_sFileName );
		SendResponse( IDR_HTML_FILENOTFOUND );
		return TRUE;
	}
	
	pServices.Cleanup();
	
	m_pOutput->Print( "HTTP/1.1 200 OK\r\n" );
	SendDefaultHeaders();
	
	CString strHeader;
	
	if ( m_oSHA1.IsValid() )
	{
		strHeader.Format( _T("X-Previewed-URN: %s\r\n"),
			(LPCTSTR)m_oSHA1.ToURN() );
	}
	else if ( m_oTiger.IsValid() )
	{
		strHeader.Format( _T("X-Previewed-URN: %s\r\n"),
			(LPCTSTR)m_oTiger.ToURN() );
	}
	else if ( m_oED2K.IsValid() )
	{
		strHeader.Format( _T("X-Previewed-URN: %s\r\n"),
			(LPCTSTR)m_oED2K.ToURN() );
	}
	
	m_pOutput->Print( strHeader );
	
	m_pOutput->Print( "Content-Type: image/jpeg\r\n" );
	
	strHeader.Format( _T("Content-Length: %lu\r\n"), nLength );
	m_pOutput->Print( strHeader );
	
	m_pOutput->Print( "\r\n" );
	
	if ( ! m_bHead )
	{
		m_pOutput->Add( pBuffer, nLength );
	}
	
	delete [] pBuffer;
	
	StartSending( upsPreview );
	
	theApp.Message( MSG_DEFAULT, IDS_UPLOAD_PREVIEW_SEND, (LPCTSTR)m_sFileName,
		(LPCTSTR)m_sAddress );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP request host browse

BOOL CUploadTransferHTTP::RequestHostBrowse()
{
	CBuffer pBuffer;
	
	int nExisting = Uploads.GetCount( this, upsBrowse );
	
	if ( nExisting >= (int)Settings.Uploads.PreviewTransfers )
	{
		theApp.Message( MSG_ERROR, IDS_UPLOAD_BROWSE_BUSY, (LPCTSTR)m_sAddress );
		m_pOutput->Print( "HTTP/1.1 503 Busy\r\n" );
		SendDefaultHeaders();
		StartSending( upsResponse );
		return TRUE;
	}
	
	if ( m_bHostBrowse < 2 )
	{
		if ( Settings.Community.ServeFiles )
		{
			CLocalSearch pSearch( NULL, &pBuffer, PROTOCOL_G1 );
			pSearch.Execute( 0 );
		}
	}
	else
	{
		if ( Settings.Community.ServeProfile && MyProfile.IsValid() )
		{
			CG2Packet* pProfile = CG2Packet::New( G2_PACKET_PROFILE_DELIVERY, TRUE );
			CString strXML = MyProfile.GetXML()->ToString( TRUE );
			pProfile->WritePacket( "XML", pProfile->GetStringLen( strXML ) );
			pProfile->WriteString( strXML, FALSE );
			pProfile->ToBuffer( &pBuffer );
			pProfile->Release();
		}
		
		if ( Settings.Community.ServeFiles )
		{
			CLocalSearch pSearch( NULL, &pBuffer, PROTOCOL_G2 );
			pSearch.Execute( 0 );
			pSearch.WriteVirtualTree();
		}
		
		if ( Settings.Community.ServeProfile && MyProfile.IsValid() )
		{
			if ( CG2Packet* pAvatar = MyProfile.CreateAvatar() )
			{
				pAvatar->ToBuffer( &pBuffer );
				pAvatar->Release();
			}
		}
	}
	
	m_pOutput->Print( "HTTP/1.1 200 OK\r\n" );
	SendDefaultHeaders();
	
	if ( m_bHostBrowse < 2 )
	{
		m_pOutput->Print( "Content-Type: application/x-gnutella-packets\r\n" );
	}
	else
	{
		m_pOutput->Print( "Content-Type: application/x-gnutella2\r\n" );
	}
	
	m_bDeflate = m_bDeflate && pBuffer.Deflate( TRUE );
	
	if ( m_bDeflate ) m_pOutput->Print( "Content-Encoding: deflate\r\n" );
	
	CString strLength;
	strLength.Format( _T("Content-Length: %lu\r\n\r\n"), pBuffer.m_nLength );
	m_pOutput->Print( strLength );
	
	if ( ! m_bHead ) m_pOutput->AddBuffer( &pBuffer );
	
	StartSending( upsBrowse );
	
	theApp.Message( MSG_SYSTEM, IDS_UPLOAD_BROWSE, (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );
	
	CTransfer::OnWrite();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadTransferHTTP formatted response

void CUploadTransferHTTP::SendResponse(UINT nResourceID, BOOL bFileHeaders)
{
	CString strBody, strResponse;
	
	HMODULE hModule = GetModuleHandle( NULL );
	HRSRC hRes = FindResource( hModule, MAKEINTRESOURCE( nResourceID ), MAKEINTRESOURCE( 23 ) );
	
	if ( hRes != NULL )
	{
		DWORD nSize			= SizeofResource( hModule, hRes );
		HGLOBAL hMemory		= LoadResource( hModule, hRes );
		LPTSTR pszOutput	= strBody.GetBuffer( nSize + 1 );
		LPCSTR pszInput		= (LPCSTR)LockResource( hMemory );
		
		while ( nSize-- ) *pszOutput++ = *pszInput++;
		*pszOutput++ = 0;
		
		strBody.ReleaseBuffer();
	}
	
	int nBreak	= strBody.Find( _T("\r\n") );
	strResponse	= strBody.Left( nBreak + 2 );
	strBody		= strBody.Mid( nBreak + 2 );
	
	while ( TRUE )
	{
		int nStart = strBody.Find( _T("<%") );
		if ( nStart < 0 ) break;
		
		int nEnd = strBody.Find( _T("%>") );
		if ( nEnd < nStart ) break;

		CString strReplace = strBody.Mid( nStart + 2, nEnd - nStart - 2 );

		strReplace.TrimLeft();
		strReplace.TrimRight();
		
		if ( strReplace.CompareNoCase( _T("Name") ) == 0 )
			strReplace = m_sFileName;
		else if ( strReplace.CompareNoCase( _T("SHA1") ) == 0 )
			strReplace = m_oSHA1.ToString();
		else if ( strReplace.CompareNoCase( _T("URN") ) == 0 )
			strReplace = m_oSHA1.ToURN();
		else if ( strReplace.CompareNoCase( _T("Version") ) == 0 )
			strReplace = theApp.m_sVersion;
		else if ( strReplace.CompareNoCase( _T("Neighbours") ) == 0 )
			GetNeighbourList( strReplace );
		else if ( strReplace.CompareNoCase( _T("ListenIP") ) == 0 )
		{
			if ( Network.IsListening() )
			{
				strReplace.Format( _T("%s:%i"),
					(LPCTSTR)CString( inet_ntoa( Network.m_pHost.sin_addr ) ),
					htons( Network.m_pHost.sin_port ) );
			}
			else strReplace.Empty();
		}
		
		strBody = strBody.Left( nStart ) + strReplace + strBody.Mid( nEnd + 2 );
	}
	
	m_pOutput->Print( _T("HTTP/1.1 ") + strResponse );
	SendDefaultHeaders();
	if ( bFileHeaders ) SendFileHeaders();
	m_pOutput->Print( "Content-Type: text/html\r\n" );
	
#ifdef _UNICODE
	int nBody = WideCharToMultiByte( CP_UTF8, 0, strBody, strBody.GetLength(), NULL, 0, NULL, NULL );
	LPSTR pszBody = new CHAR[ nBody ];
	WideCharToMultiByte( CP_UTF8, 0, strBody, strBody.GetLength(), pszBody, nBody, NULL, NULL );
#else
	int nBody = strBody.GetLength();
	LPCSTR pszBody = (LPCSTR)strBody;
#endif
	
	strResponse.Format( _T("Content-Length: %lu\r\n\r\n"), nBody );
	m_pOutput->Print( strResponse );
	
	if ( ! m_bHead ) m_pOutput->Add( pszBody, nBody );
	
#ifdef _UNICODE
	delete [] pszBody;
#endif
	
	StartSending( upsResponse );
}

void CUploadTransferHTTP::GetNeighbourList(CString& strOutput)
{
	static LPCTSTR pszModes[4][3] =
	{
		{ _T("Handshake"), _T("Handshake"), _T("Handshake") },
		{ _T("G1 Peer"), _T("G1 Ultrapeer"), _T("G1 Leaf") },
		{ _T("G2 Peer"), _T("G2 Hub"), _T("G2 Leaf") },
		{ _T("eDonkey2000"), _T("eDonkey2000"), _T("eDonkey2000") }
	};
	
	strOutput.Empty();
	
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 100 ) ) return;
		
	DWORD tNow = GetTickCount();
		
	for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
	{
		CNeighbour* pNeighbour = Neighbours.GetNext( pos );
		
		if ( pNeighbour->m_nState == nrsConnected )
		{
			CString strNode;
			
			DWORD nTime = ( tNow - pNeighbour->m_tConnected ) / 1000;
			
			strNode.Format( _T("<tr><td class=\"fi\"><a href=\"gnutella:host:%s:%lu\">%s:%lu</a></td><td class=\"fi\" align=\"center\">%i:%.2i:%.2i</td><td class=\"fi\">%s</td><td class=\"fi\">%s</td><td class=\"fi\"><a href=\"http://%s:%lu/\">Browse</a></td></tr>\r\n"),
				(LPCTSTR)pNeighbour->m_sAddress, htons( pNeighbour->m_pHost.sin_port ),
				(LPCTSTR)pNeighbour->m_sAddress, htons( pNeighbour->m_pHost.sin_port ),
				nTime / 3600, ( nTime % 3600 ) / 60, nTime % 60,
				pszModes[ pNeighbour->m_nProtocol ][ pNeighbour->m_nNodeType ],
				(LPCTSTR)pNeighbour->m_sUserAgent,
				(LPCTSTR)pNeighbour->m_sAddress, htons( pNeighbour->m_pHost.sin_port ) );
			
			strOutput += strNode;
		}
	}
}
