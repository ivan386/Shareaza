//
// FragmentedFile.cpp
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
#include "FragmentedFile.h"
#include "TransferFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#include <winioctl.h>


//////////////////////////////////////////////////////////////////////
// CFragmentedFile construction

CFragmentedFile::CFragmentedFile()
{
	m_pFile = NULL;
	m_nShift = m_nUnflushed = m_nTotal = 0;
}

CFragmentedFile::~CFragmentedFile()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile create

BOOL CFragmentedFile::Create(LPCTSTR pszFile, QWORD nLength)
{
	if ( m_pFile || m_nTotal || ! ( m_nTotal = nLength ) ) return FALSE;
	if ( ! ( m_pFile = TransferFiles.Open( pszFile, TRUE, TRUE ) ) ) return FALSE;
	m_oFree.Add( 0, nLength );
	if ( Settings.Downloads.SparseThreshold > 0 && theApp.m_bNT &&
		nLength >= Settings.Downloads.SparseThreshold * 1024 )
	{
		DWORD dwOut = 0;
		HANDLE hFile = m_pFile->GetHandle( TRUE );
		if ( ! DeviceIoControl( hFile, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &dwOut, NULL ) )
		{
			theApp.Message( MSG_ERROR, _T("Unable to set sparse file: \"%s\", Win32 error %x."),
				pszFile, GetLastError() );
		}
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile open

BOOL CFragmentedFile::Open(LPCTSTR pszFile)
{
	if ( m_pFile || !m_nTotal ) return FALSE;
	m_pFile = TransferFiles.Open( pszFile, TRUE, FALSE );
	return m_pFile != NULL;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile flush

BOOL CFragmentedFile::Flush()
{
	if ( !m_nUnflushed || !m_pFile || ! m_pFile->IsOpen() ) return FALSE;
	FlushFileBuffers( m_pFile->GetHandle() );
	m_nUnflushed = 0;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile close

void CFragmentedFile::Close()
{
	if ( m_pFile )
	{
		m_pFile->Release( TRUE );
		m_pFile = NULL;
		m_nUnflushed = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile clear

void CFragmentedFile::Clear()
{
	Close();
	m_oFree.Delete();
	m_nTotal = m_nUnflushed = 0;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile make complete

BOOL CFragmentedFile::MakeComplete()
{
	if ( !m_nTotal || m_oFree.IsEmpty() ) return FALSE;
	m_oFree.Delete();
	if ( m_pFile != NULL )
	{
		HANDLE hFile = m_pFile->GetHandle( TRUE );
		if ( hFile != INVALID_HANDLE_VALUE )
		{
			DWORD nSizeHigh	= (DWORD)( m_nTotal >> 32 );
			DWORD nSizeLow	= (DWORD)m_nTotal;
			SetFilePointer( hFile, nSizeLow, (PLONG)&nSizeHigh, FILE_BEGIN );
			SetEndOfFile( hFile );
		}
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile serialize

void CFragmentedFile::Serialize(CArchive& ar, int nVersion)
{
	if ( nVersion >= 32 )
	{
		if ( ar.IsStoring() ) ar << m_nTotal; else ar >> m_nTotal;
		m_oFree.Serialize( ar, nVersion );
		return;
	}
	QWORD nRemaining;
	ASSERT( ar.IsLoading() );
	ASSERT( m_nTotal == 0 );
	if ( nVersion >= 29 )
	{
		ar >> m_nTotal;
		ar >> nRemaining;
	}
	else
	{
		DWORD nInt32;
		ar >> nInt32; m_nTotal = nInt32;
		ar >> nInt32; nRemaining = nInt32;
	}
	m_oFree.Serialize( ar, nVersion, FALSE );
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile write some data to a range

BOOL CFragmentedFile::WriteRange(QWORD nOffset, LPCVOID pData, QWORD nLength)
{
	if ( m_pFile == NULL || m_oFree.IsEmpty() ) return FALSE;
	if ( nLength == 0 ) return TRUE;
	QWORD nResult, nProcessed = 0;
	CFileFragmentList m_oToWrite;
	m_oToWrite.Extract( m_oFree, nOffset, nOffset + nLength );
	if ( CFileFragment* pFragment = m_oToWrite.GetFirst() ) do
	{
		LPBYTE pSource = (LPBYTE)pData + pFragment->Offset() - nOffset;
		if ( ! m_pFile->Write( pFragment->Offset(), pSource, pFragment->Length(), &nResult ) )
		{
			m_oFree.Merge( m_oToWrite );
			return FALSE;
		}
		nProcessed += pFragment->Length();
	}
	while ( pFragment = pFragment->GetNext() );
	m_nUnflushed += nProcessed;
	return nProcessed > 0;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile read some data from a range

BOOL CFragmentedFile::ReadRange(QWORD nOffset, LPVOID pData, QWORD nLength)
{
	if ( !m_pFile || !nLength || m_oFree.OverlapsRange( nOffset, nOffset + nLength ) ) return FALSE;
	QWORD nRead = 0;
	m_pFile->Read( nOffset, pData, nLength, &nRead );
	return nRead == nLength;
}

BOOL CFragmentedFile::ReadRangeUnlimited(QWORD nOffset, LPVOID pData, QWORD nLength)
{
	if ( !m_pFile || !nLength ) return FALSE;
	QWORD nRead = 0;
	m_pFile->Read( nOffset, pData, nLength, &nRead );
	return nRead == nLength;
}

//////////////////////////////////////////////////////////////////////
// CFragmentedFile invalidate a range

QWORD CFragmentedFile::InvalidateRange(const QWORD nOffset, const QWORD nLength)
{
	return m_oFree.Add( nOffset, nOffset + nLength );
}

void CFragmentedFile::InvalidateRange(const CFileFragmentList& Corrupted)
{
	m_oFree.Merge( Corrupted );
}