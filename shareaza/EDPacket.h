//
// EDPacket.h
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

#pragma once

#include "Packet.h"

#pragma pack(1)

typedef struct
{
	BYTE	nProtocol;
	DWORD	nLength;
	BYTE	nType;
} ED2K_TCP_HEADER;

typedef struct
{
	BYTE	nProtocol;
	BYTE	nType;
} ED2K_UDP_HEADER;

typedef struct
{
	BYTE	nProtocol;
	DWORD	nLength;
	BYTE	nType;
	Hashes::Ed2kHash::RawStorage pMD4;
	DWORD	nOffset1;
	DWORD	nOffset2;
} ED2K_PART_HEADER;

typedef struct
{
	BYTE	nProtocol;
	DWORD	nLength;
	BYTE	nType;
	Hashes::Ed2kHash::RawStorage pMD4;
	QWORD	nOffset1;
	QWORD	nOffset2;
} ED2K_PART_HEADER_I64;

#pragma pack()

typedef struct
{
	DWORD	nType;
	LPCTSTR	pszName;
} ED2K_PACKET_DESC;

#define	ED2K_VERSION					0x3D

#define ED2K_PROTOCOL_EDONKEY			0xE3	// eDonkey, Overnet, MLDonkey and other
#define ED2K_PROTOCOL_KAD				0xE4	// eMule KAD
#define ED2K_PROTOCOL_KAD_PACKED		0xE5	// eMule KAD compressed
#define ED2K_PROTOCOL_REVCONNECT		0xD0	// RevConnect KAD
#define ED2K_PROTOCOL_REVCONNECT_PACKED	0xD1	// RevConnect KAD compressed
#define ED2K_PROTOCOL_EMULE_PACKED		0xD4	// eMule compressed
#define ED2K_PROTOCOL_EMULE				0xC5	// eMule
//#define ED2K_PROTOCOL_MLDONKEY		0x00
//#define ED2K_PROTOCOL_LANCAST			0xC6	// eMule Plus LANCast

#define ED2K_MET						0x0E	// First byte of .met-file
#define ED2K_MET_I64TAGS				0x0F	// First byte of .met-file with "Large File" support

class CBuffer;


class CEDPacket : public CPacket  
{
// Construction
protected:
	CEDPacket();
	virtual ~CEDPacket();
	
// Attributes
public:
	BYTE	m_nEdProtocol;
	BYTE	m_nType;
	
// Operations
public:
	CString				ReadEDString(DWORD ServerFlags);
	void				WriteEDString(LPCTSTR psz, DWORD ServerFlags);
	CString				ReadEDString(BOOL bUnicode);
	void				WriteEDString(LPCTSTR psz, BOOL bUnicode);
	CString				ReadLongEDString(BOOL bUnicode);
	void				WriteLongEDString(LPCTSTR psz, BOOL bUnicode);
	BOOL				Deflate();
	// Unzip packet if any.
	//	Returns: FALSE - ok; TRUE - unzip error and packed was released
	BOOL				InflateOrRelease();
public:
	virtual	void		ToBuffer(CBuffer* pBuffer) const;
	virtual	void		ToBufferUDP(CBuffer* pBuffer) const;
	static	CEDPacket*	ReadBuffer(CBuffer* pBuffer);
public:
	virtual CString GetType() const;
	virtual void	Debug(LPCTSTR pszReason) const;

// Utility
public:
	inline static BOOL IsLowID(DWORD nID) { return nID > 0 && nID < 16777216; }

// Packet Types
protected:
	static ED2K_PACKET_DESC m_pszTypes[];

// Packet Pool
protected:
	class CEDPacketPool : public CPacketPool
	{
	public:
		virtual ~CEDPacketPool() { Clear(); }
	protected:
		virtual void NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch);
		virtual void FreePoolImpl(CPacket* pPool);
	};
	
	static CEDPacketPool POOL;

// Construction
public:
	static CEDPacket* New(BYTE nType, BYTE nProtocol = ED2K_PROTOCOL_EDONKEY)
	{
		CEDPacket* pPacket		= (CEDPacket*)POOL.New();
		pPacket->m_nEdProtocol	= nProtocol;
		pPacket->m_nType		= nType;
		return pPacket;
	}
	
	static CEDPacket* New(ED2K_TCP_HEADER* pHeader)
	{
		CEDPacket* pPacket		= (CEDPacket*)POOL.New();
		pPacket->m_nEdProtocol	= pHeader->nProtocol;
		pPacket->m_nType		= pHeader->nType;
		pPacket->Write( &pHeader[1], pHeader->nLength - 1 );
		return pPacket;
	}

	static CEDPacket* New(ED2K_UDP_HEADER* pHeader, DWORD nLength)
	{
		CEDPacket* pPacket		= (CEDPacket*)POOL.New();
		pPacket->m_nEdProtocol	= pHeader->nProtocol;
		pPacket->m_nType		= pHeader->nType;
		pPacket->Write( &pHeader[1], nLength - sizeof(*pHeader) );
		return pPacket;
	}

	inline virtual void Delete()
	{
		POOL.Delete( this );
	}
	
	friend class CEDPacket::CEDPacketPool;
};

inline void CEDPacket::CEDPacketPool::NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch)
{
	nPitch	= sizeof(CEDPacket);
	pPool	= new CEDPacket[ nSize ];
	if ( pPool == NULL )
	{
		theApp.Message( MSG_ERROR, _T("Memory allocation error in CEDPacket::CEDPacketPool::NewPoolImpl()") );
	}
}

inline void CEDPacket::CEDPacketPool::FreePoolImpl(CPacket* pPacket)
{
	delete [] (CEDPacket*)pPacket;
}

// Client - Server, Local (TCP)
#define ED2K_C2S_LOGINREQUEST			0x01
#define ED2K_C2S_GETSERVERLIST			0x14
#define	ED2K_C2S_OFFERFILES				0x15
#define ED2K_C2S_SEARCHREQUEST			0x16
#define ED2K_C2S_SEARCHUSER				0x1a
#define ED2K_C2S_GETSOURCES				0x19
#define ED2K_C2S_CALLBACKREQUEST		0x1C
#define ED2K_C2S_MORERESULTS			0x21
#define ED2K_S2C_REJECTED				0x05
#define ED2K_S2C_SERVERMESSAGE			0x38
#define ED2K_S2C_IDCHANGE				0x40
#define ED2K_S2C_SERVERLIST				0x32
#define ED2K_S2C_SEARCHRESULTS			0x33
#define ED2K_S2C_FOUNDSOURCES			0x42
#define	ED2K_S2C_SERVERSTATUS			0x34
#define ED2K_S2C_SERVERIDENT			0x41
#define ED2K_S2C_CALLBACKREQUESTED		0x35

// Client - Server, Global (UDP)
#define ED2K_C2SG_SEARCHREQUEST3		0x90
#define ED2K_C2SG_SEARCHREQUEST2		0x92
#define ED2K_C2SG_GETSOURCES2			0x94
#define ED2K_C2SG_SERVERSTATUSREQUEST	0x96
#define	ED2K_S2CG_SERVERSTATUS			0x97
#define ED2K_C2SG_SEARCHREQUEST			0x98
#define ED2K_S2CG_SEARCHRESULT			0x99
#define ED2K_C2SG_GETSOURCES			0x9A
#define ED2K_S2CG_FOUNDSOURCES			0x9B
#define ED2K_C2SG_CALLBACKREQUEST		0x9C
#define ED2K_S2CG_CALLBACKFAIL			0x9E

// Client - Client, TCP
#define ED2K_C2C_HELLO					0x01
#define ED2K_C2C_HELLOANSWER			0x4C
#define ED2K_C2C_FILEREQUEST			0x58
#define ED2K_C2C_FILEREQANSWER			0x59
#define ED2K_C2C_FILENOTFOUND			0x48
#define ED2K_C2C_FILESTATUS				0x50
#define ED2K_C2C_QUEUEREQUEST			0x54
#define ED2K_C2C_QUEUERELEASE			0x56
#define ED2K_C2C_QUEUERANK				0x5C
#define ED2K_C2C_STARTUPLOAD			0x55
#define ED2K_C2C_FINISHUPLOAD			0x57
#define ED2K_C2C_REQUESTPARTS			0x47
#define ED2K_C2C_SENDINGPART			0x46
#define ED2K_C2C_FILESTATUSREQUEST		0x4F
#define ED2K_C2C_HASHSETREQUEST			0x51
#define ED2K_C2C_HASHSETANSWER			0x52
#define ED2K_C2C_ASKSHAREDFILES			0x4A
#define ED2K_C2C_ASKSHAREDFILESANSWER	0x4B
#define ED2K_C2C_MESSAGE				0x4E
#define ED2K_C2C_CHANGECLIENTID			0x4D
#define ED2K_C2C_ASKSHAREDDIRS			0x5D
#define ED2K_C2C_ASKSHAREDDIRSANSWER	0x5F
#define ED2K_C2C_VIEWSHAREDDIR			0x5E
#define ED2K_C2C_VIEWSHAREDDIRANSWER	0x60

// eMule Client - Client TCP
#define	ED2K_C2C_EMULEINFO				0x01
#define	ED2K_C2C_EMULEINFOANSWER		0x02
#define ED2K_C2C_COMPRESSEDPART			0x40
#define ED2K_C2C_QUEUERANKING			0x60
#define ED2K_C2C_FILEDESC				0x61
#define ED2K_C2C_REQUESTSOURCES			0x81
#define ED2K_C2C_ANSWERSOURCES			0x82
#define ED2K_C2C_REQUESTPREVIEW			0x90
#define ED2K_C2C_PREVIEWANWSER			0x91

// eMule Client - Client TCP (64Bit LargeFile support)
#define ED2K_C2C_COMPRESSEDPART_I64		0xA1
#define ED2K_C2C_SENDINGPART_I64		0xA2
#define ED2K_C2C_REQUESTPARTS_I64		0xA3

// Client - Client, UDP
#define ED2K_C2C_UDP_REASKFILEPING		0x90
#define ED2K_C2C_UDP_REASKACK			0x91
#define ED2K_C2C_UDP_FILENOTFOUND		0x92
#define ED2K_C2C_UDP_QUEUEFULL			0x93

// Server TCP flags
#define	ED2K_SERVER_TCP_DEFLATE			0x00000001
#define	ED2K_SERVER_TCP_SMALLTAGS		0x00000008
#define	ED2K_SERVER_TCP_UNICODE			0x00000010
#define	ED2K_SERVER_TCP_GETSOURCES2		0x00000020
#define	ED2K_SERVER_TCP_RELATEDSEARCH	0x00000040
#define	ED2K_SERVER_TCP_64BITSIZE		0x00000080
// Server UDP flags
#define	ED2K_SERVER_UDP_GETSOURCES		0x00000001
#define	ED2K_SERVER_UDP_GETFILES		0x00000002
#define	ED2K_SERVER_UDP_UNICODE			0x00000010
#define	ED2K_SERVER_UDP_GETSOURCES2		0x00000020
#define	ED2K_SERVER_UDP_64BITSIZE		0x00000080


class CEDTag
{
// Construction
public:
	CEDTag();
	CEDTag(BYTE nKey);
	CEDTag(BYTE nKey, DWORD nValue);
	CEDTag(BYTE nKey, LPCTSTR pszValue);
	CEDTag(LPCTSTR pszKey);
	CEDTag(LPCTSTR pszKey, DWORD nValue);
	CEDTag(LPCTSTR pszKey, LPCTSTR pszValue);
	~CEDTag() {};

// Attributes
public:
	BYTE	m_nType;
	CString	m_sKey;
	BYTE	m_nKey;
	CString	m_sValue;
	DWORD	m_nValue;
	
// Operations
public:
	void	Clear();
	void	Write(CEDPacket* pPacket, DWORD ServerFlags = 0);
	BOOL	Read(CEDPacket* pPacket, DWORD ServerFlags = 0);
	BOOL	Read(CFile* pFile);
	
// Inlines
public:
	inline BOOL Check(BYTE nKey, BYTE nType) const
	{
		return m_nKey == nKey && m_nType == nType;
	}
};

// Standard tags
#define ED2K_TAG_NULL				0x00
#define ED2K_TAG_HASH				0x01
#define ED2K_TAG_STRING				0x02
#define ED2K_TAG_INT				0x03
#define ED2K_TAG_FLOAT				0x04
#define ED2K_TAG_BOOL				0x05
#define ED2K_TAG_BOOL_ARRAY			0x06
#define ED2K_TAG_BLOB				0x07
// New tags (See ED2K_SERVER_TCP_NEWTAGS)
#define ED2K_TAG_UINT16				0x08
#define ED2K_TAG_UINT8				0x09
#define ED2K_TAG_UNUSED				0x0A // 8 bit size, then value
#define ED2K_TAG_SHORTSTRING		0x11 //String <=16 bytes, using tag ID for length
// 0x10 to 0x20 are reserved for short strings
#define ED2K_TAG_STRING1			0x11
#define ED2K_TAG_STRING16			0x20

// Server tags / met files
#define ED2K_ST_SERVERNAME			0x01
#define ED2K_ST_DESCRIPTION			0x0B
#define ED2K_ST_PING				0x0C
#define ED2K_ST_PREFERENCE			0x0E
#define ED2K_ST_FAIL				0x0D
#define	ED2K_ST_DYNIP				0x85
#define ED2K_ST_LASTPING			0x86
#define ED2K_ST_MAXUSERS			0x87
#define ED2K_ST_MAXFILES			0x88
#define ED2K_ST_UDPFLAGS			0x92

// Client tags
#define ED2K_CT_NAME				0x01
#define	ED2K_CT_PORT				0x0F
#define ED2K_CT_VERSION				0x11
#define ED2K_CT_FLAGS				0x20	// Tell server about compression, new tags, unicode
#define ED2K_CT_MODVERSION			0x55	// MOD version
#define	ED2K_CT_UDPPORTS			0xF9	// Ports used for UDP	
#define	ED2K_CT_FEATUREVERSIONS		0xFA	// Tells extended features (like emule info)
#define	ED2K_CT_SOFTWAREVERSION		0xFB	// Version of the program.
#define	ED2K_CT_UNKNOWN1			0xFC	// ?
#define	ED2K_CT_UNKNOWN2			0xFD	// ?
#define	ED2K_CT_MOREFEATUREVERSIONS	0xFE	// KAD version (Unused)
#define	ED2K_CT_UNKNOWN3			0xFF	// ?


// File tags
#define ED2K_FT_FILENAME			0x01
#define ED2K_FT_FILESIZE			0x02
#define ED2K_FT_FILETYPE			0x03
#define ED2K_FT_FILEFORMAT			0x04
#define ED2K_FT_LASTSEENCOMPLETE	0x05
#define ED2K_FT_TRANSFERED			0x08
#define ED2K_FT_GAPSTART			0x09
#define ED2K_FT_GAPEND				0x0A
#define ED2K_FT_PARTFILENAME		0x12
#define ED2K_FT_PRIORITY			0x13
#define ED2K_FT_STATUS				0x14
#define ED2K_FT_SOURCES				0x15
#define ED2K_FT_PERMISSIONS			0x16
#define ED2K_FT_ULPRIORITY			0x17
#define ED2K_FT_COMPLETESOURCES		0x30
#define ED2K_FT_FILESIZEUPPER		0x32
#define ED2K_FT_ATTRANSFERED		0x50
#define ED2K_FT_ATREQUESTED			0x51
#define ED2K_FT_ATACCEPTED			0x52
#define ED2K_FT_LENGTH				0xD3
#define ED2K_FT_BITRATE				0xD4
#define ED2K_FT_CODEC				0xD5
#define ED2K_FT_FILERATING			0xF7

// eMuleinfo tags. Note this is now obsolete, due to ED2K_CT_FEATUREVERSIONS
#define ED2K_ET_COMPRESSION			0x20
#define ED2K_ET_UDPPORT				0x21
#define ED2K_ET_UDPVER				0x22
#define ED2K_ET_SOURCEEXCHANGE		0x23
#define ED2K_ET_COMMENTS			0x24
#define ED2K_ET_EXTENDEDREQUEST		0x25
#define ED2K_ET_COMPATIBLECLIENT	0x26	// Client ID.
#define ED2K_ET_FEATURES			0x27	// Preview and sec ID

// Max files (Hash + Size) in a getsources packet
#define ED2K_MAXFILESINPACKET		0x20

// Max message (chat) length
#define ED2K_MESSAGE_MAX			500
// Max file comment length
#define ED2K_COMMENT_MAX			250
// Max file size in 32 bits
#define MAX_SIZE_32BIT				0xFFFFFFFF

// Client ID
#define ED2K_COMPATIBLECLIENT_ID	ED2K_CLIENT_ID

// "Unknown" and "Unknown mod" client ID for compatible client variable
#define ED2K_CLIENT_UNKNOWN			0xFF
#define ED2K_CLIENT_MOD				0xFE

// Shareaza's advertised capabilities / feature versions
#define ED2K_VERSION_COMPRESSION	0x01
#define ED2K_VERSION_UDP			0x02
#define ED2K_VERSION_SOURCEEXCHANGE	0x02
#define ED2K_VERSION_COMMENTS		0x01
#define ED2K_VERSION_EXTENDEDREQUEST 0x02 // Note: Defined at run time. 0, 1, or 2

// Things that aren't supported
#define ED2K_VERSION_AICH			0x00
#define ED2K_VERSION_SECUREID		0x00




