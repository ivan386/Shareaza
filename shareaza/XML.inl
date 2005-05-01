//
// XML.inl
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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


//////////////////////////////////////////////////////////////////////
// CXMLNode node type and casting access

int CXMLNode::GetType() const
{
	return m_nNode;
}

CXMLNode* CXMLNode::AsNode() const
{
	return (CXMLNode*)this;
}

CXMLElement* CXMLNode::AsElement() const
{
	return ( m_nNode == xmlElement ) ? (CXMLElement*)this : NULL;
}

CXMLAttribute* CXMLNode::AsAttribute() const
{
	return ( m_nNode == xmlAttribute ) ? (CXMLAttribute*)this : NULL;
}

//////////////////////////////////////////////////////////////////////
// CXMLNode parent access and delete

CXMLElement* CXMLNode::GetParent() const
{
	return m_pParent;
}

void CXMLNode::Delete()
{
	if ( this == NULL ) return;

	if ( m_pParent != NULL )
	{
		if ( m_nNode == xmlElement ) m_pParent->RemoveElement( (CXMLElement*)this );
		else if ( m_nNode == xmlAttribute ) m_pParent->RemoveAttribute( (CXMLAttribute*)this );
	}

	delete this;
}

//////////////////////////////////////////////////////////////////////
// CXMLNode name access

CString CXMLNode::GetName() const
{
	return m_sName;
}

void CXMLNode::SetName(LPCTSTR pszValue)
{
	m_sName = pszValue;
}

BOOL CXMLNode::IsNamed(LPCTSTR pszName) const
{
	if ( this == NULL ) return FALSE;
	return m_sName.CompareNoCase( pszName ) == 0;
}

//////////////////////////////////////////////////////////////////////
// CXMLNode value access

CString CXMLNode::GetValue() const
{
	return m_sValue;
}

void CXMLNode::SetValue(LPCTSTR pszValue)
{
	m_sValue = pszValue;
}

//////////////////////////////////////////////////////////////////////
// CXMLElement detach

CXMLElement* CXMLElement::Detach()
{
	if ( m_pParent ) m_pParent->RemoveElement( this );
	m_pParent = NULL;
	return this;
}

//////////////////////////////////////////////////////////////////////
// CXMLElement element access

CXMLElement* CXMLElement::AddElement(LPCTSTR pszName)
{
	CXMLElement* pElement = new CXMLElement( this, pszName );
	m_pElements.AddTail( pElement );
	return pElement;
}

CXMLElement* CXMLElement::AddElement(CXMLElement* pElement)
{
	if ( pElement->m_pParent ) return NULL;
	m_pElements.AddTail( pElement );
	pElement->m_pParent = this;
	return pElement;
}

int CXMLElement::GetElementCount() const
{
	return (int)m_pElements.GetCount();
}

CXMLElement* CXMLElement::GetFirstElement() const
{
	if ( this == NULL ) return NULL;
	return m_pElements.GetCount() ? (CXMLElement*)m_pElements.GetHead() : NULL;
}

POSITION CXMLElement::GetElementIterator() const
{
	return m_pElements.GetHeadPosition();
}

CXMLElement* CXMLElement::GetNextElement(POSITION& pos) const
{
	return (CXMLElement*)m_pElements.GetNext( pos );
}

CXMLElement* CXMLElement::GetElementByName(LPCTSTR pszName) const
{
	for ( POSITION pos = GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = GetNextElement( pos );
		if ( pElement->GetName().CompareNoCase( pszName ) == 0 ) return pElement;
	}
	return NULL;
}

CXMLElement* CXMLElement::GetElementByName(LPCTSTR pszName, BOOL bCreate)
{
	for ( POSITION pos = GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = GetNextElement( pos );
		if ( pElement->GetName().CompareNoCase( pszName ) == 0 ) return pElement;
	}

	return bCreate ? AddElement( pszName ) : NULL;
}

void CXMLElement::RemoveElement(CXMLElement* pElement)
{
	POSITION pos = m_pElements.Find( pElement );
	if ( pos ) m_pElements.RemoveAt( pos );
}

//////////////////////////////////////////////////////////////////////
// CXMLElement attribute access

CXMLAttribute* CXMLElement::AddAttribute(LPCTSTR pszName, LPCTSTR pszValue)
{
	CXMLAttribute* pAttribute = GetAttribute( pszName );

	if ( ! pAttribute )
	{
		pAttribute = new CXMLAttribute( this, pszName );
		CString strName( pszName );
		strName.MakeLower();
		m_pAttributes.SetAt( strName, pAttribute );
	}

	if ( pszValue ) pAttribute->SetValue( pszValue );

	return pAttribute;
}

CXMLAttribute* CXMLElement::AddAttribute(CXMLAttribute* pAttribute)
{
	if ( pAttribute->m_pParent ) return NULL;
	CString strName( pAttribute->m_sName );
	strName.MakeLower();
	m_pAttributes.SetAt( pAttribute->m_sName, pAttribute );
	pAttribute->m_pParent = this;
	return pAttribute;
}

int CXMLElement::GetAttributeCount() const
{
	return (int)m_pAttributes.GetCount();
}

POSITION CXMLElement::GetAttributeIterator() const
{
	return m_pAttributes.GetStartPosition();
}

CXMLAttribute* CXMLElement::GetNextAttribute(POSITION& pos) const
{
	CXMLAttribute* pAttribute = NULL;
	CString strName;
	m_pAttributes.GetNextAssoc( pos, strName, XMLVOID(pAttribute) );
	return pAttribute;
}

CXMLAttribute* CXMLElement::GetAttribute(LPCTSTR pszName) const
{
	CXMLAttribute* pAttribute = NULL;
	CString strName( pszName );
	strName.MakeLower();
	return m_pAttributes.Lookup( strName, XMLVOID(pAttribute) ) ? pAttribute : NULL;
}

CString CXMLElement::GetAttributeValue(LPCTSTR pszName, LPCTSTR pszDefault) const
{
	CXMLAttribute* pAttribute = GetAttribute( pszName );
	CString strResult;
	if ( pAttribute ) strResult = pAttribute->m_sValue;
	else if ( pszDefault ) strResult = pszDefault;
	return strResult;
}

void CXMLElement::RemoveAttribute(CXMLAttribute* pAttribute)
{
	CString strName( pAttribute->m_sName );
	strName.MakeLower();
	m_pAttributes.RemoveKey( strName );
}

void CXMLElement::DeleteAttribute(LPCTSTR pszName)
{
	CXMLAttribute* pAttribute = GetAttribute( pszName );
	if ( pAttribute ) pAttribute->Delete();
}
