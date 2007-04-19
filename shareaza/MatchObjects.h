//
// MatchObjects.h
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

#if !defined(AFX_MATCHOBJECTS_H__3D4FE6DE_027F_44B9_A70E_A229D970D7A8__INCLUDED_)
#define AFX_MATCHOBJECTS_H__3D4FE6DE_027F_44B9_A70E_A229D970D7A8__INCLUDED_

#pragma once

#include "ShareazaFile.h"

class CSchema;
class CSchemaMember;
class CQuerySearch;
class CQueryHit;
class CMatchFile;
class CResultFilters;

typedef struct
{
	BOOL	bHadSHA1;
	BOOL	bHadTiger;
	BOOL	bHadED2K;
	int		nHadCount;
	int		nHadFiltered;
	BOOL	bHad[3];
} FILESTATS;

class CMatchList
{
// Construction
public:
	CMatchList();
	virtual ~CMatchList();
	
// Attributes
public:
	CMutex		m_pSection;
public:
	CString			m_sFilter;
	BOOL			m_bFilterBusy;
	BOOL			m_bFilterPush;
	BOOL			m_bFilterUnstable;
	BOOL			m_bFilterReject;
	BOOL			m_bFilterLocal;
	BOOL			m_bFilterBogus;
	BOOL			m_bFilterDRM;
	BOOL			m_bFilterAdult;
	BOOL			m_bFilterSuspicious;
	QWORD			m_nFilterMinSize;
	QWORD			m_nFilterMaxSize;
	DWORD			m_nFilterSources;
	int				m_nSortColumn;
	BOOL			m_bSortDir;
	CSchema*		m_pSchema;
	BOOL			m_bNew;
	CResultFilters*	m_pResultFilters;
public:
	CMatchFile**	m_pFiles;
	DWORD			m_nFiles;
	DWORD			m_nItems;
	DWORD			m_nFilteredFiles;
	DWORD			m_nFilteredHits;
	DWORD			m_nGnutellaHits;
	DWORD			m_nED2KHits;
	BOOL			m_bUpdated;
	DWORD			m_nUpdateMin;
	DWORD			m_nUpdateMax;
	CList< CMatchFile* > m_pSelectedFiles;
	CList< CQueryHit* > m_pSelectedHits;
protected:
	DWORD			m_nBuffer;
	CMatchFile**	m_pSizeMap;
	CMatchFile**	m_pMapSHA1;
	CMatchFile**	m_pMapTiger;
	CMatchFile**	m_pMapED2K;
	LPTSTR			m_pszFilter;
	CSchemaMember**	m_pColumns;
	int				m_nColumns;

	static enum findType
	{
		fSHA1	= 0,
		fTiger	= 1,
		fED2K	= 2,
		fSize	= 3,
	};
	
// Operations
public:
	void		AddHits(CQueryHit* pHits, CQuerySearch* pFilter = NULL, BOOL bRequire = FALSE);
	DWORD		FileToItem(CMatchFile* pFile);
	void		Clear();
	BOOL		Select(CMatchFile* pFile, CQueryHit* pHit, BOOL bSelected = TRUE);
	CMatchFile*	GetSelectedFile(BOOL bFromHit = FALSE) const;
	CQueryHit*	GetSelectedHit() const;
	INT_PTR		GetSelectedCount() const;
	BOOL		ClearSelection();
	void		Filter();
	void		SelectSchema(CSchema* pSchema, CList< CSchemaMember* >* pColumns);
	void		SetSortColumn(int nColumn = -1, BOOL bDirection = FALSE);
	void		UpdateRange(DWORD nMin = 0, DWORD nMax = 0xFFFFFFFF);
	void		ClearUpdated();
	void		ClearNew();
	void		Serialize(CArchive& ar);
protected:
	CMatchFile* FindFileAndAddHit(CQueryHit* pHit, findType nFindFlag, FILESTATS FAR* Stats);
	void		InsertSorted(CMatchFile* pFile);
	BOOL		FilterHit(CQueryHit* pHit);
	
	friend class CMatchFile;

};


class CMatchFile : public CShareazaFile
{
public:
	CMatchFile(CMatchList* pList, CQueryHit* pHit = NULL);
	virtual ~CMatchFile();
	
public:
	CMatchList*	m_pList;
	CQueryHit*	m_pHits;
	CQueryHit*	m_pBest;
	DWORD		m_nTotal;
	DWORD		m_nFiltered;
	DWORD		m_nSources;
	CMatchFile*	m_pNextSize;
	CMatchFile*	m_pNextSHA1;
	CMatchFile*	m_pNextTiger;
	CMatchFile*	m_pNextED2K;
	CString		m_sSize;
	TRISTATE	m_bBusy;
	TRISTATE	m_bPush;
	TRISTATE	m_bStable;
	BOOL		m_bPreview;
	DWORD		m_nSpeed;
	CString		m_sSpeed;
	int			m_nRating;				// Total value of all ratings
	int			m_nRated;				// Number of ratings recieved
	BOOL		m_bDRM;					// Appears to have DRM
	BOOL		m_bSuspicious;			// Appears to be a suspicious file (small exe, vbs, etc)
	BOOL		m_bCollection;			// Appears to be a collection
	BOOL		m_bTorrent;				// Appears to be a torrent
	BOOL		m_bExpanded;
	BOOL		m_bSelected;
	BOOL		m_bDownload;
	BOOL		m_bNew;
	BOOL		m_bOneValid;
	int			m_nShellIndex;
	CString*	m_pColumns;
	int			m_nColumns;
	BYTE*		m_pPreview;
	DWORD		m_nPreview;

	BOOL		Add(CQueryHit* pHit, BOOL bForce = FALSE);
	BOOL		Check(CQueryHit* pHit) const;
	BOOL		Expand(BOOL bExpand = TRUE);
	inline int	Compare(CMatchFile* pFile) const;
	CString		GetURN() const;
	void		Serialize(CArchive& ar, int nVersion);
protected:
	inline DWORD	Filter();
	inline void		Added(CQueryHit* pHit);
	inline void		ClearNew();
public:
	
	inline DWORD GetFilteredCount()
	{
		if ( m_pList->m_bFilterLocal && GetLibraryStatus() == TS_FALSE ) return 0;
		if ( m_pList->m_bFilterDRM && m_bDRM ) return 0;
		if ( m_pList->m_bFilterSuspicious && m_bSuspicious ) return 0;
		if ( m_nSources < m_pList->m_nFilterSources ) return 0;
		if ( m_pBest == NULL ) return 0;

		return m_nFiltered;
	}
	
	inline DWORD GetItemCount()
	{
		if ( m_pList->m_bFilterLocal && GetLibraryStatus() == TS_FALSE )return 0;
		if ( m_pList->m_bFilterDRM && m_bDRM ) return 0;
		if ( m_pList->m_bFilterSuspicious && m_bSuspicious ) return 0;
		if ( m_nSources < m_pList->m_nFilterSources ) return 0;
		if ( m_pBest == NULL ) return 0;

		if ( m_nFiltered == 1 || ! m_bExpanded )
			return 1;
		else
			return m_nFiltered + 1;
	}
	
/*	inline int GetRating() const
	{
		int nRating = 0;
		
		if ( m_bPush != TS_TRUE ) nRating += 4;
		if ( m_bBusy != TS_TRUE ) nRating += 2;
		if ( m_bStable == TS_TRUE ) nRating ++;

		return nRating;
	}*/
	

	// Refresh file status (name, uri, etc.) in accord with Hits list
	void		RefreshStatus();
	
	// Is this file known (i.e. exist in Library)?
	// TS_UNKNOWN	- Not
	// TS_FALSE		- Yes
	// TS_TRUE		- Yes, Ghost
	TRISTATE	GetLibraryStatus();

	friend class CMatchList;

protected:
	TRISTATE	m_bExisting;
};


#endif // !defined(AFX_MATCHOBJECTS_H__3D4FE6DE_027F_44B9_A70E_A229D970D7A8__INCLUDED_)
