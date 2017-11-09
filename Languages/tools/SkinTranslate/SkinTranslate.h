//
// SkinTranslate.h
//
// Copyright (c) Shareaza Development Team, 2009-2014.
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

#define XML_FOR_EACH_BEGIN(x,y) \
{ \
	CComPtr< IXMLDOMNodeList > pList##x; \
	HRESULT hr = (x)->get_childNodes( &(pList##x) ); \
	long nLength##x = 0; \
	if ( hr == S_OK && ( hr = (pList##x)->get_length( &(nLength##x) ) ) == S_OK && (nLength##x) ) \
	{ \
		(pList##x)->reset(); \
		while ( hr == S_OK ) \
		{ \
			CComPtr< IXMLDOMNode > y; \
			hr = (pList##x)->nextNode( &(y) ); \
			if ( hr == S_OK ) \
			{

#define XML_FOR_EACH_END() \
			} \
		} \
	} \
}

class CXMLLoader
{
public:
	CXMLLoader();

	// Load translator data .po-file
	bool LoadPO(LPCWSTR szFilename);

	// Load (and translate) translator data .xml-file
	bool LoadXML(LPCWSTR szFilename, const CXMLLoader* pTranslator = NULL);

	// Add translated strings from specified translator object
	void Translate(const CXMLLoader& oTranslator);

	// Save translator data as .xml-file
	bool SaveXML(LPCTSTR szFilename) const;

	// Extract translator data as .po-file
	bool SavePO(LPCTSTR szFilename) const;

	size_t GetUniqueCount() const;
	size_t GetTotalCount() const;

protected:
	class CItem
	{
	public:
		CItem();
		CItem(const CItem& it);
		CItem(const CString& r, const CString& t, bool k);
		CItem& operator=(const CItem& it);

		void Clear();
		void SetTranslate(LPCSTR szText);
		void SetID(LPCSTR szText);

		CString sRef;
		CString sID;
		CString sTranslated;
		bool	bKeepUnderscores;
		bool	bFuzzy;
		bool	bError;
	} ;

	typedef CAtlList< CItem > CItemList;
	typedef CAtlMap< CString, CItem* > CItemMap;

	CComPtr< IXMLDOMDocument > m_pXMLDoc;	// XML data
	bool				m_bRemoveComments;	// Remove comments from XML data
	const CXMLLoader*	m_pXMLTranslator;	// XML load translator
	CItemList			m_Items;			// PO data
	CItemMap			m_IDIndex;			// PO data index by message ID
	CItemMap			m_RefIndex;			// PO data index by reference

	static CStringA UTF8Encode(LPCWSTR szInput, int nInput = -1);
	static CStringW UTF8Decode(LPCSTR szInput, int nInput = -1);
	static CString MakeSafe(const CString& str);
	static CString UnMakeSafe(const CString& str);
	static bool IsEqual(const CString& _left, const CString& _right);

	// Re-index strings
	void ReIndex();

	CItem& Add(const CItem& item);

	bool Load(LPCWSTR szParentName, LPCWSTR szRefName, LPCWSTR szTextName,
		IXMLDOMElement* pXMLElement, bool bKeepUnderscores = false, bool bSubstitute = false);

	bool LoadManifest(IXMLDOMElement* pXMLElement);
	bool LoadToolbar(IXMLDOMElement* pXMLRoot);
	bool LoadToolbars(IXMLDOMElement* pXMLRoot);
	bool LoadMenu(IXMLDOMElement* pXMLRoot, LPCTSTR szParentName, int& i);
	bool LoadMenus(IXMLDOMElement* pXMLRoot);
	bool LoadDocument(IXMLDOMElement* pXMLRoot, LPCTSTR szParentName, int& i_text, int& i_heading);
	bool LoadDocuments(IXMLDOMElement* pXMLRoot);
	bool LoadCommandTips(IXMLDOMElement* pXMLRoot);
	bool LoadControlTips(IXMLDOMElement* pXMLRoot);
	bool LoadStrings(IXMLDOMElement* pXMLRoot);
	bool LoadDialog(IXMLDOMElement* pXMLRoot);
	bool LoadDialogs(IXMLDOMElement* pXMLRoot);
	bool LoadListColumn(IXMLDOMElement* pXMLRoot);
	bool LoadListColumns(IXMLDOMElement* pXMLRoot);
	bool LoadFonts(IXMLDOMElement* pXMLRoot);
	bool LoadSkin(IXMLDOMElement* pXMLRoot);
};
