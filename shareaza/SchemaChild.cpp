//
// SchemaChild.cpp
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Schema.h"
#include "SchemaChild.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CSchemaChild construction

CSchemaChild::CSchemaChild(CSchemaPtr pSchema)
	: m_pSchema	( pSchema )
	, m_nType	( CSchema::stFile )
{
}

CSchemaChild::~CSchemaChild()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CSchemaChild load

BOOL CSchemaChild::Load(const CXMLElement* pXML)
{
	m_sURI = pXML->GetAttributeValue( _T("location") );
	if ( m_sURI.IsEmpty() ) return FALSE;

	CString strType = pXML->GetAttributeValue( _T("type") );

	if ( strType == _T("folder") )
		m_nType = CSchema::stFolder;
	else if ( strType == _T("file") )
		m_nType = CSchema::stFile;
	else
		return FALSE;

	for ( POSITION pos = pXML->GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = pXML->GetNextElement( pos );

		if (	pElement->IsNamed( _T("identity") ) ||
				pElement->IsNamed( _T("shared") ) )
		{
			CSchemaChildMap* pMap = new CSchemaChildMap();

			if ( pMap->Load( pElement ) )
			{
				m_pMap.AddTail( pMap );
			}
			else
			{
				delete pMap;
				return FALSE;
			}
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSchemaChild clear

void CSchemaChild::Clear()
{
	for ( POSITION pos = m_pMap.GetHeadPosition() ; pos ; )
	{
		delete m_pMap.GetNext( pos );
	}
	m_pMap.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CSchemaChild member copy

BOOL CSchemaChild::MemberCopy(CXMLElement* pLocal, CXMLElement* pRemote, BOOL bToRemote, BOOL bAggressive) const
{
	if ( ! pLocal || ! pRemote ) return FALSE;

	BOOL bChanged = FALSE;

	for ( POSITION pos = m_pMap.GetHeadPosition() ; pos ; )
	{
		CSchemaChildMapPtr pMap = m_pMap.GetNext( pos );
		const CXMLAttribute* pAttribute1	= NULL;
		const CXMLAttribute* pAttribute2	= NULL;

		if ( bToRemote )
		{
			pAttribute1 = pLocal->GetAttribute( pMap->m_sLocal );
			pAttribute2 = pRemote->GetAttribute( pMap->m_sRemote );
		}
		else
		{
			pAttribute1 = pRemote->GetAttribute( pMap->m_sRemote );
			pAttribute2 = pLocal->GetAttribute( pMap->m_sLocal );
		}

		if ( pAttribute1 && ( ! pAttribute2 || bAggressive ) )
		{
			CString strValue( pAttribute1->GetValue() );

			if ( pMap->m_bIdentity ) CXMLNode::UniformString( strValue );

			if ( bToRemote )
				pRemote->AddAttribute( pMap->m_sRemote, strValue );
			else
				pLocal->AddAttribute( pMap->m_sLocal, strValue );

			bChanged = TRUE;
		}
	}

	return bChanged;
}

//////////////////////////////////////////////////////////////////////
// CSchemaChildMap construction

CSchemaChildMap::CSchemaChildMap()
	: m_bIdentity ( FALSE )
{
}

//////////////////////////////////////////////////////////////////////
// CSchemaChildMap operation

BOOL CSchemaChildMap::Load(const CXMLElement* pXML)
{
	if ( pXML->IsNamed( _T("identity") ) )
		m_bIdentity = TRUE;
	else if ( pXML->IsNamed( _T("shared") ) )
		m_bIdentity = FALSE;
	else
		return FALSE;

	m_sLocal	= pXML->GetAttributeValue( _T("local") );
	m_sRemote	= pXML->GetAttributeValue( _T("remote") );

	if ( m_sLocal.IsEmpty() || m_sRemote.IsEmpty() ) return FALSE;

	return TRUE;
}
