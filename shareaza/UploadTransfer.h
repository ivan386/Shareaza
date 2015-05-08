//
// UploadTransfer.h
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

#include "Transfer.h"
#include "ShareazaFile.h"
#include "FragmentedFile.h"

class CUploadQueue;
class CUploadFile;
class CLibraryFile;
class CDownload;

#define ULA_SLOTS	16


class CUploadTransfer abstract : public CTransfer, public CShareazaFile
{
public:
	CUploadTransfer(PROTOCOLID nProtocol);
	virtual ~CUploadTransfer();

public:
	CUploadQueue*	m_pQueue;		// Queue reference
	CUploadFile*	m_pBaseFile;	// Reference file
	DWORD			m_nUserRating;	// Has the downloader uploaded anything?

	QWORD			m_nFileBase;	// Base offset in requested file
	BOOL			m_bFilePartial;	// Partial file flag
	CString			m_sFileTags;	// File sharing tags

	BOOL			m_bLive;		// Live connection tag
	DWORD			m_nRequests;	// Request count
	QWORD			m_nUploaded;	// Bytes uploaded
	DWORD			m_tContent;		// Send start timestamp

protected:
	BOOL			m_bStopTransfer;// Should this transfer stop? (to allow queue rotation, etc)
	DWORD			m_tRotateTime;
	DWORD			m_tAverageTime;
	int				m_nAveragePos;
	DWORD			m_nAverageRate[ULA_SLOTS];
	DWORD			m_nMaxRate;		// Maximum average speed we got
	DWORD			m_tRatingTime;	// When rating was last calculated

private:
	auto_ptr< CFragmentedFile > m_pFile;	// Disk file

public:
	virtual void	Remove(BOOL bMessage = TRUE, UINT nError = 0);
	virtual void	Close(UINT nError = 0);
	virtual BOOL	Promote();
	// Rename, delete or release uploading file.
	// pszTarget == 0 - delete file; pszTarget == 1 - release file.
	virtual BOOL	OnRename(LPCTSTR pszSource, LPCTSTR pszTarget);

	virtual float	GetProgress() const;
	virtual DWORD	GetAverageSpeed();
	virtual DWORD	GetMeasuredSpeed();
	virtual DWORD	GetMaxSpeed() const;
	virtual void	SetSpeedLimit(DWORD nLimit);

protected:
	virtual BOOL	OnRun();
	virtual BOOL	OnRead();
	virtual BOOL	OnWrite();
	virtual void	OnQueueKick() {};

	void		LongTermAverage(DWORD tNow);
	void		RotatingQueue(DWORD tNow);
	void		CalculateRating(DWORD tNow);
	void		ClearHashes();
	BOOL		HashesFromURN(LPCTSTR pszURN);
	void		ClearRequest();
	BOOL		RequestComplete(const CLibraryFile* pFile);
	BOOL		RequestPartial(CDownload* pDownload);
	void		StartSending(int nState);
	void		AllocateBaseFile();
	void		AttachFile(CFragmentedFile* pFile);

	virtual BOOL	IsFileOpen() const;
	virtual BOOL	OpenFile();
	virtual void	CloseFile();
	virtual BOOL	WriteFile(QWORD nOffset, LPCVOID pData, QWORD nLength, QWORD* pnWritten = NULL);
	virtual BOOL	ReadFile(QWORD nOffset, LPVOID pData, QWORD nLength, QWORD* pnRead = NULL);
};

enum UserRating
{
	urNull, urCredit, urSharing, urNew, urNotSharing
	// 1 = Uploaded more to us than we have to them (We 'owe' them upload)
	// 2 = Known Sharer (We've given them more than they have given us)
	// 3 = New user ()
	// 4 = Known user who has not uploaded (May simply not have anything we want)
};


enum UploadState
{
	upsNull, upsReady, upsConnecting,
	upsRequest, upsHeaders, upsQueued,
	upsUploading, upsResponse,
	upsBrowse, upsTigerTree, upsMetadata, upsPreview, upsPreQueue
};
