//
// Settings.cpp
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
#include "Schema.h"
#include "Skin.h"

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
	Add( _T(".Debug"), &General.Debug, FALSE );
	Add( _T(".DebugLog"), &General.DebugLog, FALSE );
	Add( _T(".UpdateCheck"), &General.UpdateCheck, TRUE );
	Add( _T("Settings.GUIMode"), &General.GUIMode, GUI_BASIC );
	Add( _T("Settings.CloseMode"), &General.CloseMode, 0 );
	Add( _T("Settings.TrayMinimise"), &General.TrayMinimise, FALSE );
	Add( _T("Settings.VerboseMode"), &General.VerboseMode, TRUE );
	Add( _T("Settings.ShowTimestamp"), &General.ShowTimestamp, TRUE );
	Add( _T("Settings.SizeLists"), &General.SizeLists, FALSE );
	Add( _T("Settings.HashIntegrity"), &General.HashIntegrity, TRUE );
	Add( _T("Settings.RatesInBytes"), &General.RatesInBytes, FALSE );
	Add( _T("Settings.RatesUnit"), &General.RatesUnit, 0 );
	Add( _T("Settings.AlwaysOpenURLs"), &General.AlwaysOpenURLs, FALSE );
	Add( _T("Settings.UserAgent"), &General.UserAgent, _T(".") );
	Add( _T("Settings.Language"), &General.Language, _T("en") );
	
	Add( _T("Interface.TipDelay"), &Interface.TipDelay, 600 );
	Add( _T("Interface.TipAlpha"), &Interface.TipAlpha, 230 );
	Add( _T("Interface.TipSearch"), &Interface.TipSearch, TRUE );
	Add( _T("Interface.TipLibrary"), &Interface.TipLibrary, TRUE );
	Add( _T("Interface.TipDownloads"), &Interface.TipDownloads, TRUE );
	Add( _T("Interface.TipUploads"), &Interface.TipUploads, TRUE );
	Add( _T("Interface.TipNeighbours"), &Interface.TipNeighbours, TRUE );
	Add( _T("Interface.TipMedia"), &Interface.TipMedia, TRUE );
	
	Add( _T("Library.WatchFolders"), &Library.WatchFolders, TRUE );
	Add( _T("Library.PartialMatch"), &Library.PartialMatch, TRUE );
	Add( _T("Library.VirtualFiles"), &Library.VirtualFiles, TRUE );
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
	Add( _T("Library.SafeExecute"), &Library.SafeExecute, _T("|ace|ape|asf|avi|bmp|gif|iso|jpg|jpeg|mid|mov|m1v|m2v|m3u|mp2|mp3|mpa|mpe|mpg|mpeg|ogg|pdf|png|qt|rar|rm|sks|tar|tgz|torrent|txt|wav|wma|wmv|zip|") );
	Add( _T("Library.PrivateTypes"), &Library.PrivateTypes, _T("|vbs|js|dat|partial|getright|pif|lnk|sd|") );
	Add( _T("Library.ThumbSize"), &Library.ThumbSize, 96 );
	Add( _T("Library.BitziAgent"), &Library.BitziAgent, _T(".") );
	Add( _T("Library.BitziWebView"), &Library.BitziWebView, _T("http://bitzi.com/lookup/(SHA1)?detail&ref=shareaza") );
	Add( _T("Library.BitziWebSubmit"), &Library.BitziWebSubmit, _T("http://bitzi.com/lookup/(SHA1).(TTH)?fl=(SIZE)&ff=(FIRST20)&fn=(NAME)&a=(AGENT)&v=Q0.4&ref=shareaza") );
	Add( _T("Library.BitziXML"), &Library.BitziXML, _T("http://ticket.bitzi.com/rdf/(SHA1)") );
	Add( _T("Library.BitziOkay"), &Library.BitziOkay, FALSE );
	
	Add( _T("Search.LastSchemaURI"), &Search.LastSchemaURI, _T("") );
	Add( _T("Search.BlankSchemaURI"), &Search.BlankSchemaURI, CSchema::uriAudio );
	Add( _T("Search.SearchPanel"), &Search.SearchPanel, TRUE );
	Add( _T("Search.ExpandMatches"), &Search.ExpandMatches, FALSE );
	Add( _T("Search.HighlightNew"), &Search.HighlightNew, TRUE );
	Add( _T("Search.SwitchToTransfers"), &Search.SwitchToTransfers, TRUE );
	Add( _T("Search.SchemaTypes"), &Search.SchemaTypes, TRUE );
	Add( _T("Search.ShowNames"), &Search.ShowNames, TRUE );
	Add( _T("Search.FilterMask"), &Search.FilterMask, 0x28 );
	Add( _T("Search.MonitorSchemaURI"), &Search.MonitorSchemaURI, CSchema::uriAudio );
	Add( _T("Search.MonitorFilter"), &Search.MonitorFilter, NULL );
	Add( _T("Search.MonitorQueue"), &Search.MonitorQueue, 128 );
	Add( _T("Search.BrowseTreeSize"), &Search.BrowseTreeSize, 180 );
	Add( _T("Search.DetailPanelVisible"), &Search.DetailPanelVisible, TRUE );
	Add( _T("Search.DetailPanelSize"), &Search.DetailPanelSize, 100 );
	Add( _T("Search.MaxPreviewLength"), &Search.MaxPreviewLength, 20*1024 );

	Add( _T("MediaPlayer.EnablePlay"), &MediaPlayer.EnablePlay, TRUE );
	Add( _T("MediaPlayer.EnableEnqueue"), &MediaPlayer.EnableEnqueue, TRUE );
	Add( _T("MediaPlayer.FileTypes"), &MediaPlayer.FileTypes, _T("|asx|wax|m3u|wvx|wmx|asf|wav|snd|au|aif|aifc|aiff|wma|mp3|cda|mid|rmi|midi|avi|asf|mpeg|mpg|m1v|mp2|mpa|mpe|wmv|") );
	Add( _T("MediaPlayer.Repeat"), &MediaPlayer.Repeat, FALSE );
	Add( _T("MediaPlayer.Random"), &MediaPlayer.Random, FALSE );
	Add( _T("MediaPlayer.Zoom"), (DWORD*)&MediaPlayer.Zoom, smzFill );
	Add( _T("MediaPlayer.Aspect"), &MediaPlayer.Aspect, smaDefault );
	Add( _T("MediaPlayer.Volume"), &MediaPlayer.Volume, 1.0f );
	Add( _T("MediaPlayer.ListVisible"), &MediaPlayer.ListVisible, TRUE );
	Add( _T("MediaPlayer.ListSize"), &MediaPlayer.ListSize, 240 );
	Add( _T("MediaPlayer.StatusVisible"), &MediaPlayer.StatusVisible, TRUE );
	Add( _T("MediaPlayer.VisCLSID"), &MediaPlayer.VisCLSID, _T("{591A5CFF-3172-4020-A067-238542DDE9C2}") );
	Add( _T("MediaPlayer.VisPath"), &MediaPlayer.VisPath, _T("") );
	Add( _T("MediaPlayer.VisSize"), &MediaPlayer.VisSize, 1 );
	
	Add( _T("Web.Magnet"), &Web.Magnet, TRUE );
	Add( _T("Web.Gnutella"), &Web.Gnutella, TRUE );
	Add( _T("Web.ED2K"), &Web.ED2K, TRUE );
	Add( _T("Web.Piolet"), &Web.Piolet, TRUE );
	Add( _T("Web.Torrent"), &Web.Torrent, TRUE );
	
	Add( _T("Connection.AutoConnect"), &Connection.AutoConnect, FALSE );
	Add( _T("Connection.Firewalled"), &Connection.Firewalled, TRUE );
	Add( _T("Connection.OutHost"), &Connection.OutHost, NULL );
	Add( _T("Connection.InHost"), &Connection.InHost, NULL );
	Add( _T("Connection.InPort"), &Connection.InPort, GNUTELLA_DEFAULT_PORT );
	Add( _T("Connection.InBind"), &Connection.InBind, FALSE );
	Add( _T("Connection.InSpeed"), &Connection.InSpeed, 56 );
	Add( _T("Connection.OutSpeed"), &Connection.OutSpeed, 56 );
	Add( _T("Connection.IgnoreLocalIP"), &Connection.IgnoreLocalIP, TRUE );
	Add( _T("Connection.TimeoutConnect"), &Connection.TimeoutConnect, 16000 );
	Add( _T("Connection.TimeoutHandshake"), &Connection.TimeoutHandshake, 45000 );
	Add( _T("Connection.TimeoutTraffic"), &Connection.TimeoutTraffic, 140000 );
	Add( _T("Connection.SendBuffer"), &Connection.SendBuffer, 2048 );
	Add( _T("Connection.AsyncIO"), &Connection.AsyncIO, TRUE );
	Add( _T("Connection.RequireForTransfers"), &Connection.RequireForTransfers, TRUE );
	
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
	Add( _T("Community.Timestamp"), &Community.Timestamp, TRUE );
	Add( _T("Community.ServeProfile"), &Community.ServeProfile, TRUE );
	Add( _T("Community.ServeFiles"), &Community.ServeFiles, TRUE );
	
	Add( _T("Discovery.AccessThrottle"), &Discovery.AccessThrottle, 3600 );
	Add( _T("Discovery.Lowpoint"), &Discovery.Lowpoint, 10 );
	Add( _T("Discovery.FailureLimit"), &Discovery.FailureLimit, 2 );
	Add( _T("Discovery.UpdatePeriod"), &Discovery.UpdatePeriod, 1800 );
	Add( _T("Discovery.DefaultUpdate"), &Discovery.DefaultUpdate, 3600 );
	Add( _T("Discovery.BootstrapCount"), &Discovery.BootstrapCount, 10 );
	Add( _T("Discovery.G2DAddress"), &Discovery.G2DAddress, _T("stats.shareaza.com:6446") );
	Add( _T("Discovery.G2DRetryAfter"), &Discovery.G2DRetryAfter, 0 );
	
	Add( _T("Gnutella.HubEnable"), &Gnutella.HubEnable, TRUE );
	Add( _T("Gnutella.HubForce"), &Gnutella.HubForce, FALSE );
	Add( _T("Gnutella.LeafEnable"), &Gnutella.LeafEnable, TRUE );
	Add( _T("Gnutella.LeafForce"), &Gnutella.LeafForce, FALSE );
	Add( _T("Gnutella.ConnectFactor"), &Gnutella.ConnectFactor, 5 );
	Add( _T("Gnutella.DeflateHub2Hub"), &Gnutella.DeflateHub2Hub, TRUE );
	Add( _T("Gnutella.DeflateLeaf2Hub"), &Gnutella.DeflateLeaf2Hub, FALSE );
	Add( _T("Gnutella.DeflateHub2Leaf"), &Gnutella.DeflateHub2Leaf, TRUE );
	Add( _T("Gnutella.MaxResults"), &Gnutella.MaxResults, 300 );
	Add( _T("Gnutella.MaxHits"), &Gnutella.MaxHits, 64 );
	Add( _T("Gnutella.HitsPerPacket"), &Gnutella.HitsPerPacket, 64 );
	Add( _T("Gnutella.RouteCache"), &Gnutella.RouteCache, 600 );
	Add( _T("Gnutella.HostCacheCount"), &Gnutella.HostCacheSize, 1024 );
	Add( _T("Gnutella.HostCacheExpire"), &Gnutella.HostCacheExpire, 10 * 60 );
	Add( _T("Gnutella.HostCacheView"), &Gnutella.HostCacheView, PROTOCOL_ED2K );
	Add( _T("Gnutella.ConnectThrottle"), &Gnutella.ConnectThrottle, 120 );
	
	Add( _T("Gnutella1.EnableAlways"), &Gnutella1.EnableAlways, FALSE );
	Add( _T("Gnutella1.Handshake04"), &Gnutella1.Handshake04, TRUE );
	Add( _T("Gnutella1.Handshake06"), &Gnutella1.Handshake06, TRUE );
	Add( _T("Gnutella1.NumHubs"), &Gnutella1.NumHubs, 2 );
	Add( _T("Gnutella1.NumLeafs"), &Gnutella1.NumLeafs, 0 );
	Add( _T("Gnutella1.NumPeers"), &Gnutella1.NumPeers, 0 );
	Add( _T("Gnutella1.PacketBufferSize"), &Gnutella1.PacketBufferSize, 64 );
	Add( _T("Gnutella1.PacketBufferTime"), &Gnutella1.PacketBufferTime, 60000 );
	Add( _T("Gnutella1.DefaultTTL"), &Gnutella1.DefaultTTL, 5 );
	Add( _T("Gnutella1.SearchTTL"), &Gnutella1.SearchTTL, 4 );
	Add( _T("Gnutella1.TranslateTTL"), &Gnutella1.TranslateTTL, 2 );
	Add( _T("Gnutella1.MaximumTTL"), &Gnutella1.MaximumTTL, 11 );
	Add( _T("Gnutella1.MaximumPacket"), &Gnutella1.MaximumPacket, 65535 );
	Add( _T("Gnutella1.MaximumQuery"), &Gnutella1.MaximumQuery, 256 );
	Add( _T("Gnutella1.StrictPackets"), &Gnutella1.StrictPackets, FALSE );
	Add( _T("Gnutella1.EnableGGEP"), &Gnutella1.EnableGGEP, TRUE );
	Add( _T("Gnutella1.VendorMsg"), &Gnutella1.VendorMsg, TRUE );
	Add( _T("Gnutella1.QueryThrottle"), &Gnutella1.QueryThrottle, 20*60 );
	Add( _T("Gnutella1.RequeryDelay"), &Gnutella1.RequeryDelay, 45*60 );
	Add( _T("Gnutella1.PingFlood"), &Gnutella1.PingFlood, 3000 );
	Add( _T("Gnutella1.PingRate"), &Gnutella1.PingRate, 15000 );
	Add( _T("Gnutella1.PongCache"), &Gnutella1.PongCache, 10000 );
	Add( _T("Gnutella1.PongCount"), &Gnutella1.PongCount, 10 );
	
	Add( _T("Gnutella2.EnableAlways"), &Gnutella2.EnableAlways, TRUE );
	Add( _T("Gnutella2.NumHubs"), &Gnutella2.NumHubs, 2 );
	Add( _T("Gnutella2.NumLeafs"), &Gnutella2.NumLeafs, 300 );
	Add( _T("Gnutella2.NumPeers"), &Gnutella2.NumPeers, 6 );
	Add( _T("Gnutella2.UdpMTU"), &Gnutella2.UdpMTU, 500 );
	Add( _T("Gnutella2.UdpBuffers"), &Gnutella2.UdpBuffers, 512 );
	Add( _T("Gnutella2.UdpInFrames"), &Gnutella2.UdpInFrames, 256 );
	Add( _T("Gnutella2.UdpOutFrames"), &Gnutella2.UdpOutFrames, 256 );
	Add( _T("Gnutella2.UdpGlobalThrottle"), &Gnutella2.UdpGlobalThrottle, 1 );
	Add( _T("Gnutella2.UdpOutExpire"), &Gnutella2.UdpOutExpire, 26000 );
	Add( _T("Gnutella2.UdpOutResend"), &Gnutella2.UdpOutResend, 6000 );
	Add( _T("Gnutella2.UdpInExpire"), &Gnutella2.UdpInExpire, 30000 );
	Add( _T("Gnutella2.KHLPeriod"), &Gnutella2.KHLPeriod, 60000 );
	Add( _T("Gnutella2.KHLHubCount"), &Gnutella2.KHLHubCount, 50 );
	Add( _T("Gnutella2.HAWPeriod"), &Gnutella2.HAWPeriod, 300000 );
	Add( _T("Gnutella2.QueryGlobalThrottle"), &Gnutella2.QueryGlobalThrottle, 120 );
	Add( _T("Gnutella2.QueryHostThrottle"), &Gnutella2.QueryHostThrottle, 120 );
	Add( _T("Gnutella2.QueryHostDeadline"), &Gnutella2.QueryHostDeadline, 10*60 );
	Add( _T("Gnutella2.RequeryDelay"), &Gnutella2.RequeryDelay, 2*60*60 );
	Add( _T("Gnutella2.HubHorizonSize"), &Gnutella2.HubHorizonSize, 128 );
	
	Add( _T("eDonkey.EnableAlways"), &eDonkey.EnableAlways, FALSE );
	Add( _T("eDonkey.NumServers"), &eDonkey.NumServers, 1 );
	Add( _T("eDonkey.MaxLinks"), &eDonkey.MaxLinks, 128 );
	Add( _T("eDonkey.MaxResults"), &eDonkey.MaxResults, 100 );
	Add( _T("eDonkey.MaxShareCount"), &eDonkey.MaxShareCount, 1000 );
	Add( _T("eDonkey.ServerWalk"), &eDonkey.ServerWalk, TRUE );
	Add( _T("eDonkey.QueryGlobalThrottle"), &eDonkey.QueryGlobalThrottle, 1000 );
	Add( _T("eDonkey.QueryServerThrottle"), &eDonkey.QueryServerThrottle, 60 );
	//Add( _T("eDonkey.RequeryDelay"), &eDonkey.RequeryDelay, 45*60 );
	Add( _T("eDonkey.LearnNewServers"), &eDonkey.LearnNewServers, TRUE );
	Add( _T("eDonkey.RequestPipe"), &eDonkey.RequestPipe, 3 );
	Add( _T("eDonkey.RequestSize"), &eDonkey.RequestSize, 180*1024/2 );
	Add( _T("eDonkey.FrameSize"), &eDonkey.FrameSize, 10240 );
	Add( _T("eDonkey.ReAskTime"), &eDonkey.ReAskTime, 1300 );
	Add( _T("eDonkey.DequeueTime"), &eDonkey.DequeueTime, 3610 );
	Add( _T("eDonkey.TagNames"), &eDonkey.TagNames, TRUE );
	Add( _T("eDonkey.ExtendedRequest"), &eDonkey.ExtendedRequest, TRUE );
	Add( _T("eDonkey.ServerListURL"), &eDonkey.ServerListURL, _T("http://ocbmaurice.dyndns.org/pl/slist.pl/server.met?download/server-good.met") );
	
	Add( _T("BitTorrent.DefaultTrackerPeriod"), &BitTorrent.DefaultTrackerPeriod, 5*60000 );
	Add( _T("BitTorrent.LinkTimeout"), &BitTorrent.LinkTimeout, 180000 );
	Add( _T("BitTorrent.LinkPing"), &BitTorrent.LinkPing, 45000 );
	Add( _T("BitTorrent.RequestPipe"), &BitTorrent.RequestPipe, 4 );
	Add( _T("BitTorrent.RequestSize"), &BitTorrent.RequestSize, 32768 );
	Add( _T("BitTorrent.RequestLimit"), &BitTorrent.RequestLimit, 131072 );
	Add( _T("BitTorrent.RandomPeriod"), &BitTorrent.RandomPeriod, 30000 );
	Add( _T("BitTorrent.SourceExchangePeriod"), &BitTorrent.SourceExchangePeriod, 10 );
	Add( _T("BitTorrent.UploadCount"), &BitTorrent.UploadCount, 4 );
	Add( _T("BitTorrent.Endgame"), &BitTorrent.Endgame, TRUE );
	
	Add( _T("Downloads.IncompletePath"), &Downloads.IncompletePath, General.Path + _T("\\Incomplete") );
	Add( _T("Downloads.CompletePath"), &Downloads.CompletePath, General.Path + _T("\\Downloads") );
	Add( _T("Downloads.BufferSize"), &Downloads.BufferSize, 81920 );
	Add( _T("Downloads.SparseThreshold"), &Downloads.SparseThreshold, 8 * 1024 );
	Add( _T("Downloads.MaxFiles"), &Downloads.MaxFiles, 32 );
	Add( _T("Downloads.MaxTransfers"), &Downloads.MaxTransfers, 128 );
	Add( _T("Downloads.MaxFileTransfers"), &Downloads.MaxFileTransfers, 8 );
	Add( _T("Downloads.MinSources"), &Downloads.MinSources, 1 );
	Add( _T("Downloads.ConnectThrottle"), &Downloads.ConnectThrottle, 200 );
	Add( _T("Downloads.QueueLimit"), &Downloads.QueueLimit, 0 );
	Add( _T("Downloads.SearchPeriod"), &Downloads.SearchPeriod, 120000 );
	Add( _T("Downloads.StarveTimeout"), &Downloads.StarveTimeout, 2700 );
	Add( _T("Downloads.RetryDelay"), &Downloads.RetryDelay, 10*60000 );
	Add( _T("Downloads.PushTimeout"), &Downloads.PushTimeout, 45000 );
	Add( _T("Downloads.StaggardStart"), &Downloads.StaggardStart, FALSE );
	Add( _T("Downloads.AllowBackwards"), &Downloads.AllowBackwards, TRUE );
	Add( _T("Downloads.ChunkSize"), &Downloads.ChunkSize, 512*1024 );
	Add( _T("Downloads.ChunkStrap"), &Downloads.ChunkStrap, 64*1024 );
	Add( _T("Downloads.Metadata"), &Downloads.Metadata, TRUE );
	Add( _T("Downloads.VerifyFiles"), &Downloads.VerifyFiles, TRUE );
	Add( _T("Downloads.VerifyTiger"), &Downloads.VerifyTiger, TRUE );
	Add( _T("Downloads.VerifyED2K"), &Downloads.VerifyED2K, TRUE );
	Add( _T("Downloads.NeverDrop"), &Downloads.NeverDrop, FALSE );
	Add( _T("Downloads.RequestHash"), &Downloads.RequestHash, FALSE );
	Add( _T("Downloads.RequestHTTP11"), &Downloads.RequestHTTP11, TRUE );
	Add( _T("Downloads.RequestURLENC"), &Downloads.RequestURLENC, TRUE );
	Add( _T("Downloads.SaveInterval"), &Downloads.SaveInterval, 60000 );
	Add( _T("Downloads.FlushSD"), &Downloads.FlushSD, TRUE );
	Add( _T("Downloads.ShowSources"), &Downloads.ShowSources, FALSE );
	Add( _T("Downloads.ShowPercent"), &Downloads.ShowPercent, FALSE );
	Add( _T("Downloads.ShowGroups"), &Downloads.ShowGroups, TRUE );
	Add( _T("Downloads.AutoExpand"), &Downloads.AutoExpand, FALSE );
	Add( _T("Downloads.AutoClear"), &Downloads.AutoClear, FALSE );
	Add( _T("Downloads.ClearDelay"), &Downloads.ClearDelay, 30000 );
	Add( _T("Downloads.FilterMask"), &Downloads.FilterMask, 0xFFFFFFFF );
	Add( _T("Downloads.ShowMonitorURLs"), &Downloads.ShowMonitorURLs, TRUE );
	
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
	
	Add( _T("Remote.Enable"), &Remote.Enable, FALSE );
	Add( _T("Remote.Username"), &Remote.Username, _T("") );
	Add( _T("Remote.Password"), &Remote.Password, _T("") );
}

//////////////////////////////////////////////////////////////////////
// CSettings construction

CSettings::CSettings()
{
	TCHAR szPath[128];
	GetModuleFileName( NULL, szPath, 128 );
	
	General.Path = szPath;
	if ( General.Path.ReverseFind( '\\' ) >= 0 )
		General.Path = General.Path.Left( General.Path.ReverseFind( '\\' ) );
	
	Live.BandwidthScale		= 90;
	Live.LoadWindowState	= FALSE;
	Live.AutoClose			= FALSE;
	Live.FirstRun			= FALSE;
	
	Setup();
}

CSettings::~CSettings()
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		delete (Item*)m_pItems.GetNext( pos );
	}
}

//////////////////////////////////////////////////////////////////////
// CSettings add items

void CSettings::Add(LPCTSTR pszName, DWORD* pDword, DWORD nDefault)
{
	m_pItems.AddTail( new Item( pszName, pDword, NULL, NULL ) );
	*pDword = nDefault;
}

void CSettings::Add(LPCTSTR pszName, int* pDword, DWORD nDefault)
{
	m_pItems.AddTail( new Item( pszName, (DWORD*)pDword, NULL, NULL ) );
	*pDword = nDefault;
}

void CSettings::Add(LPCTSTR pszName, DOUBLE* pFloat, DOUBLE nDefault)
{
	m_pItems.AddTail( new Item( pszName, NULL, pFloat, NULL ) );
	*pFloat = nDefault;
}

void CSettings::Add(LPCTSTR pszName, CString* pString, LPCTSTR pszDefault)
{
	m_pItems.AddTail( new Item( pszName, NULL, NULL, pString ) );
	if ( pszDefault ) *pString = pszDefault;
}

//////////////////////////////////////////////////////////////////////
// CSettings load

#define SMART_VERSION	28

void CSettings::Load()
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		Item* pItem = (Item*)m_pItems.GetNext( pos );
		pItem->Load();
	}
	
	if ( theApp.GetProfileInt( _T("Settings"), _T("FirstRun"), TRUE ) )
	{
		Live.FirstRun = TRUE;
		theApp.WriteProfileInt( _T("Settings"), _T("FirstRun"), FALSE );
	}
	
	SmartUpgrade();
	
	if ( theApp.GetProfileInt( _T("Settings"), _T("Running"), FALSE ) )
	{
		theApp.WriteProfileInt( _T("VersionCheck"), _T("NextCheck"), 0 );
	}
	else
	{
		theApp.WriteProfileInt( _T("Settings"), _T("Running"), TRUE );
	}
	
	Gnutella1.EnableToday	= Gnutella1.EnableAlways;
	Gnutella2.EnableToday	= Gnutella2.EnableAlways;
	eDonkey.EnableToday		= eDonkey.EnableAlways;
	
	CreateDirectory( General.Path + _T("\\Data"), NULL );
}

void CSettings::Save(BOOL bShutdown)
{
	if ( Connection.TimeoutConnect == 0 ) return;
	
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		Item* pItem = (Item*)m_pItems.GetNext( pos );
		if ( pItem->m_sName != _T(".Path") ) pItem->Save();
	}
	
	theApp.WriteProfileInt( _T("Settings"), _T("SmartVersion"), SMART_VERSION );
	theApp.WriteProfileInt( _T("Settings"), _T("Running"), bShutdown ? FALSE : TRUE );
}

//////////////////////////////////////////////////////////////////////
// CSettings smart upgrade

void CSettings::SmartUpgrade()
{
	int nVersion = theApp.GetProfileInt( _T("Settings"), _T("SmartVersion"), SMART_VERSION );
	
	if ( nVersion < 20 )
	{
		Gnutella.HubForce	= FALSE;
		
		Gnutella1.TranslateTTL			= 3;
		Gnutella1.RequeryDelay			= 45*60;
		
		Gnutella2.NumPeers				= 4;
		Gnutella2.QueryGlobalThrottle	= 200;
		Gnutella2.UdpOutResend			= 6000;
		Gnutella2.UdpOutExpire			= 26000;
		
		Library.TigerHeight		= 9;
		Library.SourceExpire	= 86400;
		Library.BitziWebSubmit	= _T("http://bitzi.com/lookup/(SHA1).(TTH)?fl=(SIZE)&ff=(FIRST20)&fn=(NAME)&a=(AGENT)&v=Q0.4&ref=shareaza");
		
		Downloads.MaxFiles				= max( Downloads.MaxFiles, 8 );
		Downloads.MaxTransfers			= max( Downloads.MaxTransfers, 32 );
		Downloads.MaxFileTransfers		= max( Downloads.MaxFileTransfers, 14 );
		Downloads.AutoExpand			= FALSE;
		
		Uploads.SharePartials	= TRUE;
		Uploads.MaxPerHost		= 2;
		Uploads.ShareTiger		= TRUE;
		
		Replace( Library.PrivateTypes, _T("|nfo|"), _T("|") );
		Replace( Library.SafeExecute, _T("|."), _T("|") );
	}
	
	if ( nVersion < 21 )
	{
		General.CloseMode		= 0;
		Library.ThumbSize		= 96;
		Library.SourceExpire	= 86400;
		Gnutella1.TranslateTTL	= 2;
	}
	
	if ( nVersion < 24 )
	{
		General.CloseMode			= 0;
		Connection.TimeoutHandshake	= max( Connection.TimeoutHandshake, 40000 );
		Connection.TimeoutConnect	= 16000;
		Connection.TimeoutHandshake	= 45000;
		Connection.TimeoutTraffic	= 140000;
		Downloads.RetryDelay		= 10*60000;
		Uploads.FilterMask			= 0xFFFFFFFD;
	}
	
	if ( nVersion < 25 )
	{
		Connection.TimeoutTraffic = 140000;
		Gnutella2.NumHubs = 2;
		Gnutella2.NumLeafs = 300;
		Gnutella2.NumPeers = 6;
	}
	
	if ( nVersion < 26 )
	{
		Downloads.ConnectThrottle = max( Downloads.ConnectThrottle, 200 );
		Downloads.MaxTransfers = min( Downloads.MaxTransfers, 128 );
	}
	
	if ( nVersion < 28 )
	{
		Connection.Firewalled = TRUE;	// We now assume so until proven otherwise
		Library.VirtualFiles = TRUE;	// Virtual files (stripping) on
		BitTorrent.Endgame = TRUE;		// Endgame on
	}
	
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
}

//////////////////////////////////////////////////////////////////////
// CSettings settings lookup

CSettings::Item* CSettings::GetSetting(LPCTSTR pszName) const
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		Item* pItem = (Item*)m_pItems.GetNext( pos );
		if ( pItem->m_sName.CompareNoCase( pszName ) == 0 ) return pItem;
	}
	
	return NULL;
}

CSettings::Item* CSettings::GetSetting(LPVOID pValue) const
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		Item* pItem = (Item*)m_pItems.GetNext( pos );
		if ( pItem->m_pDword == pValue ||
			 pItem->m_pFloat == pValue ||
			 pItem->m_pString == pValue ) return pItem;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CSettings window position persistance

BOOL CSettings::LoadWindow(LPCTSTR pszName, CWnd* pWindow)
{
	WINDOWPLACEMENT pPos;
	CString strEntry;
	
	if ( pszName != NULL )
		strEntry = pszName;
	else
		strEntry = pWindow->GetRuntimeClass()->m_lpszClassName;
	
	int nShowCmd = theApp.GetProfileInt( _T("Windows"), strEntry + _T(".ShowCmd"), -1 );
	if ( nShowCmd == -1 ) return FALSE;
	
	ZeroMemory( &pPos, sizeof(pPos) );
	pPos.length = sizeof(pPos);
	
	pPos.rcNormalPosition.left		= theApp.GetProfileInt( _T("Windows"), strEntry + _T(".Left"), 0 );
	pPos.rcNormalPosition.top		= theApp.GetProfileInt( _T("Windows"), strEntry + _T(".Top"), 0 );
	pPos.rcNormalPosition.right		= theApp.GetProfileInt( _T("Windows"), strEntry + _T(".Right"), 0 );
	pPos.rcNormalPosition.bottom	= theApp.GetProfileInt( _T("Windows"), strEntry + _T(".Bottom"), 0 );
	
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

	theApp.WriteProfileInt( _T("Windows"), strEntry + _T(".ShowCmd"), pPos.showCmd );

	if ( pPos.showCmd != SW_SHOWNORMAL ) return;

	theApp.WriteProfileInt( _T("Windows"), strEntry + _T(".Left"), pPos.rcNormalPosition.left );
	theApp.WriteProfileInt( _T("Windows"), strEntry + _T(".Top"), pPos.rcNormalPosition.top );
	theApp.WriteProfileInt( _T("Windows"), strEntry + _T(".Right"), pPos.rcNormalPosition.right );
	theApp.WriteProfileInt( _T("Windows"), strEntry + _T(".Bottom"), pPos.rcNormalPosition.bottom );
}

//////////////////////////////////////////////////////////////////////
// CSettings list header persistance

BOOL CSettings::LoadList(LPCTSTR pszName, CListCtrl* pCtrl, int nSort)
{
	LV_COLUMN pColumn;
	pColumn.mask = LVCF_FMT;
	for ( int nColumns = 0 ; pCtrl->GetColumn( nColumns, &pColumn ) ; nColumns++ );

	CString strOrdering, strWidths, strItem;
	BOOL bSuccess = FALSE;

	strItem.Format( _T("%s.Ordering"), pszName );
	strOrdering = theApp.GetProfileString( _T("ListStates"), strItem, _T("") );
	strItem.Format( _T("%s.Widths"), pszName );
	strWidths = theApp.GetProfileString( _T("ListStates"), strItem, _T("") );
	strItem.Format( _T("%s.Sort"), pszName );
	nSort = theApp.GetProfileInt( _T("ListStates"), strItem, nSort );

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

	SetWindowLong( pCtrl->GetSafeHwnd(), GWL_USERDATA, nSort );

	CHeaderCtrl* pHeader = (CHeaderCtrl*)pCtrl->GetWindow( GW_CHILD );
	if ( pHeader ) Skin.Translate( pszName, pHeader );

	return bSuccess;
}

void CSettings::SaveList(LPCTSTR pszName, CListCtrl* pCtrl)
{
	LV_COLUMN pColumn;
	pColumn.mask = LVCF_FMT;
	for ( int nColumns = 0 ; pCtrl->GetColumn( nColumns, &pColumn ) ; nColumns++ );
	
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
	
	int nSort = GetWindowLong( pCtrl->GetSafeHwnd(), GWL_USERDATA );
	
	strItem.Format( _T("%s.Ordering"), pszName );
	theApp.WriteProfileString( _T("ListStates"), strItem, strOrdering );
	strItem.Format( _T("%s.Widths"), pszName );
	theApp.WriteProfileString( _T("ListStates"), strItem, strWidths );
	strItem.Format( _T("%s.Sort"), pszName );
	theApp.WriteProfileInt( _T("ListStates"), strItem, nSort );
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
// CSettings configurable user agent

CString CSettings::SmartAgent(LPCTSTR pszAgent)
{
	CString strAgent;

	if ( pszAgent && *pszAgent )
	{
		if ( _tcscmp( pszAgent, _T(".") ) == 0 )
			strAgent = _T("Shareaza ") + theApp.m_sVersion;
		else
			strAgent = pszAgent;
	}				

	return strAgent;
}

//////////////////////////////////////////////////////////////////////
// CSettings volume

CString CSettings::SmartVolume(QWORD nVolume, BOOL bKB, BOOL bRateInBits)
{
	LPCTSTR pszUnit = _T("B");
	CString strVolume;
	
	if ( bRateInBits )
	{
		if ( General.RatesInBytes )
		{
			nVolume /= 8;
			pszUnit = _T("B/s");
		}
		else
		{
			pszUnit = _T("b/s");
		}
		
		if ( General.RatesUnit > 0 )
		{
			if ( bKB ) nVolume *= 1024;
			
			switch ( General.RatesUnit )
			{
			case 1:
				strVolume.Format( _T("%I64i %s"), nVolume, pszUnit );
				return strVolume;
			case 2:
				strVolume.Format( _T("%.2lf K%s"), (double)nVolume / 1024, pszUnit );
				return strVolume;
			case 3:
				strVolume.Format( _T("%.2lf M%s"), (double)nVolume / (1024*1024), pszUnit );
				return strVolume;
			}
			
			if ( bKB ) nVolume /= 1024;
		}
	}
	
	if ( ! bKB )
	{
		if ( nVolume < 1024 )
		{
			strVolume.Format( _T("%I64i %s"), nVolume, pszUnit );
			return strVolume;
		}
		
		nVolume /= 1024;
	}
	
	if ( nVolume < 1024 )
	{
		strVolume.Format( _T("%I64i K%s"), nVolume, pszUnit );
	}
	else if ( nVolume < 1024*1024 )
	{
		strVolume.Format( _T("%.2lf M%s"), (double)nVolume / 1024, pszUnit );
	}
	else if ( nVolume < 1024*1024*1024 )
	{
		strVolume.Format( _T("%.3lf G%s"), (double)nVolume / (1024*1024), pszUnit );
	}
	else
	{
		strVolume.Format( _T("%.3lf T%s"), (double)nVolume / (1024*1024*1024), pszUnit );
	}
	
	return strVolume;
}

QWORD CSettings::ParseVolume(LPCTSTR psz, BOOL bSpeedInBits)
{
	double val = 0;
	
	if ( _stscanf( psz, _T("%lf"), &val ) != 1 ) return 0;
	
	if ( _tcsstr( psz, _T(" K") ) ) val *= 1024;
	if ( _tcsstr( psz, _T(" k") ) ) val *= 1024;
	
	if ( _tcsstr( psz, _T(" M") ) ) val *= 1024*1024;
	if ( _tcsstr( psz, _T(" m") ) ) val *= 1024*1024;
	
	if ( _tcsstr( psz, _T(" G") ) ) val *= 1024*1024*1024;
	if ( _tcsstr( psz, _T(" g") ) ) val *= 1024*1024*1024;
	
	if ( _tcsstr( psz, _T(" T") ) ) val *= 1099511627776.0f;
	if ( _tcsstr( psz, _T(" t") ) ) val *= 1099511627776.0f;
	
	if ( bSpeedInBits )
	{
		if ( _tcschr( psz, 'b' ) )
			return (QWORD)val;
		else if ( _tcschr( psz, 'B' ) )
			return (QWORD)( val * 8 );
		else
			return 0;
	}
	else
	{
		return (QWORD)val;
	}
}

//////////////////////////////////////////////////////////////////////
// CSettings::Item construction and operations

CSettings::Item::Item(LPCTSTR pszName, DWORD* pDword, DOUBLE* pFloat, CString* pString)
{
	m_sName		= pszName;
	m_pDword	= pDword;
	m_pFloat	= pFloat;
	m_pString	= pString;
}

void CSettings::Item::Load()
{
	int nPos = m_sName.Find( '.' );
	if ( nPos < 0 ) return;

	if ( m_pDword )
	{
		*m_pDword = theApp.GetProfileInt( m_sName.Left( nPos ), m_sName.Mid( nPos + 1 ), *m_pDword );
	}
	else if ( m_pFloat )
	{
		CString str = theApp.GetProfileString( m_sName.Left( nPos ), m_sName.Mid( nPos + 1 ), _T("") );
		if ( str.GetLength() ) _stscanf( str, _T("%lf"), m_pFloat );
	}
	else
	{
		*m_pString = theApp.GetProfileString( m_sName.Left( nPos ), m_sName.Mid( nPos + 1 ), *m_pString );
	}
}

void CSettings::Item::Save()
{
	int nPos = m_sName.Find( '.' );
	if ( nPos < 0 ) return;

	if ( m_pDword )
	{
		theApp.WriteProfileInt( m_sName.Left( nPos ), m_sName.Mid( nPos + 1 ), *m_pDword );
	}
	else if ( m_pFloat )
	{
		CString str;
		str.Format( _T("%e"), *m_pFloat );
		theApp.WriteProfileString( m_sName.Left( nPos ), m_sName.Mid( nPos + 1 ), str );
	}
	else
	{
		theApp.WriteProfileString( m_sName.Left( nPos ), m_sName.Mid( nPos + 1 ), *m_pString );
	}
}
