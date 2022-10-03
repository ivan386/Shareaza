//
// DownloadWithExtras.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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

CDownloadWithExtras::CDownloadWithExtras() :
	m_pMonitorWnd			( NULL )
,	m_pPreviewWnd			( NULL )

,	m_pReviewFirst			( NULL )
,	m_pReviewLast			( NULL )
,	m_nReviewCount			( 0 )

,	m_bWaitingPreview		( FALSE )
,	m_bRemotePreviewCapable	( FALSE )
{
}

CDownloadWithExtras::~CDownloadWithExtras()
{
	DeletePreviews();
	DeleteReviews();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithExtras preview function

BOOL CDownloadWithExtras::PreviewFile(DWORD nIndex, CSingleLock* /* pLock */)
{
	DeletePreviews();

	if ( ! CanPreview( nIndex ) )
		return FALSE;

	ASSERT( m_pPreviewWnd == NULL );
	m_pPreviewWnd = new CFilePreviewDlg( this, nIndex );

	return TRUE;
}

BOOL CDownloadWithExtras::GotPreview() const
{
	return ( GetFileAttributes( CString( _T("\\\\?\\") ) + m_sPath + _T(".png") ) != INVALID_FILE_ATTRIBUTES );
}

BOOL CDownloadWithExtras::IsPreviewVisible() const
{
	return ( m_pPreviewWnd != NULL );
}

BOOL CDownloadWithExtras::CanPreview(DWORD nIndex)
{
	return ( m_pPreviewWnd == NULL ) && ! IsTasking() && GetCompleted( nIndex );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithExtras preview file management

void CDownloadWithExtras::AddPreviewName(LPCTSTR pszFile)
{
	m_pPreviews.AddTail( pszFile );
	SetModified();
}

void CDownloadWithExtras::DeletePreviews()
{
	for ( POSITION pos = m_pPreviews.GetHeadPosition() ; pos ; )
	{
		POSITION posRemove = pos;
		CString strPath = m_pPreviews.GetNext( pos );

		if ( DeleteFileEx( strPath, FALSE, FALSE, TRUE ) )
			m_pPreviews.RemoveAt( posRemove );
	}

	SetModified();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithExtras review file management

BOOL CDownloadWithExtras::AddReview(IN_ADDR* pIP, int nClientID, int nRating, LPCTSTR pszUserName, LPCTSTR pszComment)
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
		theApp.Message( MSG_DEBUG, _T("Ignoring multiple reviews from %s"), inet_ntoa( *pIP ) );
		return FALSE;
	}

	// If we already have a review from this user name with the same data, then exit
	if ( FindReview( nRating, pszUserName, pszComment ) )
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


BOOL CDownloadWithExtras::AddReview(CDownloadReview* pReview)
{
	// If we have too may reviews, then exit
	if ( m_nReviewCount > Settings.Downloads.MaxReviews )
	{
		theApp.Message( MSG_DEBUG, _T("Maximum number of reviews reached") );
		delete pReview;
		return FALSE;
	}

	// Add the review
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

// Delete a reviews
void CDownloadWithExtras::DeleteReview(CDownloadReview *pReview)
{
	if ( pReview == NULL ) return;

	if ( m_nReviewCount ) m_nReviewCount--;

	if ( pReview->m_pNext == NULL )
	{
		if ( pReview->m_pPrev == NULL )
		{
			// Delete the last review on the list
			m_pReviewFirst = m_pReviewLast = NULL;
			delete pReview;
			return;
		}
		else
		{
			// Delete the only review
			pReview->m_pPrev->m_pNext = NULL;
			m_pReviewLast = pReview->m_pPrev;
			delete pReview;
			return;
		}
	}
	else if ( pReview->m_pPrev == NULL )
	{
		// Delete the first review on the list
		pReview->m_pNext->m_pPrev = NULL;
		m_pReviewFirst = pReview->m_pNext;
		delete pReview;
		return;

	}
	else
	{
		// Delete a review in the middle of the list
		pReview->m_pPrev->m_pNext = pReview->m_pNext;
		pReview->m_pNext->m_pPrev = pReview->m_pPrev;
		delete pReview;
		return;
	}
}

// Delete all reviews
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

// Find a review given an IP
CDownloadReview* CDownloadWithExtras::FindReview(IN_ADDR* pIP) const
{
	CDownloadReview *pReview = m_pReviewFirst;
	if ( pIP == NULL ) return NULL;

	while ( pReview )
	{
		if ( pReview->m_pUserIP.S_un.S_addr == pIP->S_un.S_addr )
			return pReview;
		pReview = pReview->m_pNext;
	}

	return NULL;
}

// Find a review given a user name
CDownloadReview* CDownloadWithExtras::FindReview(LPCTSTR pszUserName) const
{
	CDownloadReview *pReview = m_pReviewFirst;
	if ( ! *pszUserName ) return NULL;

	while ( pReview )
	{
		if ( _tcscmp( pReview->m_sUserName, pszUserName ) == 0 )
			return pReview;
		pReview = pReview->m_pNext;
	}

	return NULL;
}

// Find a review given a rating/name/comment
CDownloadReview* CDownloadWithExtras::FindReview(int nRating, LPCTSTR pszName, LPCTSTR pszComment) const
{
	CDownloadReview *pReview = m_pReviewFirst;
	CString strName, strComment;
	strName = pszName;
	strComment = pszComment;

	while ( pReview )
	{
		if ( ( pReview->m_nFileRating == nRating ) &&
			 ( _tcscmp( pReview->m_sUserName, pszName ) == 0 ) &&
			 ( _tcscmp( pReview->m_sFileComments, pszComment ) == 0 ) )
			return pReview;
		pReview = pReview->m_pNext;
	}

	return NULL;
}

// Get the average rating of all reviews
int CDownloadWithExtras::GetReviewAverage() const
{
	int nAverageRating = 0, nCount = 0;

	CDownloadReview *pNext = NULL, *pReview = m_pReviewFirst;

	while ( pReview )
	{
		pNext = pReview->m_pNext;
		if ( pReview->m_nFileRating > 0 )
		{
			nAverageRating += pReview->m_nFileRating;
			nCount ++;
		}
		pReview = pNext;
	}

	if ( nCount ) nAverageRating /= nCount;

	return nAverageRating;
}


//////////////////////////////////////////////////////////////////////
// CDownloadWithExtras monitor window

void CDownloadWithExtras::ShowMonitor()
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( ! IsMonitorVisible() )
	{
		m_pMonitorWnd = new CDownloadMonitorDlg( (CDownload*)this );
	}

	if ( m_pMonitorWnd )
	{
		m_pMonitorWnd->Show();
	}
}

BOOL CDownloadWithExtras::IsMonitorVisible() const
{
	return m_pMonitorWnd && m_pMonitorWnd->m_pDownload &&
		IsWindow( m_pMonitorWnd->GetSafeHwnd() );
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

		ar.WriteCount( GetReviewCount() );

		CDownloadReview *pReview = m_pReviewFirst;
		while ( pReview )
		{
			pReview->Serialize( ar, nVersion );
			pReview = pReview->m_pNext;
		}


	}
	else
	{
		for ( DWORD_PTR nCount = ar.ReadCount() ; nCount ; nCount-- )
		{
			CString str;
			ar >> str;
			m_pPreviews.AddTail( str );
		}

		if ( nVersion >= 32 )
		{

			for ( DWORD_PTR nCount = ar.ReadCount() ; nCount ; nCount-- )
			{
				CDownloadReview *pReview = new CDownloadReview;
				pReview->Serialize( ar, nVersion );
				AddReview( pReview );
			}
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

	m_nUserPicture	= nUserPicture;
	m_sUserName		= pszUserName;
	m_nFileRating	= nRating;

	m_sFileComments = pszComment;

	if ( pIP != NULL )
	{
		m_pUserIP = *pIP;
		if ( m_sUserName.IsEmpty() ) m_sUserName = inet_ntoa( *pIP );
	}
	else
	{
		m_pUserIP.S_un.S_addr = 0;
	}
}

CDownloadReview::~CDownloadReview()
{
	// If a preview pic or any other dynamically added item is ever added to the review, remember
	// to delete it here.

}

//////////////////////////////////////////////////////////////////////
// CDownloadReview serialize

void CDownloadReview::Serialize(CArchive& ar, int /*nVersion*/)
{
	if ( ar.IsStoring() )
	{
		ar << m_pUserIP.S_un.S_addr;
		ar << m_nUserPicture;
		ar << m_sUserName;
		ar << m_nFileRating;
		ar << m_sFileComments;
	}
	else
	{
		ar >> m_pUserIP.S_un.S_addr;
		ar >> m_nUserPicture;
		ar >> m_sUserName;
		ar >> m_nFileRating;
		ar >> m_sFileComments;
	}
}
