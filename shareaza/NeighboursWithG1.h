//
// NeighboursWithG1.h
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

#if !defined(AFX_NEIGHBOURSWITHG1_H__6CDF87CC_1439_4C48_84E5_D44ADE9141A7__INCLUDED_)
#define AFX_NEIGHBOURSWITHG1_H__6CDF87CC_1439_4C48_84E5_D44ADE9141A7__INCLUDED_

#pragma once

#include "NeighboursBase.h"

class CG1Neighbour;
class CRouteCache;
class CPongCache;


class CNeighboursWithG1 : public CNeighboursBase
{
// Construction
public:
	CNeighboursWithG1();
	virtual ~CNeighboursWithG1();
	
// Attributes
public:
	CRouteCache*	m_pPingRoute;
	CPongCache*		m_pPongCache;
	
// Operations
public:
	virtual void	Connect();
	virtual void	Close();
protected:
	virtual void	Remove(CNeighbour* pNeighbour);
public:
	void	OnG1Ping();
	void	OnG1Pong(CG1Neighbour* pFrom, IN_ADDR* pAddress, WORD nPort, BYTE nHops, DWORD nFiles, DWORD nVolume);

};

#endif // !defined(AFX_NEIGHBOURSWITHG1_H__6CDF87CC_1439_4C48_84E5_D44ADE9141A7__INCLUDED_)
