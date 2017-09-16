//
// SharedFile.cpp
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
#include "SharedFile.h"
#include "SharedFolder.h"
#include "AlbumFolder.h"
#include "Library.h"
#include "LibraryBuilder.h"
#include "LibraryDictionary.h"
#include "LibraryFolders.h"
#include "LibraryHistory.h"
#include "HashDatabase.h"
#include "Plugins.h"
#include "WndChild.h"
#include "WndMain.h"

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
,	m_tCreateTime		( 0 )
{
	ZeroMemory( &m_pTime, sizeof(m_pTime) );
	ZeroMemory( &m_pMetadataTime, sizeof(m_pMetadataTime) );
	if ( pszName )
	{
		m_sName = pszName;
		m_pSchema = SchemaCache.GuessByFilename( m_sName );
	}
	if ( pFolder )
		m_sPath = pFolder->m_sPath;

	EnableDispatch( IID_ILibraryFile );
}

CLibraryFile::~CLibraryFile()
{
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

const CLibraryFolder* CLibraryFile::GetFolderPtr() const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pFolder;
}

CString CLibraryFile::GetFolder() const
{
	if ( m_pFolder )
		return m_pFolder->m_sPath;
	else
		return CString();
}

CString CLibraryFile::GetPath() const
{
	if ( m_pFolder )
		return m_pFolder->m_sPath + _T('\\') + m_sName;
	else
		return m_sName;
}

CString CLibraryFile::GetSearchName() const
{
	int nBase = 0;
	CString str;

	if ( m_pFolder && m_pFolder->m_pParent )
	{
		for ( const CLibraryFolder* pFolder = m_pFolder ; ; pFolder = pFolder->m_pParent )
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
		str = m_pFolder->m_sPath.Mid( nBase + 1 ) + _T('\\') + m_sName;
	}

	ToLower( str );
	return str;
}

CXMLElement* CLibraryFile::CreateXML(CXMLElement* pRoot, BOOL bSharedOnly, XmlType nType) const
{
	if ( bSharedOnly && ! IsShared() )
		return NULL;

	CXMLElement* pFile;

	switch ( nType )
	{
	case xmlDC:
		pFile = pRoot->AddElement( _T("File") );
		if ( pFile )
		{
			pFile->AddAttribute( _T("Name"), m_sName );
			pFile->AddAttribute( _T("Size"), m_nSize );
			pFile->AddAttribute( _T("TTH"), m_oTiger.toString() );
		}
		break;

	default:
		pFile = pRoot->AddElement( _T("file") );
		if ( pFile )
		{
			if ( m_oSHA1 && m_oTiger )
			{
				pFile->AddElement( _T("id") )->SetValue( _T("urn:bitprint:") + m_oSHA1.toString() + _T('.') + m_oTiger.toString() );
			}
			else if ( m_oSHA1 )
			{
				pFile->AddElement( _T("id") )->SetValue( m_oSHA1.toUrn() );
			}
			else if ( m_oTiger )
			{
				pFile->AddElement( _T("id") )->SetValue( m_oTiger.toUrn() );
			}

			if ( m_oMD5 )
			{
				pFile->AddElement( _T("id") )->SetValue( m_oMD5.toUrn() );
			}

			if ( m_oED2K )
			{
				pFile->AddElement( _T("id") )->SetValue( m_oED2K.toUrn() );
			}

			if ( m_oBTH )
			{
				pFile->AddElement( _T("id") )->SetValue( m_oBTH.toUrn() );
			}

			if ( CXMLElement* pDescription = pFile->AddElement( _T("description") ) )
			{
				pDescription->AddElement( _T("name") )->SetValue( m_sName );

				CString str;
				str.Format( _T("%I64u"), GetSize() );
				pDescription->AddElement( _T("size") )->SetValue( str );
			}

			if ( m_pMetadata && m_pSchema )
			{
				if ( CXMLElement* pMetadata = pFile->AddElement( _T("metadata") ) )
				{
					pMetadata->AddAttribute( _T("xmlns:s"), m_pSchema->GetURI() );
					pMetadata->AddElement( m_pMetadata->Prefix( _T("s:") ) );
				}
			}
		}
	}

	return pFile;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile shared check

bool CLibraryFile::IsShared(bool bIgnoreOverride) const
{
	ASSUME_LOCK( Library.m_pSection );

	// Don't share offline files
	if ( m_pFolder && m_pFolder->IsOffline() )
		return false;

	// Don't share private torrents
	if ( IsPrivateTorrent() )
		return false;

	// Use override shared flag of file
	if ( m_bShared != TRI_UNKNOWN && ! bIgnoreOverride )
		return ( m_bShared == TRI_TRUE );

	// Ghost files by default shared, then use folder shared flag
	return ! m_pFolder || m_pFolder->IsShared();
}

void CLibraryFile::SetShared(bool bShared, bool bOverride)
{
	TRISTATE bNewShare = bOverride ? ( bShared ? TRI_TRUE : TRI_FALSE ) : TRI_UNKNOWN;

	// Don't share not verified files
	if ( m_bVerify == TRI_FALSE )
		bNewShare = TRI_FALSE;

	// Don't share private torrents
	if ( bNewShare != TRI_FALSE && IsPrivateTorrent() )
		bNewShare = TRI_FALSE;

	if ( bNewShare == TRI_UNKNOWN )
	{
		// Use folder default shared flag
		bool bFolderShared = ! m_pFolder || m_pFolder->IsShared();
		bNewShare = bShared ?
			( bFolderShared ? TRI_UNKNOWN : TRI_TRUE ) :
			( bFolderShared ? TRI_FALSE : TRI_UNKNOWN );
	}

	if ( m_bShared != bNewShare )
	{
		m_bShared = bNewShare;
		m_nUpdateCookie++;

		LibraryDictionary.Invalidate();
	}
}

bool CLibraryFile::IsPrivateTorrent() const
{
	return ( m_pSchema && m_pMetadata &&
		m_pSchema->CheckURI( CSchema::uriBitTorrent ) &&
		m_pMetadata->GetAttributeValue( _T("privateflag"), _T("true") ).CompareNoCase( _T("true") ) == 0 );
}

DWORD CLibraryFile::GetCreationTime()
{
	if ( m_tCreateTime )
		return m_tCreateTime;

	if ( m_pFolder && m_pFolder->IsOffline() )
		return 0;

	HANDLE hFile = CreateFile( CString( _T("\\\\?\\") ) + GetPath(), FILE_READ_ATTRIBUTES,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
		return 0;

	FILETIME ftLastWriteTime;
	BOOL bResult = GetFileTime( hFile, NULL, NULL, &ftLastWriteTime );

	CloseHandle( hFile );

	if ( ! bResult )
		return 0;

	return m_tCreateTime = (DWORD)( ( MAKEQWORD( ftLastWriteTime.dwLowDateTime,
		ftLastWriteTime.dwHighDateTime ) ) / 10000000ui64 - 11644473600ui64 );
}

BOOL CLibraryFile::SetCreationTime(DWORD tTime)
{
	if ( tTime > static_cast< DWORD >( time( NULL ) ) )
		return FALSE;

	m_tCreateTime = tTime;

	if ( m_pFolder && m_pFolder->IsOffline() )
		return FALSE;

	HANDLE hFile = CreateFile( CString( _T("\\\\?\\") ) + GetPath(), FILE_WRITE_ATTRIBUTES,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
		return FALSE;

	QWORD t = ( (QWORD)m_tCreateTime + 11644473600ui64 ) * 10000000ui64;
	FILETIME ftLastWriteTime = { (DWORD)t, (DWORD)( t >> 32 ) };
	BOOL bResult = SetFileTime( hFile, NULL, NULL, &ftLastWriteTime );

	CloseHandle( hFile );

	return bResult;
}

BOOL CLibraryFile::CheckFileAttributes(QWORD nSize, BOOL bSharedOnly, BOOL bAvailableOnly) const
{
	return ( nSize == SIZE_UNKNOWN || nSize == 0 || nSize == m_nSize ) &&
		( ! bAvailableOnly || m_pFolder ) &&
		( ! bSharedOnly || IsShared() );
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
	if ( ! m_pFolder ) return FALSE;

	Library.RemoveFile( this );

	m_oSHA1.clear();
	m_oTiger.clear();
	m_oMD5.clear();
	m_oED2K.clear();
	m_oBTH.clear();

	m_nVirtualBase = m_nVirtualSize = 0;

	if ( m_pMetadata && m_bMetadataAuto )
	{
		delete m_pMetadata;
		m_pMetadata	= NULL;
	}

	Library.AddFile( this );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile rename

BOOL CLibraryFile::Rename(LPCTSTR pszName)
{
	if ( ! m_pFolder ) return FALSE;
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
	if ( m_pFolder )
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

BOOL CLibraryFile::AddMetadata(const CLibraryFile* pFile)
{
	BOOL bAdded = FALSE;

	for ( POSITION pos = pFile->m_pSources.GetHeadPosition(); pos; )
	{
		const CSharedSource* pSource = pFile->m_pSources.GetNext( pos );
		if ( AddAlternateSource( pSource->m_sURL, &pSource->m_pTime ) )
		{
			bAdded = TRUE;
		}
	}

	if ( pFile->m_oBTH && ! m_oBTH )
	{
		m_oBTH = pFile->m_oBTH;
		bAdded = TRUE;
	}

	if ( pFile->m_bVerify == TRI_FALSE && m_bVerify != TRI_FALSE )
	{
		m_bVerify = TRI_FALSE;
		SetShared( false );
		bAdded = TRUE;
	}

	if ( pFile->m_bBogus && ! m_bBogus )
	{
		m_bBogus = TRUE;
		bAdded = TRUE;
	}

	if ( pFile->m_nHitsToday || pFile->m_nHitsTotal || pFile->m_nUploadsToday || pFile->m_nUploadsTotal )
	{
		m_nHitsToday += pFile->m_nHitsToday;
		m_nHitsTotal += pFile->m_nHitsTotal;
		m_nUploadsToday += pFile->m_nUploadsToday;
		m_nUploadsTotal += pFile->m_nUploadsTotal;
		bAdded = TRUE;
	}

	if ( pFile->m_pSchema && ! m_pSchema )
	{
		m_pSchema = pFile->m_pSchema;
		bAdded = TRUE;
	}

	if ( pFile->m_pMetadata )
	{
		CAutoPtr< CXMLElement > pXML( pFile->m_pMetadata->Clone() );
		if ( ! m_pMetadata || pXML->Merge( m_pMetadata, TRUE ) )
		{
			delete m_pMetadata;
			m_pMetadata = pXML.Detach();
			m_bMetadataAuto = pFile->m_bMetadataAuto;
			bAdded = TRUE;
		}
	}

	if ( pFile->m_nRating && ! m_nRating )
	{
		m_nRating = pFile->m_nRating;
		bAdded = TRUE;
	}

	if ( ! pFile->m_sComments.IsEmpty() && m_sComments.IsEmpty() )
	{
		CString strUntransl;
		strUntransl.LoadString( IDS_LIBRARY_GHOST_FILE );
		if ( _tcsistr( pFile->m_sComments, strUntransl ) == NULL )
		{
			m_sComments = pFile->m_sComments;
			bAdded = TRUE;
		}
	}

	if ( ! pFile->m_sShareTags.IsEmpty() && m_sShareTags.IsEmpty() )
	{
		m_sShareTags = pFile->m_sShareTags;
		bAdded = TRUE;
	}

	if ( bAdded )
		ModifyMetadata();

	return bAdded;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile metadata access

void CLibraryFile::UpdateMetadata(const CDownload* pDownload)
{
	// Disable sharing of incomplete files
	if ( pDownload->m_bVerify == TRI_FALSE )
	{
		m_bVerify = TRI_FALSE;
		SetShared( false );
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
			if ( MergeMetadata( pDownload->m_pXML ) )
			{
				// Preserve flag
				m_bMetadataAuto = bMetadataAuto;
			}
		}
		else if ( CXMLElement* pBody = pDownload->m_pXML->GetFirstElement() )
		{
			// Recreate metadata
			TRACE( "Using download XML:%s", (LPCSTR)CT2A( pBody->ToString( FALSE, TRUE ) ) );
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
			if ( pSchema == NULL )
			{
				pSchema = SchemaCache.GuessByFilename( m_sName );
			}
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
	else
	{
		pSchema = SchemaCache.GuessByFilename( m_sName );
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

BOOL CLibraryFile::MergeMetadata(const CXMLElement* pXML)
{
	BOOL bResult = FALSE;
	if ( CXMLElement* pCloned = pXML->Clone() )
	{
		bResult = SetMetadata( pCloned, TRUE, FALSE );
		delete pCloned;
	}
	return bResult;
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
	if ( ! m_oTiger )
		// Not hashed yet
		return NULL;

	if ( ! m_pFolder )
		// Virtual file
		return NULL;

	CAutoPtr< CTigerTree > pTiger( new CTigerTree() );
	if ( ! pTiger )
		// Out of memory
		return NULL;

	if ( ! LibraryHashDB.GetTiger( m_nIndex, pTiger ) )
		// Database error
		return NULL;

	pTiger->SetupParameters( m_nSize );

	Hashes::TigerHash oRoot;
	pTiger->GetRoot( &oRoot[ 0 ] );
	oRoot.validate();
	if ( m_oTiger != oRoot )
	{
		// Wrong hash
		LibraryHashDB.DeleteTiger( m_nIndex );

		Library.RemoveFile( this );
		m_oTiger.clear();
		Library.AddFile( this );

		return NULL;
	}

	// OK
	return pTiger.Detach();
}

CED2K* CLibraryFile::GetED2K()
{
	if ( ! m_oED2K )
		// Not hashed yet
		return NULL;

	if ( ! m_pFolder )
		// Virtual file
		return NULL;

	CAutoPtr< CED2K > pED2K( new CED2K() );
	if ( ! pED2K )
		// Out of memory
		return NULL;

	if ( ! LibraryHashDB.GetED2K( m_nIndex, pED2K ) )
		// Database error
		return NULL;

	Hashes::Ed2kHash oRoot;
	pED2K->GetRoot( &oRoot[ 0 ] );
	oRoot.validate();
	if ( m_oED2K != oRoot )
	{
		// Wrong hash
		LibraryHashDB.DeleteED2K( m_nIndex );

		Library.RemoveFile( this );
		m_oED2K.clear();
		Library.AddFile( this );

		return NULL;
	}

	return pED2K.Detach();
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

CSharedSource* CLibraryFile::AddAlternateSource(LPCTSTR pszURL, const FILETIME* tSeen)
{
	if ( pszURL == NULL ) return NULL;
	if ( *pszURL == 0 ) return NULL;

	CString strURL( pszURL );
	CShareazaURL pURL;

	FILETIME tSeenLocal = {};
	BOOL bSeen = tSeen && tSeen->dwLowDateTime && tSeen->dwHighDateTime;

	int nPos = strURL.ReverseFind( ' ' );
	if ( nPos > 0 )
	{
		CString strTime = strURL.Mid( nPos + 1 );
		strURL = strURL.Left( nPos );
		strURL.TrimRight();
		if ( TimeFromString( strTime, &tSeenLocal ) )
		{
			bSeen = TRUE;
			tSeen = &tSeenLocal;
		}
	}

	if ( ! pURL.Parse( strURL ) ) return NULL;

	if ( Network.IsFirewalledAddress( &pURL.m_pAddress, Settings.Connection.IgnoreOwnIP ) ||
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
				TRACE( "CLibraryFile::GetAlternateSources() Bad URL: %s\n", (LPCSTR)CT2A( pSource->m_sURL ) );
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
			if ( ! m_pMetadata )
				AfxThrowMemoryException();
			m_pMetadata->Serialize( ar );
			m_pSchema = SchemaCache.Get( strURI );
			if ( m_pSchema == NULL )
			{
				delete m_pMetadata;
				m_pMetadata = NULL;
			}
			// else schema URI changed
		}
		if ( m_pSchema == NULL )
		{
			m_pSchema = SchemaCache.GuessByFilename( m_sName );
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
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile threaded scan

BOOL CLibraryFile::ThreadScan(DWORD nScanCookie, QWORD nSize, FILETIME* pTime/*, LPCTSTR pszMetaData*/)
{
	ASSUME_LOCK( Library.m_pSection );
	ASSERT( m_pFolder );

	m_nScanCookie = nScanCookie;

	if ( m_nSize != nSize || CompareFileTime( &m_pTime, pTime ) != 0 )
	{
		Library.RemoveFile( this );

		CopyMemory( &m_pTime, pTime, sizeof(FILETIME) );
		m_nSize = nSize;

		m_oSHA1.clear();
		m_oTiger.clear();
		m_oMD5.clear();
		m_oED2K.clear();

		Library.AddFile( this );

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
	if ( ! m_pFolder )
		return FALSE;

	HANDLE hFile = CreateFile( CString( _T("\\\\?\\") ) + GetPath(), GENERIC_READ,
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
	if ( m_pFolder )
	{
		CThumbCache::Delete( GetPath() );

		if ( bCreateGhost == TRI_TRUE )
		{
			if ( ! IsRated() )
			{
				CString strTransl = LoadString( IDS_LIBRARY_GHOST_FILE );
				CString strUntransl;
				strUntransl.LoadString( IDS_LIBRARY_GHOST_FILE );
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
	ASSUME_LOCK( Library.m_pSection );

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

BOOL CLibraryFile::OnVerifyDownload(const CLibraryRecent* pRecent)
{
	ASSERT( IsAvailable() );
	ASSERT( IsHashed() );

	if ( Settings.Downloads.VerifyFiles && m_bVerify == TRI_UNKNOWN && m_nVirtualSize == 0 )
	{
		if ( (bool)m_oSHA1 && (bool)pRecent->m_oSHA1 && pRecent->m_oSHA1.isTrusted() )
		{
			m_bVerify = ( m_oSHA1 == pRecent->m_oSHA1 ) ? TRI_TRUE : TRI_FALSE;
		}
		if ( m_bVerify != TRI_FALSE && (bool)m_oTiger && (bool)pRecent->m_oTiger && pRecent->m_oTiger.isTrusted() )
		{
			m_bVerify = ( m_oTiger == pRecent->m_oTiger ) ? TRI_TRUE : TRI_FALSE;
		}
		if ( m_bVerify != TRI_FALSE && (bool)m_oED2K && (bool)pRecent->m_oED2K && pRecent->m_oED2K.isTrusted() )
		{
			m_bVerify = ( m_oED2K == pRecent->m_oED2K ) ? TRI_TRUE : TRI_FALSE;
		}
		if ( m_bVerify != TRI_FALSE && (bool)m_oMD5 && (bool)pRecent->m_oMD5 && pRecent->m_oMD5.isTrusted() )
		{
			m_bVerify = ( m_oMD5 == pRecent->m_oMD5 ) ? TRI_TRUE : TRI_FALSE;
		}
		if ( m_bVerify != TRI_FALSE && (bool)m_oBTH && (bool)pRecent->m_oBTH && pRecent->m_oBTH.isTrusted() )
		{
			m_bVerify = ( m_oBTH == pRecent->m_oBTH ) ? TRI_TRUE : TRI_FALSE;
		}

		Downloads.OnVerify( this, m_bVerify );

		if ( m_bVerify == TRI_TRUE )
		{
			theApp.Message( MSG_NOTICE, IDS_DOWNLOAD_VERIFY_SUCCESS, (LPCTSTR)m_sName );
		}
		else if ( m_bVerify == TRI_FALSE )
		{
			m_bShared = TRI_FALSE;
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_VERIFY_FAIL, (LPCTSTR)m_sName );
			return FALSE;
		}
	}

	AddAlternateSources( pRecent->m_sSources );

	// Notify library plugins
	if ( Plugins.OnNewFile( this ) )
		return TRUE;

	// Notify all windows about this file
	if ( CMainWnd* pMainWnd = theApp.SafeMainWnd() )
	{
		CChildWnd* pChildWnd = NULL;
		while ( ( pChildWnd = pMainWnd->m_pWindows.Find( NULL, pChildWnd ) ) != NULL )
		{
			if ( pChildWnd->OnNewFile( this ) )
				return TRUE;
		}
	}

	// Notify version checker
	if ( VersionChecker.CheckUpgradeHash( this ) )
		return TRUE;

	return TRUE;
}

BOOL CLibraryFile::PrepareDoc(LPCTSTR pszTemplate, CArray< CString >& oDocs) const
{
	ASSUME_LOCK( Library.m_pSection );

	CString strDoc( pszTemplate );

	if ( m_pMetadata && m_pSchema )
	{
		// Should be all meta data replacement
		const CXMLElement* pMetadata = m_pMetadata;
		for ( POSITION pos = pMetadata->GetAttributeIterator() ; pos ; )
		{
			CString str;
			const CXMLNode* pNode = pMetadata->GetNextAttribute( pos );
			str = pNode->GetName();
			CString strReplace = pNode->GetValue();
			if ( str == _T("seconds") || str == _T("minutes") )
			{
				double nTotalSecs = ( str == _T("minutes") ) ?
					_tstof( (LPCTSTR)strReplace ) * 60 : _tstof( (LPCTSTR)strReplace );
				int nSecs = int( nTotalSecs );
				int nHours = nSecs / 3600;
				nSecs -= nHours * 3600;
				int nMins = nSecs / 60;
				nSecs -= nMins * 60;

				str.Format( _T("%d"), nHours );
				ReplaceNoCase( strDoc, _T("$meta:hours$"), str );
				str.Format( _T("%d"), nMins );
				ReplaceNoCase( strDoc, _T("$meta:minutes$"), str );
				str.Format( _T("%d"), nSecs );
				ReplaceNoCase( strDoc, _T("$meta:seconds$"), str );

				if ( nHours )
					str.Format( _T("%d:%d:%.2d"), nHours, nMins, nSecs );
				else
					str.Format( _T("%d:%.2d"), nMins, nSecs );
				ReplaceNoCase( strDoc, _T("$meta:time$"), str );
			}
			else if ( str == "track" )
			{
				int nTrack = _ttoi( (LPCTSTR)strReplace );
				str.Format( _T("%d"), nTrack );
				ReplaceNoCase( strDoc, _T("$meta:track$"), str );
			}
			else
			{
				CString strOld;
				strOld.Format( _T("$meta:%s$"), (LPCTSTR)str );
				ReplaceNoCase( strDoc, strOld, strReplace );
			}
		}
	}

	CString strFileName, strNameURI, strSize, strMagnet;

	if ( m_sName )
	{
		strFileName = m_sName;
		strNameURI = URLEncode( m_sName );
	}

	if ( m_nSize != SIZE_UNKNOWN )
	{
		strSize.Format( _T("%I64u"), m_nSize ); // bytes
		ReplaceNoCase( strDoc, _T("$meta:sizebytes$"), strSize );

		CString strHumanSize;
		if ( m_nSize / ( 1024*1024 ) > 1 )
			strHumanSize.Format( _T("%.2f MB"), (float)m_nSize / 1024 / 1024 );
		else
			strHumanSize.Format( _T("%.2f KB"), (float)m_nSize / 1024 );
		ReplaceNoCase( strDoc, _T("$meta:size$"), strHumanSize );
	}

	if ( m_oSHA1 )
	{
		strMagnet = _T("xt=urn:sha1:") + m_oSHA1.toString();

		ReplaceNoCase( strDoc, _T("$meta:sha1$"), m_oSHA1.toString() );

		ReplaceNoCase( strDoc, _T("$meta:gnutella$"), _T("gnutella://urn:sha1:") + m_oSHA1.toString() + _T('/') + strNameURI + _T('/') );
	}

	if ( m_oTiger )
	{
		strMagnet = _T("xt=urn:tree:tiger/:") + m_oTiger.toString();

		ReplaceNoCase( strDoc, _T("$meta:tiger$"), m_oTiger.toString() );
	}

	if ( m_oSHA1 && m_oTiger )
	{
		strMagnet = _T("xt=urn:bitprint:") + m_oSHA1.toString() + _T('.') + m_oTiger.toString();

		ReplaceNoCase( strDoc, _T("$meta:bitprint$"), m_oSHA1.toString() + _T('.') + m_oTiger.toString() );
	}

	if ( m_oED2K )
	{
		if ( strMagnet.GetLength() ) strMagnet += _T("&amp;");
		strMagnet += _T("xt=urn:ed2khash:") + m_oED2K.toString();

		ReplaceNoCase( strDoc, _T("$meta:ed2khash$"), m_oED2K.toString() );

		if ( strSize.GetLength() )
			ReplaceNoCase( strDoc, _T("$meta:ed2k$"), _T("ed2k://|file|") + strNameURI + _T('|') + strSize + _T('|') + m_oED2K.toString() + _T("|/") );
	}

	if ( m_oMD5 )
	{
		if ( strMagnet.GetLength() ) strMagnet += _T("&amp;");
		strMagnet += _T("xt=urn:md5:") + m_oMD5.toString();

		ReplaceNoCase( strDoc, _T("$meta:md5$"), m_oMD5.toString() );
	}

	if ( m_oBTH )
	{
		if ( strMagnet.GetLength() ) strMagnet += _T("&amp;");
		strMagnet += _T("xt=urn:btih:") + m_oMD5.toString();

		ReplaceNoCase( strDoc, _T("$meta:btih$"), m_oBTH.toString() );
	}

	if ( strSize.GetLength() ) strMagnet += _T("&amp;xl=") + strSize;
	strMagnet = _T("magnet:?") + strMagnet + _T("&amp;dn=") + strNameURI;
	ReplaceNoCase( strDoc, _T("$meta:magnet$"), strMagnet );

	ReplaceNoCase( strDoc, _T("$meta:name$"), strFileName );
	if ( ! m_sComments.IsEmpty() )
		ReplaceNoCase( strDoc, _T("$meta:comments$"), m_sComments );

	CString strNumber;
	strNumber.Format( _T("%d"), (int)oDocs.GetCount() + 1 );
	ReplaceNoCase( strDoc, _T("$meta:number$"), strNumber );

	// Replace all "$meta:xxx$" which were left in the file to "N/A"
	while ( LPCTSTR szStart = StrStrI( strDoc, _T("$meta:") ) )
	{
		if ( LPCTSTR szEnd = StrChr( szStart + 6, _T('$') ) )
			strDoc.Replace( CString( szStart, (int)( szEnd - szStart + 1 ) ), _T("N/A") );
		else
			break;
	}

	oDocs.Add( strDoc );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSharedSource construction

CSharedSource::CSharedSource(LPCTSTR pszURL, const FILETIME* pTime)
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

void CSharedSource::Freshen(const FILETIME* pTime)
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

BOOL CSharedSource::IsExpired(FILETIME& tNow) const
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

STDMETHODIMP CLibraryFile::XLibraryFile::get_Magnet(BSTR FAR* psMagnet)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return pThis->m_xShareazaFile.get_Magnet( psMagnet );
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
	if ( ! pThis->m_pFolder )
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

	if ( pThis->m_pSchema == NULL ) return S_OK;

	CXMLElement* pXML	= pThis->m_pSchema->Instantiate( TRUE );
	*ppXML				= (ISXMLElement*)CXMLCOM::Wrap( pXML, IID_ISXMLElement );

	if ( pThis->m_pMetadata )
	{
		pXML->AddElement( pThis->m_pMetadata->Clone() );
	}

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
	return CFileExecutor::Execute( pThis->GetPath() ) ? S_OK : E_FAIL;
}

STDMETHODIMP CLibraryFile::XLibraryFile::SmartExecute()
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return CFileExecutor::Execute( pThis->GetPath() ) ? S_OK : E_FAIL;
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
