//
// Security.h
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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

#include "QuerySearch.h"

class CSecureRule;
class CXMLElement;

enum
{
	banSession, ban5Mins, ban30Mins, ban2Hours, banWeek, banForever 
};

class CSecurity
{
// Construction
public:
	CSecurity();
	virtual ~CSecurity();

// Attributes
public:
	mutable CCriticalSection	m_pSection;
	BOOL						m_bDenyPolicy;
	static LPCTSTR				xmlns;

protected:
	typedef struct
	{
		DWORD	m_nExpire;
		BYTE	m_nScore;
	} CComplain;
	typedef CMap< DWORD, DWORD, CComplain*, CComplain* > CComplainMap;

	CList< CSecureRule* >		m_pRules;
	CList< CSecureRule* >		m_pRegExpRules;
	CComplainMap				m_Complains;

// Operations
public:
	POSITION		GetIterator() const;
	CSecureRule*	GetNext(POSITION& pos) const;
	INT_PTR			GetCount() const;
	POSITION		GetRegExpIterator() const;
	CSecureRule*	GetNextRegExp(POSITION& pos) const;
	INT_PTR			GetRegExpCount() const;
	BOOL			Check(CSecureRule* pRule) const;
	void			Add(CSecureRule* pRule);
	void			Remove(CSecureRule* pRule);
	void			MoveUp(CSecureRule* pRule);
	void			MoveDown(CSecureRule* pRule);

	inline void		Ban(const IN_ADDR* pAddress, int nBanLength, BOOL bMessage = TRUE)
	{
		BanHelper( pAddress, NULL, nBanLength, bMessage );
	}

	inline void		Ban(const CShareazaFile* pFile, int nBanLength, BOOL bMessage = TRUE)
	{
		BanHelper( NULL, pFile, nBanLength, bMessage );
	}

	bool			Complain(const IN_ADDR* pAddress, int nBanLength = ban5Mins, int nExpire = 30, int nCount = 3);
	void			Clear();
	BOOL			IsDenied(const IN_ADDR* pAddress);
	BOOL			IsDenied(LPCTSTR pszContent);
	BOOL			IsDenied(const CShareazaFile* pFile);
	BOOL			IsDenied(CQuerySearch::const_iterator itStart, 
							 CQuerySearch::const_iterator itEnd, LPCTSTR pszContent);
	void			Expire();
	BOOL			Load();
	BOOL			Save();
	BOOL			Import(LPCTSTR pszFile);

	// Checks the user agent to see if it's a GPL breaker, or other trouble-maker
	// We don't ban them, but also don't offer leaf slots to them.
	BOOL			IsClientBad(const CString& sUserAgent);

	// Checks the user agent to see if it's a leecher client, or other banned client
	// Test new releases, and remove block if/when they are fixed.
	BOOL			IsClientBanned(const CString& sUserAgent);

protected:
	void			BanHelper(const IN_ADDR* pAddress, const CShareazaFile* pFile, int nBanLength, BOOL bMessage);
	CSecureRule*	GetGUID(const GUID& oGUID) const;
	CXMLElement*	ToXML(BOOL bRules = TRUE);
	BOOL			FromXML(CXMLElement* pXML);
	void			Serialize(CArchive& ar);
};

class CSecureRule
{
// Construction
public:
	CSecureRule(BOOL bCreate = TRUE);
	CSecureRule(const CSecureRule& pRule);
	CSecureRule& operator=(const CSecureRule& pRule);
	virtual ~CSecureRule();

// Attributes
public:
	int			m_nType;
	BYTE		m_nAction;
	CString		m_sComment;
	GUID		m_pGUID;
	DWORD		m_nExpire;
	DWORD		m_nToday;
	DWORD		m_nEver;
	BYTE		m_nIP[4];
	BYTE		m_nMask[4];
	TCHAR*		m_pContent;
	DWORD		m_nContentLength;

	enum { srAddress, srContent };
	enum { srNull, srAccept, srDeny };
	enum { srIndefinite = 0, srSession = 1 };

// Operations
public:
	void	Remove();
	void	Reset();
	void	MaskFix();
	BOOL	IsExpired(DWORD nNow, BOOL bSession = FALSE) const;
	BOOL	Match(const IN_ADDR* pAddress) const;
	BOOL	Match(LPCTSTR pszContent) const;
	BOOL	Match(const CShareazaFile* pFile) const;
	BOOL	Match(CQuerySearch::const_iterator itStart, 
				  CQuerySearch::const_iterator itEnd, LPCTSTR pszContent) const;
	void	SetContentWords(const CString& strContent);
	CString	GetRegExpFilter(CQuerySearch::const_iterator itStart, 
							CQuerySearch::const_iterator itEnd) const;
	CString	GetContentWords();

public:
	void			Serialize(CArchive& ar, int nVersion);
	CXMLElement*	ToXML();
	BOOL			FromXML(CXMLElement* pXML);
	CString			ToGnucleusString();
	BOOL			FromGnucleusString(CString& str);

};

// An adult filter class, used in searches, chat, etc
class CAdultFilter
{
// Construction
public:
	CAdultFilter();
	virtual ~CAdultFilter();

// Attributes
private:
	LPTSTR		m_pszBlockedWords;			// Definitely adult content
	LPTSTR		m_pszDubiousWords;			// Possibly adult content
	LPTSTR		m_pszChildWords;			// Words related to child ponography

// Operations
public:
	void		Load();
	BOOL		IsHitAdult(LPCTSTR);		// Does this search result have adult content?
	BOOL		IsSearchFiltered(LPCTSTR);	// Check if search is filtered
	BOOL		IsChatFiltered(LPCTSTR);	// Check filter for chat
	BOOL		Censor(TCHAR*);				// Censor (remove) bad words from a string
	BOOL		IsChildPornography(LPCTSTR);
private:
	BOOL		IsFiltered(LPCTSTR);
};

// A message filter class for chat messages. (Spam protection)
class CMessageFilter
{
// Construction
public:
	CMessageFilter();
	virtual ~CMessageFilter();

// Attributes
private:
	LPTSTR		m_pszED2KSpam;				// Known ED2K spam phrases
	LPTSTR		m_pszFilteredPhrases;		// Known spam phrases

// Operations
public:
	void		Load();
	BOOL		IsED2KSpam( LPCTSTR );		// ED2K message spam filter (ED2K only, always on)
	BOOL		IsFiltered( LPCTSTR );		// Chat message spam filter
};

extern CMessageFilter MessageFilter;
extern CAdultFilter AdultFilter;
extern CSecurity Security;
