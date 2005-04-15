//
// Settings.h
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
		CString		Path;						// Installation path for Shareaza
		CString		UserPath;					// Path for user data. (May be the same as above for single user installs)
		BOOL		Debug;						// Display debug information
		BOOL		DebugLog;					// Create a log file
		DWORD		MaxDebugLogSize;			// Max size of the log file
		BOOL		UpdateCheck;				// Does Shareaza check for new versions?
		DWORD		DiskSpaceWarning;			// Value at which to warn the user about low disk space
		DWORD		MinTransfersRest;			// For how long at least to suspend Transfers each round
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
		BOOL		IgnoreXPsp2;				// Ignore the presence of Windows XPsp2 limits
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
		BOOL		HighPriorityHash;			// Use high priority hashing

		//Not used at the moment
		DWORD		BufferSize;					// I/O buffer for hash operation in MB, ignored if Parallel = 1; 0 use 1/4 of phys ram
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
		BOOL		AdvancedPanel;
		DWORD		GeneralThrottle;			// A general throttle for how often each indidivual search may run. Low values may cause source finding to get overlooked. 
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
		BOOL		IgnoreLocalIP;				// Ingnore all 'local' (LAN) IPs
		BOOL		IgnoreOwnIP;				// Do not accept any ports on your external IP as a source
		DWORD		TimeoutConnect;
		DWORD		TimeoutHandshake;
		DWORD		TimeoutTraffic;
		DWORD		SendBuffer;
		BOOL		RequireForTransfers;		// Only upload/download to connected networks
		BOOL		AsyncIO;
		DWORD		ConnectThrottle;			// Delay between connection attempts. (Neighbour connections)
		BOOL		DetectConnectionLoss;		// Detect loss of internet connection
		BOOL		DetectConnectionReset;		// Detect regaining of internet connection
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
		BOOL		ChatEnable;					// Is chat enabled with compatible clients?
		BOOL		ChatAllNetworks;			// Is chat allowed over other protocols? (ed2k, etc)
		BOOL		ChatFilter;					// Filter out chat spam
		BOOL		ChatFilterED2K;				// Filter known ed2k spam. (pretty bad- always on)
		BOOL		ChatCensor;					// Censor 'bad' words from chat. (Uses adult filter)
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
		int			CacheCount;					// Limit ability to learn new caches
	} Discovery;
	
	struct sGnutella
	{
		DWORD		ConnectFactor;
		BOOL		DeflateHub2Hub;
		BOOL		DeflateLeaf2Hub;
		BOOL		DeflateHub2Leaf;
		DWORD		MaxResults;					// Maximum results to return to a single query
		DWORD		MaxHits;
		DWORD		HitsPerPacket;
		DWORD		RouteCache;
		DWORD		HostCacheSize;
		DWORD		HostCacheExpire;
		DWORD		HostCacheView;
		DWORD		ConnectThrottle;
		BOOL		BlockBlankClients;			// Block Ultrapeers with no user agent
	} Gnutella;
	
	struct sGnutella1
	{
		DWORD		ClientMode;					// Desired mode of operation: MODE_AUTO, MODE_LEAF, MODE_ULTRAPEER
		BOOL		EnableToday;
		BOOL		EnableAlways;
		int			NumHubs;					// Number of ultrapeers a leaf has
		int			NumLeafs;					// Number of leafs an ultrapeer has
		int			NumPeers;					// Number of peers an ultrapeer has
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
		int			HitQueueLimit;				// Protect G1 clients from badly configured queues
		BOOL		QueryHitUTF8;				// Use UTF-8 encoding to read Gnutella1 QueryHit packets
		BOOL		QuerySearchUTF8;			// Use UTF-8 encoding to create Gnutella1 Query packets
	} Gnutella1;

	struct sGnutella2
	{
		DWORD		ClientMode;					// Desired mode of operation: MODE_AUTO, MODE_LEAF, MODE_HUB
		BOOL		EnableToday;
		BOOL		EnableAlways;
		int			NumHubs;					// Number of hubs a leaf has
		int			NumLeafs;					// Number of leafs a hub has
		int			NumPeers;					// Number of peers a hub has
		int			PingRelayLimit;				// Number of other leafs to forward a /PI/UDP to: 10 - 30
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
		DWORD		QueryGlobalThrottle;		// Max G2 query rate (Cannot exceed 8/sec)
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
		BOOL		FastConnect;				// Try connecting to 2 servers to get online faster
		DWORD		NumServers;					// 1
		int			MaxLinks;					// Max ed2k client links
		int			MaxResults;
		DWORD		MaxShareCount;				// Hard limit on file list sent to server
		BOOL		ServerWalk;					// Enable global UDP walk of servers
		DWORD		StatsGlobalThrottle;		// Global throttle for server UDP stats requests
		DWORD		QueryGlobalThrottle;		// Global throttle for all ed2k searches (TCP, UDP, manual and auto)
		DWORD		StatsServerThrottle;		// Max rate at which an individual server can be asked for stats
		DWORD		QueryServerThrottle;		// Max rate at which an individual server can be queried
		DWORD		QueryFileThrottle;			// Max rate a file can have GetSources done
		DWORD		GetSourcesThrottle;			// Max rate a general GetSources can done
		DWORD		QueueRankThrottle;			// How frequently queue ranks are sent
		DWORD		PacketThrottle;				// ED2K packet rate limiter
		BOOL		LearnNewServers;
		CString		ServerListURL;
		DWORD		RequestPipe;
		DWORD		RequestSize;
		DWORD		FrameSize;
		DWORD		ReAskTime;
		DWORD		DequeueTime;
		BOOL		ExtendedRequest;
		BOOL		MagnetSearch;				// Search for magnets over ed2k (lower server load)
		DWORD		MinServerFileSize;			// Minimum size a file in the library must be in order to be included in the server file list. (In KB)
		BOOL		TagNames;					// Add (Shareaza.com) to user name over ed2k
		DWORD		DefaultServerFlags;			// Default server flags (for UDP searches)
	} eDonkey;
	
	struct sBitTorrent
	{
		BOOL		AdvancedInterface;			// Display BT 'extras' (Seed Torrent box, etc)
		CString		TorrentCreatorPath;			// Location of the program used to create .torrent files
		CString		DefaultTracker;
		DWORD		DefaultTrackerPeriod;		// Delay between tracker contact attempts if one is not specified by tracker
		int			TorrentCodePage;			// The code page to assume a .torrent file is in the event of an encoding error
		DWORD		LinkTimeout;
		DWORD		LinkPing;
		DWORD		RequestPipe;
		DWORD		RequestSize;
		DWORD		RequestLimit;
		DWORD		RandomPeriod;
		DWORD		SourceExchangePeriod;
		int			UploadCount;
		int			DownloadConnections;		// Number active torrent connections allowed
		int			DownloadTorrents;			// Number of torrents to download at once
		BOOL		Endgame;					// Allow endgame mode when completing torrents. (Download same chunk from multiple sources)
		BOOL		AutoClear;					// Clear completed torrents when they meet the required share ratio
		DWORD		ClearRatio;					// Share ratio a torrent must reach to be cleared. (Minimum 100%)
		DWORD		BandwidthPercentage;		// Percentage of bandwidth to use when BT active.
		BOOL		TrackerKey;					// Send a key (random value) to trackers
	} BitTorrent;

	struct sDownloads
	{
		CString		IncompletePath;				// Where incomplete downloads are stored
		CString		CompletePath;				// Where downloads are moved when they complete
		CString		TorrentPath;				// Where .torrent files are stored
		CString		CollectionPath;				// Where .collection and .co files are stored
		DWORD		BufferSize;
		DWORD		SparseThreshold;			// NTFS 'sparse files' are not used on files below this size. (0 = Disable)
		INT			MaxFiles;					// How many files download at once
		INT			MaxTransfers;				// How many total tranfers take place
		INT			MaxFileTransfers;			// How mnay transfers are allowed per file
		INT			MaxFileSearches;			// Number number of files over the download limit that prepare to start. (Search, etc)
		INT			MaxConnectingSources;		// The maximum number of sources that can be in the 'connecting' state. (Important for XPsp2)
		INT			MinSources;					// The minimum number of sources a download has before Shareaza regards it as having a problem
		DWORD		ConnectThrottle;			// Delay between download attempts. (Very important for routers)
		INT			QueueLimit;					// Longest queue to wait in. (0 to disable. This should be >800 or 0 to get good performance from ed2k)
		DWORD		SearchPeriod;
		DWORD		StarveTimeout;
		DWORD		StarveGiveUp;				// How long (in hours) before Shareaza will give up and try another download if it gets no data. (+ 0-9 h, depending on sources)
		DWORD		RetryDelay;
		DWORD		PushTimeout;
		BOOL		StaggardStart;
		BOOL		AllowBackwards;				// Permit download to run in reverse when appropriate
		DWORD		ChunkSize;
		DWORD		ChunkStrap;
		BOOL		Metadata;
		BOOL		VerifyFiles;
		BOOL		VerifyTiger;
		BOOL		VerifyED2K;
		BOOL		NeverDrop;					// Do not drop bad sources (may pollute source list with many dead sources)
		BOOL		RequestHash;
		BOOL		RequestHTTP11;
		BOOL		RequestURLENC;
		DWORD		SaveInterval;
		BOOL		FlushSD;
		BOOL		ShowSources;
		BOOL		SimpleBar;					// Displays a simplified progress bar (lower CPU use)
		BOOL		ShowPercent;				// Display small green % complete bar on progress graphic
		BOOL		ShowGroups;
		BOOL		AutoExpand;
		BOOL		AutoClear;
		DWORD		ClearDelay;
		DWORD		FilterMask;
		BOOL		ShowMonitorURLs;
		BOOL		SortColumns;				// Allow user to sort downloads by clicking column headers
		BOOL		SortSources;				// Automatically sort sources (Status, protocol, queue)
		int			SourcesWanted;				// Number of sources Shareaza 'wants'. (Will not request more than this number of sources from ed2k/BT)
		int			MaxReviews;					// Maximum number of reviews to store per download
	} Downloads;
	
	struct sUploads
	{
		int			MaxPerHost;					// Max simultaneous uploads to one remote client
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
		BOOL		AllowBackwards;				// Allow data to be sent from end of range to begining where supported
		BOOL		HubUnshare;
		CString		BlockAgents;
		BOOL		AutoClear;					// Automatically clear completed uploads ('Completed' queue)
		DWORD		ClearDelay;					// Delay between auto-clears
		DWORD		FilterMask;
		int 		RewardQueuePercentage;		// The percentage of each reward queue reserved for uploaders
	} Uploads;
	
	struct sRemote
	{
		BOOL		Enable;
		CString		Username;
		CString		Password;
	} Remote;

	struct sScheduler
	{
		BOOL		Enable;						// Enable the scheduler
		DWORD		LimitedBandwidth;			// % of bandwidth to use in limited mode
		BOOL		LimitedNetworks;			// Only connect to G2/BT when limited
		BOOL		AllowHub;					// Allow hub mode while scheduler is active
	} Scheduler;
	
	struct sLive
	{
		BOOL		DiskSpaceWarning;			// Has the user been warned of low disk space?
		BOOL		DiskWriteWarning;			// Has the user been warned of write problems?
		BOOL		AdultWarning;				// Has the user been warned about the adult filter?
		BOOL		QueueLimitWarning;			// Has the user been warned about limiting the max Q position accepted?
		DWORD		BandwidthScale;				// Monitor slider settings
		BOOL		LoadWindowState;
		BOOL		AutoClose;
		BOOL		FirstRun;					// Is this the firt time Shareaza is being run?
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
	CString	SmartVolume(QWORD nVolume, BOOL bInKB, BOOL bRateInBits = FALSE, BOOL bTruncate = FALSE );
	QWORD	ParseVolume(LPCTSTR psz, BOOL bSpeedInBits);
	DWORD	GetOutgoingBandwidth();						//Returns available outgoing bandwidth in KB/s
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
