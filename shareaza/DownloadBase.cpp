//
// DownloadBase.cpp
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
// CDownloadBase local name

void CDownloadBase::GenerateLocalName()
{
	if ( m_sLocalName.GetLength() > 0 ) return;
	
	if ( m_bSHA1 ) m_sLocalName += CSHA::HashToString( &m_pSHA1 );
	else if ( m_bTiger ) m_sLocalName += CTigerNode::HashToString( &m_pTiger );
	else if ( m_bED2K ) m_sLocalName += CED2K::HashToString( &m_pED2K );
	else if ( m_bBTH ) m_sLocalName += CSHA::HashToString( &m_pBTH );
	
	if ( m_sRemoteName.GetLength() > 0 )
	{
		if ( m_sLocalName.GetLength() > 0 ) m_sLocalName += _T(" ");
		m_sLocalName += CDownloadTask::SafeFilename( m_sRemoteName );
	}
	
	if ( m_sLocalName.GetLength() > 0 )
	{
		CreateDirectory( Settings.Downloads.IncompletePath, NULL );
		m_sLocalName = Settings.Downloads.IncompletePath + _T("\\") + m_sLocalName;
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadBase serialize

void CDownloadBase::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar << m_sRemoteName;
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
		ar >> m_sRemoteName;
		
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

		ar >> m_bTiger;
		if ( m_bTiger ) ar.Read( &m_pTiger, sizeof(TIGEROOT) );
		if ( nVersion >= 31 ) ar >> m_bTigerTrusted;

		if ( nVersion >= 22 ) ar >> m_bMD5;
		if ( m_bMD5 ) ar.Read( &m_pMD5, sizeof(MD5) );
		if ( nVersion >= 31 ) ar >> m_bMD5Trusted;

		if ( nVersion >= 13 ) ar >> m_bED2K;
		if ( m_bED2K ) ar.Read( &m_pED2K, sizeof(MD4) );
		if ( nVersion >= 31 ) ar >> m_bED2KTrusted;
	}
}
