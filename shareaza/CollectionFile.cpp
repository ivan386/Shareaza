//
// ColletionFile.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "CollectionFile.h"

#include "ZIPFile.h"
#include "Buffer.h"
#include "XML.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "EDPacket.h"

#include "Library.h"
#include "Downloads.h"
#include "ShareazaURL.h"
#include "SharedFile.h"

#include "SHA.h"
#include "MD5.h"
#include "ED2K.h"
#include "TigerTree.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CCollectionFile, CComObject)


/////////////////////////////////////////////////////////////////////////////
// CCollectionFile construction

CCollectionFile::CCollectionFile() :
	m_pMetadata ( NULL ),
	m_nType( ShareazaCollection )
{
}

CCollectionFile::~CCollectionFile()
{
	Close();
}

/////////////////////////////////////////////////////////////////////////////
// CCollectionFile open a collection file

BOOL CCollectionFile::Open(LPCTSTR pszFile)
{
	Close();

	CString strType = PathFindExtension( pszFile );
	strType.MakeLower();
	if ( strType == _T(".co") || strType == _T(".collection") )
	{
		m_nType = ShareazaCollection;

		if ( LoadShareaza( pszFile ) )
			return TRUE;
	}
	else if ( strType == _T(".emulecollection") )
	{
		m_nType = eMuleCollection;

		if ( LoadEMule( pszFile ) )
			return TRUE;
	}
	return LoadText( pszFile );
}

/////////////////////////////////////////////////////////////////////////////
// CCollectionFile close a collection file

void CCollectionFile::Close()
{
	for ( POSITION pos = GetFileIterator() ; pos ; )
		delete GetNextFile( pos );
	m_pFiles.RemoveAll();

	delete m_pMetadata;
	m_pMetadata = NULL;

	m_sTitle.Empty();
	m_sThisURI.Empty();
	m_sParentURI.Empty();
}

/////////////////////////////////////////////////////////////////////////////
// CCollectionFile find a file by URN

CCollectionFile::File* CCollectionFile::FindByURN(LPCTSTR pszURN)
{
    Hashes::Sha1Hash oSHA1;
    Hashes::TigerHash oTiger;
    Hashes::Md5Hash oMD5;
    Hashes::Ed2kHash oED2K;
	Hashes::BtHash oBTH;
	
	oSHA1.fromUrn( pszURN );
    oMD5.fromUrn( pszURN );
    oTiger.fromUrn( pszURN );
    oED2K.fromUrn( pszURN );
	oBTH.fromUrn( pszURN );
	
	for ( POSITION pos = GetFileIterator() ; pos ; )
	{
		File* pFile = GetNextFile( pos );
		
		if ( validAndEqual( oSHA1, pFile->m_oSHA1 ) ) return pFile;
		if ( validAndEqual( oMD5, pFile->m_oMD5 ) ) return pFile;
		if ( validAndEqual( oTiger, pFile->m_oTiger ) ) return pFile;
		if ( validAndEqual( oED2K, pFile->m_oED2K ) ) return pFile;
		if ( validAndEqual( oBTH, pFile->m_oBTH ) ) return pFile;
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CCollectionFile find a shared file

CCollectionFile::File* CCollectionFile::FindFile(CLibraryFile* pShared, BOOL bApply)
{
	File* pFile = NULL;

	for ( POSITION pos = GetFileIterator() ; pos ; )
	{
		pFile = GetNextFile( pos );
		if ( validAndEqual( pShared->m_oSHA1, pFile->m_oSHA1 ) ) break;
		if ( validAndEqual( pShared->m_oMD5, pFile->m_oMD5 ) ) break;
		if ( validAndEqual( pShared->m_oTiger, pFile->m_oTiger ) ) break;
		if ( validAndEqual( pShared->m_oED2K, pFile->m_oED2K ) ) break;
		if ( validAndEqual( pShared->m_oBTH, pFile->m_oBTH ) ) break;
		pFile = NULL;
	}

	if ( bApply && pFile != NULL ) pFile->ApplyMetadata( pShared );

	return pFile;
}

/////////////////////////////////////////////////////////////////////////////
// CCollectionFile get count

int CCollectionFile::GetMissingCount()
{
	int nCount =0;

	for ( POSITION pos = GetFileIterator() ; pos ; )
	{
		File* pFile = GetNextFile( pos );
		if ( ! pFile->IsComplete() && ! pFile->IsDownloading() ) nCount++;
	}

	return nCount;
}

/////////////////////////////////////////////////////////////////////////////
// Load Shareaza collection

BOOL CCollectionFile::LoadShareaza(LPCTSTR pszFile)
{
	CZIPFile pZIP;
	if ( ! pZIP.Open( pszFile ) ) return FALSE;

	CZIPFile::File* pFile = pZIP.GetFile( _T("Collection.xml"), TRUE );
	if ( pFile == NULL ) return FALSE;

	auto_ptr< CBuffer > pBuffer ( pFile->Decompress() );
	if ( ! pBuffer.get() ) return FALSE;

	auto_ptr< CXMLElement > pXML ( CXMLElement::FromString( pBuffer->ReadString( pBuffer->m_nLength, CP_UTF8 ), TRUE ) );
	if ( ! pXML.get() ) return FALSE;
	if ( ! pXML->IsNamed( _T("collection") ) ) return FALSE;

	CXMLElement* pProperties = pXML->GetElementByName( _T("properties") );
	if ( pProperties == NULL ) return FALSE;
	CXMLElement* pContents = pXML->GetElementByName( _T("contents") );
	if ( pContents == NULL ) return FALSE;

	for ( POSITION pos = pContents->GetElementIterator() ; pos ; )
	{
		auto_ptr< File > pFile( new File( this ) );
		if ( pFile.get() && pFile->Parse( pContents->GetNextElement( pos ) ) )
		{
			m_pFiles.AddTail( pFile.release() );
		}
		else
		{
			Close();
			return FALSE;
		}
	}

	if ( CXMLElement* pMetadata = pProperties->GetElementByName( _T("metadata") ) )
	{
		m_pMetadata = CloneMetadata( pMetadata );
		if ( m_pMetadata )
			m_sThisURI = m_pMetadata->GetAttributeValue( CXMLAttribute::schemaName );
	}

	if ( CXMLElement* pTitle = pProperties->GetElementByName( _T("title") ) )
	{
		m_sTitle = pTitle->GetValue();
	}

	if ( CXMLElement* pMounting = pProperties->GetElementByName( _T("mounting") ) )
	{
		if ( CXMLElement* pParent = pMounting->GetElementByName( _T("parent") ) )
		{
			m_sParentURI = pParent->GetAttributeValue( _T("uri") );
		}
		if ( CXMLElement* pThis = pMounting->GetElementByName( _T("this") ) )
		{
			m_sThisURI = pThis->GetAttributeValue( _T("uri") );
		}
	}

	if ( m_sThisURI.IsEmpty() )
	{
		Close();
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Load eMule collection

BOOL CCollectionFile::LoadEMule(LPCTSTR pszFile)
{
	// TODO: Add schema detection
	m_sThisURI = CSchema::uriFolder;
	m_sParentURI = CSchema::uriCollectionsFolder;

	// Open collection
	DWORD nFileCount = 0;
	CFile pFile;
	if ( pFile.Open( pszFile, CFile::modeRead ) )
	{
		// Check collection version
		DWORD read;
		DWORD nVersion;
		if ( pFile.Read( &nVersion, sizeof( nVersion ) ) == sizeof( nVersion ) &&
			( nVersion == ED2K_FILE_VERSION1_INITIAL ||
			  nVersion == ED2K_FILE_VERSION2_LARGEFILES ) )
		{
			// Load collection properties
			DWORD nCount;
			if ( pFile.Read( &nCount, sizeof( nCount ) ) == sizeof( nCount ) &&
				nCount > 0 && nCount < 10 )
			{
				for ( DWORD i = 0; i < nCount; ++i )
				{
					CEDTag pTag;
					if ( ! pTag.Read( &pFile ) )
						break;

					if ( pTag.Check( ED2K_FT_FILENAME, ED2K_TAG_STRING ) )
					{						
						m_sTitle = pTag.m_sValue;
					}
					else if ( pTag.Check( ED2K_FT_COLLECTIONAUTHOR, ED2K_TAG_STRING ) )
					{
						// TODO: ED2K_FT_COLLECTIONAUTHOR
					}
					else if ( pTag.Check( ED2K_FT_COLLECTIONAUTHORKEY, ED2K_TAG_BLOB ) )
					{
						// TODO: ED2K_FT_COLLECTIONAUTHORKEY
					}
				}
			}

			// Load collection files
			if ( pFile.Read( &nFileCount, sizeof( nFileCount ) ) == sizeof( nFileCount ) &&
				nFileCount > 0 && nFileCount < 20000 )
			{
				for ( DWORD i = 0; i < nFileCount; ++i )
				{
					auto_ptr< File > pCollectionFile( new File( this ) );
					if ( pCollectionFile.get() && pCollectionFile->Parse( pFile ) )
					{
						m_pFiles.AddTail( pCollectionFile.release() );
					}
					else
						break;
				}
			}
		}
	}
	return nFileCount && ( m_pFiles.GetCount() == nFileCount );
}

/////////////////////////////////////////////////////////////////////////////
// Load simple text collection

BOOL CCollectionFile::LoadText(LPCTSTR pszFile)
{
	// TODO: Add schema detection
	m_sThisURI = CSchema::uriFolder;
	m_sParentURI = CSchema::uriCollectionsFolder;

	// Make collection title from file name
	m_sTitle = PathFindFileName( pszFile );
	int nPos = m_sTitle.ReverseFind( _T('.') );
	if ( nPos != -1 )
		m_sTitle = m_sTitle.Left( nPos );

	CStdioFile pFile;
	if ( pFile.Open( pszFile, CFile::modeRead ) )
	{
		for (;;)
		{
			CString strText;
			if ( ! pFile.ReadString( strText ) )
				// End of file
				break;

			auto_ptr< File > pCollectionFile( new File( this ) );
			if ( ! pCollectionFile.get() )
				// Out of memory
				break;

			if ( pCollectionFile->Parse( strText ) )
				m_pFiles.AddTail( pCollectionFile.release() );
		}
	}
	return ( m_pFiles.GetCount() != 0 );
}

/////////////////////////////////////////////////////////////////////////////
// CCollectionFile clone metadata

CXMLElement* CCollectionFile::CloneMetadata(CXMLElement* pMetadata)
{
	CString strURI = pMetadata->GetAttributeValue( _T("xmlns:s") );
	if ( strURI.IsEmpty() ) return NULL;

	CXMLElement* pCore = pMetadata->GetFirstElement();
	if ( pCore == NULL ) return NULL;

	if ( CSchema* pSchema = SchemaCache.Get( strURI ) )
	{
		pMetadata = pSchema->Instantiate();
	}
	else
	{
		pMetadata = new CXMLElement( NULL, pCore->GetName() + 's' );
		pMetadata->AddAttribute( CXMLAttribute::schemaName, strURI );
	}

	pCore = pCore->Clone();
	pMetadata->AddElement( pCore );

	CString strName = pMetadata->GetName();
	if ( _tcsnicmp( strName, _T("s:"), 2 ) == 0 ) pMetadata->SetName( strName.Mid( 2 ) );

	strName = pCore->GetName();
	if ( _tcsnicmp( strName, _T("s:"), 2 ) == 0 ) pCore->SetName( strName.Mid( 2 ) );

	for ( POSITION pos = pCore->GetElementIterator() ; pos ; )
	{
		CXMLNode* pNode = pCore->GetNextElement( pos );
		CString strName = pNode->GetName();
		if ( _tcsnicmp( strName, _T("s:"), 2 ) == 0 ) pNode->SetName( strName.Mid( 2 ) );
	}

	for ( POSITION pos = pCore->GetAttributeIterator() ; pos ; )
	{
		CXMLNode* pNode = pCore->GetNextAttribute( pos );
		CString strName = pNode->GetName();
		if ( _tcsnicmp( strName, _T("s:"), 2 ) == 0 ) pNode->SetName( strName.Mid( 2 ) );
	}

	return pMetadata;
}


/////////////////////////////////////////////////////////////////////////////
// CCollectionFile::File construction

CCollectionFile::File::File(CCollectionFile* pParent) :
	m_pParent	( pParent ),
	m_pMetadata	( NULL )
{
}

CCollectionFile::File::~File()
{
	delete m_pMetadata;
}

/////////////////////////////////////////////////////////////////////////////
// CCollectionFile::File parse

BOOL CCollectionFile::File::Parse(CXMLElement* pRoot)
{
	if ( ! pRoot->IsNamed( _T("file") ) ) return FALSE;

	for ( POSITION pos = pRoot->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pRoot->GetNextElement( pos );

		if ( pXML->IsNamed( _T("id") ) )
		{
			if ( !m_oSHA1 ) m_oSHA1.fromUrn( pXML->GetValue() );
            if ( !m_oMD5 ) m_oMD5.fromUrn( pXML->GetValue() );
			if ( !m_oTiger ) m_oTiger.fromUrn( pXML->GetValue() );
			if ( !m_oED2K ) m_oED2K.fromUrn( pXML->GetValue() );
			if ( !m_oBTH ) m_oBTH.fromUrn( pXML->GetValue() );
		}
		else if ( pXML->IsNamed( _T("description") ) )
		{
			if ( CXMLElement* pName = pXML->GetElementByName( _T("name") ) )
			{
				m_sName = pName->GetValue();
			}
			if ( CXMLElement* pSize = pXML->GetElementByName( _T("size") ) )
			{
				if ( _stscanf( pSize->GetValue(), _T("%I64i"), &m_nSize ) != 1 )
					return FALSE;
			}
		}
		else if ( pXML->IsNamed( _T("metadata") ) )
		{
			if ( m_pMetadata != NULL ) delete m_pMetadata;
			m_pMetadata = CCollectionFile::CloneMetadata( pXML );
		}
		else if ( pXML->IsNamed( _T("packaged") ) )
		{
			if ( CXMLElement* pSource = pXML->GetElementByName( _T("source") ) )
			{
				/* m_sSource =*/ pSource->GetValue();
			}
		}
	}
	
	return IsHashed();
}

BOOL CCollectionFile::File::Parse(CFile& pFile)
{
	DWORD nCount;
	if ( pFile.Read( &nCount, sizeof( nCount ) ) == sizeof( nCount ) &&
		nCount > 0 && nCount < 10 )
	{
		for ( DWORD i = 0; i < nCount; ++i )
		{
			CEDTag pTag;
			if ( ! pTag.Read( &pFile ) )
				break;

			if ( pTag.Check( ED2K_FT_FILEHASH, ED2K_TAG_HASH ) )
			{
				m_oED2K = pTag.m_oValue;
			}
			else if ( pTag.Check( ED2K_FT_FILESIZE, ED2K_TAG_INT ) )
			{
				m_nSize = pTag.m_nValue;
			}
			else if ( pTag.Check( ED2K_FT_FILENAME, ED2K_TAG_STRING ) )
			{						
				m_sName = pTag.m_sValue;
			}
			else if ( pTag.Check( ED2K_FT_FILETYPE, ED2K_TAG_STRING ) )
			{
				// TODO: ED2K_FT_FILETYPE
			}
			else if ( pTag.Check( ED2K_FT_FILECOMMENT, ED2K_TAG_STRING ) )
			{
				// TODO: ED2K_FT_FILECOMMENT
			}
			else if ( pTag.Check( ED2K_FT_FILERATING, ED2K_TAG_INT ) )
			{
				// TODO: ED2K_FT_FILERATING
			}
		}
	}
	return ! m_sName.IsEmpty() && m_oED2K && m_nSize != SIZE_UNKNOWN;
}

BOOL CCollectionFile::File::Parse(LPCTSTR szText)
{
	CShareazaURL pURL;
	if ( pURL.Parse( szText ) &&
		pURL.m_nAction == CShareazaURL::uriDownload &&
		pURL.m_sName.GetLength() &&
		pURL.m_nSize != SIZE_UNKNOWN && pURL.m_nSize != 0 &&
		pURL.IsHashed() )
	{
		m_sName = pURL.m_sName;
		if ( pURL.m_oSHA1 )
			m_oSHA1 = pURL.m_oSHA1;
		if ( pURL.m_oTiger )
			m_oTiger = pURL.m_oTiger;
		if ( pURL.m_oED2K )
			m_oED2K = pURL.m_oED2K;
		if ( pURL.m_oMD5 )
			m_oMD5 = pURL.m_oMD5;
		if ( pURL.m_oBTH )
			m_oBTH = pURL.m_oBTH;
		m_nSize = pURL.m_nSize;
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CCollectionFile::File state

BOOL CCollectionFile::File::IsComplete() const
{
	return LibraryMaps.LookupFileBySHA1( m_oSHA1, FALSE, TRUE )
		|| LibraryMaps.LookupFileByTiger( m_oTiger, FALSE, TRUE )
		|| LibraryMaps.LookupFileByED2K( m_oED2K, FALSE, TRUE )
		|| LibraryMaps.LookupFileByBTH( m_oBTH, FALSE, TRUE )
		|| LibraryMaps.LookupFileByMD5( m_oMD5, FALSE, TRUE );
}

BOOL CCollectionFile::File::IsDownloading() const
{
	return Downloads.FindBySHA1( m_oSHA1 )
		|| Downloads.FindByTiger( m_oTiger )
		|| Downloads.FindByED2K( m_oED2K )
		|| Downloads.FindByMD5( m_oMD5 )
		|| Downloads.FindByBTH( m_oBTH );
}

/////////////////////////////////////////////////////////////////////////////
// CCollectionFile::File download

BOOL CCollectionFile::File::Download()
{
	CShareazaURL pURL;

	if ( IsComplete() || IsDownloading() ) return FALSE;

	pURL.m_nAction	= CShareazaURL::uriDownload;
	pURL.m_oSHA1 = m_oSHA1;
    pURL.m_oMD5 = m_oMD5;
	pURL.m_oTiger = m_oTiger;
	pURL.m_oED2K = m_oED2K;
	pURL.m_oBTH = m_oBTH;
	pURL.m_sName	= m_sName;
	pURL.m_bSize	= ( m_nSize != SIZE_UNKNOWN );
	pURL.m_nSize	= m_nSize;

	return Downloads.Add( &pURL ) != NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CCollectionFile::File apply metadata to a shared file

BOOL CCollectionFile::File::ApplyMetadata(CLibraryFile* pShared)
{
	ASSERT( pShared != NULL );
	if ( m_pMetadata == NULL ) return FALSE;

	CXMLElement* pXML = m_pMetadata->Clone();
	BOOL bResult = pShared->SetMetadata( pXML );
	delete pXML;

	return bResult;
}
