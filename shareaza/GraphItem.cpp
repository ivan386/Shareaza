//
// GraphItem.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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
#include "Statistics.h"
#include "GraphItem.h"
#include "Skin.h"

#include "Network.h"
#include "Datagrams.h"
#include "HostCache.h"
#include "Neighbours.h"
#include "Neighbour.h"
#include "Transfers.h"
#include "Downloads.h"
#include "Uploads.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CGraphItem construction

CGraphItem::CGraphItem(DWORD nCode, float nMultiplier, COLORREF nColour)
{
	m_nCode		= nCode;
	m_nMultiplier = nMultiplier;
	m_nColour	= nColour;

	m_nData		= 64;
	m_pData		= new DWORD[ m_nData ];
	m_nLength	= 0;
	m_nPosition	= 0;

	if ( m_nCode ) SetCode( m_nCode );
}

CGraphItem::~CGraphItem()
{
	if ( m_pData ) delete [] m_pData;
}

//////////////////////////////////////////////////////////////////////
// CGraphItem code

void CGraphItem::SetCode(DWORD nCode)
{
	if ( m_nCode != nCode )
	{
		m_nCode = nCode;
		Clear();
	}

	const GRAPHITEM* pDesc = GetItemDesc( m_nCode );
	if ( pDesc )
		Skin.LoadString( m_sName, pDesc->m_nStringID );
}

//////////////////////////////////////////////////////////////////////
// CGraphItem clear

void CGraphItem::Clear()
{
	m_nLength	= 0;
	m_nPosition	= 0;
}

//////////////////////////////////////////////////////////////////////
// CGraphItem add

DWORD CGraphItem::Add(DWORD nValue)
{
	m_pData[ m_nPosition++ ] = nValue;
	m_nLength = max( m_nLength, m_nPosition );
	if ( m_nPosition >= m_nData ) m_nPosition = 0;
	return nValue;
}

//////////////////////////////////////////////////////////////////////
// CGraphItem value

DWORD CGraphItem::GetValueAt(DWORD nPosition) const
{
	if ( nPosition >= m_nData ) return 0;
	DWORD nPos = m_nData + m_nPosition - nPosition - 1;
	if ( nPos >= m_nData ) nPos -= m_nData;
	return m_pData[ nPos ];
}

DWORD CGraphItem::GetMaximum() const
{
	DWORD nMaximum = 0;

	for ( DWORD nPosition = 0 ; nPosition < m_nLength ; nPosition++ )
	{
		DWORD nPos = m_nData + m_nPosition - nPosition - 1;
		if ( nPos >= m_nData ) nPos -= m_nData;
		nMaximum = max( nMaximum, m_pData[ nPos ] );
	}

	return nMaximum;
}

//////////////////////////////////////////////////////////////////////
// CGraphItem set history

void CGraphItem::SetHistory(DWORD nSize, BOOL bMax)
{
	if ( bMax && m_nData >= nSize ) return;
	else if ( ! bMax && m_nData == nSize ) return;

	DWORD* pOldData		= m_pData;
	DWORD nOldTotal		= m_nData;
	DWORD nOldLength	= min( m_nLength, nSize );
	DWORD nOldPosition	= m_nPosition;

	m_nData		= nSize;
	m_pData		= new DWORD[ m_nData ];
	m_nPosition	= 0;
	m_nLength	= 0;

	if ( pOldData == NULL ) return;

	for ( DWORD nPosition = 0 ; nPosition < nOldLength ; nPosition++ )
	{
		DWORD nPos = nOldTotal + nOldPosition - nOldLength + nPosition;
		if ( nPos >= nOldTotal ) nPos -= nOldTotal;
		Add( pOldData[ nPos ] );
	}

	delete [] pOldData;
}

//////////////////////////////////////////////////////////////////////
// CGraphItem update

DWORD CGraphItem::Update()
{
	return Add( (DWORD)GetValue( m_nCode, m_nMultiplier ) );
}

//////////////////////////////////////////////////////////////////////
// CGraphItem serialize

void CGraphItem::Serialize(CArchive& ar)
{
	if ( ar.IsStoring() )
	{
		ar << m_nCode;
		ar << m_nMultiplier;
		ar << m_nColour;
	}
	else
	{
		ar >> m_nCode;
		ar >> m_nMultiplier;
		if ( m_nMultiplier == 0.0f )
			m_nMultiplier = 1.0f;
		ar >> m_nColour;

		SetCode( m_nCode );
	}
}

//////////////////////////////////////////////////////////////////////
// CGraphItem make gradient

void CGraphItem::MakeGradient(COLORREF crBack)
{
	if ( m_cPen[0] != m_nColour && m_pPen[0].m_hObject != NULL )
		m_pPen[0].DeleteObject();
	if ( m_pPen[0].m_hObject == NULL )
		m_pPen[0].CreatePen( PS_SOLID, 1, m_cPen[0] = m_nColour );

	int nOffset = ( crBack != RGB( 0, 0, 0 ) ) ? 1 : 0;

	for ( int nLayer = 1 ; nLayer < 4 ; nLayer++ )
	{
		int nAlpha	= ( nLayer + nOffset ) * 255 / 5;
		int nRed	= GetRValue( m_nColour ) * ( 255 - nAlpha ) / 255 + GetRValue( crBack ) * nAlpha / 255;
		int nGreen	= GetGValue( m_nColour ) * ( 255 - nAlpha ) / 255 + GetGValue( crBack ) * nAlpha / 255;
		int nBlue	= GetBValue( m_nColour ) * ( 255 - nAlpha ) / 255 + GetBValue( crBack ) * nAlpha / 255;

		COLORREF cr = RGB( nRed, nGreen, nBlue );

		if ( m_cPen[ nLayer ] != cr && m_pPen[ nLayer ].m_hObject != NULL )
			m_pPen[ nLayer ].DeleteObject();
		if ( m_pPen[ nLayer ].m_hObject == NULL )
			m_pPen[ nLayer ].CreatePen( PS_SOLID, 1, m_cPen[ nLayer ] = cr );
	}
}

//////////////////////////////////////////////////////////////////////
// CGraphItem value retrieval

QWORD CGraphItem::GetValue(const DWORD nCode, const float nMultiplier)
{
	long double nValue = 0;

	switch ( nCode )
	{
	case GRC_RANDOM:
		nValue = GetRandomNum( 0, 99 );
		break;

	case GRC_TOTAL_BANDWIDTH_IN:
		nValue = (long double)GetValue( GRC_GNUTELLA_BANDWIDTH_IN ) + GetValue( GRC_DOWNLOADS_BANDWIDTH ) + Datagrams.m_nInBandwidth * 8;
		break;
	case GRC_TOTAL_BANDWIDTH_OUT:
		nValue = (long double)GetValue( GRC_GNUTELLA_BANDWIDTH_OUT ) + GetValue( GRC_UPLOADS_BANDWIDTH ) + Datagrams.m_nOutBandwidth * 8;
		break;
	case GRC_TOTAL_BANDWIDTH_TCP_IN:
		nValue = (long double)GetValue( GRC_GNUTELLA_BANDWIDTH_IN ) + GetValue( GRC_DOWNLOADS_BANDWIDTH );
		break;
	case GRC_TOTAL_BANDWIDTH_TCP_OUT:
		nValue = (long double)GetValue( GRC_GNUTELLA_BANDWIDTH_OUT ) + GetValue( GRC_UPLOADS_BANDWIDTH );
		break;
	case GRC_TOTAL_BANDWIDTH_UDP_IN:
		nValue = Datagrams.m_nInBandwidth * 8;
		break;
	case GRC_TOTAL_BANDWIDTH_UDP_OUT:
		nValue = Datagrams.m_nOutBandwidth * 8;
		break;

	case GRC_GNUTELLA_CONNECTIONS:
		if ( ! Network.m_pSection.Lock( 20 ) ) break;
		nValue = Neighbours.GetCount( PROTOCOL_ANY, nrsConnected, -1 );
		Network.m_pSection.Unlock();
		break;
	case GRC_GNUTELLA_CONNECTIONS_ALL:
		if ( ! Network.m_pSection.Lock( 20 ) ) break;
		nValue = Neighbours.GetCount( PROTOCOL_ANY, -1, -1 );
		Network.m_pSection.Unlock();
		break;
	case GRC_GNUTELLA_BANDWIDTH_IN:
		nValue = Neighbours.BandwidthIn() * 8;
		break;
	case GRC_GNUTELLA_BANDWIDTH_OUT:
		nValue = Neighbours.BandwidthOut() * 8;
		break;
	case GRC_GNUTELLA_PACKETS_IN:
		if ( ! Network.m_pSection.Lock( 20 ) ) break;
		Statistics.Update();
		nValue = (long double)Statistics.Last.Gnutella1.Incoming + Statistics.Last.Gnutella2.Incoming;
		Network.m_pSection.Unlock();
		break;
	case GRC_GNUTELLA_PACKETS_OUT:
		if ( ! Network.m_pSection.Lock( 20 ) ) break;
		Statistics.Update();
		nValue = (long double)Statistics.Last.Gnutella1.Outgoing + Statistics.Last.Gnutella2.Outgoing;
		Network.m_pSection.Unlock();
		break;

	case GRC_DOWNLOADS_FILES:
		if ( ! Transfers.m_pSection.Lock( 20 ) ) break;
		nValue = (long double)Downloads.GetCount( TRUE );
		Transfers.m_pSection.Unlock();
		break;
	case GRC_DOWNLOADS_TRANSFERS:
		nValue = Downloads.m_nTransfers;
		break;
	case GRC_DOWNLOADS_BANDWIDTH:
		nValue = Downloads.m_nBandwidth * 8;
		break;

	case GRC_UPLOADS_TRANSFERS:
		nValue = Uploads.m_nCount;
		break;
	case GRC_UPLOADS_BANDWIDTH:
		nValue = Uploads.m_nBandwidth * 8;
		break;

	case GRC_GNUTELLA_ROUTED:
		if ( ! Network.m_pSection.Lock( 20 ) ) break;
		Statistics.Update();
		nValue = (long double)Statistics.Last.Gnutella1.Routed + Statistics.Last.Gnutella2.Routed;
		Network.m_pSection.Unlock();
		break;
	case GRC_GNUTELLA_DROPPED:
		if ( ! Network.m_pSection.Lock( 20 ) ) break;
		Statistics.Update();
		nValue = (long double)Statistics.Last.Gnutella1.Dropped + Statistics.Last.Gnutella2.Dropped;
		Network.m_pSection.Unlock();
		break;
	case GRC_GNUTELLA_LOST:
		if ( ! Network.m_pSection.Lock( 20 ) ) break;
		Statistics.Update();
		nValue = (long double)Statistics.Last.Gnutella1.Lost + Statistics.Last.Gnutella2.Lost;
		Network.m_pSection.Unlock();
		break;

	case GRC_GNUTELLA_QUERIES:
		if ( ! Network.m_pSection.Lock( 20 ) ) break;
		Statistics.Update();
		nValue = (long double)Statistics.Last.Gnutella1.Queries + Statistics.Last.Gnutella2.Queries;
		Network.m_pSection.Unlock();
		break;

	case GRC_GNUTELLA_QUERIES_PROCESSED:
		if ( ! Network.m_pSection.Lock( 20 ) ) break;
		Statistics.Update();
		nValue = (long double)Statistics.Last.Gnutella1.QueriesProcessed + Statistics.Last.Gnutella2.QueriesProcessed;
		Network.m_pSection.Unlock();
		break;

	case GRC_GNUTELLA_PINGS:
		if ( ! Network.m_pSection.Lock( 20 ) ) break;
		Statistics.Update();
		if ( Statistics.Last.Gnutella1.PingsSent + Statistics.Last.Gnutella2.PingsSent )
			nValue = (long double)( Statistics.Last.Gnutella1.PingsReceived + Statistics.Last.Gnutella2.PingsReceived ) /
					 ( Statistics.Last.Gnutella1.PingsSent + Statistics.Last.Gnutella2.PingsSent ) * 100;
		Network.m_pSection.Unlock();
		break;
	case GRC_GNUTELLA_PONGS:
		if ( ! Network.m_pSection.Lock( 20 ) ) break;
		Statistics.Update();
		if ( Statistics.Last.Gnutella1.PongsSent + Statistics.Last.Gnutella2.PongsSent )
			nValue = (long double)( Statistics.Last.Gnutella1.PongsReceived + Statistics.Last.Gnutella2.PongsReceived ) /
					 ( Statistics.Last.Gnutella1.PongsSent + Statistics.Last.Gnutella2.PongsSent ) * 100;
		Network.m_pSection.Unlock();
		break;
	case GRC_CONNECTION_ERRORS:
		if ( ! Network.m_pSection.Lock( 20 ) ) break;
		Statistics.Update();
		if ( Statistics.Ever.Connections.Incoming + Statistics.Ever.Connections.Outgoing )
			nValue = (long double)Statistics.Ever.Connections.Errors / 
				( Statistics.Ever.Connections.Incoming + Statistics.Ever.Connections.Outgoing ) * 100;
		Network.m_pSection.Unlock();
	};

	return (QWORD)floor( nValue * nMultiplier );
}

//////////////////////////////////////////////////////////////////////
// CGraphItem item names

const GRAPHITEM CGraphItem::m_pItemDesc[] =
{
	{ 0, 0, 0, 0 },

	// Stats item, label in legend, units, multiplier (see CGraphItemDlg::OnSelChangeGraphSource)
	{ GRC_TOTAL_BANDWIDTH_IN, IDS_GRAPH_TOTAL_BANDWIDTH_IN, 1, 1.0f },
	{ GRC_TOTAL_BANDWIDTH_OUT, IDS_GRAPH_TOTAL_BANDWIDTH_OUT, 1, 1.0f },
	{ GRC_TOTAL_BANDWIDTH_TCP_IN, IDS_GRAPH_TOTAL_BANDWIDTH_TCP_IN, 1, 1.0f },
	{ GRC_TOTAL_BANDWIDTH_TCP_OUT, IDS_GRAPH_TOTAL_BANDWIDTH_TCP_OUT, 1, 1.0f },
	{ GRC_TOTAL_BANDWIDTH_UDP_IN, IDS_GRAPH_TOTAL_BANDWIDTH_UDP_IN, 1, 1.0f },
	{ GRC_TOTAL_BANDWIDTH_UDP_OUT, IDS_GRAPH_TOTAL_BANDWIDTH_UDP_OUT, 1, 1.0f },

	{ GRC_GNUTELLA_CONNECTIONS, IDS_GRAPH_GNUTELLA_CONNECTIONS, 0, 1.0f },
	{ GRC_GNUTELLA_CONNECTIONS_ALL, IDS_GRAPH_GNUTELLA_CONNECTIONS_ALL, 0, 1.0f },
	{ GRC_GNUTELLA_BANDWIDTH_IN, IDS_GRAPH_GNUTELLA_BANDWIDTH_IN, 1, 1.0f },
	{ GRC_GNUTELLA_BANDWIDTH_OUT, IDS_GRAPH_GNUTELLA_BANDWIDTH_OUT, 1, 1.0f },
	{ GRC_GNUTELLA_PACKETS_IN, IDS_GRAPH_GNUTELLA_PACKETS_IN, 0, 1.0f },
	{ GRC_GNUTELLA_PACKETS_OUT, IDS_GRAPH_GNUTELLA_PACKETS_OUT, 0, 1.0f },

	{ GRC_DOWNLOADS_FILES, IDS_GRAPH_DOWNLOADS_FILES, 0, 1.0f },
	{ GRC_DOWNLOADS_TRANSFERS, IDS_GRAPH_DOWNLOADS_TRANSFERS, 0, 1.0f },
	{ GRC_DOWNLOADS_BANDWIDTH, IDS_GRAPH_DOWNLOADS_BANDWIDTH, 1, 1.0f },

	{ GRC_UPLOADS_TRANSFERS, IDS_GRAPH_UPLOADS_TRANSFERS, 0, 1.0f },
	{ GRC_UPLOADS_BANDWIDTH, IDS_GRAPH_UPLOADS_BANDWIDTH, 1, 1.0f },

	{ GRC_GNUTELLA_ROUTED, IDS_GRAPH_GNUTELLA_ROUTED, 0, 1.0f },
	{ GRC_GNUTELLA_DROPPED, IDS_GRAPH_GNUTELLA_DROPPED, 0, 1.0f },
	{ GRC_GNUTELLA_LOST, IDS_GRAPH_GNUTELLA_LOST, 0, 1.0f },
	{ GRC_GNUTELLA_QUERIES, IDS_GRAPH_GNUTELLA_QUERIES, 0, 1.0f },
	{ GRC_GNUTELLA_QUERIES_PROCESSED, IDS_GRAPH_GNUTELLA_QUERIES_PROCESSED, 0, 1.0f },

	{ GRC_GNUTELLA_PINGS, IDS_GRAPH_GNUTELLA_PINGS, 3, 1.0f },
	{ GRC_GNUTELLA_PONGS, IDS_GRAPH_GNUTELLA_PONGS, 3, 1.0f },

	{ GRC_CONNECTION_ERRORS, IDS_GRAPH_CONNECTION_ERRORS, 3, 1.0f },

	{ 0, 0, 0, 0 }
};

const GRAPHITEM* CGraphItem::GetItemDesc(const DWORD nCode)
{
	if ( ! nCode ) return NULL;

	for ( int nItem = 1 ; m_pItemDesc[ nItem ].m_nCode ; nItem++ )
	{
		if ( m_pItemDesc[ nItem ].m_nCode == nCode ) return &m_pItemDesc[ nItem ];
	}

	return NULL;
}
