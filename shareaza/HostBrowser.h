//
// HostBrowser.h
//
// Copyright © Shareaza Development Team, 2002-2009.
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

#if !defined(AFX_HOSTBROWSER_H__062DB5F6_EAE7_484D_BA12_28B4BCD99599__INCLUDED_)
#define AFX_HOSTBROWSER_H__062DB5F6_EAE7_484D_BA12_28B4BCD99599__INCLUDED_

#pragma once

#include "Transfer.h"
#include "zlib.h"

class CG1Packet;
class CG2Packet;
class CGProfile;
class CBuffer;
class CVendor;
class CBrowseHostWnd;


class CHostBrowser : public CTransfer
{
// Construction
public:
	CHostBrowser(CBrowseHostWnd* pNotify = NULL, IN_ADDR* pAddress = NULL, WORD nPort = 0,
		BOOL bMustPush = FALSE, const Hashes::Guid& pClientID = Hashes::Guid());
	virtual ~CHostBrowser();

// Attributes
public:
	int				m_nState;
	CBrowseHostWnd*	m_pNotify;
	CGProfile*		m_pProfile;
public:
	BOOL			m_bNewBrowse;
	IN_ADDR			m_pAddress;
	WORD			m_nPort;
	Hashes::Guid	m_oClientID;
	Hashes::Guid	m_oPushID;
	BOOL			m_bMustPush;
	BOOL			m_bCanPush;
	DWORD			m_tPushed;
	BOOL			m_bConnect;
	int				m_nHits;
	CVendor*		m_pVendor;
	BOOL			m_bCanChat;
public:
	CString			m_sServer;
	BOOL			m_bDeflate;
	DWORD			m_nLength;
	DWORD			m_nReceived;
	CBuffer*		m_pBuffer;
	z_streamp		m_pInflate;

	enum { hbsNull, hbsConnecting, hbsRequesting, hbsHeaders, hbsContent };

// Operations
public:
	void		Serialize(CArchive& ar);
	BOOL		Browse();
	void		Stop(BOOL bCompleted = FALSE);
	BOOL		IsBrowsing() const;
	float		GetProgress() const;
protected:
	BOOL		SendPush(BOOL bMessage);
	void		SendRequest();
	BOOL		ReadResponseLine();
	BOOL		ReadContent();
	BOOL		StreamContent();
	BOOL		StreamPacketsG1();
	BOOL		StreamPacketsG2();
	BOOL		StreamHTML();
	BOOL		OnPacket(CG1Packet* pPacket);
	BOOL		OnPacket(CG2Packet* pPacket);
	void		OnProfilePacket(CG2Packet* pPacket);
protected:
	virtual BOOL	OnConnected();
	virtual BOOL	OnRead();
	virtual void	OnDropped();
	virtual BOOL	OnHeaderLine(CString& strHeader, CString& strValue);
	virtual BOOL	OnHeadersComplete();
	virtual BOOL	OnRun();
public:
	virtual BOOL	OnPush(const Hashes::Guid& oClientID, CConnection* pConnection);

};

#endif // !defined(AFX_HOSTBROWSER_H__062DB5F6_EAE7_484D_BA12_28B4BCD99599__INCLUDED_)
