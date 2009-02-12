//
// EDPacket.cpp
//
// Copyright © Shareaza Development Team, 2002-2009.
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
#include "EDPacket.h"
#include "Buffer.h"
#include "ZLib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CEDPacket::CEDPacketPool CEDPacket::POOL;


//////////////////////////////////////////////////////////////////////
// CEDPacket construction

CEDPacket::CEDPacket() : CPacket( PROTOCOL_ED2K )
{
}

CEDPacket::~CEDPacket()
{
}

//////////////////////////////////////////////////////////////////////
// CEDPacket length prefixed strings

CString CEDPacket::ReadEDString(DWORD ServerFlags)
{
	int nLen = ReadShortLE();
	if ( ServerFlags & ED2K_SERVER_TCP_UNICODE )
		return ReadStringUTF8( nLen );
	else
		return ReadStringASCII( nLen );
}

void CEDPacket::WriteEDString(LPCTSTR psz, DWORD ServerFlags)
{
	int nLen;
	if ( ServerFlags & ED2K_SERVER_TCP_UNICODE )
	{
		nLen = GetStringLenUTF8( psz );
		WriteShortLE( WORD( nLen ) );
		WriteStringUTF8( psz, FALSE );
	}
	else
	{
		nLen = GetStringLen( psz );
		WriteShortLE( WORD( nLen ) );
		WriteString( psz, FALSE );
	}
	ASSERT( nLen <= 0xFFFF );
}

CString CEDPacket::ReadEDString(BOOL bUnicode)
{
	int nLen = ReadShortLE();
	if ( bUnicode )
		return ReadStringUTF8( nLen );
	else
		return ReadStringASCII( nLen );
}

void CEDPacket::WriteEDString(LPCTSTR psz, BOOL bUnicode)
{
	int nLen;
	if ( bUnicode )
	{
		nLen = GetStringLenUTF8( psz );
		WriteShortLE( WORD( nLen ) );
		WriteStringUTF8( psz, FALSE );
	}
	else
	{
		nLen = GetStringLen( psz );
		WriteShortLE( WORD( nLen ) );
		WriteString( psz, FALSE );
	}
	ASSERT( nLen <= 0xFFFF );
}


CString CEDPacket::ReadLongEDString(BOOL bUnicode)
{
	DWORD nLen = ReadLongLE();
	if ( bUnicode )
		return ReadStringUTF8( nLen );
	else
		return ReadStringASCII( nLen );
}

void CEDPacket::WriteLongEDString(LPCTSTR psz, BOOL bUnicode)
{
	DWORD nLen;
	if ( bUnicode )
	{
		nLen = GetStringLenUTF8( psz );
		WriteLongLE( nLen );
		WriteStringUTF8( psz, FALSE );
	}
	else
	{
		nLen = GetStringLen( psz );
		WriteLongLE( nLen );
		WriteString( psz, FALSE );
	}
	ASSERT( nLen <= 0xFFFF );
}

//////////////////////////////////////////////////////////////////////
// CEDPacket buffers

void CEDPacket::ToBuffer(CBuffer* pBuffer) const
{
	ED2K_TCP_HEADER pHeader;
	pHeader.nProtocol	= m_nEdProtocol;
	pHeader.nLength		= m_nLength + 1;
	pHeader.nType		= m_nType;
	pBuffer->Add( &pHeader, sizeof(pHeader) );
	pBuffer->Add( m_pBuffer, m_nLength );
}

void CEDPacket::ToBufferUDP(CBuffer* pBuffer) const
{
	ED2K_UDP_HEADER pHeader;
	pHeader.nProtocol	= m_nEdProtocol;
	pHeader.nType		= m_nType;
	pBuffer->Add( &pHeader, sizeof(pHeader) );
	pBuffer->Add( m_pBuffer, m_nLength );
}

CEDPacket* CEDPacket::ReadBuffer(CBuffer* pBuffer)
{
	if ( pBuffer->m_nLength < sizeof( ED2K_TCP_HEADER ) ) return NULL;
	ED2K_TCP_HEADER* pHeader = reinterpret_cast<ED2K_TCP_HEADER*>(pBuffer->m_pBuffer);
	if ( pHeader->nProtocol != ED2K_PROTOCOL_EDONKEY &&
		 pHeader->nProtocol != ED2K_PROTOCOL_EMULE &&
		 pHeader->nProtocol != ED2K_PROTOCOL_EMULE_PACKED ) return NULL;
	if ( pBuffer->m_nLength - sizeof(*pHeader) + 1 < pHeader->nLength ) return NULL;
	CEDPacket* pPacket = CEDPacket::New( pHeader );
	pBuffer->Remove( sizeof(*pHeader) + pHeader->nLength - 1 );
	if ( pPacket->InflateOrRelease() ) return NULL;
	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CEDPacket compression

BOOL CEDPacket::Deflate()
{
	if ( m_nEdProtocol != ED2K_PROTOCOL_EDONKEY &&
		 m_nEdProtocol != ED2K_PROTOCOL_EMULE ) return FALSE;

	DWORD nOutput = 0;
	auto_array< BYTE > pOutput( CZLib::Compress( m_pBuffer, m_nLength, &nOutput ) );

	if ( pOutput.get() == NULL )
		return FALSE;

	if ( nOutput >= m_nLength )
		return FALSE;

	m_nEdProtocol = ED2K_PROTOCOL_EMULE_PACKED;

	memcpy( m_pBuffer, pOutput.get(), nOutput );
	m_nLength = nOutput;

	return TRUE;
}

BOOL CEDPacket::InflateOrRelease()
{
	if ( m_nEdProtocol != ED2K_PROTOCOL_EMULE_PACKED &&
		 m_nEdProtocol != ED2K_PROTOCOL_KAD_PACKED &&
		 m_nEdProtocol != ED2K_PROTOCOL_REVCONNECT_PACKED )
		return FALSE;

	DWORD nOutput = 0;
	auto_array< BYTE > pOutput( CZLib::Decompress( m_pBuffer, m_nLength, &nOutput ) );

	if ( pOutput.get() != NULL )
	{
		switch ( m_nEdProtocol )
		{
		case ED2K_PROTOCOL_EMULE_PACKED:
			m_nEdProtocol = ED2K_PROTOCOL_EMULE;
			break;
		case ED2K_PROTOCOL_KAD_PACKED:
			m_nEdProtocol = ED2K_PROTOCOL_KAD;
			break;
		case ED2K_PROTOCOL_REVCONNECT_PACKED:
			m_nEdProtocol = ED2K_PROTOCOL_REVCONNECT;
			break;
		}

		if ( m_nBuffer >= nOutput )
		{
			CopyMemory( m_pBuffer, pOutput.get(), nOutput );
			m_nLength = nOutput;
		}
		else
		{
			delete [] m_pBuffer;
			m_pBuffer = pOutput.release();
			m_nLength = nOutput;
			m_nBuffer = nOutput;
		}

		return FALSE;
	}
	else
	{
		Release();
		return TRUE;
	}
}

//////////////////////////////////////////////////////////////////////
// CEDPacket debug tools

CString CEDPacket::GetType() const
{
	for ( ED2K_PACKET_DESC* pType = m_pszTypes ; pType->nType ; pType++ )
	{
		if ( pType->nType == m_nType ) return pType->pszName;
	}

	CString tmp;
	tmp.Format( _T("0x%x"), int( m_nType ) );
	return tmp;
}

void CEDPacket::Debug(LPCTSTR pszReason) const
{
#ifdef _DEBUG
	if ( m_nType == ED2K_C2C_SENDINGPART ) return;
	if ( m_nType == ED2K_C2C_HASHSETANSWER ) return;
	if ( m_nType == ED2K_C2C_COMPRESSEDPART ) return;

	CString strOutput;
	strOutput.Format( L"[ED2K] %s Proto: 0x%x Type: %s", pszReason, int( m_nEdProtocol ), GetType() );
	CPacket::Debug( strOutput );
#else
	pszReason;
#endif
}


//////////////////////////////////////////////////////////////////////
// CEDTag construction

CEDTag::CEDTag() :
	m_nType ( ED2K_TAG_NULL ),
	m_nKey	( 0 )
{
}

CEDTag::CEDTag(BYTE nKey, const Hashes::Ed2kHash& oHash) :
	m_nType ( ED2K_TAG_HASH ),
	m_nKey	( nKey ),
	m_oValue( oHash )
{
}

CEDTag::CEDTag(BYTE nKey, QWORD nValue) :
	m_nType	( ED2K_TAG_INT ),
	m_nKey	( nKey ),
	m_nValue( nValue )
{
}

CEDTag::CEDTag(BYTE nKey, LPCTSTR pszValue) :
	m_nType	( ED2K_TAG_STRING ),
	m_nKey	( nKey ),
	m_sValue( pszValue )
{
}

CEDTag::CEDTag(LPCTSTR pszKey, QWORD nValue) :
	m_nType	( ED2K_TAG_INT ),
	m_sKey	( pszKey ),
	m_nKey	( 0 ),
	m_nValue( nValue )
{
}

CEDTag::CEDTag(LPCTSTR pszKey, LPCTSTR pszValue) :
	m_nType	( ED2K_TAG_STRING ),
	m_sKey	( pszKey ),
	m_nKey	( 0 ),
	m_sValue( pszValue )
{
}

//////////////////////////////////////////////////////////////////////
// CEDTag operations

void CEDTag::Clear()
{
	m_nType		= ED2K_TAG_NULL;
	m_nKey		= 0;
	m_nValue	= 0;
	m_sKey.Empty();
	m_sValue.Empty();
	m_oValue.clear();
}

//////////////////////////////////////////////////////////////////////
// CEDTag write to packet

void CEDTag::Write(CEDPacket* pPacket, DWORD ServerFlags)
{
	BOOL bSmallTags = ServerFlags & ED2K_SERVER_TCP_SMALLTAGS;
	BOOL bUnicode = ServerFlags & ED2K_SERVER_TCP_UNICODE;
	DWORD nPos = pPacket->m_nLength;

	pPacket->WriteByte( m_nType );

	if ( int nKeyLen = m_sKey.GetLength() )
	{
		pPacket->WriteEDString( m_sKey, ServerFlags );
	}
	else
	{
		if ( ! bSmallTags ) pPacket->WriteShortLE( 1 );
		pPacket->WriteByte( m_nKey );
	}

	if ( m_nType == ED2K_TAG_STRING )
	{
		if ( bSmallTags )// If we're supporting small tags
		{
			int nLength;
			if ( bUnicode )
				nLength = pPacket->GetStringLenUTF8( m_sValue );
			else
				nLength = pPacket->GetStringLen( m_sValue );

			if ( ( nLength <= 16 ) )
			{
				// We should use a 'short' string tag
				// Correct the packet type
				pPacket->m_pBuffer[nPos] = BYTE( 0x80 | ( ( ED2K_TAG_SHORTSTRING - 1 ) + nLength ) );

				// Write the string
				if ( bUnicode ) pPacket->WriteStringUTF8( m_sValue, FALSE );
				else pPacket->WriteString( m_sValue, FALSE );
			}
			else
			{	// We should use a normal string tag
				// Correct the packet type
				pPacket->m_pBuffer[nPos] = 0x80 | ED2K_TAG_STRING ;

				// Write the string
				pPacket->WriteEDString( m_sValue, ServerFlags );
			}
		}
		else
		{
			// Write the string
			pPacket->WriteEDString( m_sValue, ServerFlags );
		}
	}
	else if ( m_nType == ED2K_TAG_INT )
	{
		if ( bSmallTags )// If we're supporting small tags
		{	// Use a short tag
			if ( m_nValue <= 0xFF )
			{	// Use a 8 bit int
				pPacket->m_pBuffer[nPos] = 0x80 | ED2K_TAG_UINT8;
				// Write a byte
				pPacket->WriteByte( (BYTE)m_nValue );
			}
			else if ( m_nValue <= 0xFFFF )
			{	// Use a 16 bit int
				pPacket->m_pBuffer[nPos] = 0x80 | ED2K_TAG_UINT16;
				// Write a word
				pPacket->WriteShortLE( (WORD)m_nValue );
			}
			else if ( m_nValue <= 0xFFFFFFFF )
			{	// Use a 32 bit int
				pPacket->m_pBuffer[nPos] = 0x80 | ED2K_TAG_INT;
				// Write a DWORD
				pPacket->WriteLongLE( (DWORD)m_nValue );
			}
			else
			{	// Use a 64 bit int
				pPacket->m_pBuffer[nPos] = 0x80 | ED2K_TAG_UINT64;
				// Write a QWORD
				pPacket->WriteInt64( m_nValue );
			}
		}
		else
		{	// Use a normal int
			ASSERT( m_nValue <= 0xFFFFFFFF );
			// Write a DWORD
			pPacket->WriteLongLE( (DWORD)m_nValue );
		}
	}
	else
	{
		ASSERT( FALSE ); // Unsupported tag
	}
}

//////////////////////////////////////////////////////////////////////
// CEDTag read from packet

BOOL CEDTag::Read(CEDPacket* pPacket, DWORD ServerFlags)
{
	WORD nLen;

	Clear();

	if ( pPacket->GetRemaining() < 3 ) return FALSE;

	m_nType = pPacket->ReadByte();

	if ( m_nType & 0x80 )
	{
		m_nType &= 0x7F;
		nLen = 1;
	}
	else
	{
		nLen = pPacket->ReadShortLE();
	}

	if ( pPacket->GetRemaining() < nLen ) return FALSE;

	if ( nLen == 1 )
	{
		m_nKey = pPacket->ReadByte();
	}
	else if ( nLen > 1 )
	{
		if ( ServerFlags & ED2K_SERVER_TCP_UNICODE )
			m_sKey = pPacket->ReadStringUTF8( nLen );
		else
			m_sKey = pPacket->ReadStringASCII( nLen );
	}

	switch ( m_nType )
	{
	case ED2K_TAG_HASH:
		if ( pPacket->GetRemaining() < 16 ) return FALSE;
		pPacket->Read( &m_oValue[ 0 ], Hashes::Ed2kHash::byteCount );
		m_oValue.validate();
		break;

	case ED2K_TAG_STRING:
		if ( pPacket->GetRemaining() < 2 ) return FALSE;
		nLen = pPacket->ReadShortLE();
		if ( pPacket->GetRemaining() < nLen ) return FALSE;
		if ( ServerFlags & ED2K_SERVER_TCP_UNICODE )
			m_sValue = pPacket->ReadStringUTF8( nLen );
		else
			m_sValue = pPacket->ReadStringASCII( nLen );
		break;

	case ED2K_TAG_BLOB:
		if ( pPacket->GetRemaining() < 4 ) return FALSE;
		{
			DWORD nLenBlob = pPacket->ReadLongLE();
			if ( pPacket->GetRemaining() < nLenBlob ) return FALSE;
			m_sValue = pPacket->ReadStringASCII( nLenBlob );
		}
		break;

	case ED2K_TAG_INT:
		if ( pPacket->GetRemaining() < 4 ) return FALSE;
		m_nValue = pPacket->ReadLongLE();
		break;

	case ED2K_TAG_FLOAT:
		if ( pPacket->GetRemaining() < 4 ) return FALSE;
		m_nValue = pPacket->ReadLongLE();
		break;

	case ED2K_TAG_UINT16:
		if ( pPacket->GetRemaining() < 2 ) return FALSE;
		m_nValue = pPacket->ReadShortLE();
		m_nType = ED2K_TAG_INT;
		break;

	case ED2K_TAG_UINT8:
		if ( pPacket->GetRemaining() < 1 ) return FALSE;
		m_nValue = pPacket->ReadByte();
		m_nType = ED2K_TAG_INT;
		break;

	case ED2K_TAG_UINT64:
		if ( pPacket->GetRemaining() < 1 ) return FALSE;
		m_nValue = pPacket->ReadInt64();
		m_nType = ED2K_TAG_INT;
		break;

	default:
		if ( m_nType >= ED2K_TAG_SHORTSTRING && m_nType <= ED2K_TAG_SHORTSTRING + 15 )
		{
			nLen = m_nType - ( ED2K_TAG_SHORTSTRING - 1 );
			m_nType = ED2K_TAG_STRING;
			if ( pPacket->GetRemaining() < nLen ) return FALSE;
			if ( ServerFlags & ED2K_SERVER_TCP_UNICODE )
				m_sValue = pPacket->ReadStringUTF8( nLen );
			else
				m_sValue = pPacket->ReadStringASCII( nLen );
		}
		else
		{
			theApp.Message( MSG_DEBUG, _T("Unrecognised ED2K tag type 0x%02x"), m_nType );
			return FALSE;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDTag read from file

BOOL CEDTag::Read(CFile* pFile)
{
	WORD nLen;

	Clear();

	if ( pFile->Read( &m_nType, sizeof(m_nType) ) != sizeof(m_nType) )
		return FALSE;

	if ( m_nType & 0x80 )
	{
		m_nType &= 0x7F;
		nLen = 1;
	}
	else
	{
		if ( pFile->Read( &nLen, sizeof(nLen) ) != sizeof(nLen) )
			return FALSE;
	}

	if ( nLen == 1 )
	{
		if ( pFile->Read( &m_nKey, sizeof(m_nKey) ) != sizeof(m_nKey) )
			return FALSE;
	}
	else if ( nLen > 1 )
	{
		auto_array< CHAR > psz( new CHAR[ nLen + 1 ] );
		if ( ! psz.get() )
			return FALSE;
		if ( pFile->Read( psz.get(), nLen ) != nLen )
			return FALSE;
		psz[ nLen ] = 0;
		m_sKey = psz.get();
	}

	switch ( m_nType )
	{
	case ED2K_TAG_HASH:
		if ( pFile->Read( &m_oValue[ 0 ], Hashes::Ed2kHash::byteCount ) != Hashes::Ed2kHash::byteCount )
			return FALSE;
		m_oValue.validate();
		break;

	case ED2K_TAG_STRING:
		if ( pFile->Read( &nLen, sizeof(nLen) ) != sizeof(nLen) )
			return FALSE;
		{
			auto_array< CHAR > psz( new CHAR[ nLen ] );
			if ( ! psz.get() )
				return FALSE;
			if ( pFile->Read( psz.get(), nLen ) != nLen )
				return FALSE;
			m_sValue = UTF8Decode( psz.get(), nLen );
		}
		break;

	case ED2K_TAG_BLOB:
		{
			DWORD nBlolbLen;
			if ( pFile->Read( &nBlolbLen, sizeof(nBlolbLen) ) != sizeof(nBlolbLen) )
				return FALSE;
			auto_array< CHAR > psz( new CHAR[ nBlolbLen ] );
			if ( ! psz.get() )
				return FALSE;
			if ( pFile->Read( psz.get(), nBlolbLen ) != nBlolbLen )
				return FALSE;
			CopyMemory( m_sValue.GetBuffer( ( nBlolbLen + 1 ) / sizeof( TCHAR ) ), psz.get(), nBlolbLen );
			m_sValue.ReleaseBuffer( ( nBlolbLen + 1 ) / sizeof( TCHAR ) );
		}
		break;

	case ED2K_TAG_INT:
	case ED2K_TAG_FLOAT:
		{
			DWORD nValue;
			if ( pFile->Read( &nValue, sizeof(nValue) ) != sizeof(nValue) )
				return FALSE;
			m_nValue = nValue;
		}
		break;

	case ED2K_TAG_UINT16:
		{
			WORD nValue;
			if ( pFile->Read( &nValue, sizeof(nValue) ) != sizeof(nValue) )
				return FALSE;
			m_nValue = nValue;
			m_nType = ED2K_TAG_INT;
		}
		break;

	case ED2K_TAG_UINT8:
		{
			BYTE nValue;
			if ( pFile->Read( &nValue, sizeof(nValue) ) != sizeof(nValue) )
				return FALSE;
			m_nValue = nValue;
			m_nType = ED2K_TAG_INT;
		}
		break;

	case ED2K_TAG_UINT64:
		if ( pFile->Read( &m_nValue, sizeof(m_nValue) ) != sizeof(m_nValue) )
			return FALSE;
		m_nType = ED2K_TAG_INT;
		break;

	default:
		if ( m_nType >= ED2K_TAG_SHORTSTRING && m_nType <= ED2K_TAG_SHORTSTRING + 15 )
		{
			// Calculate length of short string
			nLen = m_nType - ( ED2K_TAG_SHORTSTRING - 1 );
			m_nType = ED2K_TAG_STRING;
			auto_array< CHAR > psz( new CHAR[ nLen ] );
			if ( ! psz.get() )
				return FALSE;
			if ( pFile->Read( psz.get(), nLen ) != nLen )
				return FALSE;
			m_sValue = UTF8Decode( psz.get(), nLen );
		}
		else
		{
			theApp.Message( MSG_DEBUG, _T("Unrecognised ED2K tag type 0x%02x"), m_nType );
			return FALSE;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDPacket packet type index

ED2K_PACKET_DESC CEDPacket::m_pszTypes[] =
{
	{ ED2K_C2S_LOGINREQUEST,		_T("Login") },
	{ ED2K_C2S_GETSERVERLIST,		_T("GetServerList") },
	{ ED2K_C2S_OFFERFILES,			_T("ShareFiles") },
	{ ED2K_C2S_SEARCHREQUEST,		_T("Search") },
	{ ED2K_C2S_SEARCHUSER,			_T("Browse") },
	{ ED2K_C2S_GETSOURCES,			_T("FindSource") },
	{ ED2K_C2S_CALLBACKREQUEST,		_T("Push1") },
	{ ED2K_C2S_MORERESULTS,			_T("Continue") },
	{ ED2K_S2C_REJECTED,			_T("Rejected") },
	{ ED2K_S2C_SERVERMESSAGE,		_T("Message") },
	{ ED2K_S2C_IDCHANGE,			_T("ClientID") },
	{ ED2K_S2C_SERVERLIST,			_T("ServerList") },
	{ ED2K_S2C_SEARCHRESULTS,		_T("Results") },
	{ ED2K_S2C_FOUNDSOURCES,		_T("Sources") },
	{ ED2K_S2C_SERVERSTATUS,		_T("Status") },
	{ ED2K_S2C_SERVERIDENT,			_T("Identity") },
	{ ED2K_S2C_CALLBACKREQUESTED,	_T("Push2") },

	{ ED2K_C2SG_SERVERSTATUSREQUEST,_T("GetStatus") },
	{ ED2K_S2CG_SERVERSTATUS,		_T("Status") },
	{ ED2K_C2SG_SEARCHREQUEST,		_T("Search") },
	{ ED2K_S2CG_SEARCHRESULT,		_T("Result") },
	{ ED2K_C2SG_GETSOURCES,			_T("FindSource") },
	{ ED2K_S2CG_FOUNDSOURCES,		_T("Sources") },
	{ ED2K_C2SG_CALLBACKREQUEST,	_T("Push1") },

	{ ED2K_C2C_HELLO,				_T("Handshake1") },
	{ ED2K_C2C_HELLOANSWER,			_T("Handshake2") },
	{ ED2K_C2C_FILEREQUEST,			_T("FileRequest") },
	{ ED2K_C2C_FILEREQANSWER,		_T("FileName") },
	{ ED2K_C2C_FILENOTFOUND,		_T("FileNotFound") },
	{ ED2K_C2C_FILESTATUS,			_T("FileStatus") },
	{ ED2K_C2C_QUEUEREQUEST,		_T("EnqueueMe") },
	{ ED2K_C2C_QUEUERELEASE,		_T("DequeueMe") },
	{ ED2K_C2C_QUEUERANK,			_T("QueueRank") },
	{ ED2K_C2C_STARTUPLOAD,			_T("StartUpload") },
	{ ED2K_C2C_FINISHUPLOAD,		_T("UploadFinished") },
	{ ED2K_C2C_REQUESTPARTS,		_T("RequestFrag") },
	{ ED2K_C2C_SENDINGPART,			_T("Fragment") },
	{ ED2K_C2C_FILESTATUSREQUEST,	_T("GetStatus") },
	{ ED2K_C2C_HASHSETREQUEST,		_T("GetHashset") },
	{ ED2K_C2C_HASHSETANSWER,		_T("Hashset") },
	{ ED2K_C2C_ASKSHAREDFILES,		_T("Browse") },
	{ ED2K_C2C_ASKSHAREDFILESANSWER,_T("BrowseResp") },
	{ ED2K_C2C_MESSAGE,				_T("Message") },
	{ ED2K_C2C_REQUESTPREVIEW,		_T("RequestPreview") },
	{ ED2K_C2C_PREVIEWANWSER,		_T("Preview") },
	{ ED2K_C2C_COMPRESSEDPART,		_T("CompFragment") },
	{ ED2K_C2C_COMPRESSEDPART_I64,	_T("CompFragment64") },
	{ ED2K_C2C_REQUESTPARTS_I64,	_T("RequestFrag64") },
	{ ED2K_C2C_SENDINGPART_I64,		_T("Fragment64") },

	{ 0, NULL }
};
