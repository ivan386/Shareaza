//
// Transfers.h
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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

#include "ThreadImpl.h"

class CTransfer;
class CTransfers;


class CTransfers :
	public CThreadImpl
{
public:
	CTransfers();
	virtual ~CTransfers();

	mutable CMutexEx	m_pSection;

	BOOL		IsConnectedTo(const IN_ADDR* pAddress) const;
	BOOL		StartThread();
	void		StopThread();
	void		Add(CTransfer* pTransfer);
	void		Remove(CTransfer* pTransfer);

	INT_PTR		GetActiveCount() const;

private:
	CList< CTransfer* >	m_pList;
	DWORD				m_nRunCookie;

	void		OnRun();
	void		OnRunTransfers();
	void		OnCheckExit();
};

extern CTransfers Transfers;
