//
// DlgIrcInput.cpp
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

#include "stdafx.h"
#include "Shareaza.h"
#include "DlgIrcInput.h"

// CIrcInputDlg dialog

CIrcInputDlg::CIrcInputDlg( CWnd* pParent /*=NULL*/, int nCaptionIndex, BOOL bKickOnly ) : 
	CSkinDialog( CIrcInputDlg::IDD, pParent )
{
	m_nCaptionIndex = nCaptionIndex;
	if ( m_nCaptionIndex )
		m_bKickOnly	= bKickOnly;
	else
		m_bKickOnly = FALSE;
}

CIrcInputDlg::~CIrcInputDlg()
{
}

// CIrcInputDlg message handlers

void CIrcInputDlg::OnOK()
{
	UpdateData( TRUE );

	m_wndAnswer.GetWindowText( m_sAnswer );

	CSkinDialog::OnOK();
}

void CIrcInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConnectToDlg)
	DDX_Control(pDX, IDC_IRC_PROMPT, m_wndPrompt);
	DDX_Control(pDX, IDC_IRC_INPUT, m_wndAnswer);
	//}}AFX_DATA_MAP
}

BOOL CIrcInputDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CIrcInputDlg"), m_nCaptionIndex == 0 ? ID_IRC_ADD : 
		( m_bKickOnly ? ID_IRC_KICKWHY : ID_IRC_BANKICKWHY ) );
	SelectCaption( this, m_nCaptionIndex );
	SelectCaption( &m_wndPrompt, m_nCaptionIndex );

	return TRUE;
}
