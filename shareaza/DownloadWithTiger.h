//
// DownloadWithTiger.h
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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

	mutable CCriticalSection	m_pTigerSection;

// Operations
public:
	BOOL		GetNextVerifyRange(QWORD& nOffset, QWORD& nLength, BOOL& bSuccess, int nHash = HASH_NULL) const;
	BOOL		NeedTigerTree() const;
	BOOL		SetTigerTree(BYTE* pTiger, DWORD nTiger);
	CTigerTree*	GetTigerTree();
	BOOL		NeedHashset() const;
	BOOL		SetHashset(BYTE* pSource, DWORD nSource);
	CED2K*		GetHashset();
	void		ResetVerification();
	void		ClearVerification();
	void		RunValidation();
	QWORD		GetVerifyLength(int nHash = HASH_NULL) const;

	virtual CString	GetAvailableRanges() const;

protected:
	BOOL		IsFullyVerified() const;
//	BOOL		ValidationCanFinish() const;
	DWORD		GetValidationCookie() const;
	BOOL		FindNewValidationBlock(int nHash);
	void		ContinueValidation();
	void		FinishValidation();
	void		SubtractHelper(Fragments::List& ppCorrupted, BYTE* pBlock, QWORD nBlock, QWORD nSize);

	virtual void	Serialize(CArchive& ar, int nVersion);

	friend class CEDClient; // AddSourceED2K && m_nHashsetBlock && m_pHashsetBlock
	friend class CDownloadTipCtrl;
};
