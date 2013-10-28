//
// Neighbours.h
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

#include "NeighboursWithConnect.h"

class CNeighbours : public CNeighboursWithConnect
{
public:
	CNeighbours();
	virtual ~CNeighbours();

	CString GetName(const CNeighbour* pNeighbour) const;
	CString GetAgent(const CNeighbour* pNeighbour) const;
	CString GetNick(const CNeighbour* pNeighbour) const;
	CString GetNeighbourList(LPCTSTR szFormat) const;

public:
	friend class CNeighbour;
	friend class CShakeNeighbour;
};

extern CNeighbours Neighbours;
