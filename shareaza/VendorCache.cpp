//
// VendorCache.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
// This file is part of SHAREAZA (www.shareaza.com)
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "VendorCache.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CVendorCache VendorCache;


//////////////////////////////////////////////////////////////////////
// CVendorCache construction

CVendorCache::CVendorCache()
{
	m_pNull = new CVendor();
	m_pNull->m_bAuto = TRUE;
	
	m_pShareaza = m_pED2K = m_pNull;
}

CVendorCache::~CVendorCache()
{
	delete m_pNull;
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CVendorCache list access

POSITION CVendorCache::GetIterator() const
{
	return m_pMap.GetStartPosition();
}

CVendor* CVendorCache::GetNext(POSITION& pos) const
{
	CVendor* pItem = NULL;
	CString strCode;
	m_pMap.GetNextAssoc( pos, strCode, (void*&)pItem );
	return pItem;
}

int CVendorCache::GetCount() const
{
	return m_pMap.GetCount();
}

//////////////////////////////////////////////////////////////////////
// CVendorCache lookup

#ifdef _UNICODE
CVendor* CVendorCache::Lookup(LPCSTR pszCode, BOOL bCreate)
{
	WCHAR szCode[5] = { pszCode[0], pszCode[1], pszCode[2], pszCode[3], 0 };
	return Lookup( szCode, bCreate );
}

CVendor* CVendorCache::Lookup(LPCWSTR pszCode, BOOL bCreate)

#else

CVendor* CVendorCache::Lookup(LPCWSTR pszCode, BOOL bCreate)
{
	CHAR szCode[5] = { pszCode[0], pszCode[1], pszCode[2], pszCode[3], 0 };
	return Lookup( szCode, bCreate );
}

CVendor* CVendorCache::Lookup(LPCSTR pszCode, BOOL bCreate)
#endif
{
	CVendor* pVendor = NULL;

	if ( m_pMap.Lookup( pszCode, (void*&)pVendor ) ) return pVendor;
	if ( ! bCreate ) return NULL;

	if ( ! pszCode[0] || ! pszCode[1] || ! pszCode[2] || ! pszCode[3] ) return m_pNull;
	if ( ! _istalpha( pszCode[0] ) || ! _istalpha( pszCode[1] ) ) return m_pNull;
	if ( ! _istalpha( pszCode[2] ) || ! _istalpha( pszCode[3] ) ) return m_pNull;

	pVendor = new CVendor( pszCode );
	m_pMap.SetAt( pszCode, pVendor );

	return pVendor;
}

CVendor* CVendorCache::LookupByName(LPCTSTR pszName) const
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CVendor* pVendor = GetNext( pos );

		if ( _tcsstr( pszName, pVendor->m_sName ) != NULL ) return pVendor;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CVendorCache clear

void CVendorCache::Clear()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		delete GetNext( pos );
	}
	m_pMap.RemoveAll();
	m_pShareaza = NULL;
}

//////////////////////////////////////////////////////////////////////
// CVendorCache load

BOOL CVendorCache::Load()
{
	CString strPath = Settings.General.Path + _T("\\Data\\Vendors.xml");
	CXMLElement* pXML = CXMLElement::FromFile( strPath, TRUE );
	BOOL bSuccess = FALSE;
	
	if ( pXML != NULL )
	{
		bSuccess = LoadFrom( pXML );
		delete pXML;
	}
	
	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CVendorCache load internal

BOOL CVendorCache::LoadFrom(CXMLElement* pXML)
{
	if ( ! pXML->IsNamed( _T("vendorCache") ) ) return FALSE;

	for ( POSITION pos = pXML->GetElementIterator() ; pos ; )
	{
		CXMLElement* pKey = pXML->GetNextElement( pos );

		if ( pKey->IsNamed( _T("vendor") ) )
		{
			CVendor* pVendor = new CVendor();
			
			if ( pVendor->LoadFrom( pKey ) )
			{
				CVendor* pOld = NULL;
				
				if ( m_pMap.Lookup( pVendor->m_sCode, (void*&)pOld ) )
				{
					theApp.Message( MSG_ERROR, _T("Duplicate Vendors.xml key for \"%s\"."),
						(LPCTSTR)pVendor->m_sCode );
					delete pOld;
				}
				
				m_pMap.SetAt( pVendor->m_sCode, pVendor );
				
				if ( pVendor->m_sCode == _T("RAZA") ) m_pShareaza = pVendor;
				if ( pVendor->m_sCode == _T("ED2K") ) m_pED2K = pVendor;
			}
			else
			{
				delete pVendor;
			}
		}
	}
	
	return GetCount() > 0;
}


//////////////////////////////////////////////////////////////////////
// CVendor construciton

CVendor::CVendor(LPCTSTR pszCode)
{
	if ( m_bAuto = ( pszCode != NULL ) )
	{
		m_sCode = m_sName = pszCode;
		while ( m_sCode.GetLength() < 4 ) m_sCode += ' ';
		if ( m_sCode.GetLength() > 4 ) m_sCode = m_sCode.Left( 4 );
	}

	m_bChatFlag		= FALSE;
	m_bHTMLBrowse	= FALSE;
}

CVendor::~CVendor()
{
}

//////////////////////////////////////////////////////////////////////
// CVendor load

BOOL CVendor::LoadFrom(CXMLElement* pXML)
{
	m_sCode = pXML->GetAttributeValue( _T("code") );
	if ( m_sCode.GetLength() != 4 ) return FALSE;

	for ( POSITION pos = pXML->GetElementIterator() ; pos ; )
	{
		CXMLElement* pKey = pXML->GetNextElement( pos );

		if ( pKey->IsNamed( _T("title") ) )
		{
			if ( m_sName.GetLength() ) return FALSE;
			m_sName = pKey->GetValue();
		}
		else if ( pKey->IsNamed( _T("link") ) )
		{
			if ( m_sLink.GetLength() ) return FALSE;
			m_sLink = pKey->GetValue();
		}
		else if ( pKey->IsNamed( _T("capability") ) )
		{
			CString strCap = pKey->GetAttributeValue( _T("name") );
			strCap.MakeLower();

			BOOL bValue = TRUE;
			
			if ( strCap == _T("chatflag") )
			{
				m_bChatFlag = bValue;
			}
			else if ( strCap == _T("htmlhostbrowse") )
			{
				m_bHTMLBrowse = bValue;
			}
		}
	}

	return m_sName.GetLength() > 0;
}
