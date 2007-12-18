//
// DownloadBase.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

CDownloadBase::CDownloadBase() :
	m_sSearchKeyword	()
,	m_nCookie			( 1 )
,	m_nSize				( SIZE_UNKNOWN )
,	m_pTask				( NULL )
{
}

CDownloadBase::~CDownloadBase()
{
}

//////////////////////////////////////////////////////////////////////

BOOL CDownloadBase::SetNewTask(CDownloadTask* pTask)
{
	if ( IsTasking() || pTask == NULL ) return FALSE;

	m_pTask = pTask;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadBase modified

void CDownloadBase::SetModified()
{
	m_nCookie ++;
}

//////////////////////////////////////////////////////////////////////
// CDownloadBase disk file name (the <hash>.partial file in the incomplete directory)

void CDownloadBase::GenerateDiskName(bool bTorrent)
{
	// Seeding torrents already have a disk name, we need only safe name
	if ( bTorrent )
	{
		m_sSafeName += _T("btih_");
		m_sSafeName += m_oBTH.toString();
		return;
	}

	// Exit if we've already named the temp file
	if ( m_sDiskName.GetLength() > 0 ) return;

	// Get a meaningful (but safe) name. Used for previews, etc. Make sure we get extension if name is long.
	m_sSafeName = CDownloadTask::SafeFilename( m_sDisplayName.Right( 64 ) );

	// Start disk file name with hash
	if ( m_oSHA1 ) 
	{
		m_sDiskName += _T("sha1_");
		m_sDiskName += m_oSHA1.toString();
	}
	else if ( m_oTiger ) 
	{
		m_sDiskName += _T("ttr_");
		m_sDiskName += m_oTiger.toString();
	}
	else if ( m_oED2K )
	{
		m_sDiskName += _T("ed2k_");
		m_sDiskName += m_oED2K.toString();
	}
	else if ( m_oBTH ) 
	{
		m_sDiskName += _T("btih_");
		m_sDiskName += m_oBTH.toString();
	}
	else if ( m_oMD5 )
	{
		m_sDiskName += _T("md5_");
		m_sDiskName += m_oMD5.toString();
	}
	else if ( m_sDisplayName.GetLength() > 0 )
	{
		m_sDiskName += _T("name_");
		m_sDiskName += CDownloadTask::SafeFilename( m_sDisplayName.Left( 32 ) );
	}
	else
	{
		m_sDiskName.Format( _T("rand_%2i%2i%2i%2i"), rand() % 100, rand() % 100, rand() % 100, rand() % 100 );
	}

	// Add a .partial extension
	m_sDiskName += _T(".partial");
	// Create download directory if it doesn't exist
	CreateDirectory( Settings.Downloads.IncompletePath );
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
		ar << m_sSearchKeyword;
		ar << m_nSize;
        SerializeOut( ar, m_oSHA1 );
        SerializeOut( ar, m_oTiger );
        SerializeOut( ar, m_oMD5 );
        SerializeOut( ar, m_oED2K );
		SerializeOut( ar, m_oBTH );
	}
	else
	{
		ar >> m_sDisplayName;

		if ( nVersion >= 29 )
		{
			if ( nVersion >= 33 )
			{
				ar >> m_sSearchKeyword;
			}
			ar >> m_nSize;
		}
		else
		{
			DWORD nSize;
			ar >> nSize;
			m_nSize = nSize;
		}
        SerializeIn( ar, m_oSHA1, nVersion );
        SerializeIn( ar, m_oTiger, nVersion );
        if ( nVersion >= 22 )
			SerializeIn( ar, m_oMD5, nVersion );
        if ( nVersion >= 13 )
			SerializeIn( ar, m_oED2K, nVersion );
		if ( nVersion >= 37 )
			SerializeIn( ar, m_oBTH, nVersion );
	}
}
