//
// CtrlLibraryPanel.cpp
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
#include "Library.h"
#include "CtrlLibraryFrame.h"
#include "CtrlLibraryPanel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CLibraryPanel, CWnd)

BEGIN_MESSAGE_MAP(CLibraryPanel, CWnd)
	//{{AFX_MSG_MAP(CLibraryPanel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLibraryPanel construction

CLibraryPanel::CLibraryPanel()
{
	m_bAvailable = FALSE;
}

CLibraryPanel::~CLibraryPanel()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryPanel operations

BOOL CLibraryPanel::Create(CWnd* pParentWnd) 
{
	CRect rect;
	return CWnd::Create( NULL, NULL, WS_CHILD, rect, pParentWnd, IDC_LIBRARY_PANEL, NULL );
}

BOOL CLibraryPanel::CheckAvailable(CLibraryTreeItem* pFolders, CLibraryList* pObjects)
{
	return ( m_bAvailable = FALSE );
}

void CLibraryPanel::Update()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryPanel helper operations

CLibraryTreeItem* CLibraryPanel::GetFolderSelection()
{
	CLibraryFrame* pFrame = (CLibraryFrame*)GetOwner();
	ASSERT_KINDOF(CLibraryFrame, pFrame);
	return pFrame->GetFolderSelection();
}

CLibraryList* CLibraryPanel::GetViewSelection()
{
	CLibraryFrame* pFrame = (CLibraryFrame*)GetOwner();
	ASSERT_KINDOF(CLibraryFrame, pFrame);
	return pFrame->GetViewSelection();
}

