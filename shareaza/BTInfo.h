//
// BTInfo.h
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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
	CBTInfo& operator=(const CBTInfo& oSource);
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
		// Find file on disk
		const CString& FindFile();

		// Get existing file on disk for this file
		inline const CString& GetBestPath() const
		{
			return m_sBestPath;
		}

	private:
		const CBTInfo*	m_pInfo;			// Parent torrent handler
		QWORD			m_nOffset;			// File offset inside torrent (cached)
		CString			m_sBestPath;		// Best existing file on disk for this file (cached)

		CBTFile(const CBTInfo* pInfo, const CBTFile* pFile = NULL);
		void Serialize(CArchive& ar, int nVersion);

		friend class CBTInfo;
	};

// Subclass
public:
	class CBTTracker
	{
	public:
		CBTTracker(LPCTSTR szAddress = NULL, INT nTier = 0);
		CBTTracker(const CBTTracker& oSource);
		CBTTracker& operator=(const CBTTracker& oSource);

		bool operator==(const CBTTracker& oSource);

	private:
		CString		m_sAddress;
		DWORD		m_tLastAccess;
		DWORD		m_tLastSuccess;
		DWORD		m_tNextTry;
		DWORD		m_nFailures;
		INT			m_nTier;
		INT			m_nType;

		void Serialize(CArchive& ar, int nVersion);

		friend class CBTInfo;
	};

// Attributes
public:
	CStringList m_sURLs;				// Add sources from torrents - DWK
	CStringList	m_oNodes;				// DHT nodes list
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
	DWORD		m_nInfoStart;
	DWORD		m_nInfoSize;

	BOOL		CheckFiles();

// Operations
public:
	void		Clear();
	void		Serialize(CArchive& ar);

	BOOL		LoadInfoPiece(BYTE *pPiece, DWORD nPieceSize, DWORD nInfoSize, DWORD nInfoPiece);
	int			NextInfoPiece() const;
	DWORD		GetInfoPiece(DWORD nPiece, BYTE **pInfoPiece) const;
	DWORD		GetInfoSize() const;
	BOOL		CheckInfoData();
	BOOL		LoadTorrentFile(LPCTSTR pszFile);
	BOOL		LoadTorrentBuffer(const CBuffer* pBuffer);
	BOOL		LoadTorrentTree(const CBENode* pRoot);
	BOOL		SaveTorrentFile(const CString& sFolder);

	void		BeginBlockTest();
	void		AddToTest(LPCVOID pInput, DWORD nLength);
	BOOL		FinishBlockTest(DWORD nBlock);

	int			AddTracker(const CBTTracker& oTracker);
	void		RemoveAllTrackers();
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

	inline bool IsAvailableInfo() const
	{
		return IsAvailable() && m_nBlockSize && m_nBlockCount;
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

	void SetNode(const CString& sNode);

	inline int GetTrackerMode() const
	{
		return m_nTrackerMode;
	}

	void SetTrackerMode(int nTrackerMode);

	inline int GetTrackerCount() const
	{
		return (int)m_oTrackers.GetCount();
	}

	// Returns hex-encoded SHA1 string of all tracker URLs for "lt_tex" extension
	CString GetTrackerHash() const;
};
