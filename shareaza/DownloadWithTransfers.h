//
// DownloadWithTransfers.h
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

#if !defined(AFX_DOWNLOADWITHTRANSFERS_H__B2CEC922_899C_412E_93B6_953073B992DB__INCLUDED_)
#define AFX_DOWNLOADWITHTRANSFERS_H__B2CEC922_899C_412E_93B6_953073B992DB__INCLUDED_

#pragma once

#include "GUID.h"
#include "DownloadWithSources.h"

class CDownloadTransfer;
class CConnection;
class CEDClient;


class CDownloadWithTransfers : public CDownloadWithSources
{
// Construction
public:
	CDownloadWithTransfers();
	virtual ~CDownloadWithTransfers();
	
// Attributes
protected:
	CDownloadTransfer*	m_pTransferFirst;
	CDownloadTransfer*	m_pTransferLast;
	int					m_nTransferCount;
	DWORD				m_tTransferStart;

// Operations
public:
	int			GetTransferCount(int nState = -1, IN_ADDR* pAddress = NULL) const;
	BOOL		StartTransfersIfNeeded(DWORD tNow = 0);
	BOOL		StartNewTransfer(DWORD tNow = 0);
	void		CloseTransfers();
public:
	DWORD		GetAverageSpeed() const;
	DWORD		GetMeasuredSpeed() const;
	BOOL		OnAcceptPush(CGUID* pClientID, CConnection* pConnection);
	BOOL		OnDonkeyCallback(CEDClient* pClient, CDownloadSource* pExcept = NULL);
protected:
	void		AddTransfer(CDownloadTransfer* pTransfer);
	void		RemoveTransfer(CDownloadTransfer* pTransfer);

// Inlines
public:
	inline CDownloadTransfer* GetFirstTransfer() const
	{
		return m_pTransferFirst;
	}
	
	friend class CDownloadTransfer;
};

#endif // !defined(AFX_DOWNLOADWITHTRANSFERS_H__B2CEC922_899C_412E_93B6_953073B992DB__INCLUDED_)
