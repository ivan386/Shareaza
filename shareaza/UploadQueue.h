//
// UploadQueue.h
//
// Copyright (c) Shareaza Development Team, 2002-2013.
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

class CUploadTransfer;


class CUploadQueue
{
// Construction
public:
	CUploadQueue();
	virtual ~CUploadQueue();

// Attributes
protected:
	CList< CUploadTransfer* >	m_pActive;
	CArray< CUploadTransfer* >	m_pQueued;
public:
	int			m_nIndex;
	CString		m_sName;
	BOOL		m_bEnable;
	DWORD		m_nProtocols;
	QWORD		m_nMinSize;
	QWORD		m_nMaxSize;
	DWORD		m_nFileStateFlag;
	CString		m_sShareTag;
	CString		m_sNameMatch;
public:
	DWORD		m_nCapacity;
	DWORD		m_nMinTransfers;
	DWORD		m_nMaxTransfers;
	DWORD		m_nBandwidthPoints;
	BOOL		m_bRotate;
	DWORD		m_nRotateTime;
	DWORD		m_nRotateChunk;
	BOOL		m_bRewardUploaders;
public:
	BOOL		m_bExpanded;
	BOOL		m_bSelected;
	DWORD		m_nMeasured;

	enum		{ ulqNull = 0, ulqPartial = 1, ulqLibrary = 2, ulqBoth = 3 };
// Operations
public:
	CString		GetCriteriaString() const;
	void		Serialize(CArchive& ar, int nVersion);
public:
	BOOL		CanAccept(PROTOCOLID nProtocol, LPCTSTR pszName, QWORD nSize, DWORD nFileState, LPCTSTR pszShareTags = NULL) const;
	BOOL		Enqueue(CUploadTransfer* pUpload, BOOL bForce = FALSE, BOOL bStart = FALSE);
	BOOL		Dequeue(CUploadTransfer* pUpload);
	int			GetPosition(CUploadTransfer* pUpload, BOOL bStart);
	BOOL		StealPosition(CUploadTransfer* pTarget, CUploadTransfer* pSource);
	BOOL		Start(CUploadTransfer* pUpload, BOOL bPeek = FALSE);
public:
	DWORD		GetBandwidthPoints(DWORD nTransfers = (DWORD)-1) const;
	DWORD		GetBandwidthLimit(DWORD nTransfers = (DWORD)-1) const;
	DWORD		GetAvailableBandwidth() const;
	DWORD		GetPredictedBandwidth() const;
	void		SpreadBandwidth();
	void		RescaleBandwidth();
protected:
	void		StartImpl(CUploadTransfer* pUpload);

// Utilities
public:
	inline DWORD GetTransferCount(BOOL bMax = FALSE) const
	{
		const DWORD nActive = (DWORD)m_pActive.GetCount();
		return bMax ? max( nActive, m_nMinTransfers ) : nActive;
	}

	inline DWORD GetActiveCount() const
	{
		return (DWORD)m_pActive.GetCount();
	}

	inline POSITION GetActiveIterator() const
	{
		return m_pActive.GetHeadPosition();
	}

	inline CUploadTransfer* GetNextActive(POSITION& nPos) const
	{
		return m_pActive.GetNext( nPos );
	}

	inline CUploadTransfer* GetQueuedAt(INT_PTR nPos) const
	{
		return m_pQueued.GetAt( nPos );
	}

	inline DWORD GetQueuedCount() const
	{
		return (DWORD)m_pQueued.GetCount();
	}

	inline bool IsFull() const
	{
		return m_nCapacity <= GetQueuedCount();
	}

	inline bool IsActive(const CUploadTransfer* pUpload) const
	{
		return m_pActive.Find( (CUploadTransfer*)pUpload ) != NULL;
	}

	inline DWORD GetMeasuredSpeed() const
	{
		return m_nMeasured;
	}
};
