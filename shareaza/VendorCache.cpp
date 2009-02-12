//
// VendorCache.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

CVendorCache::CVendorCache() :
	m_pNull( new CVendor() )
{
	// experimental values
	m_pCodeMap.InitHashTable( 83 );
	m_pNameMap.InitHashTable( 83 );
}

CVendorCache::~CVendorCache()
{
	delete m_pNull;
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CVendorCache lookup

CVendor* CVendorCache::LookupByName(LPCTSTR pszName) const
{
	ASSERT( pszName );

	if ( ! pszName || ! *pszName )
		return NULL;

	CString sName( pszName );
	int n = sName.FindOneOf( _T("/ \t\r\n\\") );
	if ( n > 0 )
		sName = sName.Left( n );
	sName.MakeLower();

	CVendor* pVendor;
	if ( m_pNameMap.Lookup( sName, pVendor ) )
		return pVendor;
	else
		return NULL;
}

//////////////////////////////////////////////////////////////////////
// CVendorCache clear

void CVendorCache::Clear()
{
	CVendor* pItem;
	CString strCode;
	for ( POSITION pos = m_pCodeMap.GetStartPosition() ; pos ; )
	{
		m_pCodeMap.GetNextAssoc( pos, strCode, pItem );
		delete pItem;
	}
	m_pCodeMap.RemoveAll();
	m_pNameMap.RemoveAll();
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
		if ( ! bSuccess )
			theApp.Message( MSG_ERROR, _T("Invalid Vendors.xml file") );
	}
	else
		theApp.Message( MSG_ERROR, _T("Missed Vendors.xml file") );

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CVendorCache load internal

BOOL CVendorCache::LoadFrom(CXMLElement* pXML)
{
	if ( ! pXML->IsNamed( _T("vendorCache") ) ) return FALSE;

	CVendor* pFoo;
	for ( POSITION pos = pXML->GetElementIterator() ; pos ; )
	{
		CXMLElement* pKey = pXML->GetNextElement( pos );

		if ( pKey->IsNamed( _T("vendor") ) )
		{
			CVendor* pVendor = new CVendor();
			
			if ( pVendor->LoadFrom( pKey ) )
			{
				if ( m_pCodeMap.Lookup( pVendor->m_sCode, pFoo ) )
				{
					theApp.Message( MSG_ERROR, _T("Duplicate Vendors.xml code for \"%s\""),
						(LPCTSTR)pVendor->m_sCode );
					delete pVendor;
				}
				else
				{
					m_pCodeMap.SetAt( pVendor->m_sCode, pVendor );
					m_pNameMap.SetAt( CString ( pVendor->m_sName ).MakeLower(), pVendor );
				}
			}
			else
			{
				theApp.Message( MSG_ERROR, _T("Invalid Vendors.xml entry") );
				delete pVendor;
			}
		}
	}
	
	return m_pCodeMap.GetCount() > 0;
}

bool CVendorCache::IsExtended(LPCTSTR pszCode) const
{
	ASSERT( pszCode );

	// Find by product name (Server or User-Agent HTTP-headers)
	CVendor* pVendor = LookupByName( pszCode );
	if ( ! pVendor )
	{
		// Find by vendor code
		pVendor = Lookup( pszCode );
	}
	if ( pVendor )
		return pVendor->m_bExtended;

	// Unknown vendor code
	return false;
}

//////////////////////////////////////////////////////////////////////
// CVendor construciton

CVendor::CVendor() :
	m_bChatFlag		( false ),
	m_bHTMLBrowse	( false ),
	m_bExtended		( false )
{
}

CVendor::CVendor(LPCTSTR pszCode) :
	m_sCode			( pszCode ),
	m_sName			( pszCode ),
	m_bChatFlag		( false ),
	m_bHTMLBrowse	( false ),
	m_bExtended		( false )
{
	if ( m_sCode.GetLength() > 4 )
		m_sCode = m_sCode.Left( 4 );
	else
		while ( m_sCode.GetLength() < 4 )
			m_sCode += ' ';
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
			ToLower( strCap );

			if ( strCap == _T("chatflag") )
			{
				m_bChatFlag = true;
			}
			else if ( strCap == _T("htmlhostbrowse") )
			{
				m_bHTMLBrowse = true;
			}
			else if ( strCap == _T("extended") )
			{
				m_bExtended = true;
			}			
		}
	}

	return m_sName.GetLength() > 0;
}
