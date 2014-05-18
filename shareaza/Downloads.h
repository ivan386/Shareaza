//
// Downloads.h
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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

class CBuffer;
class CConnection;
class CDownload;
class CDownloadSource;
class CEDClient;
class CLibraryFile;
class CMatchFile;
class CQueryHit;
class CShareazaURL;


class CDownloads
{
public:
	enum { dlPathNull, dlPathComplete, dlPathIncomplete };

// Construction
public:
	CDownloads();
	virtual ~CDownloads();

// Attributes
public:
	DWORD			m_tBandwidthAtMax;			// The last time download bandwidth was all in use
	DWORD			m_tBandwidthAtMaxED2K;		// The last time all ed2k bandwidth was used
	DWORD			m_nTransfers;
	DWORD			m_nBandwidth;
	bool			m_bClosing;
	QWORD			m_nComplete;				// The last complete size of incomplete downloads
	QWORD			m_nTotal;					// The last total size of incomplete downloads

private:
	CList< CDownload* >	m_pList;
	CMap< ULONG, ULONG, DWORD, DWORD > m_pHostLimits;
	int				m_nRunCookie;
	DWORD			m_tBandwidthLastCalc;		// The last time the bandwidth was calculated
	DWORD			m_nLimitGeneric;
	DWORD			m_nLimitDonkey;
	bool			m_bAllowMoreDownloads;
	bool			m_bAllowMoreTransfers;

// Operations
public:
	CDownload*	Add(BOOL bAddToHead = FALSE);
	CDownload*	Add(CQueryHit* pHit, BOOL bAddToHead = FALSE);
	CDownload*	Add(CMatchFile* pFile, BOOL bAddToHead = FALSE);
	CDownload*	Add(const CShareazaURL& oURL, BOOL bAddToHead = FALSE);
	void		PauseAll();
	void		ClearCompleted();
	void		ClearPaused();
	void		Clear(bool bClosing = false);
	void		CloseTransfers();

	int			GetSeedCount() const;
	INT_PTR		GetCount(BOOL bActiveOnly = FALSE) const;
	DWORD		GetTryingCount(bool bTorrentsOnly = false) const;
	DWORD		GetConnectingTransferCount() const;
	BOOL		Check(CDownloadSource* pSource) const;
	bool		CheckActive(CDownload* pDownload, int nScope) const;
	BOOL		Move(CDownload* pDownload, int nDelta);
	BOOL		Reorder(CDownload* pDownload, CDownload* pBefore);
	QWORD		GetAmountDownloadedFrom(IN_ADDR* pAddress);

	CDownload*	FindBySDName(const CString& sSDName) const;
	CDownload*	FindByPath(const CString& sPath) const;
	CDownload*	FindByURN(LPCTSTR pszURN, BOOL bSharedOnly = FALSE) const;
	CDownload*	FindBySHA1(const Hashes::Sha1Hash& oSHA1, BOOL bSharedOnly = FALSE) const;
	CDownload*	FindByTiger(const Hashes::TigerHash& oTiger, BOOL bSharedOnly = FALSE) const;
	CDownload*	FindByED2K(const Hashes::Ed2kHash& oED2K, BOOL bSharedOnly = FALSE) const;
	CDownload*	FindByBTH(const Hashes::BtHash& oBTH, BOOL bSharedOnly = FALSE) const;
	CDownload*	FindByMD5(const Hashes::Md5Hash& oMD5, BOOL bSharedOnly = FALSE) const;
	CDownload*	FindBySID(DWORD nSerID) const;
	DWORD		GetFreeSID();

	// Load all available .sd-files from Incomplete folder
	void		Load();
	// Load specified .sd-file
	CDownload*	Load(const CString& strPath);
	void		Save(BOOL bForce = TRUE);
	void		OnRun();
	BOOL		OnPush(const Hashes::Guid& oGUID, CConnection* pConnection);
	bool		OnQueryHits(const CQueryHit* pHits);
	// File was hashed and verified in the Library
	void		OnVerify(const CLibraryFile* pFile, TRISTATE bVerified);
	// Rename, delete or release downloading file.
	// pszTarget == 0 - delete file; pszTarget == 1 - release file.
	void		OnRename(LPCTSTR pszSource, LPCTSTR pszTarget);
	void		SetPerHostLimit(IN_ADDR* pAddress, DWORD nLimit);
	BOOL		IsSpaceAvailable(QWORD nVolume, int nPath = dlPathNull);

	void		UpdateAllows();
	bool		AllowMoreDownloads() const;
	bool		AllowMoreTransfers() const;
	bool		AllowMoreTransfers(IN_ADDR* pAdress) const;
	void		Remove(CDownload* pDownload);

	POSITION	GetIterator() const;
	POSITION	GetReverseIterator() const;
	CDownload*	GetNext(POSITION& pos) const;
	CDownload*	GetPrevious(POSITION& pos) const;
	BOOL		Check(CDownload* pDownload) const;
private:
	int			GetActiveTorrentCount() const;
//	DWORD		GetTransferCount() const;
	BOOL		Swap(CDownload* p1, CDownload* p2);
	DWORD		GetBandwidth() const;
	BOOL		OnDonkeyCallback(CEDClient* pClient, CDownloadSource* pExcept = NULL);
	void		LoadFromCompoundFiles();
	BOOL		LoadFromCompoundFile(LPCTSTR pszFile);
	BOOL		LoadFromTimePair();
	void		SerializeCompound(CArchive& ar);
	void		PurgePreviews();

	CDownloads(const CDownloads&);
	CDownloads& operator=(const CDownloads&);
};

extern CDownloads Downloads;
