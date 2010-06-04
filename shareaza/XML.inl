//
// XML.inl
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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


//////////////////////////////////////////////////////////////////////
// CXMLNode node type and casting access

inline int CXMLNode::GetType() const
{
	return m_nNode;
}

inline CXMLNode* CXMLNode::AsNode() const
{
	return (CXMLNode*)this;
}

inline CXMLElement* CXMLNode::AsElement() const
{
	return ( m_nNode == xmlElement ) ? (CXMLElement*)this : NULL;
}

inline CXMLAttribute* CXMLNode::AsAttribute() const
{
	return ( m_nNode == xmlAttribute ) ? (CXMLAttribute*)this : NULL;
}

//////////////////////////////////////////////////////////////////////
// CXMLNode parent access and delete

inline CXMLElement* CXMLNode::GetParent() const
{
	return m_pParent;
}

//////////////////////////////////////////////////////////////////////
// CXMLNode name access

inline CString CXMLNode::GetName() const
{
	return m_sName;
}

inline void CXMLNode::SetName(LPCTSTR pszValue)
{
	m_sName = pszValue;
}

inline BOOL CXMLNode::IsNamed(LPCTSTR pszName) const
{
	if ( this == NULL ) return FALSE;
	return m_sName.CompareNoCase( pszName ) == 0;
}

//////////////////////////////////////////////////////////////////////
// CXMLNode value access

inline CString CXMLNode::GetValue() const
{
	return m_sValue;
}

inline void CXMLNode::SetValue(LPCTSTR pszValue)
{
	m_sValue = pszValue;
}

//////////////////////////////////////////////////////////////////////
// CXMLElement detach

inline CXMLElement* CXMLElement::Detach()
{
	if ( m_pParent ) m_pParent->RemoveElement( this );
	m_pParent = NULL;
	return this;
}

//////////////////////////////////////////////////////////////////////
// CXMLElement element access

inline CXMLElement* CXMLElement::AddElement(CXMLElement* pElement)
{
	if ( pElement->m_pParent ) return NULL;
	m_pElements.AddTail( pElement );
	pElement->m_pParent = this;
	return pElement;
}

inline INT_PTR CXMLElement::GetElementCount() const
{
	return m_pElements.GetCount();
}

inline CXMLElement* CXMLElement::GetFirstElement() const
{
	if ( this == NULL ) return NULL;
	return m_pElements.GetCount() ? m_pElements.GetHead() : NULL;
}

inline POSITION CXMLElement::GetElementIterator() const
{
	return m_pElements.GetHeadPosition();
}

inline CXMLElement* CXMLElement::GetNextElement(POSITION& pos) const
{
	return m_pElements.GetNext( pos );
}

inline CXMLElement* CXMLElement::GetElementByName(LPCTSTR pszName) const
{
	for ( POSITION pos = GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = GetNextElement( pos );
		if ( pElement->GetName().CompareNoCase( pszName ) == 0 ) return pElement;
	}
	return NULL;
}

inline CXMLElement* CXMLElement::GetElementByName(LPCTSTR pszName, BOOL bCreate)
{
	for ( POSITION pos = GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = GetNextElement( pos );
		if ( pElement->GetName().CompareNoCase( pszName ) == 0 ) return pElement;
	}

	return bCreate ? AddElement( pszName ) : NULL;
}

inline void CXMLElement::RemoveElement(CXMLElement* pElement)
{
	POSITION pos = m_pElements.Find( pElement );
	if ( pos ) m_pElements.RemoveAt( pos );
}

//////////////////////////////////////////////////////////////////////
// CXMLElement attribute access

inline int CXMLElement::GetAttributeCount() const
{
	return (int)m_pAttributes.GetCount();
}

inline POSITION CXMLElement::GetAttributeIterator() const
{
	return m_pAttributes.GetStartPosition();
}

inline CXMLAttribute* CXMLElement::GetNextAttribute(POSITION& pos) const
{
	CXMLAttribute* pAttribute = NULL;
	CString strName;
	m_pAttributes.GetNextAssoc( pos, strName, pAttribute );
	return pAttribute;
}

inline CXMLAttribute* CXMLElement::GetAttribute(LPCTSTR pszName) const
{
	CXMLAttribute* pAttribute = NULL;
	return m_pAttributes.Lookup( CString( pszName ).MakeLower(), pAttribute ) ? pAttribute : NULL;
}

inline CString CXMLElement::GetAttributeValue(LPCTSTR pszName, LPCTSTR pszDefault) const
{
	CXMLAttribute* pAttribute = GetAttribute( pszName );
	CString strResult;

	if ( pAttribute )
		strResult = pAttribute->m_sValue;
	else if ( pszDefault )
		strResult = pszDefault;

	return strResult;
}

inline void CXMLElement::RemoveAttribute(CXMLAttribute* pAttribute)
{
	m_pAttributes.RemoveKey( CString( pAttribute->m_sName ).MakeLower() );
}

inline void CXMLElement::DeleteAttribute(LPCTSTR pszName)
{
	CXMLAttribute* pAttribute = GetAttribute( pszName );
	if ( pAttribute ) pAttribute->Delete();
}
