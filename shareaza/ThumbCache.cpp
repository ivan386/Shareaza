//
// ThumbCache.cpp
//
// Copyright © Shareaza Development Team, 2002-2009.
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
#include "SQLite.h"
#include "Settings.h"
#include "ThumbCache.h"
#include "ImageServices.h"
#include "ImageFile.h"
#include "Library.h"
#include "SharedFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CThumbCache init

void CThumbCache::InitDatabase()
{
	SQLite::CDatabase db( Settings.General.UserPath + _T("\\Data\\Shareaza.db3") );
	if ( ! db )
	{
		TRACE( _T("CThumbCache::InitDatabase : Database error: %s\n"), db.GetLastErrorMessage() );
		return;
	}

	// Recreate table
	if ( !db.Exec( L"CREATE TABLE Files ("
			 L"Filename TEXT UNIQUE NOT NULL PRIMARY KEY, "
			 L"FileSize INTEGER NOT NULL, "
			 L"LastWriteTime INTEGER NOT NULL, "
			 L"Image BLOB NOT NULL, " // as JPEG
			 L"Flags INTEGER DEFAULT 0 NULL, "
			 L"SHA1 TEXT NULL, TTH TEXT NULL, ED2K TEXT NULL, MD5 TEXT NULL); "
			 L"CREATE INDEX IDX_SHA1 ON Files(SHA1 ASC); "
			 L"CREATE INDEX IDX_TTH ON Files(TTH ASC); "
			 L"CREATE INDEX IDX_ED2K ON Files(ED2K ASC); "
			 L"CREATE INDEX IDX_MD5 ON Files(MD5 ASC);" ) )
		db.Exec( L"VACUUM;");
}

//////////////////////////////////////////////////////////////////////
// CThumbCache load

BOOL CThumbCache::Load(LPCTSTR pszPath, CImageFile* pImage)
{
	ASSERT( pszPath );
	ASSERT( pImage );

	// Load file info from disk
	WIN32_FIND_DATA fd = { 0 };
	if ( ! GetFileAttributesEx( pszPath, GetFileExInfoStandard, &fd ) )
	{
		// Deleted (or Ghost) file
		TRACE( _T("CThumbCache::Load : Can't load info for %s\n"), pszPath );
		return FALSE;
	}

	// Load file info from database
	SQLite::CDatabase db( Settings.General.UserPath + _T("\\Data\\Shareaza.db3") );
	if ( ! db )
	{
		TRACE( _T("CThumbCache::InitDatabase : Database error: %s\n"), db.GetLastErrorMessage() );
		return FALSE;
	}

	CString sPath( pszPath );
	sPath.MakeLower();

	SQLite::CStatement st( db,
		_T("SELECT FileSize, LastWriteTime, Image FROM Files WHERE Filename == ?;") );
	if ( ! st.Bind( 1, sPath ) ||
		 ! st.Step() ||
		 ! ( st.GetCount() == 0 || st.GetCount() == 3 ) )
	{
		TRACE( _T("CThumbCache::Load : Database error: %s\n"), db.GetLastErrorMessage() );
		return FALSE;
	}
	if ( st.GetCount() == 0 )
	{
		TRACE( _T("CThumbCache::Load : No thumbnail for %s\n"), pszPath );
		return FALSE;
	}

	QWORD nFileSize = (QWORD)st.GetInt64( _T("FileSize") );
	QWORD nLastWriteTime = (QWORD)st.GetInt64( _T("LastWriteTime") );
	int data_len;
	LPCVOID data = st.GetBlob( _T("Image"), &data_len );
	if ( ! data )
	{
		TRACE( _T("CThumbCache::Load : Database error: %s\n"), db.GetLastErrorMessage() );
		return FALSE;
	}

	// Compare it
	BOOL loaded = FALSE;
	if ( nFileSize == MAKEQWORD( fd.nFileSizeLow, fd.nFileSizeHigh ) &&
		nLastWriteTime == MAKEQWORD( fd.ftLastWriteTime.dwLowDateTime, fd.ftLastWriteTime.dwHighDateTime ) )
	{
		// Load image
		loaded = pImage->LoadFromMemory( _T(".jpg"), data, data_len );
	}

	if ( ! loaded )
	{
		// Remove outdated or bad thumbnail
		Delete( pszPath );
	}

	return loaded;
}

void CThumbCache::Delete(LPCTSTR pszPath)
{
	SQLite::CDatabase db( Settings.General.UserPath + _T("\\Data\\Shareaza.db3") );
	if ( ! db )
	{
		TRACE( _T("CThumbCache::InitDatabase : Database error: %s\n"), db.GetLastErrorMessage() );
		return;
	}

	CString sPath( pszPath );
	sPath.MakeLower();

	SQLite::CStatement st( db, _T("DELETE FROM Files WHERE Filename == ?;") );
	if ( ! st.Bind( 1, sPath ) )
	{
		TRACE( _T("CThumbCache::Load : Database error: %s\n"), db.GetLastErrorMessage() );
	}
	else
	{
		st.Step();
	}
}

//////////////////////////////////////////////////////////////////////
// CThumbCache store

BOOL CThumbCache::Store(LPCTSTR pszPath, CImageFile* pImage)
{
	ASSERT( pszPath );
	ASSERT( pImage );
	ASSERT( pImage->m_nWidth >= 0 && pImage->m_nHeight >= 0 );

	// Load file info from disk
	WIN32_FIND_DATA fd = { 0 };
	if ( ! GetFileAttributesEx( pszPath, GetFileExInfoStandard, &fd ) )
	{
		TRACE( _T("CThumbCache::Store : Can't load info for %s\n"), pszPath );
		return FALSE;
	}

	SQLite::CDatabase db( Settings.General.UserPath + _T("\\Data\\Shareaza.db3") );
	if ( ! db )
	{
		TRACE( _T("CThumbCache::InitDatabase : Database error: %s\n"), db.GetLastErrorMessage() );
		return FALSE;
	}

	CString sPath( pszPath );
	sPath.MakeLower();

	// Save to memory as JPEG image
	BYTE* buf = NULL;
	DWORD data_len = 0;
	if ( ! pImage->SaveToMemory( _T(".jpg"), 75, &buf, &data_len ) )
	{
		TRACE( _T("CThumbCache::Store : Can't save thumbnail to JPEG for %s\n"), pszPath );
		return FALSE;
	}
	auto_array< BYTE > data( buf );	

	// Remove old image
	SQLite::CStatement st1( db, _T("DELETE FROM Files WHERE Filename == ?;") );
	if ( ! st1.Bind( 1, sPath ) )
	{
		TRACE( _T("CThumbCache::Store : Database error: %s\n"), db.GetLastErrorMessage() );
		return FALSE;
	}
	st1.Step();

	// Store new one
	SQLite::CStatement st2( db, _T("INSERT INTO Files ")
		_T("( Filename, FileSize, LastWriteTime, Image ) VALUES ( ?, ?, ?, ? );") );
	if ( ! st2.Bind( 1, sPath ) ||
		 ! st2.Bind( 2, (__int64)MAKEQWORD( fd.nFileSizeLow, fd.nFileSizeHigh ) ) ||
		 ! st2.Bind( 3, (__int64)MAKEQWORD( fd.ftLastWriteTime.dwLowDateTime, fd.ftLastWriteTime.dwHighDateTime ) ) ||
		 ! st2.Bind( 4, data.get(), data_len ) ||
		 ! st2.Step() )
	{
		TRACE( _T("CThumbCache::Store : Database error: %s\n"), db.GetLastErrorMessage() );
		return FALSE;
	}

	TRACE( _T("CThumbCache::Store : Thumbnail saved for %s\n"), pszPath );

	CSingleLock oLock( &Library.m_pSection, FALSE );
	if ( oLock.Lock( 250 ) )
	{
		if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( pszPath ) )
		{
			ASSERT( pFile->GetPath().MakeLower() == sPath );
			pFile->m_bCachedPreview = TRUE;
			Library.Update();
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CThumbCache cache

BOOL CThumbCache::Cache(LPCTSTR pszPath, CImageFile* pImage, BOOL bLoadFromFile)
{
	CImageFile pFile;
	if ( ! pImage )
		pImage = &pFile;

	// Load from cache
	if ( CThumbCache::Load( pszPath, pImage ) )
		return TRUE;

	if ( ! bLoadFromFile )
		return FALSE;

	// Load from file
	if ( ! pImage->LoadFromFile( pszPath, FALSE, TRUE ) || ! pImage->EnsureRGB() ||
		pImage->m_nHeight <= 0 || pImage->m_nWidth <= 0 )
		return FALSE;

	// Resample to desired size
	if ( ! pImage->FitTo( THUMB_STORE_SIZE, THUMB_STORE_SIZE ) )
		return FALSE;

	// Save to cache
	CThumbCache::Store( pszPath, pImage );

	return TRUE;
}
