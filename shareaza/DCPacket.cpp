//
// DCPacket.cpp
//
// Copyright (c) Shareaza Development Team, 2010-2014.
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
#include "Datagrams.h"
#include "DCPacket.h"
#include "DCClient.h"
#include "Network.h"
#include "QueryHit.h"
#include "Security.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDCPacket::CDCPacketPool CDCPacket::POOL;

CDCPacket::CDCPacket()
	: CPacket( PROTOCOL_DC )
{
}

CDCPacket::~CDCPacket()
{
}

CString CDCPacket::GetType() const
{
	if ( m_nLength )
	{
		if ( *m_pBuffer == '<' )
			return _T("Chat");
		else if ( *m_pBuffer == '|' )
			return _T("Ping");
		else if ( *m_pBuffer == '$' )
		{
			const BYTE* p = m_pBuffer;
			for ( DWORD n = m_nLength; n && *p != ' ' && *p != '|' ; ++p, --n );
			return CString( (LPCSTR)&m_pBuffer[ 1 ], (int)( p - m_pBuffer - 1 ) );
		}
	}
	return CString();
}

CString CDCPacket::ToHex() const
{
	const BYTE* p = m_pBuffer;
	DWORD n = m_nLength;
	if ( n )
	{
		if ( *p == '$' )
		{
			for ( ; n && *p != ' ' && *p != '|' ; ++p, --n );
			if ( n && *p == ' ' )
			{
				++p;
				--n;
			}
		}
	}
	if ( ! n )
		return CString();

	LPCTSTR pszHex = _T("0123456789ABCDEF");

	// Make a string and open it to write the characters in it directly, for speed
	CString strDump;
	LPTSTR pszDump = strDump.GetBuffer( n * 3 ); // Each byte will become 3 characters

	// Loop i down each byte in the packet
	for ( DWORD i = 0 ; i < n ; i++ )
	{
		// Copy the byte at i into an integer called nChar
		int nChar = p[i];

		// If this isn't the very start, write a space into the text
		if ( i ) *pszDump++ = ' '; // Write a space at pszDump, then move the pszDump pointer forward to the next character

		// Express the byte as two characters in the text, "00" through "FF"
		*pszDump++ = pszHex[ nChar >> 4 ];
		*pszDump++ = pszHex[ nChar & 0x0F ];
	}

	// Write a null terminator beyond the characters we wrote, close direct memory access to the string, and return it
	*pszDump = 0;
	strDump.ReleaseBuffer();
	return strDump;
}

CString CDCPacket::ToASCII() const
{
	const BYTE* p = m_pBuffer;
	DWORD n = m_nLength;
	if ( n )
	{
		if ( *p == '$' )
		{
			for ( ; n && *p != ' ' && *p != '|' ; ++p, --n );
			if ( n && *p == ' ' )
			{
				++p;
				--n;
			}
		}
	}
	if ( ! n )
		return CString();

	// Make a string and get direct access to its memory buffer
	CStringA strDump;
	LPSTR pszDump = strDump.GetBuffer( n + 1 ); // We'll write a character for each byte, and 1 more for the null terminator

	// Loop i down each byte in the packet
	for ( DWORD i = 0 ; i < n ; i++ )
	{
		// Copy the byte at i into an integer called nChar
		int nChar = p[i];

		// If the byte is 32 or greater, read it as an ASCII character and copy that character into the string
		*pszDump++ = CHAR( nChar >= 32 ? nChar : '.' ); // If it's 0-31, copy in a period instead
	}

	// Write a null terminator beyond the characters we wrote, close direct memory access to the string, and return it
	*pszDump = 0;
	strDump.ReleaseBuffer();
	return (LPCTSTR)CA2CT( (LPCSTR)strDump );
}

void CDCPacket::Reset()
{
	CPacket::Reset();
}

void CDCPacket::ToBuffer(CBuffer* pBuffer, bool /*bTCP*/)
{
	ASSERT( m_pBuffer && m_nLength );

	pBuffer->Add( m_pBuffer, m_nLength );
}

CDCPacket* CDCPacket::ReadBuffer(CBuffer* pBuffer)
{
	CDCPacket* pPacket = NULL;

	if ( pBuffer->m_nLength &&
		( pBuffer->m_pBuffer[ 0 ] == '$' ||
		  pBuffer->m_pBuffer[ 0 ] == '<' ||
		  pBuffer->m_pBuffer[ 0 ] == '|' ) )
	{
		for ( DWORD i = 0; i < pBuffer->m_nLength ; ++i )
		{
			if ( pBuffer->m_pBuffer[ i ] == '|' )
			{
				pPacket = CDCPacket::New( pBuffer->m_pBuffer, i + 1 );
				pBuffer->Remove( i + 1 );
				break;
			}
		}
	}

	return pPacket;
}

#ifdef _DEBUG

void CDCPacket::Debug(LPCTSTR pszReason) const
{
	CString strOutput;
	strOutput.Format( L"[DC++] %s ", pszReason );
	CPacket::Debug( strOutput );
}

#endif // _DEBUG

BOOL CDCPacket::OnPacket(const SOCKADDR_IN* pHost)
{
	SmartDump( pHost, TRUE, FALSE );

	if ( m_nLength > 4 && memcmp( m_pBuffer, "$SR ", 4 ) == 0 )
	{
		if ( ! OnCommonHit( pHost ) )
		{
			theApp.Message( MSG_ERROR, IDS_PROTOCOL_BAD_HIT,
				(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ) );
		}
		return TRUE;
	}
	else
	{
#ifdef _DEBUG
		CString tmp;
		tmp.Format( _T("Unknown packet from %s:%u."),
			(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ),
			htons( pHost->sin_port ) );
		Debug( tmp );
#endif // _DEBUG
	}

	return FALSE;
}

BOOL CDCPacket::OnCommonHit(const SOCKADDR_IN* /* pHost */)
{
	if ( CQueryHit* pHit = CQueryHit::FromDCPacket( this ) )
	{
		// Assume UDP is stable
		Datagrams.SetStable();

		Network.OnQueryHits( pHit );
	}

	return TRUE;
}
