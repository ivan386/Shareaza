//
// PageComment.cpp
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
#include "TorrentWizard.h"
#include "PageComment.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CCommentPage, CWizardPage)

BEGIN_MESSAGE_MAP(CCommentPage, CWizardPage)
	//{{AFX_MSG_MAP(CCommentPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCommentPage property page

CCommentPage::CCommentPage() : CWizardPage(CCommentPage::IDD)
{
	//{{AFX_DATA_INIT(CCommentPage)
	m_sComment = _T("");
	//}}AFX_DATA_INIT
}

CCommentPage::~CCommentPage()
{
}

void CCommentPage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCommentPage)
	DDX_Text(pDX, IDC_COMMENT, m_sComment);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CCommentPage message handlers

void CCommentPage::OnReset() 
{
	// Nothing here
}

BOOL CCommentPage::OnSetActive() 
{
	SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );
	return CWizardPage::OnSetActive();
}

LRESULT CCommentPage::OnWizardBack() 
{
	return IDD_TRACKER_PAGE;
}

LRESULT CCommentPage::OnWizardNext() 
{
	return IDD_OUTPUT_PAGE;
}
