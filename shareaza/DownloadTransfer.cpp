//
// DownloadTransfer.cpp
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Download.h"
#include "Downloads.h"
#include "DownloadSource.h"
#include "DownloadTransfer.h"
#include "FragmentedFile.h"
#include "Network.h"
#include "Buffer.h"
#include "SHA.h"
#include "ED2K.h"
#include "SourceURL.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadTransfer construction

CDownloadTransfer::CDownloadTransfer(CDownloadSource* pSource, PROTOCOLID nProtocol)
{
	m_nProtocol		= nProtocol;
	m_pDownload		= pSource->m_pDownload;
	m_pDlPrev		= NULL;
	m_pDlNext		= NULL;
	m_pSource		= pSource;
	
	m_nState		= dtsNull;
	
	m_nQueuePos		= 0;
	m_nQueueLen		= 0;
	
	m_nOffset		= SIZE_UNKNOWN;
	m_nLength		= 0;
	m_nPosition		= 0;
	m_nDownloaded	= 0;
	
	m_bWantBackwards = m_bRecvBackwards = FALSE;
	
	m_pDownload->AddTransfer( this );
}

CDownloadTransfer::~CDownloadTransfer()
{
	ASSERT( m_pSource == NULL );
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransfer close

void CDownloadTransfer::Close(TRISTATE bKeepSource)
{
	CTransfer::Close();
	
	if ( m_pSource != NULL )
	{
		switch ( bKeepSource )
		{
		case TS_TRUE:
			m_pSource->OnFailure( TRUE );
			break;
		case TS_UNKNOWN:
			m_pSource->OnFailure( FALSE );
			break;
		case TS_FALSE:
			m_pSource->Remove( FALSE, TRUE );
			break;
		}
		
		m_pSource = NULL;
	}
	
	ASSERT( m_pDownload != NULL );
	m_pDownload->RemoveTransfer( this );
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransfer speed controls

void CDownloadTransfer::Boost()
{
	m_mInput.pLimit = m_mOutput.pLimit = NULL;
}

DWORD CDownloadTransfer::GetAverageSpeed()
{
	return GetMeasuredSpeed();
}

DWORD CDownloadTransfer::GetMeasuredSpeed()
{
	Measure();
	return m_mInput.nMeasure;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransfer state

CString CDownloadTransfer::GetStateText(BOOL bLong)
{
	CString str;
	
	switch ( m_nState )
	{
	case dtsConnecting:
		str = _T("Connecting");
		break;
	case dtsRequesting:
		str = _T("Requesting");
		break;
	case dtsHeaders:
	case dtsFlushing:
		str = _T("Response");
		break;
	case dtsDownloading:
		str = _T("Downloading");
		break;
	case dtsTiger:
		str = _T("TigerTree");
		break;
	case dtsHashset:
		str = _T("Hashset");
		break;
	case dtsMetadata:
		str = _T("Metadata");
		break;
	case dtsBusy:
		str = _T("Busy");
		break;
	case dtsEnqueue:
		str = _T("Enqueue");
		break;
	case dtsQueued:
		if ( ! bLong )
		{
			str.Format( m_nQueueLen ? _T("Q %i of %i") : _T("Q #%i"),
				m_nQueuePos, m_nQueueLen );
		}
		else if ( m_sQueueName.GetLength() )
		{
			str.Format( _T("Queued: %s: %i of %i"),
				(LPCTSTR)m_sQueueName, m_nQueuePos, m_nQueueLen );
		}
		else
		{
			str.Format( _T("Queued: %i of %i"),
				m_nQueuePos, m_nQueueLen );
		}
		break;
	default:
		str = _T("Unknown");
		break;
	}
	
	return str;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransfer run handler

BOOL CDownloadTransfer::OnRun()
{
	return CTransfer::OnRun();
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransfer state management

void CDownloadTransfer::SetState(int nState)
{
	if ( m_pDownload != NULL )
	{
		if ( nState == dtsDownloading && m_nState != dtsDownloading )
		{
			m_pDownload->SortSource( m_pSource, TRUE );
		}
		else if ( nState != dtsDownloading && m_nState == dtsDownloading )
		{
			m_pDownload->SortSource( m_pSource, FALSE );
		}
	}
	
	m_nState = nState;
}

//////////////////////////////////////////////////////////////////////
// CDownloadTransfer fragment size management

void CDownloadTransfer::ChunkifyRequest(QWORD* pnOffset, QWORD* pnLength, QWORD nChunk, BOOL bVerifyLock)
{
	ASSERT( pnOffset != NULL && pnLength != NULL );
	
	if ( m_pSource->m_bCloseConn ) return;
	
	nChunk = min( nChunk, (QWORD)Settings.Downloads.ChunkSize );
	
	if ( bVerifyLock )
	{
		if ( QWORD nVerify = m_pDownload->GetVerifyLength() )
		{
			nVerify = nVerify * 3 / 2;
			nChunk = max( nChunk, nVerify );
		}
		
		if ( Settings.Downloads.ChunkStrap > 0 && m_nDownloaded == 0 )
		{
			nChunk = Settings.Downloads.ChunkStrap;
		}
	}
	
	if ( nChunk == 0 || *pnLength <= nChunk ) return;
	
	if ( m_pDownload->GetVolumeComplete() == 0 || *pnOffset == 0 )
	{
		*pnLength = nChunk;
	}
	else if ( m_bWantBackwards )
	{
		*pnOffset = *pnOffset + *pnLength - nChunk;
		*pnLength = nChunk;
	}
	else
	{
		QWORD nCount = *pnLength / nChunk;
		if ( *pnLength % nChunk ) nCount++;
		nCount = rand() % nCount;
		
		QWORD nStart = *pnOffset + nChunk * nCount;
		*pnLength = min( nChunk, *pnOffset + *pnLength - nStart );
		*pnOffset = nStart;
	}
}
