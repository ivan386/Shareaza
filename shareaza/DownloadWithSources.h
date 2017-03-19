//
// DownloadWithSources.h
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

#include "DownloadBase.h"

class CDownloadSource;
class CQueryHit;
class CMatchFile;
class CShareazaURL;
class CXMLElement;

class CFailedSource
{
public:
	CFailedSource(LPCTSTR pszURL, bool bLocal=true, bool bOffline=false)
		: m_nTimeAdded( GetTickCount() )
		, m_nPositiveVotes( 0 )
		, m_nNegativeVotes( 0 )
		, m_sURL( pszURL )
		, m_bLocal( bLocal )
		, m_bOffline( bOffline )
	{
	}

	DWORD	m_nTimeAdded;
	int		m_nPositiveVotes;
	int		m_nNegativeVotes;
	CString	m_sURL;
	bool	m_bLocal;
	bool	m_bOffline;
};

class CDownloadWithSources : public CDownloadBase
{
protected:
	CDownloadWithSources();
	virtual ~CDownloadWithSources();
	
private:
	CList< CDownloadSource* >	m_pSources;			// Download sources
	CList< CFailedSource* >		m_pFailedSources;	// Failed source with a timestamp when added
	int					m_nG1SourceCount;
	int					m_nG2SourceCount;
	int					m_nEdSourceCount;
	int					m_nHTTPSourceCount;
	int					m_nBTSourceCount;
	int					m_nFTPSourceCount;
	int					m_nDCSourceCount;

public:
	CXMLElement*		m_pXML;

	CString				GetSourceURLs(CList< CString >* pState = NULL, int nMaximum = 0, PROTOCOLID nProtocol = PROTOCOL_NULL, CDownloadSource* pExcept = NULL) const;
	CString				GetTopFailedSources(int nMaximum, PROTOCOLID nProtocol);
	DWORD				GetEffectiveSourceCount() const;
	DWORD				GetSourceCount(BOOL bNoPush = FALSE, BOOL bSane = FALSE) const;
	DWORD				GetBTSourceCount(BOOL bNoPush = FALSE) const;
	DWORD				GetED2KCompleteSourceCount() const;
	BOOL				CheckSource(CDownloadSource* pSource) const;
	void				AddFailedSource(const CDownloadSource* pSource, bool bLocal = true, bool bOffline = false);
	void				AddFailedSource(LPCTSTR pszUrl, bool bLocal = true, bool bOffline = false);
	CFailedSource*		LookupFailedSource(LPCTSTR pszUrl, bool bReliable = false);
	void				ExpireFailedSources();
	void				ClearSources();
	void				ClearFailedSources();
	void				MergeMetadata(const CXMLElement* pXML);
	BOOL				AddSourceHit(const CQueryHit* pHit, BOOL bForce = FALSE);
	BOOL				AddSourceHit(const CMatchFile* pMatchFile, BOOL bForce = FALSE);
	BOOL				AddSourceHit(const CShareazaURL& oURL, BOOL bForce = FALSE, int nRedirectionCount = 0);
	BOOL				AddSourceED2K(DWORD nClientID, WORD nClientPort, DWORD nServerIP, WORD nServerPort, const Hashes::Guid& oGUID);
    BOOL				AddSourceBT(const Hashes::BtGuid& oGUID, const IN_ADDR* pAddress, WORD nPort, BOOL bIgnoreLocalIP = FALSE);
	BOOL				AddSourceURL(LPCTSTR pszURL, FILETIME* pLastSeen = NULL, int nRedirectionCount = 0, BOOL bFailed = FALSE, BOOL bForce = FALSE);
	int					AddSourceURLs(LPCTSTR pszURLs, BOOL bFailed = FALSE);
	// Remove source from list, add it to failed sources if bBan == TRUE
	void				RemoveSource(CDownloadSource* pSource, BOOL bBan);

	virtual BOOL		OnQueryHits(const CQueryHit* pHits);
	virtual void		Serialize(CArchive& ar, int nVersion /* DOWNLOAD_SER_VERSION */);
	COLORREF			GetSourceColour();
	// Get source iterator (first source position)
	POSITION			GetIterator() const;
	// Get next source
	CDownloadSource*	GetNext(POSITION& rPosition) const;
	// Get source count
	INT_PTR				GetCount() const;

	bool				HasMetadata() const;

protected:
	BOOL				AddSource(const CShareazaFile* pHit, BOOL bForce);
	void				RemoveOverlappingSources(QWORD nOffset, QWORD nLength);
	BOOL				AddSourceInternal(CDownloadSource* pSource);
	void				SortSource(CDownloadSource* pSource, BOOL bTop);
	void				SortSource(CDownloadSource* pSource);
	// Add new source to list, updating counters
	void				InternalAdd(CDownloadSource* pSource);
	// Remove existing source from list, updating counters
	void				InternalRemove(CDownloadSource* pSource);

	void				VoteSource(LPCTSTR pszUrl, bool bPositively);
};
