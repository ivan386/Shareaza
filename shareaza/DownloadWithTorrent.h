//
// DownloadWithTorrent.h
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

#if !defined(AFX_DOWNLOADWITHTORRENT_H__0F93FE22_BFCF_4B6E_8416_7C896432E65A__INCLUDED_)
#define AFX_DOWNLOADWITHTORRENT_H__0F93FE22_BFCF_4B6E_8416_7C896432E65A__INCLUDED_

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
public:
	CDownloadWithTorrent();
	virtual ~CDownloadWithTorrent();
	
// Attributes
public:
	CBTInfo		m_pTorrent;
	BOOL		m_bTorrentRequested;
	BOOL		m_bTorrentStarted;
	DWORD		m_tTorrentTracker;
	QWORD		m_nTorrentUploaded;
	QWORD		m_nTorrentDownloaded;
	BOOL		m_bTorrentEndgame;
	BOOL		m_bTorrentTrackerError;
	CString		m_sTorrentTrackerError;
	SHA1		m_pPeerID;
	int			m_nStartTorrentDownloads;
protected:
	BYTE*		m_pTorrentBlock;
	DWORD		m_nTorrentBlock;
	DWORD		m_nTorrentSize;
	DWORD		m_nTorrentSuccess;
	BOOL		m_bSeeding;
private:
	CPtrList	m_pTorrentUploads;
	DWORD		m_tTorrentChoke;
	DWORD		m_tTorrentSources;
	
// Operations
public:
	virtual void	Serialize(CArchive& ar, int nVersion);
	BOOL			SetTorrent(CBTInfo* pTorrent);
	void			AddUpload(CUploadTransferBT* pUpload);
	void			RemoveUpload(CUploadTransferBT* pUpload);
	void			ChokeTorrent(DWORD tNow = 0);
	void			OnTrackerEvent(BOOL bSuccess, LPCTSTR pszReason = NULL);
	virtual BOOL	FindMoreSources();
	BOOL			SeedTorrent(LPCTSTR pszTarget);
	void			CloseTorrent();
	inline BOOL		IsSeeding() const { return m_bSeeding; }
	float			GetRatio() const;
	BOOL 			CheckTorrentRatio() const;
public:
	CDownloadTransferBT*	CreateTorrentTransfer(CBTClient* pClient);
	CBTPacket*				CreateBitfieldPacket();
protected:
	BOOL			GenerateTorrentDownloadID();	//Generate Peer ID
	BOOL			RunTorrent(DWORD tNow);
	void			OnFinishedTorrentBlock(DWORD nBlock);
	void			CloseTorrentUploads();
	
	friend class CDownloadTransferBT;
};

enum
{
	dtAlways, dtWhenRatio, dtNever
};

#endif // !defined(AFX_DOWNLOADWITHTORRENT_H__0F93FE22_BFCF_4B6E_8416_7C896432E65A__INCLUDED_)
