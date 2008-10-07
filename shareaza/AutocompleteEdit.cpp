// AutocompleteEdit.cpp : implementation file
//

#include "stdafx.h"
#include "Shareaza.h"
#include "AutocompleteEdit.h"

#define MAX_AUTOCOMPLETE 100

BEGIN_INTERFACE_MAP(CAutocompleteEdit::CEnumString, CComObject)
	INTERFACE_PART(CAutocompleteEdit::CEnumString, IID_IEnumString, EnumString)
END_INTERFACE_MAP()

CAutocompleteEdit::CEnumString::CEnumString() :
	m_sect( _T("Autocomplete") ),
	m_root( _T("Default.") ),
	m_iter( 0 )
{
}

IMPLEMENT_UNKNOWN(CAutocompleteEdit::CEnumString, EnumString)

STDMETHODIMP CAutocompleteEdit::CEnumString::XEnumString::Next(
	/* [in] */ ULONG celt,
	/* [length_is][size_is][out] */ LPOLESTR* rgelt,
	/* [out] */ ULONG *pceltFetched)
{
	METHOD_PROLOGUE( CAutocompleteEdit::CEnumString, EnumString )

	if ( rgelt == NULL || ( celt != 1 && pceltFetched == NULL ) )
		return E_POINTER;

	LPOLESTR* pelt = rgelt;
	ULONG nActual = 0;
	HRESULT hr = S_OK;
	while ( SUCCEEDED( hr ) && nActual < celt )
	{
		CString strKey;
		strKey.Format( _T("%s%02u"), pThis->m_root, pThis->m_iter );
		CString strValue( AfxGetApp()->GetProfileString( pThis->m_sect, strKey ) );
		if ( strValue.IsEmpty() )
			break;
		size_t len = ( strValue.GetLength() + 1 ) * sizeof( WCHAR );
			*pelt = (LPWSTR)CoTaskMemAlloc( len );
		if ( ! *pelt )
			return E_OUTOFMEMORY;
		CopyMemory( *pelt, (LPCWSTR)strValue, len );
		pThis->m_iter++;
		pelt++;
		nActual++;
	}
	if ( pceltFetched )
		*pceltFetched = nActual;
	if ( SUCCEEDED( hr ) && ( nActual < celt ) )
		hr = S_FALSE;
	return hr;
}

STDMETHODIMP CAutocompleteEdit::CEnumString::XEnumString::Skip(
	/* [in] */ ULONG celt)
{
	METHOD_PROLOGUE( CAutocompleteEdit::CEnumString, EnumString )

	HRESULT hr = S_OK;
	while ( celt-- )
	{
		CString strKey;
		strKey.Format( _T("%s%02u"), pThis->m_root, pThis->m_iter );
		CString strValue( AfxGetApp()->GetProfileString( pThis->m_sect, strKey ) );
		if ( strValue.IsEmpty() )
		{
			hr = S_FALSE;
			break;
		}
		pThis->m_iter++;
	}
	return hr;
}

STDMETHODIMP CAutocompleteEdit::CEnumString::XEnumString::Reset(void)
{
	METHOD_PROLOGUE( CAutocompleteEdit::CEnumString, EnumString )

	pThis->m_iter = 0;

	return S_OK;
}

STDMETHODIMP CAutocompleteEdit::CEnumString::XEnumString::Clone(
	/* [out] */ IEnumString** ppEnum)
{
	METHOD_PROLOGUE( CAutocompleteEdit::CEnumString, EnumString )

	HRESULT hr = E_POINTER;
	if ( ppEnum != NULL )
	{
		*ppEnum = NULL;
		hr = E_OUTOFMEMORY;
		CEnumString* p = new CEnumString();
		if ( p )
		{
			p->m_root = pThis->m_root;
			p->m_iter = pThis->m_iter;
			hr = p->InternalQueryInterface( &IID_IEnumString, (void**)ppEnum );
			p->InternalRelease();
		}
	}
	return hr;
}

void CAutocompleteEdit::CEnumString::AddString(CString& rString) const
{
	rString.Trim();
	if ( rString.IsEmpty() )
		return;

	size_t count = 0;
	for( ;; count++ )
	{
		CString strKey;
		strKey.Format( _T("%s%02u"), m_root, count );
		CString strValue( AfxGetApp()->GetProfileString( m_sect, strKey ) );
		if ( strValue.IsEmpty() )
			break;
		if ( strValue == rString )
			return;
	}
	if ( count >= MAX_AUTOCOMPLETE )
		count = 0;

	CString strKey;
	strKey.Format( _T("%s%02u"), m_root, count );
	AfxGetApp()->WriteProfileString( m_sect, strKey, rString );
}

IMPLEMENT_DYNAMIC(CAutocompleteEdit, CEdit)

CAutocompleteEdit::CAutocompleteEdit()
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif
}

BEGIN_MESSAGE_MAP(CAutocompleteEdit, CEdit)
	ON_WM_CREATE()
END_MESSAGE_MAP()

void CAutocompleteEdit::SetRegistryKey(LPCTSTR szSection, LPCTSTR szRoot)
{
	m_oData.m_sect = szSection;
	m_oData.m_root = szRoot;
}

int CAutocompleteEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CEdit::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	m_oData.m_root.Format( _T("Edit.%08x."), GetDlgCtrlID() );

	HRESULT hr = m_pIAutoComplete.CoCreateInstance( CLSID_AutoComplete );
	if ( SUCCEEDED( hr ) )
	{
		CComPtr< IUnknown > pIUnknown;
		hr = m_oData.InternalQueryInterface( &IID_IUnknown, (LPVOID*)&pIUnknown );
		if ( SUCCEEDED( hr ) )
		{
			hr = m_pIAutoComplete->Init( GetSafeHwnd(), pIUnknown, NULL, NULL );
			if ( SUCCEEDED( hr ) )
			{
				CComPtr< IAutoComplete2 > pIAutoComplete2;
				hr = m_pIAutoComplete->QueryInterface( IID_IAutoComplete2,
					(LPVOID*)&pIAutoComplete2 );
				if ( SUCCEEDED( hr ) )
				{
					hr = pIAutoComplete2->SetOptions( ACO_AUTOSUGGEST |
						ACO_UPDOWNKEYDROPSLIST );
				}
			}
		}
	}

	return 0;
}

int CAutocompleteEdit::GetWindowText(LPTSTR lpszStringBuf, int nMaxCount) const
{
	int n = CEdit::GetWindowText( lpszStringBuf, nMaxCount );
	if ( n > 0 )
	{
		CString strString( lpszStringBuf );
		m_oData.AddString( strString );
	}
	return n;
}

void CAutocompleteEdit::GetWindowText(CString& rString) const
{
	CEdit::GetWindowText( rString );

	m_oData.AddString( rString );
}
