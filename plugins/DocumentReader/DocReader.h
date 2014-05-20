//
// DocReader.h
//
//	Created by:		Rolandas Rudomanskis
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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

#include "resource.h"
#include "DocumentReader.h"

// CDocReader

class ATL_NO_VTABLE CDocReader : 
	public CComObjectRootEx< CComMultiThreadModel >,
	public CComCoClass< CDocReader, &CLSID_DocReader >,
	public IImageServicePlugin,
	public ILibraryBuilderPlugin
{
public:
	CDocReader();
	~CDocReader();

	DECLARE_REGISTRY_RESOURCEID(IDR_DOCREADER)

	BEGIN_COM_MAP(CDocReader)
		COM_INTERFACE_ENTRY(IImageServicePlugin)
		COM_INTERFACE_ENTRY(ILibraryBuilderPlugin)
	END_COM_MAP()

protected:
	class CDocumentProperties
	{
	public:
		CDocumentProperties(BOOL bOnlyThumb	= FALSE);
		~CDocumentProperties(void);

		BOOL	m_bOnlyThumb;

		////////////////////////////////////////////////////////////////////
		// CSummaryProperties - Collection Class For Summary Properties
		//   (FMTID_SummaryInformation and FMTID_DocSummaryInformation)
		//
		class CSummaryProperties
		{
		public:
			CSummaryProperties(BOOL bOnlyThumb	= FALSE);
			~CSummaryProperties(void);

			BOOL	m_bOnlyThumb;

			// SummaryProperties Implementation
			// FMTID_SummaryInformation Properties...
			HRESULT get_Title(BSTR* pbstrTitle);
			HRESULT put_Title(BSTR bstrTitle);
			HRESULT get_Subject(BSTR* pbstrSubject);
			HRESULT put_Subject(BSTR bstrSubject);
			HRESULT get_Author(BSTR* pbstrAuthor);
			HRESULT put_Author(BSTR bstrAuthor);
			HRESULT get_Keywords(BSTR* pbstrKeywords);
			HRESULT put_Keywords(BSTR bstrKeywords);
			HRESULT get_Comments(BSTR* pbstrComments);
			HRESULT put_Comments(BSTR bstrComments);
			HRESULT get_Template(BSTR* pbstrTemplate);
			HRESULT get_LastSavedBy(BSTR* pbstrLastSavedBy);
			HRESULT put_LastSavedBy(BSTR bstrLastSavedBy);
			HRESULT get_RevisionNumber(BSTR* pbstrRevisionNumber);
			HRESULT get_TotalEditTime(long* plTotalEditTime);
			HRESULT get_DateLastPrinted(VARIANT* pdtDateLastPrinted);
			HRESULT get_DateCreated(VARIANT* pdtDateCreated);
			HRESULT get_DateLastSaved(VARIANT* pdtDateLastSaved);
			HRESULT get_PageCount(long* plPageCount);
			HRESULT get_WordCount(long* plWordCount);
			HRESULT get_CharacterCount(long* plCharacterCount);
			HRESULT get_Thumbnail(VARIANT* pvtThumbnail);
			HRESULT get_ApplicationName(BSTR* pbstrAppName);
			HRESULT get_DocumentSecurity(long* plDocSecurity);

			// FMTID_DocSummaryInformation Properties...
			HRESULT get_Category(BSTR* pbstrCategory);
			HRESULT put_Category(BSTR bstrCategory);
			HRESULT get_PresentationFormat(BSTR* pbstrPresFormat);
			HRESULT get_ByteCount(long* plByteCount);
			HRESULT get_LineCount(long* plLineCount);
			HRESULT get_ParagraphCount(long* plParagraphCount);
			HRESULT get_SlideCount(long* plSlideCount);
			HRESULT get_NoteCount(long* plNoteCount);
			HRESULT get_HiddenSlideCount(long* plHiddenSlideCount);
			HRESULT get_MultimediaClipCount(long* plMultimediaClipCount);
			HRESULT get_Manager(BSTR* pbstrManager);
			HRESULT put_Manager(BSTR bstrManager);
			HRESULT get_Company(BSTR* pbstrCompany);
			HRESULT put_Company(BSTR bstrCompany);
			HRESULT get_CharacterCountWithSpaces(long* plCharCountWithSpaces);
			HRESULT get_SharedDocument(VARIANT_BOOL* pbSharedDocument);
			HRESULT get_Version(BSTR* pbstrVersion);
			HRESULT get_DigitalSignature(VARIANT* pvtDigSig);

			// Internal Functions
			HRESULT LoadProperties(IPropertySetStorage* pPropSS, BOOL fIsReadOnly, dsoFileOpenOptions dwFlags);
			HRESULT ReadProperty(CDocProperty* pPropList, PROPID pid, VARTYPE vt, void* pv);
			HRESULT WriteProperty(CDocProperty** ppPropList, PROPID pid, VARTYPE vt, void* pv);
			CDocProperty* GetPropertyFromList(CDocProperty* plist, PROPID id, BOOL fAppendNew);
			HRESULT SaveProperties(BOOL fCommitChanges);
			BOOL FIsDirty();
			void Disconnect();

		private:
			IPropertySetStorage*    m_pPropSetStg;  // Property Set Storage
			dsoFileOpenOptions      m_dwFlags;      // Open Flags
			BOOL		            m_fReadOnly;    // Should be read-only?
			BOOL                    m_fExternal;    // Does object have external ref count?
			BOOL                    m_fDeadObj;     // Is object still connected?
			CDocProperty*			m_pSummPropList;// List of Summary Properties
			CDocProperty*			m_pDocPropList; // List of Doc Summary Properties
			WORD                    m_wCodePageSI;  // Code Page for SummPropList
			WORD                    m_wCodePageDSI; // Code Page for DocPropList
		};

		CSummaryProperties*		m_pSummProps;   // Summary Properties Object

		// Implementation
		HRESULT Open(BSTR sFileName, VARIANT_BOOL ReadOnly, dsoFileOpenOptions Options);
		HRESULT Close(VARIANT_BOOL SaveBeforeClose);
		HRESULT get_IsReadOnly(VARIANT_BOOL* pbReadOnly);
		HRESULT get_IsDirty(VARIANT_BOOL* pbDirty);
		HRESULT Save();
		HRESULT get_Icon(IDispatch** ppicIcon);
		HRESULT get_Name(BSTR* pbstrName);
		HRESULT get_Path(BSTR* pbstrPath);
		HRESULT get_IsOleFile(VARIANT_BOOL* pIsOleFile);
		HRESULT get_CLSID(BSTR* pbstrCLSID);
		HRESULT get_ProgID(BSTR* pbstrProgID);
		HRESULT get_OleDocumentFormat(BSTR* pbstrFormat);
		HRESULT get_OleDocumentType(BSTR* pbstrType);

	private:
		HRESULT get_SummaryProperties(CSummaryProperties** ppSummaryProperties);

	private:
		BSTR				    m_bstrFileName; // Filename of open document
		ULONG				    m_cFilePartIdx; // Path/Name Index
		IStorage*				m_pStorage;     // IStorage document pointer
		IPropertySetStorage*    m_pPropSetStg;  // Property Set Storage
		dsoFileOpenOptions      m_dwFlags;      // Open Flags
		BOOL		            m_fReadOnly;    // Should be read-only?
		WORD                    m_wCodePage;    // Code Page for MBCS/Unicode translation
	};

	CDocumentProperties*	m_pDocProps;

	static LPCWSTR	uriBook;
	static LPCWSTR	uriDocument;
	static LPCWSTR	uriSpreadsheet;
	static LPCWSTR	uriPresentation;
	static LPCWSTR	msWordExt;
	static LPCWSTR	msExcelExt;
	static LPCWSTR	msPPointExt;
	static LPCWSTR	msProjectExt;
	static LPCWSTR	msVisioExt;
	static LPCWSTR	ooWriterExt;
	static LPCWSTR	ooCalcExt;
	static LPCWSTR	ooImpressExt;

	// ILibraryBuilderPlugin Methods
public:
	STDMETHOD(Process)(BSTR sFile, ISXMLElement* pXML);

	// IImageServicePlugin Methods
public:
	STDMETHOD(LoadFromFile)(BSTR sFile, IMAGESERVICEDATA* pParams, SAFEARRAY** ppImage);
	STDMETHOD(LoadFromMemory)(BSTR sType, SAFEARRAY* pMemory, 
		IMAGESERVICEDATA* pParams, SAFEARRAY** ppImage);
	STDMETHOD(SaveToFile)(BSTR sFile, IMAGESERVICEDATA* pParams, SAFEARRAY* pImage);
	STDMETHOD(SaveToMemory)(BSTR sType, SAFEARRAY** ppMemory, 
		IMAGESERVICEDATA* pParams, SAFEARRAY* pImage);

private:
	STDMETHOD(ProcessMSDocument)(BSTR bsFile, ISXMLElement* pXML, LPCWSTR pszSchema, LPCWSTR pszFormat);
	STDMETHOD(ProcessNewMSDocument)(BSTR bsFile, ISXMLElement* pXML, LPCWSTR pszSchema, LPCWSTR pszFormat);
	STDMETHOD(ProcessOODocument)(BSTR bsFile, ISXMLElement* pXML, LPCWSTR pszSchema, LPCWSTR pszFormat);
	STDMETHOD(GetMSThumbnail)(BSTR bsFile, IMAGESERVICEDATA* pParams, SAFEARRAY** ppImage);
	STDMETHOD(GetOOThumbnail)(BSTR bsFile, IMAGESERVICEDATA* pParams, SAFEARRAY** ppImage);
	CComBSTR GetMetadataXML(unzFile pFile, const char* pszFile);

	HBITMAP GetBitmapFromMetaFile(PICTDESC pds, int nResolution, 
		WORD wBitsPerSample, BITMAPINFO **ppBI);
	HBITMAP GetBitmapFromEnhMetaFile(PICTDESC pds, int nResolution, 
		WORD wBitsPerSample, BITMAPINFO **ppBI);
public:
	LPWSTR GetDocumentFormat(LPCWSTR pszExt);
	LPWSTR GetSchema(BSTR sFile, LPCWSTR pszExt);

	inline static int CalculateDotsForHimetric(int nResolution, int nHimetricUnits)
	{
		return static_cast<int>( nResolution * nHimetricUnits * 0.01 / 25.4 );
	}
};

OBJECT_ENTRY_AUTO(__uuidof(DocReader), CDocReader)

// Old ImageServices
//const CLSID CLSID_PNGReader = { 0xD427C22F, 0x23FB, 0x4E51, { 0xA8, 0xB8, 0x70, 0xF2, 0x03, 0x6E, 0xD3, 0xBA } };

// GFLImageServices - {FF5FCD00-2C20-49D8-84F6-888D2E2C95DA}
const CLSID CLSID_PNGReader = { 0xFF5FCD00, 0x2C20, 0x49D8, { 0x84, 0xF6, 0x88, 0x8D, 0x2E, 0x2C, 0x95, 0xDA } };
