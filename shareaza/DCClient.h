//
// DCClient.h
//
// Copyright (c) Shareaza Development Team, 2010-2011.
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

class CDCNeighbour;
class CDownloadTransferDC;
class CUploadTransferDC;

// Class for DC++ remote client connection.
// This class uses fictive CUploadTransferDC and CDownloadTransferDC classes 
// to mimic download and upload connections like CEDClient.

class CDCClient : public CTransfer
{
public:
	CDCClient(const IN_ADDR* pHubAddress = NULL, WORD nHubPort = 0, LPCTSTR szNick = NULL, LPCTSTR szRemoteNick = NULL);
	virtual ~CDCClient();

	Hashes::Guid	m_oGUID;				// GUID to identify callback connections

	virtual BOOL	ConnectTo(const IN_ADDR* pAddress, WORD nPort);
	virtual void	AttachTo(CConnection* pConnection);
	virtual void	Close(UINT nError = 0);
	virtual BOOL	OnRun();

	// Detect remote user agent
	CString			GetUserAgent();
	// Re-connect
	BOOL			Connect();
	// Attach download transfer
	void			AttachDownload(CDownloadTransferDC* pTransfer);
	// When download transfer closed
	void			OnDownloadClose();
	// When upload transfer closed
	void			OnUploadClose();
	// Second part of handshaking - Send [$MyNick, $Lock,] $Supports, $Direction and $Key commands
	BOOL			Handshake();
	// Send command
	BOOL			SendCommand(const CString& strSend);
	// Merge all useful data from old to current client and then destroy it
	void			Merge(CDCClient* pClient);
	// Destroy this object
	void			Remove();
	// Check if client on-line
	BOOL			IsOnline() const;
	// Check if client busy downloading something (not in command mode)
	BOOL			IsDownloading() const;
	// Check if client busy uploading something (not in command mode)
	BOOL			IsUploading() const;
	// Check if client does nothing (no upload, no download)
	BOOL			IsIdle() const;
	// Accept push connection
	BOOL			OnPush();

protected:
	CString			m_sNick;				// User nick
	CDownloadTransferDC*	m_pDownloadTransfer;	// Download stream
	CUploadTransferDC*		m_pUploadTransfer;		// Upload stream
	std::string		m_strKey;				// Key calculated for remote client lock
	BOOL			m_bExtended;			// Using extended protocol
	CStringList		m_oFeatures;			// Remote client supported features
	TRISTATE		m_bDirection;			// Got $Direction command: TRI_TRUE - remote client want download, TRI_FALSE - upload.
	BOOL			m_bNumberSent;			// My $Direction number sent
	int				m_nNumber;				// My $Direction number (0...0x7fff)
	int				m_nRemoteNumber;		// Remote client $Direction number (0...0x7fff), -1 - unknown.
	BOOL			m_bLogin;				// Got $Lock command
	BOOL			m_bKey;					// Got $Key command

	virtual BOOL	OnConnected();
	virtual void	OnDropped();
	virtual BOOL	OnRead();
	virtual BOOL	OnWrite();

	// Read single command from input buffer
	BOOL			ReadCommand(std::string& strLine);
	// Got DC++ command
	BOOL			OnCommand(const std::string& strCommand, const std::string& strParams);
	// Got $MyNick command
	BOOL			OnMyNick(const std::string& strParams);
	// Got $Lock command
	BOOL			OnLock(const std::string& strParams);
	// Got $Supports command
	BOOL			OnSupports(const std::string& strParams);
	// Got $Key command
	BOOL			OnKey(const std::string& strParams);
	// Got $Direction command
	BOOL			OnDirection(const std::string& strParams);
	// Got $ADCGET command
	BOOL			OnADCGet(const std::string& strParams);
	// Got $ADCSND command
	BOOL			OnADCSnd(const std::string& strParams);
	// Got $Get command
	BOOL			OnGet(const std::string& strParams);
	// Got $Send command
	BOOL			OnSend(const std::string& strParams);
	// Got $MaxedOut command
	BOOL			OnMaxedOut(const std::string& strParams);
	// Got $Error command
	BOOL			OnError(const std::string& strParams);
	// Generating challenge for this client
	std::string		GenerateLock() const;
	// Generating direction number
	int				GenerateNumber() const;
	// First part of handshaking - Send $MyNick, $Lock
	BOOL			Greetings();
	// Can Shareaza start download?
	BOOL			CanDownload() const;
	// Can Shareaza start upload?
	BOOL			CanUpload() const;
	// Close download
	void			DetachDownload();
	// Close upload
	void			DetachUpload();
	// Start download
	BOOL			StartDownload();
};
