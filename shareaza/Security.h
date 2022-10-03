//
// Security.h
//
// Copyright (c) Shareaza Development Team, 2002-2013.
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

#include "SecureRule.h"
#include "Network.h"


#define SECURITY_SER_VERSION	6
// History:
// 5 - extended security rule type (ryo-oh-ki)
// 6 - m_nIPv6, m_nIPv6PrefixLen in CSecureRule (ivan386)

enum
{
	banSession, ban5Mins, ban30Mins, ban2Hours, banWeek, banForever 
};

class CSecurity
{
// Construction
public:
	CSecurity();
	~CSecurity();

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
	CComplainMap				m_Complains;

	typedef std::set< DWORD > CAddressMap;
	CAddressMap		m_Cache; // miss cache

	// single IP blocking rules
	typedef std::map< DWORD, CSecureRule* > CAddressRuleMap;
	CAddressRuleMap	m_pIPRules;

	struct ipv6_compare {
		inline bool operator()(const IN6_ADDR& _Left, const IN6_ADDR& _Right) const throw()
		{
			return ( memcmp( &_Left, &_Right, sizeof( IN6_ADDR ) ) > 0 );
		}
	};

	typedef std::set< IN6_ADDR, ipv6_compare > CAddressIPv6Map;
	CAddressIPv6Map		m_CacheIPv6; // miss cache

	typedef std::map< IN6_ADDR, CSecureRule*, ipv6_compare > CAddressIPv6RuleMap;
	CAddressIPv6RuleMap	m_pIPv6Rules;

// Operations
public:
	POSITION		GetIterator() const;
	CSecureRule*	GetNext(POSITION& pos) const;
	INT_PTR			GetCount() const;
	BOOL			Check(CSecureRule* pRule) const;
	void			Add(CSecureRule* pRule);
	void			Remove(CSecureRule* pRule);
	void			MoveUp(CSecureRule* pRule);
	void			MoveDown(CSecureRule* pRule);

	void			Ban(const IN_ADDR* pAddress, int nBanLength, BOOL bMessage = TRUE, LPCTSTR szComment = NULL);
	void			Ban(const IN6_ADDR* pAddress, int nBanLength, BOOL bMessage = TRUE, LPCTSTR szComment = NULL);
	void			Ban(const CShareazaFile* pFile, int nBanLength, BOOL bMessage = TRUE, LPCTSTR szComment = NULL);

	bool			Complain(const IN_ADDR* pAddress, int nBanLength = ban5Mins, int nExpire = 10, int nCount = 3);
	void			Clear();
	BOOL			IsIgnoredCountry(const CString& strCountry);
	BOOL			IsDenied(const IN_ADDR* pAddress);
	BOOL			IsDenied(const IN6_ADDR* pAddress);
	BOOL			IsDenied(LPCTSTR pszContent);
	BOOL			IsDenied(const CShareazaFile* pFile);
	BOOL			IsDenied(const CQuerySearch* pQuery, const CString& strContent);

	inline  BOOL    IsDeniedComplexCheck(const IN_ADDR* pAddress, const IN_ADDR* pForAddress, BOOL bIncludeSelf = FALSE)
	{
		return Network.IsFirewalledAddress( pAddress, bIncludeSelf ) ||
			   Network.IsReserved( pAddress ) ||
			   ! Network.IsValidAddressFor( pForAddress , pAddress ) ||
		       IsIgnoredCountry( theApp.GetCountryCode( *pAddress ) ) || 
			   IsDenied( pAddress );
	}

	inline  BOOL    IsDeniedComplexCheck(const IN6_ADDR* pAddress, const IN6_ADDR* pForAddress, BOOL bIncludeSelf = FALSE)
	{
		return Network.IsFirewalledAddress( pAddress, bIncludeSelf ) ||
			   Network.IsReserved( pAddress ) ||
			   ! Network.IsValidAddressFor( pForAddress , pAddress ) ||
		       IsIgnoredCountry( theApp.GetCountryCode( *pAddress ) ) || 
			   IsDenied( pAddress );
	}

	void			Expire();
	BOOL			Load();
	BOOL			Save();
	BOOL			Import(LPCTSTR pszFile);

	// Creates new CLiveList object filled by all security rules
	CLiveList*		GetList() const;

	// Checks the user agent to see if it's a GPL breaker, or other trouble-maker
	// We don't ban them, but also don't offer leaf slots to them.
	BOOL			IsClientBad(const CString& sUserAgent) const;

	// Checks the user agent to see if it's a leecher client, or other banned client
	// Test new releases, and remove block if/when they are fixed.
	BOOL			IsClientBanned(const CString& sUserAgent);

	// Check the other computer's software title against our list of programs
	// not to talk to
	BOOL			IsAgentBlocked(const CString& sUserAgent) const;
	
	// Check the evil's G1/G2 vendor code
	BOOL			IsVendorBlocked(const CString& sVendor) const;

protected:
	CSecureRule*	GetGUID(const GUID& oGUID) const;
	CXMLElement*	ToXML(BOOL bRules = TRUE);
	BOOL			FromXML(CXMLElement* pXML);
	void			Serialize(CArchive& ar);
};

// An adult filter class, used in searches, chat, etc
class CAdultFilter
{
// Construction
public:
	CAdultFilter();
	~CAdultFilter();

// Attributes
private:
	LPTSTR		m_pszBlockedWords;			// Definitely adult content
	LPTSTR		m_pszDubiousWords;			// Possibly adult content
	LPTSTR		m_pszChildWords;			// Words related to child pornography

// Operations
public:
	void		Load();
	BOOL		IsHitAdult(LPCTSTR) const;		// Does this search result have adult content?
	BOOL		IsSearchFiltered(LPCTSTR) const;// Check if search is filtered
	BOOL		IsChatFiltered(LPCTSTR) const;	// Check filter for chat
	BOOL		Censor(CString& sText) const;	// Censor (remove) bad words from a string
	BOOL		IsChildPornography(LPCTSTR) const;
private:
	BOOL		IsFiltered(LPCTSTR) const;
};

// A message filter class for chat messages. (Spam protection)
class CMessageFilter
{
// Construction
public:
	CMessageFilter();
	~CMessageFilter();

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
