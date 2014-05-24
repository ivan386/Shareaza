//
// Packet.h
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

// CPacket represents a packet on a peer-to-peer network, and CPacketPool keeps lists of them
// http://www.shareazasecurity.be/wiki/index.php?title=Developers.Code.CPacket

#pragma once

// When the allocated block of memory needs to be bigger, make it 128 bytes bigger
const DWORD PACKET_GROW = 128u;

// Tell the compiler these classes exist, and it will find out more about them soon
class CBuffer;
class CNeighbour;

// A packet on a peer-to-peer network
class CPacket
{

protected:

	// Create a new CPacket object, and delete this one
	CPacket(PROTOCOLID nProtocol); // Make a new CPacket object for the given protocol id, like Gnutella or eDonkey2000
	virtual ~CPacket();            // The destructor is virtual, meaning classes that inherit from CPacket may replace it with their own destructor

	volatile LONG m_nReference;		// The number of other objects that need this packet and point to it, 0 if everyone is done with it

public:

	// The network this packet is on, like Gnutella or eDonkey2000
	PROTOCOLID m_nProtocol;

	// List pointer and reference count
	CPacket* m_pNext;      // Unused packets in the packet pool are linked together from m_pFree, through each packet's m_pNext pointer

public:

	// Packet data
	BYTE* m_pBuffer;    // A pointer to memory we allocated to hold the bytes of the payload of the packet, this is not a CBuffer object
	DWORD m_nBuffer;    // The size of the allocated block of memory that holds the payload
	DWORD m_nLength;    // The number of bytes of data we've written into the allocated block of memory
	DWORD m_nPosition;  // What byte we are on, this position index is remembered by the packet between calls to methods
	BOOL  m_bBigEndian; // True if the bytes of the packet are in big endian format, which is the default
	
	BOOL  m_bUDP;
	BOOL  m_bOutgoing;
	DWORD_PTR m_nNeighbourUnique;

	// Set the position a given distance forwards from the start, or backwards from the end
	enum { seekStart, seekEnd, seekCurrent };

public:

	// Reset this packet object to make it like it was when it came from the constructor
	virtual void Reset();

	// What is const = 0 (do)
	virtual void ToBuffer(CBuffer* pBuffer, bool bTCP = true) = 0;

public:

	// Set the position the given distance from the given end
	void Seek(DWORD nPosition, int nRelative = seekStart);	
	// Shorten the packet to the given number of bytes
	void Shorten(DWORD nLength);
	// Remove data from packet start
	void Remove(DWORD nLength);
	// Compare (case-sensetive) content of packet buffer with string at specified offset
	BOOL Compare(const void* szString, DWORD nLength, DWORD nOffset = 0) const;
	// Find character inside packet buffer. Returns character offset or -1 if not found.
	int Find(BYTE c, DWORD nOffset = 0) const;
	// Get character at specified (range-safe) offset from packet buffer
	BYTE GetAt(DWORD nOffset) const;

	virtual CString ReadString(UINT cp, DWORD nMaximum = 0xFFFFFFFF);

	// Read and write ASCII text in the packet
	virtual CString ReadStringASCII(DWORD nMaximum = 0xFFFFFFFF);      // Read null terminated ASCII text at our position in the packet
	virtual void    WriteString(LPCTSTR pszString, BOOL bNull = TRUE); // Write ASCII text and a null terminator into the end of the packet

	// String utility, not at all related to the packet
	virtual int GetStringLen(LPCTSTR pszString) const; // Takes a string, and determines how long it would be as ASCII text

	// Read and write ASCII text in the packet, using the UTF8 code page
	virtual CString ReadStringUTF8(DWORD nMaximum = 0xFFFFFFFF);           // Read null terminated ASCII text at our position in the packet
	virtual void    WriteStringUTF8(LPCTSTR pszString, BOOL bNull = TRUE); // Write ASCII text and a null terminator into the end of the packet

	// String utility, not at all related to the packet
	virtual int GetStringLenUTF8(LPCTSTR pszString) const; // Takes a string, and determines how long it would be as ASCII text converted UTF8

	// Insert data into the packet
	BYTE* WriteGetPointer(DWORD nLength, DWORD nOffset = 0xFFFFFFFF); // Makes room at the given spot, and returns a pointer to it

public:

	// Inheriting classes will override this to return text describing what type of packet this is
	virtual CString GetType() const;

	// Encode the bytes of the packet into text
	virtual CString ToHex()   const; // Express the bytes of the packet in base 16 with spaces, like "08 C0 12 AF"
	virtual CString ToASCII() const; // Express the bytes of the packet as ASCII characters, like "abc..fgh.i", spaces replace low characters

#ifdef _DEBUG
	// Inheriting classes will override this to (do)
	virtual void    Debug(LPCTSTR pszReason) const;
#endif // _DEBUG

	// Gives this packet and related objects to each window in the tab bar for them to process it
	virtual void SmartDump(const SOCKADDR_IN* pAddress, BOOL bUDP, BOOL bOutgoing, DWORD_PTR nNeighbourUnique = 0);

public:
	// Does nothing (do)
	void			RazaSign();
	BOOL			RazaVerify() const;

public:
	// Get current position
	inline const BYTE* GetCurrent() const
	{
		return m_pBuffer + m_nPosition;
	}

	// Get the length beyond our position in the packet
	inline DWORD GetRemaining() const
	{
		// Return the number of bytes of packet data at and beyond our position in the packet
		ASSERT( m_nLength >= m_nPosition );
		return m_nLength - m_nPosition;
	}

	// Takes a pointer to a buffer, and the number of bytes we want written there
	// Copies this number of bytes from the packet, and moves the packet's position beyond them
	inline void Read(LPVOID pData, int nLength)
	{
		// Make sure the requested length doesn't poke beyond the end of the packet
		if ( m_nPosition + nLength > m_nLength ) AfxThrowUserException();

		// Copy memory from the packet to the given buffer
		CopyMemory(
			pData,                   // Destination is the given pointer
			m_pBuffer + m_nPosition, // Source is our position in the packet
			nLength );               // Size is the requested length

		// Move our position in the packet beyond the data we just copied out
		m_nPosition += nLength;
	}

	// Read the next byte in the packet, moving the position beyond it
	// Returns the byte
	inline BYTE ReadByte()
	{
		// Make sure the position isn't at the end or beyond it, where there is no byte to read
		if ( m_nPosition >= m_nLength ) AfxThrowUserException();

		// Read one byte, return it, and move our position in this packet beyond it
		return m_pBuffer[ m_nPosition++ ];
	}

	// Read any Hash directly from a packet
	template
	<
		typename Descriptor,
		template< typename > class StoragePolicy,
		template< typename > class CheckingPolicy,
		template< typename > class ValidationPolicy
	>
	void Read(Hashes::Hash< Descriptor, StoragePolicy,
			CheckingPolicy, ValidationPolicy >& oHash)
	{
		Read( &oHash[ 0 ], oHash.byteCount );
		oHash.validate();
	}

	// Get the next byte in the packet, not moving the position beyond it
	// Returns the byte
	inline BYTE PeekByte()
	{
		// Make sure the position isn't at the end or beyond it, where there is no byte to read
		if ( m_nPosition >= m_nLength ) AfxThrowUserException();

		// Read one byte, return it, but don't move our position in this packet beyond it
		return m_pBuffer[ m_nPosition ];
	}

	// Read the next 2 bytes in the packet, moving the position beyond them
	// Returns the bytes in a word, and assumes little endian order which we don't need to change
	inline WORD ReadShortLE()
	{
		// Make sure there are at least 2 bytes of data at our position in the packet
		if ( m_nPosition + 2 > m_nLength ) AfxThrowUserException();

		// Read the 2 bytes at the position, and move the position past them
		WORD nValue = *(WORD*)( m_pBuffer + m_nPosition ); // Look at the pointer as a 2 byte word
		m_nPosition += 2;                                  // Move the position beyond it

		// Return the 2 bytes in a word
		return nValue;
	}

	// Read the next 2 bytes in the packet, moving the position beyond them
	// Returns the bytes in a word, switching their order if the packet thinks its contents are in big endian order
	inline WORD ReadShortBE()
	{
		// Make sure there are at least 2 bytes of data at our position in the packet
		if ( m_nPosition + 2 > m_nLength ) AfxThrowUserException();

		// Read the 2 bytes at the position, move the position past them, and return them
		WORD nValue = *(WORD*)( m_pBuffer + m_nPosition ); // Look at the pointer as a 2 byte word
		m_nPosition += 2;                                  // Move the position beyond it

		// If the packet is in big endian, reverse the order of the 2 bytes before returning them in a word
		return m_bBigEndian ? swapEndianess( nValue ) : nValue;
	}

	// Read the next 4 bytes in the packet, moving the position beyond them
	// Returns the bytes in a DWORD, and assumes little endian order which we don't need to change
	inline DWORD ReadLongLE()
	{
		// Make sure there are at least 4 bytes of data at our position in the packet
		if ( m_nPosition + 4 > m_nLength ) AfxThrowUserException();

		// Read the 4 bytes at the position, and move the position past them
		DWORD nValue = *(DWORD*)( m_pBuffer + m_nPosition ); // Look at the pointer as a 4 byte DWORD
		m_nPosition += 4;                                    // Move the position beyond it

		// Return the 4 bytes in a DWORD
		return nValue;
	}

	// Read the next 4 bytes in the packet, moving the position beyond them
	// Returns the bytes in a DWORD, reversing their order if the packet thinks its contents are in big endian order
	inline DWORD ReadLongBE()
	{
		// Make sure there are at least 4 bytes of data at our position in the packet
		if ( m_nPosition + 4 > m_nLength ) AfxThrowUserException();

		// Read the 4 bytes at the position, and move the position past them
		DWORD nValue = *(DWORD*)( m_pBuffer + m_nPosition ); // Look at the pointer as a 4 byte DWORD
		m_nPosition += 4;                                    // Move the position beyond it

		// If the packet is in big endian, reverse the order of the 4 bytes before returning them in a DWORD
		return m_bBigEndian ? swapEndianess( nValue ) : nValue;
	}

	// Read the next 8 bytes in the packet, moving the position beyond them
	// Returns the bytes in a QWORD, reversing their order if the packet thinks its contents are in big endian order
	inline QWORD ReadInt64()
	{
		// Make sure there are at least 8 bytes of data at our position in the packet
		if ( m_nPosition + 8 > m_nLength ) AfxThrowUserException();

		// Read the 8 bytes at the position, and move the position past them
		QWORD nValue = *(QWORD*)( m_pBuffer + m_nPosition ); // Look at the pointer as a 8 byte QWORD
		m_nPosition += 8;                                    // Move the position beyond it

		// If the packet is in big endian, reverse the order of the 8 bytes before returning them in a QWORD
		return m_bBigEndian ? swapEndianess( nValue ) : nValue;
	}

	// Takes a length of bytes we would like to add to the packet
	// Ensures the allocated block of memory is big enough for them, making it bigger if necessary
	inline BOOL Ensure(DWORD nLength)
	{
		// If the buffer isn't big enough to hold that many more new bytes
		if ( m_nLength + nLength > m_nBuffer )
		{
			// Switch to a new bigger one
			m_nBuffer += max( nLength, PACKET_GROW );	// Size the buffer larger by the requested amount, or the packet grow size of 128 bytes
			LPBYTE pNew = new (std::nothrow) BYTE[ m_nBuffer ];		// Allocate a new block of memory of that size
			if ( pNew == NULL )							// Check for out of memory error
				return FALSE;
			if ( m_pBuffer )
			{
				CopyMemory( pNew, m_pBuffer, m_nLength );	// Copy the packet data from the old buffer to the new one
				delete [] m_pBuffer;						// If there is an old buffer, free it
			}
			m_pBuffer = pNew;							// Point the packet object's member variable pointer at the new buffer
		}
		return TRUE;
	}

	// Takes a pointer to data and the number of bytes there
	// Adds them to the end of the buffer
	inline BOOL Write(LPCVOID pData, DWORD nLength)
	{
		// If the allocated block of memory doesn't have enough extra space to hold the new data
		if ( m_nLength + nLength > m_nBuffer )
		{
			// Make it bigger
			m_nBuffer += max( nLength, PACKET_GROW );	// Calculate the new size to be nLength or 128 bytes bigger
			LPBYTE pNew = new (std::nothrow) BYTE[ m_nBuffer ];		// Allocate a new buffer of that size
			if ( pNew == NULL )							// Check for out of memory error
				return FALSE;
			if ( m_pBuffer )
			{
				CopyMemory( pNew, m_pBuffer, m_nLength );	// Copy the data from the old buffer to the new one
				delete [] m_pBuffer;						// Delete the old buffer
			}
			m_pBuffer = pNew;							// Point m_pBuffer at the new, bigger buffer
		}

		// Add the given data to the end of the packet
		if ( nLength && pData )
		{
			CopyMemory( m_pBuffer + m_nLength, pData, nLength ); // Copy the data into the end
			m_nLength += nLength;                                // Record that the new bytes are stored here
		}
		return TRUE;
	}

	// Write any Hash directly into a packet ( just the raw data )
	template
	<
		typename Descriptor,
		template< typename > class StoragePolicy,
		template< typename > class CheckingPolicy,
		template< typename > class ValidationPolicy
	>
	void Write(const Hashes::Hash< Descriptor, StoragePolicy,
			CheckingPolicy, ValidationPolicy >& oHash)
	{
		if ( !oHash.isValid() ) AfxThrowUserException();
		Write( &oHash[ 0 ], oHash.byteCount );
	}

	// Takes a byte
	// Writes it into the end of the packet
	inline void WriteByte(BYTE nValue)
	{
		// Make sure there is room for the byte
		if ( m_nLength + sizeof( nValue ) > m_nBuffer )
		{
			if ( ! Ensure( sizeof( nValue ) ) ) return;
		}

		// Write it at the end of the packet, and record that it is there
		m_pBuffer[ m_nLength++ ] = nValue;
	}

	// Takes 2 bytes in a word
	// Writes them into the end of the packet, keeping them in the same order
	inline void WriteShortLE(WORD nValue)
	{
		// Make sure there is room for the 2 bytes
		if ( m_nLength + sizeof(nValue) > m_nBuffer )
		{
			if ( ! Ensure( sizeof( nValue ) ) ) return;
		}

		// Write the two bytes as a word at the end of the packet, and record that it is there
		*(WORD*)( m_pBuffer + m_nLength ) = nValue;
		m_nLength += sizeof(nValue);
	}

	// Takes 2 bytes in a word
	// Writes them into the end of the packet, reversing their order if the packet is in big endian order
	inline void WriteShortBE(WORD nValue)
	{
		// Make sure there is room for the 2 bytes
		if ( m_nLength + sizeof(nValue) > m_nBuffer )
		{
			if ( ! Ensure( sizeof( nValue ) ) ) return;
		}

		// Write the 2 bytes as a word at the end of the packet, and record that it is there
		*(WORD*)( m_pBuffer + m_nLength ) = m_bBigEndian ? swapEndianess( nValue ) : nValue; // Reverse their order if necessary
		m_nLength += sizeof(nValue);
	}

	// Takes 4 bytes in a DWORD
	// Writes them into the end of the packet, keeping them in the same order
	inline void WriteLongLE(DWORD nValue)
	{
		// Make sure there is room for the 4 bytes
		if ( m_nLength + sizeof(nValue) > m_nBuffer )
		{
			if ( ! Ensure( sizeof( nValue ) ) ) return;
		}

		// Write the 4 bytes as a DWORD at the end of the packet, and record that it is there
		*(DWORD*)( m_pBuffer + m_nLength ) = nValue;
		m_nLength += sizeof(nValue);
	}

	// Takes 4 bytes in a DWORD
	// Writes them into the end of the packet, reversing their order if the packet is in big endian order
	inline void WriteLongBE(DWORD nValue)
	{
		// Make sure there is room for the 4 bytes
		if ( m_nLength + sizeof(nValue) > m_nBuffer )
		{
			if ( ! Ensure( sizeof( nValue ) ) ) return;
		}

		// Write the 4 bytes as a DWORD at the end of the packet, and record that it is there
		*(DWORD*)( m_pBuffer + m_nLength ) = m_bBigEndian ? swapEndianess( nValue ) : nValue; // Reverse their order if necessary
		m_nLength += sizeof(nValue);
	}

	// Takes 8 bytes in a QWORD
	// Writes them into the end of the packet, reversing their order if the packet is in big endian order
	inline void WriteInt64(QWORD nValue)
	{
		// Make sure there is room for the 8 bytes
		if ( m_nLength + sizeof(nValue) > m_nBuffer )
		{
			if ( ! Ensure( sizeof( nValue ) ) ) return;
		}

		// Write the 8 bytes as a QWORD at the end of the packet, and record that it is there
		*(QWORD*)( m_pBuffer + m_nLength ) = m_bBigEndian ? swapEndianess( nValue ) : nValue; // Reverse their order if necessary
		m_nLength += sizeof(nValue);
	}

public:

	// Have this packet object remember that one more thing is referencing it
	inline void AddRef()
	{
		InterlockedIncrement( &m_nReference );
	}

	// Tell this packet object that one less thing needs it
	inline void Release()
	{
		ASSERT( m_nReference > 0 && this );
		if ( this != NULL && ! InterlockedDecrement( &m_nReference ) )
			Delete();
	}

	// Decrement the reference count of this packet, the packet it points to, and so on down the linked list from here
	inline void ReleaseChain()
	{
		// Make sure this object exists
		if ( this == NULL ) return;

		// Point pPacket at this packet, and loop until that pointer becomes null
		for ( CPacket* pPacket = this ; pPacket ; )
		{
			// Decrement the reference count of this packet, and move to the next one
			CPacket* pNext = pPacket->m_pNext; // Save a pointer to the next packet
			pPacket->Release();                // Record that one fewer object needs this one, which might delete it
			pPacket = pNext;                   // Move pPacket to the next one
		}
	}

	// (do)
	virtual inline void Delete() = 0;

	// Packet handler
	virtual BOOL OnPacket(const SOCKADDR_IN* pHost) = 0;

	// Let the CPacketPool class access the private members of this one
	friend class CPacketPool;

private:
	CPacket(const CPacket&);
	CPacket& operator=(const CPacket&);
};

// Allocates and holds array of 256 packets so we can grab a packet to use it quickly
class CPacketPool
{
public:

	// Make a new packet pool, and delete one
	CPacketPool();
	virtual ~CPacketPool(); // Virtual lets inheriting classes override this with their own custom destructor

protected:

	// Free packets ready to be used
	CPacket* m_pFree; // A linked list of packets that are free and ready to be removed from the linked list and used
	DWORD    m_nFree; // The total number of free packets in the linked list

protected:

	// Used to make sure only one thread can access this object at a time
	CCriticalSection m_pSection;

	// An array of pointers, each of which points to an array of 256 packets
	CArray< CPacket* > m_pPools;

protected:

	// Delete all the packet pools, and make a new one
	void Clear();   // Delete all the packet pools in this CPacketPool object
	void NewPool(); // Create a new packet pool, which is an array that can hold 256 packets, and add it to the list here

protected:

	// Methods inheriting classes implement to allocate and free arrays of 256 packets
	virtual void NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch) = 0; // Allocate a new array of 256 packets
	virtual void FreePoolImpl(CPacket* pPool) = 0;                         // Free an array of 256 packets

public:

	// Removes a packet from the linked list of free packets, resets it, and adds a reference count
	// Returns a pointer to the new packet the caller can use
	inline CPacket* New()
	{
		// Make sure this is the only thread accessing this CPacketPool object at this time
		m_pSection.Lock();

		// If there aren't any packet pools yet, make one
		if ( m_nFree == 0 ) NewPool(); // Now, m_pFree will point to the last packet in that pool, and the first packet will point to null
		ASSERT( m_nFree > 0 );         // Make sure this caused the count of packets to go up, it should go to 256

		// Remove the last linked packet in the most recently added packet pool from the linked list of free packets
		CPacket* pPacket = m_pFree; // Point pPacket at the last packet in the newest pool
		m_pFree = m_pFree->m_pNext; // Move m_pFree to the second to last packet in the newest pool
		m_nFree--;                  // Record that there is one fewer packet linked together in all the packet pools here

		// We're done, let other threads stuck on a Lock line like that one above inside
		m_pSection.Unlock();

		// Prepare the packet for use
		pPacket->Reset();  // Clear the values of the packet we just unlinked from the list
		pPacket->AddRef(); // Record that an external object is referencing this packet

		// Return a pointer to the packet we just
		return pPacket;
	}

	// Takes a pointer to a packet
	// Links it back into the list of free packets we can grab to use quickly
	inline void Delete(CPacket* pPacket)
	{
		// Make sure the pointer points to a packet, and that packet doesn't still have a reference count
		ASSERT( pPacket->m_nReference == 0 );

		// Make sure this is the only thread accessing this CPacketPool object at this time
		m_pSection.Lock();

		// Link the given packet back into the list of free ones we can use later
		pPacket->m_pNext = m_pFree; // Link the given packet into the front of the free list
		m_pFree = pPacket;
		m_nFree++;                  // Record one more packet is in the free list

		// We're done, let other threads stuck on a Lock line like that one above inside
		m_pSection.Unlock();
	}
};
