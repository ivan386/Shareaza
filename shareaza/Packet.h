//
// Packet.h
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

#if !defined(AFX_PACKET_H__3094C1CC_8AD2_49BD_BF10_EA639A9EAE6F__INCLUDED_)
#define AFX_PACKET_H__3094C1CC_8AD2_49BD_BF10_EA639A9EAE6F__INCLUDED_

#pragma once

//
// Tools
//

#define SWAP_SHORT(x) ( ( ( (x) & 0xFF00 ) >> 8 ) + ( ( (x) & 0x00FF ) << 8 ) )
#define SWAP_LONG(x) ( ( ( (x) & 0xFF000000 ) >> 24 ) + ( ( (x) & 0x00FF0000 ) >> 8 ) + ( ( (x) & 0x0000FF00 ) << 8 ) + ( ( (x) & 0x000000FF ) << 24 ) )
#define SWAP_64(x) ( ( SWAP_LONG( (x) & 0xFFFFFFFF ) << 32 ) | SWAP_LONG( (x) >> 32 ) )

#define PACKET_GROW			128
#define PACKET_BUF_SCHAR	127
#define PACKET_BUF_WCHAR	127
#define SHAREAZA_VENDOR_A	"RAZA"
#define SHAREAZA_VENDOR_T	_T("RAZA")

class CBuffer;
class CNeighbour;

//
// Packet
//

class CPacket
{
// Construction
protected:
	CPacket(PROTOCOLID nProtocol);
	virtual ~CPacket();

// Attributes
public:
	PROTOCOLID	m_nProtocol;
	CPacket*	m_pNext;
	DWORD		m_nReference;
public:
	BYTE*		m_pBuffer;
	DWORD		m_nBuffer;
	DWORD		m_nLength;
	DWORD		m_nPosition;
	BOOL		m_bBigEndian;
	
	enum { seekStart, seekEnd };
	
// Static Buffers
protected:
	static CHAR		m_szSCHAR[PACKET_BUF_SCHAR+1];
	static WCHAR	m_szWCHAR[PACKET_BUF_WCHAR+1];
	
// Operations
public:
	virtual void	Reset();
	virtual void	ToBuffer(CBuffer* pBuffer) const = 0;
public:
	void			Seek(DWORD nPosition, int nRelative = seekStart);
	void			Shorten(DWORD nLength);
	virtual CString	ReadString(DWORD nMaximum = 0xFFFFFFFF);
	virtual void	WriteString(LPCTSTR pszString, BOOL bNull = TRUE);
	virtual int		GetStringLen(LPCTSTR pszString) const;
	LPBYTE			ReadZLib(DWORD nLength, DWORD* pnOutput, DWORD nSuggest = 0);
	void			WriteZLib(LPCVOID pData, DWORD nLength);
	BYTE*			WriteGetPointer(DWORD nLength, DWORD nOffset = 0xFFFFFFFF);
public:
	virtual LPCTSTR	GetType() const;
	CString			ToHex() const;
	CString			ToASCII() const;
	virtual void	Debug(LPCTSTR pszReason) const;
	void			SmartDump(CNeighbour* pNeighbour, IN_ADDR* pUDP, BOOL bOutgoing) const;
public:
	virtual BOOL	GetRazaHash(SHA1* pHash, DWORD nLength = 0xFFFFFFFF) const;
	void			RazaSign();
	BOOL			RazaVerify() const;

// Inline Packet Operations
public:
	
	inline int GetRemaining()
	{
		return m_nLength - m_nPosition;
	}
	
	inline void Read(LPVOID pData, int nLength)
	{
		if ( m_nPosition + nLength > m_nLength ) AfxThrowUserException();
		CopyMemory( pData, m_pBuffer + m_nPosition, nLength );
		m_nPosition += nLength;
	}
	
	inline BYTE ReadByte()
	{
		if ( m_nPosition >= m_nLength ) AfxThrowUserException();
		return m_pBuffer[ m_nPosition++ ];
	}
	
	inline BYTE PeekByte()
	{
		if ( m_nPosition >= m_nLength ) AfxThrowUserException();
		return m_pBuffer[ m_nPosition ];
	}
	
	inline WORD ReadShortLE()
	{
		if ( m_nPosition + 2 > m_nLength ) AfxThrowUserException();
		WORD nValue = *(WORD*)( m_pBuffer + m_nPosition );
		m_nPosition += 2;
		return nValue;
	}
	
	inline WORD ReadShortBE()
	{
		if ( m_nPosition + 2 > m_nLength ) AfxThrowUserException();
		WORD nValue = *(WORD*)( m_pBuffer + m_nPosition );
		m_nPosition += 2;
		return m_bBigEndian ? SWAP_SHORT( nValue ) : nValue;
	}
	
	inline DWORD ReadLongLE()
	{
		if ( m_nPosition + 4 > m_nLength ) AfxThrowUserException();
		DWORD nValue = *(DWORD*)( m_pBuffer + m_nPosition );
		m_nPosition += 4;
		return nValue;
	}
	
	inline DWORD ReadLongBE()
	{
		if ( m_nPosition + 4 > m_nLength ) AfxThrowUserException();
		DWORD nValue = *(DWORD*)( m_pBuffer + m_nPosition );
		m_nPosition += 4;
		return m_bBigEndian ? SWAP_LONG( nValue ) : nValue;
	}
	
	inline QWORD ReadInt64()
	{
		if ( m_nPosition + 8 > m_nLength ) AfxThrowUserException();
		QWORD nValue = *(QWORD*)( m_pBuffer + m_nPosition );
		m_nPosition += 8;
		return m_bBigEndian ? SWAP_64( nValue ) : nValue;
	}
	
	inline void Ensure(int nLength)
	{
		if ( m_nLength + nLength > m_nBuffer )
		{
			m_nBuffer += max( nLength, PACKET_GROW );
			LPBYTE pNew = new BYTE[ m_nBuffer ];
			CopyMemory( pNew, m_pBuffer, m_nLength );
			if ( m_pBuffer ) delete [] m_pBuffer;
			m_pBuffer = pNew;
		}
	}
	
	inline void Write(LPCVOID pData, int nLength)
	{
		if ( m_nLength + nLength > m_nBuffer )
		{
			m_nBuffer += max( nLength, PACKET_GROW );
			LPBYTE pNew = new BYTE[ m_nBuffer ];
			CopyMemory( pNew, m_pBuffer, m_nLength );
			if ( m_pBuffer ) delete [] m_pBuffer;
			m_pBuffer = pNew;
		}
		
		CopyMemory( m_pBuffer + m_nLength, pData, nLength );
		m_nLength += nLength;
	}
	
	inline void WriteByte(BYTE nValue)
	{
		if ( m_nLength + sizeof(nValue) > m_nBuffer ) Ensure( sizeof(nValue) );
		m_pBuffer[ m_nLength++ ] = nValue;
	}
	
	inline void WriteShortLE(WORD nValue)
	{
		if ( m_nLength + sizeof(nValue) > m_nBuffer ) Ensure( sizeof(nValue) );
		*(WORD*)( m_pBuffer + m_nLength ) = nValue;
		m_nLength += sizeof(nValue);
	}
	
	inline void WriteShortBE(WORD nValue)
	{
		if ( m_nLength + sizeof(nValue) > m_nBuffer ) Ensure( sizeof(nValue) );
		*(WORD*)( m_pBuffer + m_nLength ) = m_bBigEndian ? SWAP_SHORT( nValue ) : nValue;
		m_nLength += sizeof(nValue);
	}
	
	inline void WriteLongLE(DWORD nValue)
	{
		if ( m_nLength + sizeof(nValue) > m_nBuffer ) Ensure( sizeof(nValue) );
		*(DWORD*)( m_pBuffer + m_nLength ) = nValue;
		m_nLength += sizeof(nValue);
	}
	
	inline void WriteLongBE(DWORD nValue)
	{
		if ( m_nLength + sizeof(nValue) > m_nBuffer ) Ensure( sizeof(nValue) );
		*(DWORD*)( m_pBuffer + m_nLength ) = m_bBigEndian ? SWAP_LONG( nValue ) : nValue;
		m_nLength += sizeof(nValue);
	}
	
	inline void WriteInt64(QWORD nValue)
	{
		if ( m_nLength + sizeof(nValue) > m_nBuffer ) Ensure( sizeof(nValue) );
		*(QWORD*)( m_pBuffer + m_nLength ) = m_bBigEndian ? SWAP_64( nValue ) : nValue;
		m_nLength += sizeof(nValue);
	}
	
// Inline Allocation
public:
	
	inline void AddRef()
	{
		m_nReference++;
	}
	
	inline void Release()
	{
		if ( this != NULL && ! --m_nReference ) Delete();
	}
	
	inline void ReleaseChain()
	{
		if ( this == NULL ) return;
		
		for ( CPacket* pPacket = this ; pPacket ; )
		{
			CPacket* pNext = pPacket->m_pNext;
			pPacket->Release();
			pPacket = pNext;
		}
	}
	
	virtual inline void Delete() = 0;
	
	friend class CPacketPool;
};


class CPacketPool
{
// Construction
public:
	CPacketPool();
	virtual ~CPacketPool();

// Attributes
protected:
	CPacket*	m_pFree;
	DWORD		m_nFree;
protected:
	CCriticalSection	m_pSection;
	CPtrArray			m_pPools;

// Operations
protected:
	void	Clear();
	void	NewPool();
protected:
	virtual void NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch) = 0;
	virtual void FreePoolImpl(CPacket* pPool) = 0;
	
// Inlines
public:
	inline CPacket* New()
	{
		m_pSection.Lock();
		
		if ( m_nFree == 0 ) NewPool();
		ASSERT( m_nFree > 0 );
		
		CPacket* pPacket = m_pFree;
		m_pFree = m_pFree->m_pNext;
		m_nFree --;
		
		m_pSection.Unlock();
		
		pPacket->Reset();
		pPacket->AddRef();
		
		return pPacket;
	}
	
	inline void Delete(CPacket* pPacket)
	{
		ASSERT( pPacket != NULL );
		ASSERT( pPacket->m_nReference == 0 );
		
		m_pSection.Lock();
		
		pPacket->m_pNext = m_pFree;
		m_pFree = pPacket;
		m_nFree ++;
		
		m_pSection.Unlock();
	}

};

#endif // !defined(AFX_PACKET_H__3094C1CC_8AD2_49BD_BF10_EA639A9EAE6F__INCLUDED_)
