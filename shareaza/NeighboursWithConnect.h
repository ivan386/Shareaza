//
// NeighboursWithConnect.h
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

#if !defined(AFX_NEIGHBOURSWITHCONNECT_H__7BAE2435_FF99_4E23_8EBE_B7C3E5FFCCB0__INCLUDED_)
#define AFX_NEIGHBOURSWITHCONNECT_H__7BAE2435_FF99_4E23_8EBE_B7C3E5FFCCB0__INCLUDED_

#pragma once

#include "NeighboursWithRouting.h"

class CConnection;


class CNeighboursWithConnect : public CNeighboursWithRouting
{
// Construction
public:
	CNeighboursWithConnect();
	virtual ~CNeighboursWithConnect();

// Operations
public:
	CNeighbour*		ConnectTo(IN_ADDR* pAddress, WORD nPort, PROTOCOLID nProtocol, BOOL bAutomatic = FALSE, BOOL bNoUltraPeer = FALSE);
	CNeighbour*		OnAccept(CConnection* pConnection);
public:
	//G2
	BOOL	IsG2Leaf();									//Check if this node is a G2 Leaf
	BOOL	IsG2Hub();									//Check if this node is a G2 Hub
	BOOL	IsG2HubCapable(BOOL bDebug = FALSE);		//Check if this node can be a G2 Hub
	//Either
	BOOL	NeedMoreHubs(TRISTATE bG2 = TS_UNKNOWN);	//Does this node need more hubs for the specified protocol
	BOOL	NeedMoreLeafs(TRISTATE bG2 = TS_UNKNOWN);	//Does this node need more leaves for the specified protocol
	BOOL	IsHubLoaded(TRISTATE bG2 = TS_UNKNOWN);		//Unused?
	//G1
	BOOL	IsG1Leaf();									//Check if this node is a G1 Leaf
	BOOL	IsG1Ultrapeer();							//Check if this node is a G1 Ultrapeer
	DWORD	IsG1UltrapeerCapable(BOOL bDebug = FALSE);	//Check if this node can be a G1 Ultrapeer
public:
	virtual void	OnRun();
protected:
	void	Maintain();
	void	PeerPrune();

// Data
protected:
	DWORD	m_tPresent[8];
	
};

#endif // !defined(AFX_NEIGHBOURSWITHCONNECT_H__7BAE2435_FF99_4E23_8EBE_B7C3E5FFCCB0__INCLUDED_)
