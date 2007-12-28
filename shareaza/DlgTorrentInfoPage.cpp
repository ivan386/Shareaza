//
// DlgTorrentInfoPage.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2006.
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
#include "DlgTorrentInfoSheet.h"
#include "DlgTorrentInfoPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CTorrentInfoPage, CPropertyPageAdv)

BEGIN_MESSAGE_MAP(CTorrentInfoPage, CPropertyPageAdv)
	//{{AFX_MSG_MAP(CTorrentInfoPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTorrentInfoPage property page

CTorrentInfoPage::CTorrentInfoPage(UINT nIDD) : 
	CPropertyPageAdv( nIDD )
{
}

CTorrentInfoPage::~CTorrentInfoPage()
{
}

void CTorrentInfoPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageAdv::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTorrentInfoPage)
	//}}AFX_DATA_MAP
}

CBTInfo* CTorrentInfoPage::GetTorrentInfo()
{
	CTorrentInfoSheet* pSheet = (CTorrentInfoSheet*)GetParent();
	return &pSheet->m_pInfo;
}

Hashes::BtGuid CTorrentInfoPage::GetPeerID()
{
	CTorrentInfoSheet* pSheet = (CTorrentInfoSheet*)GetParent();
	return pSheet->m_pPeerID;
}

/////////////////////////////////////////////////////////////////////////////
// CTorrentInfoPage message handlers

BOOL CTorrentInfoPage::OnInitDialog()
{
	CPropertyPageAdv::OnInitDialog();

	m_pInfo = GetTorrentInfo();
	m_pPeerID = GetPeerID();
	
	return TRUE;
}

