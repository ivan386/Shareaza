//
// UploadTransferBT.h
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

#if !defined(AFX_UPLOADTRANSFERBT_H__3F81C2F1_E76C_4CCB_8AA2_AD6B9D877CBA__INCLUDED_)
#define AFX_UPLOADTRANSFERBT_H__3F81C2F1_E76C_4CCB_8AA2_AD6B9D877CBA__INCLUDED_

#pragma once

#include "UploadTransfer.h"

class CBTClient;
class CBTPacket;


class CUploadTransferBT : public CUploadTransfer
{
// Construction
public:
	CUploadTransferBT(CBTClient* pClient, CDownload* pDownload);
	virtual ~CUploadTransferBT();
	
// Attributes
public:
	CBTClient*		m_pClient;
	CDownload*		m_pDownload;
public:
	BOOL			m_bInterested;
	BOOL			m_bChoked;
	int				m_nRandomUnchoke;
	DWORD			m_tRandomUnchoke;
public:
	CFileFragmentQueue m_pRequested;
	CFileFragmentList m_pServed;
	
// Operations
public:
	void			SetChoke(BOOL bChoke);
	virtual void	Close(BOOL bMessage = FALSE);
	virtual DWORD	GetMeasuredSpeed();
	virtual BOOL	OnConnected();
	virtual BOOL	OnRun();
public:
	BOOL	OnInterested(CBTPacket* pPacket);
	BOOL	OnUninterested(CBTPacket* pPacket);
	BOOL	OnRequest(CBTPacket* pPacket);
	BOOL	OnCancel(CBTPacket* pPacket);
protected:
	BOOL	OpenFile();
	BOOL	ServeRequests();
	
};

#endif // !defined(AFX_UPLOADTRANSFERBT_H__3F81C2F1_E76C_4CCB_8AA2_AD6B9D877CBA__INCLUDED_)
