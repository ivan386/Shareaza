//
// DownloadWithExtras.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
	m_pMonitorWnd	= NULL;
	m_pPreviewWnd	= NULL;
	m_pReviewFirst	= NULL;
	m_pReviewLast	= NULL;
	m_nReviewCount	= 0;
}

CDownloadWithExtras::~CDownloadWithExtras()
{
	DeletePreviews();
	DeleteReviews();
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
// CDownloadWithExtras review file management

BOOL CDownloadWithExtras::AddReview(in_addr *pIP, int nClientID, int nRating, LPCTSTR pszUserName, LPCTSTR pszComment)
{
	// If we have too may reviews, then exit
	if ( m_nReviewCount > Settings.Downloads.MaxReviews ) 
	{
		theApp.Message( MSG_DEBUG, _T("Maximum number of reviews reached") );
		return FALSE;
	}

	// If we already have a review from this IP, then exit
	if ( FindReview( pIP ) ) 
	{
		theApp.Message( MSG_DEBUG, _T("Ignoring duplicate review from %s"), inet_ntoa( *pIP ) );
		return FALSE;
	}

	// Add the review
	CDownloadReview* pReview = new CDownloadReview(pIP, nClientID, nRating, pszUserName, pszComment);
	m_nReviewCount++;

	pReview->m_pPrev = m_pReviewLast;
	pReview->m_pNext = NULL;
		
	if ( m_pReviewLast != NULL )
	{
		m_pReviewLast->m_pNext = pReview;
		m_pReviewLast = pReview;
	}
	else
	{
		m_pReviewFirst = m_pReviewLast = pReview;
	}

	return TRUE;
}

void CDownloadWithExtras::DeleteReviews()
{
	CDownloadReview *pNext = NULL, *pReview = m_pReviewFirst;

	while ( pReview )
	{
		pNext = pReview->m_pNext;
		delete pReview;
		pReview = pNext;
	}

	m_pReviewFirst	= NULL;
	m_pReviewLast	= NULL;
	m_nReviewCount	= 0;
}

CDownloadReview* CDownloadWithExtras::FindReview(in_addr *pIP) const
{
	CDownloadReview *pReview = m_pReviewFirst;

	while ( pReview )
	{
		if ( pReview->m_pUserIP.S_un.S_addr == pIP->S_un.S_addr )
			return pReview;
		pReview = pReview->m_pNext;
	}

	return NULL;
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

//////////////////////////////////////////////////////////////////////
// CDownloadReview construction

CDownloadReview::CDownloadReview()
{
	m_pNext			= NULL;
	m_pPrev			= NULL;

	m_nUserPicture	= 0;
	m_nFileRating	= 0;
}

CDownloadReview::CDownloadReview(in_addr *pIP, int nUserPicture, int nRating, LPCTSTR pszUserName, LPCTSTR pszComment)
{
	m_pNext			= NULL;
	m_pPrev			= NULL;

	m_pUserIP		= *pIP;
	m_nUserPicture	= nUserPicture;
	m_sUserName		= pszUserName;
	m_nFileRating	= nRating;

	m_sFileComments = pszComment;

	if ( m_sUserName.IsEmpty() ) m_sUserName = inet_ntoa( *pIP );
}

CDownloadReview::~CDownloadReview()
{
	// If a preview pic or any other dynamically added item is ever added to the review, remember 
	// to delete it here.

}

