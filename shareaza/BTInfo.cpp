//
// BTInfo.cpp
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
	m_bDataSHA1		= FALSE;
	m_nTotalSize	= 0;
	m_nBlockSize	= 0;
	m_nBlockCount	= 0;
	m_pBlockSHA1	= NULL;
	m_nFiles		= 0;
	m_pFiles		= NULL;
}

CBTInfo::~CBTInfo()
{
	Clear();
}

CBTInfo::CBTFile::CBTFile()
{
	m_nSize = 0;
	m_bSHA1 = FALSE;
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
	m_pInfoSHA1		= pSource->m_pInfoSHA1;
	m_bDataSHA1		= pSource->m_bDataSHA1;
	m_pDataSHA1		= pSource->m_pDataSHA1;
	m_nTotalSize	= pSource->m_nTotalSize;
	m_nBlockSize	= pSource->m_nBlockSize;
	m_nBlockCount	= pSource->m_nBlockCount;
	m_sName			= pSource->m_sName;
	m_sTracker		= pSource->m_sTracker;
	m_nFiles		= pSource->m_nFiles;
	
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
	int nVersion = 2;
	
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
	m_sPath = pSource->m_sPath;
	m_nSize = pSource->m_nSize;
	m_bSHA1 = pSource->m_bSHA1;
	m_pSHA1 = pSource->m_pSHA1;
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
	
	CBENode* pAnnounce = pRoot->GetNode( "announce" );
	
	if ( pAnnounce->IsType( CBENode::beString ) )
	{
		m_sTracker = pAnnounce->GetString();
		if ( m_sTracker.Find( _T("http") ) != 0 ) m_sTracker.Empty();
	}
	
	CBENode* pInfo = pRoot->GetNode( "info" );
	if ( ! pInfo->IsType( CBENode::beDict ) ) return FALSE;
	
	CBENode* pName = pInfo->GetNode( "name" );
	if ( pName->IsType( CBENode::beString ) ) m_sName = pName->GetString();
    if ( m_sName.IsEmpty() ) m_sName.Format( _T("Unnamed_Torrent_%i"), (int)rand() );
	
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
	
	if ( CBENode* pSHA1 = pInfo->GetNode( "sha1" ) )
	{
		if ( ! pSHA1->IsType( CBENode::beString ) || pSHA1->m_nValue != sizeof(SHA1) ) return FALSE;
		m_bDataSHA1 = TRUE;
		CopyMemory( &m_pDataSHA1, pSHA1->m_pValue, sizeof(SHA1) );
	}
	
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
			
			// Hack to prefix all
			m_pFiles[ nFile ].m_sPath = m_sName;
			
			for ( int nPath = 0 ; nPath < pPath->GetCount() ; nPath++ )
			{
				CBENode* pPart = pPath->GetNode( nPath );
				if ( ! pPart->IsType( CBENode::beString ) ) return FALSE;
				
				if ( m_pFiles[ nFile ].m_sPath.GetLength() )
					m_pFiles[ nFile ].m_sPath += '\\';
				
				m_pFiles[ nFile ].m_sPath += CDownloadTask::SafeFilename( pPart->GetString() );
			}
			
			if ( CBENode* pSHA1 = pFile->GetNode( "sha1" ) )
			{
				if ( ! pSHA1->IsType( CBENode::beString ) || pSHA1->m_nValue != sizeof(SHA1) ) return FALSE;
				m_pFiles[ nFile ].m_bSHA1 = TRUE;
				CopyMemory( &m_pFiles[ nFile ].m_pSHA1, pSHA1->m_pValue, sizeof(SHA1) );
			}
			
			m_nTotalSize += m_pFiles[ nFile ].m_nSize;
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
