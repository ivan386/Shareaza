//
// QuerySearch.h
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

#if !defined(AFX_QUERYSEARCH_H__2141B926_3F6B_4A5D_9FBD_C67FD0A5C46C__INCLUDED_)
#define AFX_QUERYSEARCH_H__2141B926_3F6B_4A5D_9FBD_C67FD0A5C46C__INCLUDED_

#pragma once

class CPacket;
class CSchema;
class CXMLElement;
class CSearchWnd;
class CG1Packet;
class CG2Packet;
class CEDPacket;


class CQuerySearch  
{
// Construction
public:
	CQuerySearch(BOOL bGUID = TRUE);
	CQuerySearch(CQuerySearch* pCopy);
	virtual ~CQuerySearch();
	
// Attributes
public:
	GGUID			m_pGUID;
public:
	CString			m_sSearch;
	CSchema*		m_pSchema;
	CXMLElement*	m_pXML;
	QWORD			m_nMinSize;
	QWORD			m_nMaxSize;
public:
	BOOL			m_bSHA1;
	SHA1			m_pSHA1;
	BOOL			m_bTiger;
	TIGEROOT		m_pTiger;
	BOOL			m_bED2K;
	MD4				m_pED2K;
	BOOL			m_bBTH;
	SHA1			m_pBTH;
public:
	BOOL			m_bWantURL;
	BOOL			m_bWantDN;
	BOOL			m_bWantXML;
	BOOL			m_bWantCOM;
	BOOL			m_bWantPFS;
	BOOL			m_bAndG1;
public:
	BOOL			m_bUDP;
	SOCKADDR_IN		m_pEndpoint;
	DWORD			m_nKey;
	BOOL			m_bFirewall;
public:
	DWORD			m_nWords;
	LPCTSTR*		m_pWordPtr;
	DWORD*			m_pWordLen;

// Packet Operations
public:
	CG1Packet*				ToG1Packet();
	CG2Packet*				ToG2Packet(SOCKADDR_IN* pUDP, DWORD nKey);
	CEDPacket*				ToEDPacket(BOOL bUDP, DWORD nServerFlags = 0);
	static CQuerySearch*	FromPacket(CPacket* pPacket, SOCKADDR_IN* pEndpoint = NULL);
protected:
	BOOL					ReadG1Packet(CPacket* pPacket);
	BOOL					ReadG2Packet(CG2Packet* pPacket, SOCKADDR_IN* pEndpoint = NULL);

// Operations
public:
	BOOL		GetHashFromXML();
	BOOL		Match(LPCTSTR pszFilename, QWORD nSize, LPCTSTR pszSchemaURI, CXMLElement* pXML, SHA1* pSHA1 = NULL, TIGEROOT* pTiger = NULL, MD4* pED2K = NULL);
	TRISTATE	MatchMetadata(LPCTSTR pszSchemaURI, CXMLElement* pXML);
	BOOL		MatchMetadataShallow(LPCTSTR pszSchemaURI, CXMLElement* pXML);
	void		BuildWordList();
	void		Serialize(CArchive& ar);
	CSearchWnd*	OpenWindow();
	BOOL		CheckValid();
protected:
	void		AddStringToWordList(LPCTSTR pszString);

// Utilities
public:
	static BOOL	WordMatch(LPCTSTR pszString, LPCTSTR pszFind);
	static BOOL	NumberMatch(const CString& strValue, const CString& strRange);

};

#endif // !defined(AFX_QUERYSEARCH_H__2141B926_3F6B_4A5D_9FBD_C67FD0A5C46C__INCLUDED_)
