//
// SharedFile.cpp
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
#include "Settings.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "Library.h"
#include "HashDatabase.h"

#include "Network.h"
#include "Uploads.h"
#include "Downloads.h"
#include "SourceURL.h"
#include "FileExecutor.h"
#include "Buffer.h"

#include "XML.h"
#include "XMLCOM.h"
#include "Schema.h"
#include "SchemaCache.h"

#include "SHA.h"
#include "MD5.h"
#include "ED2K.h"
#include "TigerTree.h"

#include "Application.h"
#include "VersionChecker.h"
#include "DlgFolderScan.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CLibraryFile, CComObject)

BEGIN_INTERFACE_MAP(CLibraryFile, CComObject)
	INTERFACE_PART(CLibraryFile, IID_ILibraryFile, LibraryFile)
END_INTERFACE_MAP()


//////////////////////////////////////////////////////////////////////
// CLibraryFile construction

CLibraryFile::CLibraryFile(CLibraryFolder* pFolder, LPCTSTR pszName)
{
	EnableDispatch( IID_ILibraryFile );
	
	m_pFolder		= pFolder;
	m_pNextSHA1		= NULL;
	m_pNextTiger	= NULL;
	m_pNextED2K		= NULL;
	m_nScanCookie	= 0;
	m_nUpdateCookie	= 0;
	m_nSelectCookie	= 0;
	
	m_nIndex		= 0;
	m_nSize			= 0;
	m_bShared		= TS_UNKNOWN;
	m_nVirtualBase	= 0;
	m_nVirtualSize	= 0;
	
	m_bSHA1			= FALSE;
	m_bTiger		= FALSE;
	m_bMD5			= FALSE;
	m_bED2K			= FALSE;
	m_bVerify		= TS_UNKNOWN;
	
	m_pSchema		= NULL;
	m_pMetadata		= NULL;
	m_bMetadataAuto	= FALSE;
	m_nRating		= 0;
	
	m_nHitsToday		= 0;
	m_nHitsTotal		= 0;
	m_nUploadsToday		= 0;
	m_nUploadsTotal		= 0;
	m_bCachedPreview	= FALSE;
	m_bBogus			= FALSE;
	
	m_nSearchCookie	= 0;
	m_nSearchWords	= 0;
	m_nCollIndex	= 0;
	m_nIcon16		= -1;
	
	if ( pszName != NULL ) m_sName = pszName;
}

CLibraryFile::~CLibraryFile()
{
	Library.RemoveFile( this );
	
	if ( m_pMetadata != NULL ) delete m_pMetadata;
	
	for ( POSITION pos = m_pSources.GetHeadPosition() ; pos ; )
	{
		delete (CSharedSource*)m_pSources.GetNext( pos );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile path computation

CString CLibraryFile::GetPath() const
{
	if ( m_pFolder != NULL )
		return m_pFolder->m_sPath + '\\' + m_sName;
	else
		return CString();
}

CString CLibraryFile::GetSearchName() const
{
	int nBase = 0;
	CString str;
	
	if ( m_pFolder != NULL && m_pFolder->m_pParent != NULL )
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
	
	str = CharLower( str.GetBuffer() );
	return str;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile shared check

BOOL CLibraryFile::IsShared() const
{
	if ( LPCTSTR pszExt = _tcsrchr( m_sName, '.' ) )
	{
		pszExt++;
		
		if ( LPCTSTR pszFind = _tcsistr( Settings.Library.PrivateTypes, pszExt ) )
		{
			if ( pszFind[ _tcslen( pszExt ) ] == 0 ||
				 pszFind[ _tcslen( pszExt ) ] == '|' )
			{
				if ( pszFind == Settings.Library.PrivateTypes ||
					 pszFind[-1] == '|' )
				{
					return FALSE;
				}
			}
		}
	}
	
	if ( m_bShared )
	{
		if ( m_bShared == TS_TRUE ) return TRUE;
		if ( m_bShared == TS_FALSE ) return FALSE;
	}
	
	for ( CLibraryFolder* pFolder = m_pFolder ; pFolder ; pFolder = pFolder->m_pParent )
	{
		if ( pFolder->m_bShared )
		{
			if ( pFolder->m_bShared == TS_TRUE ) return TRUE;
			if ( pFolder->m_bShared == TS_FALSE ) return FALSE;
		}
	}
	
	return TRUE;
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
// CLibraryFile rebuild hashes and metadata

BOOL CLibraryFile::Rebuild()
{
	if ( m_pFolder == NULL ) return FALSE;
	
	Library.RemoveFile( this );
	
	m_bSHA1 = m_bTiger = m_bMD5 = m_bED2K = FALSE;
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
// CLibraryFile execute

BOOL CLibraryFile::Execute(BOOL bPrompt)
{
	if ( m_pFolder == NULL ) return FALSE;
	CString strPath = GetPath();
	Library.Unlock();
	BOOL bResult = CFileExecutor::Execute( strPath, ! bPrompt );
	Library.Lock();
	return bResult;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile rename

BOOL CLibraryFile::Rename(LPCTSTR pszName)
{
	if ( m_pFolder == NULL ) return FALSE;
	if ( ! pszName || ! *pszName ) return FALSE;
	if ( _tcschr( pszName, '\\' ) ) return FALSE;
	
	CString strNew = m_pFolder->m_sPath + '\\' + pszName;
	
	Uploads.OnRename( GetPath() );
	
	if ( MoveFile( GetPath(), strNew ) )
	{
		Uploads.OnRename( GetPath(), strNew );
	}
	else
	{
		Uploads.OnRename( GetPath(), GetPath() );
		return FALSE;
	}
	
	if ( m_pMetadata != NULL )
	{
		CString strMetaFolder	= m_pFolder->m_sPath + _T("\\Metadata");
		CString strMetaOld		= strMetaFolder + '\\' + m_sName + _T(".xml");
		CString strMetaNew		= strMetaFolder + '\\' + pszName + _T(".xml");
		
		MoveFile( strMetaOld, strMetaNew );
	}
	
	Library.RemoveFile( this );
	
	m_sName = pszName;
	
	m_pFolder->OnFileRename( this );
	
	Library.AddFile( this );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile delete

BOOL CLibraryFile::Delete()
{
	if ( m_pFolder != NULL )
	{
		Uploads.OnRename( GetPath(), NULL );
		
		LPTSTR pszPath = new TCHAR[ GetPath().GetLength() + 2 ];
		_tcscpy( pszPath, GetPath() );
		pszPath[ GetPath().GetLength() + 1 ] = 0;
		
		SHFILEOPSTRUCT pOp;
		ZeroMemory( &pOp, sizeof(pOp) );
		pOp.wFunc	= FO_DELETE;
		pOp.pFrom	= pszPath;
		pOp.fFlags	= FOF_ALLOWUNDO|FOF_NOCONFIRMATION;
		
		if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) pOp.fFlags &= ~FOF_ALLOWUNDO;
		
		int nReturn = SHFileOperation( &pOp );
		
		delete [] pszPath;
		
		if ( nReturn != 0 )
		{
			Uploads.OnRename( GetPath(), GetPath() );
			return FALSE;
		}
		
		if ( m_pMetadata != NULL || m_sComments.GetLength() || m_nRating > 0 )
		{
			CString strMetaFolder	= m_pFolder->m_sPath + _T("\\Metadata");
			CString strMetaFile		= strMetaFolder + '\\' + m_sName + _T(".xml");
			
			if ( DeleteFile( strMetaFile ) )
			{
				RemoveDirectory( strMetaFolder );
			}
		}
	}
	else
	{
		OnDelete();
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile metadata access

BOOL CLibraryFile::SetMetadata(CXMLElement* pXML)
{
	if ( m_pFolder == NULL ) return FALSE;
	if ( m_pMetadata == NULL && pXML == NULL ) return TRUE;
	
	CSchema* pSchema = NULL;
	
	if ( pXML != NULL )
	{
		pSchema = SchemaCache.Get( pXML->GetAttributeValue( CXMLAttribute::schemaName ) );
		if ( pSchema == NULL ) return FALSE;
		if ( ! pSchema->Validate( pXML, TRUE ) ) return FALSE;
		
		if ( m_pMetadata != NULL && m_pSchema == pSchema )
		{
			if ( m_pMetadata->Equals( pXML->GetFirstElement() ) ) return TRUE;
		}
	}
	
	Library.RemoveFile( this );
	
	if ( m_pMetadata != NULL )
	{
		delete m_pMetadata;
		m_pSchema		= NULL;
		m_pMetadata		= NULL;
		m_bMetadataAuto	= FALSE;
	}
	
	m_pSchema		= pSchema;
	m_pMetadata		= pXML ? pXML->GetFirstElement()->Detach() : NULL;
	m_bMetadataAuto	= FALSE;
	
	if ( m_pMetadata == NULL )
	{
		m_bSHA1 = m_bTiger = m_bMD5 = m_bED2K = NULL;
	}
	
	Library.AddFile( this );
	
	SaveMetadata();
	
	return TRUE;
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
	if ( m_bTiger == FALSE ) return NULL;
	if ( m_pFolder == NULL ) return NULL;
	
	CTigerTree* pTiger = new CTigerTree();
	
	if ( LibraryHashDB.GetTiger( m_nIndex, pTiger ) )
	{
		TIGEROOT pRoot;
		pTiger->GetRoot( &pRoot );
		if ( ! m_bTiger || m_pTiger == pRoot ) return pTiger;
		
		LibraryHashDB.DeleteTiger( m_nIndex );
		
		Library.RemoveFile( this );
		m_bTiger = FALSE;
		Library.AddFile( this );
	}
	
	delete pTiger;
	return NULL;
}

CED2K* CLibraryFile::GetED2K()
{
	if ( m_bED2K == FALSE ) return NULL;
	if ( m_pFolder == NULL ) return NULL;
	
	CED2K* pED2K = new CED2K();
	
	if ( LibraryHashDB.GetED2K( m_nIndex, pED2K ) )
	{
		MD4 pRoot;
		pED2K->GetRoot( &pRoot );
		if ( m_pED2K == pRoot ) return pED2K;
		
		LibraryHashDB.DeleteED2K( m_nIndex );
	}
	
	delete pED2K;
	Library.RemoveFile( this );
	m_bED2K = FALSE;
	Library.AddFile( this );
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile alternate sources

CSharedSource* CLibraryFile::AddAlternateSources(LPCTSTR pszURL)
{
	CString strURLs( pszURL );
	CSharedSource* pFirst = NULL;
	BOOL bQuote = FALSE;
	
	for ( int nScan = 0 ; nScan < strURLs.GetLength() ; nScan++ )
	{
		if ( strURLs[ nScan ] == '\"' )
		{
			bQuote = ! bQuote;
			strURLs.SetAt( nScan, ' ' );
		}
		else if ( strURLs[ nScan ] == ',' && bQuote )
		{
			strURLs.SetAt( nScan, '`' );
		}
	}
	
	strURLs += ',';
	
	for ( int nCount = 0 ; ; )
	{
		int nPos = strURLs.Find( ',' );
		if ( nPos < 0 ) break;
		
		CString strURL	= strURLs.Left( nPos );
		strURLs			= strURLs.Mid( nPos + 1 );
		strURL.TrimLeft();
		
		if ( _tcsistr( strURL, _T("://") ) != NULL )
		{
			for ( int nScan = 0 ; nScan < strURL.GetLength() ; nScan++ )
			{
				if ( strURL[ nScan ] == '`' ) strURL.SetAt( nScan, ',' );
			}
		}
		else
		{
			nPos = strURL.Find( ':' );
			if ( nPos < 1 ) continue;
			
			int nPort = 0;
			_stscanf( strURL.Mid( nPos + 1 ), _T("%i"), &nPort );
			strURL.Truncate( nPos );
			USES_CONVERSION;
			DWORD nAddress = inet_addr( T2CA( strURL ) );
			strURL.Empty();
			
			if ( ! Network.IsFirewalledAddress( &nAddress, TRUE ) && nPort != 0 && nAddress != INADDR_NONE )
			{
				if ( m_bSHA1 )
				{
					strURL.Format( _T("http://%s:%i/uri-res/N2R?%s"),
						(LPCTSTR)CString( inet_ntoa( *(IN_ADDR*)&nAddress ) ),
						nPort, (LPCTSTR)CSHA::HashToString( &m_pSHA1, TRUE ) );
				}
			}
		}
		
		if ( CSharedSource* pSource = AddAlternateSource( strURL, FALSE ) )
		{
			pFirst = pSource;
			nCount++;
		}
	}
	
	return pFirst;
}

CSharedSource* CLibraryFile::AddAlternateSource(LPCTSTR pszURL, BOOL bForce)
{
	if ( pszURL == NULL ) return NULL;
	if ( *pszURL == 0 ) return NULL;
	
	CString strURL( pszURL );
	CSourceURL pURL;
	
	FILETIME tSeen = { 0, 0 };
	BOOL bSeen = FALSE;
	
	int nPos = strURL.ReverseFind( ' ' );
	
	if ( nPos > 0 )
	{
		CString strTime = strURL.Mid( nPos + 1 );
		strURL = strURL.Left( nPos );
		strURL.TrimRight();
		bSeen = TimeFromString( strTime, &tSeen );
	}
	
	// if ( ! bSeen && ! bForce ) return NULL;
	if ( ! pURL.Parse( strURL ) ) return NULL;
	
	if ( memcmp( &pURL.m_pAddress, &Network.m_pHost.sin_addr,
		 sizeof(IN_ADDR) ) == 0 ) return NULL;
	
	if ( Network.IsFirewalledAddress( &pURL.m_pAddress, TRUE ) ) return FALSE;
	
	if ( pURL.m_bSHA1 && m_bSHA1 && pURL.m_pSHA1 != m_pSHA1 ) return NULL;
	
	for ( POSITION pos = m_pSources.GetHeadPosition() ; pos ; )
	{
		CSharedSource* pSource = (CSharedSource*)m_pSources.GetNext( pos );
		
		if ( pSource->m_sURL.CompareNoCase( strURL ) == 0 )
		{
			pSource->Freshen( bSeen ? &tSeen : NULL );
			return pSource;
		}
	}
	
	CSharedSource* pSource = new CSharedSource( strURL, bSeen ? &tSeen : NULL );
	m_pSources.AddTail( pSource );
	
	return pSource;
}

CString CLibraryFile::GetAlternateSources(CStringList* pState, int nMaximum, BOOL bHTTP)
{
	CString strSources;
	SYSTEMTIME stNow;
	FILETIME ftNow;
	
	GetSystemTime( &stNow );
	SystemTimeToFileTime( &stNow, &ftNow );
	
	for ( POSITION pos = m_pSources.GetHeadPosition() ; pos ; )
	{
		CSharedSource* pSource = (CSharedSource*)m_pSources.GetNext( pos );
		
		if ( ! pSource->IsExpired( ftNow ) &&
			 ( pState == NULL || pState->Find( pSource->m_sURL ) == NULL ) )
		{
			if ( pState != NULL ) pState->AddTail( pSource->m_sURL );
			
			if ( bHTTP && _tcsncmp( pSource->m_sURL, _T("http://"), 7 ) != 0 ) continue;
			
			CString strURL = pSource->m_sURL;
			Replace( strURL, _T(","), _T("%2C") );

			if ( strSources.GetLength() ) strSources += _T(", ");
			strSources += strURL;
			strSources += ' ';
			strSources += TimeToString( &pSource->m_pTime );

			if ( nMaximum == 1 ) break;
			else if ( nMaximum > 1 ) nMaximum --;
		}
	}
	
	if ( strSources.Find( _T("Zhttp://") ) >= 0 ) strSources.Empty();
	
	return strSources;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile serialize

void CLibraryFile::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar << m_sName;
		ar << m_nIndex;
		ar << m_nSize;
		ar.Write( &m_pTime, sizeof(m_pTime) );
		ar << m_bShared;
		
		ar << m_nVirtualSize;
		if ( m_nVirtualSize > 0 ) ar << m_nVirtualBase;
		
		ar << m_bSHA1;
		if ( m_bSHA1 ) ar.Write( &m_pSHA1, sizeof(SHA1) );
		ar << m_bTiger;
		if ( m_bTiger ) ar.Write( &m_pTiger, sizeof(TIGEROOT) );
		ar << m_bMD5;
		if ( m_bMD5 ) ar.Write( &m_pMD5, sizeof(MD5) );
		ar << m_bED2K;
		if ( m_bED2K ) ar.Write( &m_pED2K, sizeof(MD4) );
		ar << m_bVerify;
		
		if ( m_pSchema != NULL && m_pMetadata != NULL )
		{
			ar << m_pSchema->m_sURI;
			ar << m_bMetadataAuto;
			if ( ! m_bMetadataAuto ) ar.Write( &m_pMetadataTime, sizeof(m_pMetadataTime) );
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
		
		if ( m_bMetadataAuto && ( m_nRating || m_sComments.GetLength() ) )
		{
			ar.Write( &m_pMetadataTime, sizeof(m_pMetadataTime) );
		}
		
		ar << m_nHitsTotal;
		ar << m_nUploadsTotal;
		ar << m_bCachedPreview;
		ar << m_bBogus;
		
		ar.WriteCount( m_pSources.GetCount() );
		
		for ( POSITION pos = m_pSources.GetHeadPosition() ; pos ; )
		{
			CSharedSource* pSource = (CSharedSource*)m_pSources.GetNext( pos );
			pSource->Serialize( ar, nVersion );
		}
	}
	else
	{
		ar >> m_sName;
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
		
		ar.Read( &m_pTime, sizeof(m_pTime) );
		
		if ( nVersion >= 5 )
		{
			ar >> m_bShared;
		}
		else
		{
			BYTE bShared;
			ar >> bShared;
			m_bShared = bShared ? TS_UNKNOWN : TS_FALSE;
		}
		
		if ( nVersion >= 21 )
		{
			ar >> m_nVirtualSize;
			if ( m_nVirtualSize > 0 ) ar >> m_nVirtualBase;
		}
		
		ar >> m_bSHA1;
		if ( m_bSHA1 ) ar.Read( &m_pSHA1, sizeof(SHA1) );
		if ( nVersion >= 8 ) ar >> m_bTiger; else m_bTiger = FALSE;
		if ( m_bTiger ) ar.Read( &m_pTiger, sizeof(TIGEROOT) );
		if ( nVersion >= 11 ) ar >> m_bMD5; else m_bMD5 = FALSE;
		if ( m_bMD5 ) ar.Read( &m_pMD5, sizeof(MD5) );
		if ( nVersion >= 11 ) ar >> m_bED2K; else m_bED2K = FALSE;
		if ( m_bED2K ) ar.Read( &m_pED2K, sizeof(MD4) );
		
		if ( nVersion >= 4 ) ar >> m_bVerify;
		
		CString strURI;
		ar >> strURI;
		
		if ( strURI.GetLength() )
		{
			ar >> m_bMetadataAuto;
			if ( ! m_bMetadataAuto ) ar.Read( &m_pMetadataTime, sizeof(m_pMetadataTime) );
			
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
			
			if ( m_bMetadataAuto && ( m_nRating || m_sComments.GetLength() ) )
			{
				ar.Read( &m_pMetadataTime, sizeof(m_pMetadataTime) );
			}
		}
		
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
			
			for ( int nSources = ar.ReadCount() ; nSources > 0 ; nSources-- )
			{
				CSharedSource* pSource = new CSharedSource();
				pSource->Serialize( ar, nVersion );
				
				if ( pSource->IsExpired( ftNow ) )
					delete pSource;
				else
					m_pSources.AddTail( pSource );
			}
		}
		
		// Rehash pre-version-22 audio files
		
		if ( nVersion < 22 && m_pSchema != NULL && m_pSchema->m_sURI.CompareNoCase( CSchema::uriAudio ) == 0 )
		{
			m_bSHA1 = m_bTiger = m_bMD5 = m_bED2K = FALSE;
		}
		
		Library.AddFile( this );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile threaded scan

BOOL CLibraryFile::ThreadScan(CSingleLock& pLock, DWORD nScanCookie, QWORD nSize, FILETIME* pTime, LPCTSTR pszMetaData)
{
	BOOL bChanged = FALSE;
	
	ASSERT( m_pFolder != NULL );
	m_nScanCookie = nScanCookie;
	
	if ( m_nSize != nSize || CompareFileTime( &m_pTime, pTime ) != 0 )
	{
		bChanged = TRUE;
		pLock.Lock();
		Library.RemoveFile( this );
		
		CopyMemory( &m_pTime, pTime, sizeof(FILETIME) );
		m_nSize = nSize;
		
		m_bSHA1 = m_bTiger = m_bMD5 = m_bED2K = FALSE;
		
		if ( m_pMetadata != NULL && m_bMetadataAuto )
		{
			delete m_pMetadata;
			m_pSchema		= NULL;
			m_pMetadata		= NULL;
			m_bMetadataAuto	= FALSE;
		}
	}
	
	HANDLE hFile	= INVALID_HANDLE_VALUE;
	BOOL bMetaData	= FALSE;
	FILETIME pMetaDataTime;
	
	if ( pszMetaData != NULL )
	{
		CString strMetaData = pszMetaData + m_sName + _T(".xml");
		
		if ( Library.m_pfnGFAEW != NULL && theApp.m_bNT )
		{
			USES_CONVERSION;
			WIN32_FILE_ATTRIBUTE_DATA pInfo;
			bMetaData = (*Library.m_pfnGFAEW)( T2CW( (LPCTSTR)strMetaData ), GetFileExInfoStandard, &pInfo );
			pMetaDataTime = pInfo.ftLastWriteTime;
		}
		else if ( Library.m_pfnGFAEA != NULL )
		{
			USES_CONVERSION;
			WIN32_FILE_ATTRIBUTE_DATA pInfo;
			bMetaData = (*Library.m_pfnGFAEA)( T2CA( (LPCTSTR)strMetaData ), GetFileExInfoStandard, &pInfo );
			pMetaDataTime = pInfo.ftLastWriteTime;
		}
		else
		{
			hFile = CreateFile( strMetaData, GENERIC_READ, FILE_SHARE_READ,
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			
			if ( hFile != INVALID_HANDLE_VALUE )
			{
				bMetaData = TRUE;
				GetFileTime( hFile, NULL, NULL, &pMetaDataTime );
			}
		}
	}
	
	if ( bMetaData )
	{
		if ( CompareFileTime( &m_pMetadataTime, &pMetaDataTime ) != 0 )
		{
			CopyMemory( &m_pMetadataTime, &pMetaDataTime, sizeof(FILETIME) );
			
			if ( hFile == INVALID_HANDLE_VALUE )
			{
				hFile = CreateFile( pszMetaData + m_sName + _T(".xml"),
					GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL, NULL );
			}
			
			if ( hFile != INVALID_HANDLE_VALUE )
			{
				if ( ! bChanged )
				{
					bChanged = TRUE;
					pLock.Lock();
					Library.RemoveFile( this );
				}
				
				LoadMetadata( hFile );
			}
		}
		
		if ( hFile != INVALID_HANDLE_VALUE ) CloseHandle( hFile );
	}
	else if ( m_pMetadata != NULL && ! m_bMetadataAuto )
	{
		BOOL bLocked = ! bChanged;
		if ( bLocked ) pLock.Lock();
		
		if ( m_pMetadata != NULL && ! m_bMetadataAuto )
		{
			if ( ! bChanged )
			{
				bChanged = TRUE;
				bLocked = FALSE;
				Library.RemoveFile( this );
			}
			
			ZeroMemory( &m_pMetadataTime, sizeof(FILETIME) );
			LoadMetadata( INVALID_HANDLE_VALUE );
			
			m_bSHA1 = m_bTiger = m_bMD5 = m_bED2K = FALSE;
		}
		
		if ( bLocked ) pLock.Unlock();
	}
	
	if ( bChanged )
	{
		Library.AddFile( this );
		CFolderScanDlg::Update( m_sName, (DWORD)( m_nSize / 1024 ), FALSE );
		pLock.Unlock();
		m_nUpdateCookie++;
	}
	else
	{
		CFolderScanDlg::Update( m_sName, (DWORD)( m_nSize / 1024 ), TRUE );
	}
	
	return bChanged;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile metadata file I/O

BOOL CLibraryFile::LoadMetadata(HANDLE hFile)
{
	ASSERT( m_pFolder != NULL );
	
	if ( m_bMetadataAuto == FALSE )
	{
		if ( m_pMetadata != NULL ) delete m_pMetadata;
		m_pSchema	= NULL;
		m_pMetadata	= NULL;
	}
	
	if ( hFile == INVALID_HANDLE_VALUE ) return FALSE;
	
	CXMLElement* pXML = CXMLElement::FromFile( hFile );
	if ( pXML == NULL ) return FALSE;
	
	if ( CXMLElement* pComment = pXML->GetElementByName( _T("comment") ) )
	{
		m_nRating = -1;
		CString strRating = pComment->GetAttributeValue( _T("rating") );
		_stscanf( strRating, _T("%i"), &m_nRating );
		m_nRating	= max( 0, min( 6, m_nRating + 1 ) );
		m_sComments	= pComment->GetValue();
		Replace( m_sComments, _T("{n}"), _T("\r\n") );
		pComment->Delete();
	}
	
	CSchema* pSchema = SchemaCache.Get( pXML->GetAttributeValue( CXMLAttribute::schemaName, _T("") ) );
	
	if ( pSchema == NULL || pXML->GetFirstElement() == NULL )
	{
		delete pXML;
		return FALSE;
	}
	
	pSchema->Validate( pXML, TRUE );
	
	if ( m_pMetadata != NULL ) delete m_pMetadata;
	
	m_pSchema		= pSchema;
	m_pMetadata		= pXML->GetFirstElement()->Detach();
	m_bMetadataAuto	= FALSE;
	
	delete pXML;
	
	return TRUE;
}

BOOL CLibraryFile::SaveMetadata()
{
	CXMLElement* pXML = NULL;
	
	m_nUpdateCookie++;
	
	if ( m_pFolder == NULL ) return TRUE;
	
	if ( m_pSchema != NULL && m_pMetadata != NULL && ! m_bMetadataAuto )
	{
		pXML = m_pSchema->Instantiate( TRUE );
		pXML->AddElement( m_pMetadata->Clone() );
	}
	
	if ( m_nRating > 0 || m_sComments.GetLength() > 0 )
	{
		if ( pXML == NULL )
		{
			pXML = new CXMLElement( NULL, _T("comments") );
		}
		
		CXMLElement* pComment = pXML->AddElement( _T("comment") );
		
		if ( m_nRating > 0 )
		{
			CString strRating;
			strRating.Format( _T("%i"), m_nRating - 1 );
			pComment->AddAttribute( _T("rating"), strRating );
		}
		
		if ( m_sComments.GetLength() > 0 )
		{
			CString strComments( m_sComments );
			Replace( strComments, _T("\r\n"), _T("{n}") );
			pComment->SetValue( strComments );
		}
	}
	
	CString strMetaFolder	= m_pFolder->m_sPath + _T("\\Metadata");
	CString strMetaFile		= strMetaFolder + _T("\\") + m_sName + _T(".xml");
	
	if ( pXML != NULL )
	{
		pXML->AddAttribute( _T("xmlns:xsi"), CXMLAttribute::xmlnsInstance );
		
		HANDLE hFile = CreateFile( strMetaFile, GENERIC_WRITE, FILE_SHARE_READ, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		
		if ( hFile == INVALID_HANDLE_VALUE )
		{
			CreateDirectory( strMetaFolder, NULL );
			SetFileAttributes( strMetaFolder, FILE_ATTRIBUTE_HIDDEN );
			
			hFile = CreateFile( strMetaFile, GENERIC_WRITE, FILE_SHARE_READ, NULL,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
			
			if ( hFile == INVALID_HANDLE_VALUE ) return FALSE;
		}
		
		CString strXML = pXML->ToString( TRUE, TRUE );
		DWORD nWritten;
		delete pXML;
		
#ifdef _UNICODE
		int nASCII = WideCharToMultiByte( CP_UTF8, 0, strXML, strXML.GetLength(), NULL, 0, NULL, NULL );
		LPSTR pszASCII = new CHAR[ nASCII ];
		WideCharToMultiByte( CP_UTF8, 0, strXML, strXML.GetLength(), pszASCII, nASCII, NULL, NULL );
		WriteFile( hFile, pszASCII, nASCII, &nWritten, NULL );
		delete [] pszASCII;
#else
		WriteFile( hFile, (LPCSTR)strXML, strXML.GetLength(), &nWritten, NULL );
#endif
		
		GetFileTime( hFile, NULL, NULL, &m_pMetadataTime );
		CloseHandle( hFile );
	}
	else
	{
		DeleteFile( strMetaFile );
		RemoveDirectory( strMetaFolder );
		ZeroMemory( &m_pMetadataTime, sizeof(FILETIME) );
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile delete handler

void CLibraryFile::OnDelete()
{
	if ( m_pFolder != NULL )
	{
		if ( m_nRating > 0 || m_sComments.GetLength() > 0 )
		{
			Ghost();
			return;
		}
	}
	
	Library.OnFileDelete( this );
	
	delete this;
}

void CLibraryFile::Ghost()
{
	SYSTEMTIME pTime;
	GetSystemTime( &pTime );
	SystemTimeToFileTime( &pTime, &m_pTime );
	Library.RemoveFile( this );
	m_pFolder = NULL;
	Library.AddFile( this );
	Library.OnFileDelete( this );
}

//////////////////////////////////////////////////////////////////////
// CLibraryFile download verification

BOOL CLibraryFile::OnVerifyDownload(const SHA1* pSHA1, const MD4* pED2K, LPCTSTR pszSources)
{
	if ( m_pFolder == NULL ) return FALSE;
	
	if ( Settings.Downloads.VerifyFiles && m_bVerify == TS_UNKNOWN && m_nVirtualSize == 0 )
	{
		if ( m_bSHA1 && pSHA1 != NULL )
		{
			m_bVerify = ( m_pSHA1 == *pSHA1 ) ? TS_TRUE : TS_FALSE;
		}
		else if ( m_bED2K && pED2K != NULL )
		{
			m_bVerify = ( m_pED2K == *pED2K ) ? TS_TRUE : TS_FALSE;
		}
		
		if ( m_bVerify == TS_TRUE )
		{
			theApp.Message( MSG_SYSTEM, IDS_DOWNLOAD_VERIFY_SUCCESS, (LPCTSTR)m_sName );
			Downloads.OnVerify( GetPath(), TRUE );
		}
		else if ( m_bVerify == TS_FALSE )
		{
			m_bShared = TS_FALSE;
			
			theApp.Message( MSG_ERROR, IDS_DOWNLOAD_VERIFY_FAIL, (LPCTSTR)m_sName );
			Downloads.OnVerify( GetPath(), FALSE );
			
			return FALSE;
		}
	}
	
	if ( m_bSHA1 && m_nVirtualSize == 0 && VersionChecker.m_bUpgrade )
	{
		VersionChecker.CheckUpgradeHash( &m_pSHA1, GetPath() );
	}
	
	if ( pszSources != NULL && *pszSources != 0 )
	{
		AddAlternateSources( pszSources );
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////
// CSharedSource construction

CSharedSource::CSharedSource(LPCTSTR pszURL, FILETIME* pTime)
{
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
			ar.Read( &m_pTime, sizeof(FILETIME) );
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

STDMETHODIMP CLibraryFile::XLibraryFile::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	*ppApplication = Application.GetApp();
	return S_OK;
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
	if ( pThis->m_pFolder == NULL )
		*ppFolder = NULL;
	else
		*ppFolder = (ILibraryFolder*)pThis->m_pFolder->GetInterface( IID_ILibraryFolder, TRUE );
	return *ppFolder != NULL ? S_OK : S_FALSE;
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Path(BSTR FAR* psPath)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	pThis->GetPath().SetSysString( psPath );
	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Name(BSTR FAR* psPath)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	pThis->m_sName.SetSysString( psPath );
	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::get_Shared(STRISTATE FAR* pnValue)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	*pnValue = (STRISTATE)pThis->m_bShared;
	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::put_Shared(STRISTATE nValue)
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

STDMETHODIMP CLibraryFile::XLibraryFile::get_Size(LONG FAR* pnSize)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	*pnSize = (LONG)pThis->GetSize();
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
	CString strURN( sURN );
	
	if ( strURN.IsEmpty() )
	{
		if ( pThis->m_bTiger && pThis->m_bSHA1 )
			strURN = _T("urn:bitprint");
		else if ( pThis->m_bTiger )
			strURN = _T("urn:tree:tiger/");
		else if ( pThis->m_bSHA1 )
			strURN = _T("urn:sha1");
		else
			return E_FAIL;
	}
	
	if ( strURN.CompareNoCase( _T("urn:bitprint") ) == 0 )
	{
		if ( ! pThis->m_bSHA1 || ! pThis->m_bTiger ) return E_FAIL;
		strURN	= _T("urn:bitprint:")
				+ CSHA::HashToString( &pThis->m_pSHA1 ) + '.'
				+ CTigerNode::HashToString( &pThis->m_pTiger );
	}
	else if ( strURN.CompareNoCase( _T("urn:sha1") ) == 0 )
	{
		if ( ! pThis->m_bSHA1 ) return E_FAIL;
		strURN = CSHA::HashToString( &pThis->m_pSHA1, TRUE );
	}
	else if ( strURN.CompareNoCase( _T("urn:tree:tiger/") ) == 0 )
	{
		if ( ! pThis->m_bTiger ) return E_FAIL;
		strURN = CTigerNode::HashToString( &pThis->m_pTiger, TRUE );
	}
	else if ( strURN.CompareNoCase( _T("urn:md5") ) == 0 )
	{
		if ( ! pThis->m_bMD5 ) return E_FAIL;
		strURN = CMD5::HashToString( &pThis->m_pMD5, TRUE );
	}
	else if ( strURN.CompareNoCase( _T("urn:ed2k") ) == 0 )
	{
		if ( ! pThis->m_bED2K ) return E_FAIL;
		strURN = CED2K::HashToString( &pThis->m_pED2K, TRUE );
	}
	else
	{
		return E_FAIL;
	}
	
	strURN.SetSysString( psURN );
	
	return S_OK;
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

	if ( pThis->m_pSchema == NULL || pThis->m_pMetadata == NULL ) return S_OK;
	
	CXMLElement* pXML	= pThis->m_pSchema->Instantiate( TRUE );
	*ppXML				= (ISXMLElement*)CXMLCOM::Wrap( pXML, IID_ISXMLElement );
	
	pXML->AddElement( pThis->m_pMetadata->Clone() );
	
	return S_OK;
}

STDMETHODIMP CLibraryFile::XLibraryFile::put_Metadata(ISXMLElement FAR* pXML)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	
	if ( CXMLElement* pReal = CXMLCOM::Unwrap( pXML ) )
	{
		return pThis->SetMetadata( pReal ) ? S_OK : E_FAIL;
	}
	else
	{
		pThis->SetMetadata( NULL );
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

STDMETHODIMP CLibraryFile::XLibraryFile::Copy(BSTR sNewPath)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return E_NOTIMPL;
}

STDMETHODIMP CLibraryFile::XLibraryFile::Move(BSTR sNewPath)
{
	METHOD_PROLOGUE( CLibraryFile, LibraryFile )
	return E_NOTIMPL;
}

