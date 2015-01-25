//
// DlgHex.cpp
//
// Copyright (c) Shareaza Development Team, 2014.
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

#include "stdafx.h"
#include "Shareaza.h"
#include "DlgHex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CHexDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CHexDlg, CSkinDialog)
END_MESSAGE_MAP()

CHexDlg::CHexDlg(CWnd* pParent /*=NULL*/)
	: CSkinDialog	( CHexDlg::IDD, pParent )
{
}

CHexDlg::~CHexDlg()
{
}

void CHexDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_HEX, m_sHex);
}

BOOL CHexDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CHexDlg"), IDR_MAINFRAME );

	return TRUE;
}

void CHexDlg::OnOK()
{
	UpdateData();

	m_pBuffer.Clear();

	m_sHex.Remove( _T(' ') );
	m_sHex.Remove( _T('\t') );
	m_sHex.Remove( _T('\r') );
	m_sHex.Remove( _T('\n') );

	if ( int length = m_sHex.GetLength() )
	{
		if ( ( length & 1 ) != 0 )
		{
			AfxMessageBox( _T("String has wrong even length!") );
			return;
		}

		m_pBuffer.EnsureBuffer( length / 2 );

		for ( int i = 0; i < length; ++i )
		{
			BYTE b = 0;
			TCHAR c = m_sHex.GetAt( i );
			if ( c >= '0' && c <= '9' )
				b = BYTE( ( c - '0' ) << 4 );
			else if ( c >= 'A' && c <= 'F' )
				b = BYTE( ( c - 'A' + 10 ) << 4 );
			else if ( c >= 'a' && c <= 'f' )
				b = BYTE( ( c - 'a' + 10 ) << 4 );
			else
			{
				CString sMsg;
				sMsg.Format( _T("Found wrong symbol at position: %d"), i );
				AfxMessageBox( sMsg );
				return;
			}
			++i;
			c = m_sHex.GetAt( i );
			if ( c >= '0' && c <= '9' )
				b |= BYTE( c - '0' );
			else if ( c >= 'A' && c <= 'F' )
				b |= BYTE( c - 'A' + 10 );
			else if ( c >= 'a' && c <= 'f' )
				b |= BYTE( c - 'a' + 10 );
			else
			{
				CString sMsg;
				sMsg.Format( _T("Found wrong symbol at position: %d"), i );
				AfxMessageBox( sMsg );
				return;
			}

			m_pBuffer.Add( &b, 1 );
		}
	}
	else
	{
		AfxMessageBox( _T("Empty string!") );
		return;
	}

	CSkinDialog::OnOK();
}
