//
// PageSettingsTraffic.cpp
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
	
	AddSetting( &Settings.General.Debug, 1, 0, 1 );
	AddSetting( &Settings.General.DebugLog, 1, 0, 1 );
	AddSetting( &Settings.General.UpdateCheck, 1, 0, 1 );
	AddSetting( &Settings.General.DiskSpaceWarning, 1, 5, 2000 , _T(" M") );
	AddSetting( &Settings.General.HashIntegrity, 1, 0, 1 );
	
	AddSetting( &Settings.Connection.IgnoreOwnIP, 1, 0, 1 );
	AddSetting( &Settings.Connection.SendBuffer, 1, 64, 10240 );
	AddSetting( &Settings.Connection.TimeoutTraffic, 1000, 10, 60*60, _T(" s") );
	// AddSetting( &Settings.Connection.AsyncIO, 1, 0, 1 );
	AddSetting( &Settings.Connection.RequireForTransfers, 1, 0, 1 );
	
	AddSetting( &Settings.Gnutella.ConnectFactor, 1, 1, 20, _T("x") );
	AddSetting( &Settings.Gnutella.MaxResults, 1, 1, 1000 );
	AddSetting( &Settings.Gnutella.MaxHits, 1, 0, 4096 );
	AddSetting( &Settings.Gnutella.HitsPerPacket, 1, 0, 1024 );
	AddSetting( &Settings.Gnutella.RouteCache, 60, 1, 120, _T(" m") );
	AddSetting( &Settings.Gnutella.HostCacheSize, 1, 32, 16384 );
	AddSetting( &Settings.Gnutella.HostCacheExpire, 60, 1, 24*60, _T(" m") );
	
	AddSetting( &Settings.Gnutella1.Handshake04, 1, 0, 1 );
	AddSetting( &Settings.Gnutella1.Handshake06, 1, 0, 1 );
	AddSetting( &Settings.Gnutella1.PacketBufferSize, 1, 1, 1024 );
	AddSetting( &Settings.Gnutella1.PacketBufferTime, 1000, 10, 180, _T(" s") );
	AddSetting( &Settings.Gnutella1.DefaultTTL, 1, 1, 5 );
	AddSetting( &Settings.Gnutella1.SearchTTL, 1, 1, 4 );
	AddSetting( &Settings.Gnutella1.TranslateTTL, 1, 1, 2 );
	AddSetting( &Settings.Gnutella1.MaximumTTL, 1, 1, 10 );
	AddSetting( &Settings.Gnutella1.MaximumPacket, 1, 32, 262144 );
	AddSetting( &Settings.Gnutella1.MaximumQuery, 1, 32, 262144 );
	AddSetting( &Settings.Gnutella1.StrictPackets, 1, 0, 1 );
	AddSetting( &Settings.Gnutella1.EnableGGEP, 1, 0, 1 );
	AddSetting( &Settings.Gnutella1.VendorMsg, 1, 0, 1 );
	AddSetting( &Settings.Gnutella1.QueryThrottle, 60, 5, 2048, _T(" m") );
	AddSetting( &Settings.Gnutella1.RequeryDelay, 60, 45, 2048, _T(" m") );
	AddSetting( &Settings.Gnutella1.PingFlood, 1000, 0, 30, _T(" s") );
	AddSetting( &Settings.Gnutella1.PingRate, 1000, 5, 180, _T(" s") );
	AddSetting( &Settings.Gnutella1.PongCache, 1000, 1, 180, _T(" s") );
	AddSetting( &Settings.Gnutella1.PongCount, 1, 1, 64 );
	
	AddSetting( &Settings.Gnutella2.EnableAlways, 1, 0, 1 );
	AddSetting( &Settings.Gnutella2.UdpMTU, 1, 16, 10240 );
	AddSetting( &Settings.Gnutella2.UdpBuffers, 1, 16, 2048 );
	AddSetting( &Settings.Gnutella2.UdpInFrames, 1, 16, 2048 );
	AddSetting( &Settings.Gnutella2.UdpOutFrames, 1, 16, 2048 );
	AddSetting( &Settings.Gnutella2.UdpGlobalThrottle, 1, 0, 10000 );
	AddSetting( &Settings.Gnutella2.UdpOutExpire, 1000, 1, 300, _T(" s") );
	AddSetting( &Settings.Gnutella2.UdpOutResend, 1000, 1, 300, _T(" s") );
	AddSetting( &Settings.Gnutella2.UdpInExpire, 1000, 1, 300, _T(" s") );
	AddSetting( &Settings.Gnutella2.KHLPeriod, 1000, 1, 60 * 60, _T(" s") );
	AddSetting( &Settings.Gnutella2.KHLHubCount, 1, 1, 256 );
	AddSetting( &Settings.Gnutella2.HAWPeriod, 1000, 1, 60 * 60, _T(" s") );
	AddSetting( &Settings.Gnutella2.QueryGlobalThrottle, 1, 1, 60*1000, _T(" ms") );
	AddSetting( &Settings.Gnutella2.QueryHostThrottle, 1, 1, 10*60, _T(" s") );
	AddSetting( &Settings.Gnutella2.QueryHostDeadline, 1, 1, 120*60, _T(" s") );
	AddSetting( &Settings.Gnutella2.RequeryDelay, 60, 45, 8*60, _T(" m") );
	AddSetting( &Settings.Gnutella2.HubHorizonSize, 1, 32, 512 );
	AddSetting( &Settings.Gnutella2.QueryLimit, 1, 0, 10000 );
	
	AddSetting( &Settings.eDonkey.QueryGlobalThrottle, 1, 1000, 20000, _T(" ms") );
	AddSetting( &Settings.eDonkey.QueryServerThrottle, 60, 1, 180, _T(" m") );
	AddSetting( &Settings.eDonkey.RequestPipe, 1, 1, 10 );
	AddSetting( &Settings.eDonkey.MaxShareCount, 1, 0, 20000 );
	AddSetting( &Settings.eDonkey.RequestSize, 1024, 10, 1000, _T(" KB") );
	AddSetting( &Settings.eDonkey.FrameSize, 1024, 1, 500, _T(" KB") );
	AddSetting( &Settings.eDonkey.ReAskTime, 60, 10, 360, _T(" m") );
	AddSetting( &Settings.eDonkey.DequeueTime, 60, 2, 512, _T(" m") );
	AddSetting( &Settings.eDonkey.TagNames, 1, 0, 1 );
	AddSetting( &Settings.eDonkey.ExtendedRequest, 1, 0, 1 );
	AddSetting( &Settings.eDonkey.MagnetSearch, 1, 0, 1 );
	AddSetting( &Settings.eDonkey.LearnNewServers, 1, 0, 1 );
	
	//AddSetting( &Settings.BitTorrent.AdvancedInterface, 1, 0, 1 );
	AddSetting( &Settings.BitTorrent.DefaultTrackerPeriod, 60000, 5, 120, _T(" m") );
	AddSetting( &Settings.BitTorrent.LinkTimeout, 1000, 10, 60*10, _T(" s") );
	AddSetting( &Settings.BitTorrent.LinkPing, 1000, 10, 60*10, _T(" s") );
	AddSetting( &Settings.BitTorrent.RequestPipe, 1, 1, 10 );
	AddSetting( &Settings.BitTorrent.RequestSize, 1024, 32, 128, _T(" KB") );
	AddSetting( &Settings.BitTorrent.RequestLimit, 1024, 1, 1024, _T(" KB") );
	AddSetting( &Settings.BitTorrent.RandomPeriod, 1000, 1, 60*5, _T(" s") );
	AddSetting( &Settings.BitTorrent.SourceExchangePeriod, 1, 1, 60*5, _T(" m") );
	AddSetting( &Settings.BitTorrent.UploadCount, 1, 2, 16 );
	//AddSetting( &Settings.BitTorrent.DownloadConnections, 1, 1, 60 );
	//AddSetting( &Settings.BitTorrent.Endgame, 1, 0, 1 );
	
	AddSetting( &Settings.Discovery.AccessThrottle, 60, 1, 180, _T(" m") );
	AddSetting( &Settings.Discovery.Lowpoint, 1, 1, 512 );
	AddSetting( &Settings.Discovery.FailureLimit, 1, 1, 512 );
	AddSetting( &Settings.Discovery.UpdatePeriod, 60, 1, 60 * 24, _T(" m") );
	AddSetting( &Settings.Discovery.DefaultUpdate, 60, 1, 60 * 24, _T(" m") );
	AddSetting( &Settings.Discovery.BootstrapCount, 1, 0, 20 );
	
	AddSetting( &Settings.Search.ShowNames, 1, 0, 1 );
	AddSetting( &Settings.Search.MonitorQueue, 1, 1, 4096 );
	AddSetting( &Settings.Search.MaxPreviewLength, 1024, 1, 4096, _T(" KB") );
	
	AddSetting( &Settings.Downloads.BufferSize, 1024, 0, 512, _T(" KB") );
	AddSetting( &Settings.Downloads.SparseThreshold, 1024, 0, 256, _T(" MB") );
	AddSetting( &Settings.Downloads.MaxFileSearches, 1, 0, 10 );
	AddSetting( &Settings.Downloads.MaxConnectingSources, 1, 5, 50 );
	AddSetting( &Settings.Downloads.MinSources, 1, 0, 8 );
	AddSetting( &Settings.Downloads.ConnectThrottle, 1, 0, 5000, _T(" ms") );
	AddSetting( &Settings.Downloads.PushTimeout, 1000, 5, 180, _T(" s") );
	AddSetting( &Settings.Downloads.StarveTimeout, 60, 45, 1440, _T(" m") );
	AddSetting( &Settings.Downloads.StarveGiveUp, 1, 3, 120, _T(" h") );
	AddSetting( &Settings.Downloads.ChunkSize, 1024, 0, 10240, _T(" KB") );
	AddSetting( &Settings.Downloads.ChunkStrap, 1024, 0, 10240, _T(" KB") );
	AddSetting( &Settings.Downloads.AllowBackwards, 1, 0, 1 );
	AddSetting( &Settings.Downloads.NeverDrop, 1, 0, 1 );
	AddSetting( &Settings.Downloads.RequestHash, 1, 0, 1 );
	AddSetting( &Settings.Downloads.RequestHTTP11, 1, 0, 1 );
	AddSetting( &Settings.Downloads.RequestURLENC, 1, 0, 1 );
	AddSetting( &Settings.Downloads.SaveInterval, 1000, 1, 120, _T(" s") );
	AddSetting( &Settings.Downloads.FlushSD, 1, 0, 1 );
	AddSetting( &Settings.Downloads.ShowPercent, 1, 0, 1 );
	AddSetting( &Settings.Downloads.ClearDelay, 1000, 1, 1800, _T(" s") );
	AddSetting( &Settings.Downloads.RetryDelay, 1000, 120, 60*60, _T(" s") );
	AddSetting( &Settings.Downloads.SearchPeriod, 1000, 10, 5*60 );
	AddSetting( &Settings.Downloads.StaggardStart, 1, 0, 1 );
	AddSetting( &Settings.Downloads.VerifyFiles, 1, 0, 1 );
	AddSetting( &Settings.Downloads.VerifyTiger, 1, 0, 1 );
	AddSetting( &Settings.Downloads.Metadata, 1, 0, 1 );
	AddSetting( &Settings.Downloads.SortColumns, 1, 0, 1 );
	AddSetting( &Settings.Downloads.SortSources, 1, 0, 1 );
	
	AddSetting( &Settings.Uploads.FreeBandwidthValue, 128, 0, 4096, _T(" Kb/s") );
	AddSetting( &Settings.Uploads.FreeBandwidthFactor, 1, 0, 100, _T("%") );
	AddSetting( &Settings.Uploads.ClampdownFactor, 1, 0, 100, _T("%") );
	AddSetting( &Settings.Uploads.ClampdownFloor, 128, 0, 4096, _T(" Kb/s") );
	AddSetting( &Settings.Uploads.QueuePollMin, 1000, 0, 60, _T(" s") );
	AddSetting( &Settings.Uploads.QueuePollMax, 1000, 30, 180, _T(" s") );
	AddSetting( &Settings.Uploads.RotateChunkLimit, 1024, 0, 10240, _T(" KB") );
	AddSetting( &Settings.Uploads.ClearDelay, 1000, 1, 1800, _T(" s") );
	AddSetting( &Settings.Uploads.AllowBackwards, 1, 0, 1 );
	AddSetting( &Settings.Uploads.DynamicPreviews, 1, 0, 1 );
	AddSetting( &Settings.Uploads.PreviewQuality, 1, 0, 100, _T("%") );
	AddSetting( &Settings.Uploads.PreviewTransfers, 1, 1, 64 );

	AddSetting( &Settings.Interface.LowResMode, 1, 0, 1 );
	
	AddSetting( &Settings.Library.SourceExpire, 60, 60, 604800, _T(" m") );
	AddSetting( &Settings.Library.SourceMesh, 1, 0, 1);
	AddSetting( &Settings.Library.TigerHeight, 1, 1, 64 );
	AddSetting( &Settings.Library.QueryRouteSize, 1, 8, 24 );
	AddSetting( &Settings.Library.ThumbSize, 1, 16, 256 );
	
	AddSetting( &Settings.Bandwidth.Request, 128, 0, 8192, _T(" Kb/s") );
	AddSetting( &Settings.Bandwidth.HubIn, 128, 0, 8192, _T(" Kb/s") );
	AddSetting( &Settings.Bandwidth.HubOut, 128, 0, 8192, _T(" Kb/s") );
	AddSetting( &Settings.Bandwidth.LeafIn, 128, 0, 8192, _T(" Kb/s") );
	AddSetting( &Settings.Bandwidth.LeafOut, 128, 0, 8192, _T(" Kb/s") );
	AddSetting( &Settings.Bandwidth.PeerIn, 128, 0, 8192, _T(" Kb/s") );
	AddSetting( &Settings.Bandwidth.PeerOut, 128, 0, 8192, _T(" Kb/s") );
	AddSetting( &Settings.Bandwidth.UdpOut, 128, 0, 8192, _T(" Kb/s") );
	AddSetting( &Settings.Bandwidth.HubUploads, 128, 0, 4096, _T(" Kb/s") );
	
	CLiveList::Sort( &m_wndList, 0 );
	CLiveList::Sort( &m_wndList, 0 );
	
	Skin.Translate( _T("CAdvancedSettingsList"), m_wndList.GetHeaderCtrl() );
	return TRUE;
}

void CAdvancedSettingsPage::AddSetting(LPVOID pValue, DWORD nScale, DWORD nMin, DWORD nMax, LPCTSTR pszSuffix)
{
	CSettings::Item* pItem = Settings.GetSetting( (DWORD*)pValue );
	if ( pItem == NULL ) return;
	
	EditItem* pEdit = new EditItem( pItem, nScale, nMin, nMax, pszSuffix );
	
	LV_ITEM pList;
	
	ZeroMemory( &pList, sizeof(pList) );

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
		
	if ( pItem->m_nMin == 0 && pItem->m_nMax == 1 )
	{
		strValue = pItem->m_nValue ? _T("True") : _T("False");
	}
	else
	{
		strValue.Format( _T("%lu"), pItem->m_nValue / pItem->m_nScale );
		strValue += pItem->m_sSuffix;
	}
	
	m_wndList.SetItemText( nItem, 1, strValue );
}

void CAdvancedSettingsPage::OnItemChangedProperties(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	
	if ( nItem >= 0 )
	{
		EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );
		CString strValue;
		
		m_wndValueSpin.SendMessage( WM_USER+111, pItem->m_nMin, pItem->m_nMax );
		
		strValue.Format( _T("%lu"), pItem->m_nValue / pItem->m_nScale );
		
		m_wndValue.SetWindowText( strValue );
		m_wndValue.EnableWindow( TRUE );
		m_wndValueSpin.EnableWindow( TRUE );
	}
	else
	{
		m_wndValue.SetWindowText( _T("") );
		m_wndValue.EnableWindow( FALSE );
		m_wndValueSpin.EnableWindow( FALSE );
	}
	
	*pResult = 0;
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
		
		if ( _stscanf( strValue, _T("%lu"), &pItem->m_nValue ) == 1 )
		{
			pItem->m_nValue = max( pItem->m_nMin, min( pItem->m_nMax, pItem->m_nValue ) );
			pItem->m_nValue *= pItem->m_nScale;
			UpdateItem( nItem );
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


/////////////////////////////////////////////////////////////////////////////
// CSettingEdit construction

CAdvancedSettingsPage::EditItem::EditItem(CSettings::Item* pItem, DWORD nScale, DWORD nMin, DWORD nMax, LPCTSTR pszSuffix)
{
	m_pItem		= pItem;
	m_nValue	= *pItem->m_pDword;
	m_sName		= pItem->m_sName;
	m_nScale	= nScale;
	m_nMin		= nMin;
	m_nMax		= nMax;
	
	if ( m_sName.GetAt( 0 ) == '.' ) m_sName = _T("razacore") + m_sName;
	
	if ( pszSuffix ) m_sSuffix = pszSuffix;
}

CAdvancedSettingsPage::EditItem::~EditItem()
{
}

void CAdvancedSettingsPage::EditItem::Commit()
{
	*(m_pItem->m_pDword) = m_nValue;
}

