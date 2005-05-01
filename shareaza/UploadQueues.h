//
// UploadQueues.h
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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

#pragma once

class CUploadQueue;
class CUploadTransfer;

class CLibraryFile;
class CDownload;


class CUploadQueues
{
// Construction
public:
	CUploadQueues();
	virtual ~CUploadQueues();

// Attributes
public:
	CCriticalSection	m_pSection;
	CUploadQueue*		m_pTorrentQueue;
	CUploadQueue*		m_pHistoryQueue;
protected:
	CPtrList			m_pList;

// Operations
public:
	BOOL	Enqueue(CUploadTransfer* pUpload, BOOL bForce = FALSE);
	BOOL	Dequeue(CUploadTransfer* pUpload);
	int		GetPosition(CUploadTransfer* pUpload, BOOL bStart);
	BOOL	StealPosition(CUploadTransfer* pTarget, CUploadTransfer* pSource);
public:
	CUploadQueue*	Create(LPCTSTR pszName = NULL, BOOL bTop = FALSE);
	void			Delete(CUploadQueue* pQueue);
	BOOL			Reorder(CUploadQueue* pQueue, CUploadQueue* pBefore);
	CUploadQueue*	SelectQueue(PROTOCOLID nProtocol, CLibraryFile* pFile);
	CUploadQueue*	SelectQueue(PROTOCOLID nProtocol, CDownload* pFile);
	CUploadQueue*	SelectQueue(PROTOCOLID nProtocol, LPCTSTR pszName, QWORD nSize, BOOL bPartial, LPCTSTR pszShareTags = NULL);
public:
	int		GetTotalBandwidthPoints( BOOL ActiveOnly = FALSE );
	int		GetQueueCapacity();
	int		GetQueuedCount();
	int		GetQueueRemaining();
	int		GetTransferCount();
	BOOL	IsTransferAvailable();
	DWORD	GetDonkeyBandwidth();
	BOOL	CanUpload(PROTOCOLID nProtocol, CLibraryFile *pFile, BOOL bCanQueue = FALSE );	// Can this file be uploaded with the current queue setup?
	int		QueueRank(PROTOCOLID nProtocol, CLibraryFile *pFile );	// What queue position would this file be in?

public:
	void	Clear();
	BOOL	Load();
	BOOL	Save();
	void	CreateDefault();
	void	Validate();
protected:
	void	Serialize(CArchive& ar);

// Inline Access
public:
	inline POSITION GetIterator() const
	{
		return m_pList.GetHeadPosition();
	}

	inline CUploadQueue* GetNext(POSITION& pos) const
	{
		return (CUploadQueue*)m_pList.GetNext( pos );
	}

	inline int GetCount() const
	{
		return m_pList.GetCount();
	}

	inline BOOL Check(CUploadQueue* pQueue) const
	{
		if ( pQueue == NULL ) return FALSE;
		return m_pList.Find( pQueue ) != NULL;
	}

};

extern CUploadQueues UploadQueues;
