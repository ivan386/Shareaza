//
// PageProfileAvatar.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "GProfile.h"
#include "ImageFile.h"
#include "PageProfileAvatar.h"
#include "CoolInterface.h"
#include "SchemaCache.h"
#include "XML.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CAvatarProfilePage, CSettingsPage)

BEGIN_MESSAGE_MAP(CAvatarProfilePage, CSettingsPage)
	//{{AFX_MSG_MAP(CAvatarProfilePage)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_AVATAR_BROWSE, OnAvatarBrowse)
	ON_BN_CLICKED(IDC_AVATAR_REMOVE, OnAvatarRemove)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAvatarProfilePage property page

CAvatarProfilePage::CAvatarProfilePage() : CSettingsPage( CAvatarProfilePage::IDD )
{
	//{{AFX_DATA_INIT(CAvatarProfilePage)
	//}}AFX_DATA_INIT
}

CAvatarProfilePage::~CAvatarProfilePage()
{
}

void CAvatarProfilePage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAvatarProfilePage)
	DDX_Control(pDX, IDC_AVATAR_REMOVE, m_wndRemove);
	DDX_Control(pDX, IDC_PREVIEW, m_wndPreview);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CAvatarProfilePage message handlers

BOOL CAvatarProfilePage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	if ( CXMLElement* pAvatar = MyProfile.GetXML( _T("avatar") ) )
	{
		m_sAvatar = pAvatar->GetAttributeValue( _T("path") );
		PrepareImage();
	}

	return TRUE;
}

void CAvatarProfilePage::OnOK()
{
	if ( CXMLElement* pAvatar = MyProfile.GetXML( _T("avatar"), TRUE ) )
	{
		pAvatar->AddAttribute( _T("path"), m_sAvatar );
	}

	CSettingsPage::OnOK();
}

void CAvatarProfilePage::OnPaint()
{
	CPaintDC dc( this );
	CRect rc;

	m_wndPreview.GetWindowRect( &rc );
	ScreenToClient( &rc );

	rc.right = rc.left + 128;
	rc.bottom = rc.top + 128;

	if ( m_bmAvatar.m_hObject != NULL )
	{
		CDC dcMem;
		dcMem.CreateCompatibleDC( &dc );
		CBitmap* pOld = (CBitmap*)dcMem.SelectObject( &m_bmAvatar );
		dc.BitBlt( rc.left, rc.top, rc.Width(), rc.Height(), &dcMem, 0, 0, SRCCOPY );
		dcMem.SelectObject( pOld );
	}
	else
	{
		rc.InflateRect( 1, 1 );
		dc.Draw3dRect( &rc, CoolInterface.m_crSysActiveCaption, CoolInterface.m_crSysActiveCaption );
	}
}

void CAvatarProfilePage::OnAvatarBrowse()
{
	CFileDialog dlg( TRUE, _T("png"), m_sAvatar, OFN_HIDEREADONLY,
		SchemaCache.GetFilter( CSchema::uriImageAll ) +
		SchemaCache.GetFilter( CSchema::uriAllFiles ) +
		_T("|"), this );

	if ( dlg.DoModal() == IDOK )
	{
		m_sAvatar = dlg.GetPathName();
		PrepareImage();
		Invalidate();
	}
}

void CAvatarProfilePage::OnAvatarRemove()
{
	m_sAvatar.Empty();
	if ( m_bmAvatar.m_hObject != NULL ) m_bmAvatar.DeleteObject();
	Invalidate();
}

void CAvatarProfilePage::PrepareImage()
{
	if ( m_bmAvatar.m_hObject != NULL ) m_bmAvatar.DeleteObject();
	if ( m_sAvatar.IsEmpty() ) return;

	CImageFile pFile;

	CClientDC dc( this );
	SendMessage( WM_CTLCOLORSTATIC, (WPARAM)dc.GetSafeHdc(), (LPARAM)m_wndPreview.GetSafeHwnd() );

	if ( pFile.LoadFromFile( m_sAvatar ) && pFile.EnsureRGB( dc.GetBkColor() ) )
	{
		pFile.Resample( 128, 128 );
		m_bmAvatar.Attach( pFile.CreateBitmap() );
	}
}
