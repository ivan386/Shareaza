//
// ED2K.cpp
//
// Copyright (c) Shareaza Pty. Ltd., 2003.
// This file is part of TorrentAid Torrent Wizard (www.torrentaid.com).
//
// TorrentAid Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// TorrentAid is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TorrentAid; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "StdAfx.h"
#include "ED2K.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CED2K construction

CED2K::CED2K()
{
	Reset();
}

CED2K::~CED2K()
{
}

//////////////////////////////////////////////////////////////////////
// CED2K reset state

void CED2K::Reset()
{
	m_pComposite.Reset();
	m_pBlock.Reset();
	m_nBlockCount = 0;
	m_nBlockBytes = 0;
}

//////////////////////////////////////////////////////////////////////
// CED2K add data

void CED2K::Add(LPCVOID pData, DWORD nLength)
{
	const BYTE* pInput = (const BYTE*)pData;
	
	while ( nLength > 0 )
	{
		DWORD nProcess = min( nLength, ED2K_BLOCK_SIZE - m_nBlockBytes );
		
		m_pBlock.Add( pInput, nProcess );
		
		pInput += nProcess;
		nLength -= nProcess;
		m_nBlockBytes += nProcess;
		
		if ( m_nBlockBytes >= ED2K_BLOCK_SIZE )
		{
			ASSERT( m_nBlockBytes == ED2K_BLOCK_SIZE );
			
			m_pBlock.Finish();
			m_pBlock.GetHash( &m_pLastBlock );
			m_pBlock.Reset();
			m_pComposite.Add( &m_pLastBlock, sizeof(m_pLastBlock) );
			m_nBlockCount ++;
			m_nBlockBytes = 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CED2K finish hash operation

void CED2K::Finish()
{
	if ( m_nBlockCount == 0 )
	{
		m_pBlock.Finish();
		m_pBlock.GetHash( &m_pHash );
	}
	else if ( m_nBlockCount == 1 && m_nBlockBytes == 0 )
	{
		m_pHash = m_pLastBlock;
	}
	else
	{
		if ( m_nBlockBytes > 0 )
		{
			m_pBlock.Finish();
			m_pBlock.GetHash( &m_pLastBlock );
			m_pBlock.Reset();
			m_pComposite.Add( &m_pLastBlock, sizeof(m_pLastBlock) );
			m_nBlockCount ++;
			m_nBlockBytes = 0;
		}
		
		m_pComposite.Finish();
		m_pComposite.GetHash( &m_pHash );
	}
}

//////////////////////////////////////////////////////////////////////
// CED2K get the hash

void CED2K::GetHash(MD4* pHash)
{
	*pHash = m_pHash;
}
