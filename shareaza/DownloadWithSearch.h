//
// DownloadWithSearch.h
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

#include "DownloadWithTiger.h"
#include "ManagedSearch.h"

class CManagedSearch;


class CDownloadWithSearch : public CDownloadWithTiger
{
public:
	BOOL			m_bUpdateSearch;	// Search must be updated
	DWORD			m_tLastED2KGlobal;	// Time the last ed2k UDP GetSources was done on this download
	DWORD			m_tLastED2KLocal;	// Time the last ed2k TCP GetSources was done on this download

	virtual BOOL	FindMoreSources();
	BOOL			IsSearching() const;

protected:
	CDownloadWithSearch();
	virtual ~CDownloadWithSearch();

	BOOL			FindSourcesAllowed(DWORD tNow) const;
	void			RunSearch(DWORD tNow);
	void			StopSearch();

private:
	CSearchPtr		m_pSearch;			// Managed search object
	DWORD			m_tSearchTime;		// Timer for manual search
	DWORD			m_tSearchCheck;		// Limit auto searches

	void			StartManualSearch();
	void			StartAutomaticSearch();
	BOOL			CanSearch() const;
	void			PrepareSearch();
};
