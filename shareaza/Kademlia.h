//
// Kademlia.h
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

#pragma once

// KADEMLIA versions
#define KADEMLIA_VERSION1_46c			0x01	// eMule 45b - 46c
#define KADEMLIA_VERSION2_47a			0x02	// eMule 47a
#define KADEMLIA_VERSION3_47b			0x03	// eMule 47b
#define KADEMLIA_VERSION				0x05	// current

// PEER[25] = <GUID 16><IP 4><UDP 2><TCP 2><NULL 1>

// KADEMLIA opcodes
#define KADEMLIA_BOOTSTRAP_REQ			0x00	// <PEER (sender) [25]>
#define KADEMLIA2_BOOTSTRAP_REQ			0x01	//
#define KADEMLIA_BOOTSTRAP_RES			0x08	// <CNT [2]> <PEER [25]>*(CNT)
#define KADEMLIA2_BOOTSTRAP_RES			0x09	//
#define KADEMLIA_HELLO_REQ	 			0x10	// <PEER (sender) [25]>
#define KADEMLIA2_HELLO_REQ				0x11	//
#define KADEMLIA_HELLO_RES				0x18	// <PEER (receiver) [25]>
#define KADEMLIA2_HELLO_RES				0x19	//
#define KADEMLIA_REQ		   			0x20	// <TYPE [1]> <HASH (target) [16]> <HASH (receiver) 16>
#define KADEMLIA2_REQ					0x21	//
#define KADEMLIA_RES					0x28	// <HASH (target) [16]> <CNT> <PEER [25]>*(CNT)
#define KADEMLIA2_RES					0x29	//
#define KADEMLIA_SEARCH_REQ				0x30	// <HASH (key) [16]> <ext 0/1 [1]> <SEARCH_TREE>[ext]
//#define UNUSED						0x31	// Old Opcode, don't use.
#define KADEMLIA_SEARCH_NOTES_REQ		0x32	// <HASH (key) [16]>
#define KADEMLIA2_SEARCH_KEY_REQ		0x33	//
#define KADEMLIA2_SEARCH_SOURCE_REQ		0x34	//
#define KADEMLIA2_SEARCH_NOTES_REQ		0x35	//
#define KADEMLIA_SEARCH_RES				0x38	// <HASH (key) [16]> <CNT1 [2]> (<HASH (answer) [16]> <CNT2 [2]> <META>*(CNT2))*(CNT1)
//#define UNUSED						0x39	// Old Opcode, don't use.
#define KADEMLIA_SEARCH_NOTES_RES		0x3A	// <HASH (key) [16]> <CNT1 [2]> (<HASH (answer) [16]> <CNT2 [2]> <META>*(CNT2))*(CNT1)
#define KADEMLIA2_SEARCH_RES			0x3B	//
#define KADEMLIA_PUBLISH_REQ			0x40	// <HASH (key) [16]> <CNT1 [2]> (<HASH (target) [16]> <CNT2 [2]> <META>*(CNT2))*(CNT1)
//#define UNUSED						0x41	// Old Opcode, don't use.
#define KADEMLIA_PUBLISH_NOTES_REQ		0x42	// <HASH (key) [16]> <HASH (target) [16]> <CNT2 [2]> <META>*(CNT2))*(CNT1)
#define	KADEMLIA2_PUBLISH_KEY_REQ		0x43	//
#define	KADEMLIA2_PUBLISH_SOURCE_REQ	0x44	//
#define KADEMLIA2_PUBLISH_NOTES_REQ		0x45	//
#define KADEMLIA_PUBLISH_RES			0x48	// <HASH (key) [16]>
//#define UNUSED						0x49	// Old Opcode, don't use.
#define KADEMLIA_PUBLISH_NOTES_RES		0x4A	// <HASH (key) [16]>
#define	KADEMLIA2_PUBLISH_RES			0x4B	//
#define KADEMLIA_FIREWALLED_REQ			0x50	// <TCPPORT (sender) [2]>
#define KADEMLIA_FINDBUDDY_REQ			0x51	// <TCPPORT (sender) [2]>
#define KADEMLIA_CALLBACK_REQ			0x52	// <TCPPORT (sender) [2]>
#define KADEMLIA_FIREWALLED_RES			0x58	// <IP (sender) [4]>
#define KADEMLIA_FIREWALLED_ACK_RES		0x59	// (null)
#define KADEMLIA_FINDBUDDY_RES			0x5A	// <TCPPORT (sender) [2]>
#define KADEMLIA2_PING					0x60	// (null)
#define KADEMLIA2_PONG					0x61	// (null)

// KADEMLIA parameter
#define KADEMLIA_FIND_VALUE				0x02
#define KADEMLIA_STORE					0x04
#define KADEMLIA_FIND_NODE				0x0B


class CEDPacket;


class CKademlia
{
public:
	BOOL Bootstrap(const SOCKADDR_IN* pHost, bool bKad2 = true);

	BOOL OnPacket(const SOCKADDR_IN* pHost, CEDPacket* pPacket);

protected:
	CCriticalSection m_pSection;

	BOOL Send(const SOCKADDR_IN* pHost, CEDPacket* pPacket);
	BOOL Send(const SOCKADDR_IN* pHost, BYTE nType);
	BOOL SendMyDetails(const SOCKADDR_IN* pHost, BYTE nType, bool bKad2);

	BOOL OnPacket_KADEMLIA_BOOTSTRAP_RES(const SOCKADDR_IN* pHost, CEDPacket* pPacket);
	BOOL OnPacket_KADEMLIA2_BOOTSTRAP_RES(const SOCKADDR_IN* pHost, CEDPacket* pPacket);
	BOOL OnPacket_KADEMLIA2_PING(const SOCKADDR_IN* pHost, CEDPacket* pPacket);
	BOOL OnPacket_KADEMLIA2_PONG(const SOCKADDR_IN* pHost, CEDPacket* pPacket);
};

extern CKademlia Kademlia;
