//
// PageSettingsIRC.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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
// Author: peer_l_@hotmail.com
//

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "PageSettingsIRC.h"
#include "CtrlIRCFrame.h"
#include "GProfile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CIRCSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CIRCSettingsPage, CSettingsPage)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_IRC_COLOR_SERVER, OnBnClickedIrcColorServer)
	ON_BN_CLICKED(IDC_IRC_COLOR_TOPIC, OnBnClickedIrcColorTopic)
	ON_BN_CLICKED(IDC_IRC_COLOR_ACTION, OnBnClickedIrcColorAction)
	ON_BN_CLICKED(IDC_IRC_COLOR_NOTICE, OnBnClickedIrcColorNotice)
	ON_BN_CLICKED(IDC_IRC_COLOR_BG, OnBnClickedIrcColorBg)
	ON_BN_CLICKED(IDC_IRC_COLOR_TEXT, OnBnClickedIrcColorText)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMediaSettingsPage property page

CIRCSettingsPage::CIRCSettingsPage() : CSettingsPage(CIRCSettingsPage::IDD)
{
}

CIRCSettingsPage::~CIRCSettingsPage()
{
}

void CIRCSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IRC_COLOR_SERVER, m_wndColorServer);
	DDX_Control(pDX, IDC_IRC_COLOR_TOPIC, m_wndColorTopic);
	DDX_Control(pDX, IDC_IRC_COLOR_ACTION, m_wndColorAction);
	DDX_Control(pDX, IDC_IRC_COLOR_NOTICE, m_wndColorNotice);
	DDX_Control(pDX, IDC_IRC_COLOR_BG, m_wndColorBg);
	DDX_Control(pDX, IDC_IRC_COLOR_TEXT, m_wndColorText);
	DDX_Check(pDX, IDC_IRC_SHOW, m_bShow);
	DDX_Check(pDX, IDC_IRC_FLOODENABLE, m_bFloodEnable);
	DDX_Check(pDX, IDC_IRC_TIMESTAMP, m_bTimestamp);
	DDX_Text(pDX, IDC_IRC_NICK, m_sNick);
	DDX_Text(pDX, IDC_IRC_ALTERNATE, m_sAlternate);
	DDX_Text(pDX, IDC_IRC_FLOODLIMIT, m_sFloodLimit);
	DDX_Text(pDX, IDC_IRC_SERVERNAME, m_sServerName);
	DDX_Text(pDX, IDC_IRC_SERVERPORT, m_sServerPort);
	DDX_Text(pDX, IDC_IRC_REALNAME, m_sRealName);
	DDX_Text(pDX, IDC_IRC_USERNAME, m_sUserName);
	DDX_FontCombo(pDX, IDC_IRC_TEXTFONT, m_sScreenFont);
}

/////////////////////////////////////////////////////////////////////////////
// CMediaSettingsPage message handlers

BOOL CIRCSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();
	m_wndColorServer.SetButtonStyle( BS_OWNERDRAW | BS_PUSHBUTTON, 1 );
	m_wndColorAction.SetButtonStyle( BS_OWNERDRAW | BS_PUSHBUTTON, 1 );
	m_wndColorNotice.SetButtonStyle( BS_OWNERDRAW | BS_PUSHBUTTON, 1 );
	m_wndColorTopic.SetButtonStyle( BS_OWNERDRAW | BS_PUSHBUTTON, 1 );
	m_wndColorBg.SetButtonStyle( BS_OWNERDRAW | BS_PUSHBUTTON, 1 );
	m_wndColorText.SetButtonStyle( BS_OWNERDRAW | BS_PUSHBUTTON, 1 );
	m_bShow	= Settings.IRC.Show == true;
	m_bTimestamp = Settings.IRC.Timestamp == true;
	m_bFloodEnable = Settings.IRC.FloodEnable == true;
	m_sFloodLimit = Settings.IRC.FloodLimit;

	CString strNick = MyProfile.GetNick();
	if ( Settings.IRC.Nick.IsEmpty() && strNick.GetLength() )
	{
		Settings.IRC.Nick = m_sNick = strNick;
	}
	else
	{
		m_sNick = Settings.IRC.Nick;
	}

	m_sAlternate = Settings.IRC.Alternate;
	m_sServerName = Settings.IRC.ServerName;
	m_sServerPort = Settings.IRC.ServerPort;
	m_sRealName = Settings.IRC.RealName;
	m_sUserName = Settings.IRC.UserName;
	m_sScreenFont = Settings.IRC.ScreenFont;
	m_wndFonts.SubclassDlgItem( IDC_IRC_TEXTFONT, this );

	UpdateData( FALSE );

	return TRUE;
}

void CIRCSettingsPage::OnDrawItem(int /* nIDCtl */, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
   UINT uStyle = DFCS_BUTTONPUSH;
   if ( lpDrawItemStruct->CtlType == ODT_COMBOBOX )
   {
	   if ( lpDrawItemStruct->CtlID == (UINT)IDC_IRC_TEXTFONT )
		   m_wndFonts.SendMessage( OCM_DRAWITEM, 0, (LPARAM)lpDrawItemStruct );
   }
   else if ( lpDrawItemStruct->CtlType == ODT_BUTTON )
   {
		if ( lpDrawItemStruct->itemState & ODS_SELECTED )
			uStyle |= DFCS_PUSHED;
		DrawFrameControl( lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem,
			DFC_BUTTON, uStyle);
		COLORREF MsgColor = RGB(100,100,100);
		int nID = lpDrawItemStruct->CtlID;
		if ( nID == IDC_IRC_COLOR_SERVER )
			MsgColor = Settings.IRC.Colors[ ID_COLOR_SERVERMSG ];
		if ( nID == IDC_IRC_COLOR_TOPIC )
			MsgColor = Settings.IRC.Colors[ ID_COLOR_TOPIC ];
		if ( nID == IDC_IRC_COLOR_ACTION )
			MsgColor = Settings.IRC.Colors[ ID_COLOR_CHANNELACTION ];
		if ( nID == IDC_IRC_COLOR_NOTICE )
			MsgColor = Settings.IRC.Colors[ ID_COLOR_NOTICE ];
		if ( nID == IDC_IRC_COLOR_BG )
			MsgColor = Settings.IRC.Colors[ ID_COLOR_CHATBACK ];
		if ( nID == IDC_IRC_COLOR_TEXT )
			MsgColor = Settings.IRC.Colors[ ID_COLOR_TEXT ];
		SetTextColor( lpDrawItemStruct->hDC, MsgColor );
		SetBkMode( lpDrawItemStruct->hDC, OPAQUE );
		SetBkColor( lpDrawItemStruct->hDC, MsgColor );
		DrawText( lpDrawItemStruct->hDC, _T("W"), 1, 
			&lpDrawItemStruct->rcItem, DT_SINGLELINE|DT_VCENTER|DT_CENTER|DT_NOCLIP );
   }
}

void CIRCSettingsPage::OnBnClickedIrcColorServer()
{
	CColorDialog colorDlg( Settings.IRC.Colors[ID_COLOR_SERVERMSG ], CC_FULLOPEN );
	if ( colorDlg.DoModal() != IDOK ) return;
    Settings.IRC.Colors[ ID_COLOR_SERVERMSG ] = colorDlg.GetColor(); 
	ReleaseCapture();
	Invalidate();
	UpdateWindow();
	UpdateData( FALSE );
}

void CIRCSettingsPage::OnBnClickedIrcColorTopic()
{
	CColorDialog colorDlg( Settings.IRC.Colors[ ID_COLOR_TOPIC ], CC_FULLOPEN );
	if ( colorDlg.DoModal() != IDOK ) return;
    Settings.IRC.Colors[ ID_COLOR_TOPIC ] = colorDlg.GetColor(); 
	ReleaseCapture();
	Invalidate();
	UpdateWindow();
	UpdateData( FALSE );
}

void CIRCSettingsPage::OnBnClickedIrcColorText()
{
	CColorDialog colorDlg( Settings.IRC.Colors[ ID_COLOR_TEXT ], 
		CC_FULLOPEN | CC_SOLIDCOLOR | CC_RGBINIT );
	if ( colorDlg.DoModal() != IDOK ) return;
    Settings.IRC.Colors[ ID_COLOR_TEXT ] = colorDlg.GetColor();
	ReleaseCapture();
	Invalidate();
	UpdateWindow();
	UpdateData( FALSE );
}

void CIRCSettingsPage::OnBnClickedIrcColorAction()
{
	CColorDialog colorDlg( Settings.IRC.Colors[ ID_COLOR_CHANNELACTION], CC_FULLOPEN );
	if ( colorDlg.DoModal() != IDOK ) return;
    Settings.IRC.Colors[ ID_COLOR_CHANNELACTION ] = colorDlg.GetColor(); 
	ReleaseCapture();
	Invalidate();
	UpdateWindow();
	UpdateData( FALSE );
}

void CIRCSettingsPage::OnBnClickedIrcColorNotice()
{
	CColorDialog colorDlg( Settings.IRC.Colors[ID_COLOR_NOTICE], CC_FULLOPEN );
	if ( colorDlg.DoModal() != IDOK ) return;
    Settings.IRC.Colors[ ID_COLOR_NOTICE ] = colorDlg.GetColor(); 
	ReleaseCapture();
	Invalidate();
	UpdateWindow();
	UpdateData( FALSE );
}

void CIRCSettingsPage::OnBnClickedIrcColorBg()
{
	CColorDialog colorDlg( Settings.IRC.Colors[ ID_COLOR_CHATBACK ], CC_FULLOPEN );
	if ( colorDlg.DoModal() != IDOK ) return;
    Settings.IRC.Colors[ ID_COLOR_CHATBACK ] = colorDlg.GetColor(); 
	ReleaseCapture();
	Invalidate();
	UpdateWindow();
	UpdateData( FALSE );
}

void CIRCSettingsPage::OnOK()
{
	Settings.IRC.Show		 = m_bShow == TRUE;
	Settings.IRC.FloodEnable = m_bFloodEnable == TRUE;
	Settings.IRC.FloodLimit	 = m_sFloodLimit;

	CString strNick = MyProfile.GetNick();
	if ( m_sNick.IsEmpty() && strNick.GetLength() )
		m_sNick = Settings.IRC.Nick = strNick;
	else
		Settings.IRC.Nick = m_sNick;
	
	Settings.IRC.Alternate	 = m_sAlternate;
	Settings.IRC.ServerName	 = m_sServerName;
	Settings.IRC.ServerPort	 = m_sServerPort;
	Settings.IRC.RealName	 = m_sRealName;
	Settings.IRC.UserName	 = m_sUserName;
	Settings.IRC.Timestamp	 = m_bTimestamp == TRUE;
	Settings.IRC.ScreenFont	 = m_sScreenFont;
	UpdateData( FALSE );
	
	m_wndFonts.Invalidate();

	if ( CIRCFrame::g_pIrcFrame )
	{
		CIRCFrame::g_pIrcFrame->OnSkinChange();
	}

	CSettingsPage::OnOK();
}
