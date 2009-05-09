//
// Download.h
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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

#define DOWNLOAD_SER_VERSION	41
// History:
// 33 - added m_sSearchKeyword to CDownloadBase (CyberBob)
// 34 - added m_bSeeding and m_sServingFileName to CDownloadWithTorrent (Rolandas)
// 35 - added m_sCountry to CDownloadSource (dcat)
// 36 - nothing (Rolandas)
// 37 - added m_oBTH to CDownloadBase, m_bBTH and m_bMD5 to CDownloadSource (Ryo-oh-ki)
// 38 - added m_sCountryName to CDownloadSource (dcat)
// 39 - added m_bClientExtended to CDownloadSource (Ryo-oh-ki)
// 40 - added virtual fragmented file (Ryo-oh-ki)
// 41 - added m_sName to CFragmentedFile (Ryo-oh-ki)

#include "DownloadWithExtras.h"
#include "Settings.h"

class CDownload : public CDownloadWithExtras
{
// Construction
public:
	CDownload();
	virtual ~CDownload();

// Attributes
public:
	DWORD		m_nSerID;
	BOOL		m_bExpanded;
	BOOL		m_bSelected;
	DWORD		m_tCompleted;
	int			m_nRunCookie;
	int			m_nSaveCookie;
	int			m_nGroupCookie;
private:
	BOOL		m_bTempPaused;
	BOOL		m_bPaused;
	BOOL		m_bBoosted;
	BOOL		m_bShared;
	BOOL		m_bComplete;
	DWORD		m_tSaved;
	DWORD		m_tBegan;		// The time when this download began trying to download (Started
								// searching, etc). 0 means has not tried this session.
	BOOL		m_bDownloading;	// This is used to store if a download is downloading. (Performance tweak)
								// You should count the transfers if you need a 100% current answer.
// Operations
public:
	void        	Pause(BOOL bRealPause = TRUE);
	void        	Resume();
	void        	Remove(bool bDelete = false);
	void        	Boost();
	void        	Share(BOOL bShared);
	BOOL        	Rename(LPCTSTR pszName);
	void        	SetStartTimer();
	BOOL			Launch(int nIndex, CSingleLock* pLock, BOOL bForceOriginal);
	BOOL			Enqueue(int nIndex, CSingleLock* pLock);

	inline DWORD GetStartTimer() const
	{
		return( m_tBegan );
	}

	// Has the download actually downloaded anything?
	inline BOOL IsStarted() const
	{
		return ( GetVolumeComplete() > 0 );
	}

	inline BOOL IsPaused( BOOL bRealState = FALSE ) const
	{
		return ( bRealState ? m_bPaused : m_bTempPaused );
	}

	// Is the download receiving data?
	inline BOOL IsDownloading() const
	{
		return m_bDownloading;
	}

	inline BOOL IsCompleted() const
	{
		return m_bComplete;
	}

	inline BOOL IsBoosted() const
	{
		return m_bBoosted;
	}

	inline BOOL IsShared() const
	{
		return ! IsPaused(TRUE) ? m_bShared ||
			( Settings.BitTorrent.EnableToday && IsTorrent() && ( IsSeeding() || IsStarted() ) ) ||
			( Settings.eDonkey.EnableToday && m_oED2K ) : m_bShared;
	}

	// Is the download currently trying to download?
	inline BOOL IsTrying() const
	{
		return ( m_tBegan != 0 );
	}

	BOOL			Load(LPCTSTR pszPath);
	BOOL			Save(BOOL bFlush = FALSE);
	virtual void	Serialize(CArchive& ar, int nVersion);
	void			OnRun();
	void			ForceComplete();

private:
	void        	StopTrying();
	void			OnTaskComplete(CDownloadTask* pTask);
	void			OnDownloaded();
	void			OnMoved(CDownloadTask* pTask);
	void			SerializeOld(CArchive& ar, int nVersion);

	friend class CDownloadTask;			// m_pTask && OnTaskComplete
	friend class CDownloadTransfer;		// GetVerifyLength
	friend class CDownloadWithTorrent;	// m_bComplete
	friend class CDownloadsWnd;			// m_pTask
	friend class CDownloads;			// m_bComplete for Load()
};
