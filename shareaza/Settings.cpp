//
// Settings.cpp
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
#include "Schema.h"
#include "Skin.h"
#include "Registry.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CSettings Settings;


//////////////////////////////////////////////////////////////////////
// CSettings setup

void CSettings::Setup()
{
	Add( _T(".Path"), &General.Path, General.Path );
	Add( _T(".UserPath"), &General.UserPath, General.UserPath );
	Add( _T(".Debug"), &General.Debug, FALSE );
	Add( _T(".DebugLog"), &General.DebugLog, FALSE );
//	Add( _T(".DebugUPnP"), &General.DebugUPnP, FALSE );
	Add( _T(".DebugBTSources"), &General.DebugBTSources, FALSE );
	Add( _T(".MaxDebugLogSize"), &General.MaxDebugLogSize, 10*1024*1024 );
	Add( _T(".UpdateCheck"), &General.UpdateCheck, TRUE );
	Add( _T(".HashIntegrity"), &General.HashIntegrity, TRUE );
	Add( _T(".DiskSpaceWarning"), &General.DiskSpaceWarning, 500 );
	Add( _T(".DiskSpaceStop"), &General.DiskSpaceStop, 25 );
	Add( _T(".MinTransfersRest"), &General.MinTransfersRest, 15 );
	Add( _T("Settings.GUIMode"), &General.GUIMode, GUI_BASIC );
	Add( _T("Settings.CloseMode"), &General.CloseMode, 0 );
	Add( _T("Settings.TrayMinimise"), &General.TrayMinimise, FALSE );
	Add( _T("Settings.VerboseMode"), &General.VerboseMode, FALSE );
	Add( _T("Settings.ShowTimestamp"), &General.ShowTimestamp, TRUE );
	Add( _T("Settings.SizeLists"), &General.SizeLists, FALSE );
	Add( _T("Settings.RatesInBytes"), &General.RatesInBytes, TRUE );
	Add( _T("Settings.RatesUnit"), &General.RatesUnit, 0 );
	Add( _T("Settings.AlwaysOpenURLs"), &General.AlwaysOpenURLs, FALSE );
	Add( _T("Settings.Language"), &General.Language, _T("en") );
	Add( _T("Settings.IgnoreXPsp2"), &General.IgnoreXPsp2, FALSE );
	Add( _T("Settings.ItWasLimited"), &General.ItWasLimited, FALSE );

	Add( _T("Interface.TipDelay"), &Interface.TipDelay, 600 );
	Add( _T("Interface.TipAlpha"), &Interface.TipAlpha, 230 );
	Add( _T("Interface.TipSearch"), &Interface.TipSearch, TRUE );
	Add( _T("Interface.TipLibrary"), &Interface.TipLibrary, TRUE );
	Add( _T("Interface.TipDownloads"), &Interface.TipDownloads, TRUE );
	Add( _T("Interface.TipUploads"), &Interface.TipUploads, TRUE );
	Add( _T("Interface.TipNeighbours"), &Interface.TipNeighbours, TRUE );
	Add( _T("Interface.TipMedia"), &Interface.TipMedia, TRUE );
	Add( _T("Interface.LowResMode"), &Interface.LowResMode, FALSE );

	Add( _T("Library.WatchFolders"), &Library.WatchFolders, TRUE );
	Add( _T("Library.WatchFoldersTimeout"), &Library.WatchFoldersTimeout, 5 );
	Add( _T("Library.PartialMatch"), &Library.PartialMatch, TRUE );
	Add( _T("Library.VirtualFiles"), &Library.VirtualFiles, FALSE );
	Add( _T("Library.SourceMesh"), &Library.SourceMesh, TRUE );
	Add( _T("Library.SourceExpire"), &Library.SourceExpire, 86400 );
	Add( _T("Library.TigerHeight"), &Library.TigerHeight, 9 );
	Add( _T("Library.QueryRouteSize"), &Library.QueryRouteSize, 20 );
	Add( _T("Library.HistoryTotal"), &Library.HistoryTotal, 32 );
	Add( _T("Library.HistoryDays"), &Library.HistoryDays, 3 );
	Add( _T("Library.ShowVirtual"), &Library.ShowVirtual, TRUE );
	Add( _T("Library.TreeSize"), &Library.TreeSize, 200 );
	Add( _T("Library.PanelSize"), &Library.PanelSize, 120 );
	Add( _T("Library.ShowPanel"), &Library.ShowPanel, TRUE );
	Add( _T("Library.StoreViews"), &Library.StoreViews, TRUE );
	Add( _T("Library.ShowCoverArt"), &Library.ShowCoverArt, TRUE );
	Add( _T("Library.SchemaURI"), &Library.SchemaURI, CSchema::uriAudio );
	Add( _T("Library.FilterURI"), &Library.FilterURI, NULL );
	Add( _T("Library.SafeExecute"), &Library.SafeExecute, _T("|3gp|aac|ace|ape|avi|bmp|flv|gif|iso|jpg|jpeg|mid|mov|m1v|m2v|m3u|m4a|mkv|mp2|mp3|mp4|mpa|mpe|mpg|mpeg|ogg|ogm|pdf|png|qt|rar|rm|sks|tar|tgz|torrent|txt|wav|wma|wmv|zip|") );
	Add( _T("Library.PrivateTypes"), &Library.PrivateTypes, _T("|vbs|js|jc!|fb!|bc!|dbx|part|partial|pst|reget|getright|pif|lnk|sd|url|wab|m4p|infodb|racestats|chk|tmp|temp|ini|inf|log|old|manifest|met|bak|$$$|---|~~~|###|__incomplete___|") );
	Add( _T("Library.ThumbSize"), &Library.ThumbSize, 96 );
	Add( _T("Library.BitziAgent"), &Library.BitziAgent, _T(".") );
	Add( _T("Library.BitziWebView"), &Library.BitziWebView, _T("http://bitzi.com/lookup/(URN)?v=detail&ref=shareaza") );
	Add( _T("Library.BitziWebSubmit"), &Library.BitziWebSubmit, _T("http://bitzi.com/lookup/(SHA1).(TTH)?fl=(SIZE)&ff=(FIRST20)&fn=(NAME)&tag.ed2k.ed2khash=(ED2K)&(INFO)&a=(AGENT)&v=Q0.4&ref=shareaza") );
	Add( _T("Library.BitziXML"), &Library.BitziXML, _T("http://ticket.bitzi.com/rdf/(SHA1).(TTH)") );
	Add( _T("Library.BitziOkay"), &Library.BitziOkay, FALSE );
	Add( _T("Library.HighPriorityHash"), &Library.HighPriorityHash, FALSE );
	Add( _T("Library.HashWindow"), &Library.HashWindow, TRUE );
	Add( _T("Library.CreateGhosts"), &Library.CreateGhosts, TRUE );
//	Add( _T("Library.BufferSize"), &Library.BufferSize, 0 );
//	Add( _T("Library.Parallel"), &Library.Parallel, 0 );
	Add( _T("Library.HighPriorityHashing"), &Library.HighPriorityHashing, 20);
	Add( _T("Library.LowPriorityHashing"), &Library.LowPriorityHashing, 2 );
	Add( _T("Library.MaxMaliciousFileSize"), &Library.MaxMaliciousFileSize, 1024 );
	Add( _T("Library.PreferAPETags"), &Library.PreferAPETags, 1 );
	Add( _T("Library.UseFolderGUID"), &Library.UseFolderGUID, 1 );
	Add( _T("Library.MarkFileAsDownload"), &Library.MarkFileAsDownload, 1 );
	Add( _T("Library.UseCustomFolders"), &Library.UseCustomFolders, 1 );

	Add( _T("Search.LastSchemaURI"), &Search.LastSchemaURI, _T("") );
	Add( _T("Search.BlankSchemaURI"), &Search.BlankSchemaURI, CSchema::uriAudio );
	Add( _T("Search.HideSearchPanel"), &Search.HideSearchPanel, FALSE );
	Add( _T("Search.SearchPanel"), &Search.SearchPanel, TRUE );
	Add( _T("Search.ExpandMatches"), &Search.ExpandMatches, FALSE );
	Add( _T("Search.HighlightNew"), &Search.HighlightNew, TRUE );
	Add( _T("Search.SwitchToTransfers"), &Search.SwitchToTransfers, TRUE );
	Add( _T("Search.SchemaTypes"), &Search.SchemaTypes, TRUE );
	Add( _T("Search.ShowNames"), &Search.ShowNames, TRUE );
	Add( _T("Search.FilterMask"), &Search.FilterMask, 0x168 );
	Add( _T("Search.MonitorSchemaURI"), &Search.MonitorSchemaURI, CSchema::uriAudio );
	Add( _T("Search.MonitorFilter"), &Search.MonitorFilter, NULL );
	Add( _T("Search.MonitorQueue"), &Search.MonitorQueue, 128 );
	Add( _T("Search.BrowseTreeSize"), &Search.BrowseTreeSize, 180 );
	Add( _T("Search.DetailPanelVisible"), &Search.DetailPanelVisible, TRUE );
	Add( _T("Search.DetailPanelSize"), &Search.DetailPanelSize, 100 );
	Add( _T("Search.MaxPreviewLength"), &Search.MaxPreviewLength, 20*1024 );
	Add( _T("Search.AdultFilter"), &Search.AdultFilter, FALSE );
	Add( _T("Search.AdvancedPanel"), &Search.AdvancedPanel, TRUE );
	Add( _T("Search.GeneralThrottle"), &Search.GeneralThrottle, 200 );
	Add( _T("Search.SpamFilterThreshold"), &Search.SpamFilterThreshold, 20 );

	Add( _T("MediaPlayer.EnablePlay"), &MediaPlayer.EnablePlay, TRUE );
	Add( _T("MediaPlayer.EnableEnqueue"), &MediaPlayer.EnableEnqueue, TRUE );
	Add( _T("MediaPlayer.FileTypes"), &MediaPlayer.FileTypes, _T("|asx|wax|m3u|wvx|wmx|asf|wav|snd|au|aif|aifc|aiff|wma|mp3|cda|mid|rmi|midi|avi|mpeg|mpg|m1v|mp2|mpa|mpe|wmv|") );
	Add( _T("MediaPlayer.Repeat"), &MediaPlayer.Repeat, FALSE );
	Add( _T("MediaPlayer.Random"), &MediaPlayer.Random, FALSE );
	Add( _T("MediaPlayer.Zoom"), (DWORD*)&MediaPlayer.Zoom, smzFill );
	Add( _T("MediaPlayer.Aspect"), &MediaPlayer.Aspect, smaDefault );
	Add( _T("MediaPlayer.Volume"), &MediaPlayer.Volume, 1.0f );
	Add( _T("MediaPlayer.ListVisible"), &MediaPlayer.ListVisible, TRUE );
	Add( _T("MediaPlayer.ListSize"), &MediaPlayer.ListSize, 240 );
	Add( _T("MediaPlayer.StatusVisible"), &MediaPlayer.StatusVisible, TRUE );
	Add( _T("MediaPlayer.MediaServicesCLSID"), &MediaPlayer.MediaServicesCLSID, _T("{3DC28AA6-A597-4E03-96DF-ADA19155B0BE}") );
	Add( _T("MediaPlayer.Mpeg1PreviewCLSID"), &MediaPlayer.Mpeg1PreviewCLSID, _T("{9AA8DF47-B8FE-47da-AB1A-2DAA0DA0B646}") );
	Add( _T("MediaPlayer.Mp3PreviewCLSID"), &MediaPlayer.Mp3PreviewCLSID, _T("{BF00DBCC-90A2-4f46-8171-7D4F929D035F}") );
	Add( _T("MediaPlayer.AviPreviewCLSID"), &MediaPlayer.AviPreviewCLSID, _T("{394011F0-6D5C-42a3-96C6-24B9AD6B010C}") );
	Add( _T("MediaPlayer.VisWrapperCLSID"), &MediaPlayer.VisWrapperCLSID, _T("{C3B7B25C-6B8B-481A-BC48-59F9A6F7B69A}") );
	Add( _T("MediaPlayer.VisSoniqueCLSID"), &MediaPlayer.VisSoniqueCLSID, _T("{D07E630D-A850-4f11-AD29-3D3848B67EFE}") );
	Add( _T("MediaPlayer.VisCLSID"), &MediaPlayer.VisCLSID, _T("{591A5CFF-3172-4020-A067-238542DDE9C2}") );
	Add( _T("MediaPlayer.VisPath"), &MediaPlayer.VisPath, _T("") );
	Add( _T("MediaPlayer.VisSize"), &MediaPlayer.VisSize, 1 );
	Add( _T("MediaPlayer.ServicePath"), &MediaPlayer.ServicePath, _T("") );
	Add( _T("MediaPlayer.ShortPaths"), &MediaPlayer.ShortPaths, FALSE );

	Add( _T("Web.Magnet"), &Web.Magnet, TRUE );
	Add( _T("Web.Gnutella"), &Web.Gnutella, TRUE );
	Add( _T("Web.ED2K"), &Web.ED2K, TRUE );
	Add( _T("Web.Piolet"), &Web.Piolet, TRUE );
	Add( _T("Web.Torrent"), &Web.Torrent, TRUE );

	Add( _T("Connection.AutoConnect"), &Connection.AutoConnect, TRUE );
	Add( _T("Connection.FirewallState"), &Connection.FirewallState, CONNECTION_AUTO );
	Add( _T("Connection.OutHost"), &Connection.OutHost, NULL );
	Add( _T("Connection.InHost"), &Connection.InHost, NULL );
	Add( _T("Connection.InPort"), &Connection.InPort, GNUTELLA_DEFAULT_PORT );
	Add( _T("Connection.InBind"), &Connection.InBind, FALSE );
	Add( _T("Connection.RandomPort"), &Connection.RandomPort, FALSE );
	Add( _T("Connection.InSpeed"), &Connection.InSpeed, 2048 );
	Add( _T("Connection.OutSpeed"), &Connection.OutSpeed, 256 );
	Add( _T("Connection.IgnoreLocalIP"), &Connection.IgnoreLocalIP, TRUE );
	Add( _T("Connection.IgnoreOwnIP"), &Connection.IgnoreOwnIP, TRUE );
	Add( _T("Connection.TimeoutConnect"), &Connection.TimeoutConnect, 16000 );
	Add( _T("Connection.TimeoutHandshake"), &Connection.TimeoutHandshake, 45000 );
	Add( _T("Connection.TimeoutTraffic"), &Connection.TimeoutTraffic, 140000 );
	Add( _T("Connection.SendBuffer"), &Connection.SendBuffer, 2048 );
//	Add( _T("Connection.AsyncIO"), &Connection.AsyncIO, TRUE );
	Add( _T("Connection.RequireForTransfers"), &Connection.RequireForTransfers, TRUE );
	Add( _T("Connection.ConnectThrottle"), &Connection.ConnectThrottle, 0 );
	Add( _T("Connection.FailurePenalty"), &Connection.FailurePenalty, 300 );
	Add( _T("Connection.FailureLimit"), &Connection.FailureLimit, 3 );
	Add( _T("Connection.DetectConnectionLoss"), &Connection.DetectConnectionLoss, TRUE );
	Add( _T("Connection.DetectConnectionReset"), &Connection.DetectConnectionReset, FALSE );
	Add( _T("Connection.ForceConnectedState"), &Connection.ForceConnectedState, TRUE );
	Add( _T("Connection.SlowConnect"), &Connection.SlowConnect, FALSE );
	Add( _T("Connection.EnableFirewallException"), &Connection.EnableFirewallException, TRUE );
	Add( _T("Connection.DeleteFirewallException"), &Connection.DeleteFirewallException, FALSE );
	Add( _T("Connection.EnableUPnP"), &Connection.EnableUPnP, FALSE );
	Add( _T("Connection.DeleteUPnPPorts"), &Connection.DeleteUPnPPorts, TRUE );
	Add( _T("Connection.SkipWANPPPSetup"), &Connection.SkipWANPPPSetup, FALSE );
	Add( _T("Connection.SkipWANIPSetup"), &Connection.SkipWANIPSetup, FALSE );

	Add( _T("Bandwidth.Request"), &Bandwidth.Request, 4096 );
	Add( _T("Bandwidth.HubIn"), &Bandwidth.HubIn, 0 );
	Add( _T("Bandwidth.HubOut"), &Bandwidth.HubOut, 0 );
	Add( _T("Bandwidth.LeafIn"), &Bandwidth.LeafIn, 0 );
	Add( _T("Bandwidth.LeafOut"), &Bandwidth.LeafOut, 0 );
	Add( _T("Bandwidth.PeerIn"), &Bandwidth.PeerIn, 0 );
	Add( _T("Bandwidth.PeerOut"), &Bandwidth.PeerOut, 0 );
	Add( _T("Bandwidth.UdpOut"), &Bandwidth.UdpOut, 0 );
	Add( _T("Bandwidth.Downloads"), &Bandwidth.Downloads, 0 );
	Add( _T("Bandwidth.Uploads"), &Bandwidth.Uploads, 0 );
	Add( _T("Bandwidth.HubUploads"), &Bandwidth.HubUploads, 5120 );

	Add( _T("Community.ChatEnable"), &Community.ChatEnable, TRUE );
	Add( _T("Community.ChatAllNetworks"), &Community.ChatAllNetworks, TRUE );
	Add( _T("Community.ChatFilter"), &Community.ChatFilter, TRUE );
	Add( _T("Community.ChatFilterED2K"), &Community.ChatFilterED2K, TRUE );
	Add( _T("Community.ChatCensor"), &Community.ChatCensor, FALSE );

	Add( _T("Community.Timestamp"), &Community.Timestamp, TRUE );
	Add( _T("Community.ServeProfile"), &Community.ServeProfile, TRUE );
	Add( _T("Community.ServeFiles"), &Community.ServeFiles, TRUE );
	Add( _T("Community.AwayMessageIdleTime"), &Community.AwayMessageIdleTime, 20*60 );

	Add( _T("Discovery.AccessThrottle"), &Discovery.AccessThrottle, 3600 );
	Add( _T("Discovery.Lowpoint"), &Discovery.Lowpoint, 10 );
	Add( _T("Discovery.FailureLimit"), &Discovery.FailureLimit, 2 );
	Add( _T("Discovery.UpdatePeriod"), &Discovery.UpdatePeriod, 1800 );
	Add( _T("Discovery.DefaultUpdate"), &Discovery.DefaultUpdate, 3600 );
	Add( _T("Discovery.BootstrapCount"), &Discovery.BootstrapCount, 10 );
	Add( _T("Discovery.G2DAddress"), &Discovery.G2DAddress, _T("stats.shareaza.com:6446") );
	Add( _T("Discovery.G2DRetryAfter"), &Discovery.G2DRetryAfter, 0 );
	Add( _T("Discovery.CacheCount"), &Discovery.CacheCount, 50 );
	Add( _T("Discovery.EnableG1GWC"), &Discovery.EnableG1GWC, FALSE );

	Add( _T("Gnutella.ConnectFactor"), &Gnutella.ConnectFactor, 4 );
	Add( _T("Gnutella.DeflateHub2Hub"), &Gnutella.DeflateHub2Hub, TRUE );
	Add( _T("Gnutella.DeflateLeaf2Hub"), &Gnutella.DeflateLeaf2Hub, FALSE );
	Add( _T("Gnutella.DeflateHub2Leaf"), &Gnutella.DeflateHub2Leaf, TRUE );
	Add( _T("Gnutella.MaxResults"), &Gnutella.MaxResults, 150 );
	Add( _T("Gnutella.MaxHits"), &Gnutella.MaxHits, 64 );
	Add( _T("Gnutella.HitsPerPacket"), &Gnutella.HitsPerPacket, 64 );
	Add( _T("Gnutella.RouteCache"), &Gnutella.RouteCache, 600 );
	Add( _T("Gnutella.HostCacheCount"), &Gnutella.HostCacheSize, 1024 );
	Add( _T("Gnutella.HostCacheView"), &Gnutella.HostCacheView, PROTOCOL_ED2K );
	Add( _T("Gnutella.ConnectThrottle"), &Gnutella.ConnectThrottle, 120 );
	Add( _T("Gnutella.BlockBlankClients"), &Gnutella.BlockBlankClients, TRUE );
	Add( _T("Gnutella.SpecifyProtocol"), &Gnutella.SpecifyProtocol, TRUE );

	Add( _T("Gnutella1.ClientMode"), &Gnutella1.ClientMode, MODE_LEAF );
	Add( _T("Gnutella1.EnableAlways"), &Gnutella1.EnableAlways, FALSE );
	Add( _T("Gnutella1.NumHubs"), &Gnutella1.NumHubs, 3 );
	Add( _T("Gnutella1.NumLeafs"), &Gnutella1.NumLeafs, 0 );
	Add( _T("Gnutella1.NumPeers"), &Gnutella1.NumPeers, 32 ); // For X-Degree
	Add( _T("Gnutella1.PacketBufferSize"), &Gnutella1.PacketBufferSize, 64 );
	Add( _T("Gnutella1.PacketBufferTime"), &Gnutella1.PacketBufferTime, 60000 );
	Add( _T("Gnutella1.DefaultTTL"), &Gnutella1.DefaultTTL, 3 );
	Add( _T("Gnutella1.SearchTTL"), &Gnutella1.SearchTTL, 3 );
	Add( _T("Gnutella1.TranslateTTL"), &Gnutella1.TranslateTTL, 2 );
	Add( _T("Gnutella1.MaximumTTL"), &Gnutella1.MaximumTTL, 10 );
	Add( _T("Gnutella1.MaximumPacket"), &Gnutella1.MaximumPacket, 65535 );
	Add( _T("Gnutella1.MaximumQuery"), &Gnutella1.MaximumQuery, 256 );
	Add( _T("Gnutella1.StrictPackets"), &Gnutella1.StrictPackets, FALSE );
	Add( _T("Gnutella1.EnableGGEP"), &Gnutella1.EnableGGEP, TRUE );
	Add( _T("Gnutella1.VendorMsg"), &Gnutella1.VendorMsg, TRUE );
	Add( _T("Gnutella1.QueryThrottle"), &Gnutella1.QueryThrottle, 30 );
	Add( _T("Gnutella1.RequeryDelay"), &Gnutella1.RequeryDelay, 30 );
	Add( _T("Gnutella1.HostExpire"), &Gnutella1.HostExpire, 2 * 24 * 60 * 60 );
	Add( _T("Gnutella1.PingFlood"), &Gnutella1.PingFlood, 3000 );
	Add( _T("Gnutella1.PingRate"), &Gnutella1.PingRate, 30000 );
	Add( _T("Gnutella1.PongCache"), &Gnutella1.PongCache, 10000 );
	Add( _T("Gnutella1.PongCount"), &Gnutella1.PongCount, 10 );
	Add( _T("Gnutella1.QueueLimter"), &Gnutella1.HitQueueLimit, 50 );
	Add( _T("Gnutella1.QuerySearchUTF8"), &Gnutella1.QuerySearchUTF8, TRUE );
	Add( _T("Gnutella1.QueryHitUTF8"), &Gnutella1.QueryHitUTF8, TRUE );
	Add( _T("Gnutella1.MaxHostsInPongs"), &Settings.Gnutella1.MaxHostsInPongs, 10 );

	Add( _T("Gnutella2.ClientMode"), &Gnutella2.ClientMode, MODE_AUTO );
	Add( _T("Gnutella2.HubVerified"), &Gnutella2.HubVerified, FALSE );
	Add( _T("Gnutella2.EnableAlways"), &Gnutella2.EnableAlways, TRUE );
	Add( _T("Gnutella2.NumHubs"), &Gnutella2.NumHubs, 2 );
	Add( _T("Gnutella2.NumLeafs"), &Gnutella2.NumLeafs, 300 );
	Add( _T("Gnutella2.NumPeers"), &Gnutella2.NumPeers, 6 );
	Add( _T("Gnutella2.PingRelayLimit"), &Gnutella2.PingRelayLimit, 10);
	Add( _T("Gnutella2.UdpMTU"), &Gnutella2.UdpMTU, 500 );
	Add( _T("Gnutella2.UdpBuffers"), &Gnutella2.UdpBuffers, 512 );
	Add( _T("Gnutella2.UdpInFrames"), &Gnutella2.UdpInFrames, 256 );
	Add( _T("Gnutella2.UdpOutFrames"), &Gnutella2.UdpOutFrames, 256 );
	Add( _T("Gnutella2.UdpGlobalThrottle"), &Gnutella2.UdpGlobalThrottle, 1 );
	Add( _T("Gnutella2.UdpOutExpire"), &Gnutella2.UdpOutExpire, 26000 );
	Add( _T("Gnutella2.UdpOutResend"), &Gnutella2.UdpOutResend, 6000 );
	Add( _T("Gnutella2.UdpInExpire"), &Gnutella2.UdpInExpire, 30000 );
	Add( _T("Gnutella2.LNIPeriod"), &Gnutella2.LNIPeriod, 60000 );
	Add( _T("Gnutella2.KHLPeriod"), &Gnutella2.KHLPeriod, 60000 );
	Add( _T("Gnutella2.KHLHubCount"), &Gnutella2.KHLHubCount, 50 );
	Add( _T("Gnutella2.HAWPeriod"), &Gnutella2.HAWPeriod, 300000 );
	Add( _T("Gnutella2.HostExpire"), &Gnutella2.HostExpire, 2 * 24 * 60 * 60 );
	Add( _T("Gnutella2.HostCurrent"), &Gnutella2.HostCurrent, 10 * 60 );
	Add( _T("Gnutella2.PingRate"), &Gnutella2.PingRate, 15000 );
	Add( _T("Gnutella2.QueryGlobalThrottle"), &Gnutella2.QueryGlobalThrottle, 125 );
	Add( _T("Gnutella2.QueryHostThrottle"), &Gnutella2.QueryHostThrottle, 120 );
	Add( _T("Gnutella2.QueryHostDeadline"), &Gnutella2.QueryHostDeadline, 10*60 );
	Add( _T("Gnutella2.RequeryDelay"), &Gnutella2.RequeryDelay, 4*60*60 );
	Add( _T("Gnutella2.HubHorizonSize"), &Gnutella2.HubHorizonSize, 128 );
	Add( _T("Gnutella2.QueryLimit"), &Gnutella2.QueryLimit, 2400 );

	Add( _T("eDonkey.EnableAlways"), &eDonkey.EnableAlways, FALSE );
	Add( _T("eDonkey.FastConnect"), &eDonkey.FastConnect, FALSE );
	Add( _T("eDonkey.ForceHighID"), &eDonkey.ForceHighID, TRUE );
	Add( _T("eDonkey.NumServers"), &eDonkey.NumServers, 1 );
	Add( _T("eDonkey.MaxLinks"), &eDonkey.MaxLinks, 200 );
	Add( _T("eDonkey.MaxResults"), &eDonkey.MaxResults, 100 );
	Add( _T("eDonkey.MaxShareCount"), &eDonkey.MaxShareCount, 1000 );
	Add( _T("eDonkey.ServerWalk"), &eDonkey.ServerWalk, TRUE );
	Add( _T("eDonkey.StatsServerThrottle"), &eDonkey.StatsServerThrottle, 7*24*60*60 );
	Add( _T("eDonkey.StatsGlobalThrottle"), &eDonkey.StatsGlobalThrottle, 30*60*1000 );
	Add( _T("eDonkey.QueryGlobalThrottle"), &eDonkey.QueryGlobalThrottle, 1000 );
	Add( _T("eDonkey.QueryServerThrottle"), &eDonkey.QueryServerThrottle, 120 );
	Add( _T("eDonkey.QueryFileThrottle"), &eDonkey.QueryFileThrottle, 60*60*1000 );
	Add( _T("eDonkey.GetSourcesThrottle"), &eDonkey.GetSourcesThrottle, 8*60*60*1000 );
	Add( _T("eDonkey.QueueRankThrottle"), &eDonkey.QueueRankThrottle, 2*60*1000 );
	Add( _T("eDonkey.PacketThrottle"), &eDonkey.PacketThrottle, 500 );
	Add( _T("eDonkey.SourceThrottle"), &eDonkey.SourceThrottle, 1000 );
	Add( _T("eDonkey.MetAutoQuery"), &eDonkey.MetAutoQuery, TRUE );
	Add( _T("eDonkey.LearnNewServers"), &eDonkey.LearnNewServers, TRUE );
	Add( _T("eDonkey.LearnNewServersClient"), &eDonkey.LearnNewServersClient, FALSE );
	Add( _T("eDonkey.ServerListURL"), &eDonkey.ServerListURL, _T("http://ocbmaurice.no-ip.org/pl/slist.pl/server.met?download/server-good.met") );
	Add( _T("eDonkey.RequestPipe"), &eDonkey.RequestPipe, 3 );
	Add( _T("eDonkey.RequestSize"), &eDonkey.RequestSize, 180*1024/2 );
	Add( _T("eDonkey.FrameSize"), &eDonkey.FrameSize, 10240 );
	Add( _T("eDonkey.ReAskTime"), &eDonkey.ReAskTime, 1740 );
	Add( _T("eDonkey.DequeueTime"), &eDonkey.DequeueTime, 3610 );
	Add( _T("eDonkey.TagNames"), &eDonkey.TagNames, TRUE );
	Add( _T("eDonkey.ExtendedRequest"), &eDonkey.ExtendedRequest, 2 );
	Add( _T("eDonkey.SendPortServer"), &eDonkey.SendPortServer, FALSE );
	Add( _T("eDonkey.MagnetSearch"), &eDonkey.MagnetSearch, TRUE );
	Add( _T("eDonkey.MinServerFileSize"), &eDonkey.MinServerFileSize, 0 );
	Add( _T("eDonkey.DefaultServerFlags"), &eDonkey.DefaultServerFlags, 0xFFFFFFFF );
	Add( _T("eDonkey.Endgame"), &eDonkey.Endgame, TRUE );
	Add( _T("eDonkey.LargeFileSupport"), &eDonkey.LargeFileSupport, FALSE );

	Add( _T("BitTorrent.AdvancedInterface"), &BitTorrent.AdvancedInterface, FALSE );
	Add( _T("BitTorrent.AdvancedInterfaceSet"), &BitTorrent.AdvancedInterfaceSet, FALSE );
	Add( _T("BitTorrent.TorrentCreatorPath"), &BitTorrent.TorrentCreatorPath, _T("") );
	Add( _T("BitTorrent.DefaultTracker"), &BitTorrent.DefaultTracker, _T("") );
	Add( _T("BitTorrent.DefaultTrackerPeriod"), &BitTorrent.DefaultTrackerPeriod, 5*60000 );
	Add( _T("BitTorrent.MaxTrackerRetry"), &BitTorrent.MaxTrackerRetry, 3 );
	Add( _T("BitTorrent.TorrentCodePage"), &BitTorrent.TorrentCodePage, 0 );
	Add( _T("BitTorrent.TorrentExtraKeys"), &BitTorrent.TorrentExtraKeys, TRUE );
	Add( _T("BitTorrent.TorrentIgnoreErrors"), &BitTorrent.TorrentIgnoreErrors, FALSE );
	Add( _T("BitTorrent.LinkTimeout"), &BitTorrent.LinkTimeout, 180000 );
	Add( _T("BitTorrent.LinkPing"), &BitTorrent.LinkPing, 120000 );
	Add( _T("BitTorrent.RequestPipe"), &BitTorrent.RequestPipe, 4 );
	Add( _T("BitTorrent.RequestSize"), &BitTorrent.RequestSize, 16384 );
	Add( _T("BitTorrent.RequestLimit"), &BitTorrent.RequestLimit, 131072 );
	Add( _T("BitTorrent.RandomPeriod"), &BitTorrent.RandomPeriod, 30000 );
	Add( _T("BitTorrent.SourceExchangePeriod"), &BitTorrent.SourceExchangePeriod, 10 );
	Add( _T("BitTorrent.UploadCount"), &BitTorrent.UploadCount, 4 );
	Add( _T("BitTorrent.DownloadConnections"), &BitTorrent.DownloadConnections, 40 );
	Add( _T("BitTorrent.DownloadTorrents"), &BitTorrent.DownloadTorrents, 3 );
	Add( _T("BitTorrent.Endgame"), &BitTorrent.Endgame, TRUE );
	Add( _T("BitTorrent.AutoClear"), &BitTorrent.AutoClear, FALSE );
	Add( _T("BitTorrent.ClearRatio"), &BitTorrent.ClearRatio, 120 );
	Add( _T("BitTorrent.AutoSeed"), &BitTorrent.AutoSeed, TRUE );
	Add( _T("BitTorrent.BandwidthPercentage"), &BitTorrent.BandwidthPercentage, 80 );
	Add( _T("BitTorrent.TrackerKey"), &BitTorrent.TrackerKey, TRUE );
	Add( _T("BitTorrent.StandardPeerID"), &BitTorrent.StandardPeerID, TRUE );
	Add( _T("BitTorrent.PreferenceBTSources"), &BitTorrent.PreferenceBTSources, TRUE );

	Add( _T("Downloads.IncompletePath"), &Downloads.IncompletePath, General.UserPath + _T("\\Incomplete") );
	Add( _T("Downloads.CompletePath"), &Downloads.CompletePath, General.UserPath + _T("\\Downloads") );
	Add( _T("Downloads.TorrentPath"), &Downloads.TorrentPath, General.UserPath + _T("\\Torrents") );
	Add( _T("Downloads.CollectionPath"), &Downloads.CollectionPath, General.UserPath + _T("\\Collections") );
	Add( _T("Downloads.BufferSize"), &Downloads.BufferSize, 81920 );
	Add( _T("Downloads.SparseThreshold"), &Downloads.SparseThreshold, 8 * 1024 );
	Add( _T("Downloads.MaxAllowedFailures"), &Downloads.MaxAllowedFailures, 10 );
	Add( _T("Downloads.MaxFiles"), &Downloads.MaxFiles, 26 );
	Add( _T("Downloads.MaxTransfers"), &Downloads.MaxTransfers, 100 );
	Add( _T("Downloads.MaxFileTransfers"), &Downloads.MaxFileTransfers, 10 );
	Add( _T("Downloads.MaxFileSearches"), &Downloads.MaxFileSearches, 2 );
	Add( _T("Downloads.MaxConnectingSources"), &Downloads.MaxConnectingSources, 28 );
	Add( _T("Downloads.MinSources"), &Downloads.MinSources, 1 );
	Add( _T("Downloads.ConnectThrottle"), &Downloads.ConnectThrottle, 250 );
	Add( _T("Downloads.QueueLimit"), &Downloads.QueueLimit, 0 );
	Add( _T("Downloads.SearchPeriod"), &Downloads.SearchPeriod, 120000 );
	Add( _T("Downloads.StarveTimeout"), &Downloads.StarveTimeout, 2700 );
	Add( _T("Downloads.StarveGiveUp"), &Downloads.StarveGiveUp, 3 );
	Add( _T("Downloads.RetryDelay"), &Downloads.RetryDelay, 10*60000 );
	Add( _T("Downloads.PushTimeout"), &Downloads.PushTimeout, 45000 );
	Add( _T("Downloads.StaggardStart"), &Downloads.StaggardStart, FALSE );
	Add( _T("Downloads.AllowBackwards"), &Downloads.AllowBackwards, TRUE );
	Add( _T("Downloads.ChunkSize"), &Downloads.ChunkSize, 512*1024 );
	Add( _T("Downloads.ChunkStrap"), &Downloads.ChunkStrap, 128*1024 );
	Add( _T("Downloads.Metadata"), &Downloads.Metadata, TRUE );
	Add( _T("Downloads.VerifyFiles"), &Downloads.VerifyFiles, TRUE );
	Add( _T("Downloads.VerifyTiger"), &Downloads.VerifyTiger, TRUE );
	Add( _T("Downloads.VerifyED2K"), &Downloads.VerifyED2K, TRUE );
	Add( _T("Downloads.NeverDrop"), &Downloads.NeverDrop, FALSE );
	Add( _T("Downloads.RequestHash"), &Downloads.RequestHash, TRUE );
	Add( _T("Downloads.RequestHTTP11"), &Downloads.RequestHTTP11, TRUE );
	Add( _T("Downloads.RequestURLENC"), &Downloads.RequestURLENC, TRUE );
	Add( _T("Downloads.SaveInterval"), &Downloads.SaveInterval, 60000 );
	Add( _T("Downloads.FlushSD"), &Downloads.FlushSD, TRUE );
	Add( _T("Downloads.ShowSources"), &Downloads.ShowSources, FALSE );
	Add( _T("Downloads.SimpleBar"), &Downloads.SimpleBar, FALSE );
	Add( _T("Downloads.ShowPercent"), &Downloads.ShowPercent, FALSE );
	Add( _T("Downloads.ShowGroups"), &Downloads.ShowGroups, TRUE );
	Add( _T("Downloads.AutoExpand"), &Downloads.AutoExpand, FALSE );
	Add( _T("Downloads.AutoClear"), &Downloads.AutoClear, FALSE );
	Add( _T("Downloads.ClearDelay"), &Downloads.ClearDelay, 30000 );
	Add( _T("Downloads.FilterMask"), &Downloads.FilterMask, 0xFFFFFFFF );
	Add( _T("Downloads.ShowMonitorURLs"), &Downloads.ShowMonitorURLs, TRUE );
	Add( _T("Downloads.SortColumns"), &Downloads.SortColumns, TRUE );
	Add( _T("Downloads.SortSources"), &Downloads.SortSources, TRUE );
	Add( _T("Downloads.SourcesWanted"), &Downloads.SourcesWanted, 500 );
	Add( _T("Downloads.MaxReviews"), &Downloads.MaxReviews, 64 );

	Add( _T("Uploads.MaxPerHost"), &Uploads.MaxPerHost, 2 );
	Add( _T("Uploads.FreeBandwidthValue"), &Uploads.FreeBandwidthValue, 2560 );
	Add( _T("Uploads.FreeBandwidthFactor"), &Uploads.FreeBandwidthFactor, 15 );
	Add( _T("Uploads.ClampdownFactor"), &Uploads.ClampdownFactor, 20 );
	Add( _T("Uploads.ClampdownFloor"), &Uploads.ClampdownFloor, 1024 );
	Add( _T("Uploads.ThrottleMode"), &Uploads.ThrottleMode, TRUE );
	Add( _T("Uploads.QueuePollMin"), &Uploads.QueuePollMin, 45000 );
	Add( _T("Uploads.QueuePollMax"), &Uploads.QueuePollMax, 120000 );
	Add( _T("Uploads.RotateChunkLimit"), &Uploads.RotateChunkLimit, 1024*1024 );
	Add( _T("Uploads.SharePartials"), &Uploads.SharePartials, TRUE );
	Add( _T("Uploads.ShareTiger"), &Uploads.ShareTiger, TRUE );
	Add( _T("Uploads.ShareHashset"), &Uploads.ShareHashset, TRUE );
	Add( _T("Uploads.ShareMetadata"), &Uploads.ShareMetadata, TRUE );
	Add( _T("Uploads.SharePreviews"), &Uploads.SharePreviews, TRUE );
	Add( _T("Uploads.DynamicPreviews"), &Uploads.DynamicPreviews, TRUE );
	Add( _T("Uploads.PreviewQuality"), &Uploads.PreviewQuality, 40 );
	Add( _T("Uploads.PreviewTransfers"), &Uploads.PreviewTransfers, 3 );
	Add( _T("Uploads.AllowBackwards"), &Uploads.AllowBackwards, TRUE );
	Add( _T("Uploads.HubUnshare"), &Uploads.HubUnshare, TRUE );
	Add( _T("Uploads.BlockAgents"), &Uploads.BlockAgents, _T("|Mozilla|") );
	Add( _T("Uploads.AutoClear"), &Uploads.AutoClear, FALSE );
	Add( _T("Uploads.ClearDelay"), &Uploads.ClearDelay, 30000 );
	Add( _T("Uploads.FilterMask"), &Uploads.FilterMask, 0xFFFFFFFD );
	Add( _T("Uploads.RewardQueuePercentage"), &Uploads.RewardQueuePercentage, 10 );

	Add( _T("Remote.Enable"), &Remote.Enable, FALSE );
	Add( _T("Remote.Username"), &Remote.Username, _T("") );
	Add( _T("Remote.Password"), &Remote.Password, _T("") );

	Add( _T("Scheduler.Enable"), &Scheduler.Enable, FALSE );
	Add( _T("Scheduler.LimitedBandwidth"), &Scheduler.LimitedBandwidth, 50 );
	Add( _T("Scheduler.LimitedNetworks"), &Scheduler.LimitedNetworks, TRUE );
	Add( _T("Scheduler.AllowHub"), &Scheduler.AllowHub, TRUE );

	Add( _T("Experimental.EnableDIPPSupport"), &Experimental.EnableDIPPSupport, FALSE );

	Add( _T("WINE.MenuFix"), &WINE.MenuFix, FALSE );
}



//////////////////////////////////////////////////////////////////////
// CSettings construction

CSettings::CSettings()
{
	TCHAR szPath[MAX_PATH];
	CRegistry pRegistry;
	GetModuleFileName( NULL, szPath, MAX_PATH );

	// Set default program and user paths
	General.Path = szPath;
	if ( General.Path.ReverseFind( '\\' ) >= 0 )
		General.Path = General.Path.Left( General.Path.ReverseFind( '\\' ) );
	General.Path = pRegistry.GetString( _T(""), _T("Path"), General.Path );	// This line is needed otherwise the value is readed too late.
	General.UserPath = General.Path;

	// Reset 'live' values.
	Live.DiskSpaceWarning			= FALSE;
	Live.DiskWriteWarning			= FALSE;
	Live.AdultWarning				= FALSE;
	Live.QueueLimitWarning			= FALSE;
	Live.DefaultED2KServersLoaded	= FALSE;
	Live.DonkeyServerWarning		= FALSE;
	Live.UploadLimitWarning			= FALSE;
	Live.DiskSpaceStop				= FALSE;
	Live.BandwidthScale				= 100;
	Live.LoadWindowState			= FALSE;
	Live.AutoClose					= FALSE;
	Live.FirstRun					= FALSE;
	Live.LastDuplicateHash			= L"";
	Live.NewFile					= FALSE;
	Live.MaliciousWarning			= FALSE;

	// Add all settings
	Setup();
	if ( Settings.Live.FirstRun ) OnChangeConnectionSpeed();	// This helps if the QuickStart Wizard is skipped.
}

CSettings::~CSettings()
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		delete m_pItems.GetNext( pos );
	}
}

//////////////////////////////////////////////////////////////////////
// CSettings add items

void CSettings::Add(LPCTSTR pszName, DWORD* pDword, DWORD nDefault)
{
	m_pItems.AddTail( new Item( pszName, pDword, NULL, NULL, NULL ) );
	*pDword = nDefault;
}

void CSettings::Add(LPCTSTR pszName, int* pDword, DWORD nDefault)
{
	m_pItems.AddTail( new Item( pszName, (DWORD*)pDword, NULL, NULL, NULL ) );
	*pDword = nDefault;
}

void CSettings::Add(LPCTSTR pszName, DOUBLE* pFloat, DOUBLE nDefault)
{
	m_pItems.AddTail( new Item( pszName, NULL, pFloat, NULL, NULL ) );
	*pFloat = nDefault;
}

void CSettings::Add(LPCTSTR pszName, CString* pString, LPCTSTR pszDefault)
{
	m_pItems.AddTail( new Item( pszName, NULL, NULL, pString, NULL ) );
	if ( pszDefault ) *pString = pszDefault;
}

void CSettings::Add(LPCTSTR pszName, string_set* pSet, LPCTSTR pszDefault)
{
	m_pItems.AddTail( new Item( pszName, NULL, NULL, NULL, pSet ) );
	if ( pszDefault )
	{
		LoadSet( pSet, pszDefault );
	}
}

void CSettings::LoadSet(string_set* pSet, LPCTSTR pszString)
{
	pSet->clear();
	for( LPCTSTR start = pszString; *start; start++ )
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

//////////////////////////////////////////////////////////////////////
// CSettings load

#define SMART_VERSION	51

void CSettings::Load()
{
	CRegistry pRegistry;

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		Item* pItem = m_pItems.GetNext( pos );
		pItem->Load();
		ASSERT( ( pItem->m_pDword && !pItem->m_pFloat && !pItem->m_pString && !pItem->m_pSet ) \
			|| ( !pItem->m_pDword && pItem->m_pFloat && !pItem->m_pString && !pItem->m_pSet ) \
			|| ( !pItem->m_pDword && !pItem->m_pFloat && pItem->m_pString && !pItem->m_pSet ) \
			|| ( !pItem->m_pDword && !pItem->m_pFloat && !pItem->m_pString && pItem->m_pSet ) );
	}

	if ( pRegistry.GetInt( _T("Settings"), _T("FirstRun"), TRUE ) )
	{
		Live.FirstRun = TRUE;
		pRegistry.SetInt( _T("Settings"), _T("FirstRun"), FALSE );
	}

	SmartUpgrade();

	if ( pRegistry.GetInt( _T("Settings"), _T("Running"), FALSE ) )
	{
		//pRegistry.SetInt( _T("VersionCheck"), _T("NextCheck"), 0 );
	}
	else
	{
		pRegistry.SetInt( _T("Settings"), _T("Running"), TRUE );
	}

	// Set current networks
	Gnutella1.EnableToday		= Gnutella1.EnableAlways;
	Gnutella2.EnableToday		= Gnutella2.EnableAlways;
	eDonkey.EnableToday			= eDonkey.EnableAlways;

	// Make sure some needed paths exist
	CreateDirectory( General.Path + _T("\\Data"), NULL );
	CreateDirectory( General.UserPath, NULL );
	CreateDirectory( General.UserPath + _T("\\Data"), NULL );
	CreateDirectory( Downloads.IncompletePath, NULL );

	// Set interface
	Interface.LowResMode		= ! ( GetSystemMetrics( SM_CYSCREEN ) > 600 );
	if ( Live.FirstRun ) Search.AdvancedPanel = ! Interface.LowResMode;

	// Reset certain network variables if bandwidth is too low
	// Set ed2k and G1
	if ( GetOutgoingBandwidth() < 2 )
	{
		eDonkey.EnableToday		= FALSE;
		eDonkey.EnableAlways	= FALSE;
		Gnutella1.EnableToday	= FALSE;
		Gnutella1.EnableAlways	= FALSE;
	}
	// Set number of torrents
	BitTorrent.DownloadTorrents = min( BitTorrent.DownloadTorrents, (int)( ( GetOutgoingBandwidth() / 2 ) + 2 ) );

	// Enforce a few sensible values to avoid being banned/dropped/etc (in case of registry fiddling)
	Downloads.SearchPeriod		= min( Downloads.SearchPeriod, 4*60*1000u );
	Downloads.StarveTimeout		= max( Downloads.StarveTimeout, 45*60u );
	Downloads.ConnectThrottle	= max( Downloads.ConnectThrottle, Connection.ConnectThrottle + 50 );
	Downloads.MaxFiles			= min( Downloads.MaxFiles, 100 );
	eDonkey.QueryGlobalThrottle = max( eDonkey.QueryGlobalThrottle, 1000u );
	Gnutella1.PingRate			= max( min( Gnutella1.PingRate, 180000u ), 15000u );
	Gnutella1.RequeryDelay		= max( min( Gnutella1.RequeryDelay, 60u ), 5u );
	Gnutella2.RequeryDelay		= max( Gnutella2.RequeryDelay, 60*60u );
	Gnutella1.SearchTTL			= max( min( Gnutella1.SearchTTL, 3u ), 1u );
	Gnutella1.DefaultTTL		= max( min( Gnutella1.DefaultTTL, 3u ), 1u );
	Gnutella1.QueryThrottle		= max( min( Gnutella1.QueryThrottle, 60u ), 5u );
	Gnutella.MaxResults			= max( min( Gnutella.MaxResults, 300u ), 1u );
	Gnutella.MaxHits			= max( min( Gnutella.MaxHits, 4096u ), 0u );

	// Set client links
	Gnutella1.NumHubs			= max( min( Gnutella1.NumHubs,  5    ), 1 );
	Gnutella1.NumLeafs			= max( min( Gnutella1.NumLeafs, 1024 ), 5 );
	Gnutella1.NumPeers			= max( min( Gnutella1.NumPeers, 64   ), 15 );
	Gnutella2.NumHubs			= max( min( Gnutella2.NumHubs,  3    ), 1 );
	Gnutella2.NumLeafs			= max( min( Gnutella2.NumLeafs, 1024 ), 50 );
	Gnutella2.NumPeers			= max( min( Gnutella2.NumPeers, 64   ), 4 );

	// Enforce HostCache Expire time to be minimum 1 day.
	Gnutella1.HostExpire		= max( Gnutella1.HostExpire, 24 * 60 * 60u );
	Gnutella2.HostExpire		= max( Gnutella2.HostExpire, 24 * 60 * 60u );

	// Get TorrentCreatorPath from HKEY_LOCAL_MACHINE if that value in HKEY_CURRENT_USER is empty or too short.
	if ( BitTorrent.TorrentCreatorPath.GetLength() <= 5 )
		BitTorrent.TorrentCreatorPath = pRegistry.GetString( _T("BitTorrent"), _T("TorrentCreatorPath"), _T(""), HKEY_LOCAL_MACHINE );
	
	// Make sure download/incomplete folders aren't the same
	if ( _tcsicmp( Downloads.IncompletePath, Downloads.CompletePath ) == 0 )
	{
		CString strMessage;
		LoadString( strMessage, IDS_SETTINGS_FILEPATH_NOT_SAME );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		// Downloads.IncompletePath = General.Path + _T("\\Incomplete");
	}

	//Temporary- until G1 ultrapeer has been updated
	Gnutella1.ClientMode		= MODE_LEAF;

	// UPnP is not supported in servers and Win9x
	if ( theApp.m_bServer || theApp.m_dwWindowsVersion < 5 && !theApp.m_bWinME )
	{
		Connection.EnableUPnP = FALSE;
		Connection.DeleteUPnPPorts = FALSE;
	}

	// UPnP will setup a random port, so we need to reset values after it sets Connection.InPort
	if ( Connection.RandomPort )
		Connection.InPort = 0;
	else if ( Connection.InPort == 0 )
		Connection.RandomPort = TRUE;

	if ( !theApp.m_bNT )
	{
		Connection.EnableFirewallException = FALSE;
		Connection.DeleteFirewallException = FALSE;
	}
}

void CSettings::Save(BOOL bShutdown)
{
	CRegistry pRegistry;

	if ( Connection.TimeoutConnect == 0 ) return;

	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		Item* pItem = m_pItems.GetNext( pos );
		if ( pItem->m_sName != _T(".Path") ) pItem->Save();
	}

	pRegistry.SetInt( _T("Settings"), _T("SmartVersion"), SMART_VERSION );
	pRegistry.SetInt( _T("Settings"), _T("Running"), bShutdown ? FALSE : TRUE );
}

//////////////////////////////////////////////////////////////////////
// CSettings smart upgrade

void CSettings::SmartUpgrade()
{	//This function resets certain values when upgrading, depending on version.
	CRegistry pRegistry;
	int nVersion = pRegistry.GetInt( _T("Settings"), _T("SmartVersion"), SMART_VERSION );

/*	// Set next update check
	if ( nVersion < SMART_VERSION )
	{
		// Don't check for a week if we've just upgraded
		CTimeSpan tPeriod( 7, 0, 0, 0 );
		CTime tNextCheck = CTime::GetCurrentTime() + tPeriod;
		theApp.WriteProfileInt( _T("VersionCheck"), _T("NextCheck"), (DWORD)tNextCheck.GetTime() );
	}
*/
	// Add OGG handling if needed
	if ( ( nVersion < SMART_VERSION || Live.FirstRun ) &&
		_tcsistr( MediaPlayer.FileTypes, _T("|ogg|") ) == NULL )
	{
		LONG nReg = 0;

		if ( RegQueryValue( HKEY_CLASSES_ROOT,
			_T("CLSID\\{02391F44-2767-4E6A-A484-9B47B506F3A4}"), NULL, &nReg )
			== ERROR_SUCCESS && nReg > 0 )
		{
			MediaPlayer.FileTypes += _T("|ogg|");
		}
	}

	if ( nVersion != SMART_VERSION )
		Uploads.SharePartials = TRUE;

	// 'SmartUpgrade' settings updates- change any settings that were mis-set in previous versions
	if ( nVersion < 20 )
	{
		Gnutella2.UdpOutResend			= 6000;
		Gnutella2.UdpOutExpire			= 26000;
		Library.TigerHeight		= 9;

		Downloads.AutoExpand			= FALSE;

		Uploads.MaxPerHost				= 2;
		Uploads.ShareTiger				= TRUE;

		Library.PrivateTypes.erase( _T("nfo") );
		Library.SafeExecute.Replace( _T("|."), _T("|") );
	}

	if ( nVersion < 21 )
	{
		Library.ThumbSize				= 96;
		Library.SourceExpire			= 86400;

		Gnutella1.TranslateTTL			= 2;
	}

	if ( nVersion < 24 )
	{
		General.CloseMode				= 0;

		Connection.TimeoutConnect		= 16000;
		Connection.TimeoutHandshake		= 45000;

		Downloads.RetryDelay			= 10*60000;

		Uploads.FilterMask				= 0xFFFFFFFD;
	}

	if ( nVersion < 25 )
	{
		Connection.TimeoutTraffic		= 140000;

		Gnutella2.NumPeers				= 6;
	}

	if ( nVersion < 28 )
	{
		BitTorrent.Endgame		= TRUE;		// Endgame on
	}

	if ( nVersion < 29 )
	{
		Downloads.MinSources	= 1;		// Lower Max value- should reset it in case
		Downloads.StarveTimeout = 2700;		// Increased due to ed2k queues (Tripping too often)

		Gnutella2.RequeryDelay	= 4*3600;	// Longer delay between sending same search to G2 hub
	}

	if ( nVersion < 30 )
	{
		BitTorrent.RequestSize	= 16384;	// Other BT clients have changed this value (undocumented)
	}

	if ( nVersion < 31 )
	{
		Downloads.SearchPeriod			= 120000;

		Gnutella1.MaximumTTL			= 10;

		Gnutella2.QueryGlobalThrottle	= 125;

		Uploads.QueuePollMin	= 45000;	// Lower values for re-ask times- a dynamic multiplier
		Uploads.QueuePollMax	= 120000;	//  Is now applied based on Q# (from 1x to 5x)
		eDonkey.PacketThrottle	= 500;		// Second throttle added for finer control
	}

	if ( nVersion < 32 )
	{
		Library.BitziWebView	= _T("http://bitzi.com/lookup/(URN)?v=detail&ref=shareaza");
		Library.BitziWebSubmit	= _T("http://bitzi.com/lookup/(SHA1).(TTH)?fl=(SIZE)&ff=(FIRST20)&fn=(NAME)&tag.ed2k.ed2khash=(ED2K)&(INFO)&a=(AGENT)&v=Q0.4&ref=shareaza");
		Library.BitziXML		= _T("http://ticket.bitzi.com/rdf/(SHA1).(TTH)");

		theApp.WriteProfileString( _T("Interface"), _T("SchemaColumns.audio"), _T("(EMPTY)") );
	}

	if ( nVersion < 33 )
	{
		RegDeleteKey( HKEY_CURRENT_USER, _T("Software\\Shareaza\\Shareaza\\Plugins\\LibraryBuilder") );
	}

	if ( nVersion < 34 )
		BitTorrent.LinkPing				= 120 * 1000;

	if ( nVersion < 35 )
	{
		Gnutella1.QuerySearchUTF8 = TRUE;
		Gnutella1.QueryHitUTF8 = TRUE;
	}

	if ( nVersion < 36 )
	{
		//Library.VirtualFiles	= TRUE;		// Virtual files (stripping) on
		Library.VirtualFiles = FALSE;
	}

	if ( nVersion < 37 )
	{
		Downloads.RequestHash = TRUE;
		Gnutella.SpecifyProtocol = TRUE;
		Search.FilterMask = Search.FilterMask | 0x140; // Turn on DRM and Suspicious filters
	}

	if ( nVersion < 39 )
	{
		General.RatesInBytes = TRUE;
		General.VerboseMode = FALSE;
	}

	if ( nVersion < 40 )
	{
		eDonkey.ForceHighID = TRUE;
		eDonkey.FastConnect = FALSE;
	}

	if ( nVersion < 41 )
	{
		eDonkey.ExtendedRequest = 2;
		Community.ChatAllNetworks = TRUE;
		Community.ChatFilter = TRUE;
	}

	if ( nVersion < 42 )
	{
		Gnutella2.NumHubs = 2;
		General.ItWasLimited = TRUE;
		OnChangeConnectionSpeed();
	}

	if ( nVersion < 43 )
	{	
		eDonkey.MetAutoQuery = TRUE;
	}

	if ( nVersion < 44 )
	{	
		BitTorrent.AutoSeed = TRUE;
	}

	if ( nVersion < 45 )
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

	if ( nVersion < 46 )
	{
		// ReGet
		if ( ! IsIn( Library.PrivateTypes, _T("reget") ) )
			Library.PrivateTypes.insert( _T("reget") );
	}

	if ( nVersion < 47 )
	{
		// Changed from minutes to seconds
		Gnutella1.QueryThrottle = 30u;
		Gnutella1.RequeryDelay = 30u;

		Gnutella.MaxResults = 150;
	}

	if ( nVersion < 49 )
	{
		eDonkey.SendPortServer = FALSE;
	}

	if ( nVersion < 50 )
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

	if ( nVersion < 51 )
	{
		Library.HashWindow = TRUE;
		Gnutella1.PingRate = 30000u;
	}
}

void CSettings::OnChangeConnectionSpeed()
{
	BOOL bLimited = theApp.m_bLimitedConnections && !General.IgnoreXPsp2;

	if ( Connection.InSpeed > 750 )
	{
		Gnutella2.NumPeers = max( Gnutella2.NumPeers, 4 );
	}

	if ( Connection.InSpeed <= 80 )
	{	// NT Modem users / Win9x Modem users
		Downloads.MaxFiles				= 8;
		Downloads.MaxTransfers			= 24;
		Downloads.MaxFileTransfers		= 4;
		Downloads.MaxConnectingSources	= 16;
		Downloads.MaxFileSearches		= 0;
		Downloads.SourcesWanted			= 200;	// Don't bother requesting so many sources
		Search.GeneralThrottle			= 300;	// Slow searches a little so we don't get flooded

		Gnutella2.NumLeafs				= 200;
		BitTorrent.DownloadTorrents		= 1;	// Best not to try too many torrents
	}
	else if ( !theApp.m_bNT )
	{	// Others Win9x users
		Downloads.MaxFiles				= 8;
		Downloads.MaxTransfers			= 24;
		Downloads.MaxFileTransfers		= 6;
		Downloads.MaxConnectingSources	= 16;
		Downloads.MaxFileSearches		= 1;
		Downloads.SourcesWanted			= 200;	// Don't bother requesting so many sources
		Search.GeneralThrottle			= 250;	// Slow searches a little so we don't get flooded

		Gnutella2.NumLeafs				= 200;
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

		Gnutella2.NumLeafs				= 300;
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
		BitTorrent.DownloadTorrents		= 3;
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

		Gnutella2.NumLeafs				= 400;	//Can probably support more leaves
		BitTorrent.DownloadTorrents		= 4;	// Should be able to handle several torrents
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

		Gnutella2.NumLeafs				= 450;	//Can probably support more leaves
		BitTorrent.DownloadTorrents		= 4;	// Should be able to handle several torrents
	}

	if( bLimited )
	{	// Window XP Service Pack 2
		theApp.Message( MSG_ERROR, _T("Warning - Windows XP Service Pack 2 detected. Performance may be reduced.") );
		Connection.ConnectThrottle		= max( Connection.ConnectThrottle, 250u );
		Downloads.ConnectThrottle		= max( Downloads.ConnectThrottle, 800u );
		Gnutella.ConnectFactor			= min( Gnutella.ConnectFactor, 3u );
		Connection.SlowConnect			= TRUE;
		Connection.RequireForTransfers	= TRUE;
		Downloads.MaxConnectingSources	= 8;
		Gnutella1.EnableAlways			= FALSE;
		Gnutella1.EnableToday			= FALSE;

		General.ItWasLimited			= TRUE;
	}
	else if( General.ItWasLimited )	// We change the settings back if the user path the half-open connection limit
	{
		Connection.ConnectThrottle		= 0;
		Downloads.ConnectThrottle		= 250;
		Gnutella.ConnectFactor			= 4;
		Connection.SlowConnect			= FALSE;

		General.ItWasLimited			= FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CSettings settings lookup

CSettings::Item* CSettings::GetSetting(LPCTSTR pszName) const
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		Item* pItem = m_pItems.GetNext( pos );
		if ( pItem->m_sName.CompareNoCase( pszName ) == 0 ) return pItem;
	}

	return NULL;
}

CSettings::Item* CSettings::GetSetting(LPVOID pValue) const
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		Item* pItem = m_pItems.GetNext( pos );
		if ( pItem->m_pDword == pValue ||
			 pItem->m_pFloat == pValue ||
			 pItem->m_pString == pValue ||
			 pItem->m_pSet == pValue ) return pItem;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CSettings window position persistance

BOOL CSettings::LoadWindow(LPCTSTR pszName, CWnd* pWindow)
{
	CRegistry pRegistry;
	CString strEntry;

	if ( pszName != NULL )
		strEntry = pszName;
	else
		strEntry = pWindow->GetRuntimeClass()->m_lpszClassName;

	int nShowCmd = pRegistry.GetInt( _T("Windows"), strEntry + _T(".ShowCmd"), -1 );
	if ( nShowCmd == -1 ) return FALSE;

	WINDOWPLACEMENT pPos = {};
	pPos.length = sizeof(pPos);

	pPos.rcNormalPosition.left		= pRegistry.GetInt( _T("Windows"), strEntry + _T(".Left"), 0 );
	pPos.rcNormalPosition.top		= pRegistry.GetInt( _T("Windows"), strEntry + _T(".Top"), 0 );
	pPos.rcNormalPosition.right		= pRegistry.GetInt( _T("Windows"), strEntry + _T(".Right"), 0 );
	pPos.rcNormalPosition.bottom	= pRegistry.GetInt( _T("Windows"), strEntry + _T(".Bottom"), 0 );

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
	CRegistry pRegistry;
	CString strEntry;

	if ( pszName != NULL )
		strEntry = pszName;
	else
		strEntry = pWindow->GetRuntimeClass()->m_lpszClassName;

	pWindow->GetWindowPlacement( &pPos );

	pRegistry.SetInt(  _T("Windows"), strEntry + _T(".ShowCmd"), pPos.showCmd );

	if ( pPos.showCmd != SW_SHOWNORMAL ) return;

	pRegistry.SetInt(  _T("Windows"), strEntry + _T(".Left"), pPos.rcNormalPosition.left );
	pRegistry.SetInt(  _T("Windows"), strEntry + _T(".Top"), pPos.rcNormalPosition.top );
	pRegistry.SetInt(  _T("Windows"), strEntry + _T(".Right"), pPos.rcNormalPosition.right );
	pRegistry.SetInt(  _T("Windows"), strEntry + _T(".Bottom"), pPos.rcNormalPosition.bottom );

}

//////////////////////////////////////////////////////////////////////
// CSettings list header persistance

BOOL CSettings::LoadList(LPCTSTR pszName, CListCtrl* pCtrl, int nSort)
{
	CRegistry pRegistry;
	LV_COLUMN pColumn;

	pColumn.mask = LVCF_FMT;
    int nColumns = 0;
	for ( ; pCtrl->GetColumn( nColumns, &pColumn ) ; nColumns++ );

	CString strOrdering, strWidths, strItem;
	BOOL bSuccess = FALSE;

	strItem.Format( _T("%s.Ordering"), pszName );
	strOrdering = pRegistry.GetString( _T("ListStates"), strItem, _T("") );
	strItem.Format( _T("%s.Widths"), pszName );
	strWidths = pRegistry.GetString( _T("ListStates"), strItem, _T("") );
	strItem.Format( _T("%s.Sort"), pszName );
	nSort = pRegistry.GetInt( _T("ListStates"), strItem, nSort );

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
	CRegistry pRegistry;
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
	pRegistry.SetString( _T("ListStates"), strItem, strOrdering);
	strItem.Format( _T("%s.Widths"), pszName );
	pRegistry.SetString( _T("ListStates"), strItem, strWidths);
	strItem.Format( _T("%s.Sort"), pszName );
	pRegistry.SetInt( _T("ListStates"), strItem, nSort );
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

	bStartup = ( RegQueryValueEx( hKey, _T("Shareaza"), NULL, NULL, NULL, NULL ) == ERROR_SUCCESS );

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
		TCHAR szPath[128];

		GetModuleFileName( NULL, szPath, 128 );
		strCommand.Format( _T("\"%s\" -tray"), szPath );

		RegSetValueEx( hKey, _T("Shareaza"), 0, REG_SZ, (const BYTE*)(LPCTSTR)strCommand,
			( strCommand.GetLength() + 1 ) * sizeof(TCHAR) );
	}
	else
	{
		RegDeleteValue( hKey, _T("Shareaza") );
	}

	RegCloseKey( hKey );
}

//////////////////////////////////////////////////////////////////////
// CSettings speed
//
//	Returns a nicely formatted string displaying a given transfer speed

CString CSettings::SmartSpeed(QWORD nVolume, int nVolumeUnits, bool bTruncate) const
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
		strVolume.Format( _T("%I64i %s"), nVolume, strUnit );
		break;

	// Kilobits - KiloBytes
	case 2:
		strVolume.Format( _T("%.2lf K%s"), nVolume / 1024.0f, strUnit );
		break;

	// Megabits - MegaBytes
	case 3:
		strVolume.Format( _T("%.2lf M%s"), nVolume / pow( 1024.0f, 2 ), strUnit );
		break;

	default:
		TRACE( _T("Unknown RatesUnit - %i"), General.RatesUnit );
		break;
	}
	return theApp.m_bRTL ? _T("\x200E") + strVolume : strVolume;
}

//////////////////////////////////////////////////////////////////////
// CSettings volume
//
//	Returns a nicely formatted string displaying a given volume

CString CSettings::SmartVolume(QWORD nVolume, int nVolumeUnits, bool bTruncate) const
{
	CString strUnit( _T("B") );
	CString strVolume;
	CString strTruncate( _T("%.0f") );

	if ( !General.RatesInBytes && nVolumeUnits == bits )
		strUnit = _T("b");

	switch ( nVolumeUnits )
	{
	// nVolume is in bits - Bytes
	case bits:
	case Bytes:
		if ( nVolume < 1024 )						// bits - Bytes
		{
			strVolume.Format( _T("%I64i %s"), nVolume, strUnit );
			break;
		}
		else if ( nVolume < 10 * 1024 )				// 10 Kilobits - KiloBytes
		{
			if ( !bTruncate )
				strTruncate = _T("%.2f");
			strVolume.Format( strTruncate + _T(" K%s"), nVolume / 1024.0f, strUnit );
			break;
		}

		// Convert to KiloBytes and drop through to next case
		nVolume /= 1024;

	// nVolume is in Kilobits - Kilobytes
	case Kilobits:
	case KiloBytes:
		if ( nVolume < 1024 )						// Kilo
			strVolume.Format( _T("%I64i K%s"), nVolume, strUnit );
		else if ( nVolume < pow( 1024.0f, 2 ) )		// Mega
		{
			if ( !bTruncate )
				strTruncate = _T("%.2f");
			strVolume.Format( strTruncate + _T(" M%s"), nVolume / 1024.0f, strUnit );
		}
		else
		{
			if ( !bTruncate )
				strTruncate = _T("%.2f");
			if ( nVolume < pow( 1024.0f, 3 ) )		// Giga
				strVolume.Format( strTruncate + _T(" G%s"), nVolume / pow( 1024.0f, 2 ), strUnit );
			else if ( nVolume < pow( 1024.0f, 4 ) )	// Tera
				strVolume.Format( strTruncate + _T(" T%s"), nVolume / pow( 1024.0f, 3 ), strUnit );
			else if ( nVolume < pow( 1024.0f, 5 ) )	// Peta
				strVolume.Format( strTruncate + _T(" P%s"), nVolume / pow( 1024.0f, 4 ), strUnit );
			else									// Exa
				strVolume.Format( strTruncate + _T(" E%s"), nVolume / pow( 1024.0f, 5 ), strUnit );
		}
	}

	return theApp.m_bRTL ? _T("\x200E") + strVolume : strVolume;
}

QWORD CSettings::ParseVolume(CString& strVolume, int nReturnUnits) const
{
	double val = 0;
	CString strSize( strVolume );

	if ( strSize.Left( 1 ) == _T("\x200E") ) strSize = strSize.Mid( 1 );

	// Return early if there is no number in the string
	if ( _stscanf( strSize, _T("%lf"), &val ) != 1 ) return 0ul;

	// Return early if the number is negative
	if ( val < 0 ) return 0ul;

	if ( _tcsstr( strSize, _T("B") ) )
		// Convert to bits if Bytes were passed in
		val *= 8.0f;

	// Work out what units are represented in the string
	if ( _tcsstr( strSize, _T("K") ) || _tcsstr( strSize, _T("k") ) )		// Kilo
		val *= 1024.0f;
	else if ( _tcsstr( strSize, _T("M") ) || _tcsstr( strSize, _T("m") ) )	// Mega
		val *= pow( 1024.0f, 2 );
	else if ( _tcsstr( strSize, _T("G") ) || _tcsstr( strSize, _T("g") ) )	// Giga
		val *= pow( 1024.0f, 3 );
	else if ( _tcsstr( strSize, _T("T") ) || _tcsstr( strSize, _T("t") ) )	// Tera
		val *= pow( 1024.0f, 4 );
	else if ( _tcsstr( strSize, _T("P") ) || _tcsstr( strSize, _T("p") ) )	// Peta
		val *= pow( 1024.0f, 5 );
	else if ( _tcsstr( strSize, _T("E") ) || _tcsstr( strSize, _T("e") ) )	// Exa
		val *= pow( 1024.0f, 6 );

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

	return ( min( ( Settings.Connection.OutSpeed / 8 ), ( Settings.Bandwidth.Uploads / 1024 ) ) );
}

//////////////////////////////////////////////////////////////////////
// CSettings::Item construction and operations

CSettings::Item::Item(LPCTSTR pszName, DWORD* pDword, DOUBLE* pFloat, CString* pString, string_set* pSet) :
	m_sName		( pszName ),
	m_pDword	( pDword ),
	m_pFloat	( pFloat ),
	m_pString	( pString ),
	m_pSet		( pSet )
{
}

void CSettings::Item::Load()
{
	CRegistry pRegistry;

	int nPos = m_sName.Find( '.' );
	if ( nPos < 0 ) return;

	if ( m_pDword )
	{
		*m_pDword = pRegistry.GetDword( m_sName.Left( nPos ), m_sName.Mid( nPos + 1 ), *m_pDword );
	}
	else if ( m_pFloat )
	{
		*m_pFloat = pRegistry.GetFloat( m_sName.Left( nPos ), m_sName.Mid( nPos + 1 ), *m_pFloat );
	}
	else if ( m_pString )
	{
		*m_pString = pRegistry.GetString( m_sName.Left( nPos ), m_sName.Mid( nPos + 1 ), *m_pString );
	}
	else if ( m_pSet )
	{
		CString foo( pRegistry.GetString( m_sName.Left( nPos ), m_sName.Mid( nPos + 1 ), _T("") ) );
		if ( foo.GetLength() )
		{
			LoadSet( m_pSet, foo );
		}
	}
}

void CSettings::Item::Save()
{
	CRegistry pRegistry;

	int nPos = m_sName.Find( '.' );
	if ( nPos < 0 ) return;

	if ( m_pDword )
	{
		pRegistry.SetInt( m_sName.Left( nPos ), m_sName.Mid( nPos + 1 ), *m_pDword );
	}
	else if ( m_pFloat )
	{
		CString str;
		str.Format( _T("%e"), *m_pFloat );
		pRegistry.SetString( m_sName.Left( nPos ), m_sName.Mid( nPos + 1 ), str );
	}
	else if ( m_pString )
	{
		pRegistry.SetString( m_sName.Left( nPos ), m_sName.Mid( nPos + 1 ), *m_pString );
	}
	else if ( m_pSet )
	{
		CString foo( _T("|") );
		for( string_set::const_iterator i = m_pSet->begin(); i != m_pSet->end(); i++ )
		{
			foo += *i;
			foo += _T('|');
		}
		pRegistry.SetString( m_sName.Left( nPos ), m_sName.Mid( nPos + 1 ), foo );
	}
}
