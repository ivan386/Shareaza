//
// BTInfo.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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
#include "BENode.h"
#include "BTInfo.h"
#include "Buffer.h"
#include "DownloadTask.h"
#include "DownloadGroups.h"
#include "Library.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "Transfers.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Check if a string is a valid path/file name.
static bool IsValid(const CString& str)
{
	return ! str.IsEmpty() && ( str.Find( _T('?') ) == -1 ) && ( str != _T("#ERROR#") );
}

//////////////////////////////////////////////////////////////////////
// CBTInfo construction

CBTInfo::CBTInfo()
	: m_nBlockSize		( 0ul )
	, m_nBlockCount		( 0ul )
	, m_pBlockBTH		( NULL )
	, m_nTotalUpload	( 0ull )
	, m_nTotalDownload	( 0ull )
	, m_nTrackerIndex	( -1 )
	, m_nTrackerMode	( tNull )
	, m_nEncoding		( Settings.BitTorrent.TorrentCodePage )
	, m_tCreationDate	( 0ul )
	, m_bPrivate		( FALSE )
	, m_nStartDownloads	( dtAlways )
	, m_bEncodingError	( false )
	, m_nTestByte		( 0ul )
	, m_nInfoSize		( 0ul )
	, m_nInfoStart		( 0ul )
{
	CBENode::m_nDefaultCP = Settings.BitTorrent.TorrentCodePage;
}

CBTInfo::CBTInfo(const CBTInfo& oSource)
	: m_nBlockSize		( 0ul )
	, m_nBlockCount		( 0ul )
	, m_pBlockBTH		( NULL )
	, m_nTotalUpload	( 0ull )
	, m_nTotalDownload	( 0ull )
	, m_nTrackerIndex	( -1 )
	, m_nTrackerMode	( tNull )
	, m_nEncoding		( Settings.BitTorrent.TorrentCodePage )
	, m_tCreationDate	( 0ul )
	, m_bPrivate		( FALSE )
	, m_nStartDownloads	( dtAlways )
	, m_bEncodingError	( false )
	, m_nTestByte		( 0ul )
	, m_nInfoSize		( 0ul )
	, m_nInfoStart		( 0ul )
{
	*this = oSource;

	CBENode::m_nDefaultCP = Settings.BitTorrent.TorrentCodePage;
}

CBTInfo::~CBTInfo()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CBTFile construction

CBTInfo::CBTFile::CBTFile(const CBTInfo* pInfo, const CBTFile* pBTFile)
	: m_pInfo			( pInfo )
	, m_nOffset			( pBTFile ? pBTFile->m_nOffset : 0 )
{
	if ( pBTFile )
	{
		CShareazaFile::operator=( *pBTFile );
	}
}

const CString& CBTInfo::CBTFile::FindFile()
{
	CString strShortPath;
	int nSlash = m_sPath.Find( _T('\\') );
	if ( nSlash != -1 )
		strShortPath = m_sPath.Mid( nSlash + 1 );

	CStringIList mFolders;

	// Try complete folder
	mFolders.AddTail( Settings.Downloads.CompletePath );

	// Try all possible download folders
	DownloadGroups.GetFolders( mFolders );

	// Try folder of original .torrent
	mFolders.AddTail( m_pInfo->m_sPath.Left( m_pInfo->m_sPath.ReverseFind( _T('\\') ) ) );

	for ( POSITION pos = mFolders.GetHeadPosition(); pos; )
	{
		const CString sFolder = mFolders.GetNext( pos );

		CString strFile = sFolder + _T("\\") + m_sPath;
		if ( GetFileSize( strFile ) == m_nSize )
		{
			m_sBestPath = strFile;
			return m_sBestPath;
		}

		// ... without outer file directory
		if ( ! strShortPath.IsEmpty() )
		{
			strFile = sFolder + _T("\\") + strShortPath;
			if ( GetFileSize( strFile ) == m_nSize )
			{
				m_sBestPath = strFile;
				return m_sBestPath;
			}
		}
	}

	CQuickLock oLock( Library.m_pSection );

	// Try find file by hash/size
	if ( const CLibraryFile* pShared = LibraryMaps.LookupFileByHash( this, FALSE, TRUE ) )
	{
		const CString strFile = pShared->GetPath();
		if ( GetFileSize( strFile ) == m_nSize )
		{
			m_sBestPath = strFile;
			return m_sBestPath;
		}
	}

	// Try find by name only
	if ( const CLibraryFile* pShared = LibraryMaps.LookupFileByName( m_sName, m_nSize, FALSE, TRUE ) )
	{
		const CString strFile = pShared->GetPath();
		if ( GetFileSize( strFile ) == m_nSize )
		{
			m_sBestPath = strFile;
			return m_sBestPath;
		}
	}

	m_sBestPath.Empty();
	return m_sPath;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo clear

void CBTInfo::Clear()
{
	m_oMD5.clear();
	m_oBTH.clear();
	m_oSHA1.clear();
	m_oED2K.clear();
	m_oTiger.clear();
	m_sName.Empty();
	m_sPath.Empty();
	m_sURL.Empty();
	m_nSize				= SIZE_UNKNOWN;

	m_oNodes.RemoveAll();
	m_sURLs.RemoveAll();

	m_nBlockSize		= 0;
	m_nBlockCount		= 0;
	delete [] m_pBlockBTH;
	m_pBlockBTH			= NULL;

	m_nTotalUpload		= 0;
	m_nTotalDownload	= 0;

	for ( POSITION pos = m_pFiles.GetHeadPosition(); pos; )
		delete m_pFiles.GetNext( pos );
	m_pFiles.RemoveAll();

	m_nEncoding			= Settings.BitTorrent.TorrentCodePage;
	m_sComment.Empty();
	m_tCreationDate		= 0;
	m_sCreatedBy.Empty();
	m_bPrivate			= FALSE;
	m_nStartDownloads	= dtAlways;
	m_oTrackers.RemoveAll();
	m_nTrackerIndex		= -1;
	m_nTrackerMode		= tNull;
	m_bEncodingError	= false;
	m_pTestSHA1.Reset();
	m_nTestByte			= 0;
	m_pSource.Clear();
	m_nInfoSize			= 0;
	m_nInfoStart		= 0;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo copy

CBTInfo& CBTInfo::operator=(const CBTInfo& oSource)
{
	Clear();

	CShareazaFile::operator=( oSource );

	for ( POSITION pos = oSource.m_sURLs.GetHeadPosition(); pos; )
		m_sURLs.AddTail( oSource.m_sURLs.GetNext( pos ) );

	m_nBlockSize		= oSource.m_nBlockSize;
	m_nBlockCount		= oSource.m_nBlockCount;
	if ( oSource.m_pBlockBTH )
	{
		m_pBlockBTH = new Hashes::BtPureHash[ m_nBlockCount ];
		std::copy( oSource.m_pBlockBTH, oSource.m_pBlockBTH + m_nBlockCount,
			stdext::make_checked_array_iterator( m_pBlockBTH, m_nBlockCount ) );
	}

	m_nTotalUpload		= oSource.m_nTotalUpload;
	m_nTotalDownload	= oSource.m_nTotalDownload;

	for ( POSITION pos = oSource.m_pFiles.GetHeadPosition(); pos; )
		m_pFiles.AddTail( new CBTFile( this, oSource.m_pFiles.GetNext( pos ) ) );

	m_nEncoding			= oSource.m_nEncoding;
	m_sComment			= oSource.m_sComment;
	m_tCreationDate		= oSource.m_tCreationDate;
	m_sCreatedBy		= oSource.m_sCreatedBy;
	m_bPrivate			= oSource.m_bPrivate;
	m_nStartDownloads	= oSource.m_nStartDownloads;

	for ( INT_PTR i = 0; i < oSource.m_oTrackers.GetCount(); ++i )
		m_oTrackers.Add( oSource.m_oTrackers[ i ] );

	for ( POSITION pos = oSource.m_oNodes.GetHeadPosition(); pos; )
		m_oNodes.AddTail( oSource.m_oNodes.GetNext( pos ) );

	m_nTrackerIndex		= oSource.m_nTrackerIndex;
	m_nTrackerMode		= oSource.m_nTrackerMode;
	m_bEncodingError	= oSource.m_bEncodingError;
	m_pTestSHA1			= oSource.m_pTestSHA1;
	m_nTestByte			= oSource.m_nTestByte;

	m_pSource.Add( oSource.m_pSource.m_pBuffer, oSource.m_pSource.m_nLength );
	m_nInfoSize			= oSource.m_nInfoSize;
	m_nInfoStart		= oSource.m_nInfoStart;
	return *this;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo serialize

#define BTINFO_SER_VERSION 11
// History:
// 7 - redesigned tracker list (ryo-oh-ki)
// 8 - removed m_nFilePriority (ryo-oh-ki)
// 9 - added m_sName (ryo-oh-ki)
// 10 - added m_pSource (ivan386)
// 11 - added m_nInfoStart and m_nInfoSize (ivan386)

void CBTInfo::Serialize(CArchive& ar)
{
	int nVersion = BTINFO_SER_VERSION;

	if ( ar.IsStoring() )
	{
		ar << nVersion;

		SerializeOut( ar, m_oBTH );
		if ( !m_oBTH )
			return;

		ar << m_nSize;
		ar << m_nBlockSize;
		ar << m_nBlockCount;
		for ( DWORD i = 0; i < m_nBlockCount; ++i )
			ar.Write( &*m_pBlockBTH[ i ].begin(), m_pBlockBTH->byteCount );

		ar << m_nTotalUpload;
		ar << m_nTotalDownload;

		ar << m_sName;

		ar << m_nEncoding;
		ar << m_sComment;
		ar << m_tCreationDate;
		ar << m_sCreatedBy;
		ar << m_bPrivate;

		ar.WriteCount( m_pFiles.GetCount() );
		for ( POSITION pos = m_pFiles.GetHeadPosition(); pos ; )
			m_pFiles.GetNext( pos )->Serialize( ar, nVersion );

		ar << m_nTrackerIndex;
		ar << m_nTrackerMode;

		int nTrackers = (int)m_oTrackers.GetCount();
		ar.WriteCount( nTrackers );
		for ( int nTracker = 0 ; nTracker < nTrackers ; nTracker++ )
			m_oTrackers[ nTracker ].Serialize( ar, nVersion );

		if ( m_pSource.m_nLength && m_nInfoSize )
		{
			ar << m_pSource.m_nLength;
			ar.Write( m_pSource.m_pBuffer, m_pSource.m_nLength );
			ar << m_nInfoStart;
			ar << m_nInfoSize;
		}
		else
			ar << (DWORD)0;
	}
	else
	{
		ar >> nVersion;
		if ( nVersion < 1 )
			AfxThrowUserException();

		SerializeIn( ar, m_oBTH, nVersion );
		if ( !m_oBTH )
			return;

		if ( nVersion >= 2 )
		{
			ar >> m_nSize;
		}
		else
		{
			DWORD nTotalSize;
			ar >> nTotalSize;
			m_nSize = nTotalSize;
		}

		ar >> m_nBlockSize;
		ar >> m_nBlockCount;

		if ( m_nBlockCount )
		{
			m_pBlockBTH = new Hashes::BtPureHash[ m_nBlockCount ];
			for ( DWORD i = 0; i < m_nBlockCount; ++i )
				ReadArchive( ar, &*m_pBlockBTH[ i ].begin(), m_pBlockBTH->byteCount );
		}

		if ( nVersion >= 4 )
			ar >> m_nTotalUpload;

		if ( nVersion >= 6 )
			ar >> m_nTotalDownload;

		ar >> m_sName;

		if ( nVersion >= 3 )
		{
			ar >> m_nEncoding;
			ar >> m_sComment;
			ar >> m_tCreationDate;
			ar >> m_sCreatedBy;
		}

		if ( nVersion >= 5 )
			ar >> m_bPrivate;

		int nFiles = (int)ar.ReadCount();
		QWORD nOffset = 0;
		for ( int nFile = 0 ; nFile < nFiles ; nFile++ )
		{
			CAutoPtr< CBTFile >pBTFile( new CBTFile( this ) );
			if ( ! pBTFile )
				// Out Of Memory
				AfxThrowUserException();

			pBTFile->Serialize( ar, nVersion );

			pBTFile->m_nOffset = nOffset;
			nOffset += pBTFile->m_nSize;

			m_pFiles.AddTail( pBTFile.Detach() );
		}

		if ( nVersion < 7 )
		{
			CString sTracker;
			ar >> sTracker;
			SetTracker( sTracker );
		}

		if ( nVersion >= 4 )
		{
			ar >> m_nTrackerIndex;
			ar >> m_nTrackerMode;

			if ( nVersion < 7 )
			{
				int nTrackers = (int)ar.ReadCount();
				if ( nTrackers )
				{
					CBTTracker oTracker;
					oTracker.Serialize( ar, nVersion );
					AddTracker( oTracker );
				}
			}

			int nTrackers = (int)ar.ReadCount();
			if ( nTrackers )
			{
				for ( int nTracker = 0 ; nTracker < nTrackers ; nTracker++ )
				{
					CBTTracker oTracker;
					oTracker.Serialize( ar, nVersion );
					AddTracker( oTracker );
				}
			}
		}

		if ( nVersion >= 10 )
		{
			DWORD nLength;
			ar >> nLength;
			if ( nLength )
			{
				m_pSource.EnsureBuffer( nLength );
				ar.Read( m_pSource.m_pBuffer, nLength );
				m_pSource.m_nLength = nLength;
				if ( nVersion >= 11 )
				{
					ar >> m_nInfoStart;
					ar >> m_nInfoSize;
				}
				else
				{
					VERIFY( CheckInfoData() );
				}
			}
		}

		SetTrackerNext();
	}
}

//////////////////////////////////////////////////////////////////////
// CBTInfo::CBTFile serialize

void CBTInfo::CBTFile::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar << m_nSize;
		ar << m_sPath;
		ar << m_sName;
		SerializeOut( ar, m_oSHA1 );
		SerializeOut( ar, m_oED2K );
		SerializeOut( ar, m_oTiger );
		SerializeOut( ar, m_oMD5 );
	}
	else
	{
		if ( nVersion >= 2 )
		{
			ar >> m_nSize;
		}
		else
		{
			DWORD nSize;
			ar >> nSize;
			m_nSize = nSize;
		}

		ar >> m_sPath;

		if ( nVersion >= 9 )
			ar >> m_sName;
		else
			// Upgrade
			m_sName = PathFindFileName( m_sPath );

		SerializeIn( ar, m_oSHA1, nVersion );

		if ( nVersion >= 4 )
		{
			SerializeIn( ar, m_oED2K, nVersion );
			SerializeIn( ar, m_oTiger, nVersion );
			if ( nVersion < 8 )
			{
				int nFilePriority;
				ar >> nFilePriority;
			}
		}

		if ( nVersion >= 6 )
		{
			SerializeIn( ar, m_oMD5, nVersion );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CBTInfo load .torrent file

BOOL CBTInfo::LoadTorrentFile(LPCTSTR pszFile)
{
	CFile pFile;

	if ( pFile.Open( pszFile, CFile::modeRead|CFile::shareDenyNone ) )
	{
		DWORD nLength = (DWORD)pFile.GetLength();
		m_sPath = pszFile;

		if ( nLength < 20 * 1024 * 1024 && nLength != 0 )
		{
			m_pSource.Clear();
			if ( m_pSource.EnsureBuffer( nLength ) )
			{
				pFile.Read( m_pSource.m_pBuffer, nLength );
				m_pSource.m_nLength = nLength;

				return LoadTorrentBuffer( &m_pSource );
			}
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo save .torrent file

BOOL CBTInfo::SaveTorrentFile(const CString& sFolder)
{
	if ( ! IsAvailable() )
		return FALSE;

	if ( m_pSource.m_nLength == 0 )
		return FALSE;

	CString strPath = sFolder + _T("\\") + SafeFilename( m_sName + _T(".torrent") );
	if ( m_sPath.CompareNoCase( strPath ) == 0 )
		// Same file
		return TRUE;

	CFile pFile;
	if ( ! pFile.Open( strPath, CFile::modeWrite | CFile::modeCreate ) )
		return FALSE;

	pFile.Write( m_pSource.m_pBuffer, m_pSource.m_nLength );
	pFile.Close();

	m_sPath = strPath;

	return TRUE;
}

#define MAX_PIECE_SIZE (16 * 1024)
BOOL CBTInfo::LoadInfoPiece(BYTE *pPiece, DWORD nPieceSize, DWORD nInfoSize, DWORD nInfoPiece)
{
	ASSERT( nPieceSize <= MAX_PIECE_SIZE );
	if ( nPieceSize > MAX_PIECE_SIZE )
		return FALSE;

	if ( m_pSource.m_nLength == 0 && nInfoPiece == 0 )
	{
		CBENode oRoot;
		if ( GetTrackerCount() > 0 )
		{
			oRoot.Add("announce")->SetString(GetTrackerAddress());

			if ( GetTrackerCount() > 1 )
			{
				CBENode* pList = oRoot.Add("announce-list")->Add();
				for ( int i = 0; i < GetTrackerCount(); i++ )
				{
					pList->Add()->SetString(GetTrackerAddress( i ));
				}
			}

		}
		oRoot.Add("info")->SetInt(0);
		oRoot.Encode(&m_pSource);
		m_pSource.m_nLength -= 4;
		m_nInfoStart = m_pSource.m_nLength;
	}

	QWORD nPieceStart = nInfoPiece * MAX_PIECE_SIZE;

	if ( nPieceStart == ( m_pSource.m_nLength - m_nInfoStart ) )
	{
		m_pSource.Add( pPiece, nPieceSize );

		if ( m_pSource.m_nLength - m_nInfoStart == nInfoSize )
		{
			m_pSource.Add( "e", 1 );
			return LoadTorrentBuffer( &m_pSource );
		}
	}
	return FALSE;
}

int CBTInfo::NextInfoPiece() const
{
	if ( m_pSource.m_nLength == 0 )
		return 0;
	else if ( m_pSource.m_nLength > m_nInfoStart && ! m_nInfoSize )
		return ( m_pSource.m_nLength - m_nInfoStart ) / MAX_PIECE_SIZE;

	return -1;
}

DWORD CBTInfo::GetInfoPiece(DWORD nPiece, BYTE **pInfoPiece) const
{
	DWORD nPiceStart = MAX_PIECE_SIZE * nPiece;
	if ( m_nInfoSize && m_nInfoStart &&
		m_pSource.m_nLength > m_nInfoStart + m_nInfoSize &&
		nPiceStart < m_nInfoSize )
	{
		*pInfoPiece = &m_pSource.m_pBuffer[ m_nInfoStart + nPiceStart ];
		DWORD nPiceSize = m_nInfoSize - nPiceStart;
		return nPiceSize > MAX_PIECE_SIZE ? MAX_PIECE_SIZE : nPiceSize;
	}
	return 0;
}

DWORD CBTInfo::GetInfoSize() const
{
	return m_nInfoSize;
}

BOOL CBTInfo::CheckInfoData()
{
	ASSERT( m_pSource.m_nLength );

	if ( ! m_pSource.m_nLength ) return FALSE;

	auto_ptr< CBENode > pNode ( CBENode::Decode( &m_pSource ) );
	if ( ! pNode.get() )
		return FALSE;

	const CBENode* pRoot = pNode.get();
	const CBENode* pInfo = pRoot->GetNode( "info" );

	if ( pInfo && pInfo->m_nSize &&
		 pInfo->m_nPosition + pInfo->m_nSize < m_pSource.m_nLength )
	{
		Hashes::BtHash oBTH;
		CSHA pBTH;
		pBTH.Add( &m_pSource.m_pBuffer[pInfo->m_nPosition], (DWORD)pInfo->m_nSize );
		pBTH.Finish();
		pBTH.GetHash( &oBTH[0] );

		if ( oBTH == m_oBTH )
		{
			m_nInfoStart = (DWORD)pInfo->m_nPosition;
			m_nInfoSize	 = (DWORD)pInfo->m_nSize;
			return TRUE;
		}
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo load torrent info from buffer

BOOL CBTInfo::LoadTorrentBuffer(const CBuffer* pBuffer)
{
	auto_ptr< CBENode > pNode ( CBENode::Decode( pBuffer ) );
	if ( ! pNode.get() )
	{
		theApp.Message( MSG_ERROR, _T("[BT] Failed to decode torrent data: %s"), (LPCTSTR)pBuffer->ReadString( (size_t)-1 ) );
		return FALSE;
	}

	return LoadTorrentTree( pNode.get() );
}

//////////////////////////////////////////////////////////////////////
// CBTInfo load torrent info from tree

BOOL CBTInfo::LoadTorrentTree(const CBENode* pRoot)
{
	ASSERT( ! m_pBlockBTH );

	theApp.Message( MSG_DEBUG,
		_T("[BT] Loading torrent tree: %s"), (LPCTSTR)pRoot->Encode() );

	if ( ! pRoot->IsType( CBENode::beDict ) )
		return FALSE;

	// Get the info node
	const CBENode* pInfo = pRoot->GetNode( "info" );
	if ( ! pInfo || ! pInfo->IsType( CBENode::beDict ) )
		return FALSE;

	if ( m_oBTH )
	{
		CSHA oSHA = pInfo->GetSHA1();
		Hashes::BtHash oBTH;
		oSHA.GetHash( &oBTH[ 0 ] );
		oBTH.validate();

		if ( oBTH != m_oBTH )
			return FALSE;
	}

	// Get the encoding (from torrents that have it)
	m_nEncoding = 0;
	const CBENode* pEncoding = pRoot->GetNode( "codepage" );
	if ( pEncoding && pEncoding->IsType( CBENode::beInt ) )
	{
		// "codepage" style (UNIT giving the exact Windows code page)
		m_nEncoding = (UINT)pEncoding->GetInt();
	}
	else
	{
		// "encoding" style (String representing the encoding to use)
		pEncoding = pRoot->GetNode( "encoding" );
		if ( pEncoding && pEncoding->IsType( CBENode::beString ) )
		{
			CString strEncoding = pEncoding->GetString();

			if ( strEncoding.GetLength() < 3 )
				theApp.Message( MSG_ERROR, _T("Torrent 'encoding' node too short") );
			else if ( _tcsistr( strEncoding.GetString() , _T("UTF-8") ) != NULL ||
					  _tcsistr( strEncoding.GetString() , _T("UTF8") ) != NULL )
				m_nEncoding = CP_UTF8;
			else if ( _tcsistr( strEncoding.GetString() , _T("ANSI") ) != NULL )
				m_nEncoding = CP_ACP;
			else if ( _tcsistr( strEncoding.GetString() , _T("BIG5") ) != NULL )
				m_nEncoding = 950;
			else if ( _tcsistr( strEncoding.GetString() , _T("Korean") ) != NULL )
				m_nEncoding = 949;
			else if ( _tcsistr( strEncoding.GetString() , _T("UHC") ) != NULL )
				m_nEncoding = 949;
			else if ( _tcsistr( strEncoding.GetString() , _T("Chinese") ) != NULL )
				m_nEncoding = 936;
			else if ( _tcsistr( strEncoding.GetString() , _T("GB2312") ) != NULL )
				m_nEncoding = 936;
			else if ( _tcsistr( strEncoding.GetString() , _T("GBK") ) != NULL )
				m_nEncoding = 936;
			else if ( _tcsistr( strEncoding.GetString() , _T("Japanese") ) != NULL )
				m_nEncoding = 932;
			else if ( _tcsistr( strEncoding.GetString() , _T("Shift-JIS") ) != NULL )
				m_nEncoding = 932;
			else if ( _tcsnicmp( strEncoding.GetString() , _T("Windows-"), 8 ) == 0 )
			{
				UINT nEncoding = 0;
				strEncoding = strEncoding.Mid( 8 );
				if ( ( _stscanf( strEncoding, _T("%u"), &nEncoding ) == 1 ) && ( nEncoding > 0 ) )
				{
					m_nEncoding = nEncoding;
				}
			}
			else if ( _tcsnicmp( strEncoding.GetString() , _T("CP"), 2 ) == 0 )
			{
				UINT nEncoding = 0;
				strEncoding = strEncoding.Mid( 2 );
				if ( ( _stscanf( strEncoding, _T("%u"), &nEncoding ) == 1 ) && ( nEncoding > 0 ) )
				{
					m_nEncoding = nEncoding;
				}
			}
		}
	}

	// Get the comments (if present)
	m_sComment = pRoot->GetStringFromSubNode( "comment", m_nEncoding );

	// Get the creation date (if present)
	const CBENode* pDate = pRoot->GetNode( "creation date" );
	if ( ( pDate ) &&  ( pDate->IsType( CBENode::beInt )  ) )
	{
		m_tCreationDate = (DWORD)pDate->GetInt();
		// CTime pTime( (time_t)m_tCreationDate );
		// theApp.Message( MSG_NOTICE, pTime.Format( _T("%Y-%m-%d %H:%M:%S") ) );
	}

	// Get the creator (if present)
	m_sCreatedBy = pRoot->GetStringFromSubNode( "created by", m_nEncoding );

	// Get nodes for DHT (if present) BEP 0005
	const CBENode* pNodeList = pRoot->GetNode( "nodes" );
	if ( pNodeList && pNodeList->IsType( CBENode::beList ) )
	{
		for ( int i = 0 ; i < pNodeList->GetCount() ; ++i )
		{
			const CBENode* pNode = pNodeList->GetNode( i );
			if ( pNode && pNode->IsType( CBENode::beList ) && pNode->GetCount() == 2 )
			{
				const CBENode* pHost = pNode->GetNode( 0 );
				const CBENode* pPort = pNode->GetNode( 1 );
				if ( pHost && pHost->IsType( CBENode::beString ) && pPort && pPort->IsType( CBENode::beInt ) )
				{
					CString sHost;
					sHost.Format( _T("%s:%u"), (LPCTSTR)pHost->GetString(), (WORD)pPort->GetInt() );
					m_oNodes.AddTail( sHost );
				}
			}
		}
	}


	// Get announce-list (if present)
	CBENode* pAnnounceList = pRoot->GetNode( "announce-list" );
	if ( ( pAnnounceList ) && ( pAnnounceList->IsType( CBENode::beList ) ) )
	{
		m_nTrackerMode = tMultiFinding;

		// Loop through all the tiers
		for ( int nTier = 0 ; nTier < pAnnounceList->GetCount() ; nTier++ )
		{
			const CBENode* pSubList = pAnnounceList->GetNode( nTier );
			if ( ( pSubList ) && ( pSubList->IsType( CBENode::beList ) ) )
			{
				CList< CString > pTrackers;
				// Read in the trackers
				for ( int nTracker = 0 ; nTracker < pSubList->GetCount() ; nTracker++ )
				{
					const CBENode* pTracker = pSubList->GetNode( nTracker );
					if ( ( pTracker ) &&  ( pTracker->IsType( CBENode::beString )  ) )
					{
						// Get the tracker
						CString strTracker = pTracker->GetString();

						// Check tracker is valid
						if ( StartsWith( strTracker, _PT("http://") ) ||
							 StartsWith( strTracker, _PT("udp://") ) )
						{
							// Store HTTP tracker
							pTrackers.AddTail( strTracker );
						}
						//else unknown tracker
					}
				}

				if ( ! pTrackers.IsEmpty() )
				{
					// Randomize the tracker order in this tier
					if ( pTrackers.GetCount() > 1 )
					{
						for ( POSITION pos = pTrackers.GetHeadPosition() ; pos ; )
						{
							if ( GetRandomNum( 0, 1 ) )
							{
								CString strTemp;
								strTemp = pTrackers.GetAt( pos );
								pTrackers.RemoveAt( pos );

								if ( GetRandomNum( 0, 1 ) )
									pTrackers.AddHead( strTemp );
								else
									pTrackers.AddTail( strTemp );
							}
							pTrackers.GetNext( pos );
						}
					}

					// Store the trackers
					for ( POSITION pos = pTrackers.GetHeadPosition() ; pos ; )
					{
						// Create the tracker and add it to the list
						AddTracker( CBTTracker( pTrackers.GetNext( pos ), nTier ) );
					}
					// Delete temporary storage
					pTrackers.RemoveAll();
				}
			}
		}

		SetTrackerNext();
	}

	// Get announce
	const CBENode* pAnnounce = pRoot->GetNode( "announce" );
	if ( pAnnounce && pAnnounce->IsType( CBENode::beString ) )
	{
		// Get the tracker
		CString strTracker = pAnnounce->GetString();

		// Store it if it's valid. (Some torrents have invalid trackers)
		if ( StartsWith( strTracker, _PT("http://") ) ||
			 StartsWith( strTracker, _PT("udp://") ) )
		{
			// Announce node is ignored by multi-tracker torrents
			if ( m_oTrackers.IsEmpty() )
			{
				// Set the torrent to be a single-tracker torrent
				m_nTrackerMode = tSingle;
				SetTracker( strTracker );
			}
		}
		//else unknown tracker
	}

	// Get the private flag (if present)
	const CBENode* pPrivate = pInfo->GetNode( "private" );
	if ( ( pPrivate ) &&  ( pPrivate->IsType( CBENode::beInt )  ) )
		m_bPrivate = ( pPrivate->GetInt() != 0 );

	// Get the name
	m_sName = pInfo->GetStringFromSubNode( "name", m_nEncoding );

	// If we still don't have a name, generate one
	if ( m_sName.IsEmpty() )
		m_sName.Format( _T("Unnamed_Torrent_%i"), GetRandomNum( 0i32, _I32_MAX ) );

	// Get the piece stuff
	const CBENode* pPL = pInfo->GetNode( "piece length" );
	if ( ! pPL || ! pPL->IsType( CBENode::beInt ) ) return FALSE;
	m_nBlockSize = (DWORD)pPL->GetInt();
	if ( ! m_nBlockSize ) return FALSE;

	const CBENode* pHash = pInfo->GetNode( "pieces" );
	if ( ! pHash || ! pHash->IsType( CBENode::beString ) ) return FALSE;
	if ( pHash->m_nValue % Hashes::Sha1Hash::byteCount ) return FALSE;
	m_nBlockCount = (DWORD)( pHash->m_nValue / Hashes::Sha1Hash::byteCount );
	if ( ! m_nBlockCount || m_nBlockCount > 209716 ) return FALSE;

	m_pBlockBTH = new Hashes::BtPureHash[ m_nBlockCount ];

	std::copy( static_cast< const Hashes::BtHash::RawStorage* >( pHash->m_pValue ),
		static_cast< const Hashes::BtHash::RawStorage* >( pHash->m_pValue ) + m_nBlockCount,
		stdext::make_checked_array_iterator( m_pBlockBTH, m_nBlockCount ) );

	// Hash info
	if ( const CBENode* pSHA1 = pInfo->GetNode( "sha1" ) )
	{
		if ( ! pSHA1->IsType( CBENode::beString ) || pSHA1->m_nValue != Hashes::Sha1Hash::byteCount ) return FALSE;
		m_oSHA1 = *static_cast< const Hashes::BtHash::RawStorage* >( pSHA1->m_pValue );
	}

	// TODO: BitComet LT-Seeding - http://wiki.bitcomet.com/long-term_seeding
	// Long-Term-seeding protocol is proprietary property belonging to the BitComet Development team.
	// Looks like changed SHA1.
	//if ( const CBENode* pSHA1Base16 = pInfo->GetNode( "filehash" ) )
	//{
	//	if ( ! pSHA1Base16->IsType( CBENode::beString ) ||
	//		pSHA1Base16->m_nValue != Hashes::BtGuid::byteCount ) return FALSE;
	//	m_oSHA1 = *static_cast< const Hashes::BtGuid::RawStorage* >( pSHA1Base16->m_pValue );
	//}

	if ( const CBENode* pED2K = pInfo->GetNode( "ed2k" ) )
	{
		if ( ! pED2K->IsType( CBENode::beString ) || pED2K->m_nValue != Hashes::Ed2kHash::byteCount ) return FALSE;
		m_oED2K = *static_cast< const Hashes::Ed2kHash::RawStorage* >( pED2K->m_pValue );
	}

	if ( const CBENode* pMD5 = pInfo->GetNode( "md5sum" ) )
	{
		if ( ! pMD5->IsType( CBENode::beString ) )
		{
			return FALSE;
		}
		else if ( pMD5->m_nValue == Hashes::Md5Hash::byteCount )
		{
			m_oMD5 = *static_cast< const Hashes::Md5Hash::RawStorage* >( pMD5->m_pValue );
		}
		else if ( pMD5->m_nValue == Hashes::Md5Hash::byteCount * 2 )
		{
			CStringA tmp;
			tmp.Append( (const char*)pMD5->m_pValue, (int)pMD5->m_nValue );
			m_oMD5.fromString( CA2W( tmp ) );
		}
		else
		{
			return FALSE;
		}
	}

	if ( const CBENode* pTiger = pInfo->GetNode( "tiger" ) )
	{
		if ( ! pTiger->IsType( CBENode::beString ) || pTiger->m_nValue != Hashes::TigerHash::byteCount ) return FALSE;
		m_oTiger = *static_cast< const Hashes::TigerHash::RawStorage* >( pTiger->m_pValue );
	}

	// Details on file (or files).
	if ( const CBENode* pSingleLength = pInfo->GetNode( "length" ) )
	{
		if ( ! pSingleLength->IsType( CBENode::beInt ) )
			return FALSE;
		m_nSize = (QWORD)pSingleLength->GetInt();
		if ( ! m_nSize )
			return FALSE;

		CAutoPtr< CBTFile >pBTFile( new CBTFile( this ) );
		if ( ! pBTFile )
			// Out of memory
			return FALSE;

		pBTFile->m_sPath = m_sName;
		pBTFile->m_sName = PathFindFileName( m_sName );
		pBTFile->m_nSize = m_nSize;
		pBTFile->m_oSHA1 = m_oSHA1;
		pBTFile->m_oTiger = m_oTiger;
		pBTFile->m_oED2K = m_oED2K;
		pBTFile->m_oMD5 = m_oMD5;
		m_pFiles.AddTail( pBTFile.Detach() );

		// Add sources from torrents - DWK
		if ( const CBENode* pSources = pRoot->GetNode( "sources" ) )
		{
			if( pSources->IsType( CBENode::beList ) )
			{
				const int nSources = pSources->GetCount();
				for ( int nSource = 0 ; nSource < nSources; ++nSource )
				{
					if ( const CBENode* pSource = pSources->GetNode( nSource ) )
						if( pSource->IsType(CBENode::beString) )
							m_sURLs.AddTail( pSource->GetString() );
				}
			}
		}

		// BEP 19 : WebSeed - HTTP/FTP Seeding (GetRight style) : http://bittorrent.org/beps/bep_0019.html
		// TODO: Support multi-file torrents
		if ( const CBENode* pUrlList = pRoot->GetNode( "url-list" ) )
		{
			if ( pUrlList->IsType( CBENode::beList ) )
			{
				const int nUrls = pUrlList->GetCount();
				for ( int nUrl = 0; nUrl < nUrls; ++nUrl )
				{
					if ( const CBENode* pUrl = pUrlList->GetNode( nUrl ) )
						if ( pUrl->IsType( CBENode::beString ) )
							m_sURLs.AddTail( pUrl->GetString() );
				}
			}
		}
	}
	else if ( const CBENode* pFiles = pInfo->GetNode( "files" ) )
	{
		CString strPath;

		if ( ! pFiles->IsType( CBENode::beList ) ) return FALSE;
		int nFiles = pFiles->GetCount();
		if ( ! nFiles || nFiles > 8192 * 8 ) return FALSE;

		m_nSize = 0;

		QWORD nOffset = 0;
		for ( int nFile = 0 ; nFile < nFiles ; nFile++ )
		{
			CAutoPtr< CBTFile > pBTFile( new CBTFile( this ) );
			if ( ! pBTFile )
				// Out of Memory
				return FALSE;

			const CBENode* pFile = pFiles->GetNode( nFile );
			if ( ! pFile || ! pFile->IsType( CBENode::beDict ) ) return FALSE;

			const CBENode* pLength = pFile->GetNode( "length" );
			if ( ! pLength || ! pLength->IsType( CBENode::beInt ) ) return FALSE;
			pBTFile->m_nSize = (QWORD)pLength->GetInt();

			pBTFile->m_nOffset = nOffset;

			strPath.Empty();

			// Try path.utf8 if it's set
			const CBENode* pPath = pFile->GetNode( "path.utf-8" );
			if ( pPath && pPath->IsType( CBENode::beList ) )
			{
				const CBENode* pPart = pPath->GetNode( 0 );
				if ( pPart && pPart->IsType( CBENode::beString ) )
					strPath = pPart->GetString();
			}

			// Get the regular path
			pPath = pFile->GetNode( "path" );
			if ( ! pPath || ! pPath->IsType( CBENode::beList ) ) return FALSE;

			const CBENode* pPathPart = pPath->GetNode( 0 );
			if ( pPathPart && pPathPart->IsType( CBENode::beString ) )
			{
				if ( ! IsValid( strPath ) )
				{
					// Get the path
					strPath = pPathPart->GetString();
				}
				else
				{
					// Check the path matches the .utf path
					CString strCheck =  pPathPart->GetString();
					if ( strPath != strCheck )
						m_bEncodingError = true;
					// Switch back to the UTF-8 path
					pPath = pFile->GetNode( "path.utf-8" );
				}
			}

			// If that didn't work, try decoding the path
			if ( ! IsValid( strPath ) )
			{
				// There was an error reading the path
				m_bEncodingError = true;
				// Open path node
				pPath = pFile->GetNode( "path" );
				if ( pPath )
				{
					const CBENode* pPart = pPath->GetNode( 0 );
					if ( pPart->IsType( CBENode::beString ) )
						strPath = pPart->DecodeString(m_nEncoding);
				}
			}

			if ( ! pPath || ! pPath->IsType( CBENode::beList ) ) return FALSE;
			if ( strPath.CompareNoCase( _T("#ERROR#") ) == 0 ) return FALSE;

			pBTFile->m_sName = PathFindFileName( strPath );

			// Hack to prefix all
			pBTFile->m_sPath = SafeFilename( m_sName );

			for ( int nPath = 0 ; nPath < pPath->GetCount() ; nPath++ )
			{
				const CBENode* pPart = pPath->GetNode( nPath );
				if ( ! pPart || ! pPart->IsType( CBENode::beString ) ) return FALSE;

				const int nPathLength = pBTFile->m_sPath.GetLength();
				if ( nPathLength && pBTFile->m_sPath.GetAt( nPathLength - 1 ) != _T('\\') )
					pBTFile->m_sPath += _T('\\');

				// Get the path

				// Check for encoding error
				if ( pPart->GetString().CompareNoCase( _T("#ERROR#") ) == 0 )
					strPath = SafeFilename( pPart->DecodeString( m_nEncoding ), true );
				else
					strPath = SafeFilename( pPart->GetString(), true );

				pBTFile->m_sPath += strPath;
			}

			if ( const CBENode* pSHA1 = pFile->GetNode( "sha1" ) )
			{
				if ( ! pSHA1->IsType( CBENode::beString ) )
				{
					return FALSE;
				}
				else if ( pSHA1->m_nValue == Hashes::Sha1Hash::byteCount )
				{
					pBTFile->m_oSHA1 =
						*static_cast< const Hashes::Sha1Hash::RawStorage* >( pSHA1->m_pValue );
				}
				else if ( pSHA1->m_nValue == Hashes::Sha1Hash::byteCount * 2 )
				{
					CStringA tmp;
					tmp.Append( (const char*)pSHA1->m_pValue, (int)pSHA1->m_nValue );
					pBTFile->m_oSHA1.fromString( CA2W( tmp ) );
				}
				else
				{
					return FALSE;
				}
			}

			if ( const CBENode* pED2K = pFile->GetNode( "ed2k" ) )
			{
				if ( ! pED2K->IsType( CBENode::beString ) ||
					   pED2K->m_nValue != Hashes::Ed2kHash::byteCount ) return FALSE;
				pBTFile->m_oED2K =
					*static_cast< Hashes::Ed2kHash::RawStorage* >( pED2K->m_pValue );
			}

			if ( const CBENode* pMD5 = pFile->GetNode( "md5sum" ) )
			{
				if ( ! pMD5->IsType( CBENode::beString ) )
				{
					return FALSE;
				}
				else if ( pMD5->m_nValue == Hashes::Md5Hash::byteCount )
				{
					pBTFile->m_oMD5 =
						*static_cast< const Hashes::Md5Hash::RawStorage* >( pMD5->m_pValue );
				}
				else if ( pMD5->m_nValue == Hashes::Md5Hash::byteCount * 2 )
				{
					CStringA tmp;
					tmp.Append( (const char*)pMD5->m_pValue, (int)pMD5->m_nValue );
					pBTFile->m_oMD5.fromString( CA2W( tmp ) );
				}
				else
				{
					return FALSE;
				}
			}

			if ( const CBENode* pTiger = pFile->GetNode( "tiger" ) )
			{
				if ( ! pTiger->IsType( CBENode::beString ) ||
					   pTiger->m_nValue != Hashes::TigerHash::byteCount ) return FALSE;
				pBTFile->m_oTiger =
					*static_cast< Hashes::TigerHash::RawStorage* >( pTiger->m_pValue );
			}

			m_nSize += pBTFile->m_nSize;
			nOffset += pBTFile->m_nSize;

			m_pFiles.AddTail( pBTFile.Detach() );
		}

		if ( nFiles == 1 )
		{
			// Single file in a multi-file torrent

			// Reset the name
			m_sName = strPath;

			// Set data/file hashes (if they aren't)
			CBTFile* pSingleFile = m_pFiles.GetHead();
			if ( pSingleFile->m_oSHA1 )
			{
				m_oSHA1 = pSingleFile->m_oSHA1;
			}
			else if ( m_oSHA1 )
			{
				pSingleFile->m_oSHA1 = m_oSHA1;
			}

			if ( pSingleFile->m_oED2K )
			{
				m_oED2K = pSingleFile->m_oED2K;
			}
			else if ( m_oED2K )
			{
				pSingleFile->m_oED2K = m_oED2K;
			}

			if ( pSingleFile->m_oMD5 )
			{
				m_oMD5 = pSingleFile->m_oMD5;
			}
			else if ( m_oMD5 )
			{
				pSingleFile->m_oMD5 = m_oMD5;
			}

			if ( pSingleFile->m_oTiger )
			{
				m_oTiger = pSingleFile->m_oTiger;
			}
			else if ( m_oTiger )
			{
				pSingleFile->m_oTiger = m_oTiger;
			}
		}
	}
	else
	{
		return FALSE;
	}

	if ( ( m_nSize + m_nBlockSize - 1 ) / m_nBlockSize != m_nBlockCount )
		return FALSE;

	if ( ! CheckFiles() ) return FALSE;

	CSHA oSHA = pInfo->GetSHA1();
	oSHA.GetHash( &m_oBTH[ 0 ] );
	m_oBTH.validate();

	if ( m_pSource.m_nLength > 0
		 && pInfo->m_nSize
		 && pInfo->m_nPosition + pInfo->m_nSize < m_pSource.m_nLength )
	{
		Hashes::BtHash oBTH;
		CSHA pBTH;
		pBTH.Add( &m_pSource.m_pBuffer[pInfo->m_nPosition], (DWORD)pInfo->m_nSize );
		pBTH.Finish();
		pBTH.GetHash( &oBTH[0] );

		if ( oBTH == m_oBTH )
		{
			m_nInfoStart = (DWORD)pInfo->m_nPosition;
			m_nInfoSize	 = (DWORD)pInfo->m_nSize;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo load torrent info from tree

BOOL CBTInfo::CheckFiles()
{
	for ( POSITION pos = m_pFiles.GetHeadPosition(); pos ; )
	{
		CBTFile* pBTFile = m_pFiles.GetNext( pos );
		pBTFile->m_sPath.Trim();

		LPCTSTR pszPath = pBTFile->m_sPath;

		if ( pszPath == NULL || *pszPath == 0 ) return FALSE;
		if ( pszPath[1] == ':' ) return FALSE;
		if ( *pszPath == '\\' || *pszPath == '/' ) return FALSE;
		if ( _tcsstr( pszPath, _T("..\\") ) != NULL ) return FALSE;
		if ( _tcsstr( pszPath, _T("../") ) != NULL ) return FALSE;
	}

	return m_pFiles.GetCount() > 0;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo block testing

void CBTInfo::BeginBlockTest()
{
	ASSERT( IsAvailable() );
	ASSERT( m_pBlockBTH != NULL );

	m_pTestSHA1.Reset();
	m_nTestByte = 0;
}

void CBTInfo::AddToTest(LPCVOID pInput, DWORD nLength)
{
	if ( nLength == 0 ) return;

	ASSERT( IsAvailable() );
	ASSERT( m_pBlockBTH != NULL );
	ASSERT( m_nTestByte + nLength <= m_nBlockSize );

	m_pTestSHA1.Add( pInput, nLength );
	m_nTestByte += nLength;
}

BOOL CBTInfo::FinishBlockTest(DWORD nBlock)
{
	ASSERT( IsAvailable() );

	if ( m_pBlockBTH == NULL || nBlock >= m_nBlockCount )
		 return FALSE;

	Hashes::BtHash oBTH;
	m_pTestSHA1.Finish();
	m_pTestSHA1.GetHash( &oBTH[ 0 ] );
	oBTH.validate();

	return m_pBlockBTH[ nBlock ] == oBTH;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo tracker handling

void CBTInfo::SetTrackerAccess(DWORD tNow)
{
	// Check that there should be a tracker
	if ( m_oTrackers.IsEmpty() )
		return;

	ASSERT( m_nTrackerIndex >= 0 && m_nTrackerIndex < m_oTrackers.GetCount() );

	// Set the current tracker's access time
	m_oTrackers[ m_nTrackerIndex ].m_tLastAccess = tNow;
}

void CBTInfo::SetTrackerSucceeded(DWORD tNow)
{
	// Check that there should be a tracker
	if ( m_oTrackers.IsEmpty() )
		return;

	ASSERT( m_nTrackerIndex >= 0 && m_nTrackerIndex < m_oTrackers.GetCount() );

	// Set the current tracker's success time
	m_oTrackers[ m_nTrackerIndex ].m_tLastSuccess = tNow;

	// Reset the failure count
	m_oTrackers[ m_nTrackerIndex ].m_nFailures = 0;
}

void CBTInfo::SetTrackerRetry(DWORD tTime)
{
	// Check that there should be a tracker
	if ( ! HasTracker() )
		return;

	ASSERT( m_nTrackerIndex >= 0 && m_nTrackerIndex < m_oTrackers.GetCount() );

	// Set the current tracker's next allowable access attempt time
	m_oTrackers[ m_nTrackerIndex ].m_tNextTry = tTime;
}

void CBTInfo::SetTrackerNext(DWORD tTime)
{
	if ( m_oTrackers.IsEmpty() )
	{
		m_nTrackerMode = tNull;
		m_nTrackerIndex = -1;
		return;
	}

	if ( m_nTrackerMode == tNull || m_nTrackerMode == tSingle )
		return;

	// Make sure this is a multitracker torrent
	if ( m_oTrackers.GetCount() < 2 )
	{
		m_nTrackerMode = tSingle;
		m_nTrackerIndex = 0;
		return;
	}

	// Get current time
	if ( !tTime )
		tTime = GetTickCount();

	// Set current mode to searching
	m_nTrackerMode = tMultiFinding;

	// Start with the first tracker in the list
	m_nTrackerIndex = 0;

	// Search through the list for an available tracker or the first one that
	// will become available
	for ( int nTracker = 0; nTracker < m_oTrackers.GetCount(); ++nTracker )
	{
		// Get the next tracker in the list
		CBTTracker& oTracker = m_oTrackers[ nTracker ];

		// If it's available, reset the retry time
		if ( oTracker.m_tNextTry < tTime )
			oTracker.m_tNextTry = 0;

		// If this tracker will become available before the current one, make
		// it the current tracker
		if ( m_oTrackers[ m_nTrackerIndex ].m_tNextTry > oTracker.m_tNextTry )
			m_nTrackerIndex = nTracker;
	}
}

DWORD CBTInfo::GetTrackerFailures() const
{
	if ( ! HasTracker() )
		return 0;

	ASSERT( m_nTrackerIndex >= 0 && m_nTrackerIndex < m_oTrackers.GetCount() );

	// Return the # of failures
	return m_oTrackers[ m_nTrackerIndex ].m_nFailures;
}

CString CBTInfo::GetTrackerAddress(int nTrackerIndex) const
{
	if ( m_oTrackers.IsEmpty() )
		return CString();

	if ( nTrackerIndex == -1 )
		nTrackerIndex = m_nTrackerIndex;

	if ( nTrackerIndex == -1 )
		return CString();

	ASSERT( nTrackerIndex >= 0 && nTrackerIndex < m_oTrackers.GetCount() );

	return m_oTrackers[ nTrackerIndex ].m_sAddress;
}

TRISTATE CBTInfo::GetTrackerStatus(int nTrackerIndex) const
{
	if ( m_oTrackers.IsEmpty() )
		return TRI_UNKNOWN;

	if ( nTrackerIndex == -1 )
		nTrackerIndex = m_nTrackerIndex;

	if ( nTrackerIndex == -1 )
		return TRI_UNKNOWN;

	ASSERT( nTrackerIndex >= 0 && nTrackerIndex < m_oTrackers.GetCount() );

	if ( ! m_oTrackers[ nTrackerIndex ].m_tNextTry &&
		 ! m_oTrackers[ nTrackerIndex ].m_tLastSuccess )
		return TRI_UNKNOWN;
	else if ( m_oTrackers[ nTrackerIndex ].m_tNextTry >
		m_oTrackers[ nTrackerIndex ].m_tLastSuccess )
		return TRI_FALSE;
	else
		return TRI_TRUE;
}

int CBTInfo::GetTrackerTier(int nTrackerIndex) const
{
	if ( m_oTrackers.IsEmpty() )
		return 0;

	if ( nTrackerIndex == -1 )
		nTrackerIndex = m_nTrackerIndex;

	if ( nTrackerIndex == -1 )
		return 0;

	ASSERT( nTrackerIndex >= 0 && nTrackerIndex < m_oTrackers.GetCount() );

	return m_oTrackers[ nTrackerIndex ].m_nTier;
}

DWORD CBTInfo::GetTrackerNextTry() const
{
	if ( ! HasTracker() )
		return (DWORD)-1;

	ASSERT( m_nTrackerIndex >= 0 && m_nTrackerIndex < m_oTrackers.GetCount() );

	return m_oTrackers[ m_nTrackerIndex ].m_tNextTry;
}

void CBTInfo::OnTrackerFailure()
{
	if ( ! HasTracker() )
		return;

	ASSERT( m_nTrackerIndex >= 0 && m_nTrackerIndex < m_oTrackers.GetCount() );

	m_oTrackers[ m_nTrackerIndex ].m_nFailures++;
}

void CBTInfo::SetTracker(const CString& sTracker)
{
	m_nTrackerIndex = AddTracker( CBTTracker( sTracker ) );
}

void CBTInfo::SetNode(const CString& sNode)
{
	m_oNodes.AddTail( sNode );
}

void CBTInfo::SetTrackerMode(int nTrackerMode)
{
	// Check it's valid
	INT_PTR nCount = m_oTrackers.GetCount();
	if ( ( nTrackerMode == CBTInfo::tMultiFound		&& nCount > 1 ) ||
		 ( nTrackerMode == CBTInfo::tMultiFinding	&& nCount > 1 ) ||
		 ( nTrackerMode == CBTInfo::tSingle			&& nCount > 0 ) ||
		   nTrackerMode == CBTInfo::tNull )
	{
		m_nTrackerMode = nTrackerMode;

		if ( nTrackerMode == CBTInfo::tNull )
			m_nTrackerIndex = -1;
		else if ( m_nTrackerIndex == -1 )
			SetTrackerNext();
	}
}

int CBTInfo::AddTracker(const CBTTracker& oTracker)
{
	for ( int i = 0; i < (int)m_oTrackers.GetCount(); ++i )
		if ( m_oTrackers[ i ] == oTracker )
			// Already have
			return i;

	return (int)m_oTrackers.Add( oTracker );
}

void CBTInfo::RemoveAllTrackers()
{
	m_nTrackerIndex = -1;
	m_nTrackerMode = CBTInfo::tNull;
	m_oTrackers.RemoveAll();
}

CString CBTInfo::GetTrackerHash() const
{
	// Get concatenated tracker URLs list sorted in alphabetical order
	string_set oAddr;
	int nCount = (int)m_oTrackers.GetCount();
	for ( int i = 0; i < nCount; ++i )
		oAddr.insert( m_oTrackers[ i ].m_sAddress );
	CStringA sAddress;
	for( string_set::const_iterator i = oAddr.begin(); i != oAddr.end(); ++i )
		sAddress += CT2A( (*i) );

	// Get SHA1 of it
	CSHA oSHA;
	oSHA.Add( (LPCSTR)sAddress, sAddress.GetLength() );
	oSHA.Finish();

	Hashes::Sha1Hash oSHA1;
	oSHA.GetHash( &oSHA1[ 0 ] );
	oSHA1.validate();

	// Return hex-encoded hash
	return oSHA1.toString< Hashes::base16Encoding >();
}

//////////////////////////////////////////////////////////////////////
// CBTInfo::CBTTracker construction

CBTInfo::CBTTracker::CBTTracker(LPCTSTR szAddress, INT nTier)
	: m_sAddress		( szAddress ? szAddress : _T("") )
	, m_tLastAccess		( 0 )
	, m_tLastSuccess	( 0 )
	, m_tNextTry		( 0 )
	, m_nFailures		( 0 )
	, m_nTier			( nTier )
	, m_nType			( 0 )
{
}

CBTInfo::CBTTracker::CBTTracker(const CBTTracker& oSource)
	: m_sAddress		( oSource.m_sAddress )
	, m_tLastAccess		( oSource.m_tLastAccess )
	, m_tLastSuccess	( oSource.m_tLastSuccess )
	, m_tNextTry		( oSource.m_tNextTry )
	, m_nFailures		( oSource.m_nFailures )
	, m_nTier			( oSource.m_nTier )
	, m_nType			( oSource.m_nType )
{
}

CBTInfo::CBTTracker& CBTInfo::CBTTracker::operator=(const CBTTracker& oSource)
{
	m_sAddress			= oSource.m_sAddress;
	m_tLastAccess		= oSource.m_tLastAccess;
	m_tLastSuccess		= oSource.m_tLastSuccess;
	m_tNextTry			= oSource.m_tNextTry;
	m_nFailures			= oSource.m_nFailures;
	m_nTier				= oSource.m_nTier;
	m_nType				= oSource.m_nType;

	return *this;
}

bool CBTInfo::CBTTracker::operator==(const CBTTracker& oSource)
{
	return ( m_sAddress == oSource.m_sAddress );
}

//////////////////////////////////////////////////////////////////////
// CBTInfo::CBTTracker serialize

void CBTInfo::CBTTracker::Serialize(CArchive& ar, int /*nVersion*/)
{
	if ( ar.IsStoring() )
	{
		ar << m_sAddress;
		ar << m_tLastAccess;
		ar << m_tLastSuccess;
		ar << m_tNextTry;
		ar << m_nFailures;
		ar << m_nTier;
		ar << m_nType;
	}
	else
	{
		ar >> m_sAddress;
		ar >> m_tLastAccess;
		ar >> m_tLastSuccess;
		ar >> m_tNextTry;
		ar >> m_nFailures;
		ar >> m_nTier;
		ar >> m_nType;
	}
}
