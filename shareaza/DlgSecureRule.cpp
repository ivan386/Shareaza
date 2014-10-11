//
// DlgSecureRule.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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
#include "Security.h"
#include "Skin.h"
#include "DlgSecureRule.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CSecureRuleDlg, CSkinDialog)
	ON_CBN_SELCHANGE(IDC_RULE_EXPIRE, OnSelChangeRuleExpire)
	ON_CBN_SELCHANGE(IDC_RULE_TYPE, OnSelChangeRuleType)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSecureRuleDlg dialog

CSecureRuleDlg::CSecureRuleDlg(CWnd* pParent, CSecureRule* pRule) : CSkinDialog(CSecureRuleDlg::IDD, pParent)
{
	m_nExpireD = 0;
	m_nExpireH = 0;
	m_nExpireM = 0;
	m_nAction = -1;
	m_nExpire = -1;
	m_nType = -1;
	m_nMatch = -1;
	m_pRule	= pRule;
	m_bNew	= FALSE;
}

CSecureRuleDlg::~CSecureRuleDlg()
{
	if ( m_pRule && m_bNew ) delete m_pRule;
}

void CSecureRuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSecureRuleDlg)
	DDX_Control(pDX, IDC_RULE_CONTENT, m_wndContent);
	DDX_Control(pDX, IDC_GROUP_CONTENT, m_wndGroupContent);
	DDX_Control(pDX, IDC_GROUP_NETWORK, m_wndGroupNetwork);
	DDX_Control(pDX, IDC_EXPIRE_M, m_wndExpireM);
	DDX_Control(pDX, IDC_EXPIRE_H, m_wndExpireH);
	DDX_Control(pDX, IDC_EXPIRE_D, m_wndExpireD);
	DDX_Control(pDX, IDC_MASK_4, m_wndMask4);
	DDX_Control(pDX, IDC_MASK_3, m_wndMask3);
	DDX_Control(pDX, IDC_MASK_2, m_wndMask2);
	DDX_Control(pDX, IDC_MASK_1, m_wndMask1);
	DDX_Control(pDX, IDC_IP_4, m_wndIP4);
	DDX_Control(pDX, IDC_IP_3, m_wndIP3);
	DDX_Control(pDX, IDC_IP_2, m_wndIP2);
	DDX_Control(pDX, IDC_IP_1, m_wndIP1);
	DDX_Text(pDX, IDC_EXPIRE_D, m_nExpireD);
	DDX_Text(pDX, IDC_EXPIRE_H, m_nExpireH);
	DDX_Text(pDX, IDC_EXPIRE_M, m_nExpireM);
	DDX_CBIndex(pDX, IDC_RULE_ACTION, m_nAction);
	DDX_CBIndex(pDX, IDC_RULE_EXPIRE, m_nExpire);
	DDX_Text(pDX, IDC_RULE_COMMENT, m_sComment);
	DDX_CBIndex(pDX, IDC_RULE_TYPE, m_nType);
	DDX_Text(pDX, IDC_RULE_CONTENT, m_sContent);
	DDX_Radio(pDX, IDC_RULE_MATCH_0, m_nMatch);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CSecureRuleDlg message handlers

BOOL CSecureRuleDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CSecureRuleDlg"), IDR_SECURITYFRAME );

	if ( ! m_pRule )
	{
		m_bNew = TRUE;
		m_pRule = new CSecureRule();
	}
	
	CEdit* pwIP[4]		= { &m_wndIP1, &m_wndIP2, &m_wndIP3, &m_wndIP4 };
	CEdit* pwMask[4]	= { &m_wndMask1, &m_wndMask2, &m_wndMask3, &m_wndMask4 };
	switch ( m_pRule->m_nType ) 
	{
	case CSecureRule::srAddress:
		m_nType = 0;
		m_nMatch = 0;
		for ( int nByte = 0 ; nByte < 4 ; nByte++ )
		{
			CString strItem;
			strItem.Format( _T("%lu"), m_pRule->m_nMask[ nByte ] );
			pwMask[ nByte ]->SetWindowText( strItem );

			if ( m_pRule->m_nMask[ nByte ] == 0 )
			{
				pwIP[ nByte ]->SetWindowText( _T("*") );
			}
			else
			{
				strItem.Format( _T("%lu"), m_pRule->m_nIP[ nByte ] );
				pwIP[ nByte ]->SetWindowText( strItem );
			}
		}
		break;
	case CSecureRule::srContentAny:
		m_nType = 1;
		m_nMatch = 0;
		m_sContent = m_pRule->GetContentWords();
		break;
	case CSecureRule::srContentAll:
		m_nType = 1;
		m_nMatch = 1;
		m_sContent = m_pRule->GetContentWords();
		break;
	case CSecureRule::srContentRegExp:
		m_nType = 1;
		m_nMatch = 2;
		m_sContent = m_pRule->GetContentWords();
		break;
	}
	m_sComment	= m_pRule->m_sComment;
	m_nAction	= m_pRule->m_nAction;
	m_nExpire	= min( m_pRule->m_nExpire, 2ul );

	if ( m_nExpire == 2 )
	{
		DWORD nTime = m_pRule->m_nExpire - static_cast< DWORD >( time( NULL ) );
		m_nExpireD = nTime / 86400;
		m_nExpireH = ( nTime % 86400 ) / 3600;
		m_nExpireM = ( nTime % 3600 ) / 60;
	}

	UpdateData( FALSE );

	OnSelChangeRuleExpire();
	OnSelChangeRuleType();

	return FALSE;
}

void CSecureRuleDlg::ShowGroup(CWnd* pWnd, BOOL bShow)
{
	while ( pWnd )
	{
		pWnd->ShowWindow( bShow ? SW_SHOW : SW_HIDE );
		pWnd = pWnd->GetNextWindow();

		if ( pWnd->GetStyle() & WS_GROUP )
		{
			if ( pWnd->GetDlgCtrlID() != IDC_RULE_MATCH_0 ) break;
		}
	}
}

void CSecureRuleDlg::OnSelChangeRuleType()
{
	UpdateData();

	ShowGroup( &m_wndGroupNetwork, m_nType == 0 );
	ShowGroup( &m_wndGroupContent, m_nType == 1 );

	if ( m_nType == 0 )
	{
		m_wndIP1.SetFocus();
		m_wndIP1.SetSel( 0, -1 );
	}
	else
	{
		m_wndContent.SetFocus();
	}
}

void CSecureRuleDlg::OnSelChangeRuleExpire()
{
	UpdateData();
	m_wndExpireD.EnableWindow( m_nExpire == 2 );
	m_wndExpireH.EnableWindow( m_nExpire == 2 );
	m_wndExpireM.EnableWindow( m_nExpire == 2 );
}

BOOL CSecureRuleDlg::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_CHAR )
	{
		CEdit* pwIP[4]		= { &m_wndIP1, &m_wndIP2, &m_wndIP3, &m_wndIP4 };
		CEdit* pwMask[4]	= { &m_wndMask1, &m_wndMask2, &m_wndMask3, &m_wndMask4 };
		CWnd* pFocus		= GetFocus();

		for ( int nByte = 0 ; nByte < 4 && pFocus ; nByte++ )
		{
			if ( pFocus == pwIP[ nByte ] || pFocus == pwMask[ nByte ] )
			{
				if ( pMsg->wParam == '.' )
				{
					if ( nByte == 3 ) return TRUE;

					if ( pFocus == pwIP[ nByte ] )
					{
						pwIP[ nByte + 1 ]->SetFocus();
						pwIP[ nByte + 1 ]->SetSel( 0, -1 );
					}
					else
					{
						pwMask[ nByte + 1 ]->SetFocus();
						pwMask[ nByte + 1 ]->SetSel( 0, -1 );
					}

					return TRUE;
				}
				else if ( pMsg->wParam == '*' )
				{
					if ( pFocus != pwIP[ nByte ] ) return TRUE;
					pwIP[ nByte ]->SetWindowText( _T("*") );
					pwMask[ nByte ]->SetWindowText( _T("0") );
					pwIP[ nByte ]->SetSel( 0, 1 );
					pwMask[ nByte ]->SetSel( 0, 1 );
					return TRUE;
				}
				else if ( pMsg->wParam >= 32 && ! _istdigit( (TCHAR)pMsg->wParam ) )
				{
					return TRUE;
				}

				break;
			}
		}
	}

	return CSkinDialog::PreTranslateMessage(pMsg);
}

void CSecureRuleDlg::OnOK()
{
	UpdateData( TRUE );

	if ( m_nType == 0 )
	{
		m_pRule->m_nType = CSecureRule::srAddress;
		CEdit* pwIP[4]		= { &m_wndIP1, &m_wndIP2, &m_wndIP3, &m_wndIP4 };
		CEdit* pwMask[4]	= { &m_wndMask1, &m_wndMask2, &m_wndMask3, &m_wndMask4 };
		for ( int nByte = 0 ; nByte < 4 ; nByte++ )
		{
			CString strItem;
			DWORD nValue = 0;

			pwIP[ nByte ]->GetWindowText( strItem );
			if ( _stscanf( strItem, _T("%lu"), &nValue ) != 1 ) nValue = 0;
			m_pRule->m_nIP[ nByte ] = (BYTE)min( 255ul, nValue );

			pwMask[ nByte ]->GetWindowText( strItem );
			if ( _stscanf( strItem, _T("%lu"), &nValue ) != 1 ) nValue = 0;
			m_pRule->m_nMask[ nByte ] = (BYTE)min( 255ul, nValue );
		}
	}
	else if ( m_nType == 1 )
	{
		switch ( m_nMatch )
		{
		case 0:
			m_pRule->m_nType = CSecureRule::srContentAny;
			m_pRule->SetContentWords( m_sContent );
			break;
		case 1:
			m_pRule->m_nType = CSecureRule::srContentAll;
			m_pRule->SetContentWords( m_sContent );
			break;
		case 2:
			m_pRule->m_nType = CSecureRule::srContentRegExp;
			m_pRule->SetContentWords( m_sContent );
			break;
		}
	}
	m_pRule->m_sComment	= m_sComment;
	m_pRule->m_nAction	= BYTE( m_nAction );
	m_pRule->m_nExpire	= m_nExpire;

	if ( m_nExpire == 2 )
	{
		m_pRule->m_nExpire	= static_cast< DWORD >( time( NULL ) ) + m_nExpireD * 86400
							+ m_nExpireH * 3600 + m_nExpireM * 60;
	}

	Security.Add( m_pRule );
	m_pRule = NULL;

	CSkinDialog::OnOK();
}
