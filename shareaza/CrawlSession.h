//
// CrawlSession.h
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

class CG2Packet;
class CCrawlNode;


class CCrawlSession
{
// Construction
public:
	CCrawlSession();
	virtual ~CCrawlSession();

// Attributes
public:
	BOOL		m_bActive;
	CList< CCrawlNode*, CCrawlNode* > m_pNodes;

// Operations
public:
	void		Clear();
	void		Bootstrap();
	void		SendCrawl(SOCKADDR_IN* pHost);
	int			GetHubCount();
	int			GetLeafCount();
public:
	void		OnRun();
	void		OnCrawl(const SOCKADDR_IN* pHost, CG2Packet* pPacket);
protected:
	CCrawlNode*	Find(const IN_ADDR* pAddress, BOOL bCreate);

	friend class CCrawlNode;
};


class CCrawlNode
{
// Construction
public:
	CCrawlNode();
	virtual ~CCrawlNode();

// Attributes
public:
	SOCKADDR_IN		m_pHost;
	int				m_nType;
	int				m_nLeaves;
	CString			m_sNick;
	float			m_nLatitude;
	float			m_nLongitude;
public:
	CList< CCrawlNode* > m_pNeighbours;
public:
	POSITION		m_nUnique;
	DWORD			m_tDiscovered;
	DWORD			m_tCrawled;
	DWORD			m_tResponse;

	enum { ntUnknown, ntHub, ntLeaf };

// Operations
public:
	void	OnCrawl(CCrawlSession* pSession, CG2Packet* pPacket);
protected:
	void	OnNode(CCrawlSession* pSession, CG2Packet* pPacket, DWORD nPacket, int nType);

	enum { parseSelf, parseHub, parseLeaf };
};

extern CCrawlSession CrawlSession;
