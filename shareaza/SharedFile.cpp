//
// SharedFile.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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
#include "SharedFile.h"
#include "SharedFolder.h"
#include "AlbumFolder.h"
#include "Library.h"
#include "LibraryBuilder.h"
#include "LibraryDictionary.h"
#include "LibraryFolders.h"
#include "LibraryHistory.h"
#include "HashDatabase.h"

#include "Network.h"
#include "Download.h"
#include "Downloads.h"
#include "ShareazaURL.h"
#include "FileExecutor.h"
#include "Buffer.h"
#include "ThumbCache.h"

#include "XML.h"
#include "XMLCOM.h"
#include "Schema.h"
#include "SchemaCache.h"

#include "Application.h"
#include "VersionChecker.h"
#include "DlgFolderScan.h"
#include "Transfers.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CLibraryFile, CShareazaFile)

BEGIN_INTERFACE_MAP(CLibraryFile, CShareazaFile)
	INTERFACE_PART(CLibraryFile, IID_ILibraryFile, LibraryFile)
END_INTERFACE_MAP()


//////////////////////////////////////////////////////////////////////
// CLibraryFile construction

CLibraryFile::CLibraryFile(CLibraryFolder* pFolder, LPCTSTR pszName) :
	m_pNextSHA1			( NULL )
,	m_pNextTiger		( NULL )
,	m_pNextED2K			( NULL )
,	m_pNextBTH			( NULL )
,	m_pNextMD5			( NULL )
,	m_nScanCookie		( 0ul )
,	m_nUpdateCookie		( 0ul )
,	m_nSelectCookie		( 0ul )
,	m_nListCookie		( 0ul )
,	m_pFolder			( pFolder )
,	m_nIndex			( 0ul )
,	m_bShared			( TRI_UNKNOWN )
,	m_nVirtualBase		( 0ull )
,	m_nVirtualSize		( 0ull )
,	m_bVerify			( TRI_UNKNOWN )
,	m_pSchema			( NULL )
,	m_pMetadata			( NULL )
,	m_bMetadataAuto		( FALSE )
,	m_bMetadataModified	( FALSE )
,	m_nRating			( 0 )
,	m_nUploadsToday		( 0ul )
,	m_nUploadsTotal		( 0ul )
,	m_bCachedPreview	( FALSE )
,	m_bBogus			( FALSE )
,	m_nHitsToday		( 0ul )
,	m_nHitsTotal		( 0ul )
,	m_nSearchCookie		( 0ul )
,	m_nSearchWords		( 0ul )
,	m_pNextHit			( NULL )
,	m_nCollIndex		( 0ul )
,	m_nIcon16			( -1 )
,	m_bNewFile			( FALSE )
{
	ZeroMemory( &m_pTime, sizeof(m_pTime) );
	ZeroMemory( &m_pMetadataTime, sizeof(m_pMetadataTime) );
	if ( pszName )
		m_sName = pszName;
	if ( pFolder )
		m_sPath = pFolder->m_sPath;

	EnableDispatch( IID_ILibraryFile );
}

CLibraryFile::~CLibraryFile()
{
	ASSERT_VALID( this );

	Library.RemoveFile( this );

	delete m_pMetadata;

	for ( POSITION pos = m_pSources.GetHeadPosition() ; pos ; )
	{
		delete m_pSources.GetNext( pos );
	}

	ASSERT( Library.LookupFile( m_nIndex ) == NULL );
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile path computation

CString CLibraryFile::GetFolder() const
{
	if ( IsAvailable() )
		return m_pFolder->m_sPath;
	else
		return CString();
}

CString CLibraryFile::GetPath() const
{
	if ( IsAvailable() )
		return m_pFolder->m_sPath + _T('\\') + m_sName;
	else
		return m_sName;
}

CString CLibraryFile::GetSearchName() const
{
	int nBase = 0;
	CString str;

	if ( IsAvailable() && m_pFolder->m_pParent != NULL )
	{
		for ( CLibraryFolder* pFolder = m_pFolder ; ; pFolder = pFolder->m_pParent )
		{
			if ( pFolder->m_pParent == NULL )
			{
				nBase = pFolder->m_sPath.GetLength();
				break;
			}
		}
	}

	if ( nBase <= 0 )
	{
		str = m_sName;
	}
	else
	{
		ASSERT( m_pFolder->m_sPath.GetLength() > nBase );
		str = m_pFolder->m_sPath.Mid( nBase + 1 ) + '\\' + m_sName;
	}

	ToLower( str );
	return str;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile shared check

bool CLibraryFile::IsShared(bool bIgnoreOverride) const
{
	// Don't share offline files
	if ( IsAvailable() && m_pFolder->IsOffline() )
		return false;

	// Don't share private torrents
	if ( m_pSchema != NULL &&
		m_pSchema->CheckURI( CSchema::uriBitTorrent ) &&
		m_pMetadata != NULL &&
		m_pMetadata->GetAttributeValue( L"privateflag", L"true" ).CompareNoCase( L"true" ) == 0 )
		return false;

	// Use override shared flag of file
	if ( m_bShared != TRI_UNKNOWN && ! bIgnoreOverride )
		return ( m_bShared == TRI_TRUE );

	// Ghost files by default shared
	if ( ! IsAvailable() )
		return true;

	// Use folder shared flag
	return ( m_pFolder->IsShared() != FALSE );
}

void CLibraryFile::SetShared(bool bShared, bool bOverride)
{
	TRISTATE bNewShare = bOverride ? ( bShared ? TRI_TRUE : TRI_FALSE ) : TRI_UNKNOWN;

	// Don't share not verified files
	if ( m_bVerify == TRI_FALSE )
		bNewShare = TRI_FALSE;

	// Don't share private torrents
	if ( m_pSchema != NULL &&
		m_pSchema->CheckURI( CSchema::uriBitTorrent ) &&
		m_pMetadata != NULL &&
		m_pMetadata->GetAttributeValue( L"privateflag", L"true" ).CompareNoCase( L"true" ) == 0 )
		bNewShare = TRI_FALSE;

	bool bFolderShared = ! IsAvailable() || m_pFolder->IsShared();

	if ( bNewShare == TRI_UNKNOWN )
		// Use folder default shared flag
		bNewShare = bShared ?
			( bFolderShared ? TRI_UNKNOWN : TRI_TRUE ) :
			( bFolderShared ? TRI_FALSE : TRI_UNKNOWN );

	if ( m_bShared != bNewShare )
	{
		m_bShared = bNewShare;
		m_nUpdateCookie++;

		LibraryDictionary.Invalidate();
	}
}

BOOL CLibraryFile::CheckFileAttributes(QWORD nSize, BOOL bSharedOnly, BOOL bAvailableOnly) const
{
	return ( nSize == SIZE_UNKNOWN || nSize == 0 || nSize == m_nSize ) &&
		( ! bSharedOnly || IsShared() ) &&
		( ! bAvailableOnly || IsAvailable() );
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile schema URI test

BOOL CLibraryFile::IsSchemaURI(LPCTSTR pszURI) const
{
	if ( m_pSchema == NULL )
	{
		return ( pszURI == NULL || *pszURI == NULL );
	}
	else
	{
		return m_pSchema->CheckURI( pszURI );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile rated (or commented)

BOOL CLibraryFile::IsRated() const
{
	return ( m_nRating || m_sComments.GetLength() );
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile rated but have no metadata

BOOL CLibraryFile::IsRatedOnly() const
{
	return IsRated() && ( m_pSchema == NULL || m_pMetadata == NULL );
}

BOOL CLibraryFile::IsHashed() const
{
	return m_oSHA1 && m_oTiger && m_oMD5 && m_oED2K; // m_oBTH ignored
}

BOOL CLibraryFile::IsNewFile() const
{
	return m_bNewFile;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile rebuild hashes and metadata

BOOL CLibraryFile::Rebuild()
{
	if ( ! IsAvailable() ) return FALSE;

	Library.RemoveFile( this );

	m_oSHA1.clear();
	m_oTiger.clear();
	m_oMD5.clear();
	m_oED2K.clear();
	m_nVirtualBase = m_nVirtualSize = 0;

	if ( m_pMetadata != NULL && m_bMetadataAuto )
	{
		delete m_pMetadata;
		m_pSchema	= NULL;
		m_pMetadata	= NULL;
	}

	Library.AddFile( this );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile rename

BOOL CLibraryFile::Rename(LPCTSTR pszName)
{
	if ( ! IsAvailable() ) return FALSE;
	if ( ! pszName || ! *pszName ) return FALSE;
	if ( _tcschr( pszName, '\\' ) ) return FALSE;

	CString strNew = m_pFolder->m_sPath + '\\' + pszName;

	// Close the file handle
	theApp.OnRename( GetPath() );

	if ( MoveFile( GetPath(), strNew ) )
	{
		// Success. Tell the file to use its new name
		theApp.OnRename( GetPath(), strNew );
	}
	else
	{
		// Failure. Continue using its old name
		theApp.OnRename( GetPath(), GetPath() );
		return FALSE;
	}

/*	if ( m_pMetadata != NULL )
	{
		CString strMetaFolder	= m_pFolder->m_sPath + _T("\\Metadata");
		CString strMetaOld		= strMetaFolder + '\\' + m_sName + _T(".xml");
		CString strMetaNew		= strMetaFolder + '\\' + pszName + _T(".xml");

		MoveFile( strMetaOld, strMetaNew );
	}*/

	Library.RemoveFile( this );

	m_sName = pszName;

	m_pFolder->OnFileRename( this );

	Library.AddFile( this );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile delete

BOOL CLibraryFile::Delete(BOOL bDeleteGhost)
{
	if ( IsAvailable() )
	{
		// Delete file
		BOOL bToRecycleBin = ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 );
		if ( ! DeleteFileEx( GetPath(), TRUE, bToRecycleBin, TRUE ) )
		{
			return FALSE;
		}
	}

	OnDelete( bDeleteGhost );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile metadata access

void CLibraryFile::UpdateMetadata(CDownload* pDownload)
{
	// Disable sharing of incomplete files
	if ( pDownload->m_bVerify == TRI_FALSE )
	{
		m_bVerify = m_bShared = TRI_FALSE;
		m_bBogus = TRUE;
	}

	// Get BTIH of recently downloaded file
	if ( ! m_oBTH && pDownload->IsSingleFileTorrent() )
	{
		m_oBTH = pDownload->m_oBTH;
	}

	// Get metadata of recently downloaded file
	if ( pDownload->HasMetadata() )
	{
		if ( m_pMetadata )
		{
			// Update existing
			BOOL bMetadataAuto = m_bMetadataAuto;
			if ( MergeMetadata( pDownload->m_pXML, FALSE ) )
			{
				if ( bMetadataAuto )
					m_bMetadataAuto = TRUE;
			}
		}
		else if ( CXMLElement* pBody = pDownload->m_pXML->GetFirstElement() )
		{
			// Recreate metadata
			TRACE( _T("Using download XML:%s"), pBody->ToString( FALSE, TRUE ) );
			m_pSchema = SchemaCache.Get( pDownload->m_pXML->GetAttributeValue(
				CXMLAttribute::schemaName ) );
			m_pMetadata = pBody->Clone();
			m_bMetadataAuto = TRUE;
			ModifyMetadata();
		}
	}
}

BOOL CLibraryFile::SetMetadata(CXMLElement*& pXML, BOOL bMerge, BOOL bOverwrite)
{
	ASSUME_LOCK( Library.m_pSection );

	if ( m_pMetadata == NULL && pXML == NULL )
		// No need
		return TRUE;

	CSchemaPtr pSchema = NULL;

	if ( pXML != NULL )
	{
		// Try fully specified XML first, for example <videos ...><video ... /></videos>
		pSchema = SchemaCache.Get( pXML->GetAttributeValue( CXMLAttribute::schemaName ) );
		if ( pSchema == NULL )
		{
			// Then try short version, for example <video ... />
			pSchema = SchemaCache.Guess( pXML->GetName() );
			if ( pSchema )
			{
				// Recreate full XML
				if ( CXMLElement* pFullXML = pSchema->Instantiate( TRUE ) )
				{
					if ( pFullXML->AddElement( pXML ) )
						pXML = pFullXML;
					else
						delete pFullXML;
				}
			}
		}

		if ( pSchema == NULL || ! pSchema->Validate( pXML, TRUE ) )
			// Invalid XML
			return FALSE;

		if ( m_pMetadata && bMerge )
		{
			pXML->GetFirstElement()->Merge( m_pMetadata, ! bOverwrite );
		}

		if ( m_pMetadata && m_pSchema == pSchema &&
			m_pMetadata->Equals( pXML->GetFirstElement() ) )
			// No need
			return TRUE;
	}

	Library.RemoveFile( this );

	delete m_pMetadata;

	m_pSchema		= pSchema;
	m_pMetadata		= pXML ? pXML->GetFirstElement()->Detach() : NULL;
	m_bMetadataAuto	= FALSE;

	delete pXML;
	pXML = NULL;

	ModifyMetadata();

	Library.AddFile( this );

	return TRUE;
}

BOOL CLibraryFile::MergeMetadata(CXMLElement*& pXML, BOOL bOverwrite)
{
	return SetMetadata( pXML, TRUE, bOverwrite );
}

void CLibraryFile::ClearMetadata()
{
	CXMLElement* pXML = NULL;
	SetMetadata( pXML );
}

void CLibraryFile::ModifyMetadata()
{
	m_bMetadataModified = TRUE;
	GetSystemTimeAsFileTime( &m_pMetadataTime );
}

CString CLibraryFile::GetMetadataWords() const
{
	if ( m_pSchema != NULL && m_pMetadata != NULL )
	{
		return m_pSchema->GetIndexedWords( m_pMetadata );
	}
	else
	{
		return CString();
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile hash volume lookups

CTigerTree* CLibraryFile::GetTigerTree()
{
	if ( ! m_oTiger ) return NULL;
	if ( ! IsAvailable() ) return NULL;

	CTigerTree* pTiger = new CTigerTree();

	if ( LibraryHashDB.GetTiger( m_nIndex, pTiger ) )
	{
		Hashes::TigerHash oRoot;
		pTiger->GetRoot( &oRoot[ 0 ] );
		oRoot.validate();
		if ( ! m_oTiger || m_oTiger == oRoot ) return pTiger;

		LibraryHashDB.DeleteTiger( m_nIndex );

		Library.RemoveFile( this );
		m_oTiger.clear();
		Library.AddFile( this );
	}

	delete pTiger;
	return NULL;
}

CED2K* CLibraryFile::GetED2K()
{
	if ( ! m_oED2K ) return NULL;
	if ( ! IsAvailable() ) return NULL;

	CED2K* pED2K = new CED2K();

	if ( LibraryHashDB.GetED2K( m_nIndex, pED2K ) )
	{
		Hashes::Ed2kHash oRoot;
		pED2K->GetRoot( &oRoot[ 0 ] );
		oRoot.validate();
		if ( m_oED2K == oRoot ) return pED2K;

		LibraryHashDB.DeleteED2K( m_nIndex );
	}

	delete pED2K;
	Library.RemoveFile( this );
	m_oED2K.clear();
	Library.AddFile( this );

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile alternate sources

CSharedSource* CLibraryFile::AddAlternateSources(LPCTSTR pszURLs)
{
	if ( pszURLs == NULL ) return NULL;
	if ( *pszURLs == 0 ) return NULL;

	CSharedSource* pFirst = NULL;

	CMapStringToFILETIME oUrls;
	SplitStringToURLs( pszURLs, oUrls );

	for ( POSITION pos = oUrls.GetStartPosition(); pos; )
	{
		CString strURL;
		FILETIME tSeen = {};
		oUrls.GetNextAssoc( pos, strURL, tSeen );
		if ( CSharedSource* pSource = AddAlternateSource( strURL, &tSeen ) )
		{
			pFirst = pSource;
		}
	}

	return pFirst;
}

CSharedSource* CLibraryFile::AddAlternateSource(LPCTSTR pszURL, FILETIME* tSeen)
{
	if ( pszURL == NULL ) return NULL;
	if ( *pszURL == 0 ) return NULL;

	CString strURL( pszURL );
	CShareazaURL pURL;

	BOOL bSeen;
	FILETIME tSeenLocal = { 0, 0 };
	if ( tSeen && tSeen->dwLowDateTime && tSeen->dwHighDateTime )
		bSeen = TRUE;
	else
	{
		tSeen = &tSeenLocal;
		bSeen = FALSE;
	}

	int nPos = strURL.ReverseFind( ' ' );
	if ( nPos > 0 )
	{
		CString strTime = strURL.Mid( nPos + 1 );
		strURL = strURL.Left( nPos );
		strURL.TrimRight();
		bSeen = TimeFromString( strTime, tSeen );
	}

	if ( ! pURL.Parse( strURL ) ) return NULL;

	if ( Network.IsFirewalledAddress( &pURL.m_pAddress, TRUE ) ||
		 Network.IsReserved( (IN_ADDR*)&pURL.m_pAddress ) ) return NULL;

	if ( pURL != *this ) return NULL;

	for ( POSITION pos = m_pSources.GetHeadPosition() ; pos ; )
	{
		CSharedSource* pSource = m_pSources.GetNext( pos );

		if ( pSource->m_sURL.CompareNoCase( strURL ) == 0 )
		{
			pSource->Freshen( bSeen ? tSeen : NULL );
			return pSource;
		}
	}

	CSharedSource* pSource = new CSharedSource( strURL, bSeen ? tSeen : NULL );
	m_pSources.AddTail( pSource );

	return pSource;
}

CString CLibraryFile::GetAlternateSources(CList< CString >* pState, int nMaximum, PROTOCOLID nProtocol)
{
	CString strSources;
	SYSTEMTIME stNow;
	FILETIME ftNow;

	GetSystemTime( &stNow );
	SystemTimeToFileTime( &stNow, &ftNow );

	for ( POSITION pos = m_pSources.GetHeadPosition() ; pos ; )
	{
		CSharedSource* pSource = m_pSources.GetNext( pos );

		if ( ! pSource->IsExpired( ftNow ) &&
			 ( pState == NULL || pState->Find( pSource->m_sURL ) == NULL ) )
		{
			if ( ( nProtocol == PROTOCOL_HTTP ) && ( _tcsncmp( pSource->m_sURL, _T("http://"), 7 ) != 0 ) )
				continue;

			if ( pState != NULL ) pState->AddTail( pSource->m_sURL );

			if ( pSource->m_sURL.Find( _T("Zhttp://") ) >= 0 ||
				pSource->m_sURL.Find( _T("Z%2C http://") ) >= 0 )
			{
				// Ignore buggy URLs
				TRACE( _T("CLibraryFile::GetAlternateSources() Bad URL: %s\n"), pSource->m_sURL );
			}
			else
			{
				CString strURL = pSource->m_sURL;
				strURL.Replace( _T(","), _T("%2C") );

				if ( strSources.GetLength() ) strSources += _T(", ");
				strSources += strURL;
				strSources += ' ';
				strSources += TimeToString( &pSource->m_pTime );
			}

			if ( nMaximum == 1 ) break;
			else if ( nMaximum > 1 ) nMaximum --;
		}
	}

	return strSources;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile serialize

void CLibraryFile::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ASSERT( m_sName.GetLength() );
		ar << m_sName;
		ar << m_nIndex;
		ar << m_nSize;
		ar.Write( &m_pTime, sizeof(m_pTime) );
		ar << m_bShared;

		ar << m_nVirtualSize;
		if ( m_nVirtualSize > 0 ) ar << m_nVirtualBase;

		SerializeOut( ar, m_oSHA1 );
		SerializeOut( ar, m_oTiger );
		SerializeOut( ar, m_oMD5 );
		SerializeOut( ar, m_oED2K );
		SerializeOut( ar, m_oBTH );
		ar << m_bVerify;

		if ( m_pSchema != NULL && m_pMetadata != NULL )
		{
			ar << m_pSchema->GetURI();
			m_pMetadata->Serialize( ar );
		}
		else
		{
			CString strURI;
			ar << strURI;
		}

		ar << m_nRating;
		ar << m_sComments;
		ar << m_sShareTags;

		ar << m_bMetadataAuto;
		ar.Write( &m_pMetadataTime, sizeof( m_pMetadataTime ) );
		m_bMetadataModified = FALSE;

		ar << m_nHitsTotal;
		ar << m_nUploadsTotal;
		ar << m_bCachedPreview;
		ar << m_bBogus;

		ar.WriteCount( m_pSources.GetCount() );

		for ( POSITION pos = m_pSources.GetHeadPosition() ; pos ; )
		{
			CSharedSource* pSource = m_pSources.GetNext( pos );
			pSource->Serialize( ar, nVersion );
		}
	}
	else
	{
		ar >> m_sName;
		ASSERT( m_sName.GetLength() );

		ar >> m_nIndex;

		if ( nVersion >= 17 )
		{
			ar >> m_nSize;
		}
		else
		{
			DWORD nSize;
			ar >> nSize;
			m_nSize = nSize;
		}

		ReadArchive( ar, &m_pTime, sizeof(m_pTime) );

		if ( nVersion >= 5 )
		{
			ar >> m_bShared;
		}
		else
		{
			BYTE bShared;
			ar >> bShared;
			m_bShared = bShared ? TRI_UNKNOWN : TRI_FALSE;
		}

		if ( nVersion >= 21 )
		{
			ar >> m_nVirtualSize;
			if ( m_nVirtualSize > 0 ) ar >> m_nVirtualBase;
		}

		SerializeIn( ar, m_oSHA1, nVersion );
		if ( nVersion >= 8 )
		{
			SerializeIn( ar, m_oTiger, nVersion );
		}
		else
		{
			m_oTiger.clear();
		}
		if ( nVersion >= 11 )
		{
			SerializeIn( ar, m_oMD5, nVersion );
		}
		else
		{
			m_oMD5.clear();
		}
		if ( nVersion >= 11 )
		{
			SerializeIn( ar, m_oED2K, nVersion );
		}
		else
		{
			m_oED2K.clear();
		}
		if ( nVersion >= 26 )
		{
			SerializeIn( ar, m_oBTH, nVersion );
		}
		else
		{
			m_oBTH.clear();
		}

		if ( nVersion >= 4 ) ar >> m_bVerify;

		CString strURI;
		ar >> strURI;

		if ( strURI.GetLength() )
		{
			if ( nVersion < 27 )
			{
				ar >> m_bMetadataAuto;
				if ( ! m_bMetadataAuto )
				{
					ReadArchive( ar, &m_pMetadataTime, sizeof(m_pMetadataTime) );
				}
			}
			m_pMetadata = new CXMLElement();
			m_pMetadata->Serialize( ar );
			m_pSchema = SchemaCache.Get( strURI );
			if ( m_pSchema == NULL )
			{
				delete m_pMetadata;
				m_pMetadata = NULL;
			}
		}

		if ( nVersion >= 13 )
		{
			ar >> m_nRating;
			ar >> m_sComments;
			if ( nVersion >= 16 ) ar >> m_sShareTags;
			if ( nVersion >= 27 )
			{
				ar >> m_bMetadataAuto;
				ReadArchive( ar, &m_pMetadataTime, sizeof(m_pMetadataTime) );
			}
			else
			{
				if ( m_bMetadataAuto && IsRated() )
				{
					ReadArchive( ar, &m_pMetadataTime, sizeof(m_pMetadataTime) );
				}
			}
		}
		m_bMetadataModified = FALSE;

		ar >> m_nHitsTotal;
		ar >> m_nUploadsTotal;
		if ( nVersion >= 14 ) ar >> m_bCachedPreview;
		if ( nVersion >= 20 ) ar >> m_bBogus;

		if ( nVersion >= 2 )
		{
			SYSTEMTIME stNow;
			FILETIME ftNow;

			GetSystemTime( &stNow );
			SystemTimeToFileTime( &stNow, &ftNow );

			for ( DWORD_PTR nSources = ar.ReadCount() ; nSources > 0 ; nSources-- )
			{
				CSharedSource* pSource = new CSharedSource();
				if ( pSource == NULL )
				{
					theApp.Message( MSG_ERROR, _T("Memory allocation error in CLibraryFile::Serialize") );
					break;
				}
				pSource->Serialize( ar, nVersion );

				if ( pSource->IsExpired( ftNow ) )
					delete pSource;
				else
					m_pSources.AddTail( pSource );
			}
		}

		// Rehash pre-version-22 audio files

		if ( nVersion < 22 && m_pSchema != NULL && m_pSchema->CheckURI( CSchema::uriAudio ) )
		{
			m_oSHA1.clear();
			m_oTiger.clear();
			m_oMD5.clear();
			m_oED2K.clear();
		}

		Library.AddFile( this );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile threaded scan

BOOL CLibraryFile::ThreadScan(CSingleLock& pLock, DWORD nScanCookie, QWORD nSize, FILETIME* pTime/*, LPCTSTR pszMetaData*/)
{
	ASSERT( IsAvailable() );

	m_nScanCookie = nScanCookie;

	if ( m_nSize != nSize || CompareFileTime( &m_pTime, pTime ) != 0 )
	{
		pLock.Lock();

		Library.RemoveFile( this );

		CopyMemory( &m_pTime, pTime, sizeof(FILETIME) );
		m_nSize = nSize;

		m_oSHA1.clear();
		m_oTiger.clear();
		m_oMD5.clear();
		m_oED2K.clear();

		Library.AddFile( this );

		pLock.Unlock();

		m_nUpdateCookie++;

		CFolderScanDlg::Update( m_sName,
			( m_nSize == SIZE_UNKNOWN ) ? 0 : (DWORD)( m_nSize / 1024 ) );

		return TRUE;
	}
	else
	{
		// If file is already in library but hashing was delayed - hash it again
		if ( m_nIndex && ! IsHashed() )
		{
			LibraryBuilder.Add( this );
		}

		CFolderScanDlg::Update( m_sName,
			( m_nSize == SIZE_UNKNOWN ) ? 0 : (DWORD)( m_nSize / 1024 ) );

		return m_bMetadataModified;
	}
}

BOOL CLibraryFile::IsReadable() const
{
	if ( ! IsAvailable() )
		return FALSE;

	HANDLE hFile = CreateFile( GetPath(), GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_DELETE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle( hFile );
		return TRUE;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile delete handler
// bDeleteGhost is used only when deleting ghost files

void CLibraryFile::OnDelete(BOOL bDeleteGhost, TRISTATE bCreateGhost)
{
	if ( IsAvailable() )
	{
		CThumbCache::Delete( GetPath() );

		if ( bCreateGhost == TRI_TRUE )
		{
			if ( ! IsRated() )
			{
				m_bShared = TRI_FALSE;

				CString strTransl;
				CString strUntransl = L"Ghost File";
				LoadString( strTransl, IDS_LIBRARY_GHOST_FILE );
				if ( strTransl == strUntransl )
				{
					m_sComments = strUntransl;
				}
				else
				{
					m_sComments = strTransl + L" (" + strUntransl + L")";
				}
			}
			Ghost();
			return;
		}
		else if ( IsRated() && bCreateGhost != TRI_FALSE )
		{
			Ghost();
			return;
		}
	}

	// Remove file from all albums and folders
	LibraryFolders.OnFileDelete( this, bDeleteGhost );

	// Remove file from library history
	LibraryHistory.OnFileDelete( this );

	// Remove tiger/ed2k hash trees
	LibraryHashDB.DeleteAll( m_nIndex );

	delete this;
}

void CLibraryFile::Ghost()
{
	SYSTEMTIME pTime;
	GetSystemTime( &pTime );
	SystemTimeToFileTime( &pTime, &m_pTime );

	// Remove file from library maps, builder and dictionaries
	Library.RemoveFile( this );

	// Remove file from all albums and folders (skipping ghost files)
	LibraryFolders.OnFileDelete( this, FALSE );

	// Remove file from library history
	LibraryHistory.OnFileDelete( this );

	// Remove tiger/ed2k hash trees
	LibraryHashDB.DeleteAll( m_nIndex );

	m_pFolder = NULL;
	m_sPath.Empty();
	Library.AddFile( this );
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile download verification

BOOL CLibraryFile::OnVerifyDownload(
	const Hashes::Sha1ManagedHash& oSHA1,
	const Hashes::TigerManagedHash& oTiger,
	const Hashes::Ed2kManagedHash& oED2K,
	const Hashes::BtManagedHash& oBTH,
	const Hashes::Md5ManagedHash& oMD5,
	LPCTSTR pszSources)
{
	if ( ! IsAvailable() ) return FALSE;

	if ( Settings.Downloads.VerifyFiles && m_bVerify == TRI_UNKNOWN && m_nVirtualSize == 0 )
	{
		if ( (bool)m_oSHA1 && (bool)oSHA1 && oSHA1.isTrusted() )
		{
			m_bVerify = ( m_oSHA1 == oSHA1 ) ? TRI_TRUE : TRI_FALSE;
		}
		if ( m_bVerify != TRI_FALSE && (bool)m_oTiger && (bool)oTiger && oTiger.isTrusted() )
		{
			m_bVerify = ( m_oTiger == oTiger ) ? TRI_TRUE : TRI_FALSE;
		}
		if ( m_bVerify != TRI_FALSE && (bool)m_oED2K && (bool)oED2K && oED2K.isTrusted() )
		{
			m_bVerify = ( m_oED2K == oED2K ) ? TRI_TRUE : TRI_FALSE;
		}
		if ( m_bVerify != TRI_FALSE && (bool)m_oMD5 && (bool)oMD5 && oMD5.isTrusted() )
		{
			m_bVerify = ( m_oMD5 == oMD5 ) ? TRI_TRUE : TRI_FALSE;
		}
		if ( m_bVerify != TRI_FALSE && (bool)m_oBTH && (bool)oBTH && oBTH.isTrusted() )
		{
			m_bVerify = ( m_oBTH == oBTH ) ? TRI_TRUE : TRI_FALSE;
		}

		if ( m_bVerify == TRI_TRUE )
		{
			theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_VERIFY_SUCCESS, (LPCTSTR)m_sName );
			Downloads.OnVerify( GetPath(), TRUE );
		}
		else if ( m_bVerify == TRI_FALSE )
		{
			m_bShared = TRI_FALSE;

			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_VERIFY_FAIL, (LPCTSTR)m_sName );
			Downloads.OnVerify( GetPath(), FALSE );

			return FALSE;
		}
	}

	if ( m_oSHA1 && m_nVirtualSize == 0 )
	{
		VersionChecker.CheckUpgradeHash( m_oSHA1, GetPath() );
	}

	AddAlternateSources( pszSources );

	return TRUE;
}


//////////////////////////////////////////////////////////////////////
// CSharedSource construction

CSharedSource::CSharedSource(LPCTSTR pszURL, FILETIME* pTime)
{
	ZeroMemory( &m_pTime, sizeof( m_pTime ) );

	if ( pszURL != NULL )
	{
		m_sURL = pszURL;
		Freshen( pTime );
	}
}

void CSharedSource::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar << m_sURL;
		ar.Write( &m_pTime, sizeof(FILETIME) );
	}
	else
	{
		ar >> m_sURL;

		if ( nVersion >= 10 )
		{
			ReadArchive( ar, &m_pTime, sizeof(FILETIME) );
		}
		else
		{
			DWORD nTemp;
			ar >> nTemp;
			Freshen();
		}
	}
}

void CSharedSource::Freshen(FILETIME* pTime)
{
	SYSTEMTIME tNow1;
	GetSystemTime( &tNow1 );

	if ( pTime != NULL )
	{
		FILETIME tNow2;

		SystemTimeToFileTime( &tNow1, &tNow2 );
		(LONGLONG&)tNow2 += 10000000;

		if ( CompareFileTime( pTime, &tNow2 ) <= 0 )
		{
			m_pTime = *pTime;
		}
		else
		{
			SystemTimeToFileTime( &tNow1, &m_pTime );
		}
	}
	else
	{
		SystemTimeToFileTime( &tNow1, &m_pTime );
	}
}

BOOL CSharedSource::IsExpired(FILETIME& tNow)
{
	LONGLONG nElapse = *((LONGLONG*)&tNow) - *((LONGLONG*)&m_pTime);
	return nElapse > (LONGLONG)Settings.Library.SourceExpire * 10000000;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile automation

IMPLEMENT_DISPATCH(CLibraryFile, LibraryFile)

STDMETHODIMP CLibraryFile::XLibraryFile::get_Hash(URN_TYPE nType, ENCODING nEncoding, BSTR FAR* psURN)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return pThis->m_xShareazaFile.get_Hash( nType, nEncoding, psURN );
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_URL(BSTR FAR* psURL)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return pThis->m_xShareazaFile.get_URL( psURL );
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return CApplication::GetApp( ppApplication );
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Library(ILibrary FAR* FAR* ppLibrary)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	*ppLibrary = (ILibrary*)Library.GetInterface( IID_ILibrary, TRUE );
	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Folder(ILibraryFolder FAR* FAR* ppFolder)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	if ( ! pThis->IsAvailable() )
		*ppFolder = NULL;
	else
		*ppFolder = (ILibraryFolder*)pThis->m_pFolder->GetInterface( IID_ILibraryFolder, TRUE );
	return *ppFolder != NULL ? S_OK : S_FALSE;
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Path(BSTR FAR* psPath)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	*psPath = CComBSTR( pThis->GetPath() ).Detach();
	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Name(BSTR FAR* psName)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return pThis->m_xShareazaFile.get_Name( psName );
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Shared(TRISTATE FAR* pnValue)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	*pnValue = pThis->m_bShared;
	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::put_Shared(TRISTATE nValue)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	pThis->m_bShared = nValue;
	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_EffectiveShared(VARIANT_BOOL FAR* pbValue)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	*pbValue = pThis->IsShared() ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Size(ULONGLONG FAR* pnSize)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	*pnSize = pThis->GetSize();
	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Index(LONG FAR* pnIndex)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	*pnIndex = (LONG)pThis->m_nIndex;
	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_URN(BSTR sURN, BSTR FAR* psURN)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return pThis->m_xShareazaFile.get_URN( sURN, psURN );
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_MetadataAuto(VARIANT_BOOL FAR* pbValue)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	*pbValue = pThis->m_bMetadataAuto ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Metadata(ISXMLElement FAR* FAR* ppXML)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	*ppXML = NULL;

	CQuickLock oLock( Library.m_pSection );

	if ( pThis->m_pSchema == NULL || pThis->m_pMetadata == NULL ) return S_OK;

	CXMLElement* pXML	= pThis->m_pSchema->Instantiate( TRUE );
	*ppXML				= (ISXMLElement*)CXMLCOM::Wrap( pXML, IID_ISXMLElement );

	pXML->AddElement( pThis->m_pMetadata->Clone() );

	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::put_Metadata(ISXMLElement FAR* pXML)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )

	CQuickLock oLock( Library.m_pSection );

	if ( CXMLElement* pReal = CXMLCOM::Unwrap( pXML ) )
	{
		return pThis->SetMetadata( pReal ) ? S_OK : E_FAIL;
	}
	else
	{
		pThis->ClearMetadata();
		return S_OK;
	}
}

STDMETHODIMP CLibraryFile::XLibraryFile::Execute()
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return CFileExecutor::Execute( pThis->GetPath(), TRUE ) ? S_OK : E_FAIL;
}

STDMETHODIMP CLibraryFile::XLibraryFile::SmartExecute()
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return CFileExecutor::Execute( pThis->GetPath(), FALSE ) ? S_OK : E_FAIL;
}

STDMETHODIMP CLibraryFile::XLibraryFile::Delete()
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return pThis->Delete() ? S_OK : E_FAIL;
}

STDMETHODIMP CLibraryFile::XLibraryFile::Rename(BSTR sNewName)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return pThis->Rename( CString( sNewName ) ) ? S_OK : E_FAIL;
}

STDMETHODIMP CLibraryFile::XLibraryFile::Copy(BSTR /*sNewPath*/)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return E_NOTIMPL;
}

STDMETHODIMP CLibraryFile::XLibraryFile::Move(BSTR /*sNewPath*/)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return E_NOTIMPL;
}

STDMETHODIMP CLibraryFile::XLibraryFile::MergeMetadata(ISXMLElement* pXML, VARIANT_BOOL bOverwrite, VARIANT_BOOL* pbValue)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )

	if ( ! pbValue )
		return E_POINTER;

	if ( CXMLElement* pReal = CXMLCOM::Unwrap( pXML ) )
	{
		*pbValue = pThis->MergeMetadata( pReal, bOverwrite ) ?
			VARIANT_TRUE : VARIANT_FALSE;
	}
	else
		*pbValue = VARIANT_FALSE;

	return S_OK;
}
