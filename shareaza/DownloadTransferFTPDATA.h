//
// CDownloadTransferFTPDATA.h
//
// Copyright (c) Nikolay Raspopov, 2002-2004.
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

#pragma once

#include "DownloadTransfer.h"
#include "FileFragment.h"

class CDownloadTransferFTPDATA : public CDownloadTransfer
{
// Construction
public:
	CDownloadTransferFTPDATA (CDownloadSource* pSource, const SOCKADDR_IN& host,
		QWORD Offset, QWORD Length, BOOL* flag) :
		CDownloadTransfer (pSource, PROTOCOL_FTP),
		m_Host (host),
		m_tContent (0),
		m_pFlag (flag)
	{
		m_nOffset = Offset;
		m_nLength = Length;
		*m_pFlag = TRUE;
	}
	virtual ~CDownloadTransferFTPDATA ()
	{
		*m_pFlag = FALSE;
	}

// Attributes
protected:
	SOCKADDR_IN		m_Host;
	DWORD			m_tContent;
	BOOL*			m_pFlag;

// Operations
public:
	virtual BOOL	Initiate()
	{
		TRACE ("0x%08x : CDownloadTransferFTPDATA::Initiate()\n", this);
		theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_CONNECTING,
			(LPCTSTR)CString( inet_ntoa( m_pSource->m_pAddress ) ), m_pSource->m_nPort,
			(LPCTSTR)m_pDownload->GetDisplayName() );
		m_tConnected	= GetTickCount();
		if ( ConnectTo( &m_Host.sin_addr, m_Host.sin_port ) ) {
			TRACE ("0x%08x : Connected\n", this);
			SetState( dtsConnecting );
			if ( ! m_pDownload->IsBoosted() )
				m_mInput.pLimit = m_mOutput.pLimit = &Downloads.m_nLimitGeneric;
			return TRUE;
		} else {
			TRACE ("0x%08x : Connect failed\n", this);
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_CONNECT_ERROR, (LPCTSTR)m_sAddress );
			Close( TS_TRUE );
			return FALSE;
		}
	}

	virtual void	Close (TRISTATE bKeepSource)
	{
		TRACE ("0x%08x : CDownloadTransferFTPDATA::Close(%d)\n", this, bKeepSource);
		SetState( dtsNull );
		CTransfer::Close();
		m_pSource = NULL;
		ASSERT( m_pDownload != NULL );
		m_pDownload->RemoveTransfer( this );
		//CDownloadTransfer::Close( bKeepSource );
	}
	virtual void	Boost()
	{
		TRACE ("0x%08x : CDownloadTransferFTPDATA::Boost()\n", this);
		m_mInput.pLimit = m_mOutput.pLimit = NULL;
	}

	virtual DWORD	GetAverageSpeed()
	{
		if ( m_nState == dtsDownloading )
		{
			DWORD nTime = ( GetTickCount() - m_tContent ) / 1000;
			if ( nTime > 0 ) m_pSource->m_nSpeed = (DWORD)( m_nPosition / nTime );
		}
		return m_pSource->m_nSpeed;
	}

	virtual BOOL	SubtractRequested(CFileFragmentList& Fragments)
	{
		TRACE ("0x%08x : CDownloadTransferFTPDATA::SubtractRequested()\n", this);
		if ( m_nOffset < SIZE_UNKNOWN && m_nLength < SIZE_UNKNOWN )
		{
			if ( m_nState == dtsRequesting || m_nState == dtsDownloading )
			{
				Fragments.Subtract( m_nOffset, m_nOffset + m_nLength );
				return TRUE;
			}
		}
		return FALSE;
	}

	virtual BOOL	OnRun()
	{
		CDownloadTransfer::OnRun();		
		DWORD tNow = GetTickCount();		
		switch ( m_nState ) {
			case dtsConnecting:
				if ( tNow - m_tConnected > Settings.Connection.TimeoutConnect )
				{
					theApp.Message( MSG_ERROR, IDS_CONNECTION_TIMEOUT_CONNECT, (LPCTSTR)m_sAddress );
					Close( TS_TRUE );
					return FALSE;
				}
				break;

			case dtsDownloading:
				if ( tNow - m_mInput.tLast > Settings.Connection.TimeoutTraffic * 2 )
				{
					theApp.Message( MSG_ERROR, IDS_DOWNLOAD_TRAFFIC_TIMEOUT, (LPCTSTR)m_sAddress );
					Close( TS_TRUE );
					return FALSE;
				}
				break;
		}
		return TRUE;
	}
	virtual BOOL OnRead()
	{
		CDownloadTransfer::OnRead();
		if ( m_pInput->m_nLength > 0 ) {
			//m_pSource->SetValid();			
			DWORD nLength	= (DWORD)min( (QWORD)m_pInput->m_nLength, m_nLength - m_nPosition );
			BOOL bSubmit	= m_pDownload->SubmitData(
				m_nOffset + m_nPosition, m_pInput->m_pBuffer, nLength );			
			m_pInput->Clear();	// Clear the buffer, we don't want any crap
			m_nPosition += nLength;
			m_nDownloaded += nLength;
		}		
		if ( m_nPosition >= m_nLength ) {
			m_pSource->AddFragment( m_nOffset, m_nLength );

			// No next fragment
			Close( TS_TRUE );
			return FALSE;
		}
		return TRUE;
	}

protected:
	virtual BOOL	OnConnected()
	{
		TRACE ("0x%08x : CDownloadTransferFTPDATA::OnConnected()\n", this);
		theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_CONNECTED, (LPCTSTR)m_sAddress );
		SetState( dtsDownloading );
		m_tConnected = GetTickCount();
		m_pSource->SetLastSeen();
		m_nPosition = 0;
		m_tContent = m_mInput.tLast = GetTickCount();
		CDownloadTransfer::OnWrite();
		return TRUE;
	}

	virtual void	OnDropped(BOOL bError)
	{
		TRACE ("0x%08x : CDownloadTransferFTPDATA::OnDropped(%d)\n", this, bError);
		if ( m_nState == dtsConnecting ) {
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_CONNECT_ERROR, (LPCTSTR)m_sAddress );
			Close( TS_TRUE );
		} else {
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_DROPPED, (LPCTSTR)m_sAddress );
			Close( TS_TRUE);
		}
	}
};
