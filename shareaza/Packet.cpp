//
// Packet.cpp
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
#include "Network.h"
#include "Packet.h"
#include "ZLib.h"
#include "SHA.h"
#include "Buffer.h"

#include "WndMain.h"
#include "WndPacket.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CHAR	CPacket::m_szSCHAR[PACKET_BUF_SCHAR+1];
WCHAR	CPacket::m_szWCHAR[PACKET_BUF_WCHAR+1];


//////////////////////////////////////////////////////////////////////
// CPacket construction

CPacket::CPacket(PROTOCOLID nProtocol)
{
	m_nProtocol		= nProtocol;
	m_pNext			= NULL;
	m_nReference	= 0;
	
	m_pBuffer		= NULL;
	m_nBuffer		= 0;
	m_nLength		= 0;
	m_nPosition		= 0;
	m_bBigEndian	= TRUE;
}

CPacket::~CPacket()
{
	ASSERT( m_nReference == 0 );
	if ( m_pBuffer ) delete [] m_pBuffer;
}

//////////////////////////////////////////////////////////////////////
// CPacket reset

void CPacket::Reset()
{
	ASSERT( m_nReference == 0 );
	
	m_pNext			= NULL;
	m_nLength		= 0;
	m_nPosition		= 0;
	m_bBigEndian	= TRUE;
}

//////////////////////////////////////////////////////////////////////
// CPacket position and seeking

void CPacket::Seek(DWORD nPosition, int nRelative)
{
	if ( nRelative == seekStart )
	{
		m_nPosition = max( DWORD(0), min( m_nLength, nPosition ) );
	}
	else
	{
		m_nPosition = max( DWORD(0), min( m_nLength, m_nLength - nPosition ) );
	}
}

void CPacket::Shorten(DWORD nLength)
{
	m_nLength	= min( m_nLength, nLength );
	m_nPosition	= min( m_nPosition, m_nLength );
}

//////////////////////////////////////////////////////////////////////
// CPacket strings

CString CPacket::ReadString(DWORD nMaximum)
{
	CString strString;
	
	nMaximum = min( nMaximum, m_nLength - m_nPosition );
	if ( ! nMaximum ) return strString;
	
	LPCSTR pszInput	= (LPCSTR)m_pBuffer + m_nPosition;
	LPCSTR pszScan	= pszInput;

    DWORD nLength = 0;
	for ( ; nLength < nMaximum ; nLength++ )
	{
		m_nPosition++;
		if ( ! *pszScan++ ) break;
	}
	
	int nWide = MultiByteToWideChar( CP_ACP, 0, pszInput, nLength, NULL, 0 );
	MultiByteToWideChar( CP_ACP, 0, pszInput, nLength, strString.GetBuffer( nWide ), nWide );
	strString.ReleaseBuffer( nWide );

	return strString;
}

void CPacket::WriteString(LPCTSTR pszString, BOOL bNull)
{
	int nByte		= WideCharToMultiByte( CP_ACP, 0, pszString, -1, NULL, 0, NULL, NULL );
	LPSTR pszByte	= nByte <= PACKET_BUF_SCHAR ? m_szSCHAR : new CHAR[ nByte ];
	WideCharToMultiByte( CP_ACP, 0, pszString, -1, pszByte, nByte, NULL, NULL );
	Write( pszByte, nByte - ( bNull ? 0 : 1 ) );
	if ( pszByte != m_szSCHAR ) delete [] pszByte;
}

int CPacket::GetStringLen(LPCTSTR pszString) const
{
	if ( *pszString == 0 ) return 0;
	
	int nLength = _tcslen( pszString );
	
	nLength = WideCharToMultiByte( CP_ACP, 0, pszString, nLength, NULL, 0, NULL, NULL );
	
	return nLength;
}

//////////////////////////////////////////////////////////////////////
// CPacket UTF-8 strings

CString CPacket::ReadStringUTF8(DWORD nMaximum)
{
	CString strString;
	
	nMaximum = min( nMaximum, m_nLength - m_nPosition );
	if ( ! nMaximum ) return strString;
	
	LPCSTR pszInput	= (LPCSTR)m_pBuffer + m_nPosition;
	LPCSTR pszScan	= pszInput;

    DWORD nLength = 0;
	for ( ; nLength < nMaximum ; nLength++ )
	{
		m_nPosition++;
		if ( ! *pszScan++ ) break;
	}
	
	int nWide = MultiByteToWideChar( CP_UTF8, 0, pszInput, nLength, NULL, 0 );
	MultiByteToWideChar( CP_UTF8, 0, pszInput, nLength, strString.GetBuffer( nWide ), nWide );
	strString.ReleaseBuffer( nWide );

	return strString;
}

void CPacket::WriteStringUTF8(LPCTSTR pszString, BOOL bNull)
{
	int nByte		= WideCharToMultiByte( CP_UTF8, 0, pszString, -1, NULL, 0, NULL, NULL );
	LPSTR pszByte	= nByte <= PACKET_BUF_SCHAR ? m_szSCHAR : new CHAR[ nByte ];
	WideCharToMultiByte( CP_UTF8, 0, pszString, -1, pszByte, nByte, NULL, NULL );
	Write( pszByte, nByte - ( bNull ? 0 : 1 ) );
	if ( pszByte != m_szSCHAR ) delete [] pszByte;
}

int CPacket::GetStringLenUTF8(LPCTSTR pszString) const
{
	if ( *pszString == 0 ) return 0;
	
	int nLength = _tcslen( pszString );
	
	nLength = WideCharToMultiByte( CP_UTF8, 0, pszString, nLength, NULL, 0, NULL, NULL );
	
	return nLength;
}

//////////////////////////////////////////////////////////////////////
// CPacket ZLIB

LPBYTE CPacket::ReadZLib(DWORD nLength, DWORD* pnOutput, DWORD nSuggest)
{
	*pnOutput = 0;
	if ( m_nLength - m_nPosition < nLength ) return NULL;
	LPBYTE pOutput = CZLib::Decompress(	m_pBuffer + m_nPosition,
										nLength, (DWORD*)pnOutput, nSuggest );
	m_nPosition += nLength;
	return pOutput;
}

void CPacket::WriteZLib(LPCVOID pData, DWORD nLength)
{
	DWORD nOutput = 0;
	BYTE* pOutput = CZLib::Compress( pData, (DWORD)nLength, &nOutput );
	Write( pOutput, nOutput );
	delete [] pOutput;
}

//////////////////////////////////////////////////////////////////////
// CPacket pointer access

BYTE* CPacket::WriteGetPointer(DWORD nLength, DWORD nOffset)
{
	if ( nOffset == 0xFFFFFFFF ) nOffset = m_nLength;
	
	if ( m_nLength + nLength > m_nBuffer )
	{
		m_nBuffer += max( nLength, DWORD(PACKET_GROW) );
		LPBYTE pNew = new BYTE[ m_nBuffer ];
		CopyMemory( pNew, m_pBuffer, m_nLength );
		if ( m_pBuffer ) delete [] m_pBuffer;
		m_pBuffer = pNew;
	}
	
	if ( nOffset != m_nLength )
	{
		MoveMemory( m_pBuffer + nOffset + nLength, m_pBuffer + nOffset, m_nLength - nOffset );
	}
	
	m_nLength += nLength;
	
	return m_pBuffer + nOffset;
}

//////////////////////////////////////////////////////////////////////
// CPacket string conversion

LPCTSTR	CPacket::GetType() const
{
	return NULL;
}

CString CPacket::ToHex() const
{
	LPCTSTR pszHex = _T("0123456789ABCDEF");
	CString strDump;
	
	LPTSTR pszDump = strDump.GetBuffer( m_nLength * 3 );
	
	for ( DWORD i = 0 ; i < m_nLength ; i++ )
	{
		int nChar = m_pBuffer[i];
		if ( i ) *pszDump++ = ' ';
		*pszDump++ = pszHex[ nChar >> 4 ];
		*pszDump++ = pszHex[ nChar & 0x0F ];
	}
	
	*pszDump = 0;
	strDump.ReleaseBuffer();
	
	return strDump;
}

CString CPacket::ToASCII() const
{
	CString strDump;
	
	LPTSTR pszDump = strDump.GetBuffer( m_nLength + 1 );
	
	for ( DWORD i = 0 ; i < m_nLength ; i++ )
	{
		int nChar = m_pBuffer[i];
		*pszDump++ = ( nChar >= 32 ? nChar : '.' );
	}
	
	*pszDump = 0;
	strDump.ReleaseBuffer();
	
	return strDump;
}

//////////////////////////////////////////////////////////////////////
// CPacket debugging

void CPacket::Debug(LPCTSTR pszReason) const
{
}

//////////////////////////////////////////////////////////////////////
// CPacket smart dumping

void CPacket::SmartDump(CNeighbour* pNeighbour, IN_ADDR* pUDP, BOOL bOutgoing) const
{
	CSingleLock pLock( &theApp.m_pSection );
	
	if ( pLock.Lock( 50 ) )
	{
		if ( CMainWnd* pMainWnd = (CMainWnd*)theApp.m_pSafeWnd )
		{
			CWindowManager* pWindows	= &pMainWnd->m_pWindows;
			CPacketWnd* pWnd			= NULL;
			
			while ( pWnd = (CPacketWnd*)pWindows->Find( RUNTIME_CLASS(CPacketWnd), pWnd ) )
			{
				pWnd->Process( pNeighbour, pUDP, bOutgoing, this );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CPacket RAZA signatures

BOOL CPacket::GetRazaHash(SHA1* pHash, DWORD nLength) const
{
	if ( nLength == 0xFFFFFFFF ) nLength = m_nLength;
	if ( (DWORD)m_nLength < nLength ) return FALSE;
	
	CSHA pSHA;
	
	pSHA.Add( m_pBuffer, nLength );
	pSHA.Finish();
	pSHA.GetHash( pHash );
	
	return TRUE;
}

void CPacket::RazaSign()
{
}

BOOL CPacket::RazaVerify() const
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////
// CPacketPool construction

CPacketPool::CPacketPool()
{
	m_pFree = NULL;
	m_nFree = 0;
}

CPacketPool::~CPacketPool()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CPacketPool clear

void CPacketPool::Clear()
{
	for ( int nIndex = m_pPools.GetSize() - 1 ; nIndex >= 0 ; nIndex-- )
	{
		CPacket* pPool = (CPacket*)m_pPools.GetAt( nIndex );
		FreePoolImpl( pPool );
	}
	
	m_pPools.RemoveAll();
	m_pFree = NULL;
	m_nFree = 0;
}

//////////////////////////////////////////////////////////////////////
// CPacketPool new pool setup

void CPacketPool::NewPool()
{
	CPacket* pPool = NULL;
	int nPitch = 0, nSize = 256;
	
	NewPoolImpl( nSize, pPool, nPitch );
	m_pPools.Add( pPool );
	
	BYTE* pBytes = (BYTE*)pPool;
	
	while ( nSize-- > 0 )
	{
		pPool = (CPacket*)pBytes;
		pBytes += nPitch;
		
		pPool->m_pNext = m_pFree;
		m_pFree = pPool;
		m_nFree++;
	}
}
