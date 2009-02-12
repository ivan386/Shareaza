//
// QueryHashMaster.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#if !defined(AFX_QUERYHASHMASTER_H__0CEA9340_D90C_4B95_8720_A2FCF769E4A2__INCLUDED_)
#define AFX_QUERYHASHMASTER_H__0CEA9340_D90C_4B95_8720_A2FCF769E4A2__INCLUDED_

#pragma once

#include "QueryHashTable.h"

class CQueryHashGroup;


class CQueryHashMaster : public CQueryHashTable
{
// Construction
public:
	CQueryHashMaster();
	virtual ~CQueryHashMaster();

// Attributes
protected:
	CList< CQueryHashGroup* > m_pGroups;
	int			m_nPerGroup;
	BOOL		m_bValid;

// Operations
public:
	void		Create();
	void		Add(CQueryHashTable* pTable);
	void		Remove(CQueryHashTable* pTable);
	void		Build();

// Inlines
public:
	inline POSITION GetIterator() const
	{
		return m_pGroups.GetHeadPosition();
	}

	inline CQueryHashGroup* GetNext(POSITION& pos) const
	{
		return m_pGroups.GetNext( pos );
	}

	inline INT_PTR GetCount() const
	{
		return m_pGroups.GetCount();
	}

	inline void Invalidate()
	{
		m_bValid = FALSE;
	}
};

extern CQueryHashMaster QueryHashMaster;

#endif // !defined(AFX_QUERYHASHMASTER_H__0CEA9340_D90C_4B95_8720_A2FCF769E4A2__INCLUDED_)
