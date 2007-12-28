//
// QueryHit.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#if !defined(AFX_QUERYHIT_H__275CD566_6CDF_4C59_B78D_7082E49C6E15__INCLUDED_)
#define AFX_QUERYHIT_H__275CD566_6CDF_4C59_B78D_7082E49C6E15__INCLUDED_

#pragma once

#include "ShareazaFile.h"

class CVendor;
class CMatchFile;
class CXMLElement;
class CQuerySearch;
class CG1Packet;
class CG2Packet;
class CEDPacket;


class CQueryHit : public CShareazaFile
{
// Construction
public:
	CQueryHit(PROTOCOLID nProtocol, const Hashes::Guid& oSearchID = Hashes::Guid());
	virtual ~CQueryHit();

// Attributes
public:
	CQueryHit*		m_pNext;
	Hashes::Guid	m_oSearchID;
public:
	PROTOCOLID		m_nProtocol;
	Hashes::Guid	m_oClientID;
	IN_ADDR			m_pAddress;
	CString			m_sCountry;
	WORD			m_nPort;
	DWORD			m_nSpeed;
	CString			m_sSpeed;
	CVendor*		m_pVendor;
	TRISTATE		m_bPush;
	TRISTATE		m_bBusy;
	TRISTATE		m_bStable;
	TRISTATE		m_bMeasured;
	BOOL			m_bChat;
	BOOL			m_bBrowseHost;
	CString			m_sNick;
	CString			m_sKeywords;
public:
	int				m_nGroup;
	DWORD			m_nIndex;
	BOOL			m_bSize;
	DWORD			m_nSources;
	DWORD			m_nPartial;
	BOOL			m_bPreview;
	CString			m_sPreview;
	int				m_nUpSlots;
	int				m_nUpQueue;
	BOOL			m_bCollection;
public:
	CString			m_sSchemaURI;
	CString			m_sSchemaPlural;
	CXMLElement*	m_pXML;
	int				m_nRating;
	CString			m_sComments;
public:
	BOOL			m_bBogus;
	BOOL			m_bMatched;
	BOOL			m_bExactMatch;
	BOOL			m_bFiltered;
	BOOL			m_bDownload;
	BOOL			m_bNew;
	BOOL			m_bSelected;
protected:
	BOOL			m_bResolveURL;

// Static Decode Operations
public:
	static CQueryHit*	FromPacket(CG1Packet* pPacket, int* pnHops = NULL);
	static CQueryHit*	FromPacket(CG2Packet* pPacket, int* pnHops = NULL);
	static CQueryHit*	FromPacket(CEDPacket* pPacket, SOCKADDR_IN* pServer, DWORD m_nServerFlags, const Hashes::Guid& pSearchID = Hashes::Guid());
protected:
	static BOOL			CheckBogus(CQueryHit* pFirstHit);
	static CXMLElement*	ReadXML(CG1Packet* pPacket, int nSize);
	static BOOL			ReadGGEP(CG1Packet* pPacket, BOOL* pbBrowseHost, BOOL* pbChat);

// Operations
public:
	void		Copy(CQueryHit* pOther);
	void		Delete();
	int			GetRating();
	void		Serialize(CArchive& ar, int nVersion);
protected:
	void		ReadG1Packet(CG1Packet* pPacket);
	void		ParseAttributes(const Hashes::Guid& pClientID, CVendor* pVendor, BYTE* nFlags, BOOL bChat, BOOL bBrowseHost);
	void		ReadG2Packet(CG2Packet* pPacket, DWORD nLength);
	BOOL		ReadEDPacket(CEDPacket* pPacket, SOCKADDR_IN* pServer, DWORD m_nServerFlags = 0);
	void		ReadEDAddress(CEDPacket* pPacket, SOCKADDR_IN* pServer);
	BOOL		ParseXML(CXMLElement* pXML, DWORD nRealIndex);
	BOOL		HasBogusMetadata();
	void		Resolve();
	BOOL		AutoDetectSchema(LPCTSTR pszInfo);
	BOOL		AutoDetectAudio(LPCTSTR pszInfo);

// Inlines
public:
	inline int GetSources() const
	{
		return ( m_nProtocol == PROTOCOL_ED2K )
			? m_nSources : ( m_nSources ? 1 : 0 );
	}
	inline BOOL IsRated() const
	{
		return ( m_nRating || m_sComments.GetLength() );
	}
};

#define HITEQUALS_NOT		0
#define HITEQUALS_SIMILAR	1
#define HITEQUALS_IDENTICAL	2

#endif // !defined(AFX_QUERYHIT_H__275CD566_6CDF_4C59_B78D_7082E49C6E15__INCLUDED_)
