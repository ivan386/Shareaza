//
// DownloadTransferFTP.h
//
// Copyright (c) Nikolay Raspopov, 2004-2005.
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

#include "Handshakes.h"

class CDownloadTransferFTP : public CDownloadTransfer
{
public:
	CDownloadTransferFTP(CDownloadSource* pSource);
	virtual ~CDownloadTransferFTP() {}
	virtual BOOL	Initiate();
	virtual void	Close(TRISTATE bKeepSource);
	virtual void	Boost();
	virtual DWORD	GetAverageSpeed();
	virtual DWORD	GetMeasuredSpeed();
	virtual BOOL	SubtractRequested(CFileFragment** ppFragments);
	virtual BOOL	OnRun();
	virtual BOOL	OnRead();
	virtual BOOL	OnConnected();
	virtual void	OnDropped(BOOL bError);
	virtual BOOL	OnHeaderLine(CString& strHeader, CString& strValue);

protected:
	// FTP "LIST" helper class
	class CFTPLIST : public CTransfer
	{
	public:
		CFTPLIST() {}
		virtual ~CFTPLIST() {}

		virtual void Close()
		{
//			Handshakes.Remove( this );
			CTransfer::Close();
		}
		virtual BOOL ConnectTo(SOCKADDR_IN* pHost)
		{
			m_sData.Empty ();
			return CConnection::ConnectTo( pHost );
		}

		virtual void AttachTo(CConnection* pConnection)
		{
			m_sData.Empty ();
			CTransfer::AttachTo( pConnection );
		}

		virtual BOOL OnRead()
		{
			if ( CTransfer::OnRead() )
			{
				if ( m_pInput->m_nLength > 0 )
				{
					m_sData.Append( CA2CT( (char*) m_pInput->m_pBuffer ),
						m_pInput->m_nLength );
					m_pInput->Clear();
				}
				return TRUE;
			}
			Close();
			return FALSE;
		}

		virtual QWORD ExtractFileSize() const
		{
			TRACE( _T("Extracting file size from:\n%ls\n"), m_sData );
			CString in( m_sData ), out;
			for( int n = 0; Split( in, _T(' '), out ); ++n )
			{
				for( int i = 0; i < out.GetLength(); ++i )
					if ( !isdigit( out [i] ) )
						break;
				if( i == out.GetLength() && out [0] != _T('0') && n != 2 )
				{
					QWORD size = _tstoi64( out );
					TRACE( _T("File size: %ld bytes\n"), size );
					return size;
				}
			}
			TRACE( _T("Unknown file size.\n") );
			return SIZE_UNKNOWN;
		}

	protected:
		CString m_sData;	// Recieved file listing data

		inline bool Split(CString& in, TCHAR token, CString& out) const
		{
			in = in.Trim (_T(" \t\r\n"));
			if (!in.GetLength ()) {
				out.Empty ();
				return false;
			}
			int p = in.ReverseFind (token);
			if (p != -1) {
				out = in.Mid (p + 1);
				in = in.Mid (0, p);
			} else {
				out = in;
				in.Empty ();
			}
			return true;
		}
	};

	// FTP "RETR" helper class
	class CFTPRETR : public CTransfer
	{
	public:
		CFTPRETR() : m_pOwner( NULL ), m_tContent( 0 ), m_nTotal( 0 ) {}
		virtual ~CFTPRETR() {}
	
		inline void SetOwner(CDownloadTransferFTP* pOwner)
		{
			m_pOwner = pOwner;
		}

		virtual void Close()
		{
//			Handshakes.Remove( this );
			CTransfer::Close();
		}

		virtual BOOL ConnectTo(SOCKADDR_IN* pHost)
		{
			m_tContent = GetTickCount();
			m_nTotal = 0;
			return CConnection::ConnectTo( pHost );
		}

		virtual void AttachTo(CConnection* pConnection)
		{
			m_tContent = GetTickCount();
			m_nTotal = 0;
			CTransfer::AttachTo( pConnection );
		}

		virtual BOOL OnRead()
		{
			if ( CTransfer::OnRead() )
			{
				if ( m_pOwner && m_pInput->m_nLength > 0 )
				{
					DWORD nLength = (DWORD) min( (QWORD) m_pInput->m_nLength,
						m_pOwner->m_nLength - m_pOwner->m_nPosition );
					m_pOwner->m_pDownload->SubmitData(
						m_pOwner->m_nOffset + m_pOwner->m_nPosition,
						m_pInput->m_pBuffer, nLength );		
					m_pOwner->m_nPosition += nLength;
					m_pOwner->m_nDownloaded += nLength;
					// Measuring speed
					DWORD nCurrent = GetTickCount();
					if (nCurrent - m_tContent != 0) {
						m_pOwner->m_pSource->m_nSpeed =
							(DWORD) ( ( ( m_pInput->m_nLength + m_nTotal ) /
							( nCurrent - m_tContent ) ) * 1000 );
						m_tContent = nCurrent;	
						m_nTotal = 0;
					} else
						m_nTotal += m_pInput->m_nLength;
					m_pInput->Clear();
					if ( m_pOwner->m_nPosition >= m_pOwner->m_nLength )
					{
						m_pOwner->m_pSource->AddFragment( m_pOwner->m_nOffset, m_pOwner->m_nLength );
						Close();
					}
				}
				return TRUE;
			}
			Close();
			return FALSE;
		}

	protected:
		CDownloadTransferFTP* m_pOwner;	// Owner object
		DWORD m_tContent;				// Last Recieve time
		QWORD m_nTotal;					// Recieved bytes from m_tContent time
	};

	enum FTP_STATES {
		ftpConnecting,
		ftpUSER, ftpPASS,
		ftpSIZE_TYPE, ftpSIZE,
		ftpLIST_TYPE, ftpLIST_PASVPORT, ftpLIST,
		ftpDownloading,
		ftpRETR_TYPE, ftpRETR_PASVPORT, ftpRETR_REST, ftpRETR,
		ftpABOR
	};

	FTP_STATES		m_FtpState;		// FTP Control chanell state
	DWORD			m_tRequest;		// Last request time
	CFTPLIST		m_LIST;			// FTP "LIST" helper object
	CFTPRETR		m_RETR;			// FTP "RETR" helper object
	BOOL			m_bPassive;		// Passive or Active FTP mode

	BOOL			StartNextFragment();
	BOOL			SendCommand (LPCTSTR args = NULL);
};
