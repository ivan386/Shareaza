//
// Connection.h
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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

// CConnection holds a socket used to communicate with a remote computer, and is the root of a big inheritance tree
// http://shareazasecurity.be/wiki/index.php?title=Developers.Code.CConnection

#pragma once

#include "Buffer.h"
#include "Packet.h"


// A socket connection to a remote computer on the Internet running peer-to-peer software
class CConnection
{
// Construction
protected:
	CConnection(PROTOCOLID nProtocol = PROTOCOL_ANY);
	virtual ~CConnection();

// Attributes
public:
	SOCKADDR_IN	m_pHost;		// The remote computer's IP address in Windows Sockets format
	CString		m_sAddress;		// The same IP address in a string like "1.2.3.4"
	CString		m_sCountry;		// The two letter country code of this host
	CString		m_sCountryName;	// The full name of the country
	BOOL		m_bInitiated;	// True if we initiated the connection, false if the remote computer connected to us
	BOOL		m_bConnected;	// True when the socket is connected
	DWORD		m_tConnected;	// The tick count when the socket connection was made
	SOCKET		m_hSocket;		// The actual Windows socket for the Internet connection to the remote computer
	CString		m_sUserAgent;	// The name of the program the remote computer is running
	BOOL		m_bClientExtended;// Does the remote computer support extended functions i.e. running under Shareaza or compatible mod? In practice, this means can we use chat, browse, etc...
	CString		m_sLastHeader;	// The handshake header that ReadHeaders most recently read
	PROTOCOLID	m_nProtocol;	// Detected protocol

// Buffers access
protected:
	// Class that looks like CBuffer* but with syncronization
	typedef CLocked< CBuffer*, CCriticalSectionPtr > CLockedBuffer;

	BOOL		m_bAutoDelete;				// Delete this object in Close() method

private:
	CCriticalSectionPtr	m_pInputSection;
	CBuffer*			m_pInput;			// Data from the remote computer, will be compressed if the remote computer is sending compressed data
	CCriticalSectionPtr	m_pOutputSection;
	CBuffer*			m_pOutput;			// Data to send to the remote computer, will be compressed if we are sending the remote computer compressed data
	int			m_nQueuedRun;				// The queued run state of 0, 1, or 2 (do)
	UINT		m_nDelayCloseReason;		// Reason for DelayClose()

	CConnection(const CConnection&);
	CConnection& operator=(const CConnection&);

public:
	inline CLockedBuffer GetInput() const throw()
	{
		return CLockedBuffer( m_pInput, m_pInputSection );
	}

	inline CLockedBuffer GetOutput() const throw()
	{
		return CLockedBuffer( m_pOutput, m_pOutputSection );
	}

// Operations
public:
	// Exchange data with the other computer, measure bandwidth, and work with headers
	virtual BOOL DoRun();	// Communicate with the other computer, reading and writing everything we can right now
	void QueueRun();		// (do) may no longer be in use
	void Measure();			// Measure the bandwidth, setting nMeasure in the bandwidth meters for each direction
	void MeasureIn();		// Measure the incoming bandwidth, setting nMeasure in the bandwidth meter
	void MeasureOut();		// Measure the outgoing bandwidth, setting nMeasure in the bandwidth meter
	BOOL ReadHeaders();		// Read text headers sitting in the input buffer
	BOOL SendMyAddress();	// If we are listening on a port, tell the other computer our IP address and port number
	void UpdateCountry();	// Call whenever the IP address is set
	void SendHTML(UINT nResourceID);
	void LogOutgoing();

	// True if the socket is valid, false if its closed
	inline BOOL IsValid() const throw()
	{
		return ( m_hSocket != INVALID_SOCKET );
	}

	inline bool IsOutputExist() const throw()
	{
		CQuickLock oOutputLock( *m_pOutputSection );

		return ( m_pOutput != NULL );
	}

	inline bool IsInputExist() const throw()
	{
		CQuickLock oOutputLock( *m_pInputSection );

		return ( m_pInput != NULL );
	}

	inline DWORD GetOutputLength() const throw()
	{
		CQuickLock oOutputLock( *m_pOutputSection );

		return m_pOutput->m_nLength;
	}

	inline DWORD GetInputLength() const throw()
	{
		CQuickLock oOutputLock( *m_pInputSection );

		return m_pInput->m_nLength;
	}

	inline void WriteReversed(const void* pData, const size_t nLength) throw()
	{
		CQuickLock oOutputLock( *m_pOutputSection );

		m_pOutput->AddReversed( pData, nLength );
	}

	inline void Write(const void* pData, const size_t nLength) throw()
	{
		CQuickLock oOutputLock( *m_pOutputSection );

		m_pOutput->Add( pData, nLength );
	}

	inline void Write(const CString& strData, const UINT nCodePage = CP_ACP) throw()
	{
		CQuickLock oOutputLock( *m_pOutputSection );

		m_pOutput->Print( strData, nCodePage );
	}

	inline void Write(const CBuffer* pBuffer) throw()
	{
		CQuickLock oOutputLock( *m_pOutputSection );

		m_pOutput->Add( pBuffer->m_pBuffer, pBuffer->m_nLength );
	}

	inline void Write(CPacket* pPacket) throw()
	{
		CQuickLock oOutputLock( *m_pOutputSection );

		pPacket->ToBuffer( m_pOutput );
	}

	template
		<
		typename Descriptor,
		template< typename > class StoragePolicy,
		template< typename > class CheckingPolicy,
		template< typename > class ValidationPolicy
		>
	inline void Write(Hashes::Hash< Descriptor, StoragePolicy,
		CheckingPolicy, ValidationPolicy >& oHash) throw()
	{
		CQuickLock oOutputLock( *m_pOutputSection );

		m_pOutput->Add( &oHash[ 0 ], oHash.byteCount );
	}

	template
		<
		typename Descriptor,
		template< typename > class StoragePolicy,
		template< typename > class CheckingPolicy,
		template< typename > class ValidationPolicy
		>
	inline void Read(Hashes::Hash< Descriptor, StoragePolicy,
		CheckingPolicy, ValidationPolicy >& oHash) throw()
	{
		CQuickLock oInputLock( *m_pInputSection );

		m_pInput->Read( &oHash[ 0 ], oHash.byteCount );
	}

	inline BOOL Read(void* pData, const size_t nLength) throw()
	{
		CQuickLock oInputLock( *m_pInputSection );

		return m_pInput->Read( pData, nLength );
	}

	inline BOOL Read(CString& strData, BOOL bPeek = FALSE) throw()
	{
		CQuickLock oInputLock( *m_pInputSection );

		return m_pInput->ReadLine( strData, bPeek );
	}

	inline void RemoveFromInput(const size_t nLength) throw()
	{
		CQuickLock oInputLock( *m_pInputSection );

		m_pInput->Remove( nLength );
	}

	inline void Prefix(LPCSTR pszText, const size_t nLength) throw()
	{
		CQuickLock oInputLock( *m_pInputSection );

		m_pInput->Prefix( pszText, nLength );
	}

	inline BYTE PeekAt(const size_t nPos) const throw()
	{
		CQuickLock oInputLock( *m_pInputSection );

		return m_pInput->m_pBuffer[ nPos ];
	}

	inline BOOL StartsWith(LPCSTR pszString, const size_t nLength)
	{
		CQuickLock oInputLock( *m_pInputSection );

		return m_pInput->StartsWith( pszString, nLength, FALSE );
	}

	inline void CreateBuffers() throw()
	{
		{
			CQuickLock oInputLock( *m_pInputSection );
			if ( ! m_pInput )
				m_pInput = new CBuffer();
		}
		{
			CQuickLock oOutputLock( *m_pOutputSection );
			if ( ! m_pOutput )
				m_pOutput = new CBuffer();
		}
	}

	inline void DestroyBuffers() throw()
	{
		{
			CQuickLock oInputLock( *m_pInputSection );
			delete m_pInput;
			m_pInput = NULL;
		}
		{
			CQuickLock oOutputLock( *m_pOutputSection );
			delete m_pOutput;
			m_pOutput = NULL;
		}
	}

// Overrides
public:
	// Make a connection, accept a connection, copy a connection, and close a connection
	virtual BOOL ConnectTo(const SOCKADDR_IN* pHost);                  // Connect to an IP address and port number
	virtual BOOL ConnectTo(const IN_ADDR* pAddress, WORD nPort);
	virtual void AcceptFrom(SOCKET hSocket, SOCKADDR_IN* pHost); // Accept a connection from a remote computer
	virtual void AttachTo(CConnection* pConnection);             // Copy a connection (do)
	virtual void Close(UINT nError = 0);		// Disconnect from the remote computer
	virtual void DelayClose(UINT nError);		// Send the buffer then close the socket, record the error given

	// Read and write data through the socket, and look at headers
	virtual BOOL OnRun();                // (do) just returns true
	virtual BOOL OnConnected();          // (do) just returns true
	virtual BOOL OnRead();               // Read data waiting in the socket into the input buffer
	virtual BOOL OnWrite();              // Move the contents of the output buffer into the socket
	virtual void OnDropped(); 			 // (do) empty
	virtual BOOL OnHeaderLine(CString& strHeader, CString& strValue); // Processes a single line from the headers
	virtual BOOL OnHeadersComplete();    // (do) just returns true

// Statics
public:
	// Hard-coded settings for the bandwidth transfer meter
	static const DWORD	METER_SECOND	= 1000ul;						// 1000 milliseconds is 1 second
	static const DWORD	METER_MINIMUM	= METER_SECOND / 10ul;			// Granuality of bandwidth meter, 1/10th of a second
	static const DWORD	METER_LENGTH	= 60ul;							// Number of slots in the bandwidth meter
	static const DWORD	METER_PERIOD	= METER_MINIMUM * METER_LENGTH;	// The time that the bandwidth meter keeps information for

	// Keep track of how fast we are reading or writing bytes to a socket
	typedef struct
	{
		// Options to limit bandwidth
		DWORD*	pLimit;		// Points to a DWORD that holds the limit for this bandwidth meter

		// Transfer statistics
		DWORD	nTotal;		// The total number of bytes read or written
		DWORD	tLast;		// The time the last read or write happened
		DWORD	nMeasure;	// The average speed in bytes per second over the last METER_PERIOD

		// The arrays of byte counts and times
		DWORD			pHistory[METER_LENGTH];	// Records of a number of bytes transferred
		DWORD			pTimes[METER_LENGTH];	// The times each of these transfers happened
		DWORD			nPosition;				// The next spot in the array to use
		mutable DWORD	tLastLimit;				// When we last calculated the limit
		DWORD			tLastSlot;				// When we started using this time slot

		void	Add(const DWORD nBytes, const DWORD tNow);					// Add to History and Time arrays
		DWORD	CalculateLimit (DWORD tNow, bool bMaxMode = true) const;	// Work out the limit
		DWORD	CalculateUsage (DWORD tTime ) const;						// Work out the meter usage from a given time
																			// ( optimal for time periods more than METER_LENGTH / 2 )
		DWORD	CalculateUsage (DWORD tTime, bool bShortPeriod ) const;		// Work out the meter usage from a given time
																			// ( optimal for time periods less than METER_LENGTH / 2 )
	} TCPBandwidthMeter;

	// Structures to control bandwidth in each direction
	TCPBandwidthMeter m_mInput;		// Input TCP bandwidth meter
	TCPBandwidthMeter m_mOutput;	// Output TCP bandwidth meter
};
