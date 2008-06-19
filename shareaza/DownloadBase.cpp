//
// DownloadBase.cpp
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
	m_bSHA1Trusted		( false )
,	m_bTigerTrusted		( false )
,	m_bED2KTrusted		( false )
,	m_bBTHTrusted		( false )
,	m_bMD5Trusted		( false )
,	m_nCookie			( 1 )
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
	if ( m_sPath.GetLength() > 0 ) return;

	// Get a meaningful (but safe) name. Used for previews, etc. Make sure we get extension if name is long.
	m_sSafeName = CDownloadTask::SafeFilename( m_sName.Right( 64 ) );

	// Start disk file name with hash
	if ( m_oSHA1 ) 
	{
		m_sPath += _T("sha1_");
		m_sPath += m_oSHA1.toString();
	}
	else if ( m_oTiger ) 
	{
		m_sPath += _T("ttr_");
		m_sPath += m_oTiger.toString();
	}
	else if ( m_oED2K )
	{
		m_sPath += _T("ed2k_");
		m_sPath += m_oED2K.toString();
	}
	else if ( m_oBTH ) 
	{
		m_sPath += _T("btih_");
		m_sPath += m_oBTH.toString();
	}
	else if ( m_oMD5 )
	{
		m_sPath += _T("md5_");
		m_sPath += m_oMD5.toString();
	}
	else if ( m_sName.GetLength() > 0 )
	{
		m_sPath += _T("name_");
		m_sPath += CDownloadTask::SafeFilename( m_sName.Left( 32 ) );
	}
	else
	{
		m_sPath.Format( _T("rand_%2i%2i%2i%2i"), rand() % 100, rand() % 100, rand() % 100, rand() % 100 );
	}

	// Add a .partial extension
	m_sPath += _T(".partial");
	// Create download directory if it doesn't exist
	CreateDirectory( Settings.Downloads.IncompletePath );
	// Add the path
	m_sPath = Settings.Downloads.IncompletePath + _T("\\") + m_sPath;

	ASSERT( m_sPath.GetLength() < MAX_PATH - 1 );
}

//////////////////////////////////////////////////////////////////////
// CDownloadBase serialize

void CDownloadBase::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar << m_sName;
		ar << m_sSearchKeyword;
		ar << m_nSize;
        SerializeOut( ar, m_oSHA1 );
		ar << (uint32)m_bSHA1Trusted;
        SerializeOut( ar, m_oTiger );
		ar << (uint32)m_bTigerTrusted;
        SerializeOut( ar, m_oMD5 );
		ar << (uint32)m_bMD5Trusted;
        SerializeOut( ar, m_oED2K );
		ar << (uint32)m_bED2KTrusted;
		SerializeOut( ar, m_oBTH );
		ar << (uint32)m_bBTHTrusted;
	}
	else
	{
		ar >> m_sName;

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
		uint32 b;
        SerializeIn( ar, m_oSHA1, nVersion );
		ar >> b;
		m_bSHA1Trusted = b != 0;
        SerializeIn( ar, m_oTiger, nVersion );
		ar >> b;
		m_bTigerTrusted = b != 0;
        if ( nVersion >= 22 )
		{
			SerializeIn( ar, m_oMD5, nVersion );
			ar >> b;
			m_bMD5Trusted = b != 0;
		}
        if ( nVersion >= 13 )
		{
			SerializeIn( ar, m_oED2K, nVersion );
			ar >> b;
			m_bED2KTrusted = b != 0;
		}
		if ( nVersion >= 37 )
		{
			SerializeIn( ar, m_oBTH, nVersion );
			ar >> b;
			m_bBTHTrusted = b != 0;
		}
	}
}
