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
		CString		Path;						//Installation path for Shareaza
		CString		UserPath;					//Path for user data. (May be the same as above for single user installs)
		BOOL		Debug;
		BOOL		DebugLog;
		DWORD		MaxDebugLogSize;			//Max size of the lof file
		BOOL		UpdateCheck;
		DWORD		DiskSpaceWarning;			//Value at which to warn the user about low disk space
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
		BOOL		LowResMode;
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

		DWORD		BufferSize;		// I/O buffer for hash operation in MB, ignored if Parallel = 1; 0 use 1/4 of phys ram
									// if buffer allocation fails hashing will run with Parallel = 1
		DWORD		Parallel;					// how many files to hash parallel: 1..6; 0 for autoselect
		int			LowPriorityHashing;			// desired speed in MB/s when hashing with low priority
	} Library;

	struct sSearch
	{
		CString		LastSchemaURI;
		CString		BlankSchemaURI;
		BOOL		HideSearchPanel;
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
		BOOL		AdultFilter;
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
		BOOL		IgnoreLocalIP;				//Ingnore all 'local' (LAN) IPs
		BOOL		IgnoreOwnIP;				//Do not accept any ports on your external IP as a source
		DWORD		TimeoutConnect;
		DWORD		TimeoutHandshake;
		DWORD		TimeoutTraffic;
		DWORD		SendBuffer;
		BOOL		RequireForTransfers;		//Only upload/download to connected networks
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
		DWORD		ClientMode;					// MODE_AUTO, MODE_LEAF, MODE_ULTRAPEER
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
		DWORD		ClientMode;					//MODE_AUTO, MODE_LEAF, MODE_HUB
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
		DWORD		QueryLimit;
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
		BOOL		LearnNewServers;
		CString		ServerListURL;
		DWORD		RequestPipe;
		DWORD		RequestSize;
		DWORD		FrameSize;
		DWORD		ReAskTime;
		DWORD		DequeueTime;
		BOOL		ExtendedRequest;
		BOOL		MagnetSearch;
		BOOL		TagNames;
	} eDonkey;
	
	struct sBitTorrent
	{
		BOOL		AdvancedInterface;			//Display BT 'extras' (Seed Torrent box, etc)
		CString		TorrentCreatorPath;			//Location of the program used to create .torrent files
		CString		DefaultTracker;
		DWORD		DefaultTrackerPeriod;		//Delay between tracker contact attempts if one is not specified by tracker
		DWORD		LinkTimeout;
		DWORD		LinkPing;
		DWORD		RequestPipe;
		DWORD		RequestSize;
		DWORD		RequestLimit;
		DWORD		RandomPeriod;
		DWORD		SourceExchangePeriod;
		int			UploadCount;
		int			DownloadConnections;		//Number active torrent connections allowed
		int			DownloadTorrents;			//Number of torrents to download at once
		BOOL		Endgame;					//Allow endgame mode when completing torrents. (Download same chunk from multiple sources)
	} BitTorrent;

	struct sDownloads
	{
		CString		IncompletePath;				//Where incomplete downloads are stored
		CString		CompletePath;				//Where downloads are moved when they complete
		CString		TorrentPath;				//Where .torrent files are stored
		CString		CollectionPath;				//Where .collection and .co files are stored
		DWORD		BufferSize;
		DWORD		SparseThreshold;			//NTFS 'sparse files' are not used on files below this size. (0 = Disable)
		INT			MaxFiles;					//How many files download at once
		INT			MaxTransfers;				//How many total tranfers take place
		INT			MaxFileTransfers;			//How mnay transfers are allowed per file
		INT			MaxFileSearches;			//Number number of files over the download limit that prepare to start. (Search, etc)
		INT			MaxConnectingSources;		//The maximum number of sources that can be in the 'connecting' state. (Important for XPsp2)
		INT			MinSources;					//The minimum number of sources a download has before Shareaza regards it as having a problem
		DWORD		ConnectThrottle;			//Delay between download attempts. (Very important for routers)
		INT			QueueLimit;					//Longest queue to wait in. (0 to disable. This should be >800 or 0 to get good performance from ed2k)
		DWORD		SearchPeriod;
		DWORD		StarveTimeout;
		DWORD		StarveGiveUp;				//How long (in hours) before Shareaza will give up and try another download if it gets no data. (+ 0-9 h, depending on sources)
		DWORD		RetryDelay;
		DWORD		PushTimeout;
		BOOL		StaggardStart;
		BOOL		AllowBackwards;				//Permit download to run in reverse when appropriate
		DWORD		ChunkSize;
		DWORD		ChunkStrap;
		BOOL		Metadata;
		BOOL		VerifyFiles;
		BOOL		VerifyTiger;
		BOOL		VerifyED2K;
		BOOL		NeverDrop;					//Do not drop bad sources (may pollute source list with many dead sources)
		BOOL		RequestHash;
		BOOL		RequestHTTP11;
		BOOL		RequestURLENC;
		DWORD		SaveInterval;
		BOOL		FlushSD;
		BOOL		ShowSources;
		BOOL		ShowPercent;				//Display small green % complete bar on progress graphic
		BOOL		ShowGroups;
		BOOL		AutoExpand;
		BOOL		AutoClear;
		DWORD		ClearDelay;
		DWORD		FilterMask;
		BOOL		ShowMonitorURLs;
		BOOL		SortColumns;				//Allow user to sort downloads by clicking column headers
		BOOL		SortSources;				//Automatically sort sources (Status, protocol, queue)
		int			SourcesWanted;				//Number of sources Shareaza 'wants'. (Will not request more than this number of sources from ed2k)
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

	struct sScheduler
	{
		BOOL		Enable;						//Enable the scheduler
		DWORD		LimitedBandwidth;			//% of bandwidth to use in limited mode
		BOOL		LimitedNetworks;			//Only connect to G2/BT when limited
		BOOL		AllowHub;					//Allow hub mode while scheduler is active
	} Scheduler;
	
	struct sLive
	{
		BOOL		DiskWarning;				//Has the user been warned of low disk space?
		BOOL		AdultWarning;				//Has the user been warned about the adult filter?
		DWORD		BandwidthScale;				//Monitor slider settings
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

enum
{
	MODE_AUTO, MODE_LEAF, MODE_HUB, MODE_ULTRAPEER = MODE_HUB
};

#define GNUTELLA_DEFAULT_PORT	6346
#define ED2K_DEFAULT_PORT		4661
