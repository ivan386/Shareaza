//
// QueryHit.h
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

#if !defined(AFX_QUERYHIT_H__275CD566_6CDF_4C59_B78D_7082E49C6E15__INCLUDED_)
#define AFX_QUERYHIT_H__275CD566_6CDF_4C59_B78D_7082E49C6E15__INCLUDED_

#pragma once

#include "Hashes.h"
#include "GUID.h"

class CVendor;
class CMatchFile;
class CXMLElement;
class CQuerySearch;
class CG1Packet;
class CG2Packet;
class CEDPacket;


class CQueryHit
{
// Construction
public:
	CQueryHit(PROTOCOLID nProtocol, CGUID* pSearchID = NULL);
	virtual ~CQueryHit();
	
// Attributes
public:
	CQueryHit*	m_pNext;
	CGUID		m_pSearchID;
public:
	PROTOCOLID	m_nProtocol;
	CGUID		m_pClientID;
	IN_ADDR		m_pAddress;
	WORD		m_nPort;
	DWORD		m_nSpeed;
	CString		m_sSpeed;
	CVendor*	m_pVendor;
	TRISTATE	m_bPush;
	TRISTATE	m_bBusy;
	TRISTATE	m_bStable;
	TRISTATE	m_bMeasured;
	BOOL		m_bChat;
	BOOL		m_bBrowseHost;
	CString		m_sNick;
public:
	int			m_nGroup;
	CManagedSHA1	m_oSHA1;
	CManagedTiger	m_oTiger;
	CManagedED2K	m_oED2K;
	CManagedBTH		m_oBTH;
	CString		m_sURL;
	CString		m_sName;
	DWORD		m_nIndex;
	BOOL		m_bSize;
	QWORD		m_nSize;
	DWORD		m_nSources;
	DWORD		m_nPartial;
	BOOL		m_bPreview;
	CString		m_sPreview;
	int			m_nUpSlots;
	int			m_nUpQueue;
	BOOL		m_bCollection;
public:
	CString			m_sSchemaURI;
	CString			m_sSchemaPlural;
	CXMLElement*	m_pXML;
	int				m_nRating;
	CString			m_sComments;
public:
	BOOL		m_bBogus;
	BOOL		m_bMatched;
	BOOL		m_bFiltered;
	BOOL		m_bDownload;
	BOOL		m_bNew;
	BOOL		m_bSelected;
protected:
	BOOL		m_bResolveURL;

// Static Decode Operations
public:
	static CQueryHit*	FromPacket(CG1Packet* pPacket, int* pnHops = NULL);
	static CQueryHit*	FromPacket(CG2Packet* pPacket, int* pnHops = NULL);
	static CQueryHit*	FromPacket(CEDPacket* pPacket, SOCKADDR_IN* pServer, CGUID* pSearchID = NULL);
protected:
	static BOOL			CheckBogus(CQueryHit* pFirstHit);
	static CXMLElement*	ReadXML(CG1Packet* pPacket, int nSize);
	static BOOL			ReadGGEP(CG1Packet* pPacket, BOOL* pbBrowseHost);

// Operations
public:
	void		Copy(CQueryHit* pOther);
	void		Delete();
	int			GetRating();
	void		Serialize(CArchive& ar, int nVersion);
protected:
	void		ReadG1Packet(CG1Packet* pPacket);
	void		ParseAttributes(CGUID* pClientID, CVendor* pVendor, BYTE* nFlags, BOOL bChat, BOOL bBrowseHost);
	void		ReadG2Packet(CG2Packet* pPacket, DWORD nLength);
	BOOL		ReadEDPacket(CEDPacket* pPacket, SOCKADDR_IN* pServer);
	void		ReadEDAddress(CEDPacket* pPacket, SOCKADDR_IN* pServer);
	BOOL		ParseXML(CXMLElement* pXML, DWORD nRealIndex);
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
};

#define HITEQUALS_NOT		0
#define HITEQUALS_SIMILAR	1
#define HITEQUALS_IDENTICAL	2

#endif // !defined(AFX_QUERYHIT_H__275CD566_6CDF_4C59_B78D_7082E49C6E15__INCLUDED_)
