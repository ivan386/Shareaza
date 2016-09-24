//
// Transfer.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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
#include "Transfers.h"
#include "Transfer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CTransfer construction

CTransfer::CTransfer(PROTOCOLID nProtocol)
	: CConnection		( nProtocol )
	, m_pServer			( { AF_INET } )
	, m_nRunCookie		( 0 )
	, m_nState			( 0 )
	, m_nBandwidth		( 0ul )
	, m_nOffset			( SIZE_UNKNOWN )
	, m_nLength			( SIZE_UNKNOWN )
	, m_nPosition		( 0 )
	, m_tRequest		( 0 )
{
}

CTransfer::~CTransfer()
{
	ASSERT( ! IsValid() );
	if ( IsValid() ) Close();
}

//////////////////////////////////////////////////////////////////////
// CTransfer operations

BOOL CTransfer::ConnectTo(const IN_ADDR* pAddress, WORD nPort)
{
	m_nState = 0;

	if ( CConnection::ConnectTo( pAddress, nPort ) )
	{
		Transfers.Add( this );
		return TRUE;
	}

	return FALSE;
}

void CTransfer::AttachTo(CConnection* pConnection)
{
	CConnection::AttachTo( pConnection );
	Transfers.Add( this );
}

void CTransfer::Close(UINT nError)
{
	Transfers.Remove( this );
	CConnection::Close( nError );
}

//////////////////////////////////////////////////////////////////////
// CTransfer HTTP headers

void CTransfer::ClearHeaders()
{
	m_pHeaderName.RemoveAll();
	m_pHeaderValue.RemoveAll();
}

BOOL CTransfer::OnHeaderLine(CString& strHeader, CString& strValue)
{
	m_pHeaderName.Add( strHeader );
	m_pHeaderValue.Add( strValue );

	return CConnection::OnHeaderLine( strHeader, strValue );
}
