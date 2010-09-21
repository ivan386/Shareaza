//
// DlgDownloadReviews.cpp
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
#include "LiveList.h"
#include "DlgDownloadReviews.h"
#include "Settings.h"
#include "Download.h"
#include "CoolInterface.h"
#include "Skin.h"
#include "Downloads.h"
#include "Transfers.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CDownloadReviewDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CDownloadReviewDlg, CSkinDialog)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDownloadReviewDlg dialog

CDownloadReviewDlg::CDownloadReviewDlg(CWnd* pParent, CDownload* pDownload) : CSkinDialog( CDownloadReviewDlg::IDD, pParent )
{
	m_pDownload = pDownload;
}

CDownloadReviewDlg::~CDownloadReviewDlg()
{

}

void CDownloadReviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REVIEWS, m_wndReviews);
	DDX_Text(pDX, IDC_REVIEW_FILENAME, m_sReviewFileName);

}

/////////////////////////////////////////////////////////////////////////////
// CDownloadReviewDlg message handlers

BOOL CDownloadReviewDlg::OnInitDialog() 
{
	CSkinDialog::OnInitDialog();

	CRect rcList;
	m_wndReviews.GetClientRect( &rcList );
	rcList.right -= GetSystemMetrics( SM_CXVSCROLL );

	CoolInterface.SetImageListTo( m_wndReviews, LVSIL_SMALL );
	m_wndReviews.InsertColumn( 0, _T("User"), LVCFMT_LEFT, 100, -1 );
	m_wndReviews.InsertColumn( 1, _T("Rating"), LVCFMT_CENTER, 90, 0 );
	m_wndReviews.InsertColumn( 2, _T("Comments"), LVCFMT_CENTER, rcList.right- 100 - 80, 1 );
	m_wndReviews.InsertColumn( 3, _T("Order"), LVCFMT_CENTER, 0, 2 );
	Skin.Translate( _T("CReviewList"), m_wndReviews.GetHeaderCtrl() );
	
	m_wndReviews.SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE,
		LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP, LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP );
	m_wndReviews.EnableToolTips();
	
	// Sort by order added- first at the top
	CLiveList::Sort( &m_wndReviews, 3, FALSE );
	CLiveList::Sort( &m_wndReviews, 3, FALSE );


	CLiveList pReviews( 4 );
	int nIndex = 1;
	// Lock while we're loading the list. (In case the download is destroyed)
	CSingleLock pLock( &Transfers.m_pSection, TRUE );
	
	if ( ! m_pDownload ) return FALSE;

	m_sReviewFileName = m_pDownload->m_sName;

	CDownloadReview* pReview = m_pDownload->GetFirstReview();

	while ( pReview )
	{
		CLiveItem* pItem = pReviews.Add( pReview );
	
		// Client picture
		// Note: We don't have pictures yet. Currently, it uses a star for a G2 
		// review, and a little person for everyone else
		switch ( pReview->m_nUserPicture )
		{
		case 0:
			pItem->SetImage( 0, CoolInterface.ImageForID( ID_TOOLS_WIZARD ) );
			break;
		case 1:
			pItem->SetImage( 0, CoolInterface.ImageForID( ID_TOOLS_PROFILE ) );
			break;
		case 2:
			pItem->SetImage( 0, CoolInterface.ImageForID( ID_TOOLS_WIZARD ) );
			break;
		case 3:
			pItem->SetImage( 0, CoolInterface.ImageForID( ID_TOOLS_PROFILE ) );
			break;
		default:
			pItem->SetImage( 0, CoolInterface.ImageForID( ID_TOOLS_PROFILE ) );
		}
		
		pItem->Set( 0, pReview->m_sUserName );

		int nRating = min( pReview->m_nFileRating, 6 );
		nRating = max ( nRating, 0 );
		CString strRating;
		LoadString( strRating, IDS_RATING_NORATING + nRating );
		pItem->Set( 1, strRating );

		
		pItem->Set( 2, pReview->m_sFileComments );
		pItem->Format( 3, _T("%i"), nIndex );
		

		nIndex++;
		pReview = pReview->m_pNext;
	}

	pLock.Unlock();

	//m_wndReviews.SetFont( &theApp.m_gdiFontBold );

	pReviews.Apply( &m_wndReviews, TRUE );

	// Set window icon
	SkinMe( NULL, IDR_MEDIAFRAME );

	UpdateData( FALSE );
	
	return TRUE;
}



void CDownloadReviewDlg::OnOK() 
{
	UpdateData( TRUE );
	
	CSkinDialog::OnOK();
}

