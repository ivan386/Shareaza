//
// EDPacket.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2013.
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
#include "Buffer.h"
#include "EDClient.h"
#include "EDClients.h"
#include "EDNeighbour.h"
#include "EDPacket.h"
#include "Kademlia.h"
#include "Network.h"
#include "Schema.h"
#include "SharedFile.h"
#include "Statistics.h"
#include "XML.h"
#include "ZLib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CEDPacket::CEDPacketPool CEDPacket::POOL;


//////////////////////////////////////////////////////////////////////
// CEDPacket construction

CEDPacket::CEDPacket()
	: CPacket		( PROTOCOL_ED2K )
	, m_nEdProtocol	( 0 )
	, m_nType		( 0 )
	, m_bDeflate	( FALSE )
{
}

CEDPacket::~CEDPacket()
{
}

void CEDPacket::Reset()
{
	CPacket::Reset();

	m_nEdProtocol	= 0;
	m_nType			= 0;
	m_bDeflate		= FALSE;
}

//////////////////////////////////////////////////////////////////////
// CEDPacket length prefixed strings

CString CEDPacket::ReadEDString(BOOL bUnicode)
{
	WORD nLen = ReadShortLE();
	if ( bUnicode )
		return ReadStringUTF8( nLen );
	else
		return ReadStringASCII( nLen );
}

void CEDPacket::WriteEDString(LPCTSTR psz, BOOL bUnicode)
{
	if ( bUnicode )
	{
		WORD nLen = (WORD)GetStringLenUTF8( psz );
		WriteShortLE( nLen );
		WriteStringUTF8( psz, FALSE );
	}
	else
	{
		WORD nLen = (WORD)GetStringLen( psz );
		WriteShortLE( nLen );
		WriteString( psz, FALSE );
	}
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
	if ( bUnicode )
	{
		DWORD nLen = GetStringLenUTF8( psz );
		WriteLongLE( nLen );
		WriteStringUTF8( psz, FALSE );
	}
	else
	{
		DWORD nLen = GetStringLen( psz );
		WriteLongLE( nLen );
		WriteString( psz, FALSE );
	}
}

void CEDPacket::WriteFile(const CShareazaFile* pShareazaFile, QWORD nSize,
	const CEDClient* pClient, const CEDNeighbour* pServer, bool bPartial)
{
	ASSERT( ( pClient && ! pServer ) || ( ! pClient && pServer ) );

	const CLibraryFile* pFile = bPartial ?
		NULL : static_cast< const CLibraryFile* >( pShareazaFile );

	bool bDeflate = ( pServer && ( pServer->m_nTCPFlags & ED2K_SERVER_TCP_DEFLATE ) != 0 );
	bool bUnicode = ( pServer && ( pServer->m_nTCPFlags & ED2K_SERVER_TCP_UNICODE ) != 0 ) ||
		( pClient && pClient->m_bEmUnicode );
	bool bSmlTags = ( pServer && ( pServer->m_nTCPFlags & ED2K_SERVER_TCP_SMALLTAGS ) != 0 ) ||
		( pClient != NULL );

	// Send the file hash
	Write( pShareazaFile->m_oED2K );

	// Send client ID + port
	DWORD nClientID;
	WORD nClientPort;
	if ( pServer )
	{
		// If we have a 'new' ed2k server
		if ( bDeflate )
		{
			if ( bPartial )
			{
				// Partial file
				nClientID = 0xFCFCFCFC;
				nClientPort = 0xFCFC;
			}
			else
			{
				// Complete file
				nClientID = 0xFBFBFBFB;
				nClientPort = 0xFBFB;
			}
		}
		else
		{
			nClientID = pServer->GetID();
			nClientPort = ntohs( Network.m_pHost.sin_port );
		}
	}
	else
	{
		nClientID = pClient ? pClient->GetID() : 0;
		nClientPort = ntohs( Network.m_pHost.sin_port );
	}
	WriteLongLE( nClientID );
	WriteShortLE( nClientPort );

	// Load metadata
	CString strType, strTitle, strArtist, strAlbum, strCodec;
	DWORD nBitrate = 0, nLength = 0;
	BYTE nRating = 0;
	if ( bSmlTags && pFile )
	{
		if ( pFile->m_pSchema && pFile->m_pSchema->m_sDonkeyType.GetLength() )
		{
			// File type
			strType = pFile->m_pSchema->m_sDonkeyType;

			if ( pFile->m_pMetadata )
			{
				// Title
				if ( pFile->m_pMetadata->GetAttributeValue( _T("title") ).GetLength() )
					strTitle = pFile->m_pMetadata->GetAttributeValue( _T("title") );

				if ( pFile->IsSchemaURI( CSchema::uriAudio ) )
				{
					// Artist
					if ( pFile->m_pMetadata->GetAttributeValue( _T("artist") ).GetLength() )
						strArtist = pFile->m_pMetadata->GetAttributeValue( _T("artist") );

					// Album
					if ( pFile->m_pMetadata->GetAttributeValue( _T("album") ).GetLength() )
						strAlbum = pFile->m_pMetadata->GetAttributeValue( _T("album") );

					// Bitrate
					if ( pFile->m_pMetadata->GetAttributeValue( _T("bitrate") ).GetLength() )
						_stscanf( pFile->m_pMetadata->GetAttributeValue( _T("bitrate") ), _T("%i"), &nBitrate );

					// Length
					if ( pFile->m_pMetadata->GetAttributeValue( _T("seconds") ).GetLength() )
						_stscanf( pFile->m_pMetadata->GetAttributeValue( _T("seconds") ), _T("%i"), &nLength );
				}
				else if ( pFile->IsSchemaURI( CSchema::uriVideo ) )
				{
					// Codec
					if ( pFile->m_pMetadata->GetAttributeValue( _T("codec") ).GetLength() )
						strCodec = pFile->m_pMetadata->GetAttributeValue( _T("codec") );

					// Length
					if ( pFile->m_pMetadata->GetAttributeValue( _T("minutes") ).GetLength() )
					{
						double nMins = 0.0;
						_stscanf( pFile->m_pMetadata->GetAttributeValue( _T("minutes") ), _T("%lf"), &nMins );
						nLength = (DWORD)( nMins * (double)60 );	// Convert to seconds
					}
				}
			}
		}

		// File rating
		if ( pFile->m_nRating )
			nRating = (BYTE)min( pFile->m_nRating, 5 );
	}

	// Set the number of tags present
	DWORD nTags = 2; // File name and size are always present
	if ( nSize > MAX_SIZE_32BIT ) nTags++;
	if ( pClient ) nTags += 2; //3;
	if ( strType.GetLength() ) nTags++;
	if ( strTitle.GetLength() ) nTags++;
	if ( strArtist.GetLength() ) nTags++;
	if ( strAlbum.GetLength() ) nTags++;
	if ( nBitrate )	 nTags++;
	if ( nLength ) nTags++;
	if ( strCodec.GetLength() )  nTags++;
	if ( nRating )  nTags++;
	WriteLongLE( nTags );

	// Filename
	CEDTag( ED2K_FT_FILENAME, pShareazaFile->m_sName ).Write( this, bUnicode, bSmlTags );

	// File size
	CEDTag( ED2K_FT_FILESIZE, (DWORD)nSize ).Write( this, bUnicode, bSmlTags );
	if ( nSize > MAX_SIZE_32BIT )
		CEDTag( ED2K_FT_FILESIZE_HI,(DWORD)( nSize >> 32 ) ).Write( this, bUnicode, bSmlTags );

	// Sources
	//if ( pClient )
	//	CEDTag( ED2K_FT_SOURCES, 1ull ).Write( this, bUnicode, bSmlTags );

	// Complete sources
	if ( pClient )
		CEDTag( ED2K_FT_COMPLETE_SOURCES, 1ull ).Write( this, bUnicode, bSmlTags );

	// Last seen
	if ( pClient )
		CEDTag( ED2K_FT_LASTSEENCOMPLETE, 0ull ).Write( this, bUnicode, bSmlTags );

	// File type
	if ( strType.GetLength() )
		CEDTag( ED2K_FT_FILETYPE, strType ).Write( this, bUnicode, bSmlTags );

	// Title
	if ( strTitle.GetLength() )
		CEDTag( ED2K_FT_TITLE, strTitle ).Write( this, bUnicode, bSmlTags );
	
	// Artist
	if ( strArtist.GetLength() )
		CEDTag( ED2K_FT_ARTIST, strArtist ).Write( this, bUnicode, bSmlTags );

	// Album
	if ( strAlbum.GetLength() )
		CEDTag( ED2K_FT_ALBUM, strAlbum ).Write( this, bUnicode, bSmlTags );

	// Bitrate
	if ( nBitrate )
		CEDTag( ED2K_FT_BITRATE, nBitrate ).Write( this, bUnicode, bSmlTags );
	
	// Length
	if ( nLength )
		CEDTag( ED2K_FT_LENGTH, nLength ).Write( this, bUnicode, bSmlTags );
	
	// Codec
	if ( strCodec.GetLength() )
		CEDTag( ED2K_FT_CODEC, strCodec ).Write( this, bUnicode, bSmlTags );

	// File rating
	if ( nRating )
		CEDTag( ED2K_FT_FILERATING, nRating ).Write( this, bUnicode, bSmlTags );
}

//////////////////////////////////////////////////////////////////////
// CEDPacket buffers

void CEDPacket::ToBuffer(CBuffer* pBuffer, bool bTCP)
{
	// ZLIB compress if available
	if ( m_bDeflate )
	{
		m_bDeflate = FALSE;
		Deflate();
	}

	if ( bTCP )
	{
		ED2K_TCP_HEADER pHeader = { m_nEdProtocol, m_nLength + 1, m_nType };
		pBuffer->Add( &pHeader, sizeof(pHeader) );
		pBuffer->Add( m_pBuffer, m_nLength );
	}
	else
	{
		ED2K_UDP_HEADER pHeader = { m_nEdProtocol, m_nType };
		pBuffer->Add( &pHeader, sizeof(pHeader) );
		pBuffer->Add( m_pBuffer, m_nLength );
	}
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
	if ( pPacket->Inflate() )
		return pPacket;	
	pPacket->Release();
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CEDPacket compression

BOOL CEDPacket::Deflate()
{
	if ( m_nEdProtocol != ED2K_PROTOCOL_EDONKEY &&
		 m_nEdProtocol != ED2K_PROTOCOL_EMULE )
		 return FALSE;

	DWORD nOutput = 0;
	auto_array< BYTE > pOutput( CZLib::Compress( m_pBuffer, m_nLength, &nOutput ) );
	if ( ! pOutput.get() )
		return FALSE;

	if ( nOutput >= m_nLength )
		return FALSE;

	m_nEdProtocol = ED2K_PROTOCOL_EMULE_PACKED;

	delete [] m_pBuffer;
	m_pBuffer = pOutput.release();
	m_nLength = nOutput;
	m_nBuffer = nOutput;
	m_nPosition = 0;

	return TRUE;
}

BOOL CEDPacket::Inflate()
{
	if ( m_nEdProtocol != ED2K_PROTOCOL_EMULE_PACKED &&
		 m_nEdProtocol != ED2K_PROTOCOL_KAD_PACKED &&
		 m_nEdProtocol != ED2K_PROTOCOL_REVCONNECT_PACKED )
		return TRUE;

	DWORD nOutput = 0;
	auto_array< BYTE > pOutput( CZLib::Decompress( m_pBuffer, m_nLength, &nOutput ) );
	if ( ! pOutput.get() )
		return FALSE;

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

	delete [] m_pBuffer;
	m_pBuffer = pOutput.release();
	m_nLength = nOutput;
	m_nBuffer = nOutput;
	m_nPosition = 0;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDPacket debug tools

CString CEDPacket::GetType() const
{
	const BOOL bServer = ( m_nNeighbourUnique != NULL );
	for ( const ED2K_PACKET_DESC* pType = m_pszTypes ; pType->nType ; pType++ )
	{
		if ( pType->nType == m_nType && pType->bServer == bServer && ( pType->nProtocol == m_nEdProtocol || bServer ) )
			return pType->pszName;
	}

	CString tmp;
	tmp.Format( _T("0x%x"), int( m_nType ) );
	return tmp;
}

CString CEDPacket::ToASCII() const
{
	CString strOutput;

	const BOOL bServer = ( m_nNeighbourUnique != NULL );
	if ( bServer )
	{
		switch ( m_nType )
		{
		case ED2K_C2S_LOGINREQUEST:
		case ED2K_S2C_SERVERIDENT:
			{
				const int nLen = ( m_bOutgoing || m_nType == ED2K_S2C_SERVERIDENT ) ? 0 : 1;
				const Hashes::Guid oGUID( *(Hashes::Guid::RawStorage*)( m_pBuffer + nLen ));
				const IN_ADDR& nIP = *(IN_ADDR*)( m_pBuffer + nLen + Hashes::Guid::byteCount );
				const WORD& nPort = *(WORD*)( m_pBuffer + nLen + Hashes::Guid::byteCount + 4 );
				strOutput.Format( _T("guid: %s, ip: %s, port: %u, tags: %s"), oGUID.toString(), (LPCTSTR)CString( inet_ntoa( nIP ) ), nPort,
					CEDTag::ToString( m_pBuffer + nLen + Hashes::Guid::byteCount + 4 + 2, m_nLength - ( nLen + Hashes::Guid::byteCount + 4 + 2 ) ) );
			}
			break;

		case ED2K_C2S_GETSOURCES:
			{
				const Hashes::Ed2kHash oMD4( *(Hashes::Ed2kHash::RawStorage*)m_pBuffer );
				strOutput.Format( _T("hash: %s"), oMD4.toString() );
			}
			break;

		case ED2K_S2C_FOUNDSOURCES:
			{
				const Hashes::Ed2kHash oMD4( *(Hashes::Ed2kHash::RawStorage*)m_pBuffer );
				const BYTE& nCount = *(BYTE*)( m_pBuffer + Hashes::Ed2kHash::byteCount );
				strOutput.Format( _T("hash: %s, number: %u"), oMD4.toString(), nCount );
			}
			break;

		case ED2K_S2C_IDCHANGE:
			{
				const IN_ADDR& nIP = *(IN_ADDR*)( m_pBuffer );
				const DWORD& nFlags = *(DWORD*)( m_pBuffer + 4 );
				strOutput.Format( _T("new ip: %s, flags: %s"), (LPCTSTR)CString( inet_ntoa( nIP ) ), GetED2KServerFlags( nFlags ) );
			}
			break;

		case ED2K_S2C_SERVERMESSAGE:
			{
				const WORD& nLen = *(WORD*)( m_pBuffer );
				strOutput.Format( _T("\"%s\" (%u bytes)"), (LPCTSTR)UTF8Decode( (char*)m_pBuffer + 2, nLen ), nLen );
			}
			break;

		case ED2K_S2C_SEARCHRESULTS:
		case ED2K_C2S_OFFERFILES:
			{
				const DWORD& nCount = *(DWORD*)( m_pBuffer );
				strOutput.Format( _T("number: %u"), nCount );
			}
			break;

		case ED2K_S2C_SERVERSTATUS:
			{
				const DWORD& nUsers = *(DWORD*)( m_pBuffer );
				const DWORD& nFiles = *(DWORD*)( m_pBuffer + 4 );
				strOutput.Format( _T("users: %u, files: %u"), nUsers, nFiles );
			}
			break;
		}
	}
	else
	{
		if ( m_nEdProtocol == ED2K_PROTOCOL_EDONKEY )
		{
			switch ( m_nType )
			{
			case ED2K_C2C_HELLO:
			case ED2K_C2C_HELLOANSWER:
				{
					const int nLen = ( m_nType == ED2K_C2C_HELLOANSWER ) ? 0 : 1;
					const Hashes::Guid oGUID( *(Hashes::Guid::RawStorage*)(m_pBuffer + nLen) );
					const IN_ADDR& nIP = *(IN_ADDR*)( m_pBuffer + nLen + Hashes::Guid::byteCount );
					const WORD& nPort = *(WORD*)( m_pBuffer + nLen + Hashes::Guid::byteCount + 4 );
					strOutput.Format( _T("guid: %s, ip: %s, port: %u, tags: %s"), oGUID.toString(), (LPCTSTR)CString( inet_ntoa( nIP ) ), nPort,
						CEDTag::ToString( m_pBuffer + nLen + Hashes::Guid::byteCount + 4 + 2, m_nLength - ( nLen + Hashes::Guid::byteCount + 4 + 2 ) ) );
				}
				break;

			case ED2K_C2C_MESSAGE:
			case ED2K_C2C_VIEWSHAREDDIR:
			case ED2K_C2C_VIEWSHAREDDIRANSWER:
				{
					const WORD& nLen = *(WORD*)( m_pBuffer );
					strOutput.Format( _T("\"%s\" (%u bytes)"), (LPCTSTR)UTF8Decode( (char*)m_pBuffer + 2, nLen ), nLen );
				}
				break;

			case ED2K_C2C_FILEREQANSWER:
				{
					const Hashes::Ed2kHash oMD4( *(Hashes::Ed2kHash::RawStorage*)m_pBuffer );
					const WORD& nLen = *(WORD*)( m_pBuffer + Hashes::Ed2kHash::byteCount );
					strOutput.Format( _T("hash: %s, name: \"%s\" (%u bytes)"), oMD4.toString(), (LPCTSTR)UTF8Decode( (char*)m_pBuffer + Hashes::Ed2kHash::byteCount + 2, nLen ), nLen );
				}
				break;

			case ED2K_C2C_FILENOTFOUND:
			case ED2K_C2C_FILEREQUEST:
			case ED2K_C2C_QUEUEREQUEST:
			case ED2K_C2C_FILESTATUSREQUEST:
			case ED2K_C2C_HASHSETREQUEST:
				{
					const Hashes::Ed2kHash oMD4( *(Hashes::Ed2kHash::RawStorage*)m_pBuffer );
					strOutput.Format( _T("hash: %s"), oMD4.toString() );
				}
				break;

			case ED2K_C2C_FILESTATUS:
			case ED2K_C2C_HASHSETANSWER:
				{
					const Hashes::Ed2kHash oMD4( *(Hashes::Ed2kHash::RawStorage*)m_pBuffer );
					const WORD& nBlocks = *(const WORD*)( m_pBuffer + Hashes::Ed2kHash::byteCount );
					strOutput.Format( _T("hash: %s, blocks: %u"), oMD4.toString(), nBlocks );
				}
				break;

			case ED2K_C2C_ASKSHAREDFILESANSWER:
			case ED2K_C2C_ASKSHAREDDIRSANSWER:
			case ED2K_C2C_QUEUERANK:
				{
					const DWORD& nCount = *(const DWORD*)( m_pBuffer );
					strOutput.Format( _T("number: %u"), nCount );
				}
				break;

			case ED2K_C2C_REQUESTPARTS:
				{
					const Hashes::Ed2kHash oMD4( *(Hashes::Ed2kHash::RawStorage*)m_pBuffer );
					const DWORD* offset = (const DWORD*)( m_pBuffer + Hashes::Ed2kHash::byteCount );
					strOutput.Format( _T("hash: %s, offsets: %u-%u, %u-%u, %u-%u"), oMD4.toString(), offset[0], offset[3], offset[1], offset[4], offset[2], offset[5] );
				}
				break;

			case ED2K_C2C_SENDINGPART:
				{
					const Hashes::Ed2kHash oMD4( *(Hashes::Ed2kHash::RawStorage*)m_pBuffer );
					const DWORD* offset = (const DWORD*)( m_pBuffer + Hashes::Ed2kHash::byteCount );
					strOutput.Format( _T("hash: %s, offsets: %u, length: %u bytes"), oMD4.toString(), offset[0], offset[1] - offset[0] );
				}
				break;
			}
		}
		else if ( m_nEdProtocol == ED2K_PROTOCOL_EMULE )
		{
			switch ( m_nType )
			{
			case ED2K_C2C_QUEUERANKING:
				{
					const WORD& position = *(const WORD*)( m_pBuffer );
					const WORD& length = *(const WORD*)( m_pBuffer + 2 );
					strOutput.Format( _T("position: %u, length: %u"), position, length );
				}
				break;

			case ED2K_C2C_FILEDESC:
				{
					const Hashes::Ed2kHash oMD4( *(Hashes::Ed2kHash::RawStorage*)m_pBuffer );
					const WORD& nLen = *(WORD*)( m_pBuffer + 16 );
					strOutput.Format( _T("hash: %s, desc: \"%s\" (%u bytes)"), (LPCTSTR)UTF8Decode( (char*)m_pBuffer + 16 + 2, nLen ), nLen );
				}
				break;

			case ED2K_C2C_REQUESTSOURCES:
			case ED2K_C2C_REQUESTPREVIEW:
				{
					const Hashes::Ed2kHash oMD4( *(Hashes::Ed2kHash::RawStorage*)m_pBuffer );
					strOutput.Format( _T("hash: %s"), oMD4.toString() );
				}
				break;

			case ED2K_C2C_REQUESTPARTS_I64:
				{
					const Hashes::Ed2kHash oMD4( *(Hashes::Ed2kHash::RawStorage*)m_pBuffer );
					const QWORD* offset = (const QWORD*)( m_pBuffer + Hashes::Ed2kHash::byteCount );
					strOutput.Format( _T("hash: %s, offsets: %I64u-%I64u, %I64u-%I64u, %I64u-%I64u"), oMD4.toString(), offset[0], offset[3], offset[1], offset[4], offset[2], offset[5] );
				}
				break;

			case ED2K_C2C_COMPRESSEDPART:
				{
					const Hashes::Ed2kHash oMD4( *(Hashes::Ed2kHash::RawStorage*)m_pBuffer );
					const DWORD& offset = *(const DWORD*)( m_pBuffer + Hashes::Ed2kHash::byteCount );
					const DWORD& length = *(const DWORD*)( m_pBuffer + Hashes::Ed2kHash::byteCount + 4 );
					strOutput.Format( _T("hash: %s, offset: %u, length: %u bytes"), oMD4.toString(), offset, length );
				}
				break;

			case ED2K_C2C_COMPRESSEDPART_I64:
				{
					const Hashes::Ed2kHash oMD4( *(Hashes::Ed2kHash::RawStorage*)m_pBuffer );
					const QWORD& offset = *(const QWORD*)( m_pBuffer + Hashes::Ed2kHash::byteCount );
					const DWORD& length = *(const DWORD*)( m_pBuffer + Hashes::Ed2kHash::byteCount + 8 );
					strOutput.Format( _T("hash: %s, offset: %I64u, length: %u bytes"), oMD4.toString(), offset, length );
				}
				break;

			case ED2K_C2C_SENDINGPART_I64:
				{
					const Hashes::Ed2kHash oMD4( *(Hashes::Ed2kHash::RawStorage*)m_pBuffer );
					const QWORD* offset = (const QWORD*)( m_pBuffer + Hashes::Ed2kHash::byteCount );
					strOutput.Format( _T("hash: %s, offsets: %I64u, length: %I64u bytes"), oMD4.toString(), offset[0], offset[1] - offset[0] );
				}
				break;
			}
		}
	}

	return strOutput.IsEmpty() ? CPacket::ToASCII() :strOutput;
}

#ifdef _DEBUG

void CEDPacket::Debug(LPCTSTR pszReason) const
{
	if ( m_nType == ED2K_C2C_SENDINGPART ) return;
	if ( m_nType == ED2K_C2C_HASHSETANSWER ) return;
	if ( m_nType == ED2K_C2C_COMPRESSEDPART ) return;

	CString strOutput;
	strOutput.Format( L"[ED2K] %s Proto: 0x%x Type: %s", pszReason, int( m_nEdProtocol ), (LPCTSTR)GetType() );
	CPacket::Debug( strOutput );
}

#endif // _DEBUG

//////////////////////////////////////////////////////////////////////
// CEDTag construction

CEDTag::CEDTag() :
	m_nType ( ED2K_TAG_NULL ),
	m_nKey	( 0 )
{
}

CEDTag::CEDTag(const CEDTag& t)
	: m_nType( t.m_nType )
	, m_sKey( t.m_sKey )
	, m_nKey( t.m_nKey )
	, m_sValue( t.m_sValue )
	, m_nValue( t.m_nValue )
	, m_oValue( t.m_oValue )
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

CEDTag& CEDTag::operator=(const CEDTag& t)
{
	m_nType = t.m_nType;
	m_sKey = t.m_sKey;
	m_nKey = t.m_nKey;
	m_sValue = t.m_sValue;
	m_nValue = t.m_nValue;
	m_oValue = t.m_oValue;
	return *this;
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

CString CEDTag::ToString() const
{
	CString strOutput;

	strOutput.Format( _T("[0x%02x]{"), m_nType );

	if ( m_sKey.IsEmpty() )
	{
		switch ( m_nKey )
		{
		case 0x01:
			strOutput += _T("name");
			break;

		case 0x0B:
			strOutput += _T("info");
			break;

		case 0x11:
			strOutput += _T("version");
			break;

		case 0xF9:
			strOutput += _T("ports");
			break;

		case 0xFA:
			strOutput += _T("features1");
			break;

		case 0xFE:
			strOutput += _T("features2");
			break;

		case 0xFB:
			strOutput += _T("client");
			break;

		case 0x20:
			strOutput += _T("flags");
			break;

		default:
			strOutput.AppendFormat( _T("0x%02x"), m_nKey );
		}
	}
	else
		strOutput +=  m_sKey;

	strOutput += _T('=');

	switch ( m_nType )
	{
	case ED2K_TAG_HASH:
		strOutput += m_oValue.toString();
		break;

	case ED2K_TAG_STRING:
		strOutput += _T('\"') + m_sValue + _T('\"');
		break;

	case ED2K_TAG_INT:
		if ( m_nKey == 0xFB )
			strOutput.AppendFormat( _T("%u.%u.%u.%u"), (BYTE)( m_nValue >> 17 ) & 0x7f, (BYTE)( m_nValue >> 10 ) & 0x7f, (BYTE)( m_nValue >> 7 ) & 0x7, (BYTE)m_nValue & 0x7f );
		else
			strOutput.AppendFormat( _T("0x%I64x"), m_nValue );
		break;
	}

	strOutput += _T("} ");

	return strOutput;
}

CString CEDTag::ToString(const BYTE* pData, DWORD nLength)
{
	CString strOutput;
	if ( pData && nLength )
	{
		if ( CEDPacket* pTempPacket = CEDPacket::New( (BYTE)0 ) )
		{
			pTempPacket->Write( pData, nLength );
			for ( DWORD nTags = pTempPacket->ReadLongLE(); nTags > 0 && pTempPacket->GetRemaining() > 1; --nTags )
			{
				CEDTag pTag;
				if ( ! pTag.Read( pTempPacket, TRUE ) )
					break;
				strOutput += pTag.ToString();
			}
			pTempPacket->Release();
		}
	}
	return strOutput;
}

//////////////////////////////////////////////////////////////////////
// CEDTag write to packet

void CEDTag::Write(CEDPacket* pPacket, BOOL bUnicode, BOOL bSmallTags)
{
	DWORD nPos = pPacket->m_nLength;

	pPacket->WriteByte( m_nType );

	if ( int nKeyLen = m_sKey.GetLength() )
	{
		pPacket->WriteEDString( m_sKey, bUnicode );
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
				if ( bUnicode )
					pPacket->WriteStringUTF8( m_sValue, FALSE );
				else
					pPacket->WriteString( m_sValue, FALSE );
			}
			else
			{	// We should use a normal string tag
				// Correct the packet type
				pPacket->m_pBuffer[nPos] = 0x80 | ED2K_TAG_STRING ;

				// Write the string
				pPacket->WriteEDString( m_sValue, bUnicode );
			}
		}
		else
		{
			// Write the string
			pPacket->WriteEDString( m_sValue, bUnicode );
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

BOOL CEDTag::Read(CEDPacket* pPacket, BOOL bUnicode)
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
		if ( bUnicode )
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
		if ( bUnicode )
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
			if ( bUnicode )
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

BOOL CEDPacket::OnPacket(const SOCKADDR_IN* pHost)
{
	Statistics.Current.eDonkey.Incoming++;

	switch ( m_nEdProtocol )
	{
	case ED2K_PROTOCOL_EDONKEY:
	case ED2K_PROTOCOL_EMULE:
		return EDClients.OnPacket( pHost, this );

	case ED2K_PROTOCOL_KAD:
		return Kademlia.OnPacket( pHost, this );

	case ED2K_PROTOCOL_REVCONNECT:
		// TODO: Implement RevConnect KAD
		DEBUG_ONLY( Debug( _T("RevConnect KAD not implemented.") ) );
		break;

	default:
		;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CEDPacket packet type index

const ED2K_PACKET_DESC CEDPacket::m_pszTypes[] =
{
	{ ED2K_C2S_LOGINREQUEST,		ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("Hello") },
	{ ED2K_C2S_GETSERVERLIST,		ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("GetServerList") },
	{ ED2K_C2S_OFFERFILES,			ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("ShareFiles") },
	{ ED2K_C2S_SEARCHREQUEST,		ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("Search") },
	{ ED2K_C2S_SEARCHUSER,			ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("Browse") },
	{ ED2K_C2S_GETSOURCES,			ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("FindSource") },
	{ ED2K_C2S_CALLBACKREQUEST,		ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("PushRequest") },
	{ ED2K_C2S_MORERESULTS,			ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("MoreResults") },
	{ ED2K_S2C_REJECTED,			ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("Rejected") },
	{ ED2K_S2C_SERVERMESSAGE,		ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("ServerMessage") },
	{ ED2K_S2C_IDCHANGE,			ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("IdChange") },
	{ ED2K_S2C_SERVERLIST,			ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("ServerList") },
	{ ED2K_S2C_SEARCHRESULTS,		ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("SearchResults") },
	{ ED2K_S2C_FOUNDSOURCES,		ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("FoundSources") },
	{ ED2K_S2C_SERVERSTATUS,		ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("ServerStatus") },
	{ ED2K_S2C_SERVERIDENT,			ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("ServerIdent") },
	{ ED2K_S2C_CALLBACKREQUESTED,	ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("PushAnswer") },

	{ ED2K_C2SG_SERVERSTATUSREQUEST,ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("StatusRequest") },
	{ ED2K_S2CG_SERVERSTATUS,		ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("Status") },
	{ ED2K_C2SG_SEARCHREQUEST,		ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("Search 1") },
	{ ED2K_C2SG_SEARCHREQUEST2,		ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("Search 2") },
	{ ED2K_C2SG_SEARCHREQUEST3,		ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("Search 3") },
	{ ED2K_S2CG_SEARCHRESULT,		ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("SearchResult") },
	{ ED2K_C2SG_GETSOURCES,			ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("FindSource") },
	{ ED2K_C2SG_GETSOURCES2,		ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("FindSource2") },
	{ ED2K_S2CG_FOUNDSOURCES,		ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("FoundSources") },
	{ ED2K_C2SG_CALLBACKREQUEST,	ED2K_PROTOCOL_EDONKEY,	TRUE,	_T("PushRequest") },

	{ ED2K_C2C_HELLO,				ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("Hello") },
	{ ED2K_C2C_HELLOANSWER,			ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("Hello Answer") },
	{ ED2K_C2C_FILEREQUEST,			ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("FileRequest") },
	{ ED2K_C2C_FILEREQANSWER,		ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("FileName") },
	{ ED2K_C2C_FILENOTFOUND,		ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("FileNotFound") },
	{ ED2K_C2C_FILESTATUS,			ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("FileStatus") },
	{ ED2K_C2C_QUEUEREQUEST,		ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("EnqueueMe") },
	{ ED2K_C2C_QUEUERELEASE,		ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("DequeueMe") },
	{ ED2K_C2C_QUEUERANK,			ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("QueueRank") },
	{ ED2K_C2C_STARTUPLOAD,			ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("StartUpload") },
	{ ED2K_C2C_FINISHUPLOAD,		ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("UploadFinished") },
	{ ED2K_C2C_REQUESTPARTS,		ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("RequestFrag") },
	{ ED2K_C2C_SENDINGPART,			ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("Fragment") },
	{ ED2K_C2C_FILESTATUSREQUEST,	ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("GetStatus") },
	{ ED2K_C2C_HASHSETREQUEST,		ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("GetHashset") },
	{ ED2K_C2C_HASHSETANSWER,		ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("Hashset") },
	{ ED2K_C2C_ASKSHAREDFILES,		ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("BrowseFiles") },
	{ ED2K_C2C_ASKSHAREDFILESANSWER,ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("BrowseFilesAnswer") },
	{ ED2K_C2C_MESSAGE,				ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("Message") },
	{ ED2K_C2C_ASKSHAREDDIRS,		ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("BrowseDirs") },
	{ ED2K_C2C_ASKSHAREDDIRSANSWER,	ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("BrowseDirsAnswer") },
	{ ED2K_C2C_ASKSHAREDDIRSDENIED,	ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("BrowseDirsDenied") },
	{ ED2K_C2C_VIEWSHAREDDIR,		ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("ViewDirs") },
	{ ED2K_C2C_VIEWSHAREDDIRANSWER,	ED2K_PROTOCOL_EDONKEY,	FALSE,	_T("ViewDirsAnswer") },

// eMule Client - Client TCP
	{ ED2K_C2C_COMPRESSEDPART,		ED2K_PROTOCOL_EMULE,	FALSE,	_T("CompFragment") },
	{ ED2K_C2C_QUEUERANKING,		ED2K_PROTOCOL_EMULE,	FALSE,	_T("QueueRanking") },
	{ ED2K_C2C_FILEDESC,			ED2K_PROTOCOL_EMULE,	FALSE,	_T("FileDescription") },
	{ ED2K_C2C_REQUESTSOURCES,		ED2K_PROTOCOL_EMULE,	FALSE,	_T("RequestSources") },
	{ ED2K_C2C_ANSWERSOURCES,		ED2K_PROTOCOL_EMULE,	FALSE,	_T("Sources") },
	{ ED2K_C2C_REQUESTPREVIEW,		ED2K_PROTOCOL_EMULE,	FALSE,	_T("Request Preview") },
	{ ED2K_C2C_PREVIEWANWSER,		ED2K_PROTOCOL_EMULE,	FALSE,	_T("Preview") },

// eMule Client - Client TCP (64Bit LargeFile support)
	{ ED2K_C2C_COMPRESSEDPART_I64,	ED2K_PROTOCOL_EMULE,	FALSE,	_T("CompFragment64") },
	{ ED2K_C2C_REQUESTPARTS_I64,	ED2K_PROTOCOL_EMULE,	FALSE,	_T("RequestFrag64") },
	{ ED2K_C2C_SENDINGPART_I64,		ED2K_PROTOCOL_EMULE,	FALSE,	_T("Fragment64") },

	{ 0,							ED2K_PROTOCOL_EDONKEY,	FALSE,	NULL }
};
