//
// NeighboursWithG2.h
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

#if !defined(AFX_NEIGHBOURSWITHG2_H__6C703D82_AD8C_48F1_9C6A_B54F5C05231E__INCLUDED_)
#define AFX_NEIGHBOURSWITHG2_H__6C703D82_AD8C_48F1_9C6A_B54F5C05231E__INCLUDED_

#pragma once

#include "GUID.h"
#include "NeighboursWithG1.h"

class CG2Neighbour;
class CG2Packet;


class CNeighboursWithG2 : public CNeighboursWithG1
{
// Construction
public:
	CNeighboursWithG2();
	virtual ~CNeighboursWithG2();
	
// Attributes
public:

// Operations
public:
	virtual void	Connect();
public:
	CG2Packet*		CreateQueryWeb(CGUID* pGUID, CNeighbour* pExcept = NULL);
	CG2Neighbour*	GetRandomHub(CG2Neighbour* pExcept = NULL, CGUID* pGUID = NULL);

};

#endif // !defined(AFX_NEIGHBOURSWITHG2_H__6C703D82_AD8C_48F1_9C6A_B54F5C05231E__INCLUDED_)
