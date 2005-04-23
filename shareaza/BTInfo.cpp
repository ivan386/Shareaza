//
// BTInfo.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
#include "BTInfo.h"
#include "BENode.h"
#include "Buffer.h"

#include "DownloadTask.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CBTInfo construction

CBTInfo::CBTInfo()
{
	m_bValid		= FALSE;
	m_bEncodingError= FALSE;
	m_bDataSHA1		= FALSE;
	m_bDataED2K		= FALSE;
	m_bDataTiger	= FALSE;
	m_nTotalSize	= 0;
	m_nBlockSize	= 0;
	m_nBlockCount	= 0;
	m_pBlockSHA1	= NULL;
	m_nFiles		= 0;
	m_pFiles		= NULL;

	m_nEncoding		= Settings.BitTorrent.TorrentCodePage;
	m_tCreationDate	= 0;
}

CBTInfo::~CBTInfo()
{
	Clear();
}

CBTInfo::CBTFile::CBTFile()
{
	m_nSize		= 0;
	m_bSHA1		= FALSE;
	m_bED2K		= FALSE;
	m_bTiger	= FALSE;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo clear

void CBTInfo::Clear()
{
	if ( m_pBlockSHA1 != NULL ) delete [] m_pBlockSHA1;
	if ( m_pFiles != NULL ) delete [] m_pFiles;
	
	m_bValid		= FALSE;
	m_nTotalSize	= 0;
	m_nBlockSize	= 0;
	m_nBlockCount	= 0;
	m_pBlockSHA1	= NULL;
	m_nFiles		= 0;
	m_pFiles		= NULL;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo copy

void CBTInfo::Copy(CBTInfo* pSource)
{
	ASSERT( pSource != NULL );
	Clear();
	
	m_bValid		= pSource->m_bValid;
	m_bEncodingError= pSource->m_bEncodingError;
	m_pInfoSHA1		= pSource->m_pInfoSHA1;
	m_bDataSHA1		= pSource->m_bDataSHA1;
	m_pDataSHA1		= pSource->m_pDataSHA1;
	m_bDataED2K		= pSource->m_bDataED2K;
	m_pDataED2K		= pSource->m_pDataED2K;
	m_bDataTiger	= pSource->m_bDataTiger;
	m_pDataTiger	= pSource->m_pDataTiger;

	m_nTotalSize	= pSource->m_nTotalSize;
	m_nBlockSize	= pSource->m_nBlockSize;
	m_nBlockCount	= pSource->m_nBlockCount;

	m_sName			= pSource->m_sName;
	m_sTracker		= pSource->m_sTracker;
	m_nFiles		= pSource->m_nFiles;

	m_nEncoding		= pSource->m_nEncoding;
	m_sComment		= pSource->m_sComment;
	m_tCreationDate	= pSource->m_tCreationDate;
	m_sCreatedBy	= pSource->m_sCreatedBy;
	
	if ( pSource->m_pBlockSHA1 != NULL )
	{
		m_pBlockSHA1 = new SHA1[ m_nBlockCount ];
		CopyMemory( m_pBlockSHA1, pSource->m_pBlockSHA1,
			sizeof(SHA1) * (DWORD)m_nBlockCount );
	}
	
	if ( pSource->m_pFiles != NULL )
	{
		m_pFiles = new CBTFile[ m_nFiles ];
		for ( int nFile = 0 ; nFile < m_nFiles ; nFile++ )
			m_pFiles[ nFile ].Copy( &pSource->m_pFiles[ nFile ] );
	}
}

//////////////////////////////////////////////////////////////////////
// CBTInfo serialize

void CBTInfo::Serialize(CArchive& ar)
{
	int nVersion = 3;
	
	if ( ar.IsStoring() )
	{
		ar << nVersion;
		
		ar << m_bValid;
		if ( ! m_bValid ) return;
		
		ar.Write( &m_pInfoSHA1, sizeof(SHA1) );
		
		ar << m_nTotalSize;
		ar << m_nBlockSize;
		ar << m_nBlockCount;
		ar.Write( m_pBlockSHA1, m_nBlockCount * sizeof(SHA1) );
		
		ar << m_sName;

		ar << m_nEncoding;
		ar << m_sComment;
		ar << m_tCreationDate;
		ar << m_sCreatedBy;
		
		ar.WriteCount( m_nFiles );
		for ( int nFile = 0 ; nFile < m_nFiles ; nFile++ )
			m_pFiles[ nFile ].Serialize( ar, nVersion );
		
		ar << m_sTracker;
	}
	else
	{
		ar >> nVersion;
		if ( nVersion < 1 ) AfxThrowUserException();
		
		ar >> m_bValid;
		if ( ! m_bValid ) return;
		
		ar.Read( &m_pInfoSHA1, sizeof(SHA1) );
		
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
		
		m_pBlockSHA1 = new SHA1[ (DWORD)m_nBlockCount ];
		ar.Read( m_pBlockSHA1, (DWORD)m_nBlockCount * sizeof(SHA1) );
		
		ar >> m_sName;

		if ( nVersion >= 3 )
		{
			ar >> m_nEncoding;
			ar >> m_sComment;
			ar >> m_tCreationDate;
			ar >> m_sCreatedBy;
		}
		
		m_nFiles = ar.ReadCount();
		m_pFiles = new CBTFile[ m_nFiles ];
		for ( int nFile = 0 ; nFile < m_nFiles ; nFile++ )
			m_pFiles[ nFile ].Serialize( ar, nVersion );
		
		ar >> m_sTracker;
	}
}

//////////////////////////////////////////////////////////////////////
// CBTInfo::CBTFile copy

void CBTInfo::CBTFile::Copy(CBTFile* pSource)
{
	m_sPath		= pSource->m_sPath;
	m_nSize		= pSource->m_nSize;
	m_bSHA1		= pSource->m_bSHA1;
	m_pSHA1		= pSource->m_pSHA1;
	m_bED2K		= pSource->m_bED2K;
	m_pED2K		= pSource->m_pED2K;
	m_bTiger	= pSource->m_bTiger;
	m_pTiger	= pSource->m_pTiger;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo::CBTFile serialize

void CBTInfo::CBTFile::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar << m_nSize;
		ar << m_sPath;
		ar << m_bSHA1;
		if ( m_bSHA1 ) ar.Write( &m_pSHA1, sizeof(SHA1) );
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
		ar >> m_bSHA1;
		if ( m_bSHA1 ) ar.Read( &m_pSHA1, sizeof(SHA1) );
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
		
		if ( nLength < 20 * 1024 * 1024 )
		{
			m_pSource.Clear();
			m_pSource.EnsureBuffer( nLength );
			pFile.Read( m_pSource.m_pBuffer, nLength );
			m_pSource.m_nLength = nLength;
			
			return LoadTorrentBuffer( &m_pSource );
		}
	}
	else
	{
		DWORD nError = GetLastError();
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo save .torrent file

BOOL CBTInfo::SaveTorrentFile(LPCTSTR pszPath)
{
	ASSERT( pszPath != NULL );
	if ( ! IsAvailable() ) return FALSE;
	if ( m_pSource.m_nLength == 0 ) return FALSE;
	
	CString strPath;
	strPath.Format( _T("%s\\%s.torrent"), pszPath, (LPCTSTR)CDownloadTask::SafeFilename( m_sName ) );
	
	CFile pFile;
	if ( ! pFile.Open( strPath, CFile::modeWrite | CFile::modeCreate ) ) return FALSE;
	
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
	m_nEncoding = Settings.BitTorrent.TorrentCodePage;
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

			if ( strEncoding.GetLength() < 4 )
				theApp.Message( MSG_ERROR, _T("Torrent 'encoding' node too short") );
			else if ( _tcsistr( strEncoding.GetString() , _T("UTF-8") ) != NULL ) 
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
	m_sComment = pRoot->GetStringFromSubNode( "comment", m_nEncoding, &m_bEncodingError );

	// Get the creation date (if present)
	CBENode* pDate = pRoot->GetNode( "creation date" );
	if ( ( pDate ) &&  ( pDate->IsType( CBENode::beInt )  ) )
	{
		m_tCreationDate = (DWORD)pDate->GetInt();
		// CTime pTime( (time_t)m_tCreationDate );
		// theApp.Message( MSG_SYSTEM, pTime.Format( _T("%Y-%m-%d %H:%M:%S") ) );
	}

	// Get the creator (if present)
	m_sCreatedBy = pRoot->GetStringFromSubNode( "created by", m_nEncoding, &m_bEncodingError );

	// Multi-Tracker: We don't support this yet. (Add it properly later)
	// *********************************
	// Get announce-list (if present)
	CBENode* pAnnounceList = pRoot->GetNode( "announce-list" );
	if ( ( pAnnounceList ) && ( pAnnounceList->IsType( CBENode::beList ) ) )
	{
		CBENode* pSubList = pAnnounceList->GetNode( 0 );
		if ( ( pSubList ) && ( pSubList->IsType( CBENode::beList ) ) )
		{
			CBENode* pTracker = pSubList->GetNode( 0 );
			if ( ( pTracker ) &&  ( pTracker->IsType( CBENode::beString )  ) )
				m_sTracker = pTracker->GetString();
		}
	}
	//*********************************

	// Get announce
	CBENode* pAnnounce = pRoot->GetNode( "announce" );
	if ( pAnnounce->IsType( CBENode::beString ) )
	{
		m_sTracker = pAnnounce->GetString();
		if ( m_sTracker.Find( _T("http") ) != 0 ) m_sTracker.Empty();
	}

	// Get the info node
	CBENode* pInfo = pRoot->GetNode( "info" );
	if ( ! pInfo->IsType( CBENode::beDict ) ) return FALSE;
	
	// Get the name
	m_sName = pInfo->GetStringFromSubNode( "name", m_nEncoding, &m_bEncodingError );
	// If we still don't have a name, generate one
	if ( m_sName.IsEmpty() ) m_sName.Format( _T("Unnamed_Torrent_%i"), (int)rand() );
	
	// Get the piece stuff
	CBENode* pPL = pInfo->GetNode( "piece length" );
	if ( ! pPL->IsType( CBENode::beInt ) ) return FALSE;
	m_nBlockSize = (DWORD)pPL->GetInt();
	if ( ! m_nBlockSize ) return FALSE;
	
	CBENode* pHash = pInfo->GetNode( "pieces" );
	if ( ! pHash->IsType( CBENode::beString ) ) return FALSE;
	if ( pHash->m_nValue % sizeof(SHA1) ) return FALSE;
	m_nBlockCount = (DWORD)( pHash->m_nValue / sizeof(SHA1) );
	if ( ! m_nBlockCount || m_nBlockCount > 209716 ) return FALSE;
	
	m_pBlockSHA1 = new SHA1[ m_nBlockCount ];
	
	for ( DWORD nBlock = 0 ; nBlock < m_nBlockCount ; nBlock++ )
	{
		SHA1* pSource = (SHA1*)pHash->m_pValue;
		CopyMemory( m_pBlockSHA1 + nBlock, pSource + nBlock, sizeof(SHA1) );
	}
	
	// Hash info
	if ( CBENode* pSHA1 = pInfo->GetNode( "sha1" ) )
	{
		if ( ! pSHA1->IsType( CBENode::beString ) || pSHA1->m_nValue != sizeof(SHA1) ) return FALSE;
		m_bDataSHA1 = TRUE;
		CopyMemory( &m_pDataSHA1, pSHA1->m_pValue, sizeof(SHA1) );
	}
	
	if ( CBENode* pED2K = pInfo->GetNode( "ed2k" ) )
	{
		if ( ! pED2K->IsType( CBENode::beString ) || pED2K->m_nValue != sizeof(MD4) ) return FALSE;
		m_bDataED2K = TRUE;
		CopyMemory( &m_pDataED2K, pED2K->m_pValue, sizeof(MD4) );
	}

	if ( CBENode* pTiger = pInfo->GetNode( "tiger" ) )
	{
		if ( ! pTiger->IsType( CBENode::beString ) || pTiger->m_nValue != sizeof(TIGEROOT) ) return FALSE;
		m_bDataTiger = TRUE;
		CopyMemory( &m_pDataTiger, pTiger->m_pValue, sizeof(TIGEROOT) );
	}
	
	// Details on file (or files).
	if ( CBENode* pLength = pInfo->GetNode( "length" ) )
	{
		if ( ! pLength->IsType( CBENode::beInt ) ) return FALSE;
		m_nTotalSize = pLength->GetInt();
		if ( ! m_nTotalSize ) return FALSE;
		
		m_nFiles = 1;
		m_pFiles = new CBTFile[ m_nFiles ];
		m_pFiles[0].m_sPath = m_sName;
		m_pFiles[0].m_nSize = m_nTotalSize;
		m_pFiles[0].m_bSHA1 = m_bDataSHA1;
		m_pFiles[0].m_pSHA1 = m_pDataSHA1;
	}
	else if ( CBENode* pFiles = pInfo->GetNode( "files" ) )
	{
		CString strPath;

		if ( ! pFiles->IsType( CBENode::beList ) ) return FALSE;
		m_nFiles = pFiles->GetCount();
		if ( ! m_nFiles || m_nFiles > 8192 ) return FALSE;
		m_pFiles = new CBTFile[ m_nFiles ];
		
		m_nTotalSize = 0;
		
		for ( int nFile = 0 ; nFile < m_nFiles ; nFile++ )
		{
			CBENode* pFile = pFiles->GetNode( nFile );
			if ( ! pFile->IsType( CBENode::beDict ) ) return FALSE;
			
			CBENode* pLength = pFile->GetNode( "length" );
			if ( ! pLength->IsType( CBENode::beInt ) ) return FALSE;
			m_pFiles[ nFile ].m_nSize = pLength->GetInt();
			
			CBENode* pPath = pFile->GetNode( "path" );
			if ( ! pPath->IsType( CBENode::beList ) ) return FALSE;
			if ( pPath->GetCount() > 32 ) return FALSE;

			// Verify the path is valid
			strPath.Empty();
			strPath = pPath->GetStringFromSubNode( 0,  m_nEncoding, &m_bEncodingError);
			if ( ! IsValid( strPath ) )
			{
				// There was an error reading the path
				m_bEncodingError = TRUE;
				// Check for other possible path nodes
				pPath = pFile->GetNode( "path.utf-8" );
				if ( pPath )
				{
					CBENode* pPart = pPath->GetNode( 0 );
					if ( pPart->IsType( CBENode::beString ) ) strPath = pPart->GetString();
				}
			}
		
			if ( ! pPath ) return FALSE;
			if ( ! pPath->IsType( CBENode::beList ) ) return FALSE;
			if ( pPath->GetCount() > 32 ) return FALSE;
			if ( _tcsicmp( strPath.GetString() , _T("#ERROR#") ) == 0 ) return FALSE;
			// 


			// Hack to prefix all
			m_pFiles[ nFile ].m_sPath = CDownloadTask::SafeFilename( m_sName );
			
			for ( int nPath = 0 ; nPath < pPath->GetCount() ; nPath++ )
			{
				CBENode* pPart = pPath->GetNode( nPath );
				if ( ! pPart->IsType( CBENode::beString ) ) return FALSE;
				
				if ( m_pFiles[ nFile ].m_sPath.GetLength() )
					m_pFiles[ nFile ].m_sPath += '\\';

				// Get the path
				strPath = CDownloadTask::SafeFilename( pPart->GetString() );
				// Check for encoding error
				if ( _tcsicmp( strPath.GetString() , _T("#ERROR#") ) == 0 )
					strPath = CDownloadTask::SafeFilename( pPart->DecodeString( m_nEncoding ) );

				m_pFiles[ nFile ].m_sPath += strPath;
			}
			
			if ( CBENode* pSHA1 = pFile->GetNode( "sha1" ) )
			{
				if ( ! pSHA1->IsType( CBENode::beString ) || pSHA1->m_nValue != sizeof(SHA1) ) return FALSE;
				m_pFiles[ nFile ].m_bSHA1 = TRUE;
				CopyMemory( &m_pFiles[ nFile ].m_pSHA1, pSHA1->m_pValue, sizeof(SHA1) );
			}

			if ( CBENode* pED2K = pInfo->GetNode( "ed2k" ) )
			{
				if ( ! pED2K->IsType( CBENode::beString ) || pED2K->m_nValue != sizeof(MD4) ) return FALSE;
				m_pFiles[ nFile ].m_bED2K = TRUE;
				CopyMemory( &m_pFiles[ nFile].m_pED2K, pED2K->m_pValue, sizeof(MD4) );
			}

			if ( CBENode* pTiger = pInfo->GetNode( "tiger" ) )
			{
				if ( ! pTiger->IsType( CBENode::beString ) || pTiger->m_nValue != sizeof(TIGEROOT) ) return FALSE;
				m_pFiles[ nFile ].m_bTiger = TRUE;
				CopyMemory( &m_pFiles[ nFile ].m_pTiger, pTiger->m_pValue, sizeof(TIGEROOT) );
			}
			
			m_nTotalSize += m_pFiles[ nFile ].m_nSize;
		}

		if ( m_nFiles == 1 )
		{
			// Single file in a multi-file torrent

			// Reset the name
			m_sName = strPath;

			// Set data/file hashes (if they aren't)
			if ( m_pFiles[0].m_bSHA1 )
			{
				m_bDataSHA1 = m_pFiles[0].m_bSHA1;
				m_pDataSHA1 = m_pFiles[0].m_pSHA1;
			}
			else if ( m_bDataSHA1 )
			{
				m_pFiles[0].m_bSHA1 = m_bDataSHA1;
				m_pFiles[0].m_pSHA1 = m_pDataSHA1;

			}
			if ( m_pFiles[0].m_bED2K )
			{
				m_bDataED2K = m_pFiles[0].m_bED2K;
				m_pDataED2K = m_pFiles[0].m_pED2K;
			}
			else if ( m_bDataED2K )
			{
				m_pFiles[0].m_bED2K = m_bDataED2K;
				m_pFiles[0].m_pED2K = m_pDataED2K;
			}
			if ( m_pFiles[0].m_bTiger )
			{
				m_bDataTiger = m_pFiles[0].m_bTiger;
				m_pDataTiger = m_pFiles[0].m_pTiger;
			}
			else if ( m_bDataTiger )
			{
				m_pFiles[0].m_bTiger = m_bDataTiger;
				m_pFiles[0].m_pTiger = m_pDataTiger;
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
	
	pInfo->GetSHA1( &m_pInfoSHA1 );
	m_bValid = TRUE;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBTInfo load torrent info from tree

BOOL CBTInfo::CheckFiles()
{
	for ( int nFile = 0 ; nFile < m_nFiles ; nFile++ )
	{
		m_pFiles[ nFile ].m_sPath.TrimLeft();
		m_pFiles[ nFile ].m_sPath.TrimRight();
		
		LPCTSTR pszPath = m_pFiles[ nFile ].m_sPath;
		
		if ( pszPath == NULL || *pszPath == 0 ) return FALSE;
		if ( pszPath[1] == ':' ) return FALSE;
		if ( *pszPath == '\\' || *pszPath == '/' ) return FALSE;
		if ( _tcsstr( pszPath, _T("..\\") ) != NULL ) return FALSE;
		if ( _tcsstr( pszPath, _T("../") ) != NULL ) return FALSE;
	}
	
	return ( m_nFiles > 0 );
}

//////////////////////////////////////////////////////////////////////
// CBTInfo block testing

void CBTInfo::BeginBlockTest()
{
	ASSERT( IsAvailable() );
	ASSERT( m_pBlockSHA1 != NULL );
	
	m_pTestSHA1.Reset();
	m_nTestByte = 0;
}

void CBTInfo::AddToTest(LPCVOID pInput, DWORD nLength)
{
	if ( nLength == 0 ) return;
	
	ASSERT( IsAvailable() );
	ASSERT( m_pBlockSHA1 != NULL );
	ASSERT( m_nTestByte + nLength <= m_nBlockSize );
	
	m_pTestSHA1.Add( pInput, nLength );
	m_nTestByte += nLength;
}

BOOL CBTInfo::FinishBlockTest(DWORD nBlock)
{
	ASSERT( IsAvailable() );
	ASSERT( m_pBlockSHA1 != NULL );
	
	if ( nBlock >= m_nBlockCount ) return FALSE;
	
	SHA1 pSHA1;
	m_pTestSHA1.Finish();
	m_pTestSHA1.GetHash( &pSHA1 );
	
	return pSHA1 == m_pBlockSHA1[ nBlock ];
}
