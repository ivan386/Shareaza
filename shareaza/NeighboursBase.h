//
// NeighboursBase.h
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

#if !defined(AFX_NEIGHBOURSBASE_H__EE5DBF25_1EC6_40A7_9343_7373A2A882AD__INCLUDED_)
#define AFX_NEIGHBOURSBASE_H__EE5DBF25_1EC6_40A7_9343_7373A2A882AD__INCLUDED_

#pragma once

class CNeighbour;


class CNeighboursBase
{
// Construction
public:
	CNeighboursBase();
	virtual ~CNeighboursBase();

// Attributes
protected:
	CMapPtrToPtr	m_pUniques;
	DWORD			m_nUnique;
	DWORD			m_nRunCookie;
public:
	DWORD			m_nStableCount;
	DWORD			m_nLeafCount;
	DWORD			m_nLeafContent;
	DWORD			m_nBandwidthIn;
	DWORD			m_nBandwidthOut;

// Operations
public:
	POSITION		GetIterator() const;
	CNeighbour*		GetNext(POSITION& pos) const;
	CNeighbour*		Get(DWORD nUnique) const;
	CNeighbour*		Get(IN_ADDR* pAddress) const;
	int				GetCount(PROTOCOLID nProtocol, int nState, int nNodeType) const;
public:
	virtual void	Connect();
	virtual void	Close();
	virtual void	OnRun();
protected:
	virtual void	Add(CNeighbour* pNeighbour, BOOL bAssignUnique = TRUE);
	virtual void	Remove(CNeighbour* pNeighbour);

	friend class CNeighbour;
	friend class CShakeNeighbour;
	friend class CEDNeighbour;
};


#endif // !defined(AFX_NEIGHBOURSBASE_H__EE5DBF25_1EC6_40A7_9343_7373A2A882AD__INCLUDED_)
