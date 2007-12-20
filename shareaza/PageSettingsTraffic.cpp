//
// PageSettingsTraffic.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "LiveList.h"
#include "PageSettingsTraffic.h"
#include "Skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CAdvancedSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CAdvancedSettingsPage, CSettingsPage)
	//{{AFX_MSG_MAP(CAdvancedSettingsPage)
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PROPERTIES, OnItemChangedProperties)
	ON_EN_CHANGE(IDC_VALUE, OnChangeValue)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_PROPERTIES, OnColumnClickProperties)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_DEFAULT_VALUE, OnBnClickedDefaultValue)
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CAdvancedSettingsPage property page

CAdvancedSettingsPage::CAdvancedSettingsPage() : CSettingsPage(CAdvancedSettingsPage::IDD)
{
	//{{AFX_DATA_INIT(CAdvancedSettingsPage)
	//}}AFX_DATA_INIT
}

CAdvancedSettingsPage::~CAdvancedSettingsPage()
{
}

void CAdvancedSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvancedSettingsPage)
	DDX_Control(pDX, IDC_VALUE_SPIN, m_wndValueSpin);
	DDX_Control(pDX, IDC_VALUE, m_wndValue);
	DDX_Control(pDX, IDC_PROPERTIES, m_wndList);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CAdvancedSettingsPage message handlers

BOOL CAdvancedSettingsPage::OnInitDialog() 
{
	CSettingsPage::OnInitDialog();

	CRect rc;
	m_wndList.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL ) + 1;

	m_wndList.InsertColumn( 0, _T("Setting"), LVCFMT_LEFT, rc.right - 80, 0 );
	m_wndList.InsertColumn( 1, _T("Value"), LVCFMT_LEFT, 80, 1 );

	m_wndList.SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE,
		LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT );

	AddSetting( &Settings.General.Debug );
	AddSetting( &Settings.General.DebugLog );
	AddSetting( &Settings.General.SearchLog );
	AddSetting( &Settings.General.DebugBTSources );
	AddSetting( &Settings.General.MaxDebugLogSize );
	AddSetting( &Settings.General.UpdateCheck );
	AddSetting( &Settings.General.DiskSpaceWarning );
	AddSetting( &Settings.General.DiskSpaceStop );
	AddSetting( &Settings.General.HashIntegrity );
	AddSetting( &Settings.General.MinTransfersRest );

	AddSetting( &Settings.Community.ChatFilterED2K );
	AddSetting( &Settings.Community.AwayMessageIdleTime );

	AddSetting( &Settings.Connection.IgnoreOwnIP );
	AddSetting( &Settings.Connection.SendBuffer );
	AddSetting( &Settings.Connection.TimeoutTraffic );
	AddSetting( &Settings.Connection.ConnectThrottle );
	AddSetting( &Settings.Connection.FailurePenalty );
	AddSetting( &Settings.Connection.FailureLimit );
	AddSetting( &Settings.Connection.DetectConnectionLoss );
	AddSetting( &Settings.Connection.DetectConnectionReset );
	AddSetting( &Settings.Connection.ForceConnectedState );
	AddSetting( &Settings.Connection.SlowConnect );
	AddSetting( &Settings.Connection.EnableFirewallException );
	AddSetting( &Settings.Connection.DeleteFirewallException );
	AddSetting( &Settings.Connection.DeleteUPnPPorts );
	AddSetting( &Settings.Connection.SkipWANPPPSetup );
	AddSetting( &Settings.Connection.SkipWANIPSetup );

	AddSetting( &Settings.Gnutella.ConnectFactor );
	AddSetting( &Settings.Gnutella.ConnectThrottle );
	AddSetting( &Settings.Gnutella.MaxResults );
	AddSetting( &Settings.Gnutella.MaxHits );
	AddSetting( &Settings.Gnutella.HitsPerPacket );
	AddSetting( &Settings.Gnutella.RouteCache );
	AddSetting( &Settings.Gnutella.HostCacheSize );
	AddSetting( &Settings.Gnutella.BlockBlankClients );
	AddSetting( &Settings.Gnutella.SpecifyProtocol );

	AddSetting( &Settings.Gnutella1.PacketBufferSize );
	AddSetting( &Settings.Gnutella1.PacketBufferTime );
	AddSetting( &Settings.Gnutella1.DefaultTTL );
	AddSetting( &Settings.Gnutella1.SearchTTL );
	AddSetting( &Settings.Gnutella1.TranslateTTL );
	AddSetting( &Settings.Gnutella1.MaximumTTL );
	AddSetting( &Settings.Gnutella1.MaximumPacket );
	AddSetting( &Settings.Gnutella1.MaximumQuery );
	AddSetting( &Settings.Gnutella1.StrictPackets );
	AddSetting( &Settings.Gnutella1.EnableGGEP );
	AddSetting( &Settings.Gnutella1.VendorMsg );
	AddSetting( &Settings.Gnutella1.QueryThrottle );
	AddSetting( &Settings.Gnutella1.RequeryDelay );
	AddSetting( &Settings.Gnutella1.HostExpire );
	AddSetting( &Settings.Gnutella1.PingFlood );
	AddSetting( &Settings.Gnutella1.PingRate );
	AddSetting( &Settings.Gnutella1.PongCache );
	AddSetting( &Settings.Gnutella1.PongCount );
	AddSetting( &Settings.Gnutella1.QuerySearchUTF8 );
	AddSetting( &Settings.Gnutella1.QueryHitUTF8 );
	AddSetting( &Settings.Gnutella1.MaxHostsInPongs );

	AddSetting( &Settings.Gnutella2.EnableAlways );
	AddSetting( &Settings.Gnutella2.HAWPeriod );
	AddSetting( &Settings.Gnutella2.HostCurrent );
	AddSetting( &Settings.Gnutella2.HostExpire );
	AddSetting( &Settings.Gnutella2.HubHorizonSize );
	AddSetting( &Settings.Gnutella2.KHLHubCount );
	AddSetting( &Settings.Gnutella2.KHLPeriod );
	AddSetting( &Settings.Gnutella2.LNIPeriod );
	AddSetting( &Settings.Gnutella2.PingRate );
	AddSetting( &Settings.Gnutella2.PingRelayLimit);
	AddSetting( &Settings.Gnutella2.QueryGlobalThrottle );
	AddSetting( &Settings.Gnutella2.QueryHostDeadline );
	AddSetting( &Settings.Gnutella2.QueryHostThrottle );
	AddSetting( &Settings.Gnutella2.QueryLimit );
	AddSetting( &Settings.Gnutella2.RequeryDelay );
	AddSetting( &Settings.Gnutella2.UdpBuffers );
	AddSetting( &Settings.Gnutella2.UdpGlobalThrottle );
	AddSetting( &Settings.Gnutella2.UdpInExpire );
	AddSetting( &Settings.Gnutella2.UdpInFrames );
	AddSetting( &Settings.Gnutella2.UdpMTU );
	AddSetting( &Settings.Gnutella2.UdpOutExpire );
	AddSetting( &Settings.Gnutella2.UdpOutFrames );
	AddSetting( &Settings.Gnutella2.UdpOutResend );

	AddSetting( &Settings.eDonkey.DequeueTime );
	AddSetting( &Settings.eDonkey.Endgame );
	AddSetting( &Settings.eDonkey.ExtendedRequest );
	AddSetting( &Settings.eDonkey.FastConnect );
	AddSetting( &Settings.eDonkey.ForceHighID );
	AddSetting( &Settings.eDonkey.FrameSize );
	AddSetting( &Settings.eDonkey.GetSourcesThrottle );
	AddSetting( &Settings.eDonkey.LargeFileSupport );
	AddSetting( &Settings.eDonkey.LearnNewServersClient );
	AddSetting( &Settings.eDonkey.MagnetSearch );
	AddSetting( &Settings.eDonkey.MaxShareCount );
	AddSetting( &Settings.eDonkey.MinServerFileSize );
	AddSetting( &Settings.eDonkey.PacketThrottle );
	AddSetting( &Settings.eDonkey.QueryFileThrottle );
	AddSetting( &Settings.eDonkey.QueryGlobalThrottle );
	AddSetting( &Settings.eDonkey.QueryServerThrottle );
	AddSetting( &Settings.eDonkey.QueueRankThrottle );
	AddSetting( &Settings.eDonkey.ReAskTime );
	AddSetting( &Settings.eDonkey.RequestPipe );
	AddSetting( &Settings.eDonkey.RequestSize );
	AddSetting( &Settings.eDonkey.SendPortServer );
	AddSetting( &Settings.eDonkey.SourceThrottle );
	AddSetting( &Settings.eDonkey.StatsGlobalThrottle );
	AddSetting( &Settings.eDonkey.StatsServerThrottle );
	AddSetting( &Settings.eDonkey.TagNames );

	AddSetting( &Settings.BitTorrent.AutoSeed );
	AddSetting( &Settings.BitTorrent.BandwidthPercentage );
	AddSetting( &Settings.BitTorrent.DefaultTrackerPeriod );
	AddSetting( &Settings.BitTorrent.LinkPing );
	AddSetting( &Settings.BitTorrent.LinkTimeout );
	AddSetting( &Settings.BitTorrent.MaxTrackerRetry );
	AddSetting( &Settings.BitTorrent.RandomPeriod );
	AddSetting( &Settings.BitTorrent.RequestLimit );
	AddSetting( &Settings.BitTorrent.RequestPipe );
	AddSetting( &Settings.BitTorrent.RequestSize );
	AddSetting( &Settings.BitTorrent.SourceExchangePeriod );
	AddSetting( &Settings.BitTorrent.StandardPeerID );
	AddSetting( &Settings.BitTorrent.TorrentCodePage );
	AddSetting( &Settings.BitTorrent.TorrentExtraKeys );
	AddSetting( &Settings.BitTorrent.TrackerKey );
	AddSetting( &Settings.BitTorrent.UploadCount );

	AddSetting( &Settings.Discovery.AccessThrottle );
	AddSetting( &Settings.Discovery.BootstrapCount );
	AddSetting( &Settings.Discovery.CacheCount );
	AddSetting( &Settings.Discovery.DefaultUpdate );
	AddSetting( &Settings.Discovery.EnableG1GWC );
	AddSetting( &Settings.Discovery.FailureLimit );
	AddSetting( &Settings.Discovery.Lowpoint );
	AddSetting( &Settings.Discovery.UpdatePeriod );

	AddSetting( &Settings.Search.AdvancedPanel );
	AddSetting( &Settings.Search.GeneralThrottle );
	AddSetting( &Settings.Search.HighlightNew );
	AddSetting( &Settings.Search.MaxPreviewLength );
	AddSetting( &Settings.Search.MonitorQueue );
	AddSetting( &Settings.Search.SchemaTypes );
	AddSetting( &Settings.Search.ShowNames );
	AddSetting( &Settings.Search.SpamFilterThreshold );

	AddSetting( &Settings.Downloads.AllowBackwards );
	AddSetting( &Settings.Downloads.AutoClear );
	AddSetting( &Settings.Downloads.BufferSize );
	AddSetting( &Settings.Downloads.ChunkSize );
	AddSetting( &Settings.Downloads.ChunkStrap );
	AddSetting( &Settings.Downloads.ClearDelay );
	AddSetting( &Settings.Downloads.ConnectThrottle );
	AddSetting( &Settings.Downloads.FlushSD );
	AddSetting( &Settings.Downloads.MaxAllowedFailures );
	AddSetting( &Settings.Downloads.MaxConnectingSources );
	AddSetting( &Settings.Downloads.MaxFileSearches );
	AddSetting( &Settings.Downloads.MaxReviews );
	AddSetting( &Settings.Downloads.Metadata );
	AddSetting( &Settings.Downloads.MinSources );
	AddSetting( &Settings.Downloads.NeverDrop );
	AddSetting( &Settings.Downloads.PushTimeout );
	AddSetting( &Settings.Downloads.RequestHTTP11 );
	AddSetting( &Settings.Downloads.RequestHash );
	AddSetting( &Settings.Downloads.RequestURLENC );
	AddSetting( &Settings.Downloads.RetryDelay );
	AddSetting( &Settings.Downloads.SaveInterval );
	AddSetting( &Settings.Downloads.SearchPeriod );
	AddSetting( &Settings.Downloads.ShowPercent );
	AddSetting( &Settings.Downloads.SortColumns );
	AddSetting( &Settings.Downloads.SortSources );
	AddSetting( &Settings.Downloads.SparseThreshold );
	AddSetting( &Settings.Downloads.StaggardStart );
	AddSetting( &Settings.Downloads.StartDroppingFailedSourcesNumber );
	AddSetting( &Settings.Downloads.StarveGiveUp );
	AddSetting( &Settings.Downloads.StarveTimeout );
	AddSetting( &Settings.Downloads.VerifyFiles );
	AddSetting( &Settings.Downloads.VerifyTiger );

	AddSetting( &Settings.Uploads.AllowBackwards );
	AddSetting( &Settings.Uploads.AutoClear );
	AddSetting( &Settings.Uploads.ClampdownFactor );
	AddSetting( &Settings.Uploads.ClampdownFloor );
	AddSetting( &Settings.Uploads.ClearDelay );
	AddSetting( &Settings.Uploads.DynamicPreviews );
	AddSetting( &Settings.Uploads.FreeBandwidthFactor );
	AddSetting( &Settings.Uploads.FreeBandwidthValue );
	AddSetting( &Settings.Uploads.PreviewQuality );
	AddSetting( &Settings.Uploads.PreviewTransfers );
	AddSetting( &Settings.Uploads.QueuePollMax );
	AddSetting( &Settings.Uploads.QueuePollMin );
	AddSetting( &Settings.Uploads.RewardQueuePercentage );
	AddSetting( &Settings.Uploads.RotateChunkLimit );

	AddSetting( &Settings.Interface.LowResMode );

	AddSetting( &Settings.Library.HashWindow );
	AddSetting( &Settings.Library.HighPriorityHashing );
	AddSetting( &Settings.Library.LowPriorityHashing );
	AddSetting( &Settings.Library.MarkFileAsDownload );
	AddSetting( &Settings.Library.MaxMaliciousFileSize );
	AddSetting( &Settings.Library.PreferAPETags );
	AddSetting( &Settings.Library.QueryRouteSize );
	AddSetting( &Settings.Library.SourceExpire );
	AddSetting( &Settings.Library.SourceMesh );
	AddSetting( &Settings.Library.ThumbSize );
	AddSetting( &Settings.Library.TigerHeight );
	AddSetting( &Settings.Library.UseCustomFolders );
	AddSetting( &Settings.Library.UseFolderGUID );
	AddSetting( &Settings.Library.VirtualFiles );
	AddSetting( &Settings.Library.WatchFoldersTimeout );

	AddSetting( &Settings.MediaPlayer.ShortPaths );

	AddSetting( &Settings.Bandwidth.HubIn );
	AddSetting( &Settings.Bandwidth.HubOut );
	AddSetting( &Settings.Bandwidth.HubUploads );
	AddSetting( &Settings.Bandwidth.LeafIn );
	AddSetting( &Settings.Bandwidth.LeafOut );
	AddSetting( &Settings.Bandwidth.PeerIn );
	AddSetting( &Settings.Bandwidth.PeerOut );
	AddSetting( &Settings.Bandwidth.Request );
	AddSetting( &Settings.Bandwidth.UdpOut );

	AddSetting( &Settings.Experimental.EnableDIPPSupport );

	AddSetting( &Settings.WINE.MenuFix );

	CLiveList::Sort( &m_wndList, 0 );
	CLiveList::Sort( &m_wndList, 0 );

	Skin.Translate( _T("CAdvancedSettingsList"), m_wndList.GetHeaderCtrl() );

	UpdateAll();

	return TRUE;
}

void CAdvancedSettingsPage::AddSetting(LPVOID pValue)
{
	CSettings::Item* pItem = Settings.GetSetting( pValue );
	ASSERT( pItem != NULL );
	if ( pItem == NULL ) return;

	EditItem* pEdit = new EditItem( pItem );
	ASSERT( pEdit != NULL );
	if ( pEdit == NULL ) return;

	LV_ITEM pList = {};
	
	pList.mask		= LVIF_PARAM|LVIF_TEXT|LVIF_IMAGE;
	pList.iItem		= m_wndList.GetItemCount();
	pList.lParam	= (LPARAM)pEdit;
	pList.iImage	= 0;
	pList.pszText	= (LPTSTR)(LPCTSTR)pEdit->m_sName;
	pList.iItem		= m_wndList.InsertItem( &pList );

	UpdateItem( pList.iItem );
}

void CAdvancedSettingsPage::UpdateItem(int nItem)
{
	EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );
	CString strValue;
		
	if ( pItem->m_pItem->m_pBool )
	{
		ASSERT( pItem->m_pItem->m_nScale == 1 &&
			pItem->m_pItem->m_nMin == 0 && pItem->m_pItem->m_nMax == 1 );
		strValue = pItem->m_bValue ? _T("True") : _T("False");
	}
	else if ( pItem->m_pItem->m_pDword )
	{
		ASSERT( pItem->m_pItem->m_nScale &&
			pItem->m_pItem->m_nMin < pItem->m_pItem->m_nMax );
		strValue.Format( _T("%lu"), pItem->m_nValue / pItem->m_pItem->m_nScale );
		if ( Settings.General.LanguageRTL )
			strValue = _T("\x200E") + strValue + pItem->m_pItem->m_szSuffix;
		else
			strValue += pItem->m_pItem->m_szSuffix;
	}
	
	m_wndList.SetItemText( nItem, 1, strValue );
}

void CAdvancedSettingsPage::OnItemChangedProperties(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
//	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	UpdateAll();

	*pResult = 0;
}

void CAdvancedSettingsPage::UpdateAll()
{
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	
	if ( nItem >= 0 )
	{
		EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );
		CString strValue;
		
		if ( pItem->m_pItem->m_pDword )
		{
			m_wndValueSpin.SendMessage( UDM_SETRANGE32, pItem->m_pItem->m_nMin, pItem->m_pItem->m_nMax );
			strValue.Format( _T("%lu"), pItem->m_nValue / pItem->m_pItem->m_nScale );
		}
		else
		{
			m_wndValueSpin.SendMessage( UDM_SETRANGE32, 0, 1 );
			strValue = pItem->m_bValue ? _T("1") : _T("0");
		}
		m_wndValue.SetWindowText( strValue );
		m_wndValue.EnableWindow( TRUE );
		m_wndValueSpin.EnableWindow( TRUE );
		GetDlgItem( IDC_DEFAULT_VALUE )->EnableWindow( ! pItem->IsDefault() );
	}
	else
	{
		m_wndValue.SetWindowText( _T("") );
		m_wndValue.EnableWindow( FALSE );
		m_wndValueSpin.EnableWindow( FALSE );
		GetDlgItem( IDC_DEFAULT_VALUE )->EnableWindow( FALSE );
	}	
}

void CAdvancedSettingsPage::OnChangeValue() 
{
	if ( m_wndList.m_hWnd == NULL ) return;
	
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	
	if ( nItem >= 0 )
	{
		EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );
		CString strValue;
		
		m_wndValue.GetWindowText( strValue );
		
		DWORD nValue = 0;
		if ( _stscanf( strValue, _T("%lu"), &nValue ) == 1 )
		{
			if ( pItem->m_pItem->m_pDword )
				pItem->m_nValue = max( pItem->m_pItem->m_nMin,
					min( pItem->m_pItem->m_nMax, nValue ) ) * pItem->m_pItem->m_nScale;
			else
				pItem->m_bValue = ( nValue == 1 );

			UpdateItem( nItem );

			GetDlgItem( IDC_DEFAULT_VALUE )->EnableWindow( ! pItem->IsDefault() );
		}
	}
}

void CAdvancedSettingsPage::OnColumnClickProperties(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	CLiveList::Sort( &m_wndList, pNMListView->iSubItem );
	*pResult = 0;
}

void CAdvancedSettingsPage::OnOK() 
{
	for ( int nItem = 0 ; nItem < m_wndList.GetItemCount() ; nItem++ )
	{
		EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );
		pItem->Commit();
	}
	
	CSettingsPage::OnOK();
}

void CAdvancedSettingsPage::OnDestroy() 
{
	for ( int nItem = 0 ; nItem < m_wndList.GetItemCount() ; nItem++ )
	{
		delete (EditItem*)m_wndList.GetItemData( nItem );
	}
	
	CSettingsPage::OnDestroy();
}

void CAdvancedSettingsPage::OnBnClickedDefaultValue()
{
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	if ( nItem >= 0 )
	{
		EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );
		pItem->Default();

		UpdateItem( nItem );
		UpdateAll();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSettingEdit construction

CAdvancedSettingsPage::EditItem::EditItem(CSettings::Item* pItem) :
	m_pItem( pItem ),
	m_nValue( pItem->m_pDword ? *pItem->m_pDword : 0 ),
	m_bValue( pItem->m_pBool ? *pItem->m_pBool : false ),
	m_sName( pItem->m_szName )
{
	if ( ! *pItem->m_szSection ||							// Settings.Name -> General.Name
		! lstrcmp( pItem->m_szSection, _T("Settings.") ) )	// .Name -> General.Name
		m_sName.Insert( 0, _T("General") );
	else
		m_sName.Insert( 0, pItem->m_szSection );
}

void CAdvancedSettingsPage::EditItem::Commit()
{
	if ( m_pItem->m_pDword )
		*m_pItem->m_pDword = m_nValue;
	else
		*m_pItem->m_pBool= m_bValue;
}

bool CAdvancedSettingsPage::EditItem::IsDefault() const
{
	if ( m_pItem->m_pDword )
		return ( m_pItem->m_DwordDefault == m_nValue );
	else
		return ( m_pItem->m_BoolDefault == m_bValue );
}

void CAdvancedSettingsPage::EditItem::Default()
{
	if ( m_pItem->m_pDword )
		m_nValue = m_pItem->m_DwordDefault;
	else
		m_bValue = m_pItem->m_BoolDefault;
}
