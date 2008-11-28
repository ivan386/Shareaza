//
// BTInfo.h
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

#pragma once

#include "Buffer.h"
#include "ShareazaFile.h"

class CBuffer;
class CBENode;


class CBTInfo : public CShareazaFile
{
// Construction
public:
	CBTInfo();
	CBTInfo(const CBTInfo& oSource);
	virtual ~CBTInfo();

	// Tracker status/types
	enum
	{
		tNull,			// No tracker
		tSingle,		// User locked tracker or single tracker
		tMultiFinding,	// Multi-tracker searching
		tMultiFound		// Multi-tracker that's found a tracker
	};

	// When to initiate new torrent transfers
	enum
	{
		dtAlways,			// Whenever wanted
		dtWhenRatio,		// When download ratio > 100%
		dtWhenRequested,	// Only when another client requests
		dtNever				// Never
	};

// Subclass
public:
	class CBTFile : public CShareazaFile
	{
	public:
		// Returns file download progress ( < 0 - unknown or 0..100% )
		float GetProgress() const;

		// Returns file download priority
		inline int GetPriority() const
		{
			return m_nFilePriority;
		}

		// Set file download priority
		inline void	SetPriority(int nFilePriority)
		{
			m_nFilePriority = nFilePriority; 
		}

		// Find file on disk
		CString	FindFile();

	private:
		const CBTInfo*	m_pInfo;			// Parent torrent handler
		int				m_nFilePriority;	// Download priority (NotWanted, Low, Normal or High)
		QWORD			m_nOffset;			// File offset inside torrent (cached)

		CBTFile(const CBTInfo* pInfo, const CBTFile* pFile = NULL);
		void Serialize(CArchive& ar, int nVersion);

		friend class CBTInfo;
	};
	enum { prNotWanted, prLow, prNormal, prHigh };

// Subclass
public:
	class CBTTracker
	{
	public:
		CBTTracker();
		CBTTracker(const CBTTracker& oSource);	

		CString		m_sAddress;
		DWORD		m_tLastAccess;
		DWORD		m_tLastSuccess;
		DWORD		m_tNextTry;
		DWORD		m_nFailures;
		INT			m_nTier;
		INT			m_nType;

	private:
		void Serialize(CArchive& ar, int nVersion);

		friend class CBTInfo;
	};
	
// Attributes
public:
	CList< CString > m_sURLs;			// Add sources from torrents - DWK
	QWORD		m_nTotalSize;
	DWORD		m_nBlockSize;
	DWORD		m_nBlockCount;
    Hashes::BtPureHash* m_pBlockBTH;
	QWORD		m_nTotalUpload;			// Total amount uploaded
	QWORD		m_nTotalDownload;		// Total amount downloaded
	CList< CBTFile* > m_pFiles;			// List of files
	UINT		m_nEncoding;
	CString		m_sComment;
	DWORD		m_tCreationDate;
	CString		m_sCreatedBy;
	BOOL		m_bPrivate;
	int			m_nStartDownloads;		// When do we start downloads for this torrent

private:
	CArray< CBTTracker > m_oTrackers;	// Tracker list
	int			m_nTrackerIndex;		// The tracker we are currently using
	int			m_nTrackerMode;			// The current tracker situation
	bool		m_bEncodingError;		// Torrent has encoding errors
	CSHA		m_pTestSHA1;
	DWORD		m_nTestByte;
	CBuffer		m_pSource;
	
	BOOL		CheckFiles();
	int			AddTracker(const CBTTracker& oTracker);

// Operations
public:
	void		Clear();
	CBTInfo&	Copy(const CBTInfo& oSource);
	void		Serialize(CArchive& ar);

	BOOL		LoadTorrentFile(LPCTSTR pszFile);
	BOOL		LoadTorrentBuffer(CBuffer* pBuffer);
	BOOL		LoadTorrentTree(CBENode* pRoot);
	BOOL		SaveTorrentFile(LPCTSTR pszPath) const;

	void		BeginBlockTest();
	void		AddToTest(LPCVOID pInput, DWORD nLength);
	BOOL		FinishBlockTest(DWORD nBlock);

	void		SetTrackerAccess(DWORD tNow);
	void		SetTrackerSucceeded(DWORD tNow);
	void		SetTrackerRetry(DWORD tTime);
	void		SetTrackerNext(DWORD tTime = 0);
	DWORD		GetTrackerFailures() const;
	CString		GetTrackerAddress(int nTrackerIndex = -1) const;
	TRISTATE	GetTrackerStatus(int nTrackerIndex = -1) const;
	int			GetTrackerTier(int nTrackerIndex = -1) const;
	DWORD		GetTrackerNextTry() const;
	void		OnTrackerFailure();

	// Count of files
	inline INT_PTR GetCount() const
	{
		return m_pFiles.GetCount();
	}

	inline bool IsAvailable() const
	{
		return m_oBTH;
	}

	inline bool HasEncodingError() const
	{
		return m_bEncodingError;
	}

	inline bool IsMultiTracker() const
	{
		return ( m_nTrackerMode > tSingle ) && m_oTrackers.GetCount() > 1;
	}

	inline bool HasTracker() const
	{
		return ( m_nTrackerMode != tNull ) && ! m_oTrackers.IsEmpty();
	}

	inline int GetTrackerIndex() const
	{
		return m_nTrackerIndex;
	}

	void SetTracker(const CString& sTracker);

	inline int GetTrackerMode() const
	{
		return m_nTrackerMode;
	}

	void SetTrackerMode(int nTrackerMode);

	inline int GetTrackerCount() const
	{
		return (int)m_oTrackers.GetCount();
	}
};
