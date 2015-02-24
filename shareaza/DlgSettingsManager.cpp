//
// DlgSettingsManager.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015
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
#include "Skin.h"
#include "DlgSettingsManager.h"
#include "PageSettingsRich.h"
#include "PageSettingsGeneral.h"
#include "PageSettingsLibrary.h"
#include "PageSettingsMedia.h"
#include "PageSettingsCommunity.h"
#include "PageSettingsIRC.h"
#include "PageSettingsWeb.h"
#include "PageSettingsConnection.h"
#include "PageSettingsDownloads.h"
#include "PageSettingsUploads.h"
#include "PageSettingsRemote.h"
#include "PageSettingsNetworks.h"
#include "PageSettingsGnutella.h"
#include "PageSettingsDonkey.h"
#include "PageSettingsBitTorrent.h"
#include "PageSettingsSkins.h"
#include "PageSettingsPlugins.h"
#include "PageSettingsAdvanced.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CSettingsManagerDlg, CSettingsSheet)

BEGIN_MESSAGE_MAP(CSettingsManagerDlg, CSettingsSheet)
	ON_COMMAND(IDRETRY, OnApply)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsManagerDlg dialog

CSettingsManagerDlg::CSettingsManagerDlg(CWnd* pParent)
	: CSettingsSheet( pParent, IDS_SETTINGS )
{
}

void CSettingsManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
}

/////////////////////////////////////////////////////////////////////////////
// CSettingsManagerDlg operations

CSettingsManagerDlg* CSettingsManagerDlg::m_pThis = NULL;

BOOL CSettingsManagerDlg::Run(LPCTSTR pszWindow)
{
	if ( m_pThis ) return FALSE;
	CSettingsManagerDlg pSheet;
	m_pThis = &pSheet;
	BOOL bResult = ( pSheet.DoModal( pszWindow ) == IDOK );
	m_pThis = NULL;
	return bResult;
}

void CSettingsManagerDlg::OnSkinChange(BOOL bSet)
{
	if ( m_pThis == NULL ) return;

	if ( bSet )
	{
		m_pThis->SkinMe( _T("CSettingSheet"), IDR_MAINFRAME );

		for ( INT_PTR i = 0; i < m_pThis->GetPageCount(); ++i )
		{
			CSettingsPage* pPage = m_pThis->GetPage( i );

			pPage->OnSkinChange();
		}

		m_pThis->Invalidate();
	}
	else
	{
		m_pThis->RemoveSkin();
	}
}

INT_PTR CSettingsManagerDlg::DoModal(LPCTSTR pszWindow)
{
	BOOL bAdvanced			= Settings.General.GUIMode != GUI_BASIC;

	CAutoPtr< CRichSettingsPage > gGeneral( new CRichSettingsPage( _T("CGeneralSettingsGroup") ) );
	CAutoPtr< CGeneralSettingsPage > pGeneral( new CGeneralSettingsPage );
	CAutoPtr< CLibrarySettingsPage > pLibrary( new CLibrarySettingsPage );
	CAutoPtr< CMediaSettingsPage > pMedia( new CMediaSettingsPage );
	CAutoPtr< CCommunitySettingsPage > pCommunity( new CCommunitySettingsPage );
	CAutoPtr< CWebSettingsPage > pWeb( new CWebSettingsPage );
	CAutoPtr< CRichSettingsPage > gInternet( new CRichSettingsPage( _T("CInternetSettingsGroup") ) );
	CAutoPtr< CConnectionSettingsPage > pConnection( new CConnectionSettingsPage );
	CAutoPtr< CDownloadsSettingsPage > pDownloads( new CDownloadsSettingsPage );
	CAutoPtr< CUploadsSettingsPage > pUploads( new CUploadsSettingsPage );
	CAutoPtr< CRemoteSettingsPage > pRemote( new CRemoteSettingsPage );
	CAutoPtr< CNetworksSettingsPage > gNetworks( new CNetworksSettingsPage );
	CAutoPtr< CGnutellaSettingsPage > pGnutella( new CGnutellaSettingsPage );
	CAutoPtr< CDonkeySettingsPage > pDonkey( new CDonkeySettingsPage );
	CAutoPtr< CBitTorrentSettingsPage > pTorrent( new CBitTorrentSettingsPage );
	CAutoPtr< CSkinsSettingsPage > pSkins( new CSkinsSettingsPage );
	CAutoPtr< CPluginsSettingsPage > pPlugins( new CPluginsSettingsPage );
	CAutoPtr< CAdvancedSettingsPage > pAdvanced( new CAdvancedSettingsPage );
	CAutoPtr< CIRCSettingsPage > pIRC( new CIRCSettingsPage );

	AddGroup( gGeneral );
	AddPage( pGeneral );
	AddPage( pLibrary );
	AddPage( pMedia );
	AddPage( pCommunity );
	AddPage( pIRC );
	AddPage( pWeb );
	AddGroup( gInternet );
	AddPage( pConnection );
	AddPage( pDownloads );
	AddPage( pUploads );
	AddPage( pRemote );
	if ( bAdvanced )
	{
		AddGroup( gNetworks );
		AddPage( pGnutella );
#ifndef LAN_MODE
		AddPage( pDonkey );
		AddPage( pTorrent );
#endif
	}
	AddGroup( pSkins );
	AddGroup( pPlugins );
	if ( bAdvanced ) AddGroup( pAdvanced );

	if ( pszWindow != NULL )
	{
		SetActivePage( GetPage( pszWindow ) );
	}
	else
	{
		SetActivePage( GetPage( Settings.General.LastSettingsPage ) );
	}

	INT_PTR nReturn = CSettingsSheet::DoModal();

	if ( m_pFirst )
	{
		Settings.General.LastSettingsPage = m_pFirst->GetRuntimeClass()->m_lpszClassName;
	}

	return nReturn;
}

void CSettingsManagerDlg::AddPage(CSettingsPage* pPage)
{
	CString strCaption = Skin.GetDialogCaption( CString( pPage->GetRuntimeClass()->m_lpszClassName ) );
	CSettingsSheet::AddPage( pPage, strCaption.GetLength() ? (LPCTSTR)strCaption : NULL );
}

void CSettingsManagerDlg::AddGroup(CSettingsPage* pPage)
{
	if ( pPage->IsKindOf( RUNTIME_CLASS(CRichSettingsPage) ) )
	{
		CString strCaption = ((CRichSettingsPage*)pPage)->m_sCaption;
		CSettingsSheet::AddGroup( pPage, strCaption );
	}
	else
	{
		CString strName( pPage->GetRuntimeClass()->m_lpszClassName );
		CString strCaption = Skin.GetDialogCaption( strName );
		CSettingsSheet::AddGroup( pPage, strCaption.GetLength() ? (LPCTSTR)strCaption : NULL );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSettingsManagerDlg message handlers

BOOL CSettingsManagerDlg::OnInitDialog()
{
	CSettingsSheet::OnInitDialog();

	SkinMe( _T("CSettingSheet"), IDR_MAINFRAME );

	return TRUE;
}

void CSettingsManagerDlg::OnOK()
{
	CSettingsSheet::OnOK();
	Settings.Save();
	AfxGetMainWnd()->Invalidate();
}

void CSettingsManagerDlg::OnApply()
{
	CSettingsSheet::OnApply();
	AfxGetMainWnd()->Invalidate();
}
