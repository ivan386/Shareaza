//
// BTInfo.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#if !defined(AFX_BTINFO_H__AA44CA36_464F_4FB8_9D79_884D8092ADA0__INCLUDED_)
#define AFX_BTINFO_H__AA44CA36_464F_4FB8_9D79_884D8092ADA0__INCLUDED_

#pragma once

#include "SHA.h"
#include "ED2K.h"
#include "MD5.h"
#include "Buffer.h"
#include "TigerTree.h"
#include "ShareazaFile.h"

class CBuffer;
class CBENode;


class CBTInfo : public CShareazaFile
{
// Construction
public:
	CBTInfo();
	virtual ~CBTInfo();
	
// Subclass
public:
	class CBTFile : public CShareazaFile
	{
	public:
		CBTFile();
		void		Copy(CBTFile* pSource);
		void		Serialize(CArchive& ar, int nVersion);
		int			nFilePriority;
	};
	enum { prNotWanted, prLow, prNormal, prHigh };

// Subclass
public:
	class CBTTracker
	{
	public:
		CBTTracker();
		virtual ~CBTTracker();
		void		Copy(CBTTracker* pSource);
		void		Serialize(CArchive& ar, int nVersion);
	public:
		CString		m_sAddress;
		DWORD		m_tLastAccess;
		DWORD		m_tLastSuccess;
		DWORD		m_tNextTry;
		DWORD		m_nFailures;
		INT			m_nTier;
		INT			m_nType;
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
	int			m_nFiles;						// Number of files
	CBTFile*	m_pFiles;						// List of files
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
	BOOL		m_bEncodingError;
	CSHA		m_pTestSHA1;
	DWORD		m_nTestByte;
	CBuffer		m_pSource;
	
// Operations
public:
	void		Clear();
	void		Copy(CBTInfo* pSource);
	void		Serialize(CArchive& ar);
public:
	BOOL		LoadTorrentFile(LPCTSTR pszFile);
	BOOL		LoadTorrentBuffer(CBuffer* pBuffer);
	BOOL		LoadTorrentTree(CBENode* pRoot);
	BOOL		SaveTorrentFile(LPCTSTR pszPath);
public:
	void		BeginBlockTest();
	void		AddToTest(LPCVOID pInput, DWORD nLength);
	BOOL		FinishBlockTest(DWORD nBlock);
public:
	void		SetTrackerAccess(DWORD tNow);
	void		SetTrackerSucceeded(DWORD tNow);
	void		SetTrackerRetry(DWORD tNow);
	void		SetTrackerNext(DWORD tNow = 0);
	DWORD		GetTrackerFailures() const;
protected:
	BOOL		CheckFiles();

// Inlines
public:
	inline bool IsAvailable() const { return m_oBTH; }
	inline BOOL	HasEncodingError() const { return m_bEncodingError; }

	// Check if a string is a valid path/file name.
	inline BOOL IsValid(LPCTSTR psz) const
	{
		if ( _tcsclen( psz ) == 0 ) return FALSE;
		if ( _tcschr( psz, '?' ) != NULL ) return FALSE;
		if ( _tcsicmp( psz , _T("#ERROR#") ) == 0 ) return FALSE;
		
		return TRUE;
	}

	inline BOOL	IsMultiTracker() const { return (m_pTrackerList.GetCount() > 0 ); }
};

// Tracker status/types
enum { tNull, tCustom, tSingle, tMultiFinding, tMultiFound };
// No tracker, User set tracker, normal torrent, multitracker searching, multitracker that's found a tracker

// When to initiate new torrent transfers
enum { dtAlways, dtWhenRatio, dtWhenRequested, dtNever };
// Whenever wanted, when download ratio > 100%, only when another client requests, never

#endif // !defined(AFX_BTINFO_H__AA44CA36_464F_4FB8_9D79_884D8092ADA0__INCLUDED_)
