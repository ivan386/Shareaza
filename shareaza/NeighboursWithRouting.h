//
// NeighboursWithRouting.h
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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

#if !defined(AFX_NEIGHBOURSWITHROUTING_H__F6307BC0_A35F_43A3_A6BE_98C69D9A351E__INCLUDED_)
#define AFX_NEIGHBOURSWITHROUTING_H__F6307BC0_A35F_43A3_A6BE_98C69D9A351E__INCLUDED_

#pragma once

#include "NeighboursWithED2K.h"

class CPacket;
class CQuerySearch;


class CNeighboursWithRouting : public CNeighboursWithED2K
{
// Construction
public:
	CNeighboursWithRouting();
	virtual ~CNeighboursWithRouting();

// Attributes
public:

// Operations
public:
	int		Broadcast(CPacket* pPacket, CNeighbour* pExcept = NULL, BOOL bGGEP = FALSE);
	int		RouteQuery(CQuerySearch* pSearch, CPacket* pPacket, CNeighbour* pFrom, BOOL bToHubs);

};

#endif // !defined(AFX_NEIGHBOURSWITHROUTING_H__F6307BC0_A35F_43A3_A6BE_98C69D9A351E__INCLUDED_)
