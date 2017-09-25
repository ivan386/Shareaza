//
// UploadQueue.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Uploads.h"
#include "UploadQueue.h"
#include "UploadQueues.h"
#include "UploadTransferED2K.h"
#include "EDClient.h"
#include "QuerySearch.h"
#include "Neighbours.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CUploadQueue construction

CUploadQueue::CUploadQueue() :
	m_nIndex			( 0 )
,	m_bEnable			( TRUE )

,	m_nProtocols		( 0ul )
,	m_nMinSize			( 0ull )
,	m_nMaxSize			( ~0ull )
,	m_nFileStateFlag	( ulqBoth )

,	m_nCapacity			( 20 )
,	m_nMinTransfers		( 1 )
,	m_nMaxTransfers		( 10 )
,	m_nBandwidthPoints	( 10 )
,	m_bRotate			( FALSE )
,	m_nRotateTime		( 300ul )
,	m_nRotateChunk		( 0ul )
,	m_bRewardUploaders	( FALSE )

,	m_bExpanded			( TRUE )
,	m_bSelected			( FALSE )
,	m_nMeasured			( 0ul )
{
}

CUploadQueue::~CUploadQueue()
{
	for ( POSITION pos = m_pActive.GetHeadPosition() ; pos ; )
	{
		CUploadTransfer* pUpload = m_pActive.GetNext( pos );
		pUpload->m_pQueue = NULL;
	}

	for ( DWORD nPosition = 0 ; nPosition < GetQueuedCount() ; nPosition++ )
	{
		CUploadTransfer* pUpload = m_pQueued.GetAt( nPosition );
		pUpload->m_pQueue = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadQueue string criteria

CString CUploadQueue::GetCriteriaString() const
{
	CString str1;

	if ( m_nProtocols != 0 )
	{
		if ( m_nProtocols & (1<<PROTOCOL_HTTP) )
		{
			if ( str1.GetLength() ) str1 += _T(", ");
			str1 += _T("HTTP");
		}
		if ( m_nProtocols & (1<<PROTOCOL_ED2K) )
		{
			if ( str1.GetLength() ) str1 += _T(", ");
			str1 += _T("ED2K");
		}
	}

	if ( m_nMinSize > 0 )
	{
		if ( str1.GetLength() ) str1 += _T(", ");
		str1 = str1 + _T(">=") + Settings.SmartVolume( m_nMinSize );
	}

	if ( m_nMaxSize < ~0ull )
	{
		if ( str1.GetLength() ) str1 += _T(", ");
		str1 = str1 + _T("<=") + Settings.SmartVolume( m_nMaxSize );
	}

	if ( m_nFileStateFlag == ulqPartial )
	{
		if ( str1.GetLength() ) str1 += _T(", ");
		str1 += LoadString(IDS_UPLOAD_QUEUE_PARTIAL );
	}
	else if ( m_nFileStateFlag == ulqLibrary )
	{
		if ( str1.GetLength() ) str1 += _T(", ");
		str1 += LoadString( IDS_UPLOAD_QUEUE_LIBRARY );
	}

	// ADD: Release states

	return str1;
}

//////////////////////////////////////////////////////////////////////
// CUploadQueue criteria testing

BOOL CUploadQueue::CanAccept(PROTOCOLID nProtocol, LPCTSTR pszName, QWORD nSize, DWORD nFileState, LPCTSTR pszShareTags) const
{
	if ( ! m_bEnable ) return FALSE;

	if ( nSize < m_nMinSize ) return FALSE;
	if ( nSize > m_nMaxSize ) return FALSE;

	if ( m_nProtocols != 0 &&
		 ( m_nProtocols & ( 1 << nProtocol ) ) == 0 ) return FALSE;

	if ( (m_nFileStateFlag & nFileState) == 0 ) return FALSE;

	if ( m_sShareTag.GetLength() > 0 )
	{
		if ( pszShareTags == NULL ) return FALSE;
		if ( _tcsistr( pszShareTags, m_sShareTag ) == NULL ) return FALSE;
	}

	if ( m_sNameMatch.GetLength() > 0 )
	{
		if ( pszName == NULL ) return FALSE;
		if ( CQuerySearch::WordMatch( pszName, m_sNameMatch ) == FALSE ) return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadQueue enqueue

BOOL CUploadQueue::Enqueue(CUploadTransfer* pUpload, BOOL bForce, BOOL bStart)
{
	ASSERT( pUpload != NULL );
	ASSERT( pUpload->m_pQueue == NULL );

	if ( ! bForce && ! bStart )	//If this upload isn't forced, check to see if it's valid to queue
	{
		if ( m_bRewardUploaders && ( pUpload->m_nUserRating > urSharing  ) )
		{
			// If reward is on, a non-sharer might not queue.
			// Check if the # already queued plus # reserved by the reward
			// percentage is greater than the queue would be able to hold.
			DWORD nReserved = m_nCapacity * Settings.Uploads.RewardQueuePercentage / 100ul;
			if ( GetQueuedCount() + nReserved >= m_nCapacity )
			{
				return FALSE;
			}
		}
		else
		{
			// If reward is off, or user is known to share, just check if the queue is full
			if ( IsFull() )
				return FALSE;
		}
	}

	m_pQueued.Add( pUpload );
	pUpload->m_pQueue = this;

	if ( bStart )
	{
		StartImpl( pUpload );

		if ( GetTransferCount() <= m_nMinTransfers )
			SpreadBandwidth();
		else
			pUpload->m_nBandwidth = Settings.Bandwidth.Uploads / max( 1ul, m_nMinTransfers );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CUploadQueue dequeue

BOOL CUploadQueue::Dequeue(CUploadTransfer* pUpload)
{
	ASSERT( pUpload != NULL );
	ASSERT( pUpload->m_pQueue == this );

	if ( POSITION pos = m_pActive.Find( pUpload ) )
	{
		pUpload->m_pQueue = NULL;
		m_pActive.RemoveAt( pos );
		RescaleBandwidth();
		return TRUE;
	}

	for ( DWORD nPosition = 0 ; nPosition < GetQueuedCount() ; nPosition++ )
	{
		if ( m_pQueued.GetAt( nPosition ) == pUpload )
		{
			pUpload->m_pQueue = NULL;
			m_pQueued.RemoveAt( nPosition );
			return TRUE;
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CUploadQueue position lookup (and optional start)

int CUploadQueue::GetPosition(CUploadTransfer* pUpload, BOOL bStart)
{
	ASSERT( pUpload != NULL );
	ASSERT( pUpload->m_pQueue == this );

	if ( m_pActive.Find( pUpload ) ) return 0;

	for ( DWORD nPosition = 0 ; nPosition < GetQueuedCount() ; nPosition++ )
	{
		if ( m_pQueued.GetAt( nPosition ) == pUpload )
		{
			if ( nPosition == 0 && Start( pUpload, ! bStart ) ) return 0;
			return nPosition + 1;
		}
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////
// CUploadQueue position stealing

BOOL CUploadQueue::StealPosition(CUploadTransfer* pTarget, CUploadTransfer* pSource)
{
	ASSERT( pTarget != NULL );
	ASSERT( pSource != NULL );
	ASSERT( pTarget->m_pQueue == NULL );
	ASSERT( pSource->m_pQueue == this );

	if ( POSITION pos = m_pActive.Find( pSource ) )
	{
		m_pActive.SetAt( pos, pTarget );
		pTarget->m_pQueue = this;
		pSource->m_pQueue = NULL;
		pTarget->m_nBandwidth = pSource->m_nBandwidth;
		return TRUE;
	}

	for ( DWORD nPosition = 0 ; nPosition < GetQueuedCount() ; nPosition++ )
	{
		if ( m_pQueued.GetAt( nPosition ) == pSource )
		{
			m_pQueued.SetAt( nPosition, pTarget );
			pTarget->m_pQueue = this;
			pSource->m_pQueue = NULL;
			return TRUE;
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CUploadQueue move transfers from queued to active

BOOL CUploadQueue::Start(CUploadTransfer* pUpload, BOOL bPeek)
{
	ASSERT( pUpload != NULL );
	ASSERT( pUpload->m_pQueue == this );
	ASSERT( m_pActive.Find( pUpload ) == NULL );

	DWORD nTransfers = GetTransferCount();
	if ( nTransfers >= m_nMaxTransfers ) return FALSE;

	if ( nTransfers < m_nMinTransfers )
	{
		if ( bPeek ) return TRUE;
		StartImpl( pUpload );
		SpreadBandwidth();
		theApp.Message( MSG_DEBUG, _T("Starting upload to %s because the minimum has not been reached."),
			(LPCTSTR)pUpload->m_sAddress );
		return TRUE;
	}

	if ( DWORD nAvailable = GetAvailableBandwidth() )
	{
		if ( bPeek ) return TRUE;
		StartImpl( pUpload );
		pUpload->SetSpeedLimit( nAvailable );
		theApp.Message( MSG_DEBUG, _T("Starting upload to %s because there is %s available."),
			(LPCTSTR)pUpload->m_sAddress, (LPCTSTR)Settings.SmartSpeed( nAvailable ) );
		return TRUE;
	}

	return FALSE;
}

void CUploadQueue::StartImpl(CUploadTransfer* pUpload)
{
	ASSERT( pUpload != NULL );
	ASSERT( pUpload->m_pQueue == this );
	Dequeue( pUpload );
	m_pActive.AddTail( pUpload );
	pUpload->m_pQueue = this;
	if ( pUpload->m_nProtocol == PROTOCOL_ED2K )
	{
		CUploadTransferED2K * pEdUpload = static_cast<CUploadTransferED2K *>(pUpload);
		pEdUpload->m_pClient->Connect();
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadQueue bandwidth limiting

DWORD CUploadQueue::GetBandwidthPoints(DWORD nTransfers) const
{
	if ( nTransfers == (DWORD)-1 ) nTransfers = GetTransferCount();

	if ( nTransfers == 0 ) return 0;
	if ( nTransfers >= m_nMinTransfers ) return m_nBandwidthPoints;

	return m_nBandwidthPoints * nTransfers / max( 1ul, m_nMinTransfers );
}

DWORD CUploadQueue::GetBandwidthLimit(DWORD nTransfers) const
{
	DWORD nLocalPoints = GetBandwidthPoints( nTransfers );
	if ( nLocalPoints == 0 ) return 0;

	DWORD nTotalPoints = nLocalPoints;

	CQuickLock oLock( UploadQueues.m_pSection );
	for ( POSITION pos = UploadQueues.GetIterator() ; pos ; )
	{
		CUploadQueue* pOther = UploadQueues.GetNext( pos );
		if ( pOther != this ) nTotalPoints += pOther->GetBandwidthPoints();
	}

	return Uploads.GetBandwidthLimit() *
		( nLocalPoints + Settings.Uploads.ThrottleMode ) / max( 1ul, nTotalPoints );
}

DWORD CUploadQueue::GetAvailableBandwidth() const
{
	DWORD nTransfers = GetTransferCount();

	if ( nTransfers < m_nMinTransfers )
	{
		nTransfers ++;
		return GetBandwidthLimit( nTransfers ) / nTransfers;
	}

	DWORD nTotal = GetBandwidthLimit();
	DWORD nUsed = 0;

	for ( POSITION pos = m_pActive.GetHeadPosition() ; pos ; )
	{
		CUploadTransfer* pActive = m_pActive.GetNext( pos );
		// If newly queued host is set as "Next", don't count allocated bandwidth
		// Max speed in such case is zero.
		nUsed += pActive->GetMaxSpeed();
	}

	if ( nUsed >= nTotal ) return 0;

	DWORD nAvailable = nTotal - nUsed;

	if ( nAvailable < Settings.Uploads.FreeBandwidthValue ) return 0;
	if ( nAvailable < ( nTotal * Settings.Uploads.FreeBandwidthFactor / 100 ) ) return 0;

	return nAvailable;
}

DWORD CUploadQueue::GetPredictedBandwidth() const
{
	// This could be more accurate
	return GetBandwidthLimit( m_nMinTransfers ) /
		min( max( m_nMinTransfers, 1ul ), GetTransferCount() + 1 );
}

//////////////////////////////////////////////////////////////////////
// CUploadQueue bandwidth spreading

void CUploadQueue::SpreadBandwidth()
{
	const DWORD nCount = GetTransferCount();
	if ( nCount == 0 )
		// Nothing to do
		return;

	const DWORD nLimit = GetBandwidthLimit() / nCount;
	for ( POSITION pos = m_pActive.GetHeadPosition() ; pos ; )
	{
		CUploadTransfer* pActive = m_pActive.GetNext( pos );
		pActive->SetSpeedLimit( nLimit );
	}
}

void CUploadQueue::RescaleBandwidth()
{
	const DWORD nCount = GetTransferCount();
	if ( nCount == 0 )
		// Nothing to do
		return;

	if ( nCount <= m_nMinTransfers )
	{
		SpreadBandwidth();
		return;
	}

	const DWORD nTotal = GetBandwidthLimit();
	if ( nTotal == 0 )
		return;

	DWORD nAllocated = 0;
	for ( POSITION pos = m_pActive.GetHeadPosition() ; pos ; )
	{
		CUploadTransfer* pActive = m_pActive.GetNext( pos );
		// If newly queued host is set as "Next", don't count allocated bandwidth
		// Max speed in such case is zero.
		nAllocated += pActive->GetMaxSpeed();
	}

	if ( nAllocated == 0 )
	{
		SpreadBandwidth();
		return;
	}

	const double nScale = (double)nTotal / nAllocated;
	for ( POSITION pos = m_pActive.GetHeadPosition() ; pos ; )
	{
		CUploadTransfer* pActive = m_pActive.GetNext( pos );
		pActive->SetSpeedLimit( (DWORD)( nScale * pActive->GetMaxSpeed() ) );
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadQueue serialize

void CUploadQueue::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar << m_sName;
		ar << m_bEnable;

		ar << m_nProtocols;
		ar << m_nMinSize;
		ar << m_nMaxSize;
		ar << m_nFileStateFlag;
		ar << m_sShareTag;
		ar << m_sNameMatch;

		ar << m_nCapacity;
		ar << m_nMinTransfers;
		ar << m_nMaxTransfers;
		ar << m_nBandwidthPoints;
		ar << m_bRotate;
		ar << m_nRotateTime;
		ar << m_nRotateChunk;
		ar << m_bRewardUploaders;

		ar << m_bExpanded;
	}
	else
	{
		ar >> m_sName;
		ar >> m_bEnable;

		ar >> m_nProtocols;

		if ( nVersion >= 3 )
		{
			ar >> m_nMinSize;
			ar >> m_nMaxSize;
		}
		else
		{
			DWORD nInt32;
			ar >> nInt32;
			m_nMinSize = nInt32;
			ar >> nInt32;
			m_nMaxSize = nInt32;
		}

		if ( nVersion >= 6 )
		{
			ar >> m_nFileStateFlag;
		}
		else
		{
			BOOL bPartial;
			ar >> bPartial;
			if ( bPartial )
			{
				m_nFileStateFlag = ulqPartial;
			}
			else
			{
				m_nFileStateFlag = ulqBoth;
			}
		}

		ar >> m_sShareTag;
		ar >> m_sNameMatch;

		ar >> m_nCapacity;
		ar >> m_nMinTransfers;
		ar >> m_nMaxTransfers;
		ar >> m_nBandwidthPoints;
		ar >> m_bRotate;
		ar >> m_nRotateTime;
		ar >> m_nRotateChunk;

		if ( nVersion >= 5 ) ar >> m_bRewardUploaders;

		if ( nVersion >= 4 ) ar >> m_bExpanded;
	}
}
