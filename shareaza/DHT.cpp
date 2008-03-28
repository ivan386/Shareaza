//
// DHT.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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
#include "Network.h"
#include "Datagrams.h"
#include "Transfers.h"
#include "Buffer.h"
#include "GProfile.h"
#include "BENode.h"
#include "BTClient.h"
#include "DHT.h"
#include "HostCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDHT DHT;


BOOL CDHT::OnPacket(SOCKADDR_IN* pHost, const CBENode* pRoot)
{
	// TODO: SmartDump( pHost, TRUE, FALSE );
	theApp.Message( MSG_DEBUG, _T("Recieved UDP BitTorrent packet from %s: %s"),
		(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), (LPCTSTR)pRoot->Encode() );

	CQuickLock oLock( m_pSection );

	BOOL bHandled = FALSE;
	
	CQuickLock oLock2( HostCache.BitTorrent.m_pSection );
	CHostCacheHost* pCache = HostCache.BitTorrent.Add(
		&pHost->sin_addr, htons( pHost->sin_port ) );
	if ( ! pCache )
		return FALSE;
	pCache->m_bDHT = TRUE;
	pCache->m_tAck = 0;
	pCache->m_tFailure = 0;
	pCache->m_nFailures = 0;
	pCache->m_bCheckedLocally = TRUE;
	HostCache.BitTorrent.m_nCookie++;

	if ( pRoot->IsType( CBENode::beDict ) )
	{
		// Get version
		CBENode* pVersion = pRoot->GetNode( "v" );
		if ( pVersion && pVersion->IsType( CBENode::beString ) )
		{
			pCache->m_sName = CBTClient::GetAzureusStyleUserAgent(
				(LPBYTE)pVersion->m_pValue, 4 );
		}

		// Get packet type and transaction id
		CBENode* pType = pRoot->GetNode( "y" );
		CBENode* pTransID = pRoot->GetNode( "t" );
		if ( pType && pType->IsType( CBENode::beString ) &&
			pTransID && pTransID->IsType( CBENode::beString ) )
		{
			if ( pType->GetString() == "q" )
			{
				// Query message
				CBENode* pQueryMethod = pRoot->GetNode( "q" );
				CBENode* pQueryData = pRoot->GetNode( "a" );
				if ( pQueryMethod && pQueryMethod->IsType( CBENode::beString ) &&
					pQueryData && pQueryData->IsType( CBENode::beDict ) )
				{
					if ( pQueryMethod->GetString() == "ping" )
					{
						// Ping
						CBENode* pNodeID = pQueryData->GetNode( "id" );
						if ( pNodeID && pNodeID->IsType( CBENode::beString ) &&
							pNodeID->m_nValue == 20 )
						{
							Hashes::BtGuid oNodeGUID;
							CopyMemory( &oNodeGUID[0], pNodeID->m_pValue, 20 );
							oNodeGUID.validate();
							pCache->m_oBtGUID = oNodeGUID;
							pCache->m_sDescription = oNodeGUID.toString();

							Pong( pHost, (LPCSTR)pTransID->m_pValue, (size_t)pTransID->m_nValue );

							bHandled = TRUE;
						}
					}
					else if ( pQueryMethod->GetString() == "find_node" )
					{
						// TODO: Find node
					}
					else if ( pQueryMethod->GetString() == "get_peers" )
					{
						// TODO: Get peers
					}
					else if ( pQueryMethod->GetString() == "announce_peer" )
					{
						// TODO: Announce peer
					}
					// else if ( pQueryMethod->GetString() == "error" ) - ???
					// else Reply: "204 Method Unknown"
				}
			}
			else if ( pType->GetString() == "r" )
			{
				// Response message
				CBENode* pResponse = pRoot->GetNode( "r" );
				if ( pResponse && pResponse->IsType( CBENode::beDict ) )
				{
					CBENode* pNodeID = pResponse->GetNode( "id" );
					if ( pNodeID && pNodeID->IsType( CBENode::beString ) &&
						pNodeID->m_nValue == 20 )
					{
						Hashes::BtGuid oNodeGUID;
						CopyMemory( &oNodeGUID[0], pNodeID->m_pValue, 20 );
						oNodeGUID.validate();
						pCache->m_oBtGUID = oNodeGUID;
						pCache->m_sDescription = oNodeGUID.toString();

						// TODO: Check queries pool for pTransID

						// Save access token
						CBENode* pToken = pResponse->GetNode( "token" );
						if ( pToken && pToken->IsType( CBENode::beString ) )
						{
							pCache->m_Token.SetSize( (INT_PTR)pToken->m_nValue );
							CopyMemory( pCache->m_Token.GetData(), pToken->m_pValue, (size_t)pToken->m_nValue );
						}

						CBENode* pPeers = pResponse->GetNode( "values" );
						if ( pPeers && pPeers->IsType( CBENode::beList) )
						{
						}

						CBENode* pNodes = pResponse->GetNode( "nodes" );
						if ( pNodes && pNodes->IsType( CBENode::beString ) )
						{
						}

						bHandled = TRUE;
					}
				}
			}
			else if ( pType->GetString() == "e" )
			{
				// TODO: Error message
				CBENode* pError = pRoot->GetNode( "e" );
				if ( pError && pError->IsType( CBENode::beList ) )
				{
				}
			}
		}
	}

	if ( bHandled )
	{
		return TRUE;
	}
	// else reply "203 Protocol Error"

	return FALSE;
}

/*BOOL CDHT::Ping(SOCKADDR_IN* pHost)
{
	CBENode pPing;
	CBENode* pPingData = pPing.Add( "a" );
	Hashes::BtGuid oMyGUID( MyProfile.oGUIDBT );
	pPingData->Add( "id" )->SetString( &oMyGUID[0], oMyGUID.byteCount );
	pPing.Add( "y" )->SetString( "q" );
	pPing.Add( "t" )->SetString( "1234" ); // TODO
	pPing.Add( "q" )->SetString( "ping" );
	pPing.Add( "v" )->SetString( theApp.m_pBTVersion, 4 );
	CBuffer pOutput;
	pPing.Encode( &pOutput );
	theApp.Message( MSG_DEBUG, _T("UDP: Sended BitTorrent ping packet to %s: %s"),
		(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), (LPCTSTR)pPing.Encode() );
	return Datagrams.Send( pHost, pOutput );
}*/

BOOL CDHT::Pong(SOCKADDR_IN* pHost, LPCSTR szTransID, size_t nTransIDLength)
{
	CBENode pPong;
	CBENode* pPongData = pPong.Add( "r" );
	Hashes::BtGuid oMyGUID( MyProfile.oGUIDBT );
	pPongData->Add( "id" )->SetString( &oMyGUID[0], oMyGUID.byteCount );
	pPong.Add( "y" )->SetString( "r" );
	pPong.Add( "t" )->SetString( szTransID, nTransIDLength );
	pPong.Add( "v" )->SetString( theApp.m_pBTVersion, 4 );
	CBuffer pOutput;
	pPong.Encode( &pOutput );
	theApp.Message( MSG_DEBUG, _T("UDP: Sended BitTorrent pong packet to %s: %s"),
		(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), (LPCTSTR)pPong.Encode() );
	return Datagrams.Send( pHost, pOutput );
}

/*BOOL CDHT::GetPeers(SOCKADDR_IN* pHost, const Hashes::BtGuid& oNodeGUID, const Hashes::BtHash& oGUID)
{
	CBENode pGetPeers;
	CBENode* pGetPeersData = pGetPeers.Add( "a" );
	pGetPeersData->Add( "id" )->SetString( &oNodeGUID[0], oNodeGUID.byteCount );
	pGetPeersData->Add( "info_hash" )->SetString( &oGUID[0], oGUID.byteCount );
	pGetPeers.Add( "y" )->SetString( "q" );
	pGetPeers.Add( "t" )->SetString( "4567" ); // TODO
	pGetPeers.Add( "q" )->SetString( "get_peers" );
	pGetPeers.Add( "v" )->SetString( theApp.m_pBTVersion, 4 );
	CBuffer pOutput;
	pGetPeers.Encode( &pOutput );
	theApp.Message( MSG_DEBUG, _T("UDP: Sended BitTorrent get peers packet to %s: %s"),
		(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), (LPCTSTR)pGetPeers.Encode() );
	return Datagrams.Send( pHost, pOutput );
}*/
