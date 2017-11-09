//
// SecureRule.h
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

class CLiveList;
class CShareazaFile;
class CQuerySearch;
class CXMLElement;


class CSecureRule
{
public:
	CSecureRule(BOOL bCreate = TRUE);
	CSecureRule(const CSecureRule& pRule);
	CSecureRule& operator=(const CSecureRule& pRule);
	~CSecureRule();

	typedef enum { srAddress, srContentAny, srContentAll, srContentRegExp } RuleType;
	enum { srNull, srAccept, srDeny };
	enum { srIndefinite = 0, srSession = 1 };

	RuleType	m_nType;
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

	void			Remove();
	void			Reset();
	void			MaskFix();
	BOOL			IsExpired(DWORD nNow, BOOL bSession = FALSE) const;
	BOOL			Match(const IN_ADDR* pAddress) const;
	BOOL			Match(LPCTSTR pszContent) const;
	BOOL			Match(const CShareazaFile* pFile) const;
	BOOL			Match(const CQuerySearch* pQuery, const CString& strContent) const;
	void			SetContentWords(const CString& strContent);
	CString			GetContentWords() const;
	void			Serialize(CArchive& ar, int nVersion);
	CXMLElement*	ToXML();
	BOOL			FromXML(CXMLElement* pXML);
	CString			ToGnucleusString() const;
	BOOL			FromGnucleusString(CString& str);

	// Adds new item to CLiveList object
	void			ToList(CLiveList* pLiveList, int nCount, DWORD tNow) const;
};
