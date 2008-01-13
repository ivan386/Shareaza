//
// CtrlIRCPanel.cpp
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
//
// Author: peer_l_@hotmail.com
//

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "CtrlIRCPanel.h"
#include "DlgIrcInput.h"

#include "CoolInterface.h"
#include "ShellIcons.h"
#include "RichDocument.h"
#include "Skin.h"
#include "XML.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CIRCPanel, CTaskPanel)
BEGIN_MESSAGE_MAP(CIRCPanel, CTaskPanel)
	//{{AFX_MSG_MAP(CIRCPanel)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CIRCUsersBox, CRichTaskBox)
BEGIN_MESSAGE_MAP(CIRCUsersBox, CRichTaskBox)
	//{{AFX_MSG_MAP(CIRCUsersBox)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_CONTEXTMENU()
	ON_LBN_DBLCLK(IDC_IRC_USERS, OnUsersDoubleClick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


IMPLEMENT_DYNAMIC(CIRCChannelsBox, CRichTaskBox)
BEGIN_MESSAGE_MAP(CIRCChannelsBox, CRichTaskBox)
	//{{AFX_MSG_MAP(CIRCChannelsBox)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_NOTIFY(NM_DBLCLK, IDC_IRC_CHANNELS, OnChansDoubleClick)
	ON_COMMAND(IDC_IRC_ADDCHANNEL, OnAddChannel)
	ON_COMMAND(IDC_IRC_REMOVECHANNEL, OnRemoveChannel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIRCPanel construction

CIRCPanel::CIRCPanel()
{
}

CIRCPanel::~CIRCPanel()
{
	m_pFont.Detach();
}

/////////////////////////////////////////////////////////////////////////////
// CIRCPanel message handlers

BOOL CIRCPanel::Create(CWnd* pParentWnd)
{
	CRect rect( 0, 0, PANEL_WIDTH, 0 );
	return CTaskPanel::Create( _T("CIRCPanel"), WS_VISIBLE, rect, pParentWnd, IDC_IRC_PANEL );
}

int CIRCPanel::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CTaskPanel::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	m_boxUsers.Create( this, _T("Users"), IDR_USERSFRAME );
	m_boxChans.Create( this, _T("Channels"), IDR_CHANSFRAME );
	m_pFont.Attach( theApp.m_gdiFontBold );
	m_boxUsers.m_wndUserList.SetFont( &m_pFont );
	m_boxChans.m_wndChanList.SetFont( &m_pFont );

	AddBox( &m_boxUsers );
	AddBox( &m_boxChans );
	
	return 0;
}

void CIRCPanel::Setup()
{
	SetWatermark( Skin.GetWatermark( _T("CIRCPanel") ) );
	SetFooter( Skin.GetWatermark( _T("CIRCPanel.Footer") ), TRUE );
	
	m_boxUsers.Setup();
	m_boxChans.Setup();
	
	Invalidate();
}

void CIRCPanel::OnSize(UINT nType, int cx, int cy)
{
	CTaskPanel::OnSize( nType, cx, cy );

	CRect rcClient;
	GetOwner()->GetClientRect( &rcClient );
	int nBoxWidth, nBoxChansHeight;
	nBoxWidth = PANEL_WIDTH - BOX_HOFFSET;
	nBoxChansHeight = rcClient.Height() - BOXUSERS_HEIGHT - BUTTON_HEIGHT - 1 - PANELOFFSET_HEIGHT;
	if ( nBoxChansHeight < BOXCHANS_MINHEIGHT ) nBoxChansHeight = BOXCHANS_MINHEIGHT;
	m_boxChans.m_wndChanList.SetWindowPos( NULL, BOX_HOFFSET, BOX_VOFFSET, nBoxWidth - BOX_HOFFSET * 4, 
		nBoxChansHeight - BOX_VOFFSET * 2, SWP_NOZORDER | SWP_SHOWWINDOW );
	m_boxChans.SetSize( nBoxChansHeight + BUTTON_HEIGHT );
}

/////////////////////////////////////////////////////////////////////////////
// CIRCUsersBox construction

CIRCUsersBox::CIRCUsersBox()
{
	SetPrimary();
}

CIRCUsersBox::~CIRCUsersBox()
{
}

/////////////////////////////////////////////////////////////////////////////
// CIRCUsersBox message handlers

int CIRCUsersBox::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CRichTaskBox::OnCreate( lpCreateStruct ) == -1 ) return -1;
	CRect rectDefault;
	m_wndUserList.Create(WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_NOTIFY, rectDefault, this, IDC_IRC_USERS);
	if ( Settings.General.LanguageRTL ) m_wndUserList.ModifyStyleEx(WS_EX_LAYOUTRTL,0,0);
	m_wndUserList.ModifyStyleEx( 0, WS_EX_CLIENTEDGE );
	
	m_hHand = theApp.LoadCursor( IDC_HAND );
	
	return 0;
}

void CIRCUsersBox::Setup()
{
	if ( m_pDocument ) delete m_pDocument;

	m_pDocument = NULL;
		
	SetCaptionmark( Skin.GetWatermark( _T("CIRCUsersBox.Caption") ) );
	
	CXMLElement* pXML = Skin.GetDocument( _T("CIRCUsersBox") );
	if ( pXML == NULL ) return;

	m_sCaption = pXML->GetAttributeValue( _T("title"), _T("Users") );

	SetCaption( m_sCaption );
	
	m_pDocument = new CRichDocument();
	
	CMap<CString, const CString &, CRichElement *, CRichElement *> pMap;
	if ( ! m_pDocument->LoadXML( pXML, &pMap ) ) return;
}

void CIRCUsersBox::UpdateCaptionCount()
{
	CString strCaption;
	strCaption.Format( _T("%d"), m_wndUserList.GetCount() );
	strCaption += _T(")");
	strCaption = _T(" (") + strCaption;
	SetCaption( m_sCaption + strCaption );		
}

void CIRCUsersBox::OnSize(UINT nType, int cx, int cy) 
{
	CTaskBox::OnSize( nType, cx, cy );
	CRect rcBox;
	m_wndUserList.SetWindowPos( NULL, BOX_HOFFSET, BOX_VOFFSET, PANEL_WIDTH - BOX_HOFFSET - BOX_HOFFSET * 4, 
		BOXUSERS_HEIGHT - BOX_VOFFSET * 2, SWP_NOZORDER | SWP_SHOWWINDOW );
 	m_wndUserList.GetClientRect(&rcBox);
	SetSize( rcBox.Height() + BOX_VOFFSET * 2 + 4 );
} 

void CIRCUsersBox::OnPaint() 
{
	CRect rcClient, rcIcon, rcText;
	CPaintDC dc( this );
	
	GetClientRect( &rcClient );
	m_wndView.GetClientRect( &rcIcon );
	rcClient.bottom -= rcIcon.Height();
	rcClient.top += 6;
	
	rcIcon.SetRect( 4, rcClient.top, 4 + 20, rcClient.top + 16 );
	rcText.SetRect( rcIcon.right, rcIcon.top, rcClient.right - 4, rcIcon.bottom );
	rcIcon.DeflateRect( 0, 2 );

	dc.SetBkMode( OPAQUE );
	dc.SetBkColor( CoolInterface.m_crWindow );
	dc.SetTextColor( RGB( 0, 0, 255 ) );
	
	rcClient.top = 0;
	dc.FillSolidRect( &rcClient, CoolInterface.m_crWindow );

	CRect rcClient2;

	GetClientRect( &rcClient2 );
	if ( rcClient2.IsRectEmpty() ) return;
	if ( m_bmBuffer.m_hObject != NULL )
	{
		m_dcBuffer.SelectObject( m_hBuffer );
		m_dcBuffer.DeleteDC();
		m_bmBuffer.DeleteObject();
	}
	m_bmBuffer.CreateCompatibleBitmap( &dc, rcClient2.Width(), rcClient2.Height() );
	m_dcBuffer.CreateCompatibleDC( &dc );
	m_hBuffer = (HBITMAP)m_dcBuffer.SelectObject( &m_bmBuffer )->m_hObject;
	if ( ! CoolInterface.DrawWatermark( &m_dcBuffer, &rcClient2, &m_bmWatermark, 0, 0 ) )
	{
		m_dcBuffer.FillSolidRect( &rcClient, Skin.m_crBannerBack );
	}
	dc.BitBlt( rcClient.left, rcClient.top, rcClient.Width(),
		rcClient.Height(), &m_dcBuffer, 0, 0, SRCCOPY );
}

void CIRCUsersBox::OnUsersDoubleClick() 
{
	IRC_PANELEVENT pNotify;
	pNotify.hdr.hwndFrom	= GetSafeHwnd();
	pNotify.hdr.idFrom		= IDC_IRC_DBLCLKUSERS;
	pNotify.hdr.code		= NM_DBLCLK;
	CWnd* m_wndFrame = GetOwner()->GetOwner();
	m_wndFrame->PostMessage( WM_NOTIFY, pNotify.hdr.idFrom, (LPARAM)&pNotify );
}

void CIRCUsersBox::OnContextMenu(CWnd* /* pWnd */, CPoint /* point */) 
{
	IRC_PANELEVENT pNotify;
	pNotify.hdr.hwndFrom	= GetSafeHwnd();
	pNotify.hdr.idFrom		= IDC_IRC_MENUUSERS;
	pNotify.hdr.code		= NM_DBLCLK;
	CWnd* m_wndFrame = GetOwner()->GetOwner();
	m_wndFrame->PostMessage( WM_NOTIFY, pNotify.hdr.idFrom, (LPARAM)&pNotify );
}
/////////////////////////////////////////////////////////////////////////////
// CIRCChannelsBox construction

CIRCChannelsBox::CIRCChannelsBox()
{
	SetPrimary();
}

CIRCChannelsBox::~CIRCChannelsBox()
{
}

/////////////////////////////////////////////////////////////////////////////
// CIRCUsersBox message handlers

int CIRCChannelsBox::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CRichTaskBox::OnCreate( lpCreateStruct ) == -1 ) return -1;
	CRect rectDefault;
	SetOwner( GetParent() );
	DWORD dwStyle = WS_CHILD | WS_VSCROLL | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_REPORT |
		WS_VISIBLE | LVS_NOCOLUMNHEADER | LVS_SORTASCENDING | LVS_NOLABELWRAP;
 	m_wndChanList.Create( dwStyle, rectDefault, this, IDC_IRC_CHANNELS );
	m_wndChanList.ModifyStyleEx( 0, WS_EX_CLIENTEDGE );

	CRect rc( 0, 0, 0, 0 );
	m_wndAddChannel.Create( rc, this, IDC_IRC_ADDCHANNEL );
	//LoadString( strCaption, IDS_IRC_ADDCHANNEL );
	m_wndAddChannel.SetWindowText( _T(" Add ") );
	m_wndAddChannel.SetIcon( CoolInterface.ExtractIcon( ID_IRC_ADD, Settings.General.LanguageRTL ) );
	m_wndAddChannel.SetHandCursor( TRUE );

	m_wndRemoveChannel.Create( rc, this, IDC_IRC_REMOVECHANNEL );
	//LoadString( strCaption, IDS_IRC_REMOVECHANNEL );
	m_wndRemoveChannel.SetWindowText( _T(" Remove ") );
	m_wndRemoveChannel.SetIcon( CoolInterface.ExtractIcon( ID_IRC_REMOVE, Settings.General.LanguageRTL ) );
	m_wndAddChannel.SetHandCursor( TRUE );

	m_hHand = theApp.LoadCursor( IDC_HAND );
	
	return 0;
}

void CIRCChannelsBox::Setup()
{
	if ( m_pDocument ) delete m_pDocument;

	m_pDocument = NULL;
		
	SetCaptionmark( Skin.GetWatermark( _T("CIRCChannelsBox.Caption") ) );
	
	CXMLElement* pXML = Skin.GetDocument( _T("CIRCChannelsBox") );
	if ( pXML == NULL ) return;
	
	SetCaption( pXML->GetAttributeValue( _T("title"), _T("Channels") ) );
	
	m_pDocument = new CRichDocument();
	
	CMap<CString, const CString&, CRichElement *, CRichElement *> pMap;
	if ( ! m_pDocument->LoadXML( pXML, &pMap ) ) return;
	
	GetView().SetDocument( m_pDocument );

	CRect rcChanList;
	m_wndChanList.GetClientRect(&rcChanList);
	int nScrollbarWidth = 17;
	int nUserCountWidth = 30;
	int nChanCountWidth = rcChanList.Width() - nUserCountWidth - nScrollbarWidth;
	m_wndChanList.InsertColumn( 0, _T("Channels"), LVCFMT_LEFT, nChanCountWidth, -1 );
	m_wndChanList.InsertColumn( 1, _T("UserCount"), LVCFMT_RIGHT, nUserCountWidth, -1 );
}

void CIRCChannelsBox::OnSize(UINT nType, int cx, int cy) 
{
	CRichTaskBox::OnSize( nType, cx, cy );

	CRect rcClient;
	GetOwner()->GetClientRect( &rcClient );
	int nBoxWidth = PANEL_WIDTH - BOX_HOFFSET;
	int nBoxChansHeight = rcClient.Height() - BOXUSERS_HEIGHT - BUTTON_HEIGHT - 1 - PANELOFFSET_HEIGHT;
	if ( nBoxChansHeight < BOXCHANS_MINHEIGHT ) nBoxChansHeight = BOXCHANS_MINHEIGHT;

	HDWP hDWP = BeginDeferWindowPos( 2 );

	DeferWindowPos( hDWP, m_wndAddChannel, NULL, BOX_HOFFSET, nBoxChansHeight - BOX_VOFFSET + 1, 
		(nBoxWidth - BOX_HOFFSET * 4) / 2 - 2, BUTTON_HEIGHT, SWP_SHOWWINDOW | SWP_NOZORDER );
	DeferWindowPos( hDWP, m_wndRemoveChannel, NULL, BOX_HOFFSET + (nBoxWidth - BOX_HOFFSET * 4) / 2 + 2, 
		nBoxChansHeight - BOX_VOFFSET + 1, (nBoxWidth - BOX_HOFFSET * 4) / 2 - 2, 
		BUTTON_HEIGHT, SWP_SHOWWINDOW | SWP_NOZORDER );
	
	EndDeferWindowPos( hDWP );
}

void CIRCChannelsBox::OnPaint() 
{
	CRect rcClient, rcIcon, rcText;
	CPaintDC dc( this );
	GetClientRect( &rcClient );
	m_wndView.GetClientRect( &rcIcon );
	rcClient.bottom -= rcIcon.Height();
	rcClient.top += 6;
	
	rcIcon.SetRect( 4, rcClient.top, 4 + 20, rcClient.top + 16 );
	rcText.SetRect( rcIcon.right, rcIcon.top, rcClient.right - 4, rcIcon.bottom );
	rcIcon.DeflateRect( 0, 2 );

	dc.SetBkMode( OPAQUE );
	dc.SetBkColor( CoolInterface.m_crWindow );
	dc.SetTextColor( RGB( 0, 0, 255 ) );
	
	rcClient.top = 0;
	dc.FillSolidRect( &rcClient, CoolInterface.m_crWindow );

	CRect rcClient2;

	GetClientRect( &rcClient2 );
	if ( rcClient2.IsRectEmpty() ) return;
	if ( m_bmBuffer.m_hObject != NULL )
	{
		m_dcBuffer.SelectObject( m_hBuffer );
		m_dcBuffer.DeleteDC();
		m_bmBuffer.DeleteObject();
	}
	m_bmBuffer.CreateCompatibleBitmap( &dc, rcClient2.Width(), rcClient2.Height() );
	m_dcBuffer.CreateCompatibleDC( &dc );
	m_hBuffer = (HBITMAP)m_dcBuffer.SelectObject( &m_bmBuffer )->m_hObject;
	if ( ! CoolInterface.DrawWatermark( &m_dcBuffer, &rcClient2, &m_bmWatermark, 0, 0 ) )
	{
		m_dcBuffer.FillSolidRect( &rcClient, Skin.m_crBannerBack );
	}
	dc.BitBlt( rcClient.left, rcClient.top, rcClient.Width(),
		rcClient.Height(), &m_dcBuffer, 0, 0, SRCCOPY );
}

void CIRCChannelsBox::OnChansDoubleClick(NMHDR* /* pNMHDR */, LRESULT* pResult) 
{
	IRC_PANELEVENT pNotify;
	pNotify.hdr.hwndFrom	= GetSafeHwnd();
	pNotify.hdr.idFrom		= IDC_IRC_DBLCLKCHANNELS;
	pNotify.hdr.code		= NM_DBLCLK;
	CWnd* m_wndFrame = GetOwner()->GetOwner();
	m_wndFrame->PostMessage( WM_NOTIFY, pNotify.hdr.idFrom, (LPARAM)&pNotify );
	*pResult = 0;
}

void CIRCChannelsBox::OnAddChannel()
{
	CIrcInputDlg dlg( this, 0, FALSE );	// 0 = select the first caption

	if ( dlg.DoModal() != IDOK ) return;
	
	CString strChannel = dlg.m_sAnswer;
	if ( ! strChannel.IsEmpty() )
	{
		if ( strChannel.GetAt( 0 ) != '#' ) strChannel = '#' + strChannel;
		for ( int nChannel=0 ; nChannel < m_wndChanList.GetItemCount() ; nChannel++ )
		{
			if ( strChannel.CompareNoCase( m_wndChanList.GetItemText( nChannel, 0 ) ) == 0 )
			{
				MessageBox( strChannel, _T("Channel already is in the list!"), MB_OK );
				return;
			}
		}
		if ( strChannel.GetAt( 0 ) == '#' ) strChannel = strChannel.Mid( 1 );
		strChannel = strChannel.Left( 1 ).MakeUpper() + strChannel.Mid( 1 );
		m_wndChanList.InsertItem( -1, strChannel );
		m_sPassedChannel = strChannel;
		GetOwner()->GetOwner()->PostMessage( WM_ADDCHANNEL, IDC_IRC_CHANNELS );
	}
}

void CIRCChannelsBox::OnRemoveChannel()
{
	GetOwner()->GetOwner()->PostMessage( WM_REMOVECHANNEL, IDC_IRC_CHANNELS );
}
