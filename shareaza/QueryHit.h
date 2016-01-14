//
// QueryHit.h
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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

class CMatchFile;
class CXMLElement;
class CQuerySearch;
class CG1Packet;
class CG2Packet;
class CEDPacket;
class CDCPacket;

#include "Schema.h"
#include "ShareazaFile.h"
#include "VendorCache.h"


class CQueryHit : public CShareazaFile
{
// Construction
public:
	CQueryHit(PROTOCOLID nProtocol, const Hashes::Guid& oSearchID = Hashes::Guid());
	CQueryHit(const CQueryHit& pHit);
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
	CVendorPtr		m_pVendor;
	TRISTATE		m_bPush;
	TRISTATE		m_bBusy;
	TRISTATE		m_bStable;
	TRISTATE		m_bMeasured;
	BOOL			m_bChat;
	BOOL			m_bBrowseHost;
	CString			m_sNick;
public:
	int				m_nGroup;
	DWORD			m_nIndex;
	BOOL			m_bSize;
	DWORD			m_nPartial;
	BOOL			m_bPreview;
	CString			m_sPreview;
	int				m_nUpSlots;
	int				m_nUpQueue;
	BOOL			m_bCollection;
public:
	CSchemaPtr		m_pSchema;
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
	DWORD			m_nHitSources;

// Static Decode Operations
public:
	static CQueryHit*	FromG1Packet(CG1Packet* pPacket, int* pnHops = NULL);
	static CQueryHit*	FromG2Packet(CG2Packet* pPacket, int* pnHops = NULL);
	static CQueryHit*	FromEDPacket(CEDPacket* pPacket, const SOCKADDR_IN* pServer, BOOL bUnicode, const Hashes::Guid& pSearchID = Hashes::Guid());
	static CQueryHit*	FromDCPacket(CDCPacket* pPacket);
protected:
	static CXMLElement*	ReadXML(CG1Packet* pPacket, int nSize);

// Operations
public:
	CQueryHit&	operator=(const CQueryHit& pOther);
	void		Delete();
	int			GetRating();
	void		Serialize(CArchive& ar, int nVersion /* MATCHLIST_SER_VERSION */);
	void		Ban(int nBanLength);	// Ban by host IP only
	void		Resolve();
	void		ReadEDPacket(CEDPacket* pPacket, const SOCKADDR_IN* pServer, BOOL bUnicode);
protected:
	void		ParseAttributes(const Hashes::Guid& pClientID, CVendorPtr pVendor, BYTE* nFlags, BOOL bChat, BOOL bBrowseHost);
	void		ReadG1Packet(CG1Packet* pPacket);
	void		ReadGGEP(CG1Packet* pPacket);
	BOOL		CheckValid() const;
	void		ReadG2Packet(CG2Packet* pPacket, DWORD nLength);
	void		ReadEDAddress(CEDPacket* pPacket, const SOCKADDR_IN* pServer);
	BOOL		ParseXML(CXMLElement* pXML, DWORD nRealIndex);

// Inlines
public:
	inline DWORD GetSources() const
	{
		return m_nHitSources;
	}
	inline BOOL IsRated() const
	{
		return ( m_nRating || m_sComments.GetLength() );
	}
};

#define HITEQUALS_NOT		0
#define HITEQUALS_SIMILAR	1
#define HITEQUALS_IDENTICAL	2
