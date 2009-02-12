//
// EDPartImporter.cpp
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
#include "EDPartImporter.h"
#include "EDPacket.h"
#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"
#include "FragmentedFile.h"
#include "CtrlText.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CEDPartImporter, CRazaThread)

BEGIN_MESSAGE_MAP(CEDPartImporter, CRazaThread)
	//{{AFX_MSG_MAP(CEDPartImporter)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CEDPartImporter construction

CEDPartImporter::CEDPartImporter()
{
	m_bAutoDelete	= FALSE;
	m_pTextCtrl		= NULL;
}

CEDPartImporter::~CEDPartImporter()
{
}

/////////////////////////////////////////////////////////////////////////////
// CEDPartImporter operations

void CEDPartImporter::AddFolder(LPCTSTR pszFolder)
{
	if ( m_pTextCtrl != NULL ) return;
	m_pFolders.AddTail( pszFolder );
}

void CEDPartImporter::Start(CEdit* pCtrl)
{
	ASSERT( pCtrl != NULL );
	m_pTextCtrl = pCtrl;
	CreateThread( "ED Part Importer" );
}

void CEDPartImporter::Stop()
{
	if ( m_pTextCtrl == NULL ) return;
	m_pTextCtrl = NULL;
	WaitForSingleObject( m_hThread, INFINITE );
}

BOOL CEDPartImporter::IsRunning()
{
	if ( m_hThread == NULL ) return FALSE;
	DWORD nCode = 0;
	if ( ! GetExitCodeThread( m_hThread, &nCode ) ) return FALSE;
	return nCode == STILL_ACTIVE;
}

/////////////////////////////////////////////////////////////////////////////
// CEDPartImporter run

int CEDPartImporter::Run()
{
	Message( IDS_ED2K_EPI_START );
	m_nCount = 0;

	CreateDirectory( Settings.Downloads.IncompletePath );

	for ( POSITION pos = m_pFolders.GetHeadPosition() ; pos && m_pTextCtrl != NULL ; )
	{
		ImportFolder( m_pFolders.GetNext( pos ) );
	}

	Message( IDS_ED2K_EPI_FINISHED, m_nCount );

	if ( m_nCount ) Downloads.Save();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CEDPartImporter import a folder

void CEDPartImporter::ImportFolder(LPCTSTR pszPath)
{
	WIN32_FIND_DATA pFind;
	CString strPath;
	HANDLE hSearch;

	Message( IDS_ED2K_EPI_FOLDER, pszPath );

	strPath.Format( _T("%s\\*.part.met"), pszPath );
	hSearch = FindFirstFile( strPath, &pFind );
	if ( hSearch == INVALID_HANDLE_VALUE ) return;

	do
	{
		if ( m_pTextCtrl == NULL ) break;

		if ( pFind.cFileName[0] == '.' ) continue;
		if ( pFind.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM) ) continue;

		strPath = pFind.cFileName;
		int nPos = strPath.Find( _T(".part.met") );
		if ( nPos < 1 ) continue;
		strPath = strPath.Left( nPos );

		if ( ImportFile( pszPath, strPath ) )
		{
			m_nCount++;
		}
		else
		{
			Message( IDS_ED2K_EPI_FILE_FAILED );
		}
	}
	while ( FindNextFile( hSearch, &pFind ) );

	FindClose( hSearch );
}

/////////////////////////////////////////////////////////////////////////////
// CEDPartImporter import file

BOOL CEDPartImporter::ImportFile(LPCTSTR pszPath, LPCTSTR pszFile)
{
	CString strPath;
	CFile pFile;

	Message( IDS_ED2K_EPI_FILE_START, pszFile );

	strPath.Format( _T("%s\\%s.part.met"), pszPath, pszFile );
	if ( ! pFile.Open( strPath, CFile::modeRead ) )
	{
		Message( IDS_ED2K_EPI_CANT_OPEN_PART, (LPCTSTR)strPath );
		return FALSE;
	}

	BYTE nMagic = 0;
	pFile.Read( &nMagic, 1 );
	if ( nMagic != 0xE0 ) return FALSE;

	LONG nDate = 0;
	WORD nParts = 0;
	CED2K pED2K;
	Hashes::Ed2kHash oED2K;

	pFile.Read( &nDate, 4 );
	pFile.Read( oED2K.begin(), Hashes::Ed2kHash::byteCount );
	oED2K.validate();
	pFile.Read( &nParts, 2 );

	if ( Transfers.m_pSection.Lock() )
	{
		CDownload* pDownload = Downloads.FindByED2K( oED2K );
		Transfers.m_pSection.Unlock();

		if ( pDownload != NULL )
		{
			Message( IDS_ED2K_EPI_ALREADY );
			return FALSE;
		}
	}

	if ( nParts == 0 )
	{
		pED2K.FromRoot( &oED2K[ 0 ] );
	}
	else if ( nParts > 0 )
	{
		CMD4::Digest* pHashset = new CMD4::Digest[ nParts ];
		pFile.Read( pHashset, sizeof( CMD4::Digest ) * nParts );
		BOOL bSuccess = pED2K.FromBytes( (BYTE*)pHashset, sizeof(  CMD4::Digest ) * nParts );
		delete [] pHashset;
		if ( ! bSuccess ) return FALSE;

		Hashes::Ed2kHash pCheck;
		pED2K.GetRoot( &pCheck[ 0 ] );
		pCheck.validate();
		if ( validAndUnequal( pCheck, oED2K ) ) return FALSE;
	}
	else
	{
		return FALSE;
	}

	if ( ! pED2K.IsAvailable() ) return FALSE;

	DWORD nCount = 0;
	pFile.Read( &nCount, 4 );
	if ( nCount > 2048 ) return FALSE;

	CMap< DWORD, DWORD, QWORD, QWORD > pGapStart, pGapStop;
	CArray< WORD > pGapIndex;
	BOOL bPaused = FALSE;
	CString strName;
	QWORD nSize = 0;

	while ( nCount-- )
	{
		CEDTag pTag;
		if ( ! pTag.Read( &pFile ) ) return FALSE;

		if ( pTag.Check( ED2K_FT_FILENAME, ED2K_TAG_STRING ) )
		{
			strName = pTag.m_sValue;
		}
		else if ( pTag.Check( ED2K_FT_FILESIZE, ED2K_TAG_INT ) )
		{
			nSize = pTag.m_nValue;
		}
		else if ( pTag.Check( ED2K_FT_STATUS, ED2K_TAG_INT ) )
		{
			bPaused = pTag.m_nValue != 0;
		}
		else if ( pTag.m_nType == ED2K_TAG_INT && pTag.m_sKey.GetLength() > 1 )
		{
			if ( pTag.m_sKey.GetAt( 0 ) == 0x09 )
			{
				int niPart = 0;
				_stscanf( (LPCTSTR)pTag.m_sKey + 1, _T("%i"), &niPart );
				WORD nPart = WORD( niPart );
				pGapStart.SetAt( nPart, pTag.m_nValue );
				pGapIndex.Add( nPart );
			}
			else if ( pTag.m_sKey.GetAt( 0 ) == 0x0A )
			{
				int niPart = 0;
				_stscanf( (LPCTSTR)pTag.m_sKey + 1, _T("%i"), &niPart );
				WORD nPart = WORD( niPart );
				pGapStop.SetAt( nPart, pTag.m_nValue );
			}
		}

		if ( m_pTextCtrl == NULL ) return FALSE;
	}

	if ( strName.IsEmpty() || nSize == 0 || pGapStart.IsEmpty() ) return FALSE;

	for ( int nGap = 0 ; nGap < pGapIndex.GetSize() ; nGap++ )
	{
		WORD nPart = pGapIndex.GetAt( nGap );
		QWORD nStart = 0, nStop = 0;

		if ( ! pGapStart.Lookup( nPart, nStart ) ) return FALSE;
		if ( nStart >= nSize ) return FALSE;

		if ( ! pGapStop.Lookup( nPart, nStop ) ) return FALSE;
		if ( nStop > nSize || nStop <= nStart ) return FALSE;
	}

	Message( IDS_ED2K_EPI_DETECTED, strName, Settings.SmartVolume( nSize ) );

	if ( ! Downloads.IsSpaceAvailable( nSize, Downloads.dlPathIncomplete ) )
	{
		Message( IDS_ED2K_EPI_DISK_SPACE );
		return FALSE;
	}

	CFileStatus pStatus;
	CFile pData;

	strPath.Format( _T("%s\\%s.part"), pszPath, pszFile );
	if ( ! pData.Open( strPath, CFile::modeRead ) ) return FALSE;
	pData.GetStatus( pStatus );
	pData.Close();
	if ( nDate > mktime( pStatus.m_mtime.GetLocalTm( NULL ) ) )
	{
		Message( IDS_ED2K_EPI_FILE_OLD );
		return FALSE;
	}

	CString strTarget;
	strTarget.Format( _T("%s\\ed2k_%s"),
		(LPCTSTR)Settings.Downloads.IncompletePath,
		(LPCTSTR)oED2K.toString() );

	Message( IDS_ED2K_EPI_COPY_START, (LPCTSTR)strPath, (LPCTSTR)strTarget );

	if ( m_pTextCtrl == NULL ) return FALSE;
	if ( ! CopyFile( strPath, strTarget ) ) return FALSE;
	if ( m_pTextCtrl == NULL ) return FALSE;

	Message( IDS_ED2K_EPI_COPY_FINISHED );

	Transfers.m_pSection.Lock();

	CDownload* pDownload = Downloads.Add();
	
	pDownload->m_oED2K			= oED2K;
	pDownload->m_bED2KTrusted = true; // .part use trusted hashes

	pDownload->m_nSize			= nSize;
	pDownload->m_sName			= strName;
	pDownload->m_sPath			= strTarget;
	
	{
		Fragments::List oNewList( nSize );
		pDownload->m_pFile->m_oFList.swap( oNewList );
	}

	for ( int nGap = 0 ; nGap < pGapIndex.GetSize() ; nGap++ )
	{
		WORD nPart = pGapIndex.GetAt( nGap );
		QWORD nStart = 0, nStop = 0;

		pGapStart.Lookup( nPart, nStart );
		pGapStop.Lookup( nPart, nStop );
		
		pDownload->m_pFile->m_oFList.insert( Fragments::Fragment( nStart, nStop ) );
	}

	if ( pED2K.IsAvailable() )
	{
		BYTE* pHashset = NULL;
		DWORD nHashset = 0;
		pED2K.ToBytes( &pHashset, &nHashset );
		pDownload->SetHashset( pHashset, nHashset );
		delete [] pHashset;
	}

	if ( bPaused ) pDownload->Pause();

	pDownload->Save();

	Transfers.m_pSection.Unlock();

	Message( IDS_ED2K_EPI_FILE_CREATED,
		Settings.SmartVolume( pDownload->m_pFile->m_oFList.length_sum() ) );
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CEDPartImporter copy file

BOOL CEDPartImporter::CopyFile(LPCTSTR pszSource, LPCTSTR pszTarget)
{
	return ::CopyFile( pszSource, pszTarget, TRUE );
}

/////////////////////////////////////////////////////////////////////////////
// CEDPartImporter message

void CEDPartImporter::Message(UINT nMessageID, ...)
{
	CEdit* pCtrl = m_pTextCtrl;
	if ( pCtrl == NULL ) return;

	const DWORD nBufferLength = 2048;
	auto_array< TCHAR > szBuffer( new TCHAR[ nBufferLength ] );
	ZeroMemory( szBuffer.get(), nBufferLength * sizeof( TCHAR ) );
	CString strFormat;
	va_list pArgs;

	LoadString( strFormat, nMessageID );
	va_start( pArgs, nMessageID );
	_vsntprintf( szBuffer.get(), nBufferLength - 8, strFormat, pArgs );
	_tcscat( szBuffer.get(), _T("\r\n") );
	va_end( pArgs );

	int nLen = pCtrl->GetWindowTextLength();
	pCtrl->SetSel( nLen, nLen );
	pCtrl->ReplaceSel( szBuffer.get() );
	nLen += static_cast< int >( _tcslen( szBuffer.get() ) );
	pCtrl->SetSel( nLen, nLen );
}
