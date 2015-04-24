//
// XML.h
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

class CXMLNode;
class CXMLElement;
class CXMLAttribute;


class CXMLNode
{
public:
	CXMLNode(CXMLElement* pParent = NULL, LPCTSTR pszName = NULL);
	virtual ~CXMLNode();

protected:
	int				m_nNode;
	CXMLElement*	m_pParent;
	CString			m_sName;
	CString			m_sValue;

	enum { xmlNode, xmlElement, xmlAttribute };

	static BOOL		ParseMatch(LPCTSTR& pszXML, LPCTSTR pszToken);
	static BOOL		ParseIdentifier(LPCTSTR& pszXML, CString& strIdentifier);
	void			Serialize(CArchive& ar);

public:
	int				GetType() const;
	CXMLNode*		AsNode() const;
	CXMLElement*	AsElement() const;
	CXMLAttribute*	AsAttribute() const;
	CXMLElement*	GetParent() const;
	void			Delete();
	CString			GetName() const;
	virtual void	SetName(LPCTSTR pszValue);
	BOOL			IsNamed(LPCTSTR pszName) const;
	CString			GetValue() const;
	void			SetValue(const CString& strValue);
	static void		UniformString(CString& str);

	friend class	CXMLElement;
	friend class	CQuerySearch;
	friend class	CXMLCOM;
};


class CXMLElement : public CXMLNode
{
public:
	CXMLElement(CXMLElement* pParent = NULL, LPCTSTR pszName = NULL);
	virtual ~CXMLElement();

protected:
	CList< CXMLElement* > m_pElements;
	CMap< CString, const CString&, CXMLAttribute*, CXMLAttribute* > m_pAttributes;	// Lowercased name of attribute  <-> attribute pointer

	void			AddRecursiveWords(CString& strWords) const;
	void			ToString(CString& strXML, BOOL bNewline) const;

public:
	CXMLElement*	Clone(CXMLElement* pParent = NULL) const;
	// Clone element and then rename all elements and attributes by using specified prefix
	CXMLElement*	Prefix(const CString& sPrefix, CXMLElement* pParent = NULL) const;
	CXMLElement*	Detach();
	CXMLElement*	AddElement(LPCTSTR pszName);
	CXMLElement*	AddElement(CXMLElement* pElement);
	INT_PTR			GetElementCount() const;
	CXMLElement*	GetFirstElement() const;
	POSITION		GetElementIterator() const;
	CXMLElement*	GetNextElement(POSITION& pos) const;
	CXMLElement*	GetElementByName(LPCTSTR pszName) const;
	CXMLElement*	GetElementByName(LPCTSTR pszName, BOOL bCreate);
	void			RemoveElement(CXMLElement* pElement);
	void			DeleteAllElements();
	CXMLAttribute*	AddAttribute(LPCTSTR pszName, LPCTSTR pszValue = NULL);
	CXMLAttribute*	AddAttribute(LPCTSTR pszName, __int64 nValue);
	CXMLAttribute*	AddAttribute(CXMLAttribute* pAttribute);
	int				GetAttributeCount() const;
	POSITION		GetAttributeIterator() const;
	CXMLAttribute*	GetNextAttribute(POSITION& pos) const;
	CXMLAttribute*	GetAttribute(LPCTSTR pszName) const;
	CString			GetAttributeValue(LPCTSTR pszName, LPCTSTR pszDefault = NULL) const;
	void			RemoveAttribute(CXMLAttribute* pAttribute);
	void			DeleteAttribute(LPCTSTR pszName);
	void			DeleteAllAttributes();
	CString			ToString(BOOL bHeader = FALSE, BOOL bNewline = FALSE, BOOL bEncoding = FALSE, TRISTATE bSnadalone = TRI_UNKNOWN) const;
	BOOL			ParseString(LPCTSTR& strXML);
	BOOL			Equals(const CXMLElement* pXML) const;
	// Add missing elements and attributes from pInput, preserve or overwrite existing
	BOOL			Merge(const CXMLElement* pInput, BOOL bOverwrite = FALSE);
	CString			GetRecursiveWords() const;
	void			Serialize(CArchive& ar);

	static CXMLElement*	FromString(LPCTSTR pszXML, BOOL bHeader = FALSE, CString* pEncoding = NULL);
	static CXMLElement* FromBytes(BYTE* pByte, DWORD nByte, BOOL bHeader = FALSE);
	static CXMLElement* FromFile(LPCTSTR pszPath, BOOL bHeader = FALSE);
	static CXMLElement* FromFile(HANDLE hFile, BOOL bHeader = FALSE);
};


class CXMLAttribute : public CXMLNode
{
public:
	CXMLAttribute(CXMLElement* pParent, LPCTSTR pszName = NULL);
	virtual ~CXMLAttribute();

	static LPCTSTR	xmlnsSchema;
	static LPCTSTR	xmlnsInstance;
	static LPCTSTR	schemaName;

	CXMLAttribute*	Clone(CXMLElement* pParent = NULL) const;
	void			ToString(CString& strXML) const;
	BOOL			ParseString(LPCTSTR& strXML);
	BOOL			Equals(const CXMLAttribute* pXML) const;
	void			Serialize(CArchive& ar);
	virtual void	SetName(LPCTSTR pszValue);
};

#include "XML.inl"
