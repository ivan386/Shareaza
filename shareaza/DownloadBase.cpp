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
	ASSERT( ! m_oSHA1.IsValid() );
	ASSERT( ! m_oTiger.IsValid() );
	ASSERT( ! m_oMD5.IsValid() );
	ASSERT( ! m_oED2K.IsValid() );
	ASSERT( ! m_oBTH.IsValid() );
	m_pTask			= NULL;
	ASSERT( m_oVerified.IsEmpty() );
	ASSERT( m_oInvalid.IsEmpty() );
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
	
	if ( m_oSHA1.IsValid() ) m_sLocalName += m_oSHA1.ToString();
	else if ( m_oTiger.IsValid() ) m_sLocalName += m_oTiger.ToString();
	else if ( m_oED2K.IsValid() ) m_sLocalName += m_oED2K.ToString();
	else if ( m_oBTH.IsValid() ) m_sLocalName += m_oBTH.ToString();
	
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
		
		m_oSHA1.SerializeStore( ar, nVersion );

		m_oTiger.SerializeStore( ar, nVersion );

		m_oMD5.SerializeStore( ar, nVersion );

		m_oED2K.SerializeStore( ar, nVersion );
	}
	else
	{
		m_oVerified.Delete();
		m_oInvalid.Delete();

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
		
		m_oSHA1.SerializeLoad( ar, nVersion );

		m_oTiger.SerializeLoad( ar, nVersion );

		if ( nVersion >= 22 ) m_oMD5.SerializeLoad( ar, nVersion );

		if ( nVersion >= 13 ) m_oED2K.SerializeLoad( ar, nVersion );
	}
}
