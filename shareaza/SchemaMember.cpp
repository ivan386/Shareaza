//
// SchemaMember.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
#include "Schema.h"
#include "SchemaMember.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CSchemaMember construction

CSchemaMember::CSchemaMember(CSchema* pSchema)
{
	m_pSchema		= pSchema;
	m_bNumeric		= FALSE;
	m_bIndexed		= FALSE;
	m_bSearched		= FALSE;
	m_nMinOccurs	= 0;
	m_nMaxOccurs	= 0;
	m_nMaxLength	= 128;
    
	m_bPrompt		= FALSE;
	m_nFormat		= smfNone;
	m_nColumnWidth	= 60;
	m_nColumnAlign	= LVCFMT_LEFT;
}

CSchemaMember::~CSchemaMember()
{
}

//////////////////////////////////////////////////////////////////////
// CSchemaMember item access

POSITION CSchemaMember::GetItemIterator() const
{
	return m_pItems.GetHeadPosition();
}

CString CSchemaMember::GetNextItem(POSITION& pos) const
{
	return m_pItems.GetNext( pos );
}

int CSchemaMember::GetItemCount() const
{
	return m_pItems.GetCount();
}

//////////////////////////////////////////////////////////////////////
// CSchemaMember value lookup

CString CSchemaMember::GetValueFrom(CXMLElement* pBase, LPCTSTR pszDefault, BOOL bFormat) const
{
	// OPTIMIZE: This could all be done with LPCTSTR pointers instead of CString
	CString strValue;
	
	if ( pBase != NULL )
	{
		if ( CXMLElement* pElement = pBase->GetElementByName( m_sName ) )
		{
			strValue = pElement->GetValue();
		}
		else
		{
			strValue = pBase->GetAttributeValue( m_sName, pszDefault );
		}
	}
	else if ( pszDefault != NULL )
	{
		strValue = pszDefault;
	}
	
	if ( strValue.IsEmpty() ) return strValue;
	
	if ( bFormat && m_bNumeric )
	{
		BOOL bInvalid = FALSE;

		if ( m_nFormat == smfTimeMMSS )
		{
			DWORD nSeconds = 0;
			_stscanf( strValue, _T("%lu"), &nSeconds );
			bInvalid = ( nSeconds < (DWORD)m_nMinOccurs || nSeconds > (DWORD)m_nMaxOccurs );
			strValue.Format( _T("%.2u:%.2u"), nSeconds / 60, nSeconds % 60 );
		}
		else if ( m_nFormat == smfTimeHHMMSSdec )
		{
			float nMinutes = 0;
			_stscanf( strValue, _T("%f"), &nMinutes );
			bInvalid = ( nMinutes < (DWORD)m_nMinOccurs || nMinutes > (DWORD)m_nMaxOccurs );
			strValue.Format( _T("%.2u:%.2u:%.2u"), (int)nMinutes / 60,
				(int)nMinutes % 60, (int)( ( nMinutes - (int)nMinutes ) * 60 ) );
		}
		else if ( m_nFormat == smfFrequency )
		{
			DWORD nRate = 0;
			_stscanf( strValue, _T("%lu"), &nRate );
			bInvalid = ( nRate < (DWORD)m_nMinOccurs || nRate > (DWORD)m_nMaxOccurs );
			strValue.Format( _T("%.1f kHz"), nRate / 1000.0 );
		}
		else if ( m_nFormat == smfBitrate )
		{
			BOOL bVariable = _tcschr( strValue, '~' ) != NULL;
			DWORD nBitrate = 0;
			_stscanf( strValue, _T("%lu"), &nBitrate );
			bInvalid = ( nBitrate < (DWORD)m_nMinOccurs || nBitrate > (DWORD)m_nMaxOccurs );
			strValue.Format( bVariable ? _T("%luk~") : _T("%luk"), nBitrate );
		}
		if ( bInvalid ) strValue.Empty();
	}
	
	return strValue;
}

//////////////////////////////////////////////////////////////////////
// CSchemaMember value set

void CSchemaMember::SetValueTo(CXMLElement* pBase, LPCTSTR pszValue)
{
	if ( CXMLElement* pElement = pBase->GetElementByName( m_sName ) )
	{
		if ( m_bElement && pszValue != NULL && _tcslen( pszValue ) > 0 )
			pElement->SetValue( pszValue );
		else
			pElement->Delete();
	}
	else if ( m_bElement && pszValue != NULL && _tcslen( pszValue ) > 0 )
	{
		CXMLElement* pElement = pBase->AddElement( m_sName );
		pElement->SetValue( pszValue );
	}
	
	if ( CXMLAttribute* pAttribute = pBase->GetAttribute( m_sName ) )
	{
		if ( ! m_bElement && pszValue != NULL && _tcslen( pszValue ) > 0 )
			pAttribute->SetValue( pszValue );
		else
			pAttribute->Delete();
	}
	else if ( ! m_bElement && pszValue != NULL && _tcslen( pszValue ) > 0 )
	{
		pBase->AddAttribute( m_sName, pszValue );
	}
}

//////////////////////////////////////////////////////////////////////
// CSchemaMember load schema

BOOL CSchemaMember::LoadSchema(CXMLElement* pRoot, CXMLElement* pElement)
{
	m_bElement = pElement->GetName().CompareNoCase( _T("element") ) == 0;

	m_sName = pElement->GetAttributeValue( _T("name"), _T("") );
	if ( m_sName.IsEmpty() ) return FALSE;

	m_sTitle = m_sName;
	m_sTitle.SetAt( 0, toupper( m_sTitle.GetAt( 0 ) ) );

	m_sType = pElement->GetAttributeValue( _T("type"), _T("") );
	CharLower( m_sType.GetBuffer() );// Lowercase'd
	m_sType.ReleaseBuffer();

	m_bNumeric = ( m_sType == _T("short") || m_sType == _T("int") || m_sType == _T("decimal") );
	
	CString strValue = pElement->GetAttributeValue( _T("minOccurs"), _T("0") );
	_stscanf( strValue, _T("%i"), &m_nMinOccurs );
	strValue = pElement->GetAttributeValue( _T("maxOccurs"), _T("65536") );
	_stscanf( strValue, _T("%i"), &m_nMaxOccurs );
	
	if ( pElement->GetElementCount() )
	{
		return LoadType( pElement->GetFirstElement() );
	}
	else if ( m_sType.GetLength() )
	{
		CXMLElement* pType = m_pSchema->GetType( pRoot, m_sType );
		return pType ? LoadType( pType ) : TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CSchemaMember::LoadType(CXMLElement* pType)
{
	CString strName = pType->GetName();

	if ( strName.CompareNoCase( _T("simpleType") ) &&
		 strName.CompareNoCase( _T("complexType") ) ) return FALSE;

	m_sType = pType->GetAttributeValue( _T("base"), _T("") );
	CharLower( m_sType.GetBuffer() );
	m_sType.ReleaseBuffer();
	m_bNumeric = ( m_sType == _T("short") || m_sType == _T("int") || m_sType == _T("decimal") );
	
	for ( POSITION pos = pType->GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement	= pType->GetNextElement( pos );
		CString strElement		= pElement->GetName();

		if ( strElement.CompareNoCase( _T("enumeration") ) == 0 )
		{
			CString strValue = pElement->GetAttributeValue( _T("value"), _T("") );
			if ( strValue.GetLength() ) m_pItems.AddTail( strValue );
		}
		else if ( strElement.CompareNoCase( _T("maxInclusive") ) == 0 )
		{
			CString strValue = pElement->GetAttributeValue( _T("value"), _T("0") );
			_stscanf( strValue, _T("%i"), &m_nMaxLength );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSchemaMember load descriptor

BOOL CSchemaMember::LoadDescriptor(CXMLElement* pXML)
{
	CString strSearch = pXML->GetAttributeValue( _T("search") );
	
	if ( strSearch.CompareNoCase( _T("generic") ) == 0 )
	{
		m_bIndexed	= TRUE;
		m_bSearched	= TRUE;
	}
	else if ( strSearch.CompareNoCase( _T("indexed") ) == 0 )
	{
		m_bIndexed = TRUE;
	}
	
	CString strTitle = m_sTitle;
	m_sTitle.Empty();
	
	for ( POSITION pos = pXML->GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = pXML->GetNextElement( pos );
		
		if ( pElement->IsNamed( _T("display") ) )
		{
			LoadDisplay( pElement );
		}
		else if ( pElement->IsNamed( _T("title") ) )
		{
			if ( pElement->GetAttributeValue( _T("language") ).
				 CompareNoCase( Settings.General.Language ) == 0 )
			{
				m_sTitle = pElement->GetValue();
			}
			else if ( m_sTitle.IsEmpty() )
			{
				m_sTitle = pElement->GetValue();
			}
		}
		else if ( pElement->IsNamed( _T("link") ) )
		{
			m_sLinkURI	= pElement->GetAttributeValue( _T("location") );
			m_sLinkName	= pElement->GetAttributeValue( _T("remote") );
		}
	}
	
	if ( m_sTitle.IsEmpty() ) m_sTitle = strTitle;
	
	return TRUE;
}

BOOL CSchemaMember::LoadDisplay(CXMLElement* pDisplay)
{
	CString strFormat	= pDisplay->GetAttributeValue( _T("format") );
	CString strWidth	= pDisplay->GetAttributeValue( _T("columnWidth") );
	CString strAlign	= pDisplay->GetAttributeValue( _T("columnAlign") );
	CString strColumn	= pDisplay->GetAttributeValue( _T("defaultColumn") );
	CString strSearch	= pDisplay->GetAttributeValue( _T("prompt") );
	
	if ( strFormat.CompareNoCase( _T("timeMMSS") ) == 0 )
		m_nFormat = smfTimeMMSS;
	else if ( strFormat.CompareNoCase( _T("timeHHMMSSdec") ) == 0 )
		m_nFormat = smfTimeHHMMSSdec;
	else if ( strFormat.CompareNoCase( _T("bitrate") ) == 0 )
		m_nFormat = smfBitrate;
	else if ( strFormat.CompareNoCase( _T("frequency") ) == 0 )
		m_nFormat = smfFrequency;
	
	_stscanf( strWidth, _T("%lu"), &m_nColumnWidth );
	
	if ( strAlign.CompareNoCase( _T("left") ) == 0 )
		m_nColumnAlign = LVCFMT_LEFT;
	else if ( strAlign.CompareNoCase( _T("center") ) == 0 )
		m_nColumnAlign = LVCFMT_CENTER;
	else if ( strAlign.CompareNoCase( _T("right") ) == 0 )
		m_nColumnAlign = LVCFMT_RIGHT;
	
	if ( strColumn.CompareNoCase( _T("true") ) == 0 )
	{
		m_pSchema->m_sDefaultColumns += '|';
		m_pSchema->m_sDefaultColumns += m_sName;
		m_pSchema->m_sDefaultColumns += '|';
	}
	
	if ( strSearch.CompareNoCase( _T("true") ) == 0 )
	{
		m_bPrompt = TRUE;
	}
	
	return TRUE;
}
