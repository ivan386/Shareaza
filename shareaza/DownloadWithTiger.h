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

#include "DownloadWithTorrent.h"
#include "TigerTree.h"
#include "ED2K.h"


class CDownloadWithTiger : public CDownloadWithTorrent
{
// Construction
public:
	CDownloadWithTiger();
	virtual ~CDownloadWithTiger();

// Attributes
private:
	CTigerTree	m_pTigerTree;
	BYTE*		m_pTigerBlock;
	DWORD		m_nTigerBlock;
	DWORD		m_nTigerSize;
	DWORD		m_nTigerSuccess;
private:
	CED2K		m_pHashset;
	BYTE*		m_pHashsetBlock;
	DWORD		m_nHashsetBlock;
	DWORD		m_nHashsetSuccess;
private:
	DWORD		m_nVerifyCookie;
	int			m_nVerifyHash;
	DWORD		m_nVerifyBlock;
	QWORD		m_nVerifyOffset;
	QWORD		m_nVerifyLength;
	DWORD		m_tVerifyLast;

// Operations
public:
	DWORD		GetValidationCookie() const;
	QWORD		GetVerifyLength(int nHash = HASH_NULL) const;
	BOOL		GetNextVerifyRange(QWORD& nOffset, QWORD& nLength, BOOL& bSuccess, int nHash = HASH_NULL) const;
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
	BOOL	FindNewValidationBlock(int nHash);
	void	ContinueValidation();
	void	FinishValidation();
    void	SubtractHelper(FF::SimpleFragmentList& ppCorrupted, BYTE* pBlock, QWORD nBlock, QWORD nSize);
public:
	virtual CString	GetAvailableRanges() const;
	virtual void	ResetVerification();
	virtual void	ClearVerification();
	virtual void	Serialize(CArchive& ar, int nVersion);
	
	friend class CEDClient;
};

#endif // !defined(AFX_DOWNLOADWITHTIGER_H__8F105434_164D_4F58_BAA4_8DB2B29CA259__INCLUDED_)
