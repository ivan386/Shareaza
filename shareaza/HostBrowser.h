//
// HostBrowser.h
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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

class CBrowseHostWnd;
class CBuffer;
class CEDPacket;
class CG1Packet;
class CG2Packet;
class CGProfile;
class CLibraryFile;
class CQueryHit;
class CXMLElement;

#include "Transfer.h"
#include "VendorCache.h"


class CHostBrowser : public CTransfer
{
public:
	CHostBrowser(CBrowseHostWnd* pNotify = NULL, PROTOCOLID nProtocol = PROTOCOL_ANY, IN_ADDR* pAddress = NULL, WORD nPort = 0, BOOL bMustPush = FALSE, const Hashes::Guid& pClientID = Hashes::Guid(), const CString& sNick = CString());
	virtual ~CHostBrowser();

	CGProfile*		m_pProfile;
	IN_ADDR			m_pAddress;
	WORD			m_nPort;
	Hashes::Guid	m_oClientID;
	BOOL			m_bMustPush;
	DWORD			m_tPushed;
	BOOL			m_bConnect;
	int				m_nHits;
	BOOL			m_bCanChat;
	CString			m_sServer;

	enum { hbsNull, hbsConnecting, hbsRequesting, hbsHeaders, hbsContent };

// Operations
public:
	void		Serialize(CArchive& ar, int nVersion /* BROWSER_SER_VERSION */);
	BOOL		Browse();
	void		Stop(BOOL bCompleted = FALSE);
	BOOL		IsBrowsing() const;
	float		GetProgress() const;
	void		OnQueryHits(CQueryHit* pHits);
	BOOL		OnPush(const Hashes::Guid& oClientID, CConnection* pConnection);
	BOOL		OnNewFile(const CLibraryFile* pFile);

	virtual BOOL	OnConnected();
	virtual void	OnDropped();
	virtual BOOL	OnHeadersComplete();

protected:
	CBrowseHostWnd*	m_pNotify;
	BOOL			m_bNewBrowse;
	Hashes::Guid	m_oPushID;
	BOOL			m_bCanPush;
	CVendorPtr		m_pVendor;
	BOOL			m_bDeflate;
	DWORD			m_nReceived;
	CBuffer*		m_pBuffer;
	z_streamp		m_pInflate;
	CString			m_sNick;

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
	BOOL		LoadDC(LPCTSTR pszFile, CQueryHit*& pHits);
	BOOL		LoadDCDirectory(CXMLElement* pRoot, CQueryHit*& pHits);

	virtual BOOL	OnRead();
	virtual BOOL	OnHeaderLine(CString& strHeader, CString& strValue);
	virtual BOOL	OnRun();
};
