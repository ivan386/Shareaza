//
// EDPartImporter.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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
#include "Download.h"
#include "Downloads.h"
#include "DownloadGroups.h"
#include "DownloadTask.h"
#include "FragmentedFile.h"
#include "CtrlText.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEDPartImporter construction

CEDPartImporter::CEDPartImporter()
	: m_pTextCtrl	( NULL )
{
}

CEDPartImporter::~CEDPartImporter()
{
}

/////////////////////////////////////////////////////////////////////////////
// CEDPartImporter operations

void CEDPartImporter::AddFolder(LPCTSTR pszFolder)
{
	m_pFolders.AddTail( pszFolder );
}

void CEDPartImporter::Start(CEdit* pCtrl)
{
	ASSERT( pCtrl != NULL );

	if ( IsThreadAlive() )
		return;

	m_pTextCtrl = pCtrl;

	BeginThread( "ED Part Importer" );
}

void CEDPartImporter::Stop()
{
	m_pTextCtrl = NULL;

	CloseThread();
}

/////////////////////////////////////////////////////////////////////////////
// CEDPartImporter run

void CEDPartImporter::OnRun()
{
	Message( IDS_ED2K_EPI_START );
	m_nCount = 0;

	CreateDirectory( Settings.Downloads.IncompletePath );

	for ( POSITION pos = m_pFolders.GetHeadPosition() ; pos && IsThreadEnabled(); )
	{
		ImportFolder( m_pFolders.GetNext( pos ) );
	}

	Message( IDS_ED2K_EPI_FINISHED, m_nCount );

	if ( m_nCount ) Downloads.Save();
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
		if ( ! IsThreadEnabled() )
			break;

		if ( pFind.cFileName[0] == '.' )
			continue;
		if ( ( pFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
			continue;

		strPath = pFind.cFileName;
		int nPos = strPath.Find( _T(".part.met") );
		if ( nPos < 1 )
			continue;
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
	Message( IDS_ED2K_EPI_FILE_START, pszFile );

	CString strPath;
	strPath.Format( _T("%s\\%s.part.met"), pszPath, pszFile );

	CFile pFile;
	if ( ! pFile.Open( strPath, CFile::modeRead ) )
	{
		Message( IDS_ED2K_EPI_CANT_OPEN_PART, (LPCTSTR)strPath );
		return FALSE;
	}

	BYTE nMagic;
	if ( pFile.Read( &nMagic, 1 ) != 1 )
		return FALSE;

	if ( nMagic != 0xE0 )
		return FALSE;

	LONG nDate;
	if ( pFile.Read( &nDate, 4 ) != 4 )
		return FALSE;

	Hashes::Ed2kHash oED2K;
	if ( pFile.Read( &*oED2K.begin(), oED2K.byteCount )
		!= oED2K.byteCount )
	{
		return FALSE;
	}

	oED2K.validate();

	WORD nParts;
	if ( pFile.Read( &nParts, 2 ) != 2 )
		return FALSE;

	{
		CQuickLock oTransfersLock( Transfers.m_pSection );

		if ( Downloads.FindByED2K( oED2K ) )
		{
			Message( IDS_ED2K_EPI_ALREADY );
			return FALSE;
		}
	}

	CED2K pED2K;
	if ( nParts == 0 )
	{
		pED2K.FromRoot( &oED2K[ 0 ] );
	}
	else if ( nParts > 0 )
	{
		UINT len = sizeof( CMD4::Digest ) * nParts;
		auto_array< CMD4::Digest > pHashset( new CMD4::Digest[ nParts ] );
		if ( pFile.Read( pHashset.get(), len ) != len )
			return FALSE;

		BOOL bSuccess = pED2K.FromBytes( (BYTE*)pHashset.get(), len );
		if ( ! bSuccess )
			return FALSE;

		Hashes::Ed2kHash pCheck;
		pED2K.GetRoot( &pCheck[ 0 ] );
		pCheck.validate();
		if ( validAndUnequal( pCheck, oED2K ) )
			return FALSE;
	}

	if ( ! pED2K.IsAvailable() )
		return FALSE;

	DWORD nCount;
	if ( pFile.Read( &nCount, 4 ) != 4 )
		return FALSE;
	if ( nCount > 2048 )
		return FALSE;

	CMap< int, int, QWORD, QWORD > pGapStart, pGapStop;
	CArray< int > pGapIndex;
	BOOL bPaused = FALSE;
	CString strName, strPartName;
	QWORD nSize = 0;

	while ( nCount-- )
	{
		if ( ! IsThreadEnabled() )
			return FALSE;

		CEDTag pTag;
		if ( ! pTag.Read( &pFile ) )
			return FALSE;

		if ( pTag.Check( ED2K_FT_FILENAME, ED2K_TAG_STRING ) )
		{
			strName = pTag.m_sValue;
		}
		else if ( pTag.Check( ED2K_FT_PARTFILENAME, ED2K_TAG_STRING ) )
		{
			strPartName = pTag.m_sValue;
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
			if ( pTag.m_sKey.GetAt( 0 ) == 0x09 )		// Start of gap
			{
				int nPart = 0;
				_stscanf( (LPCTSTR)pTag.m_sKey + 1, _T("%i"), &nPart );
				pGapStart.SetAt( nPart, pTag.m_nValue );
				pGapIndex.Add( nPart );
			}
			else if ( pTag.m_sKey.GetAt( 0 ) == 0x0A )	// End of gap
			{
				int nPart = 0;
				_stscanf( (LPCTSTR)pTag.m_sKey + 1, _T("%i"), &nPart );
				pGapStop.SetAt( nPart, pTag.m_nValue );
			}
		}
	}

	if ( strName.IsEmpty() || nSize == SIZE_UNKNOWN || nSize == 0 ||
		pGapStart.IsEmpty() )
		return FALSE;

	// Test gap list
	Fragments::List oGaps( nSize );
	for ( int nGap = 0 ; nGap < pGapIndex.GetSize() ; nGap++ )
	{
		if ( ! IsThreadEnabled() )
			return FALSE;

		int nPart = pGapIndex.GetAt( nGap );
		QWORD nStart, nStop;
		if ( ! pGapStart.Lookup( nPart, nStart ) )
			return FALSE;

		if ( nStart >= nSize )
			return FALSE;

		if ( ! pGapStop.Lookup( nPart, nStop ) )
			return FALSE;

		if ( nStop > nSize || nStop <= nStart )
			return FALSE;

		oGaps.insert( Fragments::Fragment( nStart, nStop ) );
	}

	Message( IDS_ED2K_EPI_DETECTED, (LPCTSTR)strName, (LPCTSTR)Settings.SmartVolume( nSize ) );

	if ( ! Downloads.IsSpaceAvailable( nSize, Downloads.dlPathIncomplete ) )
	{
		Message( IDS_ED2K_EPI_DISK_SPACE );
		return FALSE;
	}

	if ( strPartName.IsEmpty() )
		strPath.Format( _T("%s\\%s.part"), pszPath, pszFile );
	else
		strPath.Format( _T("%s\\%s"), pszPath, (LPCTSTR)strPartName );

	CFile pData;
	if ( ! pData.Open( strPath, CFile::modeRead ) )
		return FALSE;

	CFileStatus pStatus;
	if ( ! pData.GetStatus( pStatus ) )
		return FALSE;

	pData.Close();

	struct tm ptmTemp = {};
	if ( nDate > mktime( pStatus.m_mtime.GetLocalTm( &ptmTemp ) ) )
	{
		Message( IDS_ED2K_EPI_FILE_OLD );
		return FALSE;
	}

	Message( IDS_ED2K_EPI_COPY_FINISHED );

	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	CDownload* pDownload = Downloads.Add();
	if ( ! pDownload )
		return FALSE;

	pDownload->m_oED2K			= oED2K;
	pDownload->m_bED2KTrusted	= true; // .part use trusted hashes
	pDownload->m_nSize			= nSize;
	pDownload->m_sName			= strName;
	pDownload->Pause();

	BYTE* pHashset = NULL;
	DWORD nHashset = 0;
	if ( pED2K.ToBytes( &pHashset, &nHashset ) )
	{
		pDownload->SetHashset( pHashset, nHashset );
		GlobalFree( pHashset );
	}

	pDownload->Save();

	DownloadGroups.Link( pDownload );

	Message( IDS_ED2K_EPI_COPY_START, (LPCTSTR)strPath, (LPCTSTR)pDownload->m_sPath );

	CList < CString > oFiles;
	oFiles.AddHead( strPath );

	pDownload->MergeFile( &oFiles, FALSE, &oGaps );

	if ( ! bPaused )
		pDownload->Resume();

	Message( IDS_ED2K_EPI_FILE_CREATED, (LPCTSTR)Settings.SmartVolume( pDownload->GetVolumeRemaining() ) );

	if ( Settings.Downloads.ShowMonitorURLs )
		pDownload->ShowMonitor();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CEDPartImporter message

void CEDPartImporter::Message(UINT nMessageID, ...)
{
	CEdit* pCtrl = m_pTextCtrl;
	if ( pCtrl == NULL ) return;

	CString sBuffer, strFormat;
	LoadString( strFormat, nMessageID );

	va_list pArgs;
	va_start( pArgs, nMessageID );
	sBuffer.FormatMessageV( strFormat, &pArgs );
	va_end( pArgs );

	sBuffer += _T( "\r\n" );

	int nLen = pCtrl->GetWindowTextLength();
	pCtrl->SetSel( nLen, nLen );
	pCtrl->ReplaceSel( sBuffer );
	nLen += sBuffer.GetLength();
	pCtrl->SetSel( nLen, nLen );
}
