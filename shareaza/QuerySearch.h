//
// QuerySearch.h
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

#include "ShareazaFile.h"
#include "Schema.h"

class CPacket;
class CXMLElement;
class CSearchWnd;
class CG1Packet;
class CG2Packet;
class CEDPacket;
class CDCPacket;
class CQuerySearch;


typedef CComObjectPtr< CQuerySearch > CQuerySearchPtr;


class CQuerySearch : public CShareazaFile
{
	DECLARE_DYNAMIC(CQuerySearch)

public:
	CQuerySearch(BOOL bGUID = TRUE);

	typedef std::vector<DWORD>					Hash32List;

// Attributes
public:
	bool				m_bAutostart;	// Autostart search (default - true)
	Hashes::Guid		m_oGUID;		// G1,G2: Search ID
	CString				m_sSearch;		// search string, transformed by lowercase table
	CString				m_sKeywords;	// search keywords (stems, minus words, split asian phrase etc.)
	CString				m_sPosKeywords;	// Positive keywords ( no minus, no quotes basically for Gnutella1 Query)
	CString				m_sG2Keywords;	// Query string for G2, containing Positive keywords and Minus Prefixed negative keywords.
	CSchemaPtr			m_pSchema;		// G1,G2,ED2K: Metadata schema
	CXMLElement*		m_pXML;			// G1,G2,ED2K: Metadata
	QWORD				m_nMinSize;		// G2,ED2K: Minimal file size
	QWORD				m_nMaxSize;		// G2,ED2K: Maximal file size
	Hashes::Ed2kHash	m_oSimilarED2K;	// ED2K: Search for Similar Files
	BOOL				m_bWantURL;		// G2: Sources, preview URL request
	BOOL				m_bWantDN;		// G2: Descriptive name request
	BOOL				m_bWantXML;		// G1,G2: Metadata request
	BOOL				m_bWantCOM;		// G2: Comments request
	BOOL				m_bWantPFS;		// G2: Partial Files Search request
	BOOL				m_bAndG1;
	BYTE				m_nHops;		// G1: Received packet hops
	BOOL				m_bUDP;			// G2: Packet received over UDP
	SOCKADDR_IN			m_pEndpoint;	// G1,G2: Packet received from this host
	DWORD				m_nKey;			// G2: Hub query key
	bool				m_bFirewall;	// G1: Firewalled host
	bool				m_bDynamic;		// G1: Leaf Guided Dynamic Query
	bool				m_bBinHash;		// G1: GGEP "H" allowed
	bool				m_bOOB;			// G1: Out of Band Query
	bool				m_bOOBv3;		// G1: OOB v3 Security Token support
	bool				m_bPartial;		// G1: Partial results support
	bool				m_bNoProxy;		// G1: Disable OOB proxying
	bool				m_bExtQuery;	// G1: Extended query (long query)
	bool				m_bWhatsNew;	// G1: "Whats New?" request
	bool				m_bDropMe;		// Silently drop this packet (to avoid overflow) 

	CString				m_sUserNick;	// DC: Nick of passive user
	SOCKADDR_IN			m_pMyHub;		// DC: Hub address
	CString				m_sMyHub;		// DC: Hub name
	CString				m_sMyNick;		// DC: Nick

	Hash32List			m_oURNs;			// Hashed URNs
	Hash32List			m_oKeywordHashList;	// list of hashed keywords to BOOST QUery Routing.

public:
	typedef WordTable::iterator iterator;
	typedef WordTable::const_iterator const_iterator;
	typedef Hash32List::iterator hash_iterator;
	typedef Hash32List::const_iterator const_hash_iterator;
	//positive keywords
	const_iterator			begin() const { return m_oWords.begin(); }
	const_iterator			end()   const { return m_oWords.end(); }
	size_t					tableSize() const { return m_oWords.size(); }
	//Negative keywords
	const_iterator			beginNeg() const { return m_oNegWords.begin(); }
	const_iterator			endNeg()   const { return m_oNegWords.end(); }
	size_t					tableSizeNeg() const { return m_oNegWords.size(); }
	// hashed URNs
	const_hash_iterator		urnBegin() const { return m_oURNs.begin(); }
	const_hash_iterator		urnEnd()   const { return m_oURNs.end(); }
	// hashed keywords (Positive words only)
	const_hash_iterator		keywordBegin() const { return m_oKeywordHashList.begin(); }
	const_hash_iterator		keywordEnd()   const { return m_oKeywordHashList.end(); }
private:
	WordTable			m_oWords;
	WordTable			m_oNegWords;

	typedef CMap< CString, const CString&, DWORD, DWORD& > CSDMap;
	static CSDMap		m_oSearchHistory;

// Packet Operations
public:
	CG1Packet*				ToG1Packet(DWORD nTTL = 0) const;
	CG2Packet*				ToG2Packet(SOCKADDR_IN* pUDP, DWORD nKey) const;
	CEDPacket*				ToEDPacket(BOOL bUDP, DWORD nServerFlags = 0) const;
	CDCPacket*				ToDCPacket() const;
private:
	BOOL					ReadG1Packet(CG1Packet* pPacket, const SOCKADDR_IN* pEndpoint = NULL);
	void					ReadGGEP(CG1Packet* pPacket);
	BOOL					ReadG2Packet(CG2Packet* pPacket, const SOCKADDR_IN* pEndpoint = NULL);
	BOOL					ReadDCPacket(CDCPacket* pPacket, const SOCKADDR_IN* pEndpoint = NULL);

// Operations
public:
	CString					GetSearch() const;
	void					SetSearch(const CString& sSearch);
	BOOL					Match(LPCTSTR pszFilename, LPCTSTR pszSchemaURI, const CXMLElement* pXML, const CShareazaFile* pFile ) const;
	TRISTATE				MatchMetadata(LPCTSTR pszSchemaURI, const CXMLElement* pXML) const;
	BOOL					MatchMetadataShallow(LPCTSTR pszSchemaURI, const CXMLElement* pXML, bool* bReject = NULL) const;
	void					BuildWordList(bool bExpression = true, bool bLocal=false);
	void					Serialize(CArchive& ar);
	BOOL					CheckValid(bool bExpression = true);
	void					PrepareCheck();

	// Build a regular expression filter from the search query words.
	// Returns an empty string if not applied or if the filter was invalid.
	//
	// Substitutes:
	// <_> - inserts all query keywords;
	// <1>..<9> - inserts query keyword number 1..9;
	// <> - inserts next query keyword.
	//
	// For example regular expression:
	//	.*(<2><1>)|(<_>).*
	// for "music mp3" query will be converted to:
	//	.*(mp3\s*music\s*)|(music\s*mp3\s*).*
	//
	// Note: \s* - matches any number of white-space symbols (including zero).
	CString					BuildRegExp(const CString& strPattern) const;

private:
	BOOL					WriteHashesToEDPacket(CEDPacket* pPacket, BOOL bUDP, BOOL bLargeFiles) const;

// Utilities
public:
	static CQuerySearchPtr	FromPacket(CPacket* pPacket, const SOCKADDR_IN* pEndpoint = NULL, BOOL bGUID = FALSE);
	static CSearchWnd*		OpenWindow(CQuerySearch* pSearch);
	static BOOL				WordMatch(LPCTSTR pszString, LPCTSTR pszFind, bool* bReject = NULL);
	static BOOL				NumberMatch(const CString& strValue, const CString& strRange);
	static void				SearchHelp();	// Shows some search help dialogs
	static BOOL				CheckOverflow(const CString& sSearch);

protected:
	virtual ~CQuerySearch();

private:
	CQuerySearch(const CQuerySearch&);
	CQuerySearch& operator=(const CQuerySearch&);
};
