//
// Settings.h
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

#pragma once

#include "Shareaza.h"


enum
{
	bits = 1, Bytes = 8, Kilobits = 1024, KiloBytes = 8192
};

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
		DWORD		LogLevel;					// Log severity (0 - MSG_ERROR .. 4 - MSG_DEBUG)
		bool		SearchLog;					// Display search facility log information
		bool		DebugLog;					// Create a log file
		DWORD		MaxDebugLogSize;			// Max size of the log file
		DWORD		DiskSpaceWarning;			// Value at which to warn the user about low disk space
		DWORD		DiskSpaceStop;				// Value at which to pause all downloads due to low disk space
		DWORD		MinTransfersRest;			// For how long at least to suspend Transfers each round
		DWORD		GUIMode;
		DWORD		CloseMode;
		bool		TrayMinimise;
		bool		ShowTimestamp;
		bool		SizeLists;
		bool		HashIntegrity;
		bool		RatesInBytes;				// Show speeds in Bytes/second
		DWORD		RatesUnit;					// Units that the rates are to be displayed in
		bool		AlwaysOpenURLs;
		CString		Language;
		bool		LanguageRTL;				// Right-to-Left GUI (2000, XP only)
		bool		IgnoreXPsp2;				// Ignore the presence of Windows XPsp2 limits
		bool		DebugBTSources;				// Display received sources for BT download when seeding
		bool		ItWasLimited;				// If the user path the half-open connection limit we change the settings back to gain full speed
		bool		FirstRun;					// Is this the first time Shareaza is being run?
		bool		Upgrade;					// Just installed or upgraded
		DWORD		SmartVersion;				// Settings version
		bool		Running;
		bool		DialogScan;					// Set Skin engine to "dialog scan" mode. This produces "C:\Dialog.xml" file with dialog templates and disables dialog translation.
		CString		LastSettingsPage;			// Last selected Settings dialog page
		DWORD		LastSettingsIndex;			// Top item index of Advanced Settings list
		bool		SearchPanelResults;			// Search Results Panel state (open or closed)
		CString		AntiVirus;					// Anti-virus path or CLSID
	} General;

	struct sVersionCheck
	{
		DWORD		NextCheck;
		CString		Quote;
		bool		UpdateCheck;				// Does Shareaza check for new versions?
		CString		UpdateCheckURL;
		CString		UpgradePrompt;
		CString		UpgradeFile;
		CString		UpgradeSHA1;
		CString		UpgradeTiger;
		CString		UpgradeSize;
		CString		UpgradeSources;
		CString		UpgradeVersion;
	} VersionCheck;

	struct sInterface
	{
		bool		AutoComplete;				// Use auto-complete feature
		bool		CoolMenuEnable;				// Use cool skinned menus
		bool		LowResMode;
		DWORD		TipDelay;
		DWORD		TipAlpha;
		bool		TipSearch;
		bool		TipLibrary;
		bool		TipDownloads;
		bool		TipUploads;
		bool		TipNeighbours;
		bool		TipMedia;
		bool		Snarl;						// Use Snarl notifications - http://www.getsnarl.info/
		DWORD		SearchWindowsLimit;			// Maximum amount of opened Search windows
		DWORD		BrowseWindowsLimit;			// Maximum amount of opened Browse Host windows
	} Interface;

	struct sWindows
	{
		bool		RunWizard;
		bool		RunWarnings;
		bool		RunPromote;
	} Windows;

	struct sToolbars
	{
		bool		ShowRemote;
		bool		ShowMonitor;
	} Toolbars;

	struct sFonts
	{
		CString		DefaultFont;				// Main font
		CString		PacketDumpFont;				// Packet Window font
		CString		SystemLogFont;				// System Window font
		DWORD		FontSize;					// The basic font size
	} Fonts;

	struct sLibrary
	{
		bool		WatchFolders;
		DWORD		WatchFoldersTimeout;
		bool		VirtualFiles;
		bool		SourceMesh;
		DWORD		SourceExpire;
		DWORD		TigerHeight;
		DWORD		QueryRouteSize;
		DWORD		HistoryTotal;
		DWORD		HistoryDays;
		bool		ShowVirtual;
		DWORD		TreeSize;
		DWORD		PanelSize;
		bool		ShowPanel;
		bool		StoreViews;
		bool		ShowCoverArt;
		CString		SchemaURI;
		CString		FilterURI;
		string_set	SafeExecute;
		string_set	PrivateTypes;
		DWORD		ThumbSize;
		bool		HighPriorityHash;			// Use high priority hashing
		bool		HashWindow;					// Display annoying hashing window
		bool		CreateGhosts;				// Default action in the delete file dialog
		DWORD		GhostLimit;					// Maximum amount of ghost files
		DWORD		HighPriorityHashing;		// desired speed in MB/s when hashing with hi priority
		DWORD		LowPriorityHashing;			// desired speed in MB/s when hashing with low priority
		DWORD		MaxMaliciousFileSize;		// Size for which to trigger malicious software search
		bool		UseFolderGUID;				// Save/Load folder GUID using NTFS stream
		bool		MarkFileAsDownload;			// Mark downloaded file using NTFS stream as Internet Explorer
		bool		UseCustomFolders;			// Use desktop.ini
		bool		UseWindowsLibrary;			// Use Windows 7 Library
		bool		ScanAPE;					// Enable .ape,.mac,.apl metadata extraction by internals
		bool		ScanASF;					// Enable .asf,.wma,.wmv metadata extraction by internals
		bool		ScanAVI;					// Enable .avi metadata extraction by internals
		bool		ScanCHM;					// Enable .chm metadata extraction by internals
		bool		ScanEXE;					// Enable .exe,.dll metadata extraction by internals
		bool		ScanFLV;					// Enable .flv metadata extraction by internals
		bool		ScanImage;					// Enable .jpg,.jpeg,.gif,.png,.bmp metadata extraction by internals
		bool		ScanMP3;					// Enable .mp3 metadata extraction by internals
		bool		ScanMPEG;					// Enable .mpeg,.mpg metadata extraction by internals
		bool		ScanMSI;					// Enable .msi metadata extraction by internals
		bool		ScanOGG;					// Enable .ogg metadata extraction by internals
		bool		ScanPDF;					// Enable .pdf metadata extraction by internals
		bool		ScanProperties;				// Enable Windows properties metadata extraction by internals
		bool		SmartSeriesDetection;		// Organize video files in Library by using predefined patterns
		CString		LastUsedView;				// Name of last used view
		CString		URLExportFormat;			// Template for URL export
		DWORD		TooManyWarning;				// Too many files warning. 0 - ask user; 1 - no; 2 - yes.
	} Library;

	struct sSearch
	{
		CString		LastSchemaURI;
		CString		BlankSchemaURI;
		bool		HideSearchPanel;
		bool		SearchPanel;
		bool		ExpandMatches;
		bool		HighlightNew;
		bool		SwitchToTransfers;
		bool		SchemaTypes;
		bool		ShowNames;
		DWORD		FilterMask;
		CString		MonitorSchemaURI;
		CString		MonitorFilter;
		DWORD		MonitorQueue;
		DWORD		BrowseTreeSize;
		bool		DetailPanelVisible;
		DWORD		DetailPanelSize;
		DWORD		MaxPreviewLength;
		bool		AdultFilter;
		bool		AdvancedPanel;
		DWORD		SpamFilterThreshold;		// Percentage of spam hits which triggers file sources to be treated as a spam
		DWORD		GeneralThrottle;			// A general throttle for how often each individual search may run. Low values may cause source finding to get overlooked.
		DWORD		ClearPrevious;				// Clear previous search results? 0 - ask user; 1 - no; 2 - yes.
		bool		SanityCheck;				// Drop hits of banned hosts
		bool		AutoPreview;				// Auto-previewing selected hit
	} Search;

	struct sMediaPlayer
	{
		bool		EnablePlay;
		bool		EnableEnqueue;
		string_set	FileTypes;
		bool		Repeat;
		bool		Random;
		MediaZoom	Zoom;
		double		Aspect;
		double		Volume;
		bool		ListVisible;
		DWORD		ListSize;
		bool		StatusVisible;
		CString		MediaServicesCLSID;
		CString		Mpeg1PreviewCLSID;
		CString		Mp3PreviewCLSID;
		CString		AviPreviewCLSID;
		CString		VisWrapperCLSID;
		CString		VisSoniqueCLSID;
		CString		VisCLSID;
		CString		VisPath;
		DWORD		VisSize;
		string_set	ServicePath;				// Keeps track of user players, beginning astrix shows the order and tailing shows selected
		bool		ShortPaths;					// Some players differently handle unicode paths but they can launch files using 8.3 paths
	} MediaPlayer;

	struct sWeb
	{
		bool		Magnet;
		bool		Foxy;						// Handle Foxy links (slightly altered magnet)
		bool		Gnutella;
		bool		ED2K;
		bool		Piolet;
		bool		Torrent;
		bool		DC;
	} Web;

	struct sConnection
	{
		bool		AutoConnect;
		DWORD		FirewallState;
		CString		OutHost;
		CString		InHost;
		DWORD		InPort;
		bool		InBind;
		bool		RandomPort;
		DWORD		InSpeed;					// Inbound internet connection speed in Kilobits/seconds
		DWORD		OutSpeed;					// Outbound internet connection speed in Kilobits/seconds
		bool		IgnoreLocalIP;				// Ingnore all 'local' (LAN) IPs
		bool		IgnoreOwnIP;				// Do not accept any ports on your external IP as a source
		bool		IgnoreOwnUDP;				// Do not accept any ports on your external IP as a source of UDP
		DWORD		TimeoutConnect;
		DWORD		TimeoutHandshake;
		DWORD		TimeoutTraffic;
		DWORD		SendBuffer;
		bool		RequireForTransfers;		// Only upload/download to connected networks
		DWORD		ConnectThrottle;			// Delay between connection attempts to neighbors (milliseconds)
		DWORD		FailurePenalty;				// Delay after connection failure (seconds, default = 300) (Neighbour connections)
		DWORD		FailureLimit;				// Max allowed connection failures (default = 3) (Neighbour connections)
		bool		DetectConnectionLoss;		// Detect loss of internet connection
		bool		DetectConnectionReset;		// Detect regaining of internet connection
		bool		ForceConnectedState;		// Force WinINet into a connected state on startup. (Put IE into online mode)
		bool		SlowConnect;				// Connect to one network at a time. Don't download while connecting. (XPsp2)
		bool		EnableFirewallException;	// Create Firewall exception at startup
		bool		DeleteFirewallException;	// Delete Firewall exception on shutdown
		bool		EnableUPnP;					// UPnP: Enable forwarded ports on startup
		bool		DeleteUPnPPorts;			// UPnP: Delete forwarded ports on shutdown
		DWORD		UPnPRefreshTime;			// UPnP: Refresh time of port mappings (millisecond)
		DWORD		UPnPTimeout;				// UPnP: The maximum time for waiting any device response (millisecond)
		bool		SkipWANPPPSetup;			// UPnP: Skip WANPPPConn1 device setup
		bool		SkipWANIPSetup;				// UPnP: Skip WANIPConn1 device setup
		DWORD		ZLibCompressionLevel;		// ZLib compression level: 0(none/fastest)..9(best/slowest)
		bool		EnableMulticast;			// Send and accept multi-cast packets (default = true)
		bool		MulticastLoop;				// Use multi-cast loopback (for debugging, default = false)
		DWORD		MulticastTTL;				// TTL for multi-cast packets (default = 1)
		bool		EnableBroadcast;			// Send and accept broadcast packets (for LAN, default = false)
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
		DWORD		Downloads;					// Inbound speed limit in Bytes/seconds
		DWORD		Uploads;					// Outbound speed limit in Bytes/seconds
	} Bandwidth;

	struct sCommunity
	{
		bool		ChatEnable;					// Is chat enabled with compatible clients?
		bool		ChatAllNetworks;			// Is chat allowed over other protocols? (ed2k, etc)
		bool		ChatFilter;					// Filter out chat spam
		bool		ChatFilterED2K;				// Filter known ed2k spam. (pretty bad- always on)
		bool		ChatCensor;					// Censor 'bad' words from chat. (Uses adult filter)
		bool		Timestamp;
		bool		ServeProfile;
		bool		ServeFiles;
		DWORD		AwayMessageIdleTime;		// Time in secs of idle system time before showing away message
		DWORD		UserPanelSize;				// Width of chat users panel (pixels)
	} Community;

	struct sDiscovery
	{
		DWORD		AccessThrottle;
		DWORD		Lowpoint;
		DWORD		FailureLimit;
		DWORD		AccessPeriod;
		DWORD		DefaultUpdate;
		DWORD		BootstrapCount;
		DWORD		CacheCount;					// Limit ability to learn new caches
	} Discovery;

	struct sGnutella
	{
		DWORD		ConnectFactor;				// Number of hosts we simultaneously try when connecting to single hub
		bool		DeflateHub2Hub;
		bool		DeflateLeaf2Hub;
		bool		DeflateHub2Leaf;
		DWORD		MaxResults;					// Maximum new results we want on single Search button press
		DWORD		MaximumPacket;				// Drop packets large than specified (32...256 KB)
		DWORD		MaxHits;					// Maximum file hits in search result (divided to packets by HitsPerPacket)
		DWORD		HitsPerPacket;				// Maximum file hits in single search result packet
		DWORD		RouteCache;					// Life time of node route (seconds)
		DWORD		HostCacheSize;				// Number of hosts of each type in Host cache
		DWORD		HostCacheView;
		DWORD		ConnectThrottle;			// Delay between connection attempts for same host (sec)
		bool		SpecifyProtocol;			// Specify G1 or G2 when initiating a connection
	} Gnutella;

	struct sGnutella1
	{
		DWORD		ClientMode;					// Desired mode of operation: MODE_AUTO, MODE_LEAF, MODE_ULTRAPEER
		bool		EnableToday;
		bool		EnableAlways;
		DWORD		NumHubs;					// Number of ultrapeers a leaf has
		DWORD		NumLeafs;					// Number of leafs an ultrapeer has
		DWORD		NumPeers;					// Number of peers an ultrapeer has
		DWORD		PacketBufferSize;			// Number of packets in packet buffer
		DWORD		PacketBufferTime;			// Life time of packet in packet buffer before drop (milliseconds)
		DWORD		DefaultTTL;
		DWORD		SearchTTL;
		DWORD		TranslateTTL;
		DWORD		MaximumTTL;
		DWORD		MaximumQuery;
		bool		EnableGGEP;
		bool		EnableOOB;
		bool		VendorMsg;
		DWORD		QueryThrottle;				// Throttle for G1 neighbor searches (sec)
		DWORD		HostCount;					// Number of hosts in X-Try-Ultrapeers
		DWORD		HostExpire;
		DWORD		PingFlood;
		DWORD		PingRate;
		DWORD		PongCache;
		DWORD		PongCount;
		bool		QueryHitUTF8;				// Use UTF-8 encoding to read Gnutella1 QueryHit packets
		bool		QuerySearchUTF8;			// Use UTF-8 encoding to create Gnutella1 Query packets
		DWORD		MaxHostsInPongs;			// The number of hosts included in the response of pings having SCP GGEP block
		DWORD		MCastPingRate;				// Multicast ping rate (ticks)
		DWORD		QueryGlobalThrottle;		// Multicast query rate (ticks)
	} Gnutella1;

	struct sGnutella2
	{
		DWORD		ClientMode;					// Desired mode of operation: MODE_AUTO, MODE_LEAF, MODE_HUB
		bool		HubVerified;
		bool		EnableToday;
		bool		EnableAlways;
		DWORD		NumHubs;					// Number of hubs a leaf has
		DWORD		NumLeafs;					// Number of leafs a hub has
		DWORD		NumPeers;					// Number of peers a hub has
		DWORD		PingRelayLimit;				// Number of other leafs to forward a /PI/UDP to: 10 - 30
		DWORD		UdpMTU;
		DWORD		UdpBuffers;
		DWORD		UdpInFrames;
		DWORD		UdpOutFrames;
		DWORD		UdpGlobalThrottle;
		DWORD		UdpOutExpire;
		DWORD		UdpOutResend;
		DWORD		UdpInExpire;
		DWORD		LNIPeriod;
		DWORD		KHLPeriod;
		DWORD		KHLHubCount;
		DWORD		HAWPeriod;
		DWORD		HostCurrent;
		DWORD		HostCount;					// Number of hosts in X-Try-Hubs
		DWORD		HostExpire;
		DWORD		PingRate;
		DWORD		QueryGlobalThrottle;		// Max G2 query rate (Cannot exceed 8/sec)
		DWORD		QueryThrottle;				// Throttle for G2 neighbor searches (sec)
		DWORD		QueryHostDeadline;
		DWORD		RequeryDelay;				// Throttle for G2 neighbor UDP searches (sec)
		DWORD		HubHorizonSize;
		DWORD		QueryLimit;
	} Gnutella2;

	struct seDonkey
	{
		bool		EnableToday;
		bool		EnableAlways;
		bool		FastConnect;				// Try connecting to 2 servers to get online faster
		bool		ForceHighID;				// Reconnect if low-id
		DWORD		NumServers;					// 1
		DWORD		MaxLinks;					// Max ed2k client links
		DWORD		MaxResults;
		DWORD		MaxShareCount;				// Hard limit on file list sent to server
		bool		ServerWalk;					// Enable global UDP walk of servers
		DWORD		StatsGlobalThrottle;		// Global throttle for server UDP stats requests
		DWORD		QueryGlobalThrottle;		// Global throttle for all ed2k searches (TCP, UDP, manual and auto)
		DWORD		StatsServerThrottle;		// Max rate at which an individual server can be asked for stats
		DWORD		QueryThrottle;				// Throttle for ED2K neighbor searches (sec)
		DWORD		QueryFileThrottle;			// Max rate a file can have GetSources done
		DWORD		GetSourcesThrottle;			// Max rate a general GetSources can done
		DWORD		QueueRankThrottle;			// How frequently queue ranks are sent
		DWORD		PacketThrottle;				// ED2K packet rate limiter
		DWORD		SourceThrottle;				// ED2K source rate limiter
		bool		LearnNewServers;			// Get new servers from servers
		bool		LearnNewServersClient;		// Get new servers from clients
		CString		ServerListURL;
		DWORD		RequestPipe;
		DWORD		RequestSize;
		DWORD		FrameSize;
		DWORD		ReAskTime;
		DWORD		DequeueTime;
		DWORD		ExtendedRequest;
		bool		SendPortServer;				// Send port in tag to ed2k servers. (not needed for newer servers)
		bool		MagnetSearch;				// Search for magnets over ed2k (lower server load)
		DWORD		MinServerFileSize;			// Minimum size a file in the library must be in order to be included in the server file list. (In KB)
		DWORD		DefaultServerFlags;			// Default server flags (for UDP searches)
		bool		Endgame;					// Allow endgame mode when completing downloads. (Download same chunk from multiple sources)
		bool		LargeFileSupport;			// Allow 64 bit file sizes
		bool		AutoDiscovery;				// Auto query for a new server list using discovery services
	} eDonkey;

	struct sDC
	{
		bool		EnableToday;
		bool		EnableAlways;
		DWORD		NumServers;					// default: 1 hub
		DWORD		QueryThrottle;				// Throttle for DC++ neighbor searches (s), default: two minutes delay
		DWORD		ReAskTime;					// How often Shareaza re-ask a remote client about download (ms), default: every minute
		DWORD		DequeueTime;				// Timeout for remote client confirmation of upload queue (ms), default: 5 min
		bool		AutoDiscovery;				// Auto query for a new server list using discovery services
	} DC;

	struct sBitTorrent
	{
		bool		EnableToday;
		bool		EnableAlways;
		CString		TorrentCreatorPath;			// Location of the program used to create .torrent files
		CString		DefaultTracker;
		DWORD		DefaultTrackerPeriod;		// Delay between tracker contact attempts if one is not specified by tracker
		DWORD		TorrentCodePage;			// The code page to assume for a .torrent file if it isn't UTF-8
		DWORD		LinkTimeout;
		DWORD		LinkPing;
		DWORD		UtPexPeriod;
		DWORD		RequestPipe;
		DWORD		RequestSize;
		DWORD		RequestLimit;
		DWORD		RandomPeriod;
		DWORD		SourceExchangePeriod;
		DWORD		UploadCount;				// Number of active torrent uploads allowed
		DWORD		DownloadConnections;		// Number of active torrent connections allowed
		DWORD		DownloadTorrents;			// Number of torrents to download at once
		bool		Endgame;					// Allow endgame mode when completing torrents. (Download same chunk from multiple sources)
		bool		AutoClear;					// Clear completed torrents when they meet the required share ratio
		DWORD		ClearRatio;					// Share ratio a torrent must reach to be cleared. (Minimum 100%)
		bool		AutoSeed;					// Automatically re-seed most recently completed torrent on start-up
		DWORD		BandwidthPercentage;		// Percentage of bandwidth to use when BT active.
		bool		TrackerKey;					// Send a key (random value) to trackers
		bool		PreferenceBTSources;		// Preference downloading from BT sources where appropriate
		DWORD		HostExpire;					// DHT hosts expiration time (seconds)
		bool		EnableDHT;					// Enable DHT protocol
		DWORD		ConnectThrottle;			// Throttle for DHT request (sec)
		DWORD		QueryHostDeadline;			// Time to wait for DHT reply (sec)
		bool		AutoMerge;					// Automatically merge download with local files on start-up
		CString		PeerID;						// Use this peer ID for trackers in form of "CCvvvv" where "CC" - agent code ("SZ", "UT" etc,), v.v.v.v - version
		bool		EnablePromote;				// Enable regular to torrent download promotion
	} BitTorrent;

	struct sDownloads
	{
		CString		IncompletePath;				// Where incomplete downloads are stored
		CString		CompletePath;				// Where downloads are moved when they complete
		CString		TorrentPath;				// Where .torrent files are stored
		CString		CollectionPath;				// Where .collection and .co files are stored
		DWORD		BufferSize;
		DWORD		SparseThreshold;			// NTFS 'sparse files' are not used on files below this size. (0 = Disable)
		DWORD		MaxAllowedFailures;
		DWORD		MaxFiles;					// How many files download at once
		DWORD		MaxTransfers;				// How many total transfers take place
		DWORD		MaxFileTransfers;			// How many transfers are allowed per file
		DWORD		MaxFileSearches;			// Number number of files over the download limit that prepare to start. (Search, etc)
		DWORD		MaxConnectingSources;		// The maximum number of sources that can be in the 'connecting' state. (Important for XPsp2)
		DWORD		MinSources;					// The minimum number of sources a download has before Shareaza regards it as having a problem
		DWORD		ConnectThrottle;			// Delay between download attempts. (Very important for routers)
		DWORD		QueueLimit;					// Longest queue to wait in. (0 to disable. This should be >800 or 0 to get good performance from ed2k)
		DWORD		SearchPeriod;
		DWORD		StarveTimeout;				// How long (in ticks) Shareaza will starve without new downloaded data before its try to search for more sources
		DWORD		StarveGiveUp;				// How long (in hours) before Shareaza will give up and try another download if it gets no data. (+ 0-9 h, depending on sources)
		DWORD		RetryDelay;
		DWORD		PushTimeout;
		bool		StaggardStart;
		bool		AllowBackwards;				// Permit download to run in reverse when appropriate
		DWORD		ChunkSize;
		DWORD		ChunkStrap;
		bool		Metadata;
		bool		VerifyFiles;
		bool		VerifyTiger;
		bool		VerifyED2K;
		bool		VerifyTorrent;
		bool		NeverDrop;					// Do not drop bad sources (may pollute source list with many dead sources)
		bool		RequestHash;
		bool		RequestHTTP11;
		bool		RequestURLENC;
		DWORD		SaveInterval;
		bool		FlushSD;
		bool		ShowSources;
		bool		SimpleBar;					// Displays a simplified progress bar (lower CPU use)
		bool		ShowPercent;				// Display small green % complete bar on progress graphic
		bool		ShowGroups;
		bool		AutoExpand;
		bool		AutoClear;
		DWORD		ClearDelay;
		DWORD		FilterMask;
		bool		ShowMonitorURLs;
		bool		SortColumns;				// Allow user to sort downloads by clicking column headers
		bool		SortSources;				// Automatically sort sources (Status, protocol, queue)
		DWORD		SourcesWanted;				// Number of sources Shareaza 'wants'. (Will not request more than this number of sources from ed2k)
		DWORD		MaxReviews;					// Maximum number of reviews to store per download
		bool		WebHookEnable;
		string_set	WebHookExtensions;
	} Downloads;

	struct sUploads
	{
		DWORD		MaxPerHost;					// Max simultaneous uploads to one remote client
		DWORD		FreeBandwidthValue;
		DWORD		FreeBandwidthFactor;
		DWORD		ClampdownFactor;
		DWORD		ClampdownFloor;
		bool		ThrottleMode;
		DWORD		QueuePollMin;
		DWORD		QueuePollMax;
		DWORD		RotateChunkLimit;
		bool		SharePartials;
		bool		ShareTiger;
		bool		ShareHashset;
		bool		ShareMetadata;
		bool		SharePreviews;
		bool		DynamicPreviews;
		DWORD		PreviewQuality;
		DWORD		PreviewTransfers;
		bool		AllowBackwards;				// Allow data to be sent from end of range to beginning where supported
		bool		HubUnshare;
		string_set	BlockAgents;
		bool		AutoClear;					// Automatically clear completed uploads ('Completed' queue)
		DWORD		ClearDelay;					// Delay between auto-clears
		DWORD		FilterMask;
		DWORD		RewardQueuePercentage;		// The percentage of each reward queue reserved for uploaders
	} Uploads;

	struct sRemote
	{
		bool		Enable;
		CString		Username;
		CString		Password;
	} Remote;

	struct sIRC
	{
		
		COLORREF		Colors[11];
		bool			Show;
		bool			FloodEnable;
		bool			Timestamp;
		CString			UserName;
		CString			RealName;
		CString			Nick;
		CString			Alternate;
		CString			ServerName;
		CString			ServerPort;
		CString			FloodLimit;
		CString			ScreenFont;
	} IRC;

	struct sLive
	{
		bool		DiskSpaceWarning;			// Has the user been warned of low disk space?
		bool		DiskWriteWarning;			// Has the user been warned of write problems?
		bool		AdultWarning;				// Has the user been warned about the adult filter?
		bool		QueueLimitWarning;			// Has the user been warned about limiting the max Q position accepted?
		bool		DefaultED2KServersLoaded;	// Has Shareaza already loaded default ED2K servers?
		bool		DefaultDCServersLoaded;		// Has Shareaza already loaded default DC++ servers?
		bool		DonkeyServerWarning;		// Has the user been warned about having an empty server list?
		bool		UploadLimitWarning;			// Has the user been warned about the ed2k/BT ratio?
		bool		DiskSpaceStop;				// Has Shareaza paused all downloads due to critical disk space?
		DWORD		BandwidthScale;				// Monitor slider settings
		bool		LoadWindowState;
		bool		AutoClose;
		bool		FirstRun;					// Is this the first time Shareaza is being run?
	} Live;

	struct sExperimental
	{
		bool		EnableDIPPSupport;			// Handle GDNA host cache exchange
		bool		TestBTPartials;
	} Experimental;

	struct sWINE
	{
		bool		MenuFix;
	} WINE;

// Attributes : Item List
public:
	enum Type
	{
		setNull, setString, setFont
	};

	class Item
	{
	public:
		inline Item(const LPCTSTR szSection, const LPCTSTR szName, bool* const pBool, const bool bDefault, const bool bHidden) throw() :
			m_szSection( szSection ),
			m_szName( szName ),
			m_pBool( pBool ),
			m_pDword( NULL ),
			m_pFloat( NULL ),
			m_pString( NULL ),
			m_pSet( NULL ),
			m_BoolDefault( bDefault ),
			m_StringDefault( NULL ),
			m_DwordDefault( 0 ),
			m_FloatDefault( 0.0 ),
			m_nScale( 1 ),
			m_nMin( 0 ),
			m_nMax( 1 ),
			m_szSuffix( NULL ),
			m_bHidden( bHidden ),
			m_nType( setNull )
		{
		}

		inline Item(const LPCTSTR szSection, const LPCTSTR szName, DWORD* const pDword, const DWORD nDefault, const DWORD nScale, const DWORD nMin, const DWORD nMax, const LPCTSTR szSuffix, const bool bHidden) throw() :
			m_szSection( szSection ),
			m_szName( szName ),
			m_pBool( NULL ),
			m_pDword( pDword ),
			m_pFloat( NULL ),
			m_pString( NULL ),
			m_pSet( NULL ),
			m_BoolDefault( false ),
			m_StringDefault( NULL ),
			m_DwordDefault( nDefault ),
			m_FloatDefault( 0.0 ),
			m_nScale( nScale ),
			m_nMin( nMin ),
			m_nMax( nMax ),
			m_szSuffix( szSuffix ),
			m_bHidden( bHidden ),
			m_nType( setNull )
		{
		}

		inline Item(const LPCTSTR szSection, const LPCTSTR szName, DOUBLE* const pFloat, const DOUBLE dDefault, const bool bHidden) throw() :
			m_szSection( szSection ),
			m_szName( szName ),
			m_pBool( NULL ),
			m_pDword( NULL ),
			m_pFloat( pFloat ),
			m_pString( NULL ),
			m_pSet( NULL ),
			m_BoolDefault( false ),
			m_StringDefault( NULL ),
			m_DwordDefault( 0 ),
			m_FloatDefault( dDefault ),
			m_nScale( 0 ),
			m_nMin( 0 ),
			m_nMax( 0 ),
			m_szSuffix( NULL ),
			m_bHidden( bHidden ),
			m_nType( setNull )
		{
		}

		inline Item(const LPCTSTR szSection, const LPCTSTR szName, CString* const pString, const LPCTSTR szDefault, const bool bHidden, const Type nType) throw() :
			m_szSection( szSection ),
			m_szName( szName ),
			m_pBool( NULL ),
			m_pDword( NULL ),
			m_pFloat( NULL ),
			m_pString( pString ),
			m_pSet( NULL ),
			m_BoolDefault( false ),
			m_StringDefault( szDefault ),
			m_DwordDefault( 0 ),
			m_FloatDefault( 0.0 ),
			m_nScale( 0 ),
			m_nMin( 0 ),
			m_nMax( 0 ),
			m_szSuffix( NULL ),
			m_bHidden( bHidden ),
			m_nType( nType )
		{
		}

		inline Item(const LPCTSTR szSection, const LPCTSTR szName, string_set* const pSet, const LPCTSTR szDefault, const bool bHidden) throw() :
			m_szSection( szSection ),
			m_szName( szName ),
			m_pBool( NULL ),
			m_pDword( NULL ),
			m_pFloat( NULL ),
			m_pString( NULL ),
			m_pSet( pSet ),
			m_BoolDefault( false ),
			m_StringDefault( szDefault ),
			m_DwordDefault( 0 ),
			m_FloatDefault( 0.0 ),
			m_nScale( 0 ),
			m_nMin( 0 ),
			m_nMax( 0 ),
			m_szSuffix( NULL ),
			m_bHidden( bHidden ),
			m_nType( setNull )
		{
		}

		inline bool operator==(LPVOID p) const
		{
			return ( m_pBool == p || m_pDword == p || m_pFloat == p ||
				m_pString == p || m_pSet == p );
		}

		void	Load();
		void	Save() const;
		void	Normalize();
		bool	IsDefault() const;
		void	SetDefault();
		template< class T > void	SetRange(T& pCtrl);

		const LPCTSTR		m_szSection;
		const LPCTSTR		m_szName;

		bool* const			m_pBool;
		DWORD* const		m_pDword;
		DOUBLE* const		m_pFloat;
		CString* const		m_pString;
		string_set* const	m_pSet;

		const bool			m_BoolDefault;
		const DWORD			m_DwordDefault;
		const DOUBLE		m_FloatDefault;
		const LPCTSTR		m_StringDefault;

		const DWORD			m_nScale;
		const DWORD			m_nMin;
		const DWORD			m_nMax;
		const LPCTSTR		m_szSuffix;

		const bool			m_bHidden;

		const Type			m_nType;
	};

protected:
	CList< Item* >	m_pItems;

	typedef std::map< CString, Item* > CSettingsMap;
	CSettingsMap m_pSettingsTable;

// Operations
public:
	void	Load();
	void	Save(BOOL bShutdown = FALSE);
	inline POSITION	GetHeadPosition() const
	{
		return m_pItems.GetHeadPosition();
	}
	inline Item*	GetNext(POSITION& rPosition) const
	{
		return m_pItems.GetNext( rPosition );
	}
	void	Normalize(LPVOID pSetting);
	bool	IsDefault(LPVOID pSetting) const;
	void	SetDefault(LPVOID pSetting);

	template< class T >
	void	SetRange(LPVOID pSetting, T& pCtrl)
	{
		for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
		{
			Item* pItem = m_pItems.GetNext( pos );
			if ( *pItem == pSetting )
			{
				pItem->SetRange< T >( pCtrl );
				break;
			}
		}
	}

	BOOL	LoadWindow(LPCTSTR pszName, CWnd* pWindow);
	void	SaveWindow(LPCTSTR pszName, CWnd* pWindow);
	BOOL	LoadList(LPCTSTR pszName, CListCtrl* pCtrl, int nSort = 0);
	void	SaveList(LPCTSTR pszName, CListCtrl* pCtrl);

	const CString	SmartSpeed(QWORD nVolume, int nVolumeUnits = Bytes, bool bTruncate = false) const;	// Convert speeds into formatted strings
	const CString	SmartVolume(QWORD nVolume, int nVolumeUnits = Bytes, bool bTruncate = false) const;	// Convert sizes into formatted strings
	QWORD	ParseVolume(const CString& strVolume, int nReturnUnits = Bytes) const;					// Convert size string into desired units
	DWORD	GetOutgoingBandwidth() const;																// Returns available outgoing bandwidth in KB/s
	BOOL	CheckStartup();
	void	SetStartup(BOOL bStartup);
	void	ClearSearches();

	bool	GetValue(LPCTSTR pszPath, VARIANT* value);

	void	OnChangeConnectionSpeed();

	static void LoadSet(string_set* pSet, LPCTSTR pszString);
	static CString SaveSet(const string_set* pSet);

protected:
	inline void Add(const LPCTSTR szSection, const LPCTSTR szName, bool* const pBool, const bool bDefault, const bool bHidden = false) throw()
	{
		m_pItems.AddTail( new Item( szSection, szName, pBool, bDefault, bHidden ) );
	}

	inline void Add(const LPCTSTR szSection, const LPCTSTR szName, DWORD* const pDword, const DWORD nDefault, DWORD nScale = 0, DWORD nMin = 0, DWORD nMax = 0, LPCTSTR szSuffix = NULL, const bool bHidden = false) throw()
	{
		m_pItems.AddTail( new Item( szSection, szName, pDword, nDefault, nScale, nMin, nMax, szSuffix, bHidden ) );
	}

	inline void Add(const LPCTSTR szSection, const LPCTSTR szName, DOUBLE* const pDouble, const DOUBLE dDefault, const bool bHidden = false) throw()
	{
		m_pItems.AddTail( new Item( szSection, szName, pDouble, dDefault, bHidden ) );
	}

	inline void Add(const LPCTSTR szSection, const LPCTSTR szName, CString* const pString, const LPCTSTR szDefault = NULL, const bool bHidden = false, const Type nType = setString) throw()
	{
		m_pItems.AddTail( new Item( szSection, szName, pString, szDefault, bHidden, nType ) );
	}

	inline void Add(const LPCTSTR szSection, const LPCTSTR szName, string_set* const pSet, const LPCTSTR szDefault, const bool bHidden = false) throw()
	{
		m_pItems.AddTail( new Item( szSection, szName, pSet, szDefault, bHidden ) );
	}

	void	SmartUpgrade();

// Inlines
public:
	// CSettings configurable user agent (Client Name + Version)
	inline const CString& SmartAgent() const throw() { return theApp.m_sSmartAgent; }

private:
	CSettings(const CSettings&);
	CSettings& operator=(const CSettings&);
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

enum
{
	CONNECTION_AUTO, CONNECTION_FIREWALLED, CONNECTION_OPEN, CONNECTION_OPEN_TCPONLY, CONNECTION_OPEN_UDPONLY
};
