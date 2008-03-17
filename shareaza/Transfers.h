//
// Transfers.h
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

#if !defined(AFX_TRANSFERS_H__950AC162_FF34_4B40_8D8A_2745AA245316__INCLUDED_)
#define AFX_TRANSFERS_H__950AC162_FF34_4B40_8D8A_2745AA245316__INCLUDED_

#pragma once

class CTransfer;

class CTransfers;

extern CTransfers Transfers;

class CTransfers
{
// Construction
public:
	CTransfers();
	virtual ~CTransfers();

// Attributes
public:
	mutable CMutex	m_pSection;
	DWORD			m_nBuffer;
	BYTE*			m_pBuffer;
protected:
	CList< CTransfer* > m_pList;
	volatile HANDLE	m_hThread;
	volatile BOOL	m_bThread;
	CEvent			m_pWakeup;
	DWORD			m_nRunCookie;

// Operations
public:
	INT_PTR		GetActiveCount() const;
	BOOL		IsConnectedTo(IN_ADDR* pAddress) const;
	BOOL		StartThread();
	void		StopThread();
protected:
	static UINT	ThreadStart(LPVOID pParam);
	void		OnRun();
	void		OnRunTransfers();
	void		OnCheckExit();
protected:
	void		Add(CTransfer* pTransfer);
	void		Remove(CTransfer* pTransfer);

// List Access
public:
	inline POSITION GetIterator() const
	{
		return m_pList.GetHeadPosition();
	}

	inline CTransfer* GetNext(POSITION& pos) const
	{
		return m_pList.GetNext( pos );
	}

	inline INT_PTR GetCount() const
	{
		return m_pList.GetCount();
	}

	inline BOOL Check(CTransfer* pTransfer) const
	{
		return m_pList.Find( pTransfer ) != NULL;
	}

	friend class CTransfer;
	friend class CUpload;
	friend class CDownloadWithTransfers;

};

#endif // !defined(AFX_TRANSFERS_H__950AC162_FF34_4B40_8D8A_2745AA245316__INCLUDED_)
