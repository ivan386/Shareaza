//
// PageComment.cpp
//
// Copyright (c) Shareaza Development Team, 2007-2014.
// This file is part of Shareaza Torrent Wizard (shareaza.sourceforge.net).
//
// Shareaza Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Torrent Wizard is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
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
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCommentPage property page

CCommentPage::CCommentPage()
	: CWizardPage(CCommentPage::IDD, _T("comment"))
{
}

void CCommentPage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_COMMENT, m_sComment);
	DDX_Text(pDX, IDC_SOURCE, m_sSource);
}

/////////////////////////////////////////////////////////////////////////////
// CCommentPage message handlers

BOOL CCommentPage::OnInitDialog() 
{
	CWizardPage::OnInitDialog();

	m_sComment = theApp.GetProfileString( _T("Comments"), _T("Comment") );
	m_sSource = theApp.GetProfileString( _T("Comments"), _T("Source") );

	UpdateData( FALSE );

	return TRUE;
}

void CCommentPage::OnReset() 
{
}

BOOL CCommentPage::OnSetActive() 
{
	SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

	if ( ! theApp.m_sCommandLineComment.IsEmpty() )
	{
		m_sComment = theApp.m_sCommandLineComment;
		theApp.m_sCommandLineComment.Empty();

		Next();
	}

	UpdateData( FALSE );

	return CWizardPage::OnSetActive();
}

LRESULT CCommentPage::OnWizardBack() 
{
	SaveComments();

	return IDD_TRACKER_PAGE;
}

LRESULT CCommentPage::OnWizardNext() 
{
	SaveComments();

	return IDD_OUTPUT_PAGE;
}

void CCommentPage::SaveComments()
{
	UpdateData();

	theApp.WriteProfileString( _T("Comments"), _T("Comment"), m_sComment );
	theApp.WriteProfileString( _T("Comments"), _T("Source"), m_sSource );
}
