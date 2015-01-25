//
// BTClient.h
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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

class CBTPacket;
class CDownload;
class CDownloadSource;
class CDownloadTransferBT;
class CUploadTransferBT;


class CBTClient : public CTransfer
{
// Construction
public:
	CBTClient();
	virtual ~CBTClient();

// Attributes
public:
    Hashes::BtGuid          m_oGUID;
	BOOL					m_bExtended;		// Extension Protocol support
	BOOL					m_bDHTPort;			// DHT Port support
	CUploadTransferBT*		m_pUploadTransfer;
	BOOL					m_bSeeder;
	BOOL					m_bPrefersEncryption;
	CDownload*				m_pDownload;
	CDownloadTransferBT*	m_pDownloadTransfer;

protected:
	BOOL					m_bShake;
	BOOL					m_bOnline;
	BOOL					m_bClosing;
	DWORD					m_tLastKeepAlive;
	DWORD					m_tLastUtPex;
	BYTE					m_nUtMetadataID;
	QWORD					m_nUtMetadataSize;
	BYTE					m_nUtPexID;
	BYTE					m_nLtTexID;
	CString					m_sLtTexTrackers;
	BYTE					m_nSrcExchangeID;

// Operations
public:
	virtual BOOL	Connect(CDownloadTransferBT* pDownloadTransfer);
	virtual void	AttachTo(CConnection* pConnection);
	virtual void	Close(UINT nError = 0);

	inline BOOL IsOnline() const
	{
		return m_bOnline;
	}

	static CString	GetAzureusStyleUserAgent(const BYTE* pVendor, size_t nVendor);

	BOOL			OnPacket(CBTPacket* pPacket);
	void			SendExtendedHandshake();
	BOOL			OnExtendedHandshake(CBTPacket* pPacket);
	void			SendMetadataRequest(QWORD nPiece);
	void			SendInfoRequest(QWORD nPiece);
	BOOL			OnMetadataRequest(CBTPacket* pPacket);
	void			SendUtPex(DWORD tConnectedAfter = 0);
	BOOL			OnUtPex(CBTPacket* pPacket);
	void			SendLtTex();
	BOOL			OnLtTex(CBTPacket* pPacket);
	void			SendHandshake(BOOL bPart1, BOOL bPart2);
	BOOL			OnHandshake1();								// First part of handshake
	BOOL			OnHandshake2();								// Second part - Peer ID
	void			SendBeHandshake();							// Send Shareaza client handshake
	BOOL			OnBeHandshake(CBTPacket* pPacket);			// Process Shareaza client handshake
	void			SendSourceRequest();						// Send Shareaza client source request
	BOOL			OnSourceRequest(CBTPacket* pPacket);		// Process Shareaza client source request
	void			SendDHTPort();
	BOOL			OnDHTPort(CBTPacket* pPacket);
	void			Choke();
	void			UnChoke();
	void			Interested();
	void			NotInterested();
	void			Request(DWORD nBlock, DWORD nOffset, DWORD nLength);
	void			Cancel(DWORD nBlock, DWORD nOffset, DWORD nLength);
	void			Have(DWORD nBlock);
	void			Piece(DWORD nBlock, DWORD nOffset, DWORD nLength, LPCVOID pBuffer);
	void			DetermineUserAgent( const Hashes::BtGuid& oGUID );	// Figure out the other client name/version from the peer ID
	void			SetUserAgent( const CString& sUserAgent, BOOL bClientExtended = FALSE, const CString& sNick = CString() );

	CDownloadSource* GetSource() const;							// Get download transfer source

protected:
	void			Send(CBTPacket* pPacket, BOOL bRelease = TRUE);

	virtual BOOL	OnRun();
	virtual BOOL	OnConnected();
	virtual void	OnDropped();
	virtual BOOL	OnWrite();
	virtual BOOL	OnRead();
};
