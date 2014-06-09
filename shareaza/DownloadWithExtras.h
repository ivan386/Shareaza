//
// DownloadWithExtras.h
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

#include "DownloadWithSearch.h"

class CDownloadMonitorDlg;
class CFilePreviewDlg;


// CDownloadReview stores a review of a download. It can be either a G2 review, copied from the
// search, or an ed2k review recieved during download.
class CDownloadReview
{
// Construction
public:
	CDownloadReview();
	CDownloadReview(in_addr *pIP, int nUserPicture, int nRating, LPCTSTR pszUserName, LPCTSTR pszComment);
	~CDownloadReview();

// Attributes
public:
	in_addr				m_pUserIP;			// To prevent duplicate reviews
	int					m_nUserPicture;		// Picture to display. 2 = G2, 3 = ED2K
	CString				m_sUserName;		// User who made comments
	int					m_nFileRating;		// 0 = Unrated, 1 = Fake, 1-6 = Num Stars
	CString				m_sFileComments;	// The review/comments

	CDownloadReview*	m_pNext;			// Next review in list (or NULL for the last)
	CDownloadReview*	m_pPrev;			// PRevious review in list (or NULL for the first)

// Operations
	void Serialize(CArchive& ar, int nVersion);

	friend class		CDownloadWithExtras;
};

class CDownloadWithExtras : public CDownloadWithSearch
{
// Construction
protected:
	CDownloadWithExtras();
	virtual ~CDownloadWithExtras();
	
// Attributes
private:
	CList< CString >		m_pPreviews;
	CDownloadMonitorDlg*	m_pMonitorWnd;
	CFilePreviewDlg*		m_pPreviewWnd;

protected:
	CDownloadReview*	m_pReviewFirst;
	CDownloadReview*	m_pReviewLast;
	DWORD				m_nReviewCount;

public:
	BOOL				m_bRemotePreviewCapable;
	BOOL				m_bWaitingPreview;	// The remote preview request was sent

// Operations
public:
	BOOL		PreviewFile(DWORD nIndex, CSingleLock* pLock);
	BOOL		GotPreview() const;
	BOOL		IsPreviewVisible() const;
	BOOL		CanPreview(DWORD nIndex);
	void		ShowMonitor();
	void		AddPreviewName(LPCTSTR pszFile);
	void		DeletePreviews();
	BOOL		AddReview(IN_ADDR* pIP, int nUserPicture, int nRating, LPCTSTR pszUserName, LPCTSTR pszComment);
	BOOL		AddReview(CDownloadReview* pReview);
	void		DeleteReviews();
	void		DeleteReview(CDownloadReview* pReview);
	int			GetReviewCount() const { return m_nReviewCount; }
	int			GetReviewAverage() const;
protected:
	virtual void Serialize(CArchive& ar, int nVersion);
public:
	CDownloadReview* GetFirstReview() const { return m_pReviewFirst; }
private:
	CDownloadReview* FindReview(IN_ADDR* pIP) const;
	CDownloadReview* FindReview(LPCTSTR pszUserName) const;
	CDownloadReview* FindReview(int nRating, LPCTSTR pszName, LPCTSTR pszComment) const;

public:
	BOOL		IsMonitorVisible() const;

	friend class CDownloadMonitorDlg;
	friend class CFilePreviewDlg;

};
