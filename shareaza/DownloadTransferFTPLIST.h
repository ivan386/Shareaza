//
// DownloadTransferFTPLIST.h
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

class CDownloadTransferFTPLIST : public CDownloadTransfer
{
// Construction
public:
	CDownloadTransferFTPLIST(CDownloadSource* pSource, const SOCKADDR_IN& host,
		CString* data, BOOL* flag) :
		CDownloadTransfer (pSource, PROTOCOL_FTP),
		m_Host (host),
		m_sData (data),
		m_pFlag (flag)
	{
		m_sData->Empty ();
		*m_pFlag = TRUE;
	}
	virtual ~CDownloadTransferFTPLIST ()
	{
		*m_pFlag = FALSE;
	}

// Attributes
protected:
	SOCKADDR_IN		m_Host;
	CString*		m_sData;
	BOOL*			m_pFlag;

// Operations
public:
	virtual BOOL	Initiate()
	{
		TRACE ("0x%08x : CDownloadTransferFTPLIST::Initiate()\n", this);
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
		TRACE ("0x%08x : CDownloadTransferFTPLIST::Close(%d)\n", this, bKeepSource);
		SetState( dtsNull );
		CTransfer::Close();		
		m_pSource = NULL;
		ASSERT( m_pDownload != NULL );
		m_pDownload->RemoveTransfer( this );
		//CDownloadTransfer::Close( bKeepSource );
	}
	virtual void	Boost()
	{
		TRACE ("0x%08x : CDownloadTransferFTPLIST::Boost()\n", this);
		m_mInput.pLimit = m_mOutput.pLimit = NULL;
	}

	virtual DWORD	GetAverageSpeed()
	{
		TRACE ("0x%08x : CDownloadTransferFTPLIST::GetAverageSpeed()\n", this);
		return m_pSource->m_nSpeed;
	}

	virtual BOOL	SubtractRequested(CFileFragment** ppFragments)
	{
		TRACE ("0x%08x : CDownloadTransferFTPLIST::SubtractRequested(0x%08x)\n", this, ppFragments);
		return FALSE;
	}

	virtual BOOL	SubtractRequested(CFileFragmentList& Fragments)
	{
		TRACE ("0x%08x : CDownloadTransferFTPDATA::SubtractRequested()\n", this);
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
			m_sData->Append (CA2CT ((char*) m_pInput->m_pBuffer), m_pInput->m_nLength);
			m_pInput->Clear ();
		}		
		return TRUE;
	}

protected:
	virtual BOOL	OnConnected()
	{
		TRACE ("0x%08x : CDownloadTransferFTPLIST::OnConnected()\n", this);
		theApp.Message( MSG_DEFAULT, IDS_DOWNLOAD_CONNECTED, (LPCTSTR)m_sAddress );
		SetState( dtsRequesting );
		m_tConnected = GetTickCount();
		m_pSource->SetLastSeen();
		CDownloadTransfer::OnWrite();
		return TRUE;
	}

	virtual void	OnDropped(BOOL bError)
	{
		TRACE ("0x%08x : CDownloadTransferFTPLIST::OnDropped(%d)\n", this, bError);
		if ( m_nState == dtsConnecting ) {
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_CONNECT_ERROR, (LPCTSTR)m_sAddress );
			Close( TS_TRUE );
		} else {
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_DROPPED, (LPCTSTR)m_sAddress );
			Close( TS_TRUE);
		}
	}
};
