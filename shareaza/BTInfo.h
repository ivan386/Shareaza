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

		// Get internal path (including file name) of file inside torrent
		inline CString GetPath() const
		{
			return m_pInfo->m_sPath;
		}

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
		CString		m_sAddress;
		DWORD		m_tLastAccess;
		DWORD		m_tLastSuccess;
		DWORD		m_tNextTry;
		DWORD		m_nFailures;
		INT			m_nTier;
		INT			m_nType;

	private:
		CBTTracker();
		CBTTracker(const CBTTracker& oSource);
		void Serialize(CArchive& ar, int nVersion);

		friend class CBTInfo;
	};
	
// Attributes
public:
	CList< CString >	m_sURLs;	// Add sources from torrents - DWK
	QWORD		m_nTotalSize;
	DWORD		m_nBlockSize;
	DWORD		m_nBlockCount;
    Hashes::BtPureHash* m_pBlockBTH;
	QWORD		m_nTotalUpload;					// Total amount uploaded
	QWORD		m_nTotalDownload;				// Total amount downloaded
	CList< CBTFile* > m_pFiles;					// List of files
	CString		m_sTracker;						// Address of tracker we are using
	CBTTracker*	m_pAnnounceTracker;				// Tracker in the announce key
	CArray< CBTTracker* > m_pTrackerList;		// Multi-tracker list
	int			m_nTrackerIndex;				// The tracker we are currently using
	int			m_nTrackerMode;					// The current tracker situation
	UINT		m_nEncoding;
	CString		m_sComment;
	DWORD		m_tCreationDate;
	CString		m_sCreatedBy;
	BOOL		m_bPrivate;
	int			m_nStartDownloads;				// When do we start downloads for this torrent

private:
	bool		m_bEncodingError;
	CSHA		m_pTestSHA1;
	DWORD		m_nTestByte;
	CBuffer		m_pSource;
	
// Operations
public:
	void		Clear();
	CBTInfo&	Copy(const CBTInfo& oSource);
	void		Serialize(CArchive& ar);
public:
	BOOL		LoadTorrentFile(LPCTSTR pszFile);
	BOOL		LoadTorrentBuffer(CBuffer* pBuffer);
	BOOL		LoadTorrentTree(CBENode* pRoot);
	BOOL		SaveTorrentFile(LPCTSTR pszPath) const;
public:
	void		BeginBlockTest();
	void		AddToTest(LPCVOID pInput, DWORD nLength);
	BOOL		FinishBlockTest(DWORD nBlock);
public:
	void		SetTrackerAccess(DWORD tNow);
	void		SetTrackerSucceeded(DWORD tNow);
	void		SetTrackerRetry(DWORD tTime);
	void		SetTrackerNext(DWORD tTime = 0);
	DWORD		GetTrackerFailures() const;
protected:
	BOOL		CheckFiles();

// Inlines
public:
	// Count of files
	INT_PTR	GetCount() const { return m_pFiles.GetCount(); }

	bool	IsAvailable() const { return m_oBTH; }

	bool	HasEncodingError() const { return m_bEncodingError; }

	// Check if a string is a valid path/file name.
	bool	IsValid(LPCTSTR psz) const
	{
		if ( _tcsclen( psz ) == 0 ) return FALSE;
		if ( _tcschr( psz, '?' ) != NULL ) return FALSE;
		if ( _tcsicmp( psz , _T("#ERROR#") ) == 0 ) return FALSE;
		
		return TRUE;
	}

	bool	IsMultiTracker() const { return m_pTrackerList.GetCount() > 0; }
};

// Tracker status/types
enum { tNull, tCustom, tSingle, tMultiFinding, tMultiFound };
// No tracker, User set tracker, normal torrent, multitracker searching, multitracker that's found a tracker

// When to initiate new torrent transfers
enum { dtAlways, dtWhenRatio, dtWhenRequested, dtNever };
// Whenever wanted, when download ratio > 100%, only when another client requests, never
