//
// ID3.h
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

#pragma once

#pragma pack(1)

typedef struct
{
	CHAR	szTag[3];
	CHAR	szSongname[30];
	CHAR	szArtist[30];
	CHAR	szAlbum[30];
	CHAR	szYear[4];
	CHAR	szComment[30];
	BYTE	nGenre;
} ID3V1;

#define ID3V1_TAG	"TAG"

typedef struct
{
	CHAR	szTag[3];
	BYTE	nMajorVersion;
	BYTE	nMinorVersion;
	BYTE	nFlags;
	DWORD	nSize;
} ID3V2_HEADER;

#define ID3V2_TAG			"ID3"

#define ID3V2_UNSYNCHRONISED	0x80
#define ID3V2_EXTENDEDHEADER	0x40
#define ID3V2_EXPERIMENTAL		0x20
#define ID3V2_FOOTERPRESENT		0x10
#define ID3V2_KNOWNMASK			0xF0

typedef struct
{
	DWORD	nSize;
	BYTE	nFlags1;
	BYTE	nFlags2;
	DWORD	nPadding;
} ID3V2_EXTENDED_HEADER_1;

typedef struct
{
	DWORD	nSize;
	BYTE	nFlagBytes;
} ID3V2_EXTENDED_HEADER_2;

typedef struct
{
	CHAR	szID[4];
	DWORD	nSize;
	BYTE	nFlags1;
	BYTE	nFlags2;
} ID3V2_FRAME;

typedef struct
{
	CHAR	szID[3];
	BYTE	nSize[3];
} ID3V2_FRAME_2;

#define ID3V2_KNOWNFRAME	0x4F

#define ID3_DESYNC_SIZE(x)	{	\
	x = ( x & 0x7F ) | ( ( x & ~0xFF ) >> 1 );	\
	x = ( x & 0x03FFF ) | ( ( x & ~0x03FFF ) >> 1 );	\
	x = ( x & 0x01FFFFF ) | ( ( x & ~0x01FFFFF ) >> 1 ); }

typedef struct
{
	char cID[4];							// should equal 'MAC '
	unsigned __int16 nVersion;				// version number * 1000 (3.81 = 3810)
	unsigned __int16 nCompressionLevel;		// the compression level
	unsigned __int16 nFormatFlags;			// any format flags (for future use)
	unsigned __int16 nChannels;				// the number of channels (1 or 2)
	unsigned __int32 nSampleRate;			// the sample rate (typically 44100)
	unsigned __int32 nHeaderBytes;			// the bytes after the MAC header that compose the WAV header
	unsigned __int32 nTerminatingBytes;		// the bytes after that raw data (for extended info)
	unsigned __int32 nTotalFrames;			// the number of frames in the file
	unsigned __int32 nFinalFrameBlocks;		// the number of samples in the final frame
} APE_HEADER;

typedef struct
{
	char cID[8];			// should equal 'APETAGEX'	
	int nVersion;			// equals CURRENT_APE_TAG_VERSION
	int nSize;				// the complete size of the tag, including this footer
	int nFields;			// the number of fields in the tag
	int nFlags;				// the tag flags (none currently defined)
	char cReserved[8];		// reserved for later use
} APE_TAG_FOOTER;

typedef struct
{
    DWORD		dwMicroSecPerFrame;	// frame display rate (or 0L)
    DWORD		dwMaxBytesPerSec;	// max. transfer rate
    DWORD		dwPaddingGranularity;	// pad to multiples of this
                                                // size; normally 2K.
    DWORD		dwFlags;		// the ever-present flags
    DWORD		dwTotalFrames;		// # frames in file
    DWORD		dwInitialFrames;
    DWORD		dwStreams;
    DWORD		dwSuggestedBufferSize;
    
    DWORD		dwWidth;
    DWORD		dwHeight;
    
    DWORD		dwReserved[4];
} AVI_HEADER;

#ifdef _ID3_DEFINE_GENRES
LPCTSTR CLibraryBuilderInternals::pszID3Genre[] =
{
	_T("Blues"), _T("Classic Rock"), _T("Country"), _T("Dance"), _T("Disco"),
	_T("Funk"), _T("Grunge"), _T("Hip-Hop"), _T("Jazz"), _T("Metal"), _T("New Age"),
	_T("Oldies"), _T("Other"), _T("Pop"), _T("R&B"), _T("Rap"), _T("Reggae"),
	_T("Rock"), _T("Techno"), _T("Industrial"), _T("Alternative"), _T("Ska"),
	_T("Death Metal"), _T("Pranks"), _T("Soundtrack"), _T("Euro-Techno"), _T("Ambient"),
	_T("Trip-Hop"), _T("Vocal"), _T("Jazz+Funk"), _T("Fusion"), _T("Trance"),
	_T("Classical"), _T("Instrumental"), _T("Acid"), _T("House"), _T("Game"),
	_T("Sound Clip"), _T("Gospel"), _T("Noise"), _T("AlternRock"), _T("Bass"),
	_T("Soul"), _T("Punk"), _T("Space"), _T("Meditative"), _T("Instrumental Pop"),
	_T("Instrumental Rock"), _T("Ethnic"), _T("Gothic"), _T("Darkwave"),
	_T("Techno-Industrial"), _T("Electronic"), _T("Pop-Folk"), _T("Eurodance"),
	_T("Dream"), _T("Southern Rock"), _T("Comedy"), _T("Cult"), _T("Gangsta"), _T("Top 40"),
	_T("Christian Rap"), _T("Pop/Funk"), _T("Jungle"), _T("Native American"), _T("Cabaret"),
	_T("New Wave"), _T("Psychadelic"), _T("Rave"), _T("Showtunes"), _T("Trailer"),
	_T("Lo-Fi"), _T("Tribal"), _T("Acid Punk"), _T("Acid Jazz"), _T("Polka"), _T("Retro"),
	_T("Musical"), _T("Rock & Roll"), _T("Hard Rock"), _T("Folk"), _T("Folk/Rock"),
	_T("National Folk"), _T("Swing"), _T("Easy Listening"), _T("Bebob"), _T("Latin"), _T("Revival"),
	_T("Celtic"), _T("Bluegrass"), _T("Avantgarde"), _T("Gothic Rock"),
	_T("Progressive Rock"), _T("Psychedelic Rock"), _T("Symphonic Rock"),
	_T("Slow Rock"), _T("Big Band"), _T("Chorus"), _T("Easy Listening"),
	_T("Acoustic"), _T("Humour"), _T("Speech"), _T("Chanson"), _T("Opera"),
	_T("Chamber Music"), _T("Sonata"), _T("Symphony"), _T("Booty Bass"),
	_T("Primus"), _T("Porn Groove"), _T("Satire"), _T("Slow Jam"), _T("Club"),
	_T("Tango"), _T("Samba"), _T("Folklore"), _T("Ballad"), _T("Power Ballad"),
	_T("Rhythmic Soul"), _T("Freestyle"), _T("Duet"), _T("Punk Rock"), _T("Drum Solo"),
	_T("A capella"), _T("Euro-House"), _T("Dance Hall")

};
#endif

#define ID3_GENRES	126

#pragma pack()
