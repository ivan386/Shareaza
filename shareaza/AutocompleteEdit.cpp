//
// AutocompleteEdit.cpp
//
// Copyright (c) Shareaza Development Team, 2008-2014.
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
#include "Shareaza.h"
#include "Settings.h"
#include "AutocompleteEdit.h"

IMPLEMENT_DYNCREATE(CRegEnum, CComObject)

BEGIN_INTERFACE_MAP(CRegEnum, CComObject)
	INTERFACE_PART(CRegEnum, IID_IEnumString, EnumString)
END_INTERFACE_MAP()

CRegEnum::CRegEnum()
	: m_iter( 0 )
{
}

CRegEnum::~CRegEnum()
{
}

IMPLEMENT_UNKNOWN(CRegEnum, EnumString)

STDMETHODIMP CRegEnum::XEnumString::Next(
	/* [in] */ ULONG celt,
	/* [length_is][size_is][out] */ LPOLESTR* rgelt,
	/* [out] */ ULONG *pceltFetched)
{
	METHOD_PROLOGUE( CRegEnum, EnumString )

	if ( rgelt == NULL || ( celt != 1 && pceltFetched == NULL ) )
		return E_POINTER;

	LPOLESTR* pelt = rgelt;
	ULONG nActual = 0;
	while ( nActual < celt )
	{
		CString strEntry;
		strEntry.Format( pThis->m_root, pThis->m_iter + 1 );
		CString strValue( AfxGetApp()->GetProfileString( pThis->m_sect, strEntry ) );
		if ( strValue.IsEmpty() )
			break;
		int lf = strValue.Find( _T('\n') );
		if ( lf != -1 )
			strValue = strValue.Left( lf );
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
	return ( nActual < celt ) ? S_FALSE : S_OK;
}

STDMETHODIMP CRegEnum::XEnumString::Skip(
	/* [in] */ ULONG celt)
{
	METHOD_PROLOGUE( CRegEnum, EnumString )

	HRESULT hr = S_OK;
	while ( celt-- )
	{
		CString strEntry;
		strEntry.Format( pThis->m_root, pThis->m_iter + 1 );
		CString strValue( AfxGetApp()->GetProfileString( pThis->m_sect, strEntry ) );
		if ( strValue.IsEmpty() )
		{
			hr = S_FALSE;
			break;
		}
		pThis->m_iter++;
	}
	return hr;
}

STDMETHODIMP CRegEnum::XEnumString::Reset(void)
{
	METHOD_PROLOGUE( CRegEnum, EnumString )

	pThis->m_iter = 0;

	return S_OK;
}

STDMETHODIMP CRegEnum::XEnumString::Clone(
	/* [out] */ IEnumString** ppEnum)
{
	METHOD_PROLOGUE( CRegEnum, EnumString )

	HRESULT hr = E_POINTER;
	if ( ppEnum != NULL )
	{
		*ppEnum = NULL;
		hr = E_OUTOFMEMORY;
		CRegEnum* p = new CRegEnum();
		if ( p )
		{
			p->m_sect = pThis->m_sect;
			p->m_root = pThis->m_root;
			p->m_iter = pThis->m_iter;
			hr = p->InternalQueryInterface( &IID_IEnumString, (void**)ppEnum );
			p->InternalRelease();
		}
	}
	return hr;
}

BOOL CRegEnum::AttachTo(HWND hWnd, LPCTSTR szSection, LPCTSTR szRoot)
{
	m_pIAutoComplete.Release();

	if ( ! hWnd )
		return TRUE;

	m_sect = szSection;
	m_root = szRoot;
	m_iter = 0;

	HRESULT hr = m_pIAutoComplete.CoCreateInstance( CLSID_AutoComplete );
	if ( SUCCEEDED( hr ) )
	{
		CComPtr< IUnknown > pIUnknown;
		hr = InternalQueryInterface( &IID_IUnknown, (LPVOID*)&pIUnknown );
		if ( SUCCEEDED( hr ) )
		{
			hr = m_pIAutoComplete->Init( hWnd, pIUnknown, NULL, NULL );
			if ( SUCCEEDED( hr ) )
			{
				CComPtr< IAutoComplete2 > pIAutoComplete2;
				hr = m_pIAutoComplete->QueryInterface( IID_IAutoComplete2,
					(LPVOID*)&pIAutoComplete2 );
				if ( SUCCEEDED( hr ) )
				{
					hr = pIAutoComplete2->SetOptions( ACO_AUTOSUGGEST |
						ACO_AUTOAPPEND | ACO_UPDOWNKEYDROPSLIST );
				}
			}
		}
	}

	return SUCCEEDED( hr );
}

void CRegEnum::AddString(const CString& rString) const
{
	CString sKeyString = rString.SpanExcluding( _T("\n") );
	if ( sKeyString.IsEmpty() )
		return;

	// Load list
	CStringList oList;
	for ( int i = 0; ; ++i )
	{
		CString strEntry;
		strEntry.Format( m_root, i + 1 );
		CString strValue( AfxGetApp()->GetProfileString( m_sect, strEntry ) );
		if ( strValue.IsEmpty() )
			break;
		CString strShortValue = strValue.SpanExcluding( _T("\n") );
		if ( sKeyString.CompareNoCase( strShortValue ) )
			oList.AddTail( strValue );
	}

	// Cut to 200 items
	while ( oList.GetCount() >= 200 )
		oList.RemoveTail();

	// Add as most recent
	oList.AddHead( rString );

	// Save list
	POSITION pos = oList.GetHeadPosition();
	for ( int i = 0; pos; ++i )
	{
		CString strEntry;
		strEntry.Format( m_root, i + 1 );
		AfxGetApp()->WriteProfileString( m_sect, strEntry, oList.GetNext( pos ) );
	}
}

IMPLEMENT_DYNCREATE(CAutocompleteEdit, CEdit)

CAutocompleteEdit::CAutocompleteEdit()
	: m_pData( NULL )
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif
}

CAutocompleteEdit::~CAutocompleteEdit()
{
}

BEGIN_MESSAGE_MAP(CAutocompleteEdit, CEdit)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL CAutocompleteEdit::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID,
	LPCTSTR szSection, LPCTSTR szRoot)
{
	if ( ! CEdit::Create( dwStyle, rect, pParentWnd, nID ) )
		return FALSE;

	if ( Settings.Interface.AutoComplete )
	{
		m_pData = static_cast< CRegEnum* >( CRegEnum::CreateObject() );
		if ( m_pData )
			m_pData->AttachTo( GetSafeHwnd(), szSection, szRoot );
	}

	return TRUE;
}

void CAutocompleteEdit::OnDestroy()
{
	if ( m_pData )
	{
		m_pData->AttachTo( NULL, NULL, NULL );
		m_pData->InternalRelease();
		m_pData = NULL;
	}

	CEdit::OnDestroy();
}

int CAutocompleteEdit::GetWindowText(LPTSTR lpszStringBuf, int nMaxCount) const
{
	int n = CEdit::GetWindowText( lpszStringBuf, nMaxCount );
	if ( m_pData && n > 0 )
	{
		CString tmp( lpszStringBuf );
		m_pData->AddString( tmp );
	}
	return n;
}

void CAutocompleteEdit::GetWindowText(CString& rString) const
{
	CEdit::GetWindowText( rString );

	if ( m_pData )
		m_pData->AddString( rString );
}
