//
// DownloadWithTiger.h
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

#if !defined(AFX_DOWNLOADWITHTIGER_H__8F105434_164D_4F58_BAA4_8DB2B29CA259__INCLUDED_)
#define AFX_DOWNLOADWITHTIGER_H__8F105434_164D_4F58_BAA4_8DB2B29CA259__INCLUDED_

#pragma once

#include "Hashes.h"
#include "FileFragment.h"
#include "DownloadWithTorrent.h"
#include "TigerTree.h"
#include "ED2K.h"
#include "MD5.h"
#include "SHA.h"


class CDownloadWithTiger : public CDownloadWithTorrent
{
// Construction
public:
	CDownloadWithTiger();
	virtual ~CDownloadWithTiger();

// Attributes
private:
	CTigerTree	m_oTigerTree;
	DWORD		m_nTigerBlock;
	DWORD		m_nTigerSize;

	DWORD	    *m_pTigerVerificationQueue;
	BYTE		*m_pTigerVerificationCandidates;
	DWORD		m_nTigerVerificationStart;
	DWORD		m_nTigerVerificationEnd;
private:
	CED2K		m_oHashset;
	DWORD		m_nHashsetBlock;

	DWORD	   *m_pHashsetVerificationQueue;
	BYTE	   *m_pHashsetVerificationCandidates;
	DWORD		m_nHashsetVerificationStart;
	DWORD		m_nHashsetVerificationEnd;
private:
	DWORD		m_nVerifyCookie;
	HASHID		m_nVerifyHash;
	DWORD		m_nVerifyBlock;
	QWORD		m_nVerifyOffset;
	QWORD		m_nVerifyLength;
	DWORD		m_tVerifyLast;

	BOOL		m_bVerifySpeculative;

	CMD5		m_oTestMD5;
	CSHA1		m_oTestSHA1;

// Operations
public:
	DWORD		GetValidationCookie() const;
	QWORD		GetVerifyLength(HASHID nHash = HASH_NULL) const;
	BOOL		GetNextVerifyRange(QWORD& nOffset, QWORD& nLength, DWORD& nVerifyState) const;
	BOOL		IsFullyVerified();
public:
	BOOL		NeedTigerTree() const;
	BOOL		SetTigerTree(BYTE* pTiger, DWORD nTiger);
	CTigerTree*	GetTigerTree();
	BOOL		NeedHashset() const;
	BOOL		SetHashset(BYTE* pSource, DWORD nSource);
	CED2K*		GetHashset();
protected:
	BOOL	ValidationCanFinish() const;
	void	RunValidation(BOOL bSeeding);
private:
	void		BeginSHA1Test();
	void		BeginMD5Test();
	BOOL		FinishSHA1Test();
	BOOL		FinishMD5Test();
	void	ContinueValidation();
	void	FinishValidation();
	void	AddVerificationBlocks(const QWORD nOffset, const QWORD nNext);
public:
	virtual CString	GetAvailableRanges() const;
	virtual void	ResetVerification();
	void			ClearTiger();
	void			ClearHashset();
	virtual void	Serialize(CArchive& ar, int nVersion);
	
	friend class CEDClient;
	friend class CDownloadWithFile;
};

#endif // !defined(AFX_DOWNLOADWITHTIGER_H__8F105434_164D_4F58_BAA4_8DB2B29CA259__INCLUDED_)
