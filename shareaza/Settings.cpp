//
// Settings.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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
#include "Schema.h"
#include "Skin.h"
#include "Registry.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define SMART_VERSION	61

#define Kilo	( 1024 )
#define Mega	( Kilo * 1024 )
#define Giga	( Mega * 1024 )
#define Tera	( Giga * 1024ui64 )
#define Peta	( Tera * 1024ui64 )
#define Exa		( Peta * 1024ui64 )

#define fKilo	( 1024.0 )
#define fMega	( fKilo * 1024.0 )
#define fGiga	( fMega * 1024.0 )
#define fTera	( fGiga * 1024.0 )
#define fPeta	( fTera * 1024.0 )
#define fExa	( fPeta * 1024.0 )

CSettings Settings;

//////////////////////////////////////////////////////////////////////
// CSettings construction

CSettings::CSettings()
{
	// Reset 'live' values.
	Live.DiskSpaceWarning			= false;
	Live.DiskWriteWarning			= false;
	Live.AdultWarning				= false;
	Live.QueueLimitWarning			= false;
	Live.DefaultED2KServersLoaded	= false;
	Live.DefaultDCServersLoaded		= false;
	Live.DonkeyServerWarning		= false;
	Live.UploadLimitWarning			= false;
	Live.DiskSpaceStop				= false;
	Live.BandwidthScale				= 100;
	Live.LoadWindowState			= false;
	Live.AutoClose					= false;
	Live.FirstRun					= false;
}

CSettings::~CSettings()
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		delete m_pItems.GetNext( pos );
	}
}

//////////////////////////////////////////////////////////////////////
// CSettings load

void CSettings::Load()
{
	// Add all settings
	Add( _T(""), _T("DebugBTSources"), &General.DebugBTSources, false );
	Add( _T(""), _T("DebugLog"), &General.DebugLog, false );
	Add( _T(""), _T("DiskSpaceStop"), &General.DiskSpaceStop, 25, 1, 0, 1000 , _T(" M") );
	Add( _T(""), _T("DiskSpaceWarning"), &General.DiskSpaceWarning, 500, 1, 5, 2000 , _T(" M") );
	Add( _T(""), _T("HashIntegrity"), &General.HashIntegrity, true );
	Add( _T(""), _T("ItWasLimited"), &General.ItWasLimited, false, true );
	Add( _T(""), _T("MaxDebugLogSize"), &General.MaxDebugLogSize, 10*Mega, Mega, 0, 100, _T(" MB") );
	Add( _T(""), _T("MinTransfersRest"), &General.MinTransfersRest, 50, 1, 1, 100, _T(" ms") );
	Add( _T(""), _T("Path"), &General.Path );
	Add( _T(""), _T("LogLevel"), &General.LogLevel, MSG_INFO, 1, MSG_ERROR, MSG_DEBUG, _T(" level") );
	Add( _T(""), _T("SearchLog"), &General.SearchLog, true );
	Add( _T(""), _T("UserPath"), &General.UserPath );
	Add( _T(""), _T("DialogScan"), &General.DialogScan, false );

	Add( _T("Settings"), _T("AlwaysOpenURLs"), &General.AlwaysOpenURLs, false );
	Add( _T("Settings"), _T("CloseMode"), &General.CloseMode, 0, 1, 0, 3 );
	Add( _T("Settings"), _T("FirstRun"), &General.FirstRun, true, true );
	Add( _T("Settings"), _T("Upgrade"), &General.Upgrade, true, true );
	Add( _T("Settings"), _T("GUIMode"), &General.GUIMode, GUI_BASIC );
	Add( _T("Settings"), _T("IgnoreXPsp2"), &General.IgnoreXPsp2, false );
	Add( _T("Settings"), _T("Language"), &General.Language, _T("en") );
	Add( _T("Settings"), _T("LanguageRTL"), &General.LanguageRTL, false );
	Add( _T("Settings"), _T("RatesInBytes"), &General.RatesInBytes, true );
	Add( _T("Settings"), _T("RatesUnit"), &General.RatesUnit, 0 );
	Add( _T("Settings"), _T("Running"), &General.Running, false, true );
	Add( _T("Settings"), _T("ShowTimestamp"), &General.ShowTimestamp, true );
	Add( _T("Settings"), _T("SizeLists"), &General.SizeLists, false );
	Add( _T("Settings"), _T("SmartVersion"), &General.SmartVersion, SMART_VERSION );
	Add( _T("Settings"), _T("TrayMinimise"), &General.TrayMinimise, false );
	Add( _T("Settings"), _T("LastSettingsPage"), &General.LastSettingsPage );
	Add( _T("Settings"), _T("LastSettingsIndex"), &General.LastSettingsIndex, 0 );
	Add( _T("Settings"), _T("SearchPanelResults"), &General.SearchPanelResults, true );
	Add( _T("Settings"), _T("AntiVirus"), &General.AntiVirus, _T("") );

	Add( _T("VersionCheck"), _T("NextCheck"), &VersionCheck.NextCheck, 0 );
	Add( _T("VersionCheck"), _T("Quote"), &VersionCheck.Quote );
	Add( _T("VersionCheck"), _T("UpdateCheck"), &VersionCheck.UpdateCheck, true );
	Add( _T("VersionCheck"), _T("UpdateCheckURL"), &VersionCheck.UpdateCheckURL, WEB_SITE_T _T("version/") );
	Add( _T("VersionCheck"), _T("UpgradeFile"), &VersionCheck.UpgradeFile );
	Add( _T("VersionCheck"), _T("UpgradePrompt"), &VersionCheck.UpgradePrompt );
	Add( _T("VersionCheck"), _T("UpgradeSHA1"), &VersionCheck.UpgradeSHA1 );
	Add( _T("VersionCheck"), _T("UpgradeSize"), &VersionCheck.UpgradeSize );
	Add( _T("VersionCheck"), _T("UpgradeSources"), &VersionCheck.UpgradeSources );
	Add( _T("VersionCheck"), _T("UpgradeTiger"), &VersionCheck.UpgradeTiger );
	Add( _T("VersionCheck"), _T("UpgradeVersion"), &VersionCheck.UpgradeVersion );

	Add( _T("Interface"), _T("AutoComplete"), &Interface.AutoComplete, true );
	Add( _T("Interface"), _T("CoolMenuEnable"), &Interface.CoolMenuEnable, true );
	Add( _T("Interface"), _T("LowResMode"), &Interface.LowResMode, false );
	Add( _T("Interface"), _T("TipAlpha"), &Interface.TipAlpha, 255, 1, 0, 255 );
	Add( _T("Interface"), _T("TipDelay"), &Interface.TipDelay, 600, 1, 100, 5000, _T(" ms") );
	Add( _T("Interface"), _T("TipDownloads"), &Interface.TipDownloads, true );
	Add( _T("Interface"), _T("TipLibrary"), &Interface.TipLibrary, true );
	Add( _T("Interface"), _T("TipMedia"), &Interface.TipMedia, true );
	Add( _T("Interface"), _T("TipNeighbours"), &Interface.TipNeighbours, true );
	Add( _T("Interface"), _T("TipSearch"), &Interface.TipSearch, true );
	Add( _T("Interface"), _T("TipUploads"), &Interface.TipUploads, true );
	Add( _T("Interface"), _T("Snarl"), &Interface.Snarl, false );
	Add( _T("Interface"), _T("SearchWindowsLimit"), &Interface.SearchWindowsLimit, 10, 1, 0, 50, _T(" windows") );
	Add( _T("Interface"), _T("BrowseWindowsLimit"), &Interface.BrowseWindowsLimit, 10, 1, 0, 50, _T(" windows") );

	Add( _T("Windows"), _T("RunWizard"), &Windows.RunWizard, false );
	Add( _T("Windows"), _T("RunWarnings"), &Windows.RunWarnings, false );
	Add( _T("Windows"), _T("RunPromote"), &Windows.RunPromote, false );

	Add( _T("Toolbars"), _T("ShowRemote"), &Toolbars.ShowRemote, true );
	Add( _T("Toolbars"), _T("ShowMonitor"), &Toolbars.ShowMonitor, true );

	Add( _T("Fonts"), _T("DefaultFont"), &Fonts.DefaultFont, NULL, false, setFont );
	Add( _T("Fonts"), _T("PacketDumpFont"), &Fonts.PacketDumpFont, NULL, false, setFont );
	Add( _T("Fonts"), _T("SystemLogFont"), &Fonts.SystemLogFont, NULL, false, setFont );
	Add( _T("Fonts"), _T("FontSize"), &Fonts.FontSize, 11, 1, 8, 48, _T(" px") );

	Add( _T("Library"), _T("CreateGhosts"), &Library.CreateGhosts, true );
	Add( _T("Library"), _T("GhostLimit"), &Library.GhostLimit, 1000, 1, 0, 100000, _T(" files") );
	Add( _T("Library"), _T("FilterURI"), &Library.FilterURI );
	Add( _T("Library"), _T("HashWindow"), &Library.HashWindow, true );
	Add( _T("Library"), _T("HighPriorityHash"), &Library.HighPriorityHash, false );
	Add( _T("Library"), _T("HighPriorityHashing"), &Library.HighPriorityHashing, 20, 1, 1, 100, _T(" MB/s") );
	Add( _T("Library"), _T("HistoryDays"), &Library.HistoryDays, 3, 1, 0, 365, _T(" days") );
	Add( _T("Library"), _T("HistoryTotal"), &Library.HistoryTotal, 32, 1, 0, 100, _T(" files") );
	Add( _T("Library"), _T("LowPriorityHashing"), &Library.LowPriorityHashing, 2, 1, 1, 100, _T(" MB/s") );
	Add( _T("Library"), _T("MarkFileAsDownload"), &Library.MarkFileAsDownload, true );
	Add( _T("Library"), _T("MaxMaliciousFileSize"), &Library.MaxMaliciousFileSize, Kilo, 1, Kilo, 5*Kilo, _T(" B") );
	Add( _T("Library"), _T("PanelSize"), &Library.PanelSize, 146, 1, 0, 1024, _T(" px") );
	Add( _T("Library"), _T("PrivateTypes"), &Library.PrivateTypes, _T("|dtapart|kdbx|ps1|ps1xml|ps2|ps2xml|psc1|psc2|ws|wsf|wsc|wsh|scf|vb|vbs|vbe|js|jse|hta|scr|application|jc!|fb!|bc!|!ut|dbx|part|partial|crdownload|pst|reget|getright|pif|lnk|sd|url|wab|infodb|racestats|chk|tmp|temp|ini|inf|log|old|manifest|met|bak|$$$|---|~~~|###|__incomplete___|") );
	Add( _T("Library"), _T("QueryRouteSize"), &Library.QueryRouteSize, 20, 1, 8, 24 );
	Add( _T("Library"), _T("SafeExecute"), &Library.SafeExecute, _T("") );
	Add( _T("Library"), _T("ScanAPE"), &Library.ScanAPE, true );
	Add( _T("Library"), _T("ScanASF"), &Library.ScanASF, true );
	Add( _T("Library"), _T("ScanAVI"), &Library.ScanAVI, true );
	Add( _T("Library"), _T("ScanCHM"), &Library.ScanCHM, true );
	Add( _T("Library"), _T("ScanEXE"), &Library.ScanEXE, true );
	Add( _T("Library"), _T("ScanFLV"), &Library.ScanFLV, true );
	Add( _T("Library"), _T("ScanImage"), &Library.ScanImage, true );
	Add( _T("Library"), _T("ScanMP3"), &Library.ScanMP3, true );
	Add( _T("Library"), _T("ScanMPEG"), &Library.ScanMPEG, true );
	Add( _T("Library"), _T("ScanMSI"), &Library.ScanMSI, true );
	Add( _T("Library"), _T("ScanOGG"), &Library.ScanOGG, true );
	Add( _T("Library"), _T("ScanPDF"), &Library.ScanPDF, true );
	Add( _T("Library"), _T("ScanProperties"), &Library.ScanProperties, true );
	Add( _T("Library"), _T("SchemaURI"), &Library.SchemaURI, CSchema::uriAudio );
	Add( _T("Library"), _T("ShowCoverArt"), &Library.ShowCoverArt, true );
	Add( _T("Library"), _T("ShowPanel"), &Library.ShowPanel, true );
	Add( _T("Library"), _T("ShowVirtual"), &Library.ShowVirtual, true );
	Add( _T("Library"), _T("SourceExpire"), &Library.SourceExpire, 24*60*60, 60, 60, 7*24*60*60, _T(" m") );
	Add( _T("Library"), _T("SourceMesh"), &Library.SourceMesh, true );
	Add( _T("Library"), _T("StoreViews"), &Library.StoreViews, true );
	Add( _T("Library"), _T("ThumbSize"), &Library.ThumbSize, 128, 1, 16, 256, _T(" px") );
	Add( _T("Library"), _T("TigerHeight"), &Library.TigerHeight, 9, 1, 1, 64 );
	Add( _T("Library"), _T("TreeSize"), &Library.TreeSize, 200, 1, 0, 1024, _T(" px") );
	Add( _T("Library"), _T("UseCustomFolders"), &Library.UseCustomFolders, true );
	Add( _T("Library"), _T("UseWindowsLibrary"), &Library.UseWindowsLibrary, true );
	Add( _T("Library"), _T("UseFolderGUID"), &Library.UseFolderGUID, true );
	Add( _T("Library"), _T("VirtualFiles"), &Library.VirtualFiles, false );
	Add( _T("Library"), _T("WatchFolders"), &Library.WatchFolders, true );
	Add( _T("Library"), _T("WatchFoldersTimeout"), &Library.WatchFoldersTimeout, 5, 1, 1, 60, _T(" s") );
	Add( _T("Library"), _T("SmartSeriesDetection"), &Library.SmartSeriesDetection, true );
	Add( _T("Library"), _T("LastUsedView"), &Library.LastUsedView );
	Add( _T("Library"), _T("URLExportFormat"), &Library.URLExportFormat, _T("<a href=\"magnet:?xt=urn:bitprint:[SHA1].[TIGER]&amp;xt=urn:ed2khash:[ED2K]&amp;xt=urn:md5:[MD5]&amp;xl=[ByteSize]&amp;dn=[NameURI]\">[Name]</a><br>") );
	Add( _T("Library"), _T("TooManyWarning"), &Library.TooManyWarning, 0, 1, 0, 2 );

	Add( _T("Search"), _T("AdultFilter"), &Search.AdultFilter, false );
	Add( _T("Search"), _T("AdvancedPanel"), &Search.AdvancedPanel, true );
	Add( _T("Search"), _T("BlankSchemaURI"), &Search.BlankSchemaURI, CSchema::uriAudio );
	Add( _T("Search"), _T("BrowseTreeSize"), &Search.BrowseTreeSize, 180 );
	Add( _T("Search"), _T("DetailPanelSize"), &Search.DetailPanelSize, 100 );
	Add( _T("Search"), _T("DetailPanelVisible"), &Search.DetailPanelVisible, true );
	Add( _T("Search"), _T("ExpandMatches"), &Search.ExpandMatches, false );
	Add( _T("Search"), _T("FilterMask"), &Search.FilterMask, 0x168 );
	Add( _T("Search"), _T("GeneralThrottle"), &Search.GeneralThrottle, 200, 1, 200, 1000, _T(" ms") );
	Add( _T("Search"), _T("HideSearchPanel"), &Search.HideSearchPanel, false );
	Add( _T("Search"), _T("HighlightNew"), &Search.HighlightNew, true );
	Add( _T("Search"), _T("LastSchemaURI"), &Search.LastSchemaURI );
	Add( _T("Search"), _T("MaxPreviewLength"), &Search.MaxPreviewLength, 20*Kilo, Kilo, 1, 4*Kilo, _T(" KB") );
	Add( _T("Search"), _T("MonitorFilter"), &Search.MonitorFilter );
	Add( _T("Search"), _T("MonitorQueue"), &Search.MonitorQueue, 128, 1, 1, 4096 );
	Add( _T("Search"), _T("MonitorSchemaURI"), &Search.MonitorSchemaURI, CSchema::uriAudio );
	Add( _T("Search"), _T("SchemaTypes"), &Search.SchemaTypes, true );
	Add( _T("Search"), _T("SearchPanel"), &Search.SearchPanel, true );
	Add( _T("Search"), _T("ShowNames"), &Search.ShowNames, true );
	Add( _T("Search"), _T("SpamFilterThreshold"), &Search.SpamFilterThreshold, 20, 1, 0, 100, _T("%") );
	Add( _T("Search"), _T("SwitchToTransfers"), &Search.SwitchToTransfers, true );
	Add( _T("Search"), _T("ClearPrevious"), &Search.ClearPrevious, 0, 1, 0, 2 );
	Add( _T("Search"), _T("SanityCheck"), &Search.SanityCheck, true );
	Add( _T("Search"), _T("AutoPreview"), &Search.AutoPreview, true );

	Add( _T("MediaPlayer"), _T("Aspect"), &MediaPlayer.Aspect, smaDefault );
	Add( _T("MediaPlayer"), _T("AviPreviewCLSID"), &MediaPlayer.AviPreviewCLSID, _T("{394011F0-6D5C-42a3-96C6-24B9AD6B010C}") );
	Add( _T("MediaPlayer"), _T("EnableEnqueue"), &MediaPlayer.EnableEnqueue, true );
	Add( _T("MediaPlayer"), _T("EnablePlay"), &MediaPlayer.EnablePlay, true );
	Add( _T("MediaPlayer"), _T("FileTypes"), &MediaPlayer.FileTypes, _T("") );
	Add( _T("MediaPlayer"), _T("ListSize"), &MediaPlayer.ListSize, 240 );
	Add( _T("MediaPlayer"), _T("ListVisible"), &MediaPlayer.ListVisible, true );
	Add( _T("MediaPlayer"), _T("MediaServicesCLSID"), &MediaPlayer.MediaServicesCLSID, _T("{3DC28AA6-A597-4E03-96DF-ADA19155B0BE}") );
	Add( _T("MediaPlayer"), _T("Mp3PreviewCLSID"), &MediaPlayer.Mp3PreviewCLSID, _T("{BF00DBCC-90A2-4f46-8171-7D4F929D035F}") );
	Add( _T("MediaPlayer"), _T("Mpeg1PreviewCLSID"), &MediaPlayer.Mpeg1PreviewCLSID, _T("{9AA8DF47-B8FE-47da-AB1A-2DAA0DA0B646}") );
	Add( _T("MediaPlayer"), _T("Random"), &MediaPlayer.Random, false );
	Add( _T("MediaPlayer"), _T("Repeat"), &MediaPlayer.Repeat, false );
	Add( _T("MediaPlayer"), _T("ServicePath"), &MediaPlayer.ServicePath,_T(""));
	Add( _T("MediaPlayer"), _T("ShortPaths"), &MediaPlayer.ShortPaths, false );
	Add( _T("MediaPlayer"), _T("StatusVisible"), &MediaPlayer.StatusVisible, true );
	Add( _T("MediaPlayer"), _T("VisCLSID"), &MediaPlayer.VisCLSID, _T("{591A5CFF-3172-4020-A067-238542DDE9C2}") );
	Add( _T("MediaPlayer"), _T("VisPath"), &MediaPlayer.VisPath );
	Add( _T("MediaPlayer"), _T("VisSize"), &MediaPlayer.VisSize, 1 );
	Add( _T("MediaPlayer"), _T("VisSoniqueCLSID"), &MediaPlayer.VisSoniqueCLSID, _T("{D07E630D-A850-4f11-AD29-3D3848B67EFE}") );
	Add( _T("MediaPlayer"), _T("VisWrapperCLSID"), &MediaPlayer.VisWrapperCLSID, _T("{C3B7B25C-6B8B-481A-BC48-59F9A6F7B69A}") );
	Add( _T("MediaPlayer"), _T("Volume"), &MediaPlayer.Volume, 1.0f );
	Add( _T("MediaPlayer"), _T("Zoom"), (DWORD*)&MediaPlayer.Zoom, smzFill );

	Add( _T("Web"), _T("ED2K"), &Web.ED2K, true );
	Add( _T("Web"), _T("Gnutella"), &Web.Gnutella, true );
	Add( _T("Web"), _T("Magnet"), &Web.Magnet, true );
	Add( _T("Web"), _T("Foxy"), &Web.Foxy, true );
	Add( _T("Web"), _T("Piolet"), &Web.Piolet, true );
	Add( _T("Web"), _T("Torrent"), &Web.Torrent, true );
	Add( _T("Web"), _T("DC"), &Web.DC, true );

	Add( _T("Connection"), _T("AutoConnect"), &Connection.AutoConnect, true );
	Add( _T("Connection"), _T("ConnectThrottle"), &Connection.ConnectThrottle, 250, 1, 0, 5000, _T(" ms") );
	Add( _T("Connection"), _T("DeleteFirewallException"), &Connection.DeleteFirewallException, false );
	Add( _T("Connection"), _T("DeleteUPnPPorts"), &Connection.DeleteUPnPPorts, true );
	Add( _T("Connection"), _T("DetectConnectionLoss"), &Connection.DetectConnectionLoss, true );
	Add( _T("Connection"), _T("DetectConnectionReset"), &Connection.DetectConnectionReset, false );
	Add( _T("Connection"), _T("EnableFirewallException"), &Connection.EnableFirewallException, true );
	Add( _T("Connection"), _T("EnableUPnP"), &Connection.EnableUPnP, true );
	Add( _T("Connection"), _T("FailureLimit"), &Connection.FailureLimit, 3, 1, 1, 512 );
	Add( _T("Connection"), _T("FailurePenalty"), &Connection.FailurePenalty, 300, 1, 30, 3600, _T(" s") );
	Add( _T("Connection"), _T("FirewallState"), &Connection.FirewallState, CONNECTION_AUTO, 1, CONNECTION_AUTO, CONNECTION_OPEN_UDPONLY );
	Add( _T("Connection"), _T("ForceConnectedState"), &Connection.ForceConnectedState, true );
	Add( _T("Connection"), _T("IgnoreLocalIP"), &Connection.IgnoreLocalIP, true );
	Add( _T("Connection"), _T("IgnoreOwnIP"), &Connection.IgnoreOwnIP, true );
	Add( _T("Connection"), _T("IgnoreOwnUDP"), &Connection.IgnoreOwnUDP, true );
	Add( _T("Connection"), _T("InBind"), &Connection.InBind, false );
	Add( _T("Connection"), _T("InHost"), &Connection.InHost );
	Add( _T("Connection"), _T("InPort"), &Connection.InPort, protocolPorts[ PROTOCOL_G2 ], 1, 1, 65535 );
#ifdef LAN_MODE
	Add( _T("Connection"), _T("InSpeed"), &Connection.InSpeed, 40960 );
	Add( _T("Connection"), _T("OutSpeed"), &Connection.OutSpeed, 40960 );
#else  // LAN_MODE
	Add( _T("Connection"), _T("InSpeed"), &Connection.InSpeed, 2048 );
	Add( _T("Connection"), _T("OutSpeed"), &Connection.OutSpeed, 256 );
#endif // LAN_MODE
	Add( _T("Connection"), _T("OutHost"), &Connection.OutHost );
	Add( _T("Connection"), _T("RandomPort"), &Connection.RandomPort, false );
	Add( _T("Connection"), _T("RequireForTransfers"), &Connection.RequireForTransfers, true );
	Add( _T("Connection"), _T("SendBuffer"), &Connection.SendBuffer, 8*Kilo, 1, 0, 64*Kilo, _T(" B") );
	Add( _T("Connection"), _T("SkipWANIPSetup"), &Connection.SkipWANIPSetup, false );
	Add( _T("Connection"), _T("SkipWANPPPSetup"), &Connection.SkipWANPPPSetup, false );
	Add( _T("Connection"), _T("SlowConnect"), &Connection.SlowConnect, false );
	Add( _T("Connection"), _T("TimeoutConnect"), &Connection.TimeoutConnect, 16*1000, 1000, 1, 2*60, _T(" s") );
	Add( _T("Connection"), _T("TimeoutHandshake"), &Connection.TimeoutHandshake, 45*1000, 1000, 1, 5*60, _T(" s") );
	Add( _T("Connection"), _T("TimeoutTraffic"), &Connection.TimeoutTraffic, 140*1000, 1000, 10, 60*60, _T(" s") );
	Add( _T("Connection"), _T("UPnPRefreshTime"), &Connection.UPnPRefreshTime, 30*60*1000, 60*1000, 5, 24*60, _T(" m") );
	Add( _T("Connection"), _T("UPnPTimeout"), &Connection.UPnPTimeout, 5*1000, 1, 0, 60*1000, _T(" ms") );
	Add( _T("Connection"), _T("ZLibCompressionLevel"), &Connection.ZLibCompressionLevel, 9, 1, 0, 9, _T(" level") ); 
	Add( _T("Connection"), _T("EnableMulticast"), &Connection.EnableMulticast, true );
	Add( _T("Connection"), _T("MulticastLoop"), &Connection.MulticastLoop, false );
	Add( _T("Connection"), _T("MulticastTTL"), &Connection.MulticastTTL, 1, 1, 0, 255 );
#ifdef LAN_MODE
	Add( _T("Connection"), _T("EnableBroadcast"), &Connection.EnableBroadcast, true );
#else  // LAN_MODE
	Add( _T("Connection"), _T("EnableBroadcast"), &Connection.EnableBroadcast, false );
#endif // LAN_MODE

	Add( _T("Bandwidth"), _T("Downloads"), &Bandwidth.Downloads, 0 );
	Add( _T("Bandwidth"), _T("HubIn"), &Bandwidth.HubIn, 0, 128, 0, 8192, _T(" Kb/s") );
	Add( _T("Bandwidth"), _T("HubOut"), &Bandwidth.HubOut, 0, 128, 0, 8192, _T(" Kb/s") );
	Add( _T("Bandwidth"), _T("LeafIn"), &Bandwidth.LeafIn, 0, 128, 0, 8192, _T(" Kb/s") );
	Add( _T("Bandwidth"), _T("LeafOut"), &Bandwidth.LeafOut, 0, 128, 0, 8192, _T(" Kb/s") );
	Add( _T("Bandwidth"), _T("PeerIn"), &Bandwidth.PeerIn, 0, 128, 0, 8192, _T(" Kb/s") );
	Add( _T("Bandwidth"), _T("PeerOut"), &Bandwidth.PeerOut, 0, 128, 0, 8192, _T(" Kb/s") );
	Add( _T("Bandwidth"), _T("Request"), &Bandwidth.Request, 32*128, 128, 0, 8192, _T(" Kb/s") );
	Add( _T("Bandwidth"), _T("UdpOut"), &Bandwidth.UdpOut, 0, 128, 0, 8192, _T(" Kb/s") );
	Add( _T("Bandwidth"), _T("Uploads"), &Bandwidth.Uploads, 0 );

	Add( _T("Community"), _T("AwayMessageIdleTime"), &Community.AwayMessageIdleTime, 20*60, 60, 5, 60, _T(" m") );
	Add( _T("Community"), _T("ChatAllNetworks"), &Community.ChatAllNetworks, true );
	Add( _T("Community"), _T("ChatCensor"), &Community.ChatCensor, false );
	Add( _T("Community"), _T("ChatEnable"), &Community.ChatEnable, true );
	Add( _T("Community"), _T("ChatFilter"), &Community.ChatFilter, true );
	Add( _T("Community"), _T("ChatFilterED2K"), &Community.ChatFilterED2K, true );
	Add( _T("Community"), _T("ServeFiles"), &Community.ServeFiles, true );
	Add( _T("Community"), _T("ServeProfile"), &Community.ServeProfile, true );
	Add( _T("Community"), _T("Timestamp"), &Community.Timestamp, true );
	Add( _T("Community"), _T("UserPanelSize"), &Community.UserPanelSize, 200, 1, 0, 1024, _T(" px") );

	Add( _T("Discovery"), _T("AccessThrottle"), &Discovery.AccessThrottle, 60*60, 60, 1, 180, _T(" m") );
	Add( _T("Discovery"), _T("BootstrapCount"), &Discovery.BootstrapCount, 10, 1, 0, 20 );
	Add( _T("Discovery"), _T("CacheCount"), &Discovery.CacheCount, 50, 1, 1, 256 );
	Add( _T("Discovery"), _T("DefaultUpdate"), &Discovery.DefaultUpdate, 60*60, 60, 1, 60*24, _T(" m") );
	Add( _T("Discovery"), _T("FailureLimit"), &Discovery.FailureLimit, 2, 1, 1, 512 );
	Add( _T("Discovery"), _T("Lowpoint"), &Discovery.Lowpoint, 10, 1, 1, 512 );
	Add( _T("Discovery"), _T("AccessPeriod"), &Discovery.AccessPeriod, 30*60, 60, 1, 60*24, _T(" m") );

	Add( _T("Gnutella"), _T("ConnectFactor"), &Gnutella.ConnectFactor, 4, 1, 1, 20, _T("x") );
	Add( _T("Gnutella"), _T("ConnectThrottle"), &Gnutella.ConnectThrottle, 30, 1, 0, 60*60, _T(" s") );
	Add( _T("Gnutella"), _T("DeflateHub2Hub"), &Gnutella.DeflateHub2Hub, true );
	Add( _T("Gnutella"), _T("DeflateHub2Leaf"), &Gnutella.DeflateHub2Leaf, true );
	Add( _T("Gnutella"), _T("DeflateLeaf2Hub"), &Gnutella.DeflateLeaf2Hub, false );
	Add( _T("Gnutella"), _T("HitsPerPacket"), &Gnutella.HitsPerPacket, 8, 1, 1, 8, _T(" files") );
	Add( _T("Gnutella"), _T("HostCacheSize"), &Gnutella.HostCacheSize, 1024, 1, 32, 16384, _T(" hosts") );
	Add( _T("Gnutella"), _T("HostCacheView"), &Gnutella.HostCacheView, PROTOCOL_ED2K );
	Add( _T("Gnutella"), _T("MaxHits"), &Gnutella.MaxHits, 64, 1, 0, 4096, _T(" files") );
	Add( _T("Gnutella"), _T("MaximumPacket"), &Gnutella.MaximumPacket, 64 * Kilo, Kilo, 32, 256, _T(" KB") );
	Add( _T("Gnutella"), _T("MaxResults"), &Gnutella.MaxResults, 150, 1, 1, 300, _T(" hits") );
	Add( _T("Gnutella"), _T("RouteCache"), &Gnutella.RouteCache, 600, 60, 1, 120, _T(" m") );
	Add( _T("Gnutella"), _T("SpecifyProtocol"), &Gnutella.SpecifyProtocol, true );

	Add( _T("Gnutella1"), _T("ClientMode"), &Gnutella1.ClientMode, MODE_LEAF, 1, MODE_AUTO, MODE_HUB );
	Add( _T("Gnutella1"), _T("DefaultTTL"), &Gnutella1.DefaultTTL, 3, 1, 1, 3 );
	Add( _T("Gnutella1"), _T("EnableAlways"), &Gnutella1.EnableAlways, false );
	Add( _T("Gnutella1"), _T("EnableGGEP"), &Gnutella1.EnableGGEP, true );
	Add( _T("Gnutella1"), _T("EnableOOB"), &Gnutella1.EnableOOB, false );	// TODO: Change this to "true" after OOB fully implemented.
	Add( _T("Gnutella1"), _T("HostCount"), &Gnutella1.HostCount, 15, 1, 1, 50 );
	Add( _T("Gnutella1"), _T("HostExpire"), &Gnutella1.HostExpire, 2*24*60*60, 24*60*60, 1, 100, _T(" d") );
	Add( _T("Gnutella1"), _T("MaxHostsInPongs"), &Gnutella1.MaxHostsInPongs, 10, 1, 5, 30 );
	Add( _T("Gnutella1"), _T("MaximumQuery"), &Gnutella1.MaximumQuery, 256, 1, 32, 262144 );
	Add( _T("Gnutella1"), _T("MaximumTTL"), &Gnutella1.MaximumTTL, 10, 1, 1, 10 );
	Add( _T("Gnutella1"), _T("MCastPingRate"), &Gnutella1.MCastPingRate, 60*1000, 1000, 60, 60*60, _T(" s") );
	Add( _T("Gnutella1"), _T("NumHubs"), &Gnutella1.NumHubs, 3, 1, 1, 5 );
	Add( _T("Gnutella1"), _T("NumLeafs"), &Gnutella1.NumLeafs, 50, 1, 5, 1024 );
	Add( _T("Gnutella1"), _T("NumPeers"), &Gnutella1.NumPeers, 32, 1, 15, 64 ); // For X-Degree
	Add( _T("Gnutella1"), _T("PacketBufferSize"), &Gnutella1.PacketBufferSize, 64, 1, 1, 1024, _T(" packets") );
	Add( _T("Gnutella1"), _T("PacketBufferTime"), &Gnutella1.PacketBufferTime, 60000, 1000, 10, 180, _T(" s") );
	Add( _T("Gnutella1"), _T("PingFlood"), &Gnutella1.PingFlood, 3000, 1000, 0, 30, _T(" s") );
	Add( _T("Gnutella1"), _T("PingRate"), &Gnutella1.PingRate, 30000, 1000, 15, 180, _T(" s") );
	Add( _T("Gnutella1"), _T("PongCache"), &Gnutella1.PongCache, 10000, 1000, 1, 180, _T(" s") );
	Add( _T("Gnutella1"), _T("PongCount"), &Gnutella1.PongCount, 10, 1, 1, 64 );
	Add( _T("Gnutella1"), _T("QueryGlobalThrottle"), &Gnutella1.QueryGlobalThrottle, 60*1000, 1000, 60, 60*60, _T(" s") );
	Add( _T("Gnutella1"), _T("QueryHitUTF8"), &Gnutella1.QueryHitUTF8, true );
	Add( _T("Gnutella1"), _T("QuerySearchUTF8"), &Gnutella1.QuerySearchUTF8, true );
	Add( _T("Gnutella1"), _T("QueryThrottle"), &Gnutella1.QueryThrottle, 120, 1, 30, 60*60, _T(" s") );
	Add( _T("Gnutella1"), _T("SearchTTL"), &Gnutella1.SearchTTL, 3, 1, 1, 3 );
	Add( _T("Gnutella1"), _T("TranslateTTL"), &Gnutella1.TranslateTTL, 2, 1, 1, 2 );
	Add( _T("Gnutella1"), _T("VendorMsg"), &Gnutella1.VendorMsg, true );

	Add( _T("Gnutella2"), _T("ClientMode"), &Gnutella2.ClientMode, MODE_AUTO );
	Add( _T("Gnutella2"), _T("EnableAlways"), &Gnutella2.EnableAlways, true );
	Add( _T("Gnutella2"), _T("HAWPeriod"), &Gnutella2.HAWPeriod, 5*60*1000, 1000, 1, 60*60, _T(" s") );
	Add( _T("Gnutella2"), _T("HostCurrent"), &Gnutella2.HostCurrent, 10*60, 60, 1, 24*60, _T(" m") );
	Add( _T("Gnutella2"), _T("HostCount"), &Gnutella2.HostCount, 15, 1, 1, 50 );
	Add( _T("Gnutella2"), _T("HostExpire"), &Gnutella2.HostExpire, 2*24*60*60, 24*60*60, 1, 100, _T(" d") );
	Add( _T("Gnutella2"), _T("HubHorizonSize"), &Gnutella2.HubHorizonSize, 128, 1, 32, 512 );
	Add( _T("Gnutella2"), _T("HubVerified"), &Gnutella2.HubVerified, false );
	Add( _T("Gnutella2"), _T("KHLHubCount"), &Gnutella2.KHLHubCount, 50, 1, 1, 256 );
	Add( _T("Gnutella2"), _T("KHLPeriod"), &Gnutella2.KHLPeriod, 60*1000, 1000, 1, 60*60, _T(" s") );
	Add( _T("Gnutella2"), _T("LNIPeriod"), &Gnutella2.LNIPeriod, 60*1000, 1000, 1, 60*60, _T(" s") );
#ifdef LAN_MODE
	Add( _T("Gnutella2"), _T("NumHubs"), &Gnutella2.NumHubs, 1, 1, 1, 3 );
	Add( _T("Gnutella2"), _T("NumLeafs"), &Gnutella2.NumLeafs, 1024, 1, 50, 1024 );
	Add( _T("Gnutella2"), _T("NumPeers"), &Gnutella2.NumPeers, 1, 1, 0, 64 );
#else // LAN_MODE
	Add( _T("Gnutella2"), _T("NumHubs"), &Gnutella2.NumHubs, 2, 1, 1, 3 );
	Add( _T("Gnutella2"), _T("NumLeafs"), &Gnutella2.NumLeafs, 300, 1, 50, 1024 );
	Add( _T("Gnutella2"), _T("NumPeers"), &Gnutella2.NumPeers, 6, 1, 4, 64 );
#endif // LAN_MODE
	Add( _T("Gnutella2"), _T("PingRate"), &Gnutella2.PingRate, 15000, 1000, 5, 180, _T(" s") );
	Add( _T("Gnutella2"), _T("PingRelayLimit"), &Gnutella2.PingRelayLimit, 10, 1, 10, 30 );
	Add( _T("Gnutella2"), _T("QueryGlobalThrottle"), &Gnutella2.QueryGlobalThrottle, 125, 1, 1, 60*1000, _T(" ms") );
	Add( _T("Gnutella2"), _T("QueryHostDeadline"), &Gnutella2.QueryHostDeadline, 10*60, 1, 1, 120*60, _T(" s") );
	Add( _T("Gnutella2"), _T("QueryThrottle"), &Gnutella2.QueryThrottle, 120, 1, 30, 60*60, _T(" s") );
	Add( _T("Gnutella2"), _T("QueryLimit"), &Gnutella2.QueryLimit, 2400, 1, 0, 10000 );
	Add( _T("Gnutella2"), _T("RequeryDelay"), &Gnutella2.RequeryDelay, 4*60*60, 60*60, 1, 24, _T(" h") );
	Add( _T("Gnutella2"), _T("UdpBuffers"), &Gnutella2.UdpBuffers, 512, 1, 16, 2048 );
	Add( _T("Gnutella2"), _T("UdpGlobalThrottle"), &Gnutella2.UdpGlobalThrottle, 1, 1, 0, 10000 );
	Add( _T("Gnutella2"), _T("UdpInExpire"), &Gnutella2.UdpInExpire, 30000, 1000, 1, 300, _T(" s") );
	Add( _T("Gnutella2"), _T("UdpInFrames"), &Gnutella2.UdpInFrames, 256, 1, 16, 2048 );
	Add( _T("Gnutella2"), _T("UdpMTU"), &Gnutella2.UdpMTU, 500, 1, 16, 10*Kilo, _T(" B") );
	Add( _T("Gnutella2"), _T("UdpOutExpire"), &Gnutella2.UdpOutExpire, 26000, 1000, 1, 300, _T(" s") );
	Add( _T("Gnutella2"), _T("UdpOutFrames"), &Gnutella2.UdpOutFrames, 256, 1, 16, 2048 );
	Add( _T("Gnutella2"), _T("UdpOutResend"), &Gnutella2.UdpOutResend, 6000, 1000, 1, 300, _T(" s") );

	Add( _T("eDonkey"), _T("DefaultServerFlags"), &eDonkey.DefaultServerFlags, 0xFFFFFFFF );
	Add( _T("eDonkey"), _T("DequeueTime"), &eDonkey.DequeueTime, 3600, 60, 2, 512, _T(" m") );
	Add( _T("eDonkey"), _T("EnableAlways"), &eDonkey.EnableAlways, false );
	Add( _T("eDonkey"), _T("Endgame"), &eDonkey.Endgame, true );
	Add( _T("eDonkey"), _T("ExtendedRequest"), &eDonkey.ExtendedRequest, 2, 1, 0, 2 );
	Add( _T("eDonkey"), _T("FastConnect"), &eDonkey.FastConnect, false );
	Add( _T("eDonkey"), _T("ForceHighID"), &eDonkey.ForceHighID, true );
	Add( _T("eDonkey"), _T("FrameSize"), &eDonkey.FrameSize, 10*Kilo, Kilo, 1, Kilo, _T(" KB") );
	Add( _T("eDonkey"), _T("GetSourcesThrottle"), &eDonkey.GetSourcesThrottle, 8*60*60*1000, 60*60*1000, 1, 24, _T(" h") );
	Add( _T("eDonkey"), _T("LargeFileSupport"), &eDonkey.LargeFileSupport, true );
	Add( _T("eDonkey"), _T("LearnNewServers"), &eDonkey.LearnNewServers, false );
	Add( _T("eDonkey"), _T("LearnNewServersClient"), &eDonkey.LearnNewServersClient, false );
	Add( _T("eDonkey"), _T("MagnetSearch"), &eDonkey.MagnetSearch, true );
	Add( _T("eDonkey"), _T("MaxLinks"), &eDonkey.MaxLinks, 200, 1, 1, 2048 );
	Add( _T("eDonkey"), _T("MaxResults"), &eDonkey.MaxResults, 100, 1, 1, 200 );
	Add( _T("eDonkey"), _T("MaxShareCount"), &eDonkey.MaxShareCount, 1000, 1, 25, 20000 );
	Add( _T("eDonkey"), _T("MinServerFileSize"), &eDonkey.MinServerFileSize, 0, 1, 0, 50, _T(" MB") );
	Add( _T("eDonkey"), _T("NumServers"), &eDonkey.NumServers, 1, 1, 0, 1 );
	Add( _T("eDonkey"), _T("PacketThrottle"), &eDonkey.PacketThrottle, 500, 1, 250, 5000, _T(" ms") );
	Add( _T("eDonkey"), _T("QueryFileThrottle"), &eDonkey.QueryFileThrottle, 60*60*1000, 60*1000, 30, 120, _T(" m") );
	Add( _T("eDonkey"), _T("QueryGlobalThrottle"), &eDonkey.QueryGlobalThrottle, 1000, 1, 1000, 20000, _T(" ms") );
	Add( _T("eDonkey"), _T("QueryThrottle"), &eDonkey.QueryThrottle, 120, 1, 60, 60*60, _T(" s") );
	Add( _T("eDonkey"), _T("QueueRankThrottle"), &eDonkey.QueueRankThrottle, 2*60*1000, 1000, 60, 600, _T(" s") );
	Add( _T("eDonkey"), _T("ReAskTime"), &eDonkey.ReAskTime, 29*60, 60, 20, 360, _T(" m") );
	Add( _T("eDonkey"), _T("RequestPipe"), &eDonkey.RequestPipe, 3, 1, 1, 10 );
	Add( _T("eDonkey"), _T("RequestSize"), &eDonkey.RequestSize, 90*Kilo, Kilo, 10, Kilo, _T(" KB") );
	Add( _T("eDonkey"), _T("SendPortServer"), &eDonkey.SendPortServer, false );
	Add( _T("eDonkey"), _T("ServerListURL"), &eDonkey.ServerListURL, _T("http://peerates.net/servers.php") );
	Add( _T("eDonkey"), _T("ServerWalk"), &eDonkey.ServerWalk, true );
	Add( _T("eDonkey"), _T("SourceThrottle"), &eDonkey.SourceThrottle, 1000, 1, 250, 5000, _T(" ms") );
	Add( _T("eDonkey"), _T("StatsGlobalThrottle"), &eDonkey.StatsGlobalThrottle, 30*60*1000, 60*1000, 30, 120, _T(" m") );
	Add( _T("eDonkey"), _T("StatsServerThrottle"), &eDonkey.StatsServerThrottle, 4*60*60, 60, 1, 7*24*60, _T(" m") );
	Add( _T("eDonkey"), _T("AutoDiscovery"), &eDonkey.AutoDiscovery, true );

	Add( _T("DC"), _T("DequeueTime"), &DC.DequeueTime, 5*60*1000, 1000, 2*60, 60*60, _T(" s") );
	Add( _T("DC"), _T("EnableAlways"), &DC.EnableAlways, false );
	Add( _T("DC"), _T("NumServers"), &DC.NumServers, 1, 1, 0, 5 );
	Add( _T("DC"), _T("QueryThrottle"), &DC.QueryThrottle, 2*60, 1, 30, 60*60, _T(" s") );
	Add( _T("DC"), _T("ReAskTime"), &DC.ReAskTime, 60*1000, 1000, 30, 60*60, _T(" s") );
	Add( _T("DC"), _T("AutoDiscovery"), &DC.AutoDiscovery, true );

	Add( _T("BitTorrent"), _T("AutoClear"), &BitTorrent.AutoClear, false );
	Add( _T("BitTorrent"), _T("AutoMerge"), &BitTorrent.AutoMerge, true );
	Add( _T("BitTorrent"), _T("AutoSeed"), &BitTorrent.AutoSeed, true );
	Add( _T("BitTorrent"), _T("BandwidthPercentage"), &BitTorrent.BandwidthPercentage, 80, 1, 50, 95, _T(" %") );
	Add( _T("BitTorrent"), _T("ClearRatio"), &BitTorrent.ClearRatio, 120, 1, 100, 999, _T(" %") );
	Add( _T("BitTorrent"), _T("ConnectThrottle"), &BitTorrent.ConnectThrottle, 6*60, 1, 0, 60*60, _T(" s") );
	Add( _T("BitTorrent"), _T("DefaultTracker"), &BitTorrent.DefaultTracker, _T("udp://tracker.openbittorrent.com:80") );
	Add( _T("BitTorrent"), _T("DefaultTrackerPeriod"), &BitTorrent.DefaultTrackerPeriod, 5*60000, 60000, 5, 120, _T(" m") );
	Add( _T("BitTorrent"), _T("DownloadConnections"), &BitTorrent.DownloadConnections, 40, 1, 1, 800 );
	Add( _T("BitTorrent"), _T("DownloadTorrents"), &BitTorrent.DownloadTorrents, 3, 1, 1, 10 );
	Add( _T("BitTorrent"), _T("EnableAlways"), &BitTorrent.EnableAlways, true );
	Add( _T("BitTorrent"), _T("EnableDHT"), &BitTorrent.EnableDHT, true );
	Add( _T("BitTorrent"), _T("EnablePromote"), &BitTorrent.EnablePromote, true );
	Add( _T("BitTorrent"), _T("Endgame"), &BitTorrent.Endgame, true );
	Add( _T("BitTorrent"), _T("HostExpire"), &BitTorrent.HostExpire, 2*24*60*60, 24*60*60, 1, 100, _T(" d") );
	Add( _T("BitTorrent"), _T("LinkPing"), &BitTorrent.LinkPing, 120*1000, 1000, 10, 60*10, _T(" s") );
	Add( _T("BitTorrent"), _T("LinkTimeout"), &BitTorrent.LinkTimeout, 180*1000, 1000, 10, 60*10, _T(" s") );
	Add( _T("BitTorrent"), _T("PeerID"), &BitTorrent.PeerID, _T("") );
	Add( _T("BitTorrent"), _T("PreferenceBTSources"), &BitTorrent.PreferenceBTSources, true );
	Add( _T("BitTorrent"), _T("QueryHostDeadline"), &BitTorrent.QueryHostDeadline, 30, 1, 1, 60*60, _T(" s") );
	Add( _T("BitTorrent"), _T("RandomPeriod"), &BitTorrent.RandomPeriod, 30*1000, 1000, 1, 60*5, _T(" s") );
	Add( _T("BitTorrent"), _T("RequestLimit"), &BitTorrent.RequestLimit, 128*Kilo, Kilo, 1, 1024, _T(" KB") );
	Add( _T("BitTorrent"), _T("RequestPipe"), &BitTorrent.RequestPipe, 4, 1, 1, 10 );
	Add( _T("BitTorrent"), _T("RequestSize"), &BitTorrent.RequestSize, 16*Kilo, Kilo, 8, 128, _T(" KB") );
	Add( _T("BitTorrent"), _T("SourceExchangePeriod"), &BitTorrent.SourceExchangePeriod, 10, 1, 1, 60*5, _T(" m") );
	Add( _T("BitTorrent"), _T("TorrentCodePage"), &BitTorrent.TorrentCodePage, 0, 1, 0, 9999999 );
	Add( _T("BitTorrent"), _T("TorrentCreatorPath"), &BitTorrent.TorrentCreatorPath );
	Add( _T("BitTorrent"), _T("TrackerKey"), &BitTorrent.TrackerKey, true );
	Add( _T("BitTorrent"), _T("UploadCount"), &BitTorrent.UploadCount, 4, 1, 2, 16 );
	Add( _T("BitTorrent"), _T("UtPexPeriod"), &BitTorrent.UtPexPeriod, 60*1000, 1000, 10, 60*10, _T(" s") );

	Add( _T("Downloads"), _T("AllowBackwards"), &Downloads.AllowBackwards, true );
	Add( _T("Downloads"), _T("AutoClear"), &Downloads.AutoClear, false );
	Add( _T("Downloads"), _T("AutoExpand"), &Downloads.AutoExpand, false );
	Add( _T("Downloads"), _T("BufferSize"), &Downloads.BufferSize, 80*Kilo, Kilo, 0, 512, _T(" KB") );
	Add( _T("Downloads"), _T("ChunkSize"), &Downloads.ChunkSize, 512*Kilo, Kilo, 0, 10*Kilo, _T(" KB") );
	Add( _T("Downloads"), _T("ChunkStrap"), &Downloads.ChunkStrap, 128*Kilo, Kilo, 0, 10*Kilo, _T(" KB") );
	Add( _T("Downloads"), _T("ClearDelay"), &Downloads.ClearDelay, 30*1000, 1000, 1, 30*60, _T(" s") );
	Add( _T("Downloads"), _T("CollectionPath"), &Downloads.CollectionPath );
	Add( _T("Downloads"), _T("CompletePath"), &Downloads.CompletePath );
	Add( _T("Downloads"), _T("ConnectThrottle"), &Downloads.ConnectThrottle, 250, 1, 0, 5000, _T(" ms") );
	Add( _T("Downloads"), _T("FilterMask"), &Downloads.FilterMask, 0xFFFFFFFF );
	Add( _T("Downloads"), _T("FlushSD"), &Downloads.FlushSD, true );
	Add( _T("Downloads"), _T("IncompletePath"), &Downloads.IncompletePath );
	Add( _T("Downloads"), _T("MaxAllowedFailures"), &Downloads.MaxAllowedFailures, 10, 1, 3, 40 );
	Add( _T("Downloads"), _T("MaxConnectingSources"), &Downloads.MaxConnectingSources, 28, 1, 5, 50 );
	Add( _T("Downloads"), _T("MaxFileSearches"), &Downloads.MaxFileSearches, 2, 1, 0, 5 );
	Add( _T("Downloads"), _T("MaxFileTransfers"), &Downloads.MaxFileTransfers, 10, 1, 1, 250 );
	Add( _T("Downloads"), _T("MaxFiles"), &Downloads.MaxFiles, 20, 1, 1, 100 );
	Add( _T("Downloads"), _T("MaxReviews"), &Downloads.MaxReviews, 64, 1, 0, 256 );
	Add( _T("Downloads"), _T("MaxTransfers"), &Downloads.MaxTransfers, 100, 1, 1, 250 );
	Add( _T("Downloads"), _T("Metadata"), &Downloads.Metadata, true );
	Add( _T("Downloads"), _T("MinSources"), &Downloads.MinSources, 1, 1, 0, 6 );
	Add( _T("Downloads"), _T("NeverDrop"), &Downloads.NeverDrop, false );
	Add( _T("Downloads"), _T("PushTimeout"), &Downloads.PushTimeout, 45*1000, 1000, 5, 180, _T(" s") );
	Add( _T("Downloads"), _T("QueueLimit"), &Downloads.QueueLimit, 0, 1, 0, 20000 );
	Add( _T("Downloads"), _T("RequestHTTP11"), &Downloads.RequestHTTP11, true );
	Add( _T("Downloads"), _T("RequestHash"), &Downloads.RequestHash, true );
	Add( _T("Downloads"), _T("RequestURLENC"), &Downloads.RequestURLENC, true );
	Add( _T("Downloads"), _T("RetryDelay"), &Downloads.RetryDelay, 10*60*1000, 1000, 120, 60*60, _T(" s") );
	Add( _T("Downloads"), _T("SaveInterval"), &Downloads.SaveInterval, 60*1000, 1000, 1, 120, _T(" s") );
	Add( _T("Downloads"), _T("SearchPeriod"), &Downloads.SearchPeriod, 120*1000, 1000, 10, 4*60, _T(" s") );
	Add( _T("Downloads"), _T("ShowGroups"), &Downloads.ShowGroups, true );
	Add( _T("Downloads"), _T("ShowMonitorURLs"), &Downloads.ShowMonitorURLs, true );
	Add( _T("Downloads"), _T("ShowPercent"), &Downloads.ShowPercent, false );
	Add( _T("Downloads"), _T("ShowSources"), &Downloads.ShowSources, false );
	Add( _T("Downloads"), _T("SimpleBar"), &Downloads.SimpleBar, false );
	Add( _T("Downloads"), _T("SortColumns"), &Downloads.SortColumns, true );
	Add( _T("Downloads"), _T("SortSources"), &Downloads.SortSources, true );
	Add( _T("Downloads"), _T("SourcesWanted"), &Downloads.SourcesWanted, 500, 1, 50, 5000 );
	Add( _T("Downloads"), _T("SparseThreshold"), &Downloads.SparseThreshold, 8*Kilo, Kilo, 0, 256, _T(" MB") );
	Add( _T("Downloads"), _T("StaggardStart"), &Downloads.StaggardStart, false );
	Add( _T("Downloads"), _T("StarveGiveUp"), &Downloads.StarveGiveUp, 3, 1, 3, 120, _T(" h") );
	Add( _T("Downloads"), _T("StarveTimeout"), &Downloads.StarveTimeout, 45*60*1000, 60*1000, 45, 24*60, _T(" m") );
	Add( _T("Downloads"), _T("TorrentPath"), &Downloads.TorrentPath );
	Add( _T("Downloads"), _T("VerifyED2K"), &Downloads.VerifyED2K, true );
	Add( _T("Downloads"), _T("VerifyFiles"), &Downloads.VerifyFiles, true );
	Add( _T("Downloads"), _T("VerifyTiger"), &Downloads.VerifyTiger, true );
	Add( _T("Downloads"), _T("VerifyTorrent"), &Downloads.VerifyTorrent, true );
	Add( _T("Downloads"), _T("WebHookEnable"), &Downloads.WebHookEnable, false );
	Add( _T("Downloads"), _T("WebHookExtensions"), &Downloads.WebHookExtensions, _T("|zip|7z|gz|rar|r0|tgz|ace|z|tar|arj|lzh|sit|hqx|fml|grs|mp3|iso|msi|exe|bin|") );

	Add( _T("Uploads"), _T("AllowBackwards"), &Uploads.AllowBackwards, true );
	Add( _T("Uploads"), _T("AutoClear"), &Uploads.AutoClear, false );
	Add( _T("Uploads"), _T("BlockAgents"), &Uploads.BlockAgents, _T("|Mozilla|Foxy|Fake Shareaza|") );
	Add( _T("Uploads"), _T("ClampdownFactor"), &Uploads.ClampdownFactor, 20, 1, 0, 100, _T("%") );
	Add( _T("Uploads"), _T("ClampdownFloor"), &Uploads.ClampdownFloor, 8*128, 128, 0, 4096, _T(" Kb/s") );
	Add( _T("Uploads"), _T("ClearDelay"), &Uploads.ClearDelay, 30*1000, 1000, 1, 1800, _T(" s") );
	Add( _T("Uploads"), _T("DynamicPreviews"), &Uploads.DynamicPreviews, true );
	Add( _T("Uploads"), _T("FilterMask"), &Uploads.FilterMask, 0xFFFFFFFD );
	Add( _T("Uploads"), _T("FreeBandwidthFactor"), &Uploads.FreeBandwidthFactor, 15, 1, 0, 100, _T("%") );
	Add( _T("Uploads"), _T("FreeBandwidthValue"), &Uploads.FreeBandwidthValue, 20*128, 128, 0, 4096, _T(" Kb/s") );
	Add( _T("Uploads"), _T("HubUnshare"), &Uploads.HubUnshare, true );
	Add( _T("Uploads"), _T("MaxPerHost"), &Uploads.MaxPerHost, 2, 1, 1, 64 );
	Add( _T("Uploads"), _T("PreviewQuality"), &Uploads.PreviewQuality, 75, 1, 5, 100, _T("%") );
	Add( _T("Uploads"), _T("PreviewTransfers"), &Uploads.PreviewTransfers, 3, 1, 1, 64 );
	Add( _T("Uploads"), _T("QueuePollMax"), &Uploads.QueuePollMax, 120*1000, 1000, 30, 180, _T(" s") );
	Add( _T("Uploads"), _T("QueuePollMin"), &Uploads.QueuePollMin, 45*1000, 1000, 0, 60, _T(" s") );
	Add( _T("Uploads"), _T("RewardQueuePercentage"), &Uploads.RewardQueuePercentage, 10, 1, 0, 99, _T("%") );
	Add( _T("Uploads"), _T("RotateChunkLimit"), &Uploads.RotateChunkLimit, Mega, Kilo, 0, 10*Kilo, _T(" KB") );
	Add( _T("Uploads"), _T("ShareHashset"), &Uploads.ShareHashset, true );
	Add( _T("Uploads"), _T("ShareMetadata"), &Uploads.ShareMetadata, true );
	Add( _T("Uploads"), _T("SharePartials"), &Uploads.SharePartials, true );
	Add( _T("Uploads"), _T("SharePreviews"), &Uploads.SharePreviews, true );
	Add( _T("Uploads"), _T("ShareTiger"), &Uploads.ShareTiger, true );
	Add( _T("Uploads"), _T("ThrottleMode"), &Uploads.ThrottleMode, true );

	Add( _T("Remote"), _T("Enable"), &Remote.Enable, false );
	Add( _T("Remote"), _T("Password"), &Remote.Password );
	Add( _T("Remote"), _T("Username"), &Remote.Username );

	Add( _T("Experimental"), _T("EnableDIPPSupport"), &Experimental.EnableDIPPSupport, false );
	Add( _T("Experimental"), _T("TestBTPartials"), &Experimental.TestBTPartials, false );

	Add( _T("WINE"), _T("MenuFix"), &WINE.MenuFix, true );

	Add( _T("IRC"), _T("Colors[0]"), &IRC.Colors[0], RGB(244,0,0) );
	Add( _T("IRC"), _T("Colors[1]"), &IRC.Colors[1], RGB(0,0,0) );
	Add( _T("IRC"), _T("Colors[2]"), &IRC.Colors[2], RGB(0,0,244) );
	Add( _T("IRC"), _T("Colors[3]"), &IRC.Colors[3], RGB(200,100,120) );
	Add( _T("IRC"), _T("Colors[4]"), &IRC.Colors[4], RGB(0,144,0) );
	Add( _T("IRC"), _T("Colors[5]"), &IRC.Colors[5], RGB(244,0,0) );
	Add( _T("IRC"), _T("Colors[6]"), &IRC.Colors[6], RGB(230,230,230) );
	Add( _T("IRC"), _T("Colors[7]"), &IRC.Colors[7], RGB(0xF8,0xF8,0xF8) );
	Add( _T("IRC"), _T("Colors[8]"), &IRC.Colors[8], RGB(0,0,0) );
	Add( _T("IRC"), _T("Colors[9]"), &IRC.Colors[9], RGB(200,30,30) );
	Add( _T("IRC"), _T("Colors[10]"), &IRC.Colors[10], RGB(200,130,230) );
	Add( _T("IRC"), _T("Show"), &IRC.Show, true );
	Add( _T("IRC"), _T("FloodEnable"), &IRC.FloodEnable, true );
	Add( _T("IRC"), _T("Nick"), &IRC.Nick );
	Add( _T("IRC"), _T("Alternate"), &IRC.Alternate );
	Add( _T("IRC"), _T("ServerName"), &IRC.ServerName, _T("shareaza.p2pchat.net") );
	Add( _T("IRC"), _T("ServerPort"), &IRC.ServerPort, _T("6667") );
	Add( _T("IRC"), _T("FloodLimit"), &IRC.FloodLimit, _T("24") );
	Add( _T("IRC"), _T("Timestamp"), &IRC.Timestamp, true );
	Add( _T("IRC"), _T("UserName"), &IRC.UserName, _T("razaIRC") );
	Add( _T("IRC"), _T("RealName"), &IRC.RealName, _T("razaIRC") );
	Add( _T("IRC"), _T("ScreenFont"), &IRC.ScreenFont, NULL, false, setFont );

	// Load settings
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		Item* pItem = m_pItems.GetNext( pos );
		pItem->Load();
		CString strPath;
		if ( *pItem->m_szSection )
			strPath.AppendFormat( L"%s.%s", pItem->m_szSection, pItem->m_szName );
		else
			strPath.AppendFormat( L"General.%s", pItem->m_szName );
		m_pSettingsTable.insert( CSettingsMap::value_type( strPath, pItem ) );
	}

	if ( Library.ScanMSI )
	{
		// Check if Windows installer library is present
		HINSTANCE hMSI = LoadLibrary( _T("Msi.dll") );
		if ( ! hMSI )
			Library.ScanMSI = false;
		else
			FreeLibrary( hMSI );
	}

	// Set default program and user paths
	if ( General.Path.IsEmpty() || ! PathFileExists( General.Path ) )
	{
		General.Path = theApp.GetProgramFilesFolder() + _T("\\") CLIENT_NAME_T;
		if ( ! PathFileExists( General.Path ) )
		{
			General.Path = theApp.GetProgramFilesFolder64() + _T( "\\" ) CLIENT_NAME_T;
			if ( ! PathFileExists( General.Path ) )
			{
				General.Path = theApp.m_strBinaryPath.Left( theApp.m_strBinaryPath.ReverseFind( '\\' ) );
			}
		}
	}

	if ( General.UserPath.IsEmpty() || ! CreateDirectory( General.UserPath + _T("\\Data") ) )
	{
		General.UserPath = theApp.GetAppDataFolder() + _T("\\") CLIENT_NAME_T;
		CreateDirectory( General.UserPath + _T("\\Data") );
	}

	if ( Downloads.IncompletePath.IsEmpty() || ! CreateDirectory( Downloads.IncompletePath ) )
	{
		Downloads.IncompletePath = theApp.GetLocalAppDataFolder() + _T("\\") CLIENT_NAME_T _T("\\Incomplete");
		CreateDirectory( Downloads.IncompletePath );
	}

	if ( Downloads.CompletePath.IsEmpty() || ! CreateDirectory( Downloads.CompletePath ) )
	{
		Downloads.CompletePath = theApp.GetDownloadsFolder();
		CreateDirectory( Downloads.CompletePath );
	}

	if ( Downloads.TorrentPath.IsEmpty() || ! CreateDirectory( Downloads.TorrentPath ) )
	{
		Downloads.TorrentPath = General.UserPath + _T("\\Torrents");
		CreateDirectory( Downloads.TorrentPath );
	}

	if ( Downloads.CollectionPath.IsEmpty() || ! CreateDirectory( Downloads.CollectionPath ) )
	{
		Downloads.CollectionPath = General.UserPath + _T("\\Collections");
		CreateDirectory( Downloads.CollectionPath );
	}

	CString sTorrent;
	GetFullPathName( BitTorrent.TorrentCreatorPath, MAX_PATH, sTorrent.GetBuffer( MAX_PATH ), NULL );
	sTorrent.ReleaseBuffer();
	BitTorrent.TorrentCreatorPath = sTorrent;
	if ( BitTorrent.TorrentCreatorPath.IsEmpty() || ! PathFileExists( BitTorrent.TorrentCreatorPath ) )
	{
		BitTorrent.TorrentCreatorPath = General.Path + _T("\\TorrentWizard.exe");
	}
	if ( ! ( StartsWith( BitTorrent.DefaultTracker, _PT("http://") ) ||
			 StartsWith( BitTorrent.DefaultTracker, _PT("udp://") ) ) )
	{
		SetDefault( &BitTorrent.DefaultTracker );
	}

	Live.FirstRun = General.FirstRun;
	General.FirstRun = false;

	SmartUpgrade();

	//if ( General.Running )
	// TODO: Shareaza is restarted after a crash

	// Set current networks
	Gnutella1.EnableToday		= Gnutella1.EnableAlways;
	Gnutella2.EnableToday		= Gnutella2.EnableAlways;
	eDonkey.EnableToday			= eDonkey.EnableAlways;
	BitTorrent.EnableToday		= BitTorrent.EnableAlways;
	DC.EnableToday				= DC.EnableAlways;

	if ( General.Upgrade )
	{
		General.Upgrade = false;

		// Copy installer to complete folder
		CString strInstaller = CRegistry::GetString( _T(""), _T("Installer") );
		if ( PathFileExists( strInstaller ) )
		{
			// Delete old Shareaza installers
			CFileFind ff;
			BOOL res = ff.FindFile( Downloads.CompletePath + _T("\\") CLIENT_NAME_T _T("_*.exe") );
			while ( res )
			{
				res = ff.FindNextFile();
				CString strPath = ff.GetFilePath();
				if ( strPath.CompareNoCase( strInstaller ) != 0 )	// Keep ourself
					DeleteFile( strPath );
			}

			CString strNewPath = Downloads.CompletePath + _T("\\") + PathFindFileName( strInstaller );
			if ( strNewPath.CompareNoCase( strInstaller ) != 0 )	// Keep ourself
				CopyFile( strInstaller, strNewPath, FALSE );
		}
	}

	// Set interface
	Interface.LowResMode		= ! ( GetSystemMetrics( SM_CYSCREEN ) > 600 );
	if ( Live.FirstRun )
	{
		Search.AdvancedPanel = ! Interface.LowResMode;

		if ( IsDefault( &General.Language ) )
		{
			CString strDefaultLanguage = CRegistry::GetString( NULL, _T("DefaultLanguage") );
			if ( ! strDefaultLanguage.IsEmpty() )
			{
				General.Language = strDefaultLanguage;
				General.LanguageRTL = CRegistry::GetDword( NULL, _T("DefaultLanguageRTL") ) != 0;
			}
		}
	}

	// Reset certain network variables if bandwidth is too low
	// Set ed2k and G1
	if ( GetOutgoingBandwidth() < 2 )
	{
		Gnutella1.EnableToday = Gnutella1.EnableAlways = false;
		eDonkey.EnableToday = eDonkey.EnableAlways = false;
		BitTorrent.EnableToday = BitTorrent.EnableAlways = false;
		DC.EnableToday = DC.EnableAlways = false;
	}
	// Set number of torrents
	BitTorrent.DownloadTorrents = min( BitTorrent.DownloadTorrents, ( GetOutgoingBandwidth() / 2u + 2u ) );

	// Enforce a few sensible values to avoid being banned/dropped/etc (in case of registry fiddling)
	Downloads.ConnectThrottle	= max( Downloads.ConnectThrottle, Connection.ConnectThrottle + 50u );

	// Make sure download/incomplete folders aren't the same
	if ( _tcsicmp( Downloads.IncompletePath, Downloads.CompletePath ) == 0 )
	{
		CString strMessage;
		LoadString( strMessage, IDS_SETTINGS_FILEPATH_NOT_SAME );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		// Downloads.IncompletePath = General.Path + _T("\\Incomplete");
	}

	// Temporary until G1 ultrapeer has been updated
	Gnutella1.ClientMode = MODE_LEAF;

	// UPnP will setup a random port, so we need to reset values after it sets Connection.InPort
	if ( Connection.RandomPort )
		Connection.InPort = 0;
	else if ( Connection.InPort == 0 )
		Connection.RandomPort = true;

#ifdef LAN_MODE
	Connection.IgnoreLocalIP = false;
	Gnutella1.EnableToday = Gnutella1.EnableAlways = false;
	Gnutella2.EnableToday = Gnutella2.EnableAlways = true;
	eDonkey.EnableToday = eDonkey.EnableAlways = false;
	BitTorrent.EnableToday = BitTorrent.EnableAlways = false;
	DC.EnableToday = DC.EnableAlways = false;
	Gnutella.MaxHits = 0;
#endif // LAN_MODE

	if ( Live.FirstRun )
		OnChangeConnectionSpeed();	// This helps if the QuickStart Wizard is skipped.
}

void CSettings::Save(BOOL bShutdown)
{
	General.Running = ! bShutdown;

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		Item* pItem = m_pItems.GetNext( pos );
		pItem->Save();
	}
}

void CSettings::Normalize(LPVOID pSetting)
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		Item* pItem = m_pItems.GetNext( pos );
		if ( *pItem == pSetting )
		{
			pItem->Normalize();
			break;
		}
	}
}

bool CSettings::IsDefault(LPVOID pSetting) const
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		Item* pItem = m_pItems.GetNext( pos );
		if ( *pItem == pSetting )
		{
			return pItem->IsDefault();
		}
	}
	return false;
}

void CSettings::SetDefault(LPVOID pSetting)
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		Item* pItem = m_pItems.GetNext( pos );
		if ( *pItem == pSetting )
		{
			pItem->SetDefault();
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CSettings smart upgrade

void CSettings::SmartUpgrade()
{
	// This function resets certain values when upgrading, depending on version.
	if ( General.SmartVersion > SMART_VERSION )
		General.SmartVersion = 1;

	if ( General.SmartVersion < SMART_VERSION )
	{
		Uploads.SharePartials = true;

		// 'SmartUpgrade' settings updates- change any settings that were mis-set in previous versions
		if ( General.SmartVersion < 20 )
		{
			Gnutella2.UdpOutResend			= 6000;
			Gnutella2.UdpOutExpire			= 26000;
			Library.TigerHeight				= 9;

			Downloads.AutoExpand			= false;

			Uploads.MaxPerHost				= 2;
			Uploads.ShareTiger				= true;

			Library.PrivateTypes.erase( _T("nfo") );

			// Remove dots
			string_set tmp;
			for ( string_set::const_iterator i = Library.SafeExecute.begin() ;
				i != Library.SafeExecute.end(); ++i )
			{
				tmp.insert( ( (*i).GetAt( 0 ) == _T('.') ) ? (*i).Mid( 1 ) : (*i) );
			}
			Library.SafeExecute = tmp;
		}

		if ( General.SmartVersion < 21 )
		{
			Library.ThumbSize				= 128;
			Library.SourceExpire			= 86400;

			Gnutella1.TranslateTTL			= 2;
		}

		if ( General.SmartVersion < 24 )
		{
			General.CloseMode				= 0;

			Connection.TimeoutConnect		= 16000;
			Connection.TimeoutHandshake		= 45000;

			Downloads.RetryDelay			= 10*60000;

			Uploads.FilterMask				= 0xFFFFFFFD;
		}

		if ( General.SmartVersion < 25 )
		{
			Connection.TimeoutTraffic		= 140000;

			Gnutella2.NumPeers				= 6;
		}

		if ( General.SmartVersion < 28 )
		{
			BitTorrent.Endgame		= true;		// Endgame on
		}

		if ( General.SmartVersion < 29 )
		{
			Downloads.MinSources	= 1;		// Lower Max value- should reset it in case

			Gnutella2.RequeryDelay	= 4*3600;	// Longer delay between sending same search to G2 hub
		}

		if ( General.SmartVersion < 30 )
		{
			BitTorrent.RequestSize	= 16384;	// Other BT clients have changed this value (undocumented)
		}

		if ( General.SmartVersion < 31 )
		{
			Downloads.SearchPeriod			= 120000;

			Gnutella1.MaximumTTL			= 10;

			Gnutella2.QueryGlobalThrottle	= 125;

			Uploads.QueuePollMin	= 45000;	// Lower values for re-ask times- a dynamic multiplier
			Uploads.QueuePollMax	= 120000;	//  Is now applied based on Q# (from 1x to 5x)
			eDonkey.PacketThrottle	= 500;		// Second throttle added for finer control
		}

		if ( General.SmartVersion < 32 )
		{
			theApp.WriteProfileString( _T("Interface"), _T("SchemaColumns.audio"), _T("(EMPTY)") );
		}

		if ( General.SmartVersion < 33 )
		{
			RegDeleteKey( HKEY_CURRENT_USER, REGISTRY_KEY _T("\\Plugins\\LibraryBuilder") );
		}

		if ( General.SmartVersion < 34 )
			BitTorrent.LinkPing				= 120 * 1000;

		if ( General.SmartVersion < 35 )
		{
			Gnutella1.QuerySearchUTF8 = true;
			Gnutella1.QueryHitUTF8 = true;
		}

		if ( General.SmartVersion < 36 )
		{
			//Library.VirtualFiles	= true;		// Virtual files (stripping) on
			Library.VirtualFiles = false;
		}

		if ( General.SmartVersion < 37 )
		{
			Downloads.RequestHash = true;
			Gnutella.SpecifyProtocol = true;
			Search.FilterMask = Search.FilterMask | 0x140; // Turn on DRM and Suspicious filters
		}

		if ( General.SmartVersion < 39 )
		{
			General.RatesInBytes = true;
		}

		if ( General.SmartVersion < 40 )
		{
			eDonkey.ForceHighID = true;
			eDonkey.FastConnect = false;
		}

		if ( General.SmartVersion < 41 )
		{
			eDonkey.ExtendedRequest = 2;
			Community.ChatAllNetworks = true;
			Community.ChatFilter = true;
		}

		if ( General.SmartVersion < 42 )
		{
			Gnutella2.NumHubs = 2;
			General.ItWasLimited = true;
		}

		if ( General.SmartVersion < 44 )
		{
			BitTorrent.AutoSeed = true;
		}

		if ( General.SmartVersion < 45 )
		{
			Library.PrivateTypes.erase( _T("dat") );

			// FlashGet
			if ( ! IsIn( Library.PrivateTypes, _T("jc!") ) )
				Library.PrivateTypes.insert( _T("jc!") );
			// FlashGet torrent
			if ( ! IsIn( Library.PrivateTypes, _T("fb!") ) )
				Library.PrivateTypes.insert( _T("fb!") );
			// BitComet
			if ( ! IsIn( Library.PrivateTypes, _T("bc!") ) )
				Library.PrivateTypes.insert( _T("bc!") );
		}

		if ( General.SmartVersion < 46 )
		{
			// ReGet
			if ( ! IsIn( Library.PrivateTypes, _T("reget") ) )
				Library.PrivateTypes.insert( _T("reget") );
		}

		if ( General.SmartVersion < 47 )
		{
			// Changed from minutes to seconds
			Gnutella1.QueryThrottle = 30u;

			Gnutella.MaxResults = 150;
		}

		if ( General.SmartVersion < 49 )
		{
			eDonkey.SendPortServer = false;
		}

		if ( General.SmartVersion < 50 )
		{
			CString strExts =
				theApp.GetProfileString( L"Plugins", L"{04CC76C7-1ED7-4CAE-9762-B8664ED008ED}" );
			if ( strExts.GetLength() > 0 && strExts.GetAt( 0 ) == '|' )
			{
				if ( _tcsistr( strExts, L"|.3gp|" ) == NULL && _tcsistr( strExts, L"|-.3gp|" ) == NULL )
					strExts += L"|.3gp|";
				if ( _tcsistr( strExts, L"|.3gpp|" ) == NULL && _tcsistr( strExts, L"|-.3gpp|" ) == NULL )
					strExts += L"|.3gpp|";
				if ( _tcsistr( strExts, L"|.3g2|" ) == NULL && _tcsistr( strExts, L"|-.3g2|" ) == NULL )
					strExts += L"|.3g2|";
				if ( _tcsistr( strExts, L"|.dv|" ) == NULL && _tcsistr( strExts, L"|-.dv|" ) == NULL )
					strExts += L"|.dv|";
				if ( _tcsistr( strExts, L"|.flv|" ) == NULL && _tcsistr( strExts, L"|-.flv|" ) == NULL )
					strExts += L"|.flv|";
				if ( _tcsistr( strExts, L"|.ivf|" ) == NULL && _tcsistr( strExts, L"|-.ivf|" ) == NULL )
					strExts += L"|.ivf|";
				if ( _tcsistr( strExts, L"|.gvi|" ) == NULL && _tcsistr( strExts, L"|-.gvi|" ) == NULL )
					strExts += L"|.gvi|";
				if ( _tcsistr( strExts, L"|.mpe|" ) == NULL && _tcsistr( strExts, L"|-.mpe|" ) == NULL )
					strExts += L"|.mpe|";
				if ( _tcsistr( strExts, L"|.wm|" ) == NULL && _tcsistr( strExts, L"|-.wm|" ) == NULL )
					strExts += L"|.wm|";
				if ( _tcsistr( strExts, L"|.rmvb|" ) == NULL && _tcsistr( strExts, L"|-.rmvb|" ) == NULL )
					strExts += L"|.rmvb|";
				if ( _tcsistr( strExts, L"|.mp4|" ) == NULL && _tcsistr( strExts, L"|-.mp4|" ) == NULL )
					strExts += L"|.mp4|";
				theApp.WriteProfileString( L"Plugins", L"{04CC76C7-1ED7-4CAE-9762-B8664ED008ED}", strExts );
			}
			else
			{
				// the value is missing or it was REG_DWORD as in earlier versions
				strExts = L"|.asf||.asx||.avi||.divx||.m2v||.m2p||.mkv||.mov||.mpeg||.mpg||.ogm||.qt||.ram||.rm||.vob||.wmv||.xvid||.mp4||.rmvb||.3gp||.3gpp||.3g2||.dv||.flv||.ivf||.gvi||.mpe||.nsv||.wm|";
				theApp.WriteProfileString( L"Plugins", L"{04CC76C7-1ED7-4CAE-9762-B8664ED008ED}", strExts );
			}
			strExts =
				theApp.GetProfileString( L"Plugins", L"{570C197C-FE9C-4D1F-B6E0-EFA44D36399F}" );
			if ( strExts.GetLength() > 0 && strExts.GetAt( 0 ) == '|' )
			{
				if ( _tcsistr( strExts, L"|.3gp|" ) == NULL && _tcsistr( strExts, L"|-.3gp|" ) == NULL )
					strExts += L"|.3gp|";
				if ( _tcsistr( strExts, L"|.3gpp|" ) == NULL && _tcsistr( strExts, L"|-.3gpp|" ) == NULL )
					strExts += L"|.3gpp|";
				if ( _tcsistr( strExts, L"|.3g2|" ) == NULL && _tcsistr( strExts, L"|-.3g2|" ) == NULL )
					strExts += L"|.3g2|";
				if ( _tcsistr( strExts, L"|.dv|" ) == NULL && _tcsistr( strExts, L"|-.dv|" ) == NULL )
					strExts += L"|.dv|";
				if ( _tcsistr( strExts, L"|.flv|" ) == NULL && _tcsistr( strExts, L"|-.flv|" ) == NULL )
					strExts += L"|.flv|";
				if ( _tcsistr( strExts, L"|.ivf|" ) == NULL && _tcsistr( strExts, L"|-.ivf|" ) == NULL )
					strExts += L"|.ivf|";
				if ( _tcsistr( strExts, L"|.gvi|" ) == NULL && _tcsistr( strExts, L"|-.gvi|" ) == NULL )
					strExts += L"|.gvi|";
				if ( _tcsistr( strExts, L"|.mpe|" ) == NULL && _tcsistr( strExts, L"|-.mpe|" ) == NULL )
					strExts += L"|.mpe|";
				if ( _tcsistr( strExts, L"|.wm|" ) == NULL && _tcsistr( strExts, L"|-.wm|" ) == NULL )
					strExts += L"|.wm|";
				if ( _tcsistr( strExts, L"|.rmvb|" ) == NULL && _tcsistr( strExts, L"|-.rmvb|" ) == NULL )
					strExts += L"|.rmvb|";
				if ( _tcsistr( strExts, L"|.mp4|" ) == NULL && _tcsistr( strExts, L"|-.mp4|" ) == NULL )
					strExts += L"|.mp4|";
				theApp.WriteProfileString( L"Plugins", L"{570C197C-FE9C-4D1F-B6E0-EFA44D36399F}", strExts );
			}
			else
			{
				strExts = L"|.asf||.asx||.avi||.divx||.m2v||.m2p||.mkv||.mov||.mpeg||.mpg||.ogm||.qt||.ram||.rm||.vob||.wmv||.xvid||.mp4||.rmvb||.3gp||.3gpp||.3g2||.dv||.flv||.ivf||.gvi||.mpe||.nsv||.wm|";
				theApp.WriteProfileString( L"Plugins", L"{570C197C-FE9C-4D1F-B6E0-EFA44D36399F}", strExts );
			}
		}

		if ( General.SmartVersion < 51 )
		{
			Library.HashWindow = true;
			Gnutella1.PingRate = 30000u;
		}

		if ( General.SmartVersion < 52 )
		{
			WINE.MenuFix = true;
			OnChangeConnectionSpeed();
		}

		if ( General.SmartVersion < 53 )
		{
			Gnutella1.NumLeafs = 50;
			if ( ! IsIn( Library.SafeExecute, _T("co") ) )
				Library.SafeExecute.insert( _T("co") );
			if ( ! IsIn( Library.SafeExecute, _T("collection") ) )
				Library.SafeExecute.insert( _T("collection") );
			if ( ! IsIn( Library.SafeExecute, _T("lit") ) )
				Library.SafeExecute.insert( _T("lit") );
		}

		if ( General.SmartVersion < 54 )
		{
			// uTorrent
			if ( ! IsIn( Library.PrivateTypes, _T("!ut") ) )
				Library.PrivateTypes.insert( _T("!ut") );
		}

		if ( General.SmartVersion < 57 )
		{
			// Delete old values
			SHDeleteValue( HKEY_CURRENT_USER,
				REGISTRY_KEY _T("\\Toolbars"), _T("CRemoteWnd") );
		}

		if ( General.SmartVersion < 58 )
		{
			eDonkey.LargeFileSupport = true;
		}

		if ( General.SmartVersion < 59 )
		{
			Fonts.DefaultFont.Empty();
			Fonts.SystemLogFont.Empty();
			Fonts.FontSize = 11;
		}

		if ( General.SmartVersion < 60 )
		{
			SetDefault( &eDonkey.ServerListURL );
		}

		if ( General.SmartVersion < 61 )
		{
			// DownThemAll for Firefox
			if ( ! IsIn( Library.PrivateTypes, _T( "dtapart" ) ) )
				Library.PrivateTypes.insert( _T( "dtapart" ) );
			// KeePass database
			if ( ! IsIn( Library.PrivateTypes, _T( "kdbx" ) ) )
				Library.PrivateTypes.insert( _T( "kdbx" ) );
			// Windows PowerShell script
			if ( ! IsIn( Library.PrivateTypes, _T( "ps1" ) ) )
				Library.PrivateTypes.insert( _T( "ps1" ) );
			if ( ! IsIn( Library.PrivateTypes, _T( "ps1xml" ) ) )
				Library.PrivateTypes.insert( _T( "ps1xml" ) );
			if ( ! IsIn( Library.PrivateTypes, _T( "ps2" ) ) )
				Library.PrivateTypes.insert( _T( "ps2" ) );
			if ( ! IsIn( Library.PrivateTypes, _T( "ps2xml" ) ) )
				Library.PrivateTypes.insert( _T( "ps2xml" ) );
			if ( ! IsIn( Library.PrivateTypes, _T( "psc1" ) ) )
				Library.PrivateTypes.insert( _T( "psc1" ) );
			if ( ! IsIn( Library.PrivateTypes, _T( "psc2" ) ) )
				Library.PrivateTypes.insert( _T( "psc2" ) );
			// Windows Script
			if ( ! IsIn( Library.PrivateTypes, _T( "ws" ) ) )
				Library.PrivateTypes.insert( _T( "ws" ) );
			if ( ! IsIn( Library.PrivateTypes, _T( "wsf" ) ) )
				Library.PrivateTypes.insert( _T( "wsf" ) );
			if ( ! IsIn( Library.PrivateTypes, _T( "wsc" ) ) )
				Library.PrivateTypes.insert( _T( "wsc" ) );
			if ( ! IsIn( Library.PrivateTypes, _T( "wsh" ) ) )
				Library.PrivateTypes.insert( _T( "wsh" ) );
			// Windows Explorer command file
			if ( ! IsIn( Library.PrivateTypes, _T( "scf" ) ) )
				Library.PrivateTypes.insert( _T( "scf" ) );
			// VBScript
			if ( ! IsIn( Library.PrivateTypes, _T( "vb" ) ) )
				Library.PrivateTypes.insert( _T( "vb" ) );
			if ( ! IsIn( Library.PrivateTypes, _T( "vbs" ) ) )
				Library.PrivateTypes.insert( _T( "vbs" ) );
			if ( ! IsIn( Library.PrivateTypes, _T( "vbe" ) ) )
				Library.PrivateTypes.insert( _T( "vbe" ) );
			// JavaScript
			if ( ! IsIn( Library.PrivateTypes, _T( "js" ) ) )
				Library.PrivateTypes.insert( _T( "js" ) );
			if ( ! IsIn( Library.PrivateTypes, _T( "jse" ) ) )
				Library.PrivateTypes.insert( _T( "jse" ) );
			// HTML application
			if ( ! IsIn( Library.PrivateTypes, _T( "hta" ) ) )
				Library.PrivateTypes.insert( _T( "hta" ) );
			// Windows screen saver
			if ( ! IsIn( Library.PrivateTypes, _T( "scr" ) ) )
				Library.PrivateTypes.insert( _T( "scr" ) );
			// ClickOnce application
			if ( ! IsIn( Library.PrivateTypes, _T( "application" ) ) )
				Library.PrivateTypes.insert( _T( "application" ) );
		}
	}

	General.SmartVersion = SMART_VERSION;
}

void CSettings::OnChangeConnectionSpeed()
{
	bool bLimited = theApp.m_bLimitedConnections && !General.IgnoreXPsp2;

	if ( Connection.InSpeed <= 80 )
	{	// NT Modem users / Win9x Modem users
		Downloads.MaxFiles				= 8;
		Downloads.MaxTransfers			= 24;
		Downloads.MaxFileTransfers		= 4;
		Downloads.MaxConnectingSources	= 16;
		Downloads.MaxFileSearches		= 0;
		Downloads.SourcesWanted			= 200;	// Don't bother requesting so many sources
		Search.GeneralThrottle			= 300;	// Slow searches a little so we don't get flooded

		Gnutella2.NumLeafs				= 50;
		BitTorrent.DownloadTorrents		= 1;	// Best not to try too many torrents
	}
	else if ( Connection.InSpeed <= 256 )
	{	// IDSN, Dual modems, etc
		Downloads.MaxFiles				= 14;
		Downloads.MaxTransfers			= 32;
		Downloads.MaxFileTransfers		= 6;
		Downloads.MaxConnectingSources	= 20;
		Downloads.MaxFileSearches		= 1;
		Downloads.SourcesWanted			= 500;
		Search.GeneralThrottle			= 250;	// Slow searches a little so we don't get flooded

		Gnutella2.NumLeafs				= 200;
		BitTorrent.DownloadTorrents		= 3;
	}
	else if ( Connection.InSpeed <= 768 )
	{	// Slower broadband
		Downloads.MaxFiles				= 20;
		Downloads.MaxTransfers			= 64;
		Downloads.MaxFileTransfers		= 8;
		Downloads.MaxConnectingSources	= 24;
		Downloads.MaxFileSearches		= 1;
		Downloads.SourcesWanted			= 500;
		Search.GeneralThrottle			= 250;

		Gnutella2.NumLeafs				= 300;
		BitTorrent.DownloadTorrents		= 3;
	}
	else if ( Connection.InSpeed <= 2500 || bLimited )
	{	// Fast broadband
		Downloads.MaxFiles				= 26;
		Downloads.MaxTransfers			= 100;
		Downloads.MaxFileTransfers		= 10;
		Downloads.MaxConnectingSources	= 28;
		Downloads.MaxFileSearches		= 2;
		Downloads.SourcesWanted			= 500;
		Search.GeneralThrottle			= 200;

		Gnutella2.NumLeafs				= 300;
		BitTorrent.DownloadTorrents		= 4;
	}
	else if ( Connection.InSpeed <= 5000 )
	{	// Very high capacity connection
		Downloads.MaxFiles				= 32;
		Downloads.MaxTransfers			= 200;
		Downloads.MaxFileTransfers		= 32;
		Downloads.MaxConnectingSources	= 32;
		Downloads.MaxFileSearches		= 3;
		Downloads.SourcesWanted			= 500;
		Search.GeneralThrottle			= 200;

		Gnutella2.NumLeafs				= 400;	// Can probably support more leaves
		BitTorrent.DownloadTorrents		= 5;	// Should be able to handle several torrents
	}
	else
	{
		Downloads.MaxFiles				= 50;
		Downloads.MaxTransfers			= 250;
		Downloads.MaxFileTransfers		= 40;
		Downloads.MaxConnectingSources	= 40;
		Downloads.MaxFileSearches		= 5;
		Downloads.SourcesWanted			= 500;
		Search.GeneralThrottle			= 200;

		if ( Connection.InSpeed <= 10000 )
		{
			Gnutella2.NumLeafs			= 450;	// Can probably support more leaves
			BitTorrent.DownloadTorrents	= 7;	// Should be able to handle several torrents
		}
		else
		{
			Gnutella2.NumLeafs			= 500;	// Can probably support more leaves
			BitTorrent.DownloadTorrents	= 10;	// Should be able to handle several torrents
		}
	}

	if( bLimited )
	{	// Window XP Service Pack 2
		Connection.ConnectThrottle		= max( Connection.ConnectThrottle, 250ul );
		Downloads.ConnectThrottle		= max( Downloads.ConnectThrottle, 800ul );
		Gnutella.ConnectFactor			= min( Gnutella.ConnectFactor, 3ul );
		Connection.SlowConnect			= true;
		Connection.RequireForTransfers	= true;
		Downloads.MaxConnectingSources	= 8;
		Gnutella1.EnableAlways			= false;
		Gnutella1.EnableToday			= false;

		General.ItWasLimited			= true;
	}
	else if( General.ItWasLimited )	// We change the settings back if the user path the half-open connection limit
	{
		SetDefault( &Connection.ConnectThrottle );
		SetDefault( &Downloads.ConnectThrottle );
		SetDefault( &Gnutella.ConnectFactor );
		SetDefault( &Connection.SlowConnect );

		General.ItWasLimited			= false;
	}
}

//////////////////////////////////////////////////////////////////////
// CSettings window position persistance

BOOL CSettings::LoadWindow(LPCTSTR pszName, CWnd* pWindow)
{
	CString strEntry;

	if ( pszName != NULL )
		strEntry = pszName;
	else
		strEntry = pWindow->GetRuntimeClass()->m_lpszClassName;

	int nShowCmd = CRegistry::GetInt( _T("Windows"), strEntry + _T(".ShowCmd"), -1 );
	if ( nShowCmd == -1 ) return FALSE;

	WINDOWPLACEMENT pPos = {};
	pPos.length = sizeof(pPos);

	pPos.rcNormalPosition.left		= CRegistry::GetInt( _T("Windows"), strEntry + _T(".Left"), 0 );
	pPos.rcNormalPosition.top		= CRegistry::GetInt( _T("Windows"), strEntry + _T(".Top"), 0 );
	pPos.rcNormalPosition.right		= CRegistry::GetInt( _T("Windows"), strEntry + _T(".Right"), 0 );
	pPos.rcNormalPosition.bottom	= CRegistry::GetInt( _T("Windows"), strEntry + _T(".Bottom"), 0 );

	if ( pPos.rcNormalPosition.right && pPos.rcNormalPosition.bottom )
	{
		pPos.showCmd = 0;
		pWindow->SetWindowPlacement( &pPos );
	}

	if ( Live.LoadWindowState && nShowCmd == SW_SHOWMINIMIZED )
	{
		pWindow->PostMessage( WM_SYSCOMMAND, SC_MINIMIZE );
	}
	else if ( ! Live.LoadWindowState && nShowCmd == SW_SHOWMAXIMIZED )
	{
		pWindow->PostMessage( WM_SYSCOMMAND, SC_MAXIMIZE );
	}

	return TRUE;
}

void CSettings::SaveWindow(LPCTSTR pszName, CWnd* pWindow)
{
	WINDOWPLACEMENT pPos;
	CString strEntry;

	if ( pszName != NULL )
		strEntry = pszName;
	else
		strEntry = pWindow->GetRuntimeClass()->m_lpszClassName;

	pWindow->GetWindowPlacement( &pPos );

	CRegistry::SetInt(  _T("Windows"), strEntry + _T(".ShowCmd"), pPos.showCmd );

	if ( pPos.showCmd != SW_SHOWNORMAL ) return;

	CRegistry::SetInt(  _T("Windows"), strEntry + _T(".Left"), pPos.rcNormalPosition.left );
	CRegistry::SetInt(  _T("Windows"), strEntry + _T(".Top"), pPos.rcNormalPosition.top );
	CRegistry::SetInt(  _T("Windows"), strEntry + _T(".Right"), pPos.rcNormalPosition.right );
	CRegistry::SetInt(  _T("Windows"), strEntry + _T(".Bottom"), pPos.rcNormalPosition.bottom );
}

//////////////////////////////////////////////////////////////////////
// CSettings list header persistance

BOOL CSettings::LoadList(LPCTSTR pszName, CListCtrl* pCtrl, int nSort)
{
	LV_COLUMN pColumn;

	pColumn.mask = LVCF_FMT;
	int nColumns = 0;
	for ( ; pCtrl->GetColumn( nColumns, &pColumn ) ; nColumns++ );

	CString strOrdering, strWidths, strItem;
	BOOL bSuccess = FALSE;

	strItem.Format( _T("%s.Ordering"), pszName );
	strOrdering = CRegistry::GetString( _T("ListStates"), strItem );
	strItem.Format( _T("%s.Widths"), pszName );
	strWidths = CRegistry::GetString( _T("ListStates"), strItem );
	strItem.Format( _T("%s.Sort"), pszName );
	nSort = CRegistry::GetInt( _T("ListStates"), strItem, nSort );

	if ( strOrdering.GetLength() == nColumns * 2 &&
		 strWidths.GetLength() == nColumns * 4 )
	{
		UINT* pOrdering = new UINT[ nColumns ];

		for ( int nColumn = 0 ; nColumn < nColumns ; nColumn++ )
		{
			_stscanf( strWidths.Mid( nColumn * 4, 4 ), _T("%x"), &pOrdering[ nColumn ] );
			pCtrl->SetColumnWidth( nColumn, pOrdering[ nColumn ] );
			_stscanf( strOrdering.Mid( nColumn * 2, 2 ), _T("%x"), &pOrdering[ nColumn ] );
		}

		pCtrl->SendMessage( LVM_SETCOLUMNORDERARRAY, nColumns, (LPARAM)pOrdering );
		delete [] pOrdering;
		bSuccess = TRUE;
	}

	SetWindowLongPtr( pCtrl->GetSafeHwnd(), GWLP_USERDATA, nSort );

	CHeaderCtrl* pHeader = (CHeaderCtrl*)pCtrl->GetWindow( GW_CHILD );
	if ( pHeader ) Skin.Translate( pszName, pHeader );

	return bSuccess;
}

void CSettings::SaveList(LPCTSTR pszName, CListCtrl* pCtrl)
{
	LV_COLUMN pColumn;

	pColumn.mask = LVCF_FMT;
	int nColumns = 0;
	for ( ; pCtrl->GetColumn( nColumns, &pColumn ) ; nColumns++ );

	UINT* pOrdering = new UINT[ nColumns ];
	ZeroMemory( pOrdering, nColumns * sizeof(UINT) );
	pCtrl->SendMessage( LVM_GETCOLUMNORDERARRAY, nColumns, (LPARAM)pOrdering );

	CString strOrdering, strWidths, strItem;

	for ( int nColumn = 0 ; nColumn < nColumns ; nColumn++ )
	{
		strItem.Format( _T("%.2x"), pOrdering[ nColumn ] );
		strOrdering += strItem;
		strItem.Format( _T("%.4x"), pCtrl->GetColumnWidth( nColumn ) );
		strWidths += strItem;
	}

	delete [] pOrdering;

	int nSort = (int)GetWindowLongPtr( pCtrl->GetSafeHwnd(), GWLP_USERDATA );

	strItem.Format( _T("%s.Ordering"), pszName );
	CRegistry::SetString( _T("ListStates"), strItem, strOrdering);
	strItem.Format( _T("%s.Widths"), pszName );
	CRegistry::SetString( _T("ListStates"), strItem, strWidths);
	strItem.Format( _T("%s.Sort"), pszName );
	CRegistry::SetInt( _T("ListStates"), strItem, nSort );
}

//////////////////////////////////////////////////////////////////////
// CSettings startup

BOOL CSettings::CheckStartup()
{
	BOOL bStartup;
	HKEY hKey;

	if ( RegOpenKeyEx( HKEY_CURRENT_USER,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_QUERY_VALUE, &hKey )
		!= ERROR_SUCCESS ) return FALSE;

	bStartup = ( RegQueryValueEx( hKey, CLIENT_NAME_T, NULL, NULL, NULL, NULL ) == ERROR_SUCCESS );

	RegCloseKey( hKey );

	return bStartup;
}

void CSettings::SetStartup(BOOL bStartup)
{
	HKEY hKey;

	if ( RegOpenKeyEx( HKEY_CURRENT_USER,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_ALL_ACCESS, &hKey )
		!= ERROR_SUCCESS ) return;

	if ( bStartup )
	{
		CString strCommand;
		strCommand.Format( _T("\"%s\" -tray"), (LPCTSTR)theApp.m_strBinaryPath );
		RegSetValueEx( hKey, CLIENT_NAME_T, 0, REG_SZ, (const BYTE*)(LPCTSTR)strCommand,
			( strCommand.GetLength() + 1 ) * sizeof(TCHAR) );
	}
	else
	{
		RegDeleteValue( hKey, CLIENT_NAME_T );
	}

	RegCloseKey( hKey );
}

void CSettings::ClearSearches()
{
	for ( int i = 0; ; i++ )
	{
		CString strEntry;
		strEntry.Format( _T("Search.%.2i"), i + 1 );
		CString strValue( theApp.GetProfileString( _T("Search"), strEntry ) );
		if ( strValue.IsEmpty() )
			break;
		theApp.WriteProfileString( _T("Search"), strEntry, NULL );
	}
}

//////////////////////////////////////////////////////////////////////
// CSettings speed
//
//	Returns a nicely formatted string displaying a given transfer speed

const CString CSettings::SmartSpeed(QWORD nVolume, int nVolumeUnits, bool bTruncate) const
{
	CString strVolume;
	CString strUnit( _T("b/s") );
	int nUnits = bits;

	// Convert to bits or bytes
	nVolume *= nVolumeUnits;
	if ( General.RatesInBytes )
	{
		strUnit = _T("B/s");
		nVolume /= Bytes;
		nUnits = Bytes;
	}

	switch ( General.RatesUnit )
	{
	// smart units
	case 0:
		return SmartVolume( nVolume, nUnits, bTruncate ) + _T("/s");

	// bits - Bytes
	case 1:
		strVolume.Format( _T("%I64u %s"), nVolume, (LPCTSTR)strUnit );
		break;

	// Kilobits - KiloBytes
	case 2:
		strVolume.Format( _T("%.2lf K%s"), nVolume / fKilo, (LPCTSTR)strUnit );
		break;

	// Megabits - MegaBytes
	case 3:
		strVolume.Format( _T("%.2lf M%s"), nVolume / fMega, (LPCTSTR)strUnit );
		break;

	default:
		TRACE( _T("Unknown RatesUnit - %i"), General.RatesUnit );
		break;
	}

	// Add Unicode RTL marker if required
	return Settings.General.LanguageRTL ? _T("\x200E") + strVolume : strVolume;
}

//////////////////////////////////////////////////////////////////////
// CSettings volume
//
//	Returns a nicely formatted string displaying a given volume

const CString CSettings::SmartVolume(QWORD nVolume, int nVolumeUnits, bool bTruncate) const
{
	LPCTSTR szUnit = ( ! General.RatesInBytes && nVolumeUnits == bits ) ? _T("b") : _T("B");
	CString strVolume;
	CString strTruncate( bTruncate ? _T("%.0f") : _T("%.2f") );

	switch ( nVolumeUnits )
	{
	// nVolume is in bits - Bytes
	case bits:
	case Bytes:
		if ( nVolume < Kilo )						// bits - Bytes
		{
			strVolume.Format( _T("%I64u %s"), nVolume, szUnit );
			break;
		}
		else if ( nVolume < 10 * Kilo )				// 10 Kilobits - KiloBytes
		{
			strVolume.Format( strTruncate + _T(" K%s"), (double)nVolume / fKilo, szUnit );
			break;
		}

		// Convert to KiloBytes and drop through to next case
		nVolume /= Kilo;

	// nVolume is in Kilobits - Kilobytes
	case Kilobits:
	case KiloBytes:
		if ( nVolume < Kilo )			// Kilo
		{
			strVolume.Format( _T("%I64u K%s"), nVolume, szUnit );
		}
		else if ( nVolume < Mega )		// Mega
		{
			strVolume.Format( strTruncate + _T(" M%s"), (double)nVolume / fKilo, szUnit );
		}
		else
		{
			if ( nVolume < Giga )		// Giga
				strVolume.Format( strTruncate + _T(" G%s"), (double)nVolume / fMega, szUnit );
			else if ( nVolume < Tera )	// Tera
				strVolume.Format( strTruncate + _T(" T%s"), (double)nVolume / fGiga, szUnit );
			else if ( nVolume < Peta )	// Peta
				strVolume.Format( strTruncate + _T(" P%s"), (double)nVolume / fTera, szUnit );
			else						// Exa
				strVolume.Format( strTruncate + _T(" E%s"), (double)nVolume / fPeta, szUnit );
		}
	}

	// Add Unicode RTL marker if required
	return Settings.General.LanguageRTL ? _T("\x200E") + strVolume : strVolume;
}

QWORD CSettings::ParseVolume(const CString& strVolume, int nReturnUnits) const
{
	double val = 0;
	CString strSize( strVolume );

	// Skip Unicode RTL marker if it's present
	if ( strSize.Left( 1 ) == _T("\x200E") ) strSize = strSize.Mid( 1 );

	// Return early if there is no number in the string
	if ( _stscanf( strSize, _T("%lf"), &val ) != 1 ) return 0ul;

	// Return early if the number is negative
	if ( val < 0 ) return 0ul;

	if ( _tcsstr( strSize, _T("B") ) )
		// Convert to bits if Bytes were passed in
		val *= 8.0f;
	else if ( !_tcsstr( strSize, _T("b") ) )
		// If bits or Bytes are not indicated return 0
		return 0ul;

	// Work out what units are represented in the string
	if ( _tcsstr( strSize, _T("K") ) || _tcsstr( strSize, _T("k") ) )		// Kilo
		val *= fKilo;
	else if ( _tcsstr( strSize, _T("M") ) || _tcsstr( strSize, _T("m") ) )	// Mega
		val *= fMega;
	else if ( _tcsstr( strSize, _T("G") ) || _tcsstr( strSize, _T("g") ) )	// Giga
		val *= fGiga;
	else if ( _tcsstr( strSize, _T("T") ) || _tcsstr( strSize, _T("t") ) )	// Tera
		val *= fTera;
	else if ( _tcsstr( strSize, _T("P") ) || _tcsstr( strSize, _T("p") ) )	// Peta
		val *= fPeta;
	else if ( _tcsstr( strSize, _T("E") ) || _tcsstr( strSize, _T("e") ) )	// Exa
		val *= fExa;

	// Convert to required Units
	val /= nReturnUnits;

	// Convert double to DWORD and return
	return static_cast< QWORD >( val );
}

//////////////////////////////////////////////////////////////////////
// CSettings::CheckBandwidth

DWORD CSettings::GetOutgoingBandwidth() const
{	// This returns the available (Affected by limit) outgoing bandwidth in KB/s
	if ( Settings.Bandwidth.Uploads == 0 )
		return ( Settings.Connection.OutSpeed / 8 );

	return ( min( ( Settings.Connection.OutSpeed / 8 ), ( Settings.Bandwidth.Uploads / Kilo ) ) );
}

bool CSettings::GetValue(LPCTSTR pszPath, VARIANT* value)
{
	if ( value->vt != VT_EMPTY ) return false;

	CSettingsMap::const_iterator i = m_pSettingsTable.find( pszPath );
	if ( i == m_pSettingsTable.end() ) return false;
	Item* pItem = (*i).second;

	if ( pItem->m_pBool )
	{
		value->vt = VT_BOOL;
		value->boolVal = *pItem->m_pBool ? VARIANT_TRUE : VARIANT_FALSE;
	}
	else if ( pItem->m_pDword )
	{
		value->vt = VT_I4;
		value->lVal = (LONG)*pItem->m_pDword;
	}
	else if ( pItem->m_pFloat )
	{
		value->vt = VT_R4;
		value->fltVal = (float)*pItem->m_pFloat;
	}
	else if ( pItem->m_pString )
	{
		value->vt = VT_BSTR;
		value->bstrVal = SysAllocString( CT2CW( *pItem->m_pString ) );
	}
	else if ( pItem->m_pSet )
	{
		value->vt = VT_BSTR;
		value->bstrVal = SysAllocString( CT2CW( SaveSet( pItem->m_pSet ) ) );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// CSettings::Item construction and operations

void CSettings::Item::Load()
{
	if ( m_pBool )
	{
		ASSERT( ! m_pDword && ! m_pFloat && ! m_pString && ! m_pSet );
		ASSERT( m_nScale == 1 && m_nMin == 0 && m_nMax == 1 );
		*m_pBool = CRegistry::GetBool( m_szSection, m_szName, m_BoolDefault );
	}
	else if ( m_pDword )
	{
		ASSERT( ! m_pFloat && ! m_pString && ! m_pSet );
		ASSERT( ( m_nScale == 0 && m_nMin == 0 && m_nMax == 0 ) \
			|| ( m_nScale && m_nMin < m_nMax ) );
		*m_pDword = CRegistry::GetDword( m_szSection, m_szName, m_DwordDefault );
		if ( m_nScale && m_nMin < m_nMax )
		{
			ASSERT( ( m_DwordDefault >= m_nMin * m_nScale ) \
				&& ( m_DwordDefault <= m_nMax * m_nScale ) );
			*m_pDword = max( min( *m_pDword, m_nMax * m_nScale ), m_nMin * m_nScale );
		}
	}
	else if ( m_pFloat )
	{
		ASSERT( ! m_pString && ! m_pSet );
		ASSERT( m_nScale == 0 && m_nMin == 0 && m_nMax == 0 );
		*m_pFloat = CRegistry::GetFloat( m_szSection, m_szName, m_FloatDefault );
	}
	else if ( m_pString )
	{
		ASSERT( ! m_pSet );
		ASSERT( m_nScale == 0 && m_nMin == 0 && m_nMax == 0 );
		*m_pString = CRegistry::GetString( m_szSection, m_szName, m_StringDefault );
	}
	else
	{
		ASSERT( m_pSet );
		ASSERT( m_nScale == 0 && m_nMin == 0 && m_nMax == 0 );
		CString tmp( CRegistry::GetString( m_szSection, m_szName ) );
		LoadSet( m_pSet, tmp.IsEmpty() ? m_StringDefault : (LPCTSTR)tmp );
	}
}

void CSettings::Item::Save() const
{
	if ( m_pBool )
	{
		CRegistry::SetBool( m_szSection, m_szName, *m_pBool );
	}
	else if ( m_pDword )
	{
		CRegistry::SetDword( m_szSection, m_szName, *m_pDword );
	}
	else if ( m_pFloat )
	{
		CRegistry::SetFloat( m_szSection, m_szName, *m_pFloat );
	}
	else if ( m_pString )
	{
		CRegistry::SetString( m_szSection, m_szName, *m_pString );
	}
	else
	{
		CRegistry::SetString( m_szSection, m_szName, SaveSet( m_pSet ) );
	}
}

void CSettings::LoadSet(string_set* pSet, LPCTSTR pszString)
{
	pSet->clear();
	for( LPCTSTR start = pszString; start && *start; start++ )
	{
		LPCTSTR c = _tcschr( start, _T('|') );
		int len = c ? (int) ( c - start ) : (int) _tcslen( start );
		if ( len > 0 )
		{
			CString tmp;
			tmp.Append( start, len );
			pSet->insert( tmp );
		}
		if ( ! c )
			break;
		start = c;
	}
}

CString CSettings::SaveSet(const string_set* pSet)
{
	if ( pSet->begin() == pSet->end() )
		return CString();

	CString tmp( _T("|") );
	for( string_set::const_iterator i = pSet->begin(); i != pSet->end(); ++i )
	{
		tmp += *i;
		tmp += _T('|');
	}
	return tmp;
}

void CSettings::Item::Normalize()
{
	if ( m_pDword && m_nScale && m_nMin < m_nMax )
	{
		*m_pDword = max( min( *m_pDword, m_nMax * m_nScale ), m_nMin * m_nScale );
	}
}

bool CSettings::Item::IsDefault() const
{
	if ( m_pDword )
		return ( m_DwordDefault == *m_pDword );
	else if ( m_pBool )
		return ( m_BoolDefault == *m_pBool );
	else if ( m_pFloat )
		return ( m_FloatDefault == *m_pFloat );
	else if ( m_pString )
		return ( m_StringDefault == *m_pString );
	else
		return ( SaveSet( m_pSet ).CompareNoCase( m_StringDefault ) == 0 );
}

void CSettings::Item::SetDefault()
{
	if ( m_pDword )
		*m_pDword = m_DwordDefault;
	else if ( m_pBool )
		*m_pBool = m_BoolDefault;
	else if ( m_pFloat )
		*m_pFloat = m_FloatDefault;
	else if ( m_pString )
		*m_pString = m_StringDefault;
	else
		LoadSet( m_pSet, m_StringDefault );
}

template<>
void CSettings::Item::SetRange< CSpinButtonCtrl >(CSpinButtonCtrl& pCtrl)
{
	if ( m_pBool )
	{
		pCtrl.SetRange32( 0, 1 );
	}
	else if ( m_pDword )
	{
		pCtrl.SetRange32( m_nMin, m_nMax );
	}
}

template<>
void CSettings::Item::SetRange< CSliderCtrl >(CSliderCtrl& pCtrl)
{
	if ( m_pBool )
	{
		pCtrl.SetRange( 0, 1 );
	}
	else if ( m_pDword )
	{
		pCtrl.SetRange( m_nMin, m_nMax );
	}
}
