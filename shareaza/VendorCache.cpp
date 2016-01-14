//
// VendorCache.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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

CVendorPtr CVendorCache::LookupByName(LPCTSTR pszName) const
{
	ASSERT( pszName );

	if ( ! pszName || ! *pszName )
		return NULL;

	CString sName( pszName );
	int n = sName.FindOneOf( _T("/ \t\r\n\\") );
	if ( n > 0 )
		sName = sName.Left( n );

	CVendorPtr pVendor;
	if ( m_pNameMap.Lookup( sName, pVendor ) )
		return pVendor;
	else
		return NULL;
}

//////////////////////////////////////////////////////////////////////
// CVendorCache clear

void CVendorCache::Clear()
{
	for ( POSITION pos = m_pCodeMap.GetStartPosition() ; pos ; )
	{
		CString strCode;
		CVendorPtr pItem;
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

BOOL CVendorCache::LoadFrom(const CXMLElement* pXML)
{
	if ( ! pXML->IsNamed( _T("vendorCache") ) ) return FALSE;

	for ( POSITION pos = pXML->GetElementIterator() ; pos ; )
	{
		const CXMLElement* pKey = pXML->GetNextElement( pos );

		if ( pKey->IsNamed( _T("vendor") ) )
		{
			CVendor* pVendor = new CVendor();
			
			if ( pVendor->LoadFrom( pKey ) )
			{
				CVendorPtr pFoo;
				if ( m_pCodeMap.Lookup( pVendor->m_sCode, pFoo ) )
				{
					theApp.Message( MSG_ERROR, _T("Duplicate Vendors.xml code for \"%s\""),
						(LPCTSTR)pVendor->m_sCode );
					delete pVendor;
				}
				else
				{
					m_pCodeMap.SetAt( pVendor->m_sCode, pVendor );
					m_pNameMap.SetAt( pVendor->m_sName, pVendor );
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
	CVendorPtr pVendor = LookupByName( pszCode );
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
	m_bBrowseFlag	( false ),
	m_bExtended		( false )
{
}

CVendor::CVendor(LPCTSTR pszCode) :
	m_sCode			( pszCode ),
	m_sName			( pszCode ),
	m_bChatFlag		( false ),
	m_bBrowseFlag	( false ),
	m_bExtended		( false )
{
	if ( m_sCode.GetLength() > 4 )
		m_sCode = m_sCode.Left( 4 );
	else
		while ( m_sCode.GetLength() < 4 )
			m_sCode += ' ';
}

//////////////////////////////////////////////////////////////////////
// CVendor load

BOOL CVendor::LoadFrom(const CXMLElement* pXML)
{
	m_sCode = pXML->GetAttributeValue( _T("code") );
	if ( m_sCode.GetLength() != 4 ) return FALSE;

	for ( POSITION pos = pXML->GetElementIterator() ; pos ; )
	{
		const CXMLElement* pKey = pXML->GetNextElement( pos );

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
			const CString strCap = pKey->GetAttributeValue( _T("name") );

			if ( strCap.CompareNoCase( _T("chatflag") ) == 0 )
			{
				m_bChatFlag = true;
			}
			else if ( strCap.CompareNoCase( _T("htmlhostbrowse") ) == 0 )
			{
				m_bBrowseFlag = true;
			}
			else if ( strCap.CompareNoCase( _T("extended") ) == 0 )
			{
				m_bExtended = true;
			}			
		}
	}

	return m_sName.GetLength() > 0;
}
