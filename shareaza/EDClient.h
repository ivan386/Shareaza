//
// EDClient.h
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
	Hashes::Guid m_oGUID;
	DWORD		m_nClientID;
	WORD		m_nUDP;
	SOCKADDR_IN	m_pServer;
public:
	CString		m_sNick;
	int			m_nVersion;
	BOOL		m_bEmule;

	int			m_nEmVersion;
	int			m_nEmCompatible;
	DWORD		m_nSoftwareVersion;
public:	//Client capabilities
	BOOL		m_bEmAICH;			// Not supported
	BOOL		m_bEmUnicode;
	BOOL		m_bEmUDPVersion;
	BOOL		m_bEmDeflate;
	BOOL		m_bEmSecureID;		// Not supported
	BOOL		m_bEmSources;
	BOOL		m_bEmRequest;
	BOOL		m_bEmComments;
	BOOL		m_bEmPeerCache;		// Not supported
	BOOL		m_bEmBrowse;		// "View shared files" supported
	BOOL		m_bEmMultiPacket;	// Not supported
	BOOL		m_bEmPreview;		// Preview support
	BOOL		m_bEmLargeFile;		// Large file support
public:
	BOOL		m_bLogin;
	Hashes::Ed2kHash m_oUpED2K;
	QWORD		m_nUpSize;
public:
	CDownloadTransferED2K*	m_pDownload;
	CUploadTransferED2K*	m_pUpload;
	bool					m_bCallbackRequested;
	BOOL					m_bSeeking;
	DWORD					m_nRunExCookie;

	BOOL		m_bOpenChat;
	BOOL		m_bCommentSent;

// Operations
public:
	BOOL	ConnectTo(DWORD nClientID, WORD nClientPort, IN_ADDR* pServerAddress, WORD nServerPort, const Hashes::Guid& oGUID);
	BOOL	Equals(CEDClient* pClient);
	BOOL	Connect();
	void	Remove();
	void	Merge(CEDClient* pClient);
	void	CopyCapabilities(CEDClient* pClient);
	void	Send(CEDPacket* pPacket, BOOL bRelease = TRUE);
	void	OnRunEx(DWORD tNow);
public:
	BOOL	AttachDownload(CDownloadTransferED2K* pDownload);
	void	OnDownloadClose();
	void	OnUploadClose();
	CString	GetSourceURL();
	void	WritePartStatus(CEDPacket* pPacket, CDownload* pDownload);
	BOOL	SeekNewDownload(CDownloadSource* pExcept = NULL);
	inline  void OpenChat() { m_bOpenChat = TRUE; }
	BOOL	SendCommentsPacket(int nRating, LPCTSTR pszComments);
	void	SendPreviewRequest(CDownload* pDownload);
protected:
	void	DeriveSoftwareVersion();		// ID clients using the newer 'SoftwareVersion' tag
	void	DeriveVersion();				// ID clients using the older method(s)
	BOOL	OnLoggedIn();
	void	DetachDownload();
	void	DetachUpload();
	void	NotifyDropped();
public:
	virtual void	AttachTo(CConnection* pConnection);
	virtual void	Close();
protected:
	virtual BOOL	OnRun();
	virtual BOOL	OnConnected();
	virtual void	OnDropped();
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
	BOOL	OnRequestPreview(CEDPacket* pPacket);
	BOOL	OnPreviewAnswer(CEDPacket* pPacket);
// Chat
	BOOL	OnMessage(CEDPacket* pPacket);
// Browse us
	BOOL	OnAskSharedDirs(CEDPacket* pPacket);
	BOOL	OnViewSharedDir(CEDPacket* pPacket);
public:
	BOOL	OnUdpReask(CEDPacket* pPacket);
	BOOL	OnUdpReaskAck(CEDPacket* pPacket);
	BOOL	OnUdpQueueFull(CEDPacket* pPacket);
	BOOL	OnUdpFileNotFound(CEDPacket* pPacket);

	inline BOOL IsOnline() const { return m_bConnected && m_bLogin; }
};
