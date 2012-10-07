//
// Statistics.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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
#include "Statistics.h"
#include "Network.h"
#include "Neighbours.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CStatistics Statistics;


//////////////////////////////////////////////////////////////////////
// CStatistics construction

CStatistics::CStatistics()
{
	ZeroMemory( &Today, sizeof(Today) );
	ZeroMemory( &Ever, sizeof(Ever) );
	ZeroMemory( &Current, sizeof(Current) );

	m_tSeconds = GetMicroCount() / 1000;
}

CStatistics::~CStatistics()
{
}

//////////////////////////////////////////////////////////////////////
// CStatistics update

void CStatistics::Update()
{
	QWORD tNow = GetMicroCount() / 1000;	// ms

	if ( tNow >= m_tSeconds + 1000 )
	{
		QWORD nElapsed = ( tNow - m_tSeconds ) / 1000; // s
		if ( Network.IsWellConnected() )
		{
			Current.Timer.Connected += nElapsed;
			if ( Neighbours.IsG2Hub() ) Current.Timer.Hub += nElapsed;
			if ( Neighbours.IsG1Ultrapeer() ) Current.Timer.Ultrapeer += nElapsed;
		}
		m_tSeconds = tNow;
	}

	CopyMemory( &Last, &Current, sizeof(Current) );
	Add( &Today, &Current, sizeof(Current) );
	Add( &Ever, &Current, sizeof(Current) );
	ZeroMemory( &Current, sizeof(Current) );
}

//////////////////////////////////////////////////////////////////////
// CStatistics adder

void CStatistics::Add(LPVOID pTarget, LPCVOID pSource, int nCount)
{
	QWORD* pqwTarget = (QWORD*)pTarget;
	QWORD* pqwSource = (QWORD*)pSource;

	for ( nCount /= sizeof(QWORD) ; nCount ; nCount-- )
	{
		*pqwTarget += *pqwSource;

		pqwTarget++;
		pqwSource++;
	}
}

