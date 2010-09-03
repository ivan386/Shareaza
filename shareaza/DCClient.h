//
// DCClient.h
//
// Copyright (c) Shareaza Development Team, 2010.
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

class CDClients;
class CDownloadTransferDC;
class CLibraryFolder;
class CLibraryFile;
class CUploadTransferDC;


class CDCClient : public CTransfer
{
public:
	CDCClient(LPCTSTR szNick = NULL);
	virtual ~CDCClient();

	virtual BOOL	ConnectTo(const IN_ADDR* pAddress, WORD nPort);
	virtual void	AttachTo(CConnection* pConnection);
	virtual void	Close(UINT nError = 0);

	// Second part of handshaking - Send [$MyNick, $Lock,] $Supports, $Direction and $Key commands
	BOOL			Handshake();

protected:
	CString					m_sNick;				// User nick
	CString					m_sRemoteNick;			// Remote user nick
	std::string				m_strKey;				// Remote client key
	BOOL					m_bExtended;			// Using extended protocol
	CStringList				m_oFeatures;			// Remote client supported features
	CDownloadTransferDC*	m_pDownloadTransfer;	// Download stream
	CUploadTransferDC*		m_pUploadTransfer;		// Upload stream
	TRISTATE				m_bDirection;			// TRI_TRUE - remote client want download, TRI_FALSE - upload.
	int						m_nNumber;				// My number (0...0x7fff)
	int						m_nRemoteNumber;		// Remote client number (0...0x7fff), -1 - unknown.
	BOOL					m_bLock;				// Got $Lock command
	BOOL					m_bClosing;				// Client closing

	virtual BOOL	OnConnected();
	virtual void	OnDropped();
	virtual BOOL	OnRun();
	virtual BOOL	OnRead();
	virtual BOOL	OnWrite();

	// Read single command from input buffer
	BOOL			ReadCommand(std::string& strLine);
	// Got DC++ command
	BOOL			OnCommand(const std::string& strCommand, const std::string& strParams);
	// Got $Lock command
	BOOL			OnLock(const std::string& strLock);
	// Got chat message
	BOOL			OnChat(const std::string& strMessage);
	// Generating challenge for this client
	std::string		GenerateLock() const;
	// First part of handshaking - Send $MyNick, $Lock
	BOOL			Greetings();
	// Can Shareaza start download?
	BOOL			CanDownload() const;

	friend class CDCClients;
	friend class CDownloadTransferDC;
	friend class CUploadTransferDC;
};
