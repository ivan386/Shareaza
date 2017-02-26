//
// DownloadWithTiger.h
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

#include "DownloadWithTorrent.h"

class CDownloadTransfer;


class CDownloadWithTiger : public CDownloadWithTorrent
{
// Construction
protected:
	CDownloadWithTiger();
	virtual ~CDownloadWithTiger();

// Attributes
private:
	CTigerTree	m_pTigerTree;
	BYTE*		m_pTigerBlock;
	DWORD		m_nTigerBlock;
	DWORD		m_nTigerSize;
	DWORD		m_nTigerSuccess;

	CED2K		m_pHashset;
	BYTE*		m_pHashsetBlock;
	DWORD		m_nHashsetBlock;
	DWORD		m_nHashsetSuccess;

	DWORD		m_nVerifyCookie;
	int			m_nVerifyHash;
	DWORD		m_nVerifyBlock;
	QWORD		m_nVerifyOffset;
	QWORD		m_nVerifyLength;
	DWORD		m_tVerifyLast;

	mutable CMutexEx			m_pTigerSection;

	mutable QWORD				m_nWFLCookie;		// Wanted fragment list cookie
	mutable Fragments::List		m_oWFLCache;		// Wanted fragment list cache

// Operations
public:
	BOOL		GetNextVerifyRange(QWORD& nOffset, QWORD& nLength, BOOL& bSuccess, int nHash = HASH_NULL) const;
	BOOL		NeedTigerTree() const;
	BOOL		SetTigerTree(BYTE* pTiger, DWORD nTiger, BOOL bLevel1 = FALSE);
	const CTigerTree* GetTigerTree() const;
	BOOL		NeedHashset() const;
	BOOL		SetHashset(BYTE* pSource, DWORD nSource);
	const CED2K* GetHashset() const;
	bool		RunMergeFile(LPCTSTR szFilename, BOOL bMergeValidation, const Fragments::List& oMissedGaps, CDownloadTask* pTask);
	void		ResetVerification();
	void		ClearVerification();
	void		RunValidation();
	DWORD		GetVerifyLength(PROTOCOLID nProtocol = PROTOCOL_ANY, int nHash = HASH_NULL) const;

	// Get list of empty fragments we really want to download
	Fragments::List GetWantedFragmentList() const;

	// Check range against fragments list got from GetWantedFragmentList() call
	BOOL		AreRangesUseful(const Fragments::List& oAvailable) const;
	BOOL		IsRangeUseful(QWORD nOffset, QWORD nLength) const;

	// Like IsRangeUseful() but take the amount of useful ranges
	// relative to the amount of garbage and source speed into account
	BOOL		IsRangeUsefulEnough(CDownloadTransfer* pTransfer, QWORD nOffset, QWORD nLength) const;

	// Select a fragment for a transfer
	BOOL		GetFragment(CDownloadTransfer* pTransfer);

protected:
	bool		IsFullyVerified() const;

	virtual void	Serialize(CArchive& ar, int nVersion);

private:
	DWORD		GetValidationCookie() const;
	BOOL		FindNewValidationBlock(int nHash);
	void		ContinueValidation();
	void		FinishValidation();
	void		SubtractHelper(Fragments::List& ppCorrupted, BYTE* pBlock, QWORD nBlock, QWORD nSize);

	// Get list of all fragments which must be downloaded
	// but rounded to nearest smallest hash block (torrent, tiger or ed2k)
	Fragments::List GetHashableFragmentList() const;

	// Get a list of possible download fragments
	Fragments::List GetPossibleFragments(const Fragments::List& oAvailable, Fragments::Fragment& oLargest);

	friend class CEDClient; // AddSourceED2K && m_nHashsetBlock && m_pHashsetBlock
	friend class CDownloadTipCtrl;
};
