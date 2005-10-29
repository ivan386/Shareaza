//
// DownloadBase.cpp
//
//	Date:			"$Date: 2005/10/29 21:41:59 $"
//	Revision:		"$Revision: 1.11 $"
//  Last change by:	"$Author: mogthecat $"
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
#include "Downloads.h"
#include "DownloadBase.h"
#include "DownloadTask.h"

#include "SHA.h"
#include "ED2K.h"
#include "TigerTree.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadBase construction

CDownloadBase::CDownloadBase()
{
	m_nCookie		= 1;
	m_nSize			= SIZE_UNKNOWN;
	m_bSHA1			= FALSE;
	m_bSHA1Trusted	= FALSE;
	m_bTiger		= FALSE;
	m_bTigerTrusted	= FALSE;
	m_bMD5			= FALSE;
	m_bMD5Trusted	= FALSE;
	m_bED2K			= FALSE;
	m_bED2KTrusted	= FALSE;
	m_bBTH			= FALSE;
	m_bBTHTrusted	= FALSE;
	m_pTask			= NULL;
}

CDownloadBase::~CDownloadBase()
{
}

//////////////////////////////////////////////////////////////////////
// CDownloadBase modified

void CDownloadBase::SetModified()
{
	m_nCookie ++;
}

//////////////////////////////////////////////////////////////////////
// CDownloadBase disk file name (the <hash>.partial file in the incomplete directory)

void CDownloadBase::GenerateDiskName()
{
	// Exit if we've already named the temp file
	if ( m_sDiskName.GetLength() > 0 ) return;

	// Get a meaningful (but safe) name. Used for previews, etc. Make sure we get extention if name is long.
	m_sSafeName = CDownloadTask::SafeFilename( m_sDisplayName.Right( 64 ) );

	// Start disk file name with hash
	if ( m_bSHA1 ) 
	{
		m_sDiskName += _T("sha1_");
		m_sDiskName += CSHA::HashToString( &m_pSHA1 );
	}
	else if ( m_bTiger ) 
	{
		m_sDiskName += _T("ttr_");
		m_sDiskName += CTigerNode::HashToString( &m_pTiger );
	}
	else if ( m_bED2K )
	{
		m_sDiskName += _T("ed2k_");
		m_sDiskName += CED2K::HashToString( &m_pED2K );
	}
	else if ( m_bBTH ) 
	{
		m_sDiskName += _T("btih_");
		m_sDiskName += CSHA::HashToString( &m_pBTH );
	}
	else if ( m_sDisplayName.GetLength() > 0 )
	{
		m_sDiskName += _T("name_");
		m_sDiskName += CDownloadTask::SafeFilename( m_sDisplayName.Left( 32 ) );
	}
	else
	{
		srand( (unsigned)GetTickCount() );
		m_sDiskName.Format( _T("rand_%2i%2i%2i%2i"), rand() % 100, rand() % 100, rand() % 100, rand() % 100 );
	}

	// Add a .partial extention
	m_sDiskName += _T(".partial");
	// Create download directory if it doesn't exist
	CreateDirectory( Settings.Downloads.IncompletePath, NULL );
	// Add the path
	m_sDiskName = Settings.Downloads.IncompletePath + _T("\\") + m_sDiskName;

	ASSERT( m_sDiskName.GetLength() < MAX_PATH - 1 );
}

//////////////////////////////////////////////////////////////////////
// CDownloadBase serialize

void CDownloadBase::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar << m_sDisplayName;
		ar << m_nSize;

		ar << m_bSHA1;
		if ( m_bSHA1 ) ar.Write( &m_pSHA1, sizeof(SHA1) );
		ar << m_bSHA1Trusted;

		ar << m_bTiger;
		if ( m_bTiger ) ar.Write( &m_pTiger, sizeof(TIGEROOT) );
		ar << m_bTigerTrusted;

		ar << m_bMD5;
		if ( m_bMD5 ) ar.Write( &m_pMD5, sizeof(MD5) );
		ar << m_bMD5Trusted;

		ar << m_bED2K;
		if ( m_bED2K ) ar.Write( &m_pED2K, sizeof(MD4) );
		ar << m_bED2KTrusted;
	}
	else
	{
		ar >> m_sDisplayName;
		m_sSafeName = CDownloadTask::SafeFilename( m_sDisplayName.Right( 64 ) );

		if ( nVersion >= 29 )
		{
			ar >> m_nSize;
		}
		else
		{
			DWORD nSize;
			ar >> nSize;
			m_nSize = nSize;
		}

		ar >> m_bSHA1;
		if ( m_bSHA1 ) ar.Read( &m_pSHA1, sizeof(SHA1) );
		if ( nVersion >= 31 ) ar >> m_bSHA1Trusted;
		else m_bSHA1Trusted = m_bSHA1;
		if ( ( m_bSHA1 ) && ( CSHA::IsNull( &m_pSHA1 ) ) ) m_bSHA1Trusted = m_bSHA1 = FALSE;

		ar >> m_bTiger;
		if ( m_bTiger ) ar.Read( &m_pTiger, sizeof(TIGEROOT) );
		if ( nVersion >= 31 ) ar >> m_bTigerTrusted;
		else m_bTigerTrusted = m_bTiger;
		if ( ( m_bTiger ) && ( CTigerNode::IsNull( &m_pTiger ) ) ) m_bTigerTrusted = m_bTiger = FALSE;

		if ( nVersion >= 22 ) ar >> m_bMD5;
		if ( m_bMD5 ) ar.Read( &m_pMD5, sizeof(MD5) );
		if ( nVersion >= 31 ) ar >> m_bMD5Trusted;
		else m_bMD5Trusted = m_bMD5;

		if ( nVersion >= 13 ) ar >> m_bED2K;
		if ( m_bED2K ) ar.Read( &m_pED2K, sizeof(MD4) );
		if ( nVersion >= 31 ) ar >> m_bED2KTrusted;
		else m_bED2KTrusted = m_bED2K;

	}
}
