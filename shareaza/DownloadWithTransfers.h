//
// DownloadWithTransfers.h
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

#include "DownloadWithSources.h"

class CConnection;
class CEDClient;
class CDownloadTransfer;

class CDownloadWithTransfers : public CDownloadWithSources
{
// Construction
protected:
	CDownloadWithTransfers();
	virtual ~CDownloadWithTransfers();
	
// Attributes
private:
	CDownloadTransfer*	m_pTransferFirst;
	CDownloadTransfer*	m_pTransferLast;
	int					m_nTransferCount;
	DWORD				m_tTransferStart;

// Operations
public:
	bool		HasActiveTransfers() const;
	DWORD		GetTransferCount() const;
	DWORD		GetTransferCount(int nState, const IN_ADDR* pAddress = NULL) const;
	QWORD		GetAmountDownloadedFrom(const IN_ADDR* pAddress) const;
	void		CloseTransfers();
	DWORD		GetAverageSpeed() const;
	DWORD		GetMeasuredSpeed() const;
	BOOL		OnAcceptPush(const Hashes::Guid& oClientID, CConnection* pConnection);
	BOOL		OnDonkeyCallback(const CEDClient* pClient, CDownloadSource* pExcept = NULL);
	BOOL		StartNewTransfer(DWORD tNow = 0);
	BOOL		CanStartTransfers(DWORD tNow);
protected:
	BOOL		StartTransfersIfNeeded(DWORD tNow = 0);
private:
	void		AddTransfer(CDownloadTransfer* pTransfer);
	void		RemoveTransfer(CDownloadTransfer* pTransfer);

// Inlines
public:
	inline bool ValidTransfer(const IN_ADDR* pAddress, const CDownloadTransfer* pTransfer) const;
	inline CDownloadTransfer* GetFirstTransfer() const { return m_pTransferFirst; }

	friend class CDownloadTransfer; // AddTransfer && RemoveTransfer
};
