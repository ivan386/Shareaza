//
// HashDatabase.cpp
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
#include "HashDatabase.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CHashDatabase LibraryHashDB;


//////////////////////////////////////////////////////////////////////
// CHashDatabase construction

CHashDatabase::CHashDatabase()
	: m_bOpen	( FALSE )
	, m_nOffset	( 0 )
	, m_pIndex	( NULL )
	, m_nIndex	( 0 )
	, m_nBuffer	( 0 )
{
}

CHashDatabase::~CHashDatabase()
{
	Close();
}

//////////////////////////////////////////////////////////////////////
// CHashDatabase create

BOOL CHashDatabase::Create()
{
	CSingleLock pLock( &m_pSection, TRUE );

	Close();

	m_sPath = Settings.General.UserPath + _T("\\Data\\TigerTree.dat");

	if ( m_pFile.Open( m_sPath, CFile::modeReadWrite ) )
	{
		try
		{
			CHAR szID[8];
			m_pFile.Read( szID, 8 );
			if ( memcmp( szID, "HFDB1000", 8 ) == 0 || memcmp( szID, "HFDB1001", 8 ) == 0 )
			{
				m_pFile.Read( &m_nOffset, 4 );
				m_pFile.Read( &m_nIndex, 4 );
				m_pFile.Seek( m_nOffset, 0 );

				for ( m_nBuffer = m_nIndex ; m_nBuffer & 63 ; m_nBuffer++ );
				m_pIndex = new HASHDB_INDEX[ m_nBuffer ];

				if ( memcmp( szID, "HFDB1001", 8 ) == 0 )
				{
					m_pFile.Read( m_pIndex, sizeof(HASHDB_INDEX) * m_nIndex );
				}
				else
				{
					HASHDB_INDEX_1000 pIndex1;

					for ( DWORD nIndex = 0 ; nIndex < m_nIndex ; nIndex++ )
					{
						m_pFile.Read( &pIndex1, sizeof(pIndex1) );
						m_pIndex[ nIndex ].nIndex	= pIndex1.nIndex;
						m_pIndex[ nIndex ].nType	= HASH_TIGERTREE;
						m_pIndex[ nIndex ].nOffset	= pIndex1.nOffset;
						m_pIndex[ nIndex ].nLength	= pIndex1.nLength;
					}
				}
				m_bOpen = TRUE;
				return TRUE;
			}

			// Wrong Magic
			m_pFile.Close();
		}
		catch ( CException* pException )
		{
			m_pFile.Abort();
			pException->Delete();
			theApp.Message( MSG_ERROR, _T("Hash Database load error: %s"), (LPCTSTR)m_sPath );
		}
	}

	Close();

	if ( m_pFile.Open( m_sPath, CFile::modeReadWrite | CFile::modeCreate ) )
	{
		try
		{
			m_nOffset = 16;
			m_pFile.Write( "HFDB1001", 8 );
			m_pFile.Write( &m_nOffset, 4 );
			m_pFile.Write( &m_nIndex, 4 );
			m_pFile.Flush();

			m_bOpen = TRUE;

			return TRUE;
		}
		catch ( CException* pException )
		{
			m_pFile.Abort();
			pException->Delete();
		}
	}

	theApp.Message( MSG_ERROR, _T("Hash Database create error: %s"), (LPCTSTR)m_sPath );
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CHashDatabase close

void CHashDatabase::Close()
{
	CSingleLock pLock( &m_pSection, TRUE );

	delete [] m_pIndex;

	if ( m_pFile.m_hFile != CFile::hFileNull )
	{
		try
		{
			m_pFile.Close();
		}
		catch ( CException* pException )
		{
			m_pFile.Abort();
			pException->Delete();
		}
	}

	m_bOpen		= FALSE;
	m_nOffset	= 0;
	m_pIndex	= NULL;
	m_nIndex	= 0;
	m_nBuffer	= 0;
}

//////////////////////////////////////////////////////////////////////
// CHashDatabase lookup

HASHDB_INDEX* CHashDatabase::Lookup(DWORD nIndex, DWORD nType) const
{
	ASSERT( m_bOpen );
	HASHDB_INDEX* pIndex = m_pIndex;

	for ( DWORD nCount = m_nIndex ; nCount ; nCount--, pIndex++ )
	{
		if ( pIndex->nIndex == nIndex && pIndex->nType == nType ) return pIndex;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CHashDatabase prepare to store

HASHDB_INDEX* CHashDatabase::PrepareToStore(DWORD nIndex, DWORD nType, DWORD nLength)
{
	ASSERT( m_bOpen );
	HASHDB_INDEX* pIndex = Lookup( nIndex, nType );

	if ( pIndex != NULL && pIndex->nLength != nLength )
	{
		pIndex->nIndex = 0;
		pIndex = NULL;
	}

	if ( pIndex != NULL ) return pIndex;

	HASHDB_INDEX* pBestIndex	= NULL;
	DWORD nBestOverhead			= 0xFFFFFFFF;
	DWORD nCount;

	for ( pIndex = m_pIndex, nCount = m_nIndex ; nCount ; nCount--, pIndex++ )
	{
		if ( pIndex->nIndex == 0 && pIndex->nLength >= nLength )
		{
			DWORD nOverhead = pIndex->nLength - nLength;

			if ( nOverhead < nBestOverhead )
			{
				pBestIndex = pIndex;
				nBestOverhead = nOverhead;
				if ( nOverhead == 0 ) break;
			}
		}
	}

	if ( pBestIndex != NULL )
	{
		pIndex = pBestIndex;
	}
	else
	{
		if ( m_nIndex >= m_nBuffer )
		{
			m_nBuffer += 64;
			HASHDB_INDEX* pNew = new HASHDB_INDEX[ m_nBuffer ];
			if ( m_pIndex )
			{
				if ( m_nIndex ) CopyMemory( pNew, m_pIndex, sizeof(HASHDB_INDEX) * m_nIndex );
				delete [] m_pIndex;
			}
			m_pIndex = pNew;
		}

		pIndex = m_pIndex + m_nIndex++;
		pIndex->nOffset = m_nOffset;
		pIndex->nLength = nLength;

		m_nOffset += nLength;
	}

	pIndex->nIndex	= nIndex;
	pIndex->nType	= nType;

	return pIndex;
}

//////////////////////////////////////////////////////////////////////
// CHashDatabase erase

BOOL CHashDatabase::Erase(DWORD nIndex, DWORD nType)
{
	ASSERT( m_bOpen );

	HASHDB_INDEX* pIndex = Lookup( nIndex, nType );
	if ( pIndex == NULL ) return FALSE;
	pIndex->nIndex = 0;
	Commit();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHashDatabase commit the changes and flush

BOOL CHashDatabase::Commit()
{
	ASSERT( m_bOpen );

	try
	{
		m_pFile.SetLength( m_nOffset + sizeof(HASHDB_INDEX) * m_nIndex );
		m_pFile.Seek( 0, 0 );
		m_pFile.Write( "HFDB1001", 8 );
		m_pFile.Write( &m_nOffset, 4 );
		m_pFile.Write( &m_nIndex, 4 );
		m_pFile.Seek( m_nOffset, 0 );
		m_pFile.Write( m_pIndex, sizeof(HASHDB_INDEX) * m_nIndex );
		m_pFile.Flush();
	}
	catch ( CException* pException )
	{
		m_pFile.Abort();
		pException->Delete();
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHashDatabase delete all (irrespective of type)

BOOL CHashDatabase::DeleteAll(DWORD nIndex)
{
	CSingleLock pLock( &m_pSection, TRUE );
	if ( m_bOpen == FALSE ) return FALSE;
	if ( nIndex == 0 ) return FALSE;

	HASHDB_INDEX* pIndex = m_pIndex;
	DWORD nChanged = 0;

	for ( DWORD nCount = m_nIndex ; nCount ; nCount--, pIndex++ )
	{
		if ( pIndex->nIndex == nIndex )
		{
			pIndex->nIndex = 0;
			nChanged++;
		}
	}

	if ( nChanged != 0 ) Commit();
	return nChanged != 0;
}

//////////////////////////////////////////////////////////////////////
// CHashDatabase tiger-tree access

BOOL CHashDatabase::GetTiger(DWORD nIndex, CTigerTree* pTree)
{
	pTree->Clear();

	CSingleLock pLock( &m_pSection, TRUE );
	if ( m_bOpen == FALSE ) return FALSE;

	HASHDB_INDEX* pIndex = Lookup( nIndex, HASH_TIGERTREE );
	if ( pIndex == NULL ) return FALSE;

	try
	{
		m_pFile.Seek( pIndex->nOffset, 0 );

		CArchive ar( &m_pFile, CArchive::load, 32768 );	// 32 KB buffer
		try
		{
			Serialize( ar, pTree );
			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			m_pFile.Abort();
			pException->Delete();
			return FALSE;
		}
	}
	catch ( CException* pException )
	{
		m_pFile.Abort();
		pException->Delete();
		return FALSE;
	}

	return TRUE;
}

void CHashDatabase::Serialize(CArchive& ar, CTigerTree* pTree)
{
	if ( ar.IsLoading() )
	{
		uint32 nHeight = 0;
		ar >> nHeight;
		pTree->SetHeight( nHeight );
		if ( uint32 nSize = pTree->GetSerialSize() )
		{
			auto_array< uchar > pBuf( new uchar[ nSize ] );
			ReadArchive( ar, pBuf.get(), nSize );
			pTree->Load( pBuf.get() );
		}
	}
	else
	{
		ar << pTree->GetHeight();
		if ( uint32 nSize = pTree->GetSerialSize() )
		{
			auto_array< uchar > pBuf( new uchar[ nSize ] );
			pTree->Save( pBuf.get() );
			ar.Write( pBuf.get(), nSize );
		}
	}
}

BOOL CHashDatabase::StoreTiger(DWORD nIndex, CTigerTree* pTree)
{
	CSingleLock pLock( &m_pSection, TRUE );
	if ( m_bOpen == FALSE ) return FALSE;

	HASHDB_INDEX* pIndex = PrepareToStore( nIndex, HASH_TIGERTREE,
		pTree->GetSerialSize() + sizeof( uint32 ) );
	if ( pIndex == NULL ) return FALSE;

	try
	{
		m_pFile.Seek( pIndex->nOffset, 0 );

		CArchive ar( &m_pFile, CArchive::store, 32768 );	// 32 KB buffer
		try
		{
			Serialize( ar, pTree );
			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			m_pFile.Abort();
			pException->Delete();
			return FALSE;
		}

		m_pFile.Flush();
	}
	catch ( CException* pException )
	{
		m_pFile.Abort();
		pException->Delete();
		return FALSE;
	}

	Commit();

	return TRUE;
}

BOOL CHashDatabase::DeleteTiger(DWORD nIndex)
{
	CSingleLock pLock( &m_pSection, TRUE );
	if ( m_bOpen == FALSE ) return FALSE;
	return Erase( nIndex, HASH_TIGERTREE );
}

//////////////////////////////////////////////////////////////////////
// CHashDatabase ED2K hashset access

BOOL CHashDatabase::GetED2K(DWORD nIndex, CED2K* pSet)
{
	pSet->Clear();

	CSingleLock pLock( &m_pSection, TRUE );
	if ( m_bOpen == FALSE ) return FALSE;

	HASHDB_INDEX* pIndex = Lookup( nIndex, HASH_ED2K );
	if ( pIndex == NULL ) return FALSE;

	try
	{
		m_pFile.Seek( pIndex->nOffset, 0 );

		CArchive ar( &m_pFile, CArchive::load, 32768 );	// 32 KB buffer
		try
		{
			Serialize( ar, pSet );
			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			m_pFile.Abort();
			pException->Delete();
			return FALSE;
		}
	}
	catch ( CException* pException )
	{
		m_pFile.Abort();
		pException->Delete();
		return FALSE;
	}

	return TRUE;
}

void CHashDatabase::Serialize(CArchive& ar, CED2K* pSet)
{
	if ( ar.IsLoading() )
	{
		uint32 nListSize = 0;
		ar >> nListSize;
		pSet->SetSize( nListSize );
		if ( uint32 nSize = pSet->GetSerialSize() )
		{
			auto_array< uchar >pBuf( new uchar[ nSize ] );
			ReadArchive( ar, pBuf.get(), nSize );
			pSet->Load( pBuf.get() );
		}
	}
	else
	{
		ar << pSet->GetSize();
		if ( uint32 nSize = pSet->GetSerialSize() )
		{
			auto_array< uchar >pBuf( new uchar[ nSize ] );
			pSet->Save( pBuf.get() );
			ar.Write( pBuf.get(), nSize );
		}
	}
}

BOOL CHashDatabase::StoreED2K(DWORD nIndex, CED2K* pSet)
{
	CSingleLock pLock( &m_pSection, TRUE );
	if ( m_bOpen == FALSE ) return FALSE;

	HASHDB_INDEX* pIndex = PrepareToStore( nIndex, HASH_ED2K,
		pSet->GetSerialSize() + sizeof( uint32 ) );
	if ( pIndex == NULL ) return FALSE;

	try
	{
		m_pFile.Seek( pIndex->nOffset, 0 );

		CArchive ar( &m_pFile, CArchive::store, 32768 );	// 32 KB buffer
		try
		{
			Serialize( ar, pSet );
			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			m_pFile.Abort();
			pException->Delete();
			return FALSE;
		}

		m_pFile.Flush();
	}
	catch ( CException* pException )
	{
		m_pFile.Abort();
		pException->Delete();
		return FALSE;
	}

	Commit();

	return TRUE;
}

BOOL CHashDatabase::DeleteED2K(DWORD nIndex)
{
	CSingleLock pLock( &m_pSection, TRUE );
	if ( m_bOpen == FALSE ) return FALSE;
	return Erase( nIndex, HASH_ED2K );
}
