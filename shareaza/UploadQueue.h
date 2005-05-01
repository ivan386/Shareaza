//
// UploadQueue.h
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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

#if !defined(AFX_UPLOADQUEUE_H__2C8AAF1A_A452_4275_B5D5_06ED2F46BAA8__INCLUDED_)
#define AFX_UPLOADQUEUE_H__2C8AAF1A_A452_4275_B5D5_06ED2F46BAA8__INCLUDED_

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
	CPtrList	m_pActive;
	CPtrArray	m_pQueued;
public:
	int			m_nIndex;
	CString		m_sName;
	BOOL		m_bEnable;
	DWORD		m_nProtocols;
	QWORD		m_nMinSize;
	QWORD		m_nMaxSize;
	BOOL		m_bPartial;
	CString		m_sShareTag;
	CString		m_sNameMatch;
public:
	int			m_nCapacity;
	int			m_nMinTransfers;
	int			m_nMaxTransfers;
	int			m_nBandwidthPoints;
	BOOL		m_bRotate;
	DWORD		m_nRotateTime;
	DWORD		m_nRotateChunk;
	BOOL		m_bRewardUploaders;
public:
	BOOL		m_bExpanded;
	BOOL		m_bSelected;
	DWORD		m_nMeasured;

// Operations
public:
	CString		GetCriteriaString() const;
	void		Serialize(CArchive& ar, int nVersion);
public:
	BOOL		CanAccept(PROTOCOLID nProtocol, LPCTSTR pszName, QWORD nSize, BOOL bPartial, LPCTSTR pszShareTags = NULL) const;
	BOOL		Enqueue(CUploadTransfer* pUpload, BOOL bForce = FALSE, BOOL bStart = FALSE);
	BOOL		Dequeue(CUploadTransfer* pUpload);
	int			GetPosition(CUploadTransfer* pUpload, BOOL bStart);
	BOOL		StealPosition(CUploadTransfer* pTarget, CUploadTransfer* pSource);
	BOOL		Start(CUploadTransfer* pUpload, BOOL bPeek = FALSE);
public:
	int			GetBandwidthPoints(int nTransfers = -1) const;
	DWORD		GetBandwidthLimit(int nTransfers = -1) const;
	DWORD		GetAvailableBandwidth() const;
	DWORD		GetPredictedBandwidth() const;
	void		SpreadBandwidth();
	void		RescaleBandwidth();
protected:
	void		StartImpl(CUploadTransfer* pUpload);

// Utilities
public:
	inline int GetTransferCount(BOOL bMax = FALSE) const
	{
		if ( ! bMax ) return m_pActive.GetCount();
		int nActive = m_pActive.GetCount();
		return max( nActive, m_nMinTransfers );
	}

	inline int GetQueueCapacity() const
	{
		return m_nCapacity;
	}

	inline int GetQueuedCount() const
	{
		return m_pQueued.GetSize();
	}

	inline int GetQueueRemaining() const
	{
		return GetQueueCapacity() - GetQueuedCount();
	}

	inline BOOL IsActive(CUploadTransfer* pUpload) const
	{
		ASSERT( pUpload != NULL );
		return ( m_pActive.Find( pUpload ) != NULL );
	}

	inline DWORD GetMeasuredSpeed() const
	{
		return m_nMeasured;
	}

	friend class CUploadsCtrl;
	friend class CUploadsWnd;
};

#endif // !defined(AFX_UPLOADQUEUE_H__2C8AAF1A_A452_4275_B5D5_06ED2F46BAA8__INCLUDED_)
