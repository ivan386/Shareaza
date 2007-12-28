//
// QueryKeys.h
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

#if !defined(AFX_QUERYKEYS_H__EF6821C2_2183_44C2_9684_66160F8A81DA__INCLUDED_)
#define AFX_QUERYKEYS_H__EF6821C2_2183_44C2_9684_66160F8A81DA__INCLUDED_

#pragma once


class CQueryKeys
{
// Construction
public:
	CQueryKeys();
	virtual ~CQueryKeys();

// Attributes
protected:
	DWORD	m_nBits;
	DWORD*	m_pTable;
	DWORD	m_nTable;
	DWORD*	m_pMap;

// Operations
public:
	DWORD	Create(DWORD nAddress);
	BOOL	Check(DWORD nAddress, DWORD nKey);

};

#endif // !defined(AFX_QUERYKEYS_H__EF6821C2_2183_44C2_9684_66160F8A81DA__INCLUDED_)
