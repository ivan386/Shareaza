//
// SchemaChild.h
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

class CSchema;
class CSchemaChild;
class CSchemaChildMap;
class CXMLElement;

typedef const CSchemaChildMap* CSchemaChildMapPtr;

#include "Schema.h"


class CSchemaChild
{
public:
	CSchemaChild(CSchemaPtr pSchema);
	~CSchemaChild();

	CSchema::Type	m_nType;
	CString			m_sURI;

	inline INT_PTR GetCount() const { return m_pMap.GetCount(); }

	BOOL		Load(const CXMLElement* pXML);
	BOOL		MemberCopy(CXMLElement* pLocal, CXMLElement* pRemote, BOOL bToRemote = FALSE, BOOL bAggressive = FALSE) const;

protected:
	CSchemaPtr					m_pSchema;
	CList< CSchemaChildMapPtr >	m_pMap;

	void		Clear();

private:
	CSchemaChild(const CSchemaChild&);
	CSchemaChild& operator=(const CSchemaChild&);
};


class CSchemaChildMap
{
public:
	CSchemaChildMap();

	BOOL		m_bIdentity;
	CString		m_sLocal;
	CString		m_sRemote;

	BOOL		Load(const CXMLElement* pXML);

private:
	CSchemaChildMap(const CSchemaChildMap&);
	CSchemaChildMap& operator=(const CSchemaChildMap&);
};
