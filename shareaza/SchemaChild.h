//
// SchemaChild.h
//
// Copyright © Shareaza Development Team, 2002-2009.
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

#if !defined(AFX_SCHEMACHILD_H__8CDBBB11_8165_478F_9E52_1B1E298CA72E__INCLUDED_)
#define AFX_SCHEMACHILD_H__8CDBBB11_8165_478F_9E52_1B1E298CA72E__INCLUDED_

#pragma once

class CSchema;
class CSchemaChildMap;
class CXMLElement;


class CSchemaChild
{
// Construction
public:
	CSchemaChild(CSchema* pSchema);
	virtual ~CSchemaChild();

// Attributes
public:
	CSchema*	m_pSchema;
	int			m_nType;
	CString		m_sURI;
public:
	CList< CSchemaChildMap* >	m_pMap;

// Operations
public:
	BOOL		Load(CXMLElement* pXML);
	void		Clear();
	BOOL		MemberCopy(CXMLElement* pLocal, CXMLElement* pRemote, BOOL bToRemote = FALSE, BOOL bAggressive = FALSE);

};


class CSchemaChildMap
{
// Construction
public:
	CSchemaChildMap();
	virtual ~CSchemaChildMap();

// Attributes
public:
	BOOL		m_bIdentity;
	CString		m_sLocal;
	CString		m_sRemote;

// Operations
public:
	BOOL		Load(CXMLElement* pXML);

};

#endif // !defined(AFX_SCHEMACHILD_H__8CDBBB11_8165_478F_9E52_1B1E298CA72E__INCLUDED_)
