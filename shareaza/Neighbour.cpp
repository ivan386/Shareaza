//
// Neighbour.cpp
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
#include "Network.h"
#include "Neighbours.h"
#include "Neighbour.h"
#include "Security.h"
#include "RouteCache.h"
#include "Library.h"
#include "Buffer.h"
#include "Packet.h"
#include "G1Packet.h"
#include "G2Packet.h"
#include "SearchManager.h"
#include "QueryHashTable.h"
#include "QueryHashGroup.h"
#include "QueryHashMaster.h"
#include "QueryHit.h"
#include "GProfile.h"
#include "Statistics.h"
#include <zlib.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define Z_TIMER		200


//////////////////////////////////////////////////////////////////////
// CNeighbour construction

CNeighbour::CNeighbour(PROTOCOLID nProtocol)
{
	m_nProtocol		= nProtocol;
	m_nState		= nrsNull;
	m_pVendor		= NULL;
	m_bGUID			= FALSE;
	m_pProfile		= NULL;
	
	m_bAutomatic	= FALSE;
	m_bShake06		= TRUE;
	m_bShareaza		= FALSE;
	m_nNodeType		= ntNode;
	m_bQueryRouting	= FALSE;
	m_bPongCaching	= FALSE;
	m_bVendorMsg	= FALSE;
	m_bGGEP			= FALSE;
	
	m_tLastQuery	= 0;
	m_tLastPacket	= 0;
	
	m_nInputCount	= m_nOutputCount = 0;
	m_nDropCount	= 0;
	m_nLostCount	= 0;
	m_nOutbound		= 0;
	m_nFileCount	= 0;
	m_nFileVolume	= 0;
	
	m_pQueryTableLocal	= NULL;
	m_pQueryTableRemote	= NULL;
	
	m_pZInput			= NULL;
	m_pZOutput			= NULL;
	m_nZInput			= 0;
	m_nZOutput			= 0;
	m_pZSInput			= NULL;
	m_pZSOutput			= NULL;
	m_bZFlush			= FALSE;
	m_tZOutput			= 0;
}

CNeighbour::CNeighbour(PROTOCOLID nProtocol, CNeighbour* pBase)
{
	AttachTo( pBase );
	
	CopyMemory( &m_zStart, &pBase->m_zStart, (LPBYTE)&m_zEnd - (LPBYTE)&m_zStart );
	
	m_nState		= nrsConnected;
	m_nProtocol		= nProtocol;
	m_tConnected	= GetTickCount();
	m_tLastPacket	= m_tConnected;
	
	if ( m_bQueryRouting )
	{
		m_pQueryTableLocal	= new CQueryHashTable();
		m_pQueryTableRemote	= new CQueryHashTable();
	}
	
	Neighbours.Add( this, FALSE );
}

CNeighbour::~CNeighbour()
{
	if ( z_streamp pStream = (z_streamp)m_pZSOutput )
	{
		deflateEnd( pStream );
		delete pStream;
	}
	
	if ( z_streamp pStream = (z_streamp)m_pZSInput )
	{
		inflateEnd( pStream );
		delete pStream;
	}
	
	if ( m_pProfile )			delete m_pProfile;
	if ( m_pZOutput )			delete m_pZOutput;
	if ( m_pZInput )			delete m_pZInput;
	if ( m_pQueryTableRemote )	delete m_pQueryTableRemote;
	if ( m_pQueryTableLocal )	delete m_pQueryTableLocal;
}

//////////////////////////////////////////////////////////////////////
// CNeighbour close

void CNeighbour::Close(UINT nError)
{
	ASSERT( m_hSocket != INVALID_SOCKET );
	
	BOOL bVoluntary = ( nError == IDS_CONNECTION_CLOSED || nError == IDS_CONNECTION_PEERPRUNE );
	
	Neighbours.Remove( this );
	
	CConnection::Close();
	
	if ( nError )
	{
		theApp.Message( bVoluntary ? MSG_DEFAULT : MSG_ERROR, nError, (LPCTSTR)m_sAddress );
	}
	
	delete this;
}

void CNeighbour::DelayClose(UINT nError)
{
	if ( nError ) theApp.Message( MSG_ERROR, nError, (LPCTSTR)m_sAddress );
	m_nState = nrsClosing;
	QueueRun();
}

//////////////////////////////////////////////////////////////////////
// CNeighbour send

BOOL CNeighbour::Send(CPacket* pPacket, BOOL bRelease, BOOL bBuffered)
{
	if ( bRelease ) pPacket->Release();
	return FALSE;
}

BOOL CNeighbour::SendQuery(CQuerySearch* pSearch, CPacket* pPacket, BOOL bLocal)
{
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CNeighbour run event handler

BOOL CNeighbour::OnRun()
{
	DWORD tNow = GetTickCount();
	
	if ( tNow - m_tLastPacket > Settings.Connection.TimeoutTraffic * 3 )
	{
		Close( IDS_CONNECTION_TIMEOUT_TRAFFIC );
		return FALSE;
	}
	
	if ( m_nNodeType == ntHub || ( m_nNodeType == ntNode && m_nProtocol == PROTOCOL_G2 ) )
	{
		if ( m_pQueryTableLocal != NULL && m_pQueryTableLocal->m_nCookie != QueryHashMaster.m_nCookie )
		{
			if ( tNow - QueryHashMaster.m_nCookie > 30000 ||
				 QueryHashMaster.m_nCookie - m_pQueryTableLocal->m_nCookie > 60000 ||
				 m_pQueryTableLocal->m_bLive == FALSE )
			{
				if ( m_pQueryTableLocal->PatchTo( &QueryHashMaster, this ) )
				{
					theApp.Message( MSG_SYSTEM, IDS_PROTOCOL_QRP_SENT, (LPCTSTR)m_sAddress,
						m_pQueryTableLocal->m_nBits, m_pQueryTableLocal->m_nHash,
						m_pQueryTableLocal->m_nInfinity, m_pQueryTableLocal->GetPercent() );
				}
			}
		}
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CNeighbour close event handler

void CNeighbour::OnDropped(BOOL bError)
{
	DWORD nTime1 = ( GetTickCount() - m_tConnected ) / 1000;
	DWORD nTime2 = ( GetTickCount() - m_tLastPacket ) / 1000;
	
	theApp.Message( MSG_DEBUG, _T("Dropped neighbour %s (%s), conn: %.2i:%.2i, packet: %.2i:%.2i"),
		(LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent,
		nTime1 / 60, nTime1 % 60, nTime2 / 60, nTime2 % 60 );
	
	Close( IDS_CONNECTION_DROPPED );
}

//////////////////////////////////////////////////////////////////////
// CNeighbour compressed read and write handlerss

BOOL CNeighbour::OnRead()
{
	CConnection::OnRead();
	
	if ( m_pZInput == NULL || m_pInput->m_nLength == 0 ) return TRUE;
	
	BOOL bNew = ( m_pZSInput == NULL );
	
	if ( bNew ) m_pZSInput = new z_stream;
	z_streamp pStream = (z_streamp)m_pZSInput;
	
	if ( bNew )
	{
		ZeroMemory( pStream, sizeof(z_stream) );
		
		if ( inflateInit( pStream ) != Z_OK )
		{
			delete pStream;
			delete m_pZInput;
			m_pZInput = NULL;
			m_pZSInput = NULL;
			return TRUE;
		}
	}
	
	while ( m_pInput->m_nLength || m_pZInput->m_nLength == m_pZInput->m_nBuffer || pStream->avail_out == 0 )
	{
		m_pZInput->EnsureBuffer( 1024 );
		
		pStream->next_in	= m_pInput->m_pBuffer;
		pStream->avail_in	= m_pInput->m_nLength;
		pStream->next_out	= m_pZInput->m_pBuffer + m_pZInput->m_nLength;
		pStream->avail_out	= m_pZInput->m_nBuffer - m_pZInput->m_nLength;
		
		inflate( pStream, Z_SYNC_FLUSH );
		
		if ( pStream->avail_in >= 0 && pStream->avail_in < m_pInput->m_nLength )
		{
			m_pInput->Remove( m_pInput->m_nLength - pStream->avail_in );
		}
		
		DWORD nBlock = ( m_pZInput->m_nBuffer - m_pZInput->m_nLength ) - pStream->avail_out;
		
		m_pZInput->m_nLength += nBlock;
		m_nZInput += nBlock;
		
		if ( ! nBlock ) break;
	}
	
	return TRUE;
}

BOOL CNeighbour::OnWrite()
{
	if ( m_pZOutput == NULL ) return CConnection::OnWrite();
	
	BOOL bNew = ( m_pZSOutput == NULL );
	
	if ( bNew ) m_pZSOutput = new z_stream;
	z_streamp pStream = (z_streamp)m_pZSOutput;
	
	if ( bNew )
	{
		ZeroMemory( pStream, sizeof(z_stream) );
		
		if ( deflateInit( pStream, 6 ) != Z_OK )
		{
			delete pStream;
			delete m_pZOutput;
			m_pZOutput = NULL;
			m_pZSOutput = NULL;
			return TRUE;
		}
	}
	
	if ( m_pOutput->m_nLength ) 
	{
		CConnection::OnWrite();
		if ( m_pOutput->m_nLength ) return TRUE;
	}
	
	DWORD tNow = GetTickCount();
	
	if ( tNow - m_tZOutput >= Z_TIMER ) m_bZFlush = TRUE;
	
	while ( ( m_pZOutput->m_nLength && ! m_pOutput->m_nLength ) || pStream->avail_out == 0 )
	{
		m_pOutput->EnsureBuffer( 1024 );
		
		pStream->next_in	= m_pZOutput->m_pBuffer;
		pStream->avail_in	= m_pZOutput->m_nLength;
		pStream->next_out	= m_pOutput->m_pBuffer + m_pOutput->m_nLength;
		pStream->avail_out	= m_pOutput->m_nBuffer - m_pOutput->m_nLength;
		
		deflate( pStream, m_bZFlush ? Z_SYNC_FLUSH : 0 );
		
		m_nZOutput += m_pZOutput->m_nLength - pStream->avail_in;
		m_pZOutput->Remove( m_pZOutput->m_nLength - pStream->avail_in );
		
		int nOutput = ( m_pOutput->m_nBuffer - m_pOutput->m_nLength ) - pStream->avail_out;
		
		if ( nOutput )
		{
			m_pOutput->m_nLength += nOutput;
			m_tZOutput = tNow;
		}
		else if ( m_bZFlush )
		{
			m_bZFlush = FALSE;
		}
		
		CConnection::OnWrite();
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CNeighbour common hit routing code

BOOL CNeighbour::OnCommonHit(CPacket* pPacket)
{
	CQueryHit* pHits = NULL;
	int nHops = 0;
	
	if ( pPacket->m_nProtocol == PROTOCOL_G1 )
	{
		pHits = CQueryHit::FromPacket( (CG1Packet*)pPacket, &nHops );
	}
	else if ( pPacket->m_nProtocol == PROTOCOL_G2 )
	{
		pHits = CQueryHit::FromPacket( (CG2Packet*)pPacket, &nHops );
	}
	else
	{
		ASSERT( FALSE );
	}
	
	if ( pHits == NULL )
	{
		pPacket->Debug( _T("BadHit") );
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_BAD_HIT, (LPCTSTR)m_sAddress );
		m_nDropCount++;
		if ( m_nProtocol == PROTOCOL_G1 )
			Statistics.Current.Gnutella1.Dropped++;
		else if ( m_nProtocol == PROTOCOL_G2 )
			Statistics.Current.Gnutella2.Dropped++;
		return TRUE;
	}
	
	if ( Security.IsDenied( &pHits->m_pAddress ) || nHops > (int)Settings.Gnutella1.MaximumTTL )
	{
		pHits->Delete();
		m_nDropCount++;
		if ( m_nProtocol == PROTOCOL_G1 )
			Statistics.Current.Gnutella1.Dropped++;
		else if ( m_nProtocol == PROTOCOL_G2 )
			Statistics.Current.Gnutella2.Dropped++;
		return TRUE;
	}
	
	Network.NodeRoute->Add( &pHits->m_pClientID, this );
	
	if ( SearchManager.OnQueryHits( pHits ) )
	{
		Network.RouteHits( pHits, pPacket );
	}
	
	Network.OnQueryHits( pHits );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CNeighbour common query hash table

BOOL CNeighbour::OnCommonQueryHash(CPacket* pPacket)
{
	if ( m_pQueryTableRemote == NULL )
	{
		m_pQueryTableRemote = new CQueryHashTable();
		theApp.Message( MSG_DEFAULT, IDS_PROTOCOL_QRP_UNEXPECTED, (LPCTSTR)m_sAddress );
	}
	
	BOOL bLive = m_pQueryTableRemote->m_bLive;
	
	if ( ! m_pQueryTableRemote->OnPacket( pPacket ) )
	{
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_QRP_FAILED, (LPCTSTR)m_sAddress );
		return FALSE;
	}
	
	if ( m_pQueryTableRemote->m_bLive && ! bLive )
	{
		theApp.Message( MSG_DEFAULT, IDS_PROTOCOL_QRP_UPDATED, (LPCTSTR)m_sAddress,
			m_pQueryTableRemote->m_nBits, m_pQueryTableRemote->m_nHash,
			m_pQueryTableRemote->m_nInfinity, m_pQueryTableRemote->GetPercent() );
	}
	
	if ( m_nNodeType == ntLeaf && m_pQueryTableRemote->m_pGroup == NULL )
	{
		QueryHashMaster.Add( m_pQueryTableRemote );
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CNeighbour compression statistics

void CNeighbour::GetCompression(float* pnInRate, float* pnOutRate)
{
	*pnInRate = -1; *pnOutRate = -1;
	
	if ( m_pZInput != NULL && m_nZInput > 0.)
	{
		*pnInRate = 1.0f - (float)m_mInput.nTotal / (float)m_nZInput;
		if ( *pnInRate < 0 ) *pnInRate = 0;
		else if ( *pnInRate > 1 ) *pnInRate = 1;
	}
	
	if ( m_pZOutput != NULL && m_mOutput.nTotal > 0.)
	{
		*pnOutRate = 1.0f - (float)m_mOutput.nTotal / (float)m_nZOutput;
		if ( *pnOutRate < 0 ) *pnOutRate = 0;
		else if ( *pnOutRate > 1 ) *pnOutRate = 1;
	}
}
