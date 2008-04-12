//
// DownloadWithTorrent.h
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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

#if !defined(DOWNLOADWITHTORRENT_H)
#define DOWNLOADWITHTORRENT_H

#pragma once

#include "DownloadWithFile.h"
#include "BTInfo.h"

class CDownloadTransferBT;
class CUploadTransferBT;
class CBTClient;
class CBTPacket;


class CDownloadWithTorrent : public CDownloadWithFile
{
// Construction
protected:
	CDownloadWithTorrent();
	virtual ~CDownloadWithTorrent();

// Attributes
public:
	CBTInfo		m_pTorrent;
	BOOL		m_bTorrentRequested;
	BOOL		m_bTorrentStarted;
	DWORD		m_tTorrentTracker;
	DWORD		m_tTorrentSources;
	QWORD		m_nTorrentUploaded;
	QWORD		m_nTorrentDownloaded;
	BOOL		m_bTorrentTrackerError;
	CString		m_sTorrentTrackerError;
    Hashes::BtGuid m_pPeerID;
	CString		m_sKey;
    BOOL		m_bTorrentEndgame;
	CString		m_sServingFileName;
protected:
	BOOL		m_bSeeding;
	DWORD		m_nTorrentBlock;
	DWORD		m_nTorrentSuccess;
	DWORD		m_nTorrentSize;
	BYTE*		m_pTorrentBlock;
private:
	CList< CUploadTransferBT* > m_pTorrentUploads;
	DWORD		m_tTorrentChoke;

// Operations
public:
	static	CString	FindTorrentFile(LPVOID pVoid);
	void			AddUpload(CUploadTransferBT* pUpload);
	void			RemoveUpload(CUploadTransferBT* pUpload);
	BOOL			SeedTorrent(LPCTSTR pszTarget);
	inline BOOL		IsSeeding() const { return m_bSeeding; }
	inline BOOL		IsTorrent() const { return m_pTorrent.IsAvailable(); }
	inline BOOL		IsSingleFileTorrent() const { return IsTorrent() && ( m_pTorrent.m_nFiles == 1 ); }
	float			GetRatio() const;
	BOOL			UploadExists(in_addr* pIP) const;
	BOOL			UploadExists(const Hashes::BtGuid& oGUID) const;
	void			OnTrackerEvent(BOOL bSuccess, LPCTSTR pszReason = NULL);
	void			ChokeTorrent(DWORD tNow = 0);
	CDownloadTransferBT*	CreateTorrentTransfer(CBTClient* pClient);
	CBTPacket*		CreateBitfieldPacket();
	BOOL			SetTorrent(CBTInfo* pTorrent);
protected:
	bool			RunTorrent(DWORD tNow);
	void			SendCompleted();
	void			CloseTorrent();
	void			CloseTorrentUploads();
	BOOL 			CheckTorrentRatio() const;
	virtual BOOL	FindMoreSources();
	void			OnFinishedTorrentBlock(DWORD nBlock);
	virtual void	Serialize(CArchive& ar, int nVersion);
private:
	BOOL			GenerateTorrentDownloadID();	//Generate Peer ID
	DWORD			GetRetryTime() const;
	void			SendStarted(DWORD nNumWant);
	void			SendUpdate(DWORD nNumWant);
	void			SendStopped();
	TCHAR			GenerateCharacter() const;

	friend class CDownloads;	// m_bSeeding for Load()
};

#endif // !defined(DOWNLOADWITHTORRENT_H)
