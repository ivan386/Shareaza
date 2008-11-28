//
// BTInfo.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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
#include "BTInfo.h"
#include "BENode.h"
#include "Buffer.h"
#include "DownloadTask.h"
#include "Download.h"
#include "Downloads.h"
#include "FragmentedFile.h"
#include "Transfers.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "Library.h"

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

CBTInfo::CBTInfo() :
	m_nTotalSize		( 0ull )
,	m_nBlockSize		( 0ul )
,	m_nBlockCount		( 0ul )
,	m_pBlockBTH			( NULL )
,	m_nTotalUpload		( 0ull )
,	m_nTotalDownload	( 0ull )
,	m_nTrackerIndex		( -1 )
,	m_nTrackerMode		( tNull )
,	m_nEncoding			( Settings.BitTorrent.TorrentCodePage )
,	m_tCreationDate		( 0ul )
,	m_bPrivate			( FALSE )
,	m_nStartDownloads	( dtAlways )
,	m_bEncodingError	( false )
,	m_nTestByte			( 0ul )
{
}

CBTInfo::CBTInfo(const CBTInfo& oSource) :
	m_nTotalSize		( 0ull )
,	m_nBlockSize		( 0ul )
,	m_nBlockCount		( 0ul )
,	m_pBlockBTH			( NULL )
,	m_nTotalUpload		( 0ull )
,	m_nTotalDownload	( 0ull )
,	m_nTrackerIndex		( -1 )
,	m_nTrackerMode		( tNull )
,	m_nEncoding			( Settings.BitTorrent.TorrentCodePage )
,	m_tCreationDate		( 0ul )
,	m_bPrivate			( FALSE )
,	m_nStartDownloads	( dtAlways )
,	m_bEncodingError	( false )
,	m_nTestByte			( 0ul )
{
	Copy( oSource );
}

CBTInfo::~CBTInfo()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CBTFile construction

CBTInfo::CBTFile::CBTFile(const CBTInfo* pInfo, const CBTFile* pBTFile) :
	m_pInfo				( pInfo )
,	m_nFilePriority		( pBTFile ? pBTFile->m_nFilePriority : CBTInfo::prNormal )
,	m_nOffset			( pBTFile ? pBTFile->m_nOffset : 0 )
{
	if ( pBTFile )
	{
		m_sName			= pBTFile->m_sName;
		m_nSize			= pBTFile->m_nSize;
		m_oSHA1			= pBTFile->m_oSHA1;
		m_oTiger		= pBTFile->m_oTiger;
		m_oED2K			= pBTFile->m_oED2K;
		m_oBTH			= pBTFile->m_oBTH;
		m_oMD5			= pBTFile->m_oMD5;
		m_sPath			= pBTFile->m_sPath;
		m_sURL			= pBTFile->m_sURL;
	}
}

CString	CBTInfo::CBTFile::FindFile()
{
	CSingleLock oLibraryLock( &Library.m_pSection, TRUE );

	// Try find file by hash/size
	CString strFile;
	CLibraryFile* pShared = LibraryMaps.LookupFileByHash( m_oSHA1, m_oTiger,
		m_oED2K, m_oBTH, m_oMD5, m_nSize, m_nSize, FALSE, TRUE );
	if ( pShared )
		strFile = pShared->GetPath();
	if ( ! pShared ||
		 GetFileSize( CString( _T("\\\\?\\") ) + strFile ) != m_nSize )
	{
		// Try complete folder
		strFile = Settings.Downloads.CompletePath + _T("\\") + m_sPath;
		if ( GetFileSize( CString( _T("\\\\?\\") ) + strFile ) != m_nSize )
		{
			// Try folder of original .torrent
			CString strTorrentPath = m_pInfo->m_sPath.Left(
				m_pInfo->m_sPath.ReverseFind( _T('\\') ) + 1 );
			strFile = strTorrentPath + m_sPath;
			if ( GetFileSize( CString( _T("\\\\?\\") ) + strFile ) != m_nSize )
			{
				// Try complete folder without outer file directory
				CString strShortPath;
				int nSlash = m_sPath.Find( _T('\\') );
				if ( nSlash != -1 )
					strShortPath = m_sPath.Mid( nSlash + 1 );
				strFile = Settings.Downloads.CompletePath + _T("\\") + strShortPath;
				if ( strShortPath.IsEmpty() ||
					 GetFileSize( CString( _T("\\\\?\\") ) + strFile ) != m_nSize )
				{
					// Try folder of original .torrent without outer file directory
					strFile = strTorrentPath + strShortPath;
					if ( strShortPath.IsEmpty() ||
						GetFileSize( CString( _T("\\\\?\\") ) + strFile ) != m_nSize )
					{
						// Try find by name only
						pShared = LibraryMaps.LookupFileByName( m_sName, FALSE, TRUE );
						if ( pShared )
							strFile = pShared->GetPath();
						if ( ! pShared ||
							 GetFileSize( CString( _T("\\\\?\\") ) + strFile ) != m_nSize )
						{
							return CString();
						}
					}
				}
			}
		}
	}

	// Refill missed hashes
	if ( ! pShared )
		pShared = LibraryMaps.LookupFileByPath( strFile, FALSE, FALSE );
	if ( pShared )
	{
		if ( ! m_oSHA1 && pShared->m_oSHA1 )
			m_oSHA1 = pShared->m_oSHA1;
		if ( ! m_oTiger && pShared->m_oTiger )
			m_oTiger = pShared->m_oTiger;
		if ( ! m_oED2K && pShared->m_oED2K )
			m_oED2K = pShared->m_oED2K;
		if ( ! m_oBTH && pShared->m_oBTH )
			m_oBTH = pShared->m_oBTH;
		if ( ! m_oMD5 && pShared->m_oMD5 )
			m_oMD5 = pShared->m_oMD5;
	}

	return strFile;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo clear

void CBTInfo::Clear()
{
	delete [] m_pBlockBTH;

	for ( POSITION pos = m_pFiles.GetHeadPosition(); pos; )
		delete m_pFiles.GetNext( pos );
	m_pFiles.RemoveAll();

    m_oBTH.clear();
	m_oSHA1.clear();
	m_oTiger.clear();
	m_oED2K.clear();
	m_oMD5.clear();
	m_nTotalSize	= 0;
	m_nBlockSize	= 0;
	m_nBlockCount	= 0;
	m_pBlockBTH 	= NULL;
	m_oTrackers.RemoveAll();
	m_nTrackerIndex	= -1;
	m_nTrackerMode	= tNull;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo copy

CBTInfo& CBTInfo::Copy(const CBTInfo& oSource)
{
	Clear();
	
	m_bEncodingError	= oSource.m_bEncodingError;
	m_oBTH				= oSource.m_oBTH;
	m_oSHA1				= oSource.m_oSHA1;
	m_oED2K				= oSource.m_oED2K;
	m_oTiger			= oSource.m_oTiger;
	m_oMD5				= oSource.m_oMD5;
	m_nTotalSize		= oSource.m_nTotalSize;
	m_nBlockSize		= oSource.m_nBlockSize;
	m_nBlockCount		= oSource.m_nBlockCount;
	m_nTotalUpload		= oSource.m_nTotalUpload;
	m_nTotalDownload	= oSource.m_nTotalDownload;
	m_sName				= oSource.m_sName;
	m_sPath				= oSource.m_sPath;

	m_oTrackers.RemoveAll();
	for ( INT_PTR i = 0; i < oSource.m_oTrackers.GetCount(); ++i )
		m_oTrackers.Add( oSource.m_oTrackers[ i ] );

	m_nTrackerIndex		= oSource.m_nTrackerIndex;
	m_nTrackerMode		= oSource.m_nTrackerMode;
	m_nEncoding			= oSource.m_nEncoding;
	m_sComment			= oSource.m_sComment;
	m_tCreationDate		= oSource.m_tCreationDate;
	m_sCreatedBy		= oSource.m_sCreatedBy;
	m_bPrivate			= oSource.m_bPrivate;
	m_nStartDownloads	= oSource.m_nStartDownloads;
	
	if ( oSource.m_pBlockBTH != NULL )
	{
		m_pBlockBTH = new Hashes::BtPureHash[ m_nBlockCount ];
		std::copy( oSource.m_pBlockBTH, oSource.m_pBlockBTH + m_nBlockCount, m_pBlockBTH );
	}
	
	// Copy files
	for ( POSITION pos = oSource.m_pFiles.GetHeadPosition(); pos; )
	{
		CBTFile* pBTFile = new CBTFile( this, oSource.m_pFiles.GetNext( pos ) );
		m_pFiles.AddTail( pBTFile );
	}

	return *this;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo serialize

#define BTINFO_SER_VERSION 7
// History:
// 7 - redesigned tracker list (ryo-oh-ki)

void CBTInfo::Serialize(CArchive& ar)
{
	int nVersion = BTINFO_SER_VERSION;

	if ( ar.IsStoring() )
	{
		ar << nVersion;
		
        SerializeOut( ar, m_oBTH );
        if ( !m_oBTH ) return;
		
		ar << m_nTotalSize;
		ar << m_nBlockSize;
		ar << m_nBlockCount;
        for ( DWORD i = 0; i < m_nBlockCount; ++i )
        {
            ar.Write( m_pBlockBTH[ i ].begin(), Hashes::BtPureHash::byteCount );
        }

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
		
		// removed in v.7
		// ar << m_sTracker;

		ar << m_nTrackerIndex;
		ar << m_nTrackerMode;

		// removed in v.7
		//if ( IsMultiTracker() || m_pAnnounceTracker == NULL )
		//{
		//	ar.WriteCount( 0 );
		//}
		//else
		//{
		//	ar.WriteCount( 1 );
		//	m_pAnnounceTracker->Serialize( ar, nVersion );
		//}

		int nTrackers = (int)m_oTrackers.GetCount();
		ar.WriteCount( nTrackers );
		for ( int nTracker = 0 ; nTracker < nTrackers ; nTracker++ )
		{
			m_oTrackers[ nTracker ].Serialize( ar, nVersion );
		}
	}
	else
	{
		ar >> nVersion;
		if ( nVersion < 1 ) AfxThrowUserException();
		
        SerializeIn( ar, m_oBTH, nVersion );
        if ( !m_oBTH ) return;
		
		if ( nVersion >= 2 )
		{
			ar >> m_nTotalSize;
		}
		else
		{
			DWORD nTotalSize;
			ar >> nTotalSize;
			m_nTotalSize = nTotalSize;
		}
		
		ar >> m_nBlockSize;
		ar >> m_nBlockCount;
		
        m_pBlockBTH = new Hashes::BtPureHash[ (DWORD)m_nBlockCount ];
        for ( DWORD i = 0; i < m_nBlockCount; ++i )
        {
            ReadArchive( ar, m_pBlockBTH[ i ].begin(), Hashes::BtPureHash::byteCount );
        }

		if ( nVersion >= 4 ) ar >> m_nTotalUpload;
		if ( nVersion >= 6 ) ar >> m_nTotalDownload;

		ar >> m_sName;

		if ( nVersion >= 3 )
		{
			ar >> m_nEncoding;
			ar >> m_sComment;
			ar >> m_tCreationDate;
			ar >> m_sCreatedBy;
		}

		if ( nVersion >= 5 ) ar >> m_bPrivate;
		
		int nFiles = (int)ar.ReadCount();
		QWORD nOffset = 0;
		for ( int nFile = 0 ; nFile < nFiles ; nFile++ )
		{
			CBTFile* pBTFile = new CBTFile( this );
			m_pFiles.AddTail( pBTFile );
			pBTFile->Serialize( ar, nVersion );
			pBTFile->m_nOffset = nOffset;
			nOffset += pBTFile->m_nSize;
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
        SerializeOut( ar, m_oSHA1 );
		SerializeOut( ar, m_oED2K );
		SerializeOut( ar, m_oTiger );
		ar << m_nFilePriority;
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
        SerializeIn( ar, m_oSHA1, nVersion );

		if ( nVersion >= 4 )
		{
			SerializeIn( ar, m_oED2K, nVersion );
			SerializeIn( ar, m_oTiger, nVersion );
            ar >> m_nFilePriority;
		}

		if ( nVersion >= 6 )
		{
			SerializeIn( ar, m_oMD5, nVersion );
		}
	}
}

float CBTInfo::CBTFile::GetProgress() const
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	if ( CDownload* pDownload = Downloads.FindByBTH( m_pInfo->m_oBTH ) )
	{
		if ( pDownload->m_pFile )
		{
			return ( (float)pDownload->m_pFile->GetCompleted( m_nOffset, m_nSize ) *
				100.f ) / (float)m_nSize;
		}
	}

	return -1.f;
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

BOOL CBTInfo::SaveTorrentFile(LPCTSTR pszPath) const
{
	ASSERT( pszPath != NULL );
	if ( ! IsAvailable() ) return FALSE;
	if ( m_pSource.m_nLength == 0 ) return FALSE;
	
	CString strPath;
	strPath.Format( _T("%s\\%s.torrent"), pszPath,
		(LPCTSTR)CDownloadTask::SafeFilename( m_sName ) );
	
	CFile pFile;
	if ( ! pFile.Open( strPath, CFile::modeWrite | CFile::modeCreate ) )
		return FALSE;
	
	pFile.Write( m_pSource.m_pBuffer, m_pSource.m_nLength );
	pFile.Close();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo load torrent info from buffer

BOOL CBTInfo::LoadTorrentBuffer(CBuffer* pBuffer)
{
	CBENode* pNode = CBENode::Decode( pBuffer );
	if ( pNode == NULL ) return FALSE;
	BOOL bSuccess = LoadTorrentTree( pNode );
	delete pNode;
	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo load torrent info from tree

BOOL CBTInfo::LoadTorrentTree(CBENode* pRoot)
{
	Clear();
	
	if ( ! pRoot->IsType( CBENode::beDict ) ) return FALSE;

	// Get the encoding (from torrents that have it)
	m_nEncoding = 0;
	CBENode* pEncoding = pRoot->GetNode( "codepage" );
	if ( ( pEncoding ) &&  ( pEncoding->IsType( CBENode::beInt )  ) )
	{
		// "codepage" style (UNIT giving the exact Windows code page)
		m_nEncoding = (UINT)pEncoding->GetInt();
	}
	else
	{
		// "encoding" style (String representing the encoding to use)
		pEncoding = pRoot->GetNode( "encoding" );
		if ( ( pEncoding ) &&  ( pEncoding->IsType( CBENode::beString )  ) )
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
	m_sComment = pRoot->GetStringFromSubNode( "comment", m_nEncoding, m_bEncodingError );

	// Get the creation date (if present)
	CBENode* pDate = pRoot->GetNode( "creation date" );
	if ( ( pDate ) &&  ( pDate->IsType( CBENode::beInt )  ) )
	{
		m_tCreationDate = (DWORD)pDate->GetInt();
		// CTime pTime( (time_t)m_tCreationDate );
		// theApp.Message( MSG_NOTICE, pTime.Format( _T("%Y-%m-%d %H:%M:%S") ) );
	}

	// Get the creator (if present)
	m_sCreatedBy = pRoot->GetStringFromSubNode( "created by", m_nEncoding, m_bEncodingError );

	// Get announce-list (if present)	
	CBENode* pAnnounceList = pRoot->GetNode( "announce-list" );
	if ( ( pAnnounceList ) && ( pAnnounceList->IsType( CBENode::beList ) ) )
	{
		m_nTrackerMode = tMultiFinding;

		// Loop through all the tiers
		for ( int nTier = 0 ; nTier < pAnnounceList->GetCount() ; nTier++ )
		{
			CBENode* pSubList = pAnnounceList->GetNode( nTier );
			if ( ( pSubList ) && ( pSubList->IsType( CBENode::beList ) ) )
			{
				CList< CString > pTrackers;
				// Read in the trackers
				for ( int nTracker = 0 ; nTracker < pSubList->GetCount() ; nTracker++ )
				{
					CBENode* pTracker = pSubList->GetNode( nTracker );
					if ( ( pTracker ) &&  ( pTracker->IsType( CBENode::beString )  ) )
					{
						// Get the tracker
						CString strTracker = pTracker->GetString();

						// Check tracker is valid
						if ( _tcsncicmp( (LPCTSTR)strTracker, _T("http://"), 7 ) == 0 )
						{
							// Store HTTP tracker
							pTrackers.AddTail( strTracker );
						}
						//else if ( _tcsncicmp( (LPCTSTR)strTracker, _T("udp://"), 6 ) == 0 )
						//{
							// TODO: UDP tracker
						//}
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
						CBTTracker oTracker;
						oTracker.m_sAddress	= pTrackers.GetNext( pos );
						oTracker.m_nTier	= nTier;
						AddTracker( oTracker );
					}
					// Delete temporary storage
					pTrackers.RemoveAll();
				}
			}
		}

		SetTrackerNext();
	}

	// Get announce
	CBENode* pAnnounce = pRoot->GetNode( "announce" );
	if ( pAnnounce && pAnnounce->IsType( CBENode::beString ) )
	{
		// Get the tracker
		CString strTracker = pAnnounce->GetString();

		// Store it if it's valid. (Some torrents have invalid trackers)
		if ( _tcsncicmp( (LPCTSTR)strTracker, _T("http://"), 7 ) == 0 ) 
		{
			// Announce node is ignored by multi-tracker torrents
			if ( m_oTrackers.IsEmpty() )
			{
				// Set the torrent to be a single-tracker torrent
				m_nTrackerMode = tSingle;
				SetTracker( strTracker );
			}
		}
		// else if ( _tcsncicmp( (LPCTSTR)strTracker, _T("udp://"), 6 ) == 0 )
		//{
			// TODO: UDP Tracker
		//}
		//else 
		//{
			// TODO: Torrents should always have a valid announce node.
		//}
	}

	// Get the info node
	CBENode* pInfo = pRoot->GetNode( "info" );
	if ( ! pInfo || ! pInfo->IsType( CBENode::beDict ) ) return FALSE;

	// Get the private flag (if present)
	CBENode* pPrivate = pInfo->GetNode( "private" );
	if ( ( pPrivate ) &&  ( pPrivate->IsType( CBENode::beInt )  ) )
		m_bPrivate = pPrivate->GetInt() > 0;
	
	// Get the name
	m_sName = pInfo->GetStringFromSubNode( "name", m_nEncoding, m_bEncodingError );

	// If we still don't have a name, generate one
	if ( m_sName.IsEmpty() ) m_sName.Format( _T("Unnamed_Torrent_%i"), GetRandomNum( (int)0, INT_MAX ) );
	
	// Get the piece stuff
	CBENode* pPL = pInfo->GetNode( "piece length" );
	if ( ! pPL || ! pPL->IsType( CBENode::beInt ) ) return FALSE;
	m_nBlockSize = (DWORD)pPL->GetInt();
	if ( ! m_nBlockSize ) return FALSE;
	
	CBENode* pHash = pInfo->GetNode( "pieces" );
	if ( ! pHash || ! pHash->IsType( CBENode::beString ) ) return FALSE;
	if ( pHash->m_nValue % Hashes::Sha1Hash::byteCount ) return FALSE;
	m_nBlockCount = (DWORD)( pHash->m_nValue / Hashes::Sha1Hash::byteCount );
	if ( ! m_nBlockCount || m_nBlockCount > 209716 ) return FALSE;

	m_pBlockBTH = new Hashes::BtPureHash[ m_nBlockCount ];

	std::copy( static_cast< const Hashes::BtHash::RawStorage* >( pHash->m_pValue ),
		static_cast< const Hashes::BtHash::RawStorage* >( pHash->m_pValue ) + m_nBlockCount,
		m_pBlockBTH );

	// Hash info
	if ( CBENode* pSHA1 = pInfo->GetNode( "sha1" ) )
	{
		if ( ! pSHA1->IsType( CBENode::beString ) || pSHA1->m_nValue != Hashes::Sha1Hash::byteCount ) return FALSE;
		m_oSHA1 = *static_cast< const Hashes::BtHash::RawStorage* >( pSHA1->m_pValue );
	} 
	else if ( CBENode* pSHA1Base16 = pInfo->GetNode( "filehash" ) )
	{
		if ( ! pSHA1Base16->IsType( CBENode::beString ) || 
			pSHA1Base16->m_nValue != Hashes::BtGuid::byteCount ) return FALSE;
		m_oSHA1 = *static_cast< const Hashes::BtGuid::RawStorage* >( pSHA1Base16->m_pValue );
	}
	
	if ( CBENode* pED2K = pInfo->GetNode( "ed2k" ) )
	{
		if ( ! pED2K->IsType( CBENode::beString ) || pED2K->m_nValue != Hashes::Ed2kHash::byteCount ) return FALSE;
		m_oED2K = *static_cast< const Hashes::Ed2kHash::RawStorage* >( pED2K->m_pValue );
	}

	if ( CBENode* pMD5 = pInfo->GetNode( "md5sum" ) )
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

	if ( CBENode* pTiger = pInfo->GetNode( "tiger" ) )
	{
		if ( ! pTiger->IsType( CBENode::beString ) || pTiger->m_nValue != Hashes::TigerHash::byteCount ) return FALSE;
		m_oTiger = *static_cast< const Hashes::TigerHash::RawStorage* >( pTiger->m_pValue );
	}
	
	// Details on file (or files).
	if ( CBENode* pLength = pInfo->GetNode( "length" ) )
	{
		if ( ! pLength->IsType( CBENode::beInt ) ) return FALSE;
		m_nTotalSize = pLength->GetInt();
		if ( ! m_nTotalSize ) return FALSE;
		
		{
			CBTFile* pBTFile = new CBTFile( this );
			m_pFiles.AddTail( pBTFile );
			pBTFile->m_sPath = m_sName;
			pBTFile->m_nSize = m_nTotalSize;
			pBTFile->m_oSHA1 = m_oSHA1;
			pBTFile->m_oTiger = m_oTiger;
			pBTFile->m_oED2K = m_oED2K;
			pBTFile->m_oMD5 = m_oMD5;
		}

		// Add sources from torrents - DWK
		CBENode* pSources = pRoot->GetNode( "sources" );
		if( pSources && pSources->IsType( CBENode::beList ) )
		{
			int m_nSources = pSources->GetCount();
			for( int nSource = 0 ; nSource < m_nSources; nSource++)
			{
				CBENode* pSource = pSources->GetNode( nSource );
				if( ! pSource || ! pSource->IsType(CBENode::beString) ) continue;
				m_sURLs.AddTail( pSource->GetString() );
			}
		}
	}
	else if ( CBENode* pFiles = pInfo->GetNode( "files" ) )
	{
		CString strPath;

		if ( ! pFiles->IsType( CBENode::beList ) ) return FALSE;
		int nFiles = pFiles->GetCount();
		if ( ! nFiles || nFiles > 8192 * 8 ) return FALSE;
		
		m_nTotalSize = 0;

		QWORD nOffset = 0;
		for ( int nFile = 0 ; nFile < nFiles ; nFile++ )
		{
			CBTFile* pBTFile = new CBTFile( this );
			m_pFiles.AddTail( pBTFile );

			CBENode* pFile = pFiles->GetNode( nFile );
			if ( ! pFile || ! pFile->IsType( CBENode::beDict ) ) return FALSE;
			
			CBENode* pLength = pFile->GetNode( "length" );
			if ( ! pLength || ! pLength->IsType( CBENode::beInt ) ) return FALSE;
			pBTFile->m_nSize = pLength->GetInt();

			pBTFile->m_nOffset = nOffset;

			strPath.Empty();
			CBENode* pPath;
			// Try path.utf8 if it's set
			if ( Settings.BitTorrent.TorrentExtraKeys )
			{
				pPath = pFile->GetNode( "path.utf-8" );
				if ( pPath )
				{
					if ( ! pPath->IsType( CBENode::beList ) ) return FALSE;
					if ( pPath->GetCount() > 32 ) return FALSE;
					CBENode* pPart = pPath->GetNode( 0 );
					if ( pPart && pPart->IsType( CBENode::beString ) ) strPath = pPart->GetString();
				}
			}

			// Get the regular path
			pPath = pFile->GetNode( "path" );

			if ( ! pPath ) return FALSE;
			if ( ! pPath->IsType( CBENode::beList ) ) return FALSE;

			CBENode* pPart = pPath->GetNode( 0 );
			if ( pPart && pPart->IsType( CBENode::beString ) ) 
			{
				if ( ! IsValid( strPath ) )
				{
					// Get the path
					strPath = pPart->GetString();
				}
				else
				{
					// Check the path matches the .utf path
					CString strCheck =  pPart->GetString();
					if ( strPath != strCheck )
						m_bEncodingError = TRUE;
					// Switch back to the UTF-8 path
					pPath = pFile->GetNode( "path.utf-8" );
				}
			}

			// If that didn't work, try decoding the path
			if ( ( ! IsValid( strPath ) )  )
			{
				// There was an error reading the path
				m_bEncodingError = TRUE;
				// Open path node
				pPath = pFile->GetNode( "path" );
				if ( pPath )
				{
					CBENode* pPart = pPath->GetNode( 0 );
					if ( pPart->IsType( CBENode::beString ) )
						strPath = pPart->DecodeString(m_nEncoding);
				}
			}
		
			if ( ! pPath ) return FALSE;
			if ( ! pPath->IsType( CBENode::beList ) ) return FALSE;
			if ( pPath->GetCount() > 32 ) return FALSE;
			if ( _tcsicmp( strPath.GetString() , _T("#ERROR#") ) == 0 ) return FALSE;

			pBTFile->m_sName = PathFindFileName( strPath );

			// Hack to prefix all
			pBTFile->m_sPath = CDownloadTask::SafeFilename( m_sName );
			
			for ( int nPath = 0 ; nPath < pPath->GetCount() ; nPath++ )
			{
				CBENode* pPart = pPath->GetNode( nPath );
				if ( ! pPart || ! pPart->IsType( CBENode::beString ) ) return FALSE;
				
				if ( pBTFile->m_sPath.GetLength() )
					pBTFile->m_sPath += '\\';

				// Get the path
				strPath = pPart->GetString();
				strPath = CDownloadTask::SafeFilename( pPart->GetString() );
				// Check for encoding error
				if ( _tcsicmp( strPath.GetString() , _T("#ERROR#") ) == 0 )
					strPath = CDownloadTask::SafeFilename( pPart->DecodeString( m_nEncoding ) );

				pBTFile->m_sPath += strPath;
			}
			
			if ( CBENode* pSHA1 = pFile->GetNode( "sha1" ) )
			{
				if ( ! pSHA1->IsType( CBENode::beString ) ||
					   pSHA1->m_nValue != Hashes::Sha1Hash::byteCount ) return FALSE;
				pBTFile->m_oSHA1 = 
					*static_cast< Hashes::Sha1Hash::RawStorage* >( pSHA1->m_pValue );
			}

			if ( CBENode* pED2K = pFile->GetNode( "ed2k" ) )
			{
				if ( ! pED2K->IsType( CBENode::beString ) ||
					   pED2K->m_nValue != Hashes::Ed2kHash::byteCount ) return FALSE;
				pBTFile->m_oED2K = 
					*static_cast< Hashes::Ed2kHash::RawStorage* >( pED2K->m_pValue );
			}

			if ( CBENode* pMD5 = pFile->GetNode( "md5sum" ) )
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

			if ( CBENode* pTiger = pFile->GetNode( "tiger" ) )
			{
				if ( ! pTiger->IsType( CBENode::beString ) ||
					   pTiger->m_nValue != Hashes::TigerHash::byteCount ) return FALSE;
				pBTFile->m_oTiger = 
					*static_cast< Hashes::TigerHash::RawStorage* >( pTiger->m_pValue );
			}

			m_nTotalSize += pBTFile->m_nSize;
			nOffset += pBTFile->m_nSize;
		}

		if ( nFiles == 1 )
		{
			// Single file in a multi-file torrent

			// Reset the name
			m_sName = strPath;

			// Set data/file hashes (if they aren't)
			if ( m_pFiles.GetHead()->m_oSHA1 )
			{
				m_oSHA1 = m_pFiles.GetHead()->m_oSHA1;
			}
			else if ( m_oSHA1 )
			{
				m_pFiles.GetHead()->m_oSHA1 = m_oSHA1;
			}

			if ( m_pFiles.GetHead()->m_oED2K )
			{
				m_oED2K = m_pFiles.GetHead()->m_oED2K;
			}
			else if ( m_oED2K )
			{
				m_pFiles.GetHead()->m_oED2K = m_oED2K;
			}

			if ( m_pFiles.GetHead()->m_oMD5 )
			{
				m_oMD5 = m_pFiles.GetHead()->m_oMD5;
			}
			else if ( m_oMD5 )
			{
				m_pFiles.GetHead()->m_oMD5 = m_oMD5;
			}

			if ( m_pFiles.GetHead()->m_oTiger )
			{
				m_oTiger = m_pFiles.GetHead()->m_oTiger;
			}
			else if ( m_oTiger )
			{
				m_pFiles.GetHead()->m_oTiger = m_oTiger;
			}
		}
	}
	else
	{
		return FALSE;
	}
	
	if ( ( m_nTotalSize + m_nBlockSize - 1 ) / m_nBlockSize != m_nBlockCount )
		return FALSE;
	
	if ( ! CheckFiles() ) return FALSE;
	
	pInfo->GetBth( m_oBTH );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo load torrent info from tree

BOOL CBTInfo::CheckFiles()
{
	for ( POSITION pos = m_pFiles.GetHeadPosition(); pos ; )
	{
		CBTFile* pBTFile = m_pFiles.GetNext( pos );
		pBTFile->m_sPath.TrimLeft();
		pBTFile->m_sPath.TrimRight();
		
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
	ASSERT( m_pBlockBTH != NULL );
	
	if ( nBlock >= m_nBlockCount ) return FALSE;

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
	if ( m_oTrackers.IsEmpty() )
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
	if ( ! tTime )
		tTime = GetTickCount();

	// Set current mode to searching
	m_nTrackerMode = tMultiFinding;

	// Search through the list for an available tracker or the first one that
	// will become available
	int nBestTracker = 0;
	for ( int nTracker = 0; nTracker < m_oTrackers.GetCount(); nTracker++ )
	{
		// Get the next tracker in the list
		CBTTracker& oTracker = m_oTrackers.GetAt( nTracker );

		// If this tracker will become available before the current one, make
		// it the current tracker
		if ( oTracker.m_tNextTry < m_oTrackers[ nBestTracker ].m_tNextTry )
		{
			nBestTracker = nTracker;
		}
	}

	m_nTrackerIndex = nBestTracker;
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
	CBTTracker oTracker;
	oTracker.m_sAddress = sTracker;
	m_nTrackerIndex = AddTracker( oTracker );
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
		if ( m_oTrackers[ i ].m_sAddress == oTracker.m_sAddress )
			// Already have
			return i;

	return (int)m_oTrackers.Add( oTracker );
}

//////////////////////////////////////////////////////////////////////
// CBTInfo::CBTTracker construction

CBTInfo::CBTTracker::CBTTracker() :
	m_tLastAccess		( 0 )
,	m_tLastSuccess		( 0 )
,	m_tNextTry			( 0 )
,	m_nFailures			( 0 )
,	m_nTier				( 0 )
,	m_nType				( 0 )
{
}

CBTInfo::CBTTracker::CBTTracker(const CBTTracker& oSource) :
	m_sAddress			( oSource.m_sAddress )
,	m_tLastAccess		( oSource.m_tLastAccess )
,	m_tLastSuccess		( oSource.m_tLastSuccess )
,	m_tNextTry			( oSource.m_tNextTry )
,	m_nFailures			( oSource.m_nFailures )
,	m_nTier				( oSource.m_nTier )
,	m_nType				( oSource.m_nType )
{
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
