//
// XML.h
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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
// Construction
public:
	CXMLNode(CXMLElement* pParent = NULL, LPCTSTR pszName = NULL);
	virtual ~CXMLNode();

// Attributes
protected:
	int				m_nNode;
	CXMLElement*	m_pParent;
	CString			m_sName;
	CString			m_sValue;

	enum { xmlNode, xmlElement, xmlAttribute };

// Operations
public:
	int				GetType() const;
	CXMLNode*		AsNode() const;
	CXMLElement*	AsElement() const;
	CXMLAttribute*	AsAttribute() const;
public:
	CXMLElement*	GetParent() const;
	void			Delete();
public:
	CString			GetName() const;
	void			SetName(LPCTSTR pszValue);
	BOOL			IsNamed(LPCTSTR pszName) const;
	CString			GetValue() const;
	void			SetValue(LPCTSTR pszValue);
protected:
	static BOOL		ParseMatch(LPCTSTR& pszXML, LPCTSTR pszToken);
	static BOOL		ParseIdentifier(LPCTSTR& pszXML, CString& strIdentifier);
	void			Serialize(CArchive& ar);
public:
	static CString	StringToValue(LPCTSTR& pszXML, int nLength);
	static void		ValueToString(LPCTSTR pszValue, CString& strXML);
	static void		UniformString(CString& str);

	friend class	CXMLElement;
	friend class	CQuerySearch;
	friend class	CXMLCOM;
};


class CXMLElement : public CXMLNode
{
// Construction
public:
	CXMLElement(CXMLElement* pParent = NULL, LPCTSTR pszName = NULL);
	virtual ~CXMLElement();

// Attributes
protected:
	CList< CXMLElement* > m_pElements;
	CMap< CString, const CString&, CXMLAttribute*, CXMLAttribute* > m_pAttributes;

// Operations
public:
	CXMLElement*	Clone(CXMLElement* pParent = NULL);
	CXMLElement*	Detach();
public:
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
public:
	CXMLAttribute*	AddAttribute(LPCTSTR pszName, LPCTSTR pszValue = NULL);
	CXMLAttribute*	AddAttribute(CXMLAttribute* pAttribute);
	int				GetAttributeCount() const;
	POSITION		GetAttributeIterator() const;
	CXMLAttribute*	GetNextAttribute(POSITION& pos) const;
	CXMLAttribute*	GetAttribute(LPCTSTR pszName) const;
	CString			GetAttributeValue(LPCTSTR pszName, LPCTSTR pszDefault = NULL) const;
	void			RemoveAttribute(CXMLAttribute* pAttribute);
	void			DeleteAttribute(LPCTSTR pszName);
	void			DeleteAllAttributes();
public:
	CString			ToString(BOOL bHeader = FALSE, BOOL bNewline = FALSE);
	void			ToString(CString& strXML, BOOL bNewline = FALSE);
	BOOL			ParseString(LPCTSTR& strXML);
	BOOL			Equals(CXMLElement* pXML) const;
	// Add missing elements and attributes from pInput, preserve existing
	BOOL			Merge(CXMLElement* pInput);
	CString			GetRecursiveWords();
	void			AddRecursiveWords(CString& strWords);
	void			Serialize(CArchive& ar);

	static CXMLElement*	FromString(LPCTSTR pszXML, BOOL bHeader = FALSE);
	static CXMLElement* FromBytes(BYTE* pByte, DWORD nByte, BOOL bHeader = FALSE);
	static CXMLElement* FromFile(LPCTSTR pszPath, BOOL bHeader = FALSE);
	static CXMLElement* FromFile(HANDLE hFile, BOOL bHeader = FALSE);
};


class CXMLAttribute : public CXMLNode
{
// Construction
public:
	CXMLAttribute(CXMLElement* pParent, LPCTSTR pszName = NULL);
	virtual ~CXMLAttribute();

// Attributes
public:
	static LPCTSTR	xmlnsSchema;
	static LPCTSTR	xmlnsInstance;
	static LPCTSTR	schemaName;

// Operations
public:
	CXMLAttribute*	Clone(CXMLElement* pParent = NULL);
	void			ToString(CString& strXML);
	BOOL			ParseString(LPCTSTR& strXML);
	BOOL			Equals(CXMLAttribute* pXML) const;
	void			Serialize(CArchive& ar);
};

#include "XML.inl"
