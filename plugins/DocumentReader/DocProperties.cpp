//
// DocProperties.cpp
//
//	Created by:		Rolandas Rudomanskis
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

#include "stdafx.h"
#include "Globals.h"
#include "DocumentReader.h"
#include <olectl.h>    //needed for OleCreatePictureIndirect

////////////////////////////////////////////////////////////////////////
// CDocumentProperties
//
////////////////////////////////////////////////////////////////////////
// Class Constructor/Destructor
//
CDocumentProperties::CDocumentProperties()
{
	ODS("CDocumentProperties::CDocumentProperties()\n");
	m_bstrFileName   = NULL;
	m_cFilePartIdx   = 0;
    m_pStorage       = NULL;
    m_pPropSetStg    = NULL;
    m_dwFlags        = dsoOptionDefault;
	m_fReadOnly		 = FALSE;
    m_wCodePage      = 0;
    m_pSummProps     = NULL;
}

CDocumentProperties::~CDocumentProperties(void)
{
    ODS("CDocumentProperties::~CDocumentProperties()\n");
    ASSERT(m_pStorage == NULL); // We should be closed before delete!
    if ( m_pStorage ) Close( VARIANT_FALSE );
}

////////////////////////////////////////////////////////////////////////
// Implementation
//
////////////////////////////////////////////////////////////////////////
// Open  -- Takes a full or relative file name and loads the property
//   set for the document. Handles both OLE and NTFS5 property sets.
//
HRESULT CDocumentProperties::Open(BSTR sFileName, VARIANT_BOOL ReadOnly, dsoFileOpenOptions Options)
{
    HRESULT hr;
    DWORD dwOpenMode;
    WCHAR wszFullName[MAX_PATH];
    ULONG ulIdx;
	
	EnterCritical();
 // Open method called. Ensure we don't have file already open...
	ODS("CDocumentProperties::Open\n");
    ASSERT(m_pStorage == NULL); // We should only load one at a time per object!
    CHECK_NULL_RETURN((m_pStorage == NULL), /*ReportError(*/E_DOCUMENTOPENED/*, NULL, m_pDispExcep)*/);

 // Validate the name passed and resolve to full path (if relative)...
    CHECK_NULL_RETURN(sFileName, E_INVALIDARG);
    if (!FFindQualifiedFileName(sFileName, wszFullName, &ulIdx))
        return /*ReportError(*/STG_E_INVALIDNAME/*, NULL, m_pDispExcep)*/;

 // Save file name and path index from SearchFile API...
    m_bstrFileName = SysAllocString(wszFullName);
    m_cFilePartIdx = ulIdx;
    if ((m_cFilePartIdx < 1) || (m_cFilePartIdx > SysStringLen(m_bstrFileName))) 
		m_cFilePartIdx = 0;

 // Set open mode flags based on ReadOnly flag (the exclusive access is required for
 // the IPropertySetStorage interface -- which sucks, but we can work around for OLE files)...
    m_fReadOnly = (ReadOnly != VARIANT_FALSE);
    m_dwFlags = Options;
    dwOpenMode = ((m_fReadOnly) ? (STGM_READ | STGM_SHARE_EXCLUSIVE) : (STGM_READWRITE | STGM_SHARE_EXCLUSIVE));

 // If the file is an OLE Storage DocFile...
    if (StgIsStorageFile(m_bstrFileName) == S_OK)
    {
     // Get the data from IStorage...
	    hr = StgOpenStorage(m_bstrFileName, NULL, dwOpenMode, NULL, 0, &m_pStorage);

     // If we failed to gain write access, try to just read access if caller allows
	 // it. This function will open the OLE file in transacted read mode, which
	 // covers cases where the file is in use or is on a read-only share. We can't
	 // save after the open so we force the read-only flag on...
        if (((hr == STG_E_ACCESSDENIED) || (hr == STG_E_SHAREVIOLATION)) && 
            (m_dwFlags & dsoOptionOpenReadOnlyIfNoWriteAccess))
        {
            m_fReadOnly = TRUE;
	        hr = StgOpenStorage(m_bstrFileName, NULL, 
				(STGM_READ | STGM_TRANSACTED | STGM_SHARE_DENY_NONE), NULL, 0, &m_pStorage);
        }
        
	 // If we are lucky, we have a storage to read from, so ask OLE to open the 
	 // associated property set for the file and return the IPSS iface...
	    if (SUCCEEDED(hr))
        {
            hr = m_pStorage->QueryInterface(IID_IPropertySetStorage, (void**)&m_pPropSetStg);
        }
    }
    else if ((v_pfnStgOpenStorageEx) && 
             ((m_dwFlags & dsoOptionOnlyOpenOLEFiles) != dsoOptionOnlyOpenOLEFiles))
    {
     // On Win2K+ we can try and open plain files on NTFS 5.0 drive and get 
     // the NTFS version of OLE properties (saved in alt stream)...
        hr = (v_pfnStgOpenStorageEx)(m_bstrFileName, dwOpenMode, STGFMT_FILE, 0, NULL, 0, 
                IID_IPropertySetStorage, (void**)&m_pPropSetStg);

     // If we failed to gain write access, try to just read access if caller
     // wants us to. This only works for access block, not share violations...
       if ((hr == STG_E_ACCESSDENIED) && (!m_fReadOnly) && 
            (m_dwFlags & dsoOptionOpenReadOnlyIfNoWriteAccess))
        {
            m_fReadOnly = TRUE;
            hr = (v_pfnStgOpenStorageEx)(m_bstrFileName, (STGM_READ | STGM_SHARE_EXCLUSIVE), STGFMT_FILE,
                0, NULL, 0, IID_IPropertySetStorage, (void**)&m_pPropSetStg);
        }
    }
    else
    {  // If we land here, the file is non-OLE file, and not on NTFS5 drive,
	   // so we return an error that file has no valid OLE/NTFS extended properties...
        hr = E_NODOCUMENTPROPS; 
    }

    if ( FAILED(hr) )
    {
        //ReportError(hr, NULL, m_pDispExcep);
        Close( VARIANT_FALSE ); // Force a cleanup on error...
    }

	LeaveCritical();
    return hr;
}

////////////////////////////////////////////////////////////////////////
// Close  --  Close the open document (optional save before close)
//
HRESULT CDocumentProperties::Close(VARIANT_BOOL SaveBeforeClose)
{
	ODS("CDocumentProperties::Close\n");

 // If caller requests full save on close, try it. Note that this is the
 // only place where Close will return an error (and NOT close)...
    if (SaveBeforeClose != VARIANT_FALSE)
    {
        HRESULT hr = Save();
        RETURN_ON_FAILURE(hr);
    }

 // The rest is just cleanup to restore us back to state where
 // we can be called again. The Zombie call disconnects sub objects
 // and should free them if caller has also released them...
    ZOMBIE_OBJECT(m_pSummProps);
    
    m_pPropSetStg->Release();
	m_pPropSetStg = NULL;
    m_pStorage->Release();
	m_pStorage = NULL;
    FREE_BSTR(m_bstrFileName);
    m_cFilePartIdx = 0;
    m_dwFlags = dsoOptionDefault;
    m_fReadOnly = FALSE;

    return S_OK;
}

////////////////////////////////////////////////////////////////////////
// get_IsReadOnly - Returns User-Friendly Name for File Type
//
HRESULT CDocumentProperties::get_IsReadOnly(VARIANT_BOOL* pbReadOnly)
{
	ODS("CDocumentProperties::get_IsReadOnly\n");
	CHECK_NULL_RETURN(pbReadOnly,  E_POINTER); 
    *pbReadOnly = ((m_fReadOnly) ? VARIANT_TRUE : VARIANT_FALSE);
    return S_OK;
}

////////////////////////////////////////////////////////////////////////
// get_IsDirty  -- Have any changes been made to the properties?
//
HRESULT CDocumentProperties::get_IsDirty(VARIANT_BOOL* pbDirty)
{
    BOOL fDirty = FALSE;
 	ODS("CDocumentProperties::get_IsDirty\n");

 // Check the status of summary properties...
    if ((m_pSummProps) && (m_pSummProps->FIsDirty()))
        fDirty = TRUE;

    if (pbDirty) // Return status to caller...
        *pbDirty = (VARIANT_BOOL)((fDirty) ? VARIANT_TRUE : VARIANT_FALSE);
 
    return S_OK;
}

////////////////////////////////////////////////////////////////////////
// Save  --  Will save the changes made back to the document.
//
HRESULT CDocumentProperties::Save()
{
    HRESULT hr = S_FALSE;
    BOOL fSaveMade = FALSE;

 	ODS("CDocumentProperties::Save\n");
    CHECK_FLAG_RETURN(m_fReadOnly, /*ReportError(*/E_DOCUMENTREADONLY/*, NULL, m_pDispExcep)*/);

 // Ask SummaryProperties to save its changes...
    if (m_pSummProps)
    {
        hr = m_pSummProps->SaveProperties(TRUE);
        if (FAILED(hr)) return /*ReportError(*/hr/*, NULL, m_pDispExcep)*/;
        fSaveMade = (hr == S_OK);
    }

 // If save was made, commit the root storage before return...
    if ((fSaveMade) && (m_pStorage))
    {
        hr = m_pStorage->Commit(STGC_DEFAULT);
    }

    return hr;
}

////////////////////////////////////////////////////////////////////////
// get_SummaryProperties - Returns SummaryProperties object
//
HRESULT CDocumentProperties::get_SummaryProperties(CSummaryProperties** ppSummaryProperties)
{
    HRESULT hr;

 	ODS("CDocumentProperties::get_SummaryProperties\n");
    CHECK_NULL_RETURN(ppSummaryProperties,  E_POINTER);
    *ppSummaryProperties = NULL;

    if (m_pSummProps == NULL)
    {
        m_pSummProps = new CSummaryProperties();
        if (m_pSummProps)
            { hr = m_pSummProps->LoadProperties(m_pPropSetStg, m_fReadOnly, m_dwFlags); }
        else hr = E_OUTOFMEMORY;

        if (FAILED(hr))
        {
            ZOMBIE_OBJECT(m_pSummProps);
            return /*ReportError(*/hr/*, NULL, m_pDispExcep)*/;
        }
    }
	*ppSummaryProperties = m_pSummProps;
    //hr = m_pSummProps->QueryInterface(IID_SummaryProperties, (void**)ppSummaryProperties);
    return hr;
}

////////////////////////////////////////////////////////////////////////
// get_Icon - Returns OLE StdPicture object with associated icon 
//
HRESULT CDocumentProperties::get_Icon(IDispatch** ppicIcon)
{
	HICON hIco;

	ODS("CDocumentProperties::get_Icon\n");
	CHECK_NULL_RETURN(ppicIcon,  E_POINTER); *ppicIcon = NULL;
    CHECK_NULL_RETURN(m_pPropSetStg, /*ReportError(*/E_DOCUMENTNOTOPEN/*, NULL, m_pDispExcep)*/);

    if ((m_bstrFileName) && FGetIconForFile(m_bstrFileName, &hIco))
    {
		PICTDESC  icoDesc;
		icoDesc.cbSizeofstruct = sizeof(PICTDESC);
		icoDesc.picType = PICTYPE_ICON;
		icoDesc.icon.hicon = hIco;
		return OleCreatePictureIndirect(&icoDesc, IID_IDispatch, TRUE, (void**)ppicIcon);
    }
    return S_FALSE;
}

////////////////////////////////////////////////////////////////////////
// get_Name - Returns the name of the file (no path)
//
HRESULT CDocumentProperties::get_Name(BSTR* pbstrName)
{
	ODS("CDocumentProperties::get_Name\n");
	CHECK_NULL_RETURN(pbstrName,  E_POINTER); *pbstrName = NULL;
    CHECK_NULL_RETURN(m_pPropSetStg, /*ReportError(*/E_DOCUMENTNOTOPEN/*, NULL, m_pDispExcep)*/);

	if (m_bstrFileName != NULL && m_cFilePartIdx > 0)
		*pbstrName = SysAllocString((LPOLESTR)&(m_bstrFileName[m_cFilePartIdx]));

	return S_OK;
}

////////////////////////////////////////////////////////////////////////
// get_Name - Returns the path to the file (no name)
//
HRESULT CDocumentProperties::get_Path(BSTR* pbstrPath)
{
	ODS("CDocumentProperties::get_Path\n");
	CHECK_NULL_RETURN(pbstrPath,  E_POINTER); *pbstrPath = NULL;
    CHECK_NULL_RETURN(m_pPropSetStg, /*ReportError(*/E_DOCUMENTNOTOPEN/*, NULL, m_pDispExcep)*/);

	if (m_bstrFileName != NULL && m_cFilePartIdx > 0)
	    *pbstrPath = SysAllocStringLen(m_bstrFileName, m_cFilePartIdx);
	
	return S_OK;
}

////////////////////////////////////////////////////////////////////////
// get_IsOleFile - Returns True if file is OLE DocFile
//
HRESULT CDocumentProperties::get_IsOleFile(VARIANT_BOOL* pIsOleFile)
{
	ODS("CDocumentProperties::get_IsOleFile\n");
	CHECK_NULL_RETURN(pIsOleFile,  E_POINTER);
    *pIsOleFile = ((m_pStorage) ? VARIANT_TRUE : VARIANT_FALSE);
    return S_OK;
}

////////////////////////////////////////////////////////////////////////
// get_Name - Returns CLSID of OLE DocFile 
//
HRESULT CDocumentProperties::get_CLSID(BSTR* pbstrCLSID)
{
    HRESULT hr;
	STATSTG stat;
	LPOLESTR pwszCLSID = NULL;

	ODS("CDocumentProperties::get_CLSID\n");
	CHECK_NULL_RETURN(pbstrCLSID,  E_POINTER); *pbstrCLSID = NULL;
    CHECK_NULL_RETURN(m_pPropSetStg, /*ReportError(*/E_DOCUMENTNOTOPEN/*, NULL, m_pDispExcep)*/);
    CHECK_NULL_RETURN(m_pStorage, /*ReportError(*/E_MUSTHAVESTORAGE/*, NULL, m_pDispExcep)*/);

    memset(&stat, 0, sizeof(stat));
    hr = m_pStorage->Stat(&stat, STATFLAG_NONAME);
    RETURN_ON_FAILURE(hr);

	hr = StringFromCLSID(stat.clsid, &pwszCLSID);
    if (SUCCEEDED(hr)) *pbstrCLSID = SysAllocString(pwszCLSID);

    FREE_COTASKMEM(pwszCLSID);
	return hr;
}

////////////////////////////////////////////////////////////////////////
// get_ProgID - Returns ProgID of OLE DocFile 
//
HRESULT CDocumentProperties::get_ProgID(BSTR* pbstrProgID)
{
    HRESULT hr;
	STATSTG stat;
	LPOLESTR pwszProgID = NULL;

	ODS("CDocumentProperties::get_ProgID\n");
	CHECK_NULL_RETURN(pbstrProgID,  E_POINTER); *pbstrProgID = NULL;
    CHECK_NULL_RETURN(m_pPropSetStg, /*ReportError(*/E_DOCUMENTNOTOPEN/*, NULL, m_pDispExcep)*/);
    CHECK_NULL_RETURN(m_pStorage, /*ReportError(*/E_MUSTHAVESTORAGE/*, NULL, m_pDispExcep)*/);

    memset(&stat, 0, sizeof(stat));
    hr = m_pStorage->Stat(&stat, STATFLAG_NONAME);
    RETURN_ON_FAILURE(hr);

	hr = ProgIDFromCLSID(stat.clsid, &pwszProgID);
	if (SUCCEEDED(hr)) *pbstrProgID = SysAllocString(pwszProgID);

    FREE_COTASKMEM(pwszProgID);
	return hr;
}

////////////////////////////////////////////////////////////////////////
// get_OleDocumentFormat - Returns ClipFormat of OLE DocFile 
//
HRESULT CDocumentProperties::get_OleDocumentFormat(BSTR* pbstrFormat)
{
    HRESULT hr = S_FALSE;
    CLIPFORMAT cf;

	ODS("CDocumentProperties::get_OleDocumentFormat\n");
	CHECK_NULL_RETURN(pbstrFormat,  E_POINTER); *pbstrFormat = NULL;
    CHECK_NULL_RETURN(m_pPropSetStg, /*ReportError(*/E_DOCUMENTNOTOPEN/*, NULL, m_pDispExcep)*/);
    CHECK_NULL_RETURN(m_pStorage, /*ReportError(*/E_MUSTHAVESTORAGE/*, NULL, m_pDispExcep)*/);

    if (SUCCEEDED(ReadFmtUserTypeStg(m_pStorage, &cf, NULL)) == TRUE)
    {
        int i;
        CHAR szName[MAX_PATH] = {0};

        if ((i = GetClipboardFormatName(cf, szName, MAX_PATH)) > 0)
        {
            szName[i] = '\0';
        }
        else
        {
            wsprintf(szName, "ClipFormat 0x%X (%d)", cf, cf);
        }
        *pbstrFormat = ConvertToBSTR(szName, CP_ACP);
        hr = ((*pbstrFormat) ? S_OK : E_OUTOFMEMORY);
    }

    return hr;
}

////////////////////////////////////////////////////////////////////////
// get_OleDocumentType - Returns User-Friendly Name for File Type
//
HRESULT CDocumentProperties::get_OleDocumentType(BSTR* pbstrType)
{
    HRESULT hr = S_FALSE;;
    LPWSTR lpolestr = NULL;

	ODS("CDocumentProperties::get_OleDocumentType\n");
	CHECK_NULL_RETURN(pbstrType,  E_POINTER); *pbstrType = NULL;
    CHECK_NULL_RETURN(m_pPropSetStg, /*ReportError(*/E_DOCUMENTNOTOPEN/*, NULL, m_pDispExcep)*/);
    CHECK_NULL_RETURN(m_pStorage, /*ReportError(*/E_MUSTHAVESTORAGE/*, NULL, m_pDispExcep)*/);

    if (SUCCEEDED(ReadFmtUserTypeStg(m_pStorage, NULL, &lpolestr)) == TRUE)
    {
        *pbstrType = SysAllocString(lpolestr);
        hr = ((*pbstrType) ? S_OK : E_OUTOFMEMORY);
        FREE_COTASKMEM(lpolestr);
    }

    return hr;
}
