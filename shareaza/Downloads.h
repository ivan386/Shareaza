//
// Downloads.h
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

#if !defined(AFX_DOWNLOADS_H__0C075423_D022_4530_8B45_6B7EF79712CB__INCLUDED_)
#define AFX_DOWNLOADS_H__0C075423_D022_4530_8B45_6B7EF79712CB__INCLUDED_

#pragma once

class CDownload;
class CDownloadSource;
class CConnection;
class CQueryHit;
class CMatchFile;
class CBuffer;
class CShareazaURL;
class CEDClient;


class CDownloads  
{
// Construction
public:
	CDownloads();
	virtual ~CDownloads();
	
// Attributes
public:
	DWORD			m_nLimitGeneric;
	DWORD			m_nLimitDonkey;
	DWORD			m_nTransfers;
	DWORD			m_nBandwidth;
	DWORD			m_nValidation;
	BOOL			m_bAllowMoreDownloads;
	BOOL			m_bAllowMoreTransfers;
	BOOL			m_bClosing;
	DWORD			m_tLastConnect;
protected:
	CPtrList		m_pList;
	CMapPtrToPtr	m_pHostLimits;
	int				m_nRunCookie;
	
// Operations
public:
	CDownload*	Add();
	CDownload*	Add(CQueryHit* pHit);
	CDownload*	Add(CMatchFile* pFile);
	CDownload*	Add(CShareazaURL* pURL);
	void		PauseAll();
	void		ClearCompleted();
	void		ClearPaused();
	void		Clear(BOOL bClosing = FALSE);
	void		CloseTransfers();
public:
	int			GetSeedCount() const;
	int			GetActiveTorrentCount() const;
	int			GetCount(BOOL bActiveOnly = FALSE) const;
	int			GetTransferCount() const;
	BOOL		Check(CDownloadSource* pSource) const;
	BOOL		CheckActive(CDownload* pDownload, int nScope) const;
	BOOL		Move(CDownload* pDownload, int nDelta);
	BOOL		Reorder(CDownload* pDownload, CDownload* pBefore);
	CDownload*	FindByURN(LPCTSTR pszURN, BOOL bSharedOnly = FALSE) const;
	CDownload*	FindBySHA1(const SHA1* pHash, BOOL bSharedOnly = FALSE) const;
	CDownload*	FindByTiger(const TIGEROOT* pHash, BOOL bSharedOnly = FALSE) const;
	CDownload*	FindByED2K(const MD4* pED2K, BOOL bSharedOnly = FALSE) const;
	CDownload*	FindByBTH(const SHA1* pHash, BOOL bSharedOnly = FALSE) const;
	CDownload*	FindBySID(DWORD nSerID) const;
	DWORD		GetFreeSID();
	DWORD		GetBandwidth() const;
public:
	void		Load();
	void		Save(BOOL bForce = TRUE);
	void		OnRun();
	BOOL		OnPush(GGUID* pGUID, CConnection* pConnection);
	BOOL		OnDonkeyCallback(CEDClient* pClient, CDownloadSource* pExcept = NULL);
	void		OnQueryHits(CQueryHit* pHits);
	void		OnVerify(LPCTSTR pszPath, BOOL bVerified);
	void		SetPerHostLimit(IN_ADDR* pAddress, int nLimit);
	BOOL		IsSpaceAvailable(QWORD nVolume);
protected:
	void		UpdateAllows(BOOL bNew);
	BOOL		AllowMoreDownloads() const;
	BOOL		AllowMoreTransfers(IN_ADDR* pAdress = NULL) const;
	void		Remove(CDownload* pDownload);
	void		LoadFromCompoundFiles();
	BOOL		LoadFromCompoundFile(LPCTSTR pszFile);
	BOOL		LoadFromTimePair();
	void		SerializeCompound(CArchive& ar);
	void		PurgeDeletes();
	void		PurgePreviews();

// Inlines
public:
	inline POSITION CDownloads::GetIterator() const
	{
		return m_pList.GetHeadPosition();
	}
	
	inline POSITION CDownloads::GetReverseIterator() const
	{
		return m_pList.GetTailPosition();
	}
	
	inline CDownload* CDownloads::GetNext(POSITION& pos) const
	{
		return (CDownload*)m_pList.GetNext( pos );
	}
	
	inline CDownload* CDownloads::GetPrevious(POSITION& pos) const
	{
		return (CDownload*)m_pList.GetPrev( pos );
	}
	
	inline BOOL CDownloads::Check(CDownload* pDownload) const
	{
		return m_pList.Find( pDownload ) != NULL;
	}

	friend class CDownload;
	friend class CDownloadBase;
	friend class CDownloadWithTransfers;
	friend class CDownloadSource;
};

extern CDownloads Downloads;


#endif // !defined(AFX_DOWNLOADS_H__0C075423_D022_4530_8B45_6B7EF79712CB__INCLUDED_)
