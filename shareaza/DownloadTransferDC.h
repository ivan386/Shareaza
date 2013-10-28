//
// DownloadTransferDC.h 
//
// Copyright (c) Shareaza Development Team, 2010-2013.
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

class CDCClient;
class CDownloadTransfer;


class CDownloadTransferDC : public CDownloadTransfer
{
public:
	CDownloadTransferDC(CDownloadSource* pSource, CDCClient* pClient);
	virtual ~CDownloadTransferDC();

	CDCClient*	m_pClient;		// Download owner

	virtual void	AttachTo(CConnection* pConnection);
	virtual BOOL	Initiate();
	virtual void	Close(TRISTATE bKeepSource, DWORD nRetryAfter = 0);
	virtual DWORD	GetMeasuredSpeed();
	virtual BOOL	OnConnected();
	virtual void	OnDropped();
	virtual BOOL	OnRun();
	virtual BOOL	OnRead();

	// Got $ADCSND command
	BOOL			OnDownload(const std::string& strType, const std::string& strFilename, QWORD nOffset, QWORD nLength, const std::string& strOptions);
	// Got $MaxedOut command with queue position
	BOOL			OnQueue(int nQueue);
	// Got $MaxedOut command without queue position
	BOOL			OnBusy();
	// Got $Error command
	BOOL			OnError();
	// Check if transfer idle
	BOOL			IsIdle() const;

protected:
	QWORD			m_nTigerLength;

	virtual BOOL	SubtractRequested(Fragments::List& ppFragments) const;
	BOOL			StartNextFragment();
	BOOL			ReadContent();
	BOOL			ReadTiger();
};
