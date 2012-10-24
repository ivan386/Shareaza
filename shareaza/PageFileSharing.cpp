//
// PageFileSharing.cpp
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
#include "Library.h"
#include "SharedFile.h"
#include "PageFileSharing.h"

#include "Transfers.h"
#include "UploadQueue.h"
#include "UploadQueues.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CFileSharingPage, CFilePropertiesPage)

BEGIN_MESSAGE_MAP(CFileSharingPage, CFilePropertiesPage)
	ON_BN_CLICKED(IDC_SHARE_OVERRIDE_0, OnShareOverride0)
	ON_BN_CLICKED(IDC_SHARE_OVERRIDE_1, OnShareOverride1)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFileSharingPage property page

CFileSharingPage::CFileSharingPage() : 
	CFilePropertiesPage(CFileSharingPage::IDD), m_bOverride( -1 ), 
	m_bShare( FALSE ), m_sTags()
{
}

CFileSharingPage::~CFileSharingPage()
{
}

void CFileSharingPage::DoDataExchange(CDataExchange* pDX)
{
	CFilePropertiesPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_FILE_TAGS, m_wndTags);
	DDX_Control(pDX, IDC_FILE_SHARE, m_wndShare);
	DDX_Control(pDX, IDC_FILE_NETWORKS, m_wndNetworks);
	DDX_Radio(pDX, IDC_SHARE_OVERRIDE_0, m_bOverride);
	DDX_Check(pDX, IDC_FILE_SHARE, m_bShare);
	DDX_CBString(pDX, IDC_FILE_TAGS, m_sTags);
}

/////////////////////////////////////////////////////////////////////////////
// CFileSharingPage message handlers

BOOL CFileSharingPage::OnInitDialog()
{
	CFilePropertiesPage::OnInitDialog();

	m_wndTags.AddString( _T("") );

	{
		CQuickLock oLock( UploadQueues.m_pSection );

		CList< CString > pAdded;

		for ( POSITION pos = UploadQueues.GetIterator() ; pos ; )
		{
			CUploadQueue* pQueue = UploadQueues.GetNext( pos );

			if ( pQueue->m_sShareTag.GetLength() )
			{
				if ( pAdded.Find( pQueue->m_sShareTag ) == NULL )
				{
					pAdded.AddTail( pQueue->m_sShareTag );
					m_wndTags.AddString( pQueue->m_sShareTag );
				}
			}
		}

		if ( pAdded.IsEmpty() )
		{
			m_wndTags.AddString( _T("Release") );
			m_wndTags.AddString( _T("Popular") );
		}
	}

	{
		CQuickLock oLock( Library.m_pSection );

		if ( CLibraryFile* pSingleFile = GetFile() )
		{
			m_bOverride	= pSingleFile->IsSharedOverride();
			m_bShare	= pSingleFile->IsShared();
			m_sTags		= pSingleFile->m_sShareTags;
		}
		else		
		{
			CLibraryListPtr pList( GetList() );
			if ( pList )
			{
				for ( POSITION pos = pList->GetHeadPosition() ; pos ; )
				{
					if ( CLibraryFile* pFile = pList->GetNextFile( pos ) )
					{
						m_bOverride	= pFile->IsSharedOverride();
						m_bShare	= pFile->IsShared();
						m_sTags		= pFile->m_sShareTags;
					}
				}
			}
		}
	}

	UpdateData( FALSE );
	m_wndShare.EnableWindow( m_bOverride );

	return TRUE;
}

void CFileSharingPage::OnShareOverride0()
{
	UpdateData();

	m_wndShare.EnableWindow( m_bOverride );

	if ( ! m_bOverride )
	{
		CSingleLock oLock( &Library.m_pSection, TRUE );
		if ( CLibraryFile* pFile = GetFile() )
		{
			m_bShare = pFile->IsShared( true );

			oLock.Unlock();
			UpdateData( FALSE );
		}
	}
}

void CFileSharingPage::OnShareOverride1()
{
	OnShareOverride0();
}

void CFileSharingPage::OnOK()
{
	UpdateData();

	CLibraryListPtr pList( GetList() );
	if ( pList )
	{
		CQuickLock oLock( Library.m_pSection );

		for ( POSITION pos = pList->GetHeadPosition() ; pos ; )
		{
			if ( CLibraryFile* pFile = pList->GetNextFile( pos ) )
			{
				pFile->SetShared( ( m_bShare != FALSE ), ( m_bOverride != FALSE ) );
				pFile->m_sShareTags = m_sTags;
			}
		}

	}

	CFilePropertiesPage::OnOK();
}
