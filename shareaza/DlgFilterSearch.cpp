//
// DlgFilterSearch.cpp
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
#include "DlgFilterSearch.h"
#include "MatchObjects.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CFilterSearchDlg, CSkinDialog)
	//{{AFX_MSG_MAP(CFilterSearchDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFilterSearchDlg dialog

CFilterSearchDlg::CFilterSearchDlg(CWnd* pParent, CMatchList* pMatches) : CSkinDialog(CFilterSearchDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFilterSearchDlg)
	m_sFilter = _T("");
	m_bHideBusy = FALSE;
	m_bHideLocal = FALSE;
	m_bHidePush = FALSE;
	m_bHideReject = FALSE;
	m_bHideUnstable = FALSE;
	m_bHideBogus = FALSE;
	m_nSources = 0;
	m_sMaxSize = _T("");
	m_sMinSize = _T("");
	//}}AFX_DATA_INIT
	m_pMatches = pMatches;
}

void CFilterSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFilterSearchDlg)
	DDX_Control(pDX, IDC_SOURCES_SPIN, m_wndSources);
	DDX_Text(pDX, IDC_FILTER, m_sFilter);
	DDX_Check(pDX, IDC_FILTER_BUSY, m_bHideBusy);
	DDX_Check(pDX, IDC_FILTER_LOCAL, m_bHideLocal);
	DDX_Check(pDX, IDC_FILTER_PUSH, m_bHidePush);
	DDX_Check(pDX, IDC_FILTER_REJECT, m_bHideReject);
	DDX_Check(pDX, IDC_FILTER_UNSTABLE, m_bHideUnstable);
	DDX_Check(pDX, IDC_FILTER_BOGUS, m_bHideBogus);
	DDX_Text(pDX, IDC_SOURCES, m_nSources);
	DDX_Text(pDX, IDC_MAX_SIZE, m_sMaxSize);
	DDX_Text(pDX, IDC_MIN_SIZE, m_sMinSize);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CFilterSearchDlg message handlers

BOOL CFilterSearchDlg::OnInitDialog() 
{
	CSkinDialog::OnInitDialog();
	
	SkinMe( _T("CFilterSearchDlg"), IDR_SEARCHFRAME );
	
	if ( m_pMatches != NULL )
	{
		m_sFilter		= m_pMatches->m_sFilter;
		m_bHideBusy		= m_pMatches->m_bFilterBusy;
		m_bHidePush		= m_pMatches->m_bFilterPush;
		m_bHideUnstable	= m_pMatches->m_bFilterUnstable;
		m_bHideLocal	= m_pMatches->m_bFilterLocal;
		m_bHideReject	= m_pMatches->m_bFilterReject;
		m_bHideBogus	= m_pMatches->m_bFilterBogus;
		m_nSources		= m_pMatches->m_nFilterSources;

		if ( m_pMatches->m_nFilterMinSize > 0 )
			m_sMinSize	= Settings.SmartVolume( m_pMatches->m_nFilterMinSize, FALSE );
		if ( m_pMatches->m_nFilterMaxSize > 0 )
			m_sMaxSize	= Settings.SmartVolume( m_pMatches->m_nFilterMaxSize, FALSE );
	}
	
	m_wndSources.SetRange( 0, 256 );
	
	UpdateData( FALSE );
	
	return TRUE;
}

void CFilterSearchDlg::OnOK() 
{
	UpdateData( TRUE );

	if ( m_pMatches != NULL )
	{
		m_pMatches->m_sFilter			= m_sFilter;
		m_pMatches->m_bFilterBusy		= m_bHideBusy;
		m_pMatches->m_bFilterPush		= m_bHidePush;
		m_pMatches->m_bFilterUnstable	= m_bHideUnstable;
		m_pMatches->m_bFilterLocal		= m_bHideLocal;
		m_pMatches->m_bFilterReject		= m_bHideReject;
		m_pMatches->m_bFilterBogus		= m_bHideBogus;
		m_pMatches->m_nFilterMinSize	= Settings.ParseVolume( m_sMinSize, FALSE );
		m_pMatches->m_nFilterMaxSize	= Settings.ParseVolume( m_sMaxSize, FALSE );
		m_pMatches->m_nFilterSources	= m_nSources;
	}
	
	CSkinDialog::OnOK();
}
