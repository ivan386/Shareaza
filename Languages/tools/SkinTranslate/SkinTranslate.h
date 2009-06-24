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
		inline CItem() :
			bKeepUnderscores( false )
		{
		}
		inline CItem(const CString& r, const CString& t, bool k) :
			sRef( r ), sID( t ), bKeepUnderscores( k )
		{
		}
		inline void Clear()
		{
			sRef.Empty();
			sID.Empty();
			sTranslated.Empty();
		}
		CString sRef;
		CString sID;
		CString sTranslated;
		bool	bKeepUnderscores;
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
	static CString& MakeSafe(CString& str);
	static CString& UnMakeSafe(CString& str);
	static bool IsEqual(const CString& _left, const CString& _right);

	// Re-index strings
	void ReIndex();

	CItem& Add(const CItem& item);
	bool Add(const CString& sRef, CString sID, bool bKeepUnderscores = false);
	bool LoadManifest(IXMLDOMElement* pXMLElement);
	bool Load(LPCWSTR szParentName, LPCWSTR szRefName, LPCWSTR szTextName,
		IXMLDOMElement* pXMLElement, bool bKeepUnderscores = false);
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
	bool LoadSkin(IXMLDOMElement* pXMLRoot);
};
