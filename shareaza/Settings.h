//
// Settings.h
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

#pragma once

class CSettingsItem;


class CSettings  
{
// Construction
public:
	CSettings();
	virtual ~CSettings();
	
// Attributes
public:
	struct sGeneral
	{
		CString		Path;
		BOOL		Debug;
		BOOL		DebugLog;
		INT			GUIMode;
		BOOL		CloseMode;
		BOOL		TrayMinimise;
		BOOL		VerboseMode;
		BOOL		ShowTimestamp;
		BOOL		SizeLists;
		BOOL		HashIntegrity;
		BOOL		RatesInBytes;
		DWORD		RatesUnit;
		BOOL		AlwaysOpenURLs;
		CString		UserAgent;
		CString		Language;
	} General;

	struct sInterface
	{
		DWORD		TipDelay;
		DWORD		TipAlpha;
		BOOL		TipSearch;
		BOOL		TipLibrary;
		BOOL		TipDownloads;
		BOOL		TipUploads;
		BOOL		TipNeighbours;
		BOOL		TipMedia;
	} Interface;
	
	struct sLibrary
	{
		BOOL		WatchFolders;
		BOOL		PartialMatch;
		BOOL		VirtualFiles;
		BOOL		SourceMesh;
		DWORD		SourceExpire;
		DWORD		TigerHeight;
		DWORD		QueryRouteSize;
		DWORD		HistoryTotal;
		DWORD		HistoryDays;

		BOOL		ShowVirtual;
		DWORD		TreeSize;
		DWORD		PanelSize;
		BOOL		ShowPanel;
		BOOL		StoreViews;
		BOOL		ShowCoverArt;
		CString		SchemaURI;
		CString		FilterURI;
		CString		SafeExecute;
		CString		PrivateTypes;
		DWORD		ThumbSize;
		CString		BitziAgent;
		CString		BitziWebView;
		CString		BitziWebSubmit;
		CString		BitziXML;
		BOOL		BitziOkay;
	} Library;

	struct sSearch
	{
		CString		LastSchemaURI;
		CString		BlankSchemaURI;
		BOOL		SearchPanel;
		BOOL		ExpandMatches;
		BOOL		HighlightNew;
		BOOL		SwitchToTransfers;
		BOOL		SchemaTypes;
		BOOL		ShowNames;
		DWORD		FilterMask;
		CString		MonitorSchemaURI;
		CString		MonitorFilter;
		DWORD		MonitorQueue;
		DWORD		BrowseTreeSize;
		BOOL		DetailPanelVisible;
		DWORD		DetailPanelSize;
		DWORD		MaxPreviewLength;
	} Search;
	
	struct sMediaPlayer
	{
		BOOL		EnablePlay;
		BOOL		EnableEnqueue;
		CString		FileTypes;
		BOOL		Repeat;
		BOOL		Random;
		MediaZoom	Zoom;
		double		Aspect;
		double		Volume;
		BOOL		ListVisible;
		DWORD		ListSize;
		BOOL		StatusVisible;
		CString		VisCLSID;
		CString		VisPath;
		INT			VisSize;
	} MediaPlayer;
	
	struct sWeb
	{
		BOOL		Magnet;
		BOOL		Gnutella;
		BOOL		ED2K;
		BOOL		Piolet;
		BOOL		Torrent;
	} Web;
	
	struct sConnection
	{
		BOOL		AutoConnect;
		BOOL		Firewalled;
		CString		OutHost;
		CString		InHost;
		DWORD		InPort;
		BOOL		InBind;
		DWORD		InSpeed;
		DWORD		OutSpeed;
		BOOL		IgnoreLocalIP;
		DWORD		TimeoutConnect;
		DWORD		TimeoutHandshake;
		DWORD		TimeoutTraffic;
		DWORD		SendBuffer;
		BOOL		RequireForTransfers;
		BOOL		AsyncIO;
	} Connection;

	struct sBandwidth
	{
		DWORD		Request;
		DWORD		HubIn;
		DWORD		HubOut;
		DWORD		LeafIn;
		DWORD		LeafOut;
		DWORD		PeerIn;
		DWORD		PeerOut;
		DWORD		UdpOut;
		DWORD		Downloads;
		DWORD		Uploads;
		DWORD		HubUploads;
	} Bandwidth;

	struct sCommunity
	{
		BOOL		ChatEnable;
		BOOL		Timestamp;
		BOOL		ServeProfile;
		BOOL		ServeFiles;
	} Community;

	struct sDiscovery
	{
		DWORD		AccessThrottle;
		DWORD		Lowpoint;
		DWORD		FailureLimit;
		DWORD		UpdatePeriod;
		DWORD		DefaultUpdate;
		DWORD		BootstrapCount;
		CString		G2DAddress;
		DWORD		G2DRetryAfter;
	} Discovery;
	
	struct sGnutella
	{
		BOOL		HubEnable;
		BOOL		HubForce;
		BOOL		LeafEnable;
		BOOL		LeafForce;
		DWORD		ConnectFactor;
		BOOL		DeflateHub2Hub;
		BOOL		DeflateLeaf2Hub;
		BOOL		DeflateHub2Leaf;
		DWORD		MaxResults;
		DWORD		MaxHits;
		DWORD		HitsPerPacket;
		DWORD		RouteCache;
		DWORD		HostCacheSize;
		DWORD		HostCacheExpire;
		DWORD		HostCacheView;
		DWORD		ConnectThrottle;
	} Gnutella;
	
	struct sGnutella1
	{
		BOOL		EnableToday;
		BOOL		EnableAlways;
		BOOL		Handshake04;
		BOOL		Handshake06;
		int			NumHubs;
		int			NumLeafs;
		int			NumPeers;
		DWORD		PacketBufferSize;
		DWORD		PacketBufferTime;
		DWORD		DefaultTTL;
		DWORD		SearchTTL;
		DWORD		TranslateTTL;
		DWORD		MaximumTTL;
		DWORD		MaximumPacket;
		DWORD		MaximumQuery;
		BOOL		StrictPackets;
		BOOL		EnableGGEP;
		BOOL		VendorMsg;
		DWORD		QueryThrottle;
		DWORD		RequeryDelay;
		DWORD		PingFlood;
		DWORD		PingRate;
		DWORD		PongCache;
		int			PongCount;
	} Gnutella1;

	struct sGnutella2
	{
		BOOL		EnableToday;
		BOOL		EnableAlways;
		int			NumHubs;
		int			NumLeafs;
		int			NumPeers;
		DWORD		UdpMTU;
		DWORD		UdpBuffers;
		DWORD		UdpInFrames;
		DWORD		UdpOutFrames;
		DWORD		UdpGlobalThrottle;
		DWORD		UdpOutExpire;
		DWORD		UdpOutResend;
		DWORD		UdpInExpire;
		DWORD		KHLPeriod;
		DWORD		KHLHubCount;
		DWORD		HAWPeriod;
		DWORD		QueryGlobalThrottle;
		DWORD		QueryHostThrottle;
		DWORD		QueryHostDeadline;
		DWORD		RequeryDelay;
		DWORD		HubHorizonSize;
	} Gnutella2;
	
	struct seDonkey
	{
		BOOL		EnableToday;
		BOOL		EnableAlways;
		DWORD		NumServers;
		int			MaxLinks;
		int			MaxResults;
		int			MaxShareCount;
		BOOL		ServerWalk;
		DWORD		QueryGlobalThrottle;
		DWORD		QueryServerThrottle;
		DWORD		RequeryDelay;
		BOOL		LearnNewServers;
		CString		ServerListURL;
		DWORD		RequestPipe;
		DWORD		RequestSize;
		DWORD		FrameSize;
		DWORD		ReAskTime;
		DWORD		DequeueTime;
		BOOL		ExtendedRequest;
		BOOL		TagNames;
	} eDonkey;
	
	struct sBitTorrent
	{
		DWORD		DefaultTrackerPeriod;
		DWORD		LinkTimeout;
		DWORD		LinkPing;
		DWORD		RequestPipe;
		DWORD		RequestSize;
		DWORD		RequestLimit;
		DWORD		RandomPeriod;
		DWORD		SourceExchangePeriod;
		int			UploadCount;
		BOOL		Endgame;
	} BitTorrent;

	struct sDownloads
	{
		CString		IncompletePath;
		CString		CompletePath;
		DWORD		BufferSize;
		DWORD		SparseThreshold;
		INT			MaxFiles;
		INT			MaxTransfers;
		INT			MaxFileTransfers;
		INT			MinSources;
		DWORD		ConnectThrottle;
		INT			QueueLimit;
		DWORD		SearchPeriod;
		DWORD		StarveTimeout;
		DWORD		RetryDelay;
		DWORD		PushTimeout;
		BOOL		StaggardStart;
		BOOL		AllowBackwards;
		DWORD		ChunkSize;
		DWORD		ChunkStrap;
		BOOL		Metadata;
		BOOL		VerifyFiles;
		BOOL		VerifyTiger;
		BOOL		VerifyED2K;
		BOOL		NeverDrop;
		BOOL		RequestHash;
		BOOL		RequestHTTP11;
		BOOL		RequestURLENC;
		DWORD		SaveInterval;
		BOOL		FlushSD;
		BOOL		ShowSources;
		BOOL		ShowPercent;
		BOOL		ShowGroups;
		BOOL		AutoExpand;
		BOOL		AutoClear;
		DWORD		ClearDelay;
		DWORD		FilterMask;
		BOOL		ShowMonitorURLs;
	} Downloads;
	
	struct sUploads
	{
		int			MaxPerHost;
		DWORD		FreeBandwidthValue;
		DWORD		FreeBandwidthFactor;
		DWORD		ClampdownFactor;
		DWORD		ClampdownFloor;
		BOOL		ThrottleMode;
		DWORD		QueuePollMin;
		DWORD		QueuePollMax;
		DWORD		RotateChunkLimit;
		BOOL		SharePartials;
		BOOL		ShareTiger;
		BOOL		ShareHashset;
		BOOL		ShareMetadata;
		BOOL		SharePreviews;
		BOOL		DynamicPreviews;
		DWORD		PreviewQuality;
		DWORD		PreviewTransfers;
		BOOL		AllowBackwards;
		BOOL		HubUnshare;
		CString		BlockAgents;
		BOOL		AutoClear;
		DWORD		ClearDelay;
		DWORD		FilterMask;
	} Uploads;
	
	struct sRemote
	{
		BOOL		Enable;
		CString		Username;
		CString		Password;
	} Remote;
	
	struct sLive
	{
		DWORD		BandwidthScale;
		BOOL		LoadWindowState;
		BOOL		AutoClose;
		BOOL		FirstRun;
	} Live;

// Attributes : Item List
protected:
	CPtrList	m_pItems;
public:
	class Item
	{
	public:
		Item(LPCTSTR pszName, DWORD* pDword, DOUBLE* pFloat, CString* pString);
		void	Load();
		void	Save();
	public:
		CString		m_sName;
		DWORD*		m_pDword;
		DOUBLE*		m_pFloat;
		CString*	m_pString;
	};

// Operations
public:
	void	Load();
	void	Save(BOOL bShutdown = FALSE);
	Item*	GetSetting(LPCTSTR pszName) const;
	Item*	GetSetting(LPVOID pValue) const;
public:
	BOOL	LoadWindow(LPCTSTR pszName, CWnd* pWindow);
	void	SaveWindow(LPCTSTR pszName, CWnd* pWindow);
	BOOL	LoadList(LPCTSTR pszName, CListCtrl* pCtrl, int nSort = 0);
	void	SaveList(LPCTSTR pszName, CListCtrl* pCtrl);
	CString	SmartAgent(LPCTSTR pszAgent);
	CString	SmartVolume(QWORD nVolume, BOOL bInKB, BOOL bRateInBits = FALSE);
	QWORD	ParseVolume(LPCTSTR psz, BOOL bSpeedInBits);
	BOOL	CheckStartup();
	void	SetStartup(BOOL bStartup);
protected:
	void	Setup();
	void	Add(LPCTSTR pszName, DWORD* pDword, DWORD nDefault);
	void	Add(LPCTSTR pszName, int* pDword, DWORD nDefault);
	void	Add(LPCTSTR pszName, DOUBLE* pFloat, DOUBLE nDefault);
	void	Add(LPCTSTR pszName, CString* pString, LPCTSTR pszDefault);
	void	SmartUpgrade();

};

extern CSettings Settings;

enum
{
	GUI_WINDOWED, GUI_TABBED, GUI_BASIC
};

#define GNUTELLA_DEFAULT_PORT	6346
#define ED2K_DEFAULT_PORT		4661
