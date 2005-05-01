//
// Neighbours.h
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

#if !defined(AFX_NEIGHBOURS_H__864DFBBB_7F72_429B_96FD_A98E720FD0D0__INCLUDED_)
#define AFX_NEIGHBOURS_H__864DFBBB_7F72_429B_96FD_A98E720FD0D0__INCLUDED_

#pragma once

#include "NeighboursWithConnect.h"


class CNeighbours : public CNeighboursWithConnect
{
// Construction
public:
	CNeighbours();
	virtual ~CNeighbours();

// Attributes
public:

// Operations
public:
	virtual void	Connect();
	virtual void	Close();
	virtual void	OnRun();

	friend class CNeighbour;
	friend class CShakeNeighbour;
};

extern CNeighbours Neighbours;

#endif // !defined(AFX_NEIGHBOURS_H__864DFBBB_7F72_429B_96FD_A98E720FD0D0__INCLUDED_)
