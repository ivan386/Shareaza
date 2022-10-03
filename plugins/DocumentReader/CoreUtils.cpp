//
// CoreUtils.cpp
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

#include "stdafx.h"

////////////////////////////////////////////////////////////////////
// Core PropertySet Functions
//
////////////////////////////////////////////////////////////////////
// OpenPropertyStorage
//
//  Function takes a PropertySetStorage and returns the desired 
//  PropertyStorage for the FMTID. The function will create a storage
//  if one does not exist (and flags allow).
//
STDAPI OpenPropertyStorage(IPropertySetStorage* pPropSS, REFFMTID fmtid, BOOL fReadOnly, DWORD dwFlags, IPropertyStorage** ppPropStg)
{
    HRESULT hr;
    DWORD dwMode;
    BOOL fNoCreate = ((dwFlags & dsoOptionDontAutoCreate) == dsoOptionDontAutoCreate);
    BOOL fUseANSI = ((dwFlags & dsoOptionUseMBCStringsForNewSets) == dsoOptionUseMBCStringsForNewSets);

    ASSERT(pPropSS); ASSERT(ppPropStg);
    if ((pPropSS == NULL) || (ppPropStg == NULL))
        return E_UNEXPECTED;

    *ppPropStg = NULL;

 // Set the access mode for read/write access...
	dwMode = (fReadOnly ? (STGM_READ | STGM_SHARE_EXCLUSIVE)
					    : (STGM_READWRITE | STGM_SHARE_EXCLUSIVE));

 // We try to open the property set. If this fails, it may be beacuse
 // it doesn't exist so we'll try to create the set...
	hr = pPropSS->Open(fmtid, dwMode, ppPropStg);
	if ((hr == STG_E_FILENOTFOUND) && !fReadOnly && !fNoCreate)
	{
     // FIX -- ADDED BY REQUEST - Feb 1, 2001
     // Outlook 2000/XP doesn't handle Unicode property sets very well. So if we need to
     // create a propset for the caller, allow the caller the ability to set the
     // PROPSETFLAG_ANSI flag on the new set.
     // 
     // The ANSI flag was the default in File 1.0, but this was changed to Unicode 
     // for version 1.2 to meet request by ASIA/EMEA clients. Unicode should work 
     // but there have been reported problems (in Outlook, Win2K SP2) that indicate
     // clients may want to use the ANSI flag (so it is passed here)...
		hr = pPropSS->Create(fmtid, NULL, (fUseANSI ? PROPSETFLAG_ANSI : 0), dwMode, ppPropStg);

    // If we created with ANSI flag, we must set the code page value to match ACP...
        if (SUCCEEDED(hr) && (fUseANSI) && (*ppPropStg))
		{
			VARIANT vtT; 
            PROPSPEC spc;
            spc.ulKind = PRSPEC_PROPID; spc.propid = PID_CODEPAGE;

         // FIX -- ADDED BY REQUEST - Oct 30, 2001
         // Check for CodePage first. It appears certain configurations choke on modification
         // of the code page. This appears to be a change in OLE32 behavior. Workaround is to
         // check to see if OLE32 has added the code page already during the create, and if so
         // we can skip out on adding it ourselves...
            if (FAILED(ReadProperty(*ppPropStg, spc, 0, &vtT)))
            { // If not, we should add it...
                vtT.vt = VT_I4; vtT.lVal = GetACP();
			    WriteProperty(*ppPropStg, spc, 0, &vtT);
            }
        }
        
	}

	return hr;
}

////////////////////////////////////////////////////////////////////
// ConvertBlobToVarVector
//
//  Takes a PROPVARIANT BLOB or pclipdata and converts it to VARIANT SAFEARRAY 
//  which can be treated by VB as vector (1-dim) Byte Array.
//
STDAPI ConvertBinaryToVarVector(PROPVARIANT *pVarBlob, VARIANT *pVarByteArray)
{
    HRESULT hr = S_FALSE;
    SAFEARRAY* pSA;
    DWORD dwSize;

    if ( ( pVarBlob == NULL ) || (pVarBlob->vt != VT_BLOB) && 
		 ( pVarBlob->vt != VT_CF ) || ( pVarByteArray == NULL ) )
        return E_UNEXPECTED;

 // Identify the size
    if ( pVarBlob->vt == VT_BLOB ) 
		dwSize = pVarBlob->blob.cbSize;
	else
		dwSize = pVarBlob->pclipdata->cbSize;

    if ((dwSize) && (dwSize < 0x800000))
    {
     // Create a vector array the size of the blob or clipdata...
        pSA = SafeArrayCreateVector(VT_UI1, 0, dwSize);
        if ( pSA != NULL )
        {
         // Copy the data over to the vector
            BYTE *pByte = NULL;
            hr = SafeArrayAccessData( pSA, (void**)&pByte );
            if ( SUCCEEDED(hr) )
            {
                SEH_TRY
				if ( pVarBlob->vt == VT_BLOB )
					memcpy( pByte, (BYTE*)(pVarBlob->blob.pBlobData), dwSize );
				else
					memcpy( pByte, (BYTE*)(pVarBlob->pclipdata->pClipData), dwSize );
                SEH_EXCEPT(hr)
                SafeArrayUnaccessData( pSA );
            }
        }

        if ( (pSA) && SUCCEEDED(hr) && (pVarByteArray) )
        {
            pVarByteArray->vt = (VT_ARRAY | VT_UI1);
            pVarByteArray->parray = pSA;
        }
        else if ( pSA ) SafeArrayDestroy( pSA );
    }

    return hr;
}

////////////////////////////////////////////////////////////////////
// ReadProperty
//
//  Reads a single property from a given PropertyStorage. Code page is
//  used if we have to translate to/from MBCS to Unicode. We handle one special
//  case for PIDSI_EDITTIME which is a INT64 saved in FILETIME structure. For
//  compatibility we cut it down to seconds and store in LONG (VT_I4). This is
//  OK as long as we don't save it back (which we don't allow in this sample). 
//
STDAPI ReadProperty(IPropertyStorage* pPropStg, PROPSPEC spc, WORD wCodePage, VARIANT* pvtResult)
{
	HRESULT     hr;
    PROPVARIANT vtProperty;

	if ((pPropStg == NULL) || (pvtResult == NULL))
		return E_POINTER;

 // Initialize PROPVARIANT...
	PropVariantInit(&vtProperty);

 // Make the call to read the property from the set...
	SEH_TRY

	pvtResult->vt = VT_EMPTY; pvtResult->lVal = 0;
	hr = pPropStg->ReadMultiple(1, &spc, &vtProperty);

	SEH_EXCEPT(hr)

 // If the call succeeded, swap the data into a VARIANT...
    if (SUCCEEDED(hr))
    {
     // Make a selected copy based on the type...
	    switch (vtProperty.vt)
	    {
	    case VT_I4:
	    case VT_UI4: pvtResult->vt = VT_I4; pvtResult->lVal = vtProperty.lVal;
            break;

	    case VT_I2:
	    case VT_UI2: pvtResult->vt = VT_I4;  pvtResult->lVal = vtProperty.iVal;
		    break;

	    case VT_BSTR:
		    pvtResult->vt = VT_BSTR;
            pvtResult->bstrVal = ((vtProperty.bstrVal) ? SysAllocString(vtProperty.bstrVal) : NULL);
		    break;

	    case VT_LPWSTR:
		    pvtResult->vt = VT_BSTR;
            pvtResult->bstrVal = ((vtProperty.pwszVal) ? SysAllocString(vtProperty.pwszVal) : NULL);
		    break;

	    case VT_LPSTR:
		    pvtResult->vt = VT_BSTR;
            pvtResult->bstrVal = ConvertAToBSTR(vtProperty.pszVal, wCodePage);
		    break;

	    case VT_FILETIME:
            // Check fo special case of edit time...
		    if ((spc.ulKind == PRSPEC_PROPID) && (spc.propid == PIDSI_EDITTIME))
		    {
			    unsigned __int64 ns, secs;
			    ////////////////////////////////////////////////
			    // FIX - 9/27/99 Assign to unsigned __int64 first, then shift...
			    // ns = ft.dwLowDateTime + (ft.dwHighDateTime << 32);
			    //
			    ns = vtProperty.filetime.dwHighDateTime; ns <<= 32;
			    ns += vtProperty.filetime.dwLowDateTime;
			    secs = ns / (10000000);

			    pvtResult->vt = VT_I4;
			    pvtResult->lVal = (LONG)((DWORD)(secs / 60));
		    }
		    else
		    {
			    DATE       dtDate;
			    FILETIME   lft;
			    SYSTEMTIME lst;
			    FILETIME*  pft = &(vtProperty.filetime);

			    if (!((pft->dwLowDateTime == 0) && (pft->dwHighDateTime == 0)))
			    {
				    if (FileTimeToLocalFileTime(pft, &lft))
					    pft = &lft;

				    if (FileTimeToSystemTime(pft, &lst) && 
					    SystemTimeToVariantTime(&lst, &dtDate))
				    {
					    pvtResult->vt = VT_DATE;
					    pvtResult->date = dtDate;
				    }
			    }
		    }
		    break;

	    case VT_BOOL:
		    pvtResult->vt = VT_BOOL; pvtResult->boolVal = vtProperty.boolVal;
		    break;

	    case VT_R4:
		    pvtResult->vt = VT_R4; pvtResult->fltVal = vtProperty.fltVal;
		    break;

	    case VT_R8:
		    pvtResult->vt = VT_R8; pvtResult->dblVal = vtProperty.dblVal;
		    break;

        case VT_CF:
            ConvertBinaryToVarVector(&vtProperty, pvtResult);
            break;

        case VT_BLOB:
            ConvertBinaryToVarVector(&vtProperty, pvtResult);
            break;

        default:
            hr = STG_E_INVALIDPARAMETER;
            break;
	    }

    }

 // Clear PropVariant and return...
	PropVariantClear(&vtProperty);
	return hr;
}

////////////////////////////////////////////////////////////////////
// WriteProperty
//
//  Writes a property to the given PropertyStorage. The code page parameter
//  is used to convert string into code page of the property set itself only 
//  if the PROPSETFLAG_ANSI is set. Otherwise we save in Unicode.
//
STDAPI WriteProperty(IPropertyStorage* pPropStg, PROPSPEC spc, WORD wCodePage, VARIANT* pvtValue)
{
	HRESULT        hr;
	PROPVARIANT    vtProperty;
	STATPROPSETSTG statstg;
	BOOL           fUseANSI = FALSE;

 // Check the storage and discover whether it is ANSI only...
	SEH_TRY

	if (SUCCEEDED(pPropStg->Stat(&statstg)))
		fUseANSI = ((statstg.grfFlags & PROPSETFLAG_ANSI) == PROPSETFLAG_ANSI);

	SEH_EXCEPT(hr)

 // We only support certain Variant types...
	switch (pvtValue->vt)
	{
	case VT_I4:
	case VT_UI4:
		vtProperty.vt = VT_I4; vtProperty.lVal = pvtValue->lVal;
		break;

	case VT_I2:
	case VT_UI2:
		vtProperty.vt = VT_I2; vtProperty.iVal = pvtValue->iVal;
		break;

	case VT_BOOL:
		vtProperty.vt = VT_BOOL; vtProperty.boolVal = pvtValue->boolVal;
		break;

	case VT_BSTR:
		if (fUseANSI) // When using ANSI propset, convert to local code page...
		{
			vtProperty.vt = VT_LPSTR;
			vtProperty.pszVal = ConvertToMBCS(pvtValue->bstrVal, wCodePage);
		}
		else // Otherwise we save the (Unicode) BSTR...
		{
		  /////////////////////////////////////////////////////////////////////
		  // BUG (6/30/01): Changed from saving directly as BSTR to LPWSTR since
		  // Win2K SP2 introduced bug with VT_BSTR types and does not show them
		  // correctly in the UI. We just copy string before handing to OLE.
			vtProperty.vt = VT_LPWSTR;
			vtProperty.pwszVal = ConvertToCoTaskMemStr(pvtValue->bstrVal);
		}
		break;

	case VT_DATE: // Date/time values should always be saved as UTC...
		{
			FILETIME utc = {0,0};
			FILETIME lft;
			SYSTEMTIME lst;
			if ((0 != pvtValue->date) && 
				(VariantTimeToSystemTime(pvtValue->date, &lst)) &&
				(SystemTimeToFileTime(&lst, &lft)))
			{
				if (!LocalFileTimeToFileTime(&lft, &utc))
					utc = lft;
			}
			vtProperty.vt = VT_FILETIME;
			vtProperty.filetime = utc;
		}
		break;

	case VT_R4:
		vtProperty.vt = VT_R4;
		vtProperty.fltVal = pvtValue->fltVal;
		break;

	case VT_R8:
		vtProperty.vt = VT_R8;
		vtProperty.dblVal = pvtValue->dblVal;
		break;

	default:
		return E_INVALIDARG; //unsupportted type...
	}

 // Do the Write operation to the given IPropertySet...
	SEH_TRY
    hr = pPropStg->WriteMultiple(1, &spc, &vtProperty, ((spc.ulKind == PRSPEC_LPWSTR) ? 0x2001 : NULL));
    SEH_EXCEPT(hr)

	PropVariantClear(&vtProperty);
	return hr;
}

////////////////////////////////////////////////////////////////////
// VarTypeReadable
//
//  Returns TRUE if PROPVARIANT VARTYPE is readable by our basic read
//  function ReadProperty. If type is not supported, we should skip it.

STDAPI_(BOOL) VarTypeReadable(VARTYPE vt)
{
    BOOL fReadable = FALSE;
    switch (vt)
    {
	    case VT_I4:
	    case VT_UI4:
	    case VT_I2:
	    case VT_UI2:
	    case VT_BSTR:
	    case VT_LPWSTR:
	    case VT_LPSTR:
	    case VT_FILETIME:
	    case VT_BOOL:
	    case VT_R4:
	    case VT_R8:
        case VT_CF:
        case VT_BLOB: fReadable = TRUE;  break;
    }
    return fReadable;
}

////////////////////////////////////////////////////////////////////
// LoadPropertySetList
//
//   This function take an IPropertyStorage and enumerates all the properties
//   to create a linked list of CDocProperty objects. The linked list is used
//   to cache the data and manipulate it before a save. 
//
//   FUTURE: We ought to consider building prop list first, then call ReadMulitple 
//   with PROPSPEC array to fill in the data in one shot. However, given the properties
//   we allow are realtively small in size, we have not seen a performance benefit to
//   make this worth the extra code and regression risk.
//
STDAPI LoadPropertySetList(IPropertyStorage *pPropStg, WORD *pwCodePage, CDocProperty** pplist, BOOL bOnlyThumb)
{
	HRESULT hr;
    CDocProperty* pList = NULL;
    CDocProperty* pLastItem = NULL;
	IEnumSTATPROPSTG* pEnumProp = NULL;
	ULONG fetched;
	STATPROPSTG sps;
	VARIANT vtCodePage;
	VARIANT vtItem;
	BSTR bstrName;
    PROPSPEC spc;
    WORD wCodePage = 0;

    if ((pPropStg == NULL) || (pplist == NULL))
        return E_UNEXPECTED;

    memset(&sps, 0, sizeof(sps));
    memset(&spc, 0, sizeof(spc));

 // Get Code page for this storage...
    spc.ulKind = PRSPEC_PROPID;
    spc.propid = PID_CODEPAGE;
	if (SUCCEEDED(ReadProperty(pPropStg, spc, wCodePage, &vtCodePage)) &&
		((vtCodePage.vt == VT_I4) || (vtCodePage.vt == VT_I2)))
	{
		wCodePage = LOWORD(vtCodePage.lVal);
	}

 // Handle exceptions as fatal events...
	SEH_TRY
    *pplist = NULL;

 // Get the property enumerator to see what properties are stored...
	hr = pPropStg->Enum( &pEnumProp );
    if ( SUCCEEDED(hr) && (pEnumProp) )
    {
		while ( SUCCEEDED(hr) && ( pEnumProp->Next( 1, &sps, &fetched ) == S_OK ) )
		{
         // We don't handle VECTOR data in this sample. And the PROPVARIANT
         // data types we handle are limited to just a subset we can convert
         // to VB supportted types (variant arrays and ole picdisp)...
			BOOL bDontSkip = !( ( ( sps.vt & VT_CF ) == VT_CF ) ^ bOnlyThumb );
            if ( ( ( sps.vt & VT_VECTOR ) != VT_VECTOR ) && 
                VarTypeReadable( (VARTYPE)(sps.vt & VT_TYPEMASK) ) && bDontSkip )
            {
                spc.ulKind = PRSPEC_PROPID;
                spc.propid = sps.propid;

             // Read in the property based on the PROPID...
			    hr = ReadProperty(pPropStg, spc, wCodePage, &vtItem);
			    if (SUCCEEDED(hr))
			    {
                 // If we got the data, make the property object to hold it
                 // and append last item to link the list (
                    bstrName = ((sps.lpwstrName) ? SysAllocString(sps.lpwstrName) : NULL);
				    pLastItem = pList;

                    pList = CDocProperty::CreateObject(bstrName, spc.propid, &vtItem, FALSE, pLastItem);
					if (pList == NULL) { hr = E_OUTOFMEMORY; pList = pLastItem;}

				    if (bstrName) SysFreeString(bstrName);
				    VariantClear(&vtItem);
			    }
            }
            // else we just skip it...

			if (sps.lpwstrName)
				CoTaskMemFree(sps.lpwstrName);
		}

        if (SUCCEEDED(hr)) // If here, we loaded all items fine
        {
            *pplist = pList;
            if (pwCodePage) *pwCodePage = wCodePage;
        }
        else
        { // If not, try to clean up...
            while (pList)
            {
                pLastItem = pList->GetNextProperty();
                pList->Disconnect(); pList = pLastItem;
            }
        }
    }

    SEH_EXCEPT(hr)

 // Release obtained interface.
	pEnumProp->Release();
	pEnumProp = NULL;
	return hr;
}

////////////////////////////////////////////////////////////////////
// SavePropertySetList
//
//   Takes a linked list of CDocProperty objects and writes those that
//   have changed back to the IPropertyStorage. It will also call DeleteMultiple
//   on any item marked as deleted on save.
//
STDAPI SavePropertySetList(IPropertyStorage *pPropStg, WORD wCodePage, CDocProperty* plist, ULONG *pcSavedItems)
{
    HRESULT hr = S_FALSE;
    CDocProperty* pitem = plist;
    VARIANT *pvt;
    BSTR bstrName = NULL;
    ULONG cItemsChanged = 0;
    PROPSPEC spc;

    if ((pPropStg == NULL) || (plist == NULL))
        return E_UNEXPECTED;

 // Loop through each item in the list...
    while (pitem)
    {
     // If the item is removed, remove it from the document...
        if (pitem->IsRemoved())
        {
         // We only need to remove it if it already exists. If
         // this is an item wehen added then deleted before save,
         // we don't need to do anything...
            if (pitem->IsNewItem() == FALSE)
            {                
             // Determine if item is known by name or by id...
                pitem->get_Name(&bstrName);
                if (bstrName)
                {
                    spc.ulKind = PRSPEC_LPWSTR;
                    spc.lpwstr = bstrName;
                }
                else
                {
                    spc.ulKind = PRSPEC_PROPID;
                    spc.propid = pitem->GetID();

                    if (spc.propid == 0)
                        { hr = E_UNEXPECTED; break; }
                }
            
             // Now remove the item...
                hr = pPropStg->DeleteMultiple(1, &spc);

             // Break out if error occurred...
                if (FAILED(hr)) break;

             // Since we changed an item in the file, we need 
             // to increment the count...
                pitem->OnRemoveComplete();
                ++cItemsChanged;
            }
        }
        else if (pitem->IsDirty())
        {
     // If the item is dirty, try to save it now...
            pvt = pitem->GetDataPtr();
            if ((pvt) && (pvt->vt != VT_EMPTY))
            {
             // Determine if we should save by name or by id...
                pitem->get_Name(&bstrName);
                if (bstrName)
                {
                    spc.ulKind = PRSPEC_LPWSTR;
                    spc.lpwstr = bstrName;
                }
                else
                {
                    spc.ulKind = PRSPEC_PROPID;
                    spc.propid = pitem->GetID();

                    if (spc.propid == 0)
                        { hr = E_UNEXPECTED; break; }
                }

             // Write the property to the property set...
                hr = WriteProperty(pPropStg, spc, wCodePage, pvt);

                FREE_BSTR(bstrName);

             // Break out if error occurred...
                if (FAILED(hr)) break;

             // Notify object that it was saved, and bump up
             // the modified item count...
                pitem->OnSaveComplete();
                ++cItemsChanged;
            }
        }

        pitem = pitem->GetNextProperty();
    }

    if (pcSavedItems)
        *pcSavedItems = cItemsChanged;

    return hr;
}

////////////////////////////////////////////////////////////////////////
// Heap Allocation (Uses CoTaskMemAlloc)
//
STDAPI_(LPVOID) MemAlloc(DWORD cbSize)
{
    CHECK_NULL_RETURN(v_hPrivateHeap, NULL);
    return HeapAlloc(v_hPrivateHeap, 0, cbSize);
}

STDAPI_(void) MemFree(LPVOID ptr)
{
    if ((v_hPrivateHeap) && (ptr))
        HeapFree(v_hPrivateHeap, 0, ptr);
}
/*
void * _cdecl operator new(size_t size){ return MemAlloc(size);}
void  _cdecl operator delete(void *ptr){ MemFree(ptr); }
*/
////////////////////////////////////////////////////////////////////////
// String Manipulation Functions
//
////////////////////////////////////////////////////////////////////////
// ConvertToUnicodeEx
//
STDAPI ConvertToUnicodeEx(LPCSTR pszMbcsString, DWORD cbMbcsLen, LPWSTR pwszUnicode, DWORD cbUniLen, WORD wCodePage)
{
	DWORD cbRet;
	UINT iCode = CP_ACP;

	if (IsValidCodePage((UINT)wCodePage))
		iCode = (UINT)wCodePage;

	CHECK_NULL_RETURN(pwszUnicode,    E_POINTER);
	pwszUnicode[0] = L'\0';

	CHECK_NULL_RETURN(pszMbcsString,  E_POINTER);
	CHECK_NULL_RETURN(cbMbcsLen,      E_INVALIDARG);
	CHECK_NULL_RETURN(cbUniLen,       E_INVALIDARG);

	cbRet = MultiByteToWideChar(iCode, 0, pszMbcsString, cbMbcsLen, pwszUnicode, cbUniLen);
	if (cbRet == 0)	return HRESULT_FROM_WIN32(GetLastError());

	pwszUnicode[cbRet] = L'\0';
	return S_OK;
}

////////////////////////////////////////////////////////////////////////
// ConvertToMBCSEx
//
STDAPI ConvertToMBCSEx(LPCWSTR pwszUnicodeString, DWORD cbUniLen, LPSTR pszMbcsString, DWORD cbMbcsLen, WORD wCodePage)
{
	DWORD cbRet;
	UINT iCode = CP_ACP;

	if (IsValidCodePage((UINT)wCodePage))
		iCode = (UINT)wCodePage;

	CHECK_NULL_RETURN(pszMbcsString,     E_POINTER);
	pszMbcsString[0] = L'\0';

	CHECK_NULL_RETURN(pwszUnicodeString, E_POINTER);
	CHECK_NULL_RETURN(cbMbcsLen,         E_INVALIDARG);
	CHECK_NULL_RETURN(cbUniLen,          E_INVALIDARG);

	cbRet = WideCharToMultiByte(iCode, 0, pwszUnicodeString, -1, pszMbcsString, cbMbcsLen, NULL, NULL);
	if (cbRet == 0)	return HRESULT_FROM_WIN32(GetLastError());

	pszMbcsString[cbRet] = '\0';
	return S_OK;
}

////////////////////////////////////////////////////////////////////////
// ConvertToCoTaskMemStr
//
STDAPI_(LPWSTR) ConvertToCoTaskMemStr(BSTR bstrString)
{
    LPWSTR pwsz;
    ULONG cbLen;

	CHECK_NULL_RETURN(bstrString, NULL);

    cbLen = SysStringLen(bstrString);
    pwsz = (LPWSTR)CoTaskMemAlloc((cbLen * 2) + sizeof(WCHAR));
    if (pwsz)
    {
        memcpy(pwsz, bstrString, (cbLen * 2));
        pwsz[cbLen] = L'\0'; // Make sure it is NULL terminated.
    }

    return pwsz;
}

////////////////////////////////////////////////////////////////////////
// ConvertToMBCS -- NOTE: This returns CoTaskMemAlloc string!
//
STDAPI_(LPSTR) ConvertToMBCS(LPCWSTR pwszUnicodeString, WORD wCodePage)
{
	LPSTR psz = NULL;
	UINT cblen, cbnew;

	CHECK_NULL_RETURN(pwszUnicodeString, NULL);

	cblen = lstrlenW(pwszUnicodeString);
	cbnew = ((cblen + 1) * sizeof(WCHAR));
	psz = (LPSTR)CoTaskMemAlloc(cbnew);
	if ((psz) && FAILED(ConvertToMBCSEx(pwszUnicodeString, cblen, psz, cbnew, wCodePage)))
	{
		CoTaskMemFree(psz);
		psz = NULL;
	}

	return psz;
}

////////////////////////////////////////////////////////////////////////
// ConvertToBSTR 
//
STDAPI_(BSTR) ConvertWToBSTR(LPCWSTR pszAnsiString, WORD wCodePage)
{
	BSTR bstr = NULL;

	CHECK_NULL_RETURN(pszAnsiString, NULL);

	bstr = SysAllocString(pszAnsiString);

	return bstr;
}

STDAPI_(BSTR) ConvertAToBSTR(LPCSTR pszAnsiString, WORD wCodePage)
{
	BSTR bstr = NULL;
	UINT cblen, cbnew;
    LPWSTR pwsz;

	CHECK_NULL_RETURN(pszAnsiString, NULL);

	cblen = lstrlenA(pszAnsiString);
	if ((cblen > 0) && (*pszAnsiString != _T('\0')))
	{
		cbnew = ((cblen + 1) * sizeof(WCHAR));
		pwsz = (LPWSTR)MemAlloc(cbnew);
		if (pwsz) 
		{
			if (SUCCEEDED(ConvertToUnicodeEx(pszAnsiString, cblen, pwsz, cbnew, wCodePage)))
				bstr = SysAllocString(pwsz);

			MemFree(pwsz);
		}
	}

	return bstr;
}

///////////////////////////////////////////////////////////////////////////////////
// CompareStrings
//
//  Calls CompareString API using Unicode version (if available on OS). Otherwise,
//  we have to thunk strings down to MBCS to compare. This is fairly inefficient for
//  Win9x systems that don't handle Unicode, but hey...this is only a sample.
//
STDAPI_(UINT) CompareStrings(LPCWSTR pwsz1, LPCWSTR pwsz2)
{
	UINT iret;
	LCID lcid = GetThreadLocale();
	UINT cblen1, cblen2;

    typedef INT (WINAPI *PFN_CMPSTRINGW)(LCID, DWORD, LPCWSTR, INT, LPCWSTR, INT);
    static PFN_CMPSTRINGW s_pfnCompareStringW = NULL;

 // Check that valid parameters are passed and then contain somethimg...
	if ((pwsz1 == NULL) || ((cblen1 = lstrlenW(pwsz1)) == 0))
		return CSTR_LESS_THAN;

	if ((pwsz2 == NULL) || ((cblen2 = lstrlenW(pwsz2)) == 0))
		return CSTR_GREATER_THAN;

 // If the string is of the same size, then we do quick compare to test for
 // equality (this is slightly faster than calling the API, but only if we
 // expect the calls to find an equal match)...
	if (cblen1 == cblen2)
	{
		for (iret = 0; iret < cblen1; iret++)
		{
			if (pwsz1[iret] == pwsz2[iret])
				continue;

			if (((pwsz1[iret] >= 'A') && (pwsz1[iret] <= 'Z')) &&
				((pwsz1[iret] + ('a' - 'A')) == pwsz2[iret]))
				continue;

			if (((pwsz2[iret] >= 'A') && (pwsz2[iret] <= 'Z')) &&
				((pwsz2[iret] + ('a' - 'A')) == pwsz1[iret]))
				continue;

			break; // don't continue if we can't quickly match...
		}

		// If we made it all the way, then they are equal...
		if (iret == cblen1)
			return CSTR_EQUAL;
	}

	iret = CompareStringW(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, pwsz1, cblen1, pwsz2, cblen2);

	return iret;
}

////////////////////////////////////////////////////////////////////////
// Unicode Win32 API wrappers (handles Unicode/ANSI convert for Win98/ME)
//
////////////////////////////////////////////////////////////////////////
// FFindQualifiedFileName
//
STDAPI_(BOOL) FFindQualifiedFileName(LPCWSTR pwszFile, LPWSTR pwszPath, ULONG *pcPathIdx)
{
    DWORD dwRet = 0;

	LPWSTR lpwszFilePart = NULL;
	SEH_TRY
	dwRet = SearchPathW( NULL, pwszFile, NULL, MAX_PATH, pwszPath, &lpwszFilePart );
	SEH_EXCEPT_NULL
    if ( ( 0 == dwRet || dwRet > MAX_PATH ) ) return FALSE;
    if ( pcPathIdx ) *pcPathIdx = (ULONG)( ( (ULONG_PTR)lpwszFilePart - (ULONG_PTR)pwszPath ) / 2 );


    return TRUE;
}

////////////////////////////////////////////////////////////////////////
// FGetModuleFileName
//
STDAPI_(BOOL) FGetModuleFileName(HMODULE hModule, WCHAR** wzFileName)
{
    LPWSTR pwsz;
    DWORD dw;

    CHECK_NULL_RETURN(wzFileName, FALSE);
    *wzFileName = NULL;

    pwsz = (LPWSTR)MemAlloc( MAX_PATH * 2 );
    CHECK_NULL_RETURN(pwsz, FALSE);

	dw = GetModuleFileNameW( hModule, pwsz, MAX_PATH );
	if ( dw == 0 )
	{
		MemFree( pwsz );
		return FALSE;
	}

    *wzFileName = pwsz;
    return TRUE;
}

////////////////////////////////////////////////////////////////////////
// FGetIconForFile
//
typedef HICON (APIENTRY* PFN_ExtractAssociatedIconA)(HINSTANCE, LPSTR, LPWORD);
typedef HICON (APIENTRY* PFN_ExtractAssociatedIconW)(HINSTANCE, LPWSTR, LPWORD);

STDAPI_(BOOL) FGetIconForFile(LPCWSTR pwszFile, HICON *pico)
{
    WORD idx;
    WORD rgBuffer[MAX_PATH];
    static HMODULE s_hShell32 = NULL;
    static PFN_ExtractAssociatedIconA s_pfnExtractAssociatedIconA = NULL;
    static PFN_ExtractAssociatedIconW s_pfnExtractAssociatedIconW = NULL;

    CHECK_NULL_RETURN(pico, FALSE); *pico = NULL;

    if (s_hShell32 == NULL)
    {
        s_hShell32 = GetModuleHandle(_T("shell32.dll"));
        CHECK_NULL_RETURN(s_hShell32, FALSE);
    }

    memset(rgBuffer, 0, sizeof(rgBuffer));

	if ( s_pfnExtractAssociatedIconW == NULL )
	{
		s_pfnExtractAssociatedIconW = (PFN_ExtractAssociatedIconW)GetProcAddress( s_hShell32, "ExtractAssociatedIconW" );
		CHECK_NULL_RETURN( s_pfnExtractAssociatedIconW, FALSE );
	}

	idx = (WORD)( lstrlenW( pwszFile ) * 2 );
	memcpy( (BYTE*)rgBuffer, (BYTE*)pwszFile, idx ); idx = 0;
	*pico = s_pfnExtractAssociatedIconW( DllModuleHandle(), (LPWSTR)rgBuffer, &idx );

	return (*pico != NULL);
}
