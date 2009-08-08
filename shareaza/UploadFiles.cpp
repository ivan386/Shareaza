//
// UploadFiles.cpp
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
#include "UploadFiles.h"
#include "UploadFile.h"
#include "UploadTransfer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CUploadFiles UploadFiles;


//////////////////////////////////////////////////////////////////////
// CUploadFiles construction

CUploadFiles::CUploadFiles()
{
}

CUploadFiles::~CUploadFiles()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CUploadFiles clear

void CUploadFiles::Clear()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		GetNext( pos )->Remove();
	}
	
	ASSERT( GetCount() == 0 );
}

//////////////////////////////////////////////////////////////////////
// CUploadFiles file allocation

CUploadFile* CUploadFiles::GetFile(CUploadTransfer* pUpload)
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CUploadFile* pFile = GetNext( pos );

		if ( pFile->m_pAddress.S_un.S_addr == pUpload->m_pHost.sin_addr.S_un.S_addr )
		{
			if ( pFile->m_sPath.CompareNoCase( pUpload->m_sPath ) == 0 )
			{
				pFile->Add( pUpload );
				return pFile;
			}
		}
	}
	
	CUploadFile* pFile = new CUploadFile( pUpload );
	m_pList.AddTail( pFile );
	
	return pFile;
}

//////////////////////////////////////////////////////////////////////
// CUploadFiles remove an upload trasnfer

void CUploadFiles::Remove(CUploadTransfer* pTransfer)
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		POSITION posRemove = pos;
		CUploadFile* pFile = GetNext( pos );
		
		if ( pFile->Remove( pTransfer ) )
		{
			delete pFile;
			m_pList.RemoveAt( posRemove );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CUploadFiles move a file with this transfer to the head/tail. (Cheap BT sorting)

void CUploadFiles::MoveToHead(CUploadTransfer* pTransfer)
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		POSITION posThis = pos;
		CUploadFile* pFile = GetNext( pos );

		if ( pFile->GetActive() == pTransfer )
		{
			m_pList.RemoveAt( posThis );
			m_pList.AddHead( pFile );
			return;
		}
	}
}

void CUploadFiles::MoveToTail(CUploadTransfer* pTransfer)
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		POSITION posThis = pos;
		CUploadFile* pFile = GetNext( pos );

		if ( pFile->GetActive() == pTransfer )
		{
			m_pList.RemoveAt( posThis );
			m_pList.AddTail( pFile );
			return;
		}
	}
}

