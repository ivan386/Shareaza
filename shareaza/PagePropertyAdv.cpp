//
// PagePropertyAdv.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2006.
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

#include "stdafx.h"
#include "Shareaza.h"
#include "PagePropertyAdv.h"

// CPropertyPageAdv dialog

IMPLEMENT_DYNAMIC(CPropertyPageAdv, CPropertyPage)
CPropertyPageAdv::CPropertyPageAdv(UINT nIDD)
	: CPropertyPage(nIDD)
{
}

CPropertyPageAdv::~CPropertyPageAdv()
{
}

void CPropertyPageAdv::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPropertyPageAdv, CPropertyPage)
END_MESSAGE_MAP()

// CPropertyPageAdv message handlers

BOOL CPropertyPageAdv::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_wndToolTip.Create( this );
	m_wndToolTip.Activate( TRUE );
	m_wndToolTip.SetMaxTipWidth( 200 );

	return TRUE;
}

BOOL CPropertyPageAdv::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST )
	{
		MSG msg;
		CopyMemory( &msg, pMsg, sizeof(MSG) );
		HWND hWndParent = ::GetParent( msg.hwnd );

		while ( hWndParent && hWndParent != m_hWnd )
		{
			msg.hwnd = hWndParent;
			hWndParent = ::GetParent( hWndParent );
		}

		if ( msg.hwnd )
		{
			m_wndToolTip.RelayEvent( &msg );
		}
	}
	return CPropertyPage::PreTranslateMessage(pMsg);
}
