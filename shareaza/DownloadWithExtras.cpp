//
// DownloadWithExtras.cpp
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
#include "Transfers.h"
#include "DownloadWithExtras.h"
#include "DlgFilePreview.h"
#include "DlgDownloadMonitor.h"
#include "Plugins.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadWithExtras construction

CDownloadWithExtras::CDownloadWithExtras()
{
	m_pMonitorWnd = NULL;
	m_pPreviewWnd = NULL;
}

CDownloadWithExtras::~CDownloadWithExtras()
{
	DeletePreviews();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithExtras preview function

BOOL CDownloadWithExtras::Preview(CSingleLock* pLock)
{
	DeletePreviews();
	
	if ( ! CanPreview() ) return FALSE;
	
	ASSERT( m_pPreviewWnd == NULL );
	m_pPreviewWnd = new CFilePreviewDlg( (CDownload*)this );
	
	if ( pLock ) pLock->Unlock();
	
	m_pPreviewWnd->Create();
	m_pPreviewWnd->ShowWindow( SW_SHOWNORMAL );
	m_pPreviewWnd->BringWindowToTop();
	
	if ( pLock ) pLock->Lock();
	
	return TRUE;
}

BOOL CDownloadWithExtras::IsPreviewVisible() const
{
	return m_pPreviewWnd != NULL;
}

BOOL CDownloadWithExtras::CanPreview()
{
	if ( m_pPreviewWnd != NULL ) return FALSE;
	
	LPCTSTR pszType = _tcsrchr( m_sLocalName, '.' );
	if ( pszType == NULL ) return FALSE;
	
	CString strType( pszType );
	strType = CharLower( strType.GetBuffer() );
	
	CLSID pCLSID;
	return Plugins.LookupCLSID( _T("DownloadPreview"), strType, pCLSID );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithExtras preview file management

void CDownloadWithExtras::AddPreviewName(LPCTSTR pszFile)
{
	m_pPreviews.AddTail( pszFile );
	theApp.WriteProfileString( _T("Delete"), pszFile, _T("") );
	SetModified();
}

void CDownloadWithExtras::DeletePreviews()
{
	for ( POSITION pos = m_pPreviews.GetHeadPosition() ; pos ; )
	{
		POSITION posRemove = pos;
		CString strPath = m_pPreviews.GetNext( pos );
		
		if ( ::DeleteFile( strPath ) )
		{
			m_pPreviews.RemoveAt( posRemove );
			theApp.WriteProfileString( _T("Delete"), strPath, NULL );
		}
	}
	
	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithExtras monitor window

void CDownloadWithExtras::ShowMonitor(CSingleLock* pLock)
{
	if ( pLock ) pLock->Unlock();
	
	if ( m_pMonitorWnd == NULL )
	{
		m_pMonitorWnd = new CDownloadMonitorDlg( (CDownload*)this );
	}
	
	m_pMonitorWnd->ShowWindow( SW_SHOWNORMAL );
	m_pMonitorWnd->BringWindowToTop();
	
	if ( pLock ) pLock->Lock();
}

BOOL CDownloadWithExtras::IsMonitorVisible() const
{
	return m_pMonitorWnd != NULL;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithExtras serialize

void CDownloadWithExtras::Serialize(CArchive& ar, int nVersion)
{
	CDownloadWithSearch::Serialize( ar, nVersion );
	
	if ( ar.IsStoring() )
	{
		ar.WriteCount( m_pPreviews.GetCount() );
		
		for ( POSITION pos = m_pPreviews.GetHeadPosition() ; pos ; )
		{
			ar << m_pPreviews.GetNext( pos );
		}
	}
	else
	{
		for ( int nCount = ar.ReadCount() ; nCount ; nCount-- )
		{
			CString str;
			ar >> str;
			m_pPreviews.AddTail( str );
		}
	}
}
