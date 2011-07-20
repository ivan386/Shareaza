//
// EDClient.h
//
// Copyright (c) Shareaza Development Team, 2002-2011.
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
#include "HostBrowser.h"

class CEDPacket;
class CDownload;
class CDownloadSource;
class CDownloadTransferED2K;
class CUploadTransferED2K;


class CEDClient : public CTransfer
{
public:
	CEDClient();

	CEDClient*	m_pEdPrev;
	CEDClient*	m_pEdNext;

	Hashes::Guid m_oGUID;
	DWORD		m_nClientID;
	WORD		m_nUDP;
	SOCKADDR_IN	m_pServer;

	CString		m_sNick;
	DWORD		m_nVersion;
	BOOL		m_bEmule;

	DWORD		m_nEmVersion;
	DWORD		m_nEmCompatible;
	DWORD		m_nSoftwareVersion;

	// Client capabilities 1
	BOOL		m_bEmAICH;					// Not supported
	BOOL		m_bEmUnicode;
	BOOL		m_bEmUDPVersion;
	BOOL		m_bEmDeflate;
	BOOL		m_bEmSecureID;				// Not supported
	BOOL		m_bEmSources;
	BOOL		m_bEmRequest;
	BOOL		m_bEmComments;
	BOOL		m_bEmPeerCache;				// Not supported
	BOOL		m_bEmBrowse;				// Browse supported
	BOOL		m_bEmMultiPacket;			// Not supported
	BOOL		m_bEmPreview;				// Preview supported

	// Client capabilities 2
	BOOL		m_bEmSupportsCaptcha;
	BOOL		m_bEmSupportsSourceEx2;		// Not supported
	BOOL		m_bEmRequiresCryptLayer;	// Not supported
	BOOL		m_bEmRequestsCryptLayer;	// Not supported
	BOOL		m_bEmSupportsCryptLayer;	// Not supported
	BOOL		m_bEmExtMultiPacket;		// Not supported
	BOOL		m_bEmLargeFile;				// Large file supported
	BOOL		m_nEmKadVersion;			// Not supported

	BOOL					m_bLogin;
	Hashes::Ed2kHash		m_oUpED2K;
	QWORD					m_nUpSize;

	CDownloadTransferED2K*	m_pDownloadTransfer;
	CUploadTransferED2K*	m_pUploadTransfer;
	bool					m_bCallbackRequested;
	BOOL					m_bSeeking;
	DWORD					m_nRunExCookie;

	BOOL		m_bOpenChat;
	BOOL		m_bCommentSent;

	DWORD		m_nDirsWaiting;

	BOOL	ConnectTo(DWORD nClientID, WORD nClientPort, IN_ADDR* pServerAddress, WORD nServerPort, const Hashes::Guid& oGUID);
	BOOL	Equals(CEDClient* pClient);
	BOOL	Connect();
	void	Remove();
	void	Merge(CEDClient* pClient);
	void	CopyCapabilities(CEDClient* pClient);
	void	Send(CPacket* pPacket, BOOL bRelease = TRUE);
	void	OnRunEx(DWORD tNow);

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
	virtual ~CEDClient();

	void	DetermineUserAgent();
	BOOL	OnLoggedIn();
	void	DetachDownload();
	void	DetachUpload();
	void	NotifyDropped();
	CHostBrowser*	GetBrowser() const;

	// Get download transfer source
	CDownloadSource* GetSource() const;

public:
	virtual void	AttachTo(CConnection* pConnection);
	virtual void	Close(UINT nError = 0);

protected:
	virtual BOOL	OnRun();
	virtual BOOL	OnConnected();
	virtual void	OnDropped();
	virtual BOOL	OnWrite();
	virtual BOOL	OnRead();

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
	BOOL	OnChatMessage(CEDPacket* pPacket);
	BOOL	OnCaptchaRequest(CEDPacket* pPacket);
	BOOL	OnCaptchaResult(CEDPacket* pPacket);
// Browse us
	BOOL	OnAskSharedDirs(CEDPacket* pPacket);
	BOOL	OnViewSharedDir(CEDPacket* pPacket);
// Browse remote host
	BOOL	OnAskSharedDirsAnswer(CEDPacket* pPacket);
	BOOL	OnViewSharedDirAnswer(CEDPacket* pPacket);
	BOOL	OnAskSharedDirsDenied(CEDPacket* pPacket);

public:
	BOOL	OnUdpReask(CEDPacket* pPacket);
	BOOL	OnUdpReaskAck(CEDPacket* pPacket);
	BOOL	OnUdpQueueFull(CEDPacket* pPacket);
	BOOL	OnUdpFileNotFound(CEDPacket* pPacket);

	inline BOOL IsOnline() const { return m_bConnected && m_bLogin; }

	DWORD GetID() const;
};
