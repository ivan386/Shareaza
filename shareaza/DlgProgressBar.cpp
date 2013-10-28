//
// DlgProgressBar.cpp
//
// Copyright (c) Shareaza Development Team, 2009.
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
#include "DlgProgressBar.h"

#include "Settings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CProgressBarDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CProgressBarDlg, CSkinDialog)
END_MESSAGE_MAP()

CProgressBarDlg::CProgressBarDlg(CWnd* pParent) :
	CSkinDialog	( CProgressBarDlg::IDD, pParent )
{
	Create( CProgressBarDlg::IDD, pParent );
}

CProgressBarDlg::~CProgressBarDlg()
{
}

void CProgressBarDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LABEL_ACTION, m_sAction);
	DDX_Text(pDX, IDC_LABEL_EVENT, m_sEvent);
	DDX_Control(pDX, IDC_PROGRESS_EVENT, m_oEventProgress);
	DDX_Text(pDX, IDC_LABEL_SUB_ACTION, m_sSubAction);
	DDX_Text(pDX, IDC_SUB_EVENT, m_sSubEvent);
	DDX_Control(pDX, IDC_PROGRESS_SUB_EVENT, m_oSubEventProgress);
}

void CProgressBarDlg::SetActionText(const CString& strText)
{
	m_sAction = strText;
}

void CProgressBarDlg::SetEventText(const CString& strText)
{
	m_sEvent = strText;
}

void CProgressBarDlg::SetEventRange(const int nLower, const int nUpper)
{
	m_oEventProgress.SetRange32( nLower, nUpper );
	m_oEventProgress.SetPos( nLower );
}

void CProgressBarDlg::SetEventPos(const int nPos)
{
	m_oEventProgress.SetPos( nPos );
}

void CProgressBarDlg::SetEventStep(const int nStep)
{
	m_oEventProgress.SetStep( nStep );
}

void CProgressBarDlg::StepEvent()
{
	m_oEventProgress.StepIt();
}

void CProgressBarDlg::StepEvent( const int nPos )
{
	m_oEventProgress.OffsetPos( nPos );
}

void CProgressBarDlg::SetSubActionText(const CString& strText)
{
	m_sSubAction = strText;
}

void CProgressBarDlg::SetSubEventText(const CString& strText)
{
	m_sSubEvent = strText;
}

void CProgressBarDlg::SetSubEventRange(const int nLower, const int nUpper)
{
	m_oSubEventProgress.SetRange32( nLower, nUpper );
	m_oSubEventProgress.SetPos( nLower );
}

void CProgressBarDlg::SetSubEventPos(const int nPos)
{
	m_oSubEventProgress.SetPos( nPos );
}

void CProgressBarDlg::SetSubEventStep(const int nStep)
{
	m_oSubEventProgress.SetStep( nStep );
}

void CProgressBarDlg::StepSubEvent()
{
	m_oSubEventProgress.StepIt();
}

void CProgressBarDlg::StepSubEvent( const int nPos )
{
	m_oSubEventProgress.OffsetPos( nPos );
}

BOOL CProgressBarDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CProgressBarDlg"), IDR_MAINFRAME );

	if ( Settings.General.LanguageRTL )
	{
		m_oEventProgress.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
		m_oSubEventProgress.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
	}

	return TRUE;
}

void CProgressBarDlg::OnCancel() {}
void CProgressBarDlg::OnOK() {}
