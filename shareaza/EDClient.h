//
// EDClient.h
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

#if !defined(AFX_EDCLIENT_H__4D9C179A_0C83_4A98_8CC0_E82644224697__INCLUDED_)
#define AFX_EDCLIENT_H__4D9C179A_0C83_4A98_8CC0_E82644224697__INCLUDED_

#pragma once

#include "Transfer.h"

class CEDPacket;
class CDownload;
class CDownloadSource;
class CDownloadTransferED2K;
class CUploadTransferED2K;


class CEDClient : public CTransfer
{
// Construction
public:
	CEDClient();
	virtual ~CEDClient();
	
// Attributes
public:
	CEDClient*	m_pEdPrev;
	CEDClient*	m_pEdNext;
public:
	BOOL		m_bGUID;
	GGUID		m_pGUID;
	DWORD		m_nClientID;
	WORD		m_nUDP;
	SOCKADDR_IN	m_pServer;
public:
	CString		m_sNick;
	int			m_nVersion;
	BOOL		m_bEmule;
	BOOL		m_bEmSources;
	BOOL		m_bEmComments;
	BOOL		m_bEmRequest;
	BOOL		m_bEmDeflate;
	int			m_nEmVersion;
	int			m_nEmCompatible;
	DWORD		m_nSoftwareVersion;
public:
	BOOL		m_bLogin;
	BOOL		m_bUpMD4;
	MD4			m_pUpMD4;
	QWORD		m_nUpSize;
public:
	CDownloadTransferED2K*	m_pDownload;
	CUploadTransferED2K*	m_pUpload;
	BOOL					m_bSeeking;
	DWORD					m_nRunExCookie;
	
// Operations
public:
	BOOL	ConnectTo(DWORD nClientID, WORD nClientPort, IN_ADDR* pServerAddress, WORD nServerPort, GGUID* pGUID);
	BOOL	Equals(CEDClient* pClient);
	BOOL	Connect();
	void	Remove();
	void	Merge(CEDClient* pClient);
	void	Send(CEDPacket* pPacket, BOOL bRelease = TRUE);
	void	OnRunEx(DWORD tNow);
public:
	BOOL	AttachDownload(CDownloadTransferED2K* pDownload);
	void	OnDownloadClose();
	void	OnUploadClose();
	CString	GetSourceURL();
	void	WritePartStatus(CEDPacket* pPacket, CDownload* pDownload);
	BOOL	SeekNewDownload(CDownloadSource* pExcept = NULL);
protected:
	void	DeriveVersion();
	BOOL	OnLoggedIn();
	void	DetachDownload();
	void	DetachUpload();
	void	NotifyDropped(BOOL bError = TRUE);
public:
	virtual void	AttachTo(CConnection* pConnection);
	virtual void	Close();
protected:
	virtual BOOL	OnRun();
	virtual BOOL	OnConnected();
	virtual void	OnDropped(BOOL bError);
	virtual BOOL	OnWrite();
	virtual BOOL	OnRead();
protected:
	BOOL	OnPacket(CEDPacket* pPacket);
	void	SendHello(BYTE nType);
	BOOL	OnHello(CEDPacket* pPacket);
	void	SendEmuleInfo(BYTE nType);
	BOOL	OnEmuleInfo(CEDPacket* pPacket);
	BOOL	OnFileRequest(CEDPacket* pPacket);
	BOOL	OnFileStatusRequest(CEDPacket* pPacket);
	BOOL	OnHashsetRequest(CEDPacket* pPacket);
	BOOL	OnQueueRequest(CEDPacket* pPacket);
	BOOL	OnSourceRequest(CEDPacket* pPacket);
	BOOL	OnSourceAnswer(CEDPacket* pPacket);
public:
	BOOL	OnUdpReask(CEDPacket* pPacket);
	BOOL	OnUdpReaskAck(CEDPacket* pPacket);
	BOOL	OnUdpQueueFull(CEDPacket* pPacket);
	BOOL	OnUdpFileNotFound(CEDPacket* pPacket);
	
	inline BOOL IsOnline() const { return m_bConnected && m_bLogin; }
};

#endif // !defined(AFX_EDCLIENT_H__4D9C179A_0C83_4A98_8CC0_E82644224697__INCLUDED_)
