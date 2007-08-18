//
// EDPacket.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

CEDPacket* CEDPacket::ReadBuffer(CBuffer* pBuffer, BYTE nEdProtocol)
{
	if ( pBuffer->m_nLength < sizeof( ED2K_TCP_HEADER ) ) return NULL;
	ED2K_TCP_HEADER* pHeader = reinterpret_cast<ED2K_TCP_HEADER*>(pBuffer->m_pBuffer);
	if ( pHeader->nProtocol != ED2K_PROTOCOL_EDONKEY &&
		 pHeader->nProtocol != ED2K_PROTOCOL_EMULE &&
		 pHeader->nProtocol != ED2K_PROTOCOL_EMULE_PACKED ) return NULL;
	if ( pBuffer->m_nLength - sizeof(*pHeader) + 1 < pHeader->nLength ) return NULL;
	CEDPacket* pPacket = CEDPacket::New( pHeader );
	pBuffer->Remove( sizeof(*pHeader) + pHeader->nLength - 1 );
	if ( pPacket->InflateOrRelease( nEdProtocol ) ) return NULL;
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

BOOL CEDPacket::InflateOrRelease(BYTE nEdProtocol)
{
	if ( m_nEdProtocol != ED2K_PROTOCOL_EMULE_PACKED &&
		 m_nEdProtocol != ED2K_PROTOCOL_KAD_PACKED &&
		 m_nEdProtocol != ED2K_PROTOCOL_REVCONNECT_PACKED )
		return FALSE;

	DWORD nOutput = 0;
	auto_array< BYTE > pOutput( CZLib::Decompress( m_pBuffer, m_nLength, &nOutput ) );

	if ( pOutput.get() != NULL )
	{
		m_nEdProtocol = nEdProtocol;

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

LPCTSTR CEDPacket::GetType() const
{
	static TCHAR szTypeBuffer[16];
	_stprintf( szTypeBuffer, _T("0x%x"), int( m_nType ) );
	return szTypeBuffer;

	/*
	for ( ED2K_PACKET_DESC* pType = m_pszTypes ; pType->nType ; pType++ )
	{
		if ( pType->nType == m_nType ) return pType->pszName;
	}

	return NULL;
	*/
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

CEDTag::CEDTag()
{
	m_nType = ED2K_TAG_NULL;
	m_nKey	= 0;
}

CEDTag::CEDTag(BYTE nKey)
{
	m_nType = ED2K_TAG_HASH;
	m_nKey	= nKey;
}

CEDTag::CEDTag(BYTE nKey, DWORD nValue)
{
	m_nType		= ED2K_TAG_INT;
	m_nKey		= nKey;
	m_nValue	= nValue;
}

CEDTag::CEDTag(BYTE nKey, LPCTSTR pszValue)
{
	m_nType		= ED2K_TAG_STRING;
	m_nKey		= nKey;
	m_sValue	= pszValue;
}

CEDTag::CEDTag(LPCTSTR pszKey)
{
	m_nType		= ED2K_TAG_HASH;
	m_sKey		= pszKey;
	m_nKey		= 0;
}

CEDTag::CEDTag(LPCTSTR pszKey, DWORD nValue)
{
	m_nType		= ED2K_TAG_INT;
	m_sKey		= pszKey;
	m_nKey		= 0;
	m_nValue	= nValue;
}

CEDTag::CEDTag(LPCTSTR pszKey, LPCTSTR pszValue)
{
	m_nType		= ED2K_TAG_STRING;
	m_sKey		= pszKey;
	m_nKey		= 0;
	m_sValue	= pszValue;
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
}

//////////////////////////////////////////////////////////////////////
// CEDTag write to packet

void CEDTag::Write(CEDPacket* pPacket, DWORD ServerFlags)
{
	BOOL bSmallTags = ServerFlags & ED2K_SERVER_TCP_SMALLTAGS, bUnicode = ServerFlags & ED2K_SERVER_TCP_UNICODE;
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
			if ( m_nValue <= 0xFF)
			{	// Correct type - to byte
				pPacket->m_pBuffer[nPos] = 0x80 | ED2K_TAG_UINT8;
				// Write a byte
				pPacket->WriteByte( (BYTE)m_nValue );
			}
			else if ( m_nValue <= 0xFFFF)
			{	// Correct type - to word
				pPacket->m_pBuffer[nPos] = 0x80 | ED2K_TAG_UINT16;
				// Write a word
				pPacket->WriteShortLE( (WORD)m_nValue );
			}
			else
			{	// Use a normal int
				pPacket->m_pBuffer[nPos] = 0x80 | ED2K_TAG_INT;
				// Write a DWORD
				pPacket->WriteLongLE( m_nValue );
			}
		}
		else
		{	// Use a normal int
			// Write a DWORD
			pPacket->WriteLongLE( m_nValue );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CEDTag read from packet

BOOL CEDTag::Read(CEDPacket* pPacket, DWORD ServerFlags)
{
	int nLen;

	Clear();

	if ( pPacket->GetRemaining() < 3 ) return FALSE;
	m_nType = pPacket->ReadByte();

	if ( m_nType & 0x80 )
	{	//This is a short tag (No length)

		//Remove the 0x80
		m_nType -= 0x80;
		// No length in packet
		nLen = 1;
	}
	else
	{	// Read in length
		nLen = pPacket->ReadShortLE();
	}

	if ( m_nType < ED2K_TAG_HASH || m_nType > ( ED2K_TAG_SHORTSTRING + 15 ) ) return FALSE;


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

	if ( m_nType == ED2K_TAG_STRING )					// Length prefixed string
	{
		if ( pPacket->GetRemaining() < 2 ) return FALSE;
		nLen = pPacket->ReadShortLE();
		if ( pPacket->GetRemaining() < nLen ) return FALSE;
		if ( ServerFlags & ED2K_SERVER_TCP_UNICODE )
			m_sValue = pPacket->ReadStringUTF8( nLen );
		else
			m_sValue = pPacket->ReadStringASCII( nLen );
	}
	else if ( m_nType == ED2K_TAG_INT )					// 32 bit integer
	{
		if ( pPacket->GetRemaining() < 4 ) return FALSE;
		m_nValue = pPacket->ReadLongLE();
	}
	else if ( m_nType == ED2K_TAG_FLOAT )				// 32 bit float
	{
		if ( pPacket->GetRemaining() < 4 ) return FALSE;
		m_nValue = pPacket->ReadLongLE();
	}
	else if ( m_nType == ED2K_TAG_UINT16 )				// 16 bit integer (New tag)
	{
		if ( pPacket->GetRemaining() < 2 ) return FALSE;
		m_nValue = pPacket->ReadShortLE();
	}
	else if ( m_nType == ED2K_TAG_UINT8 )				// 8 bit integer (New tag)
	{
		if ( pPacket->GetRemaining() < 1 ) return FALSE;
		m_nValue = pPacket->ReadByte();
	}
	else if ( ( m_nType >= ED2K_TAG_SHORTSTRING ) && ( m_nType <= ED2K_TAG_SHORTSTRING + 15 ) )
	{													// Short strings (New tag)
		nLen = ( m_nType - (ED2K_TAG_SHORTSTRING - 1) );//Calculate length of short string
		if ( pPacket->GetRemaining() < nLen ) return FALSE;

		if ( ServerFlags & ED2K_SERVER_TCP_UNICODE )
			m_sValue = pPacket->ReadStringUTF8( nLen );
		else
			m_sValue = pPacket->ReadStringASCII( nLen );
	}
	else if ( m_nType == ED2K_TAG_BLOB )				// ?
	{
		nLen = pPacket->ReadLongLE();
		m_sValue = pPacket->ReadStringASCII( nLen );
	}
	else
	{
		theApp.Message( MSG_DEBUG, _T("Unrecognised ed2k packet") );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDTag read from file

BOOL CEDTag::Read(CFile* pFile)
{
	Clear();

	if ( pFile->Read( &m_nType, sizeof(m_nType) ) != sizeof(m_nType) ) return FALSE;
	if ( m_nType < ED2K_TAG_HASH || m_nType > ED2K_TAG_INT ) return FALSE;

	WORD nLen;
	if ( pFile->Read( &nLen, sizeof(nLen) ) != sizeof(nLen) ) return FALSE;

	if ( nLen == 1 )
	{
		if ( pFile->Read( &m_nKey, sizeof(m_nKey) ) != sizeof(m_nKey) ) return FALSE;
	}
	else if ( nLen > 1 )
	{
		LPSTR psz = new CHAR[ nLen + 1 ];

		if ( psz == NULL )
		{
			theApp.Message( MSG_ERROR, _T("Memory allocation error in CEDTag::Read()") );
			return FALSE;
		}

		if ( pFile->Read( psz, nLen ) == nLen )
		{
			psz[ nLen ] = 0;
			m_sKey = psz;
			delete [] psz;
		}
		else
		{
			delete [] psz;
			return FALSE;
		}
	}

	if ( m_nType == ED2K_TAG_STRING )
	{
		if ( pFile->Read( &nLen, sizeof(nLen) ) != sizeof(nLen) ) return FALSE;

		LPSTR psz = new CHAR[ nLen + 1 ];

		if ( psz == NULL )
		{
			theApp.Message( MSG_ERROR, _T("Memory allocation error in CEDTag::Read()") );
			return FALSE;
		}

		if ( pFile->Read( psz, nLen ) == nLen )
		{
			int nWide = MultiByteToWideChar( CP_UTF8, 0, psz, nLen, NULL, 0 );
			MultiByteToWideChar( CP_UTF8, 0, psz, nLen, m_sValue.GetBuffer( nWide ), nWide );
			m_sValue.ReleaseBuffer( nWide );

			delete [] psz;
		}
		else
		{
			delete [] psz;
			return FALSE;
		}
	}
	else if ( m_nType == ED2K_TAG_INT )
	{
		if ( pFile->Read( &m_nValue, sizeof(m_nValue) ) != sizeof(m_nValue) ) return FALSE;
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
