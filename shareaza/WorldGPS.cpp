//
// WorldGPS.cpp
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
#include "WorldGPS.h"
#include "Buffer.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CWorldGPS construction

CWorldGPS::CWorldGPS()
{
	m_pCountry = NULL;
	m_nCountry = 0;
}

CWorldGPS::~CWorldGPS()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CWorldGPS load

BOOL CWorldGPS::Load()
{
	CFile pFile;

	Clear();

	CString strFile = Settings.General.Path + L"\\Data\\WorldGPS";

	bool bImport = theApp.GetProfileInt( L"", L"ImportWorldGPS", FALSE ) != 0;
	strFile.Append( bImport ? L".xml" : L".dat" );
		
	if ( ! pFile.Open( (LPCTSTR)strFile.GetBuffer(), CFile::modeRead ) )
	{
		if ( bImport )
		{
			strFile = Settings.General.Path + L"\\Data\\WorldGPS.dat";
			if ( ! pFile.Open( (LPCTSTR)strFile.GetBuffer(), CFile::modeRead ) ) return FALSE;
			bImport = false;
		}
		else
			return FALSE;
	}

	if ( !bImport )
	{
		try
		{
			CArchive ar( &pFile, CArchive::load );
			Serialize( ar );
		}
		catch ( CException* pException )
		{
			pException->Delete();
		}
		return TRUE;
	}

	CBuffer pBuffer;
	pBuffer.EnsureBuffer( (DWORD)pFile.GetLength() );
	pBuffer.m_nLength = (DWORD)pFile.GetLength();
	pFile.Read( pBuffer.m_pBuffer, pBuffer.m_nLength );
	pFile.Close();

	CXMLElement* pXML = CXMLElement::FromBytes( pBuffer.m_pBuffer, pBuffer.m_nLength );
	if ( NULL == pXML ) return FALSE;

	BOOL bSuccess = LoadFrom( pXML );
	delete pXML;

	if ( ! bSuccess ) return FALSE;

	strFile = Settings.General.Path + L"\\Data\\WorldGPS.dat";
	if ( ! pFile.Open( strFile, CFile::modeWrite|CFile::modeCreate ) )
		return FALSE;

	try
	{
		CArchive ar( &pFile, CArchive::store );	// 4 KB buffer
		try
		{
			Serialize( ar );
			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
			return FALSE;
		}
		pFile.Close();
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
		return FALSE;
	}

	return TRUE;
}

void CWorldGPS::Serialize(CArchive& ar)
{
	if ( ar.IsStoring() )
	{
		ar.WriteCount( m_nCountry );
	}
	else
	{
		m_nCountry = static_cast< DWORD >( ar.ReadCount() );
		m_pCountry = new CWorldCountry[ m_nCountry ];
	}

	for ( DWORD nCountry = 0 ; nCountry < m_nCountry ; nCountry++ )
	{
		m_pCountry[ nCountry ].Serialize( ar );
	}
}

BOOL CWorldGPS::LoadFrom(CXMLElement* pRoot)
{
	if ( ! pRoot->IsNamed( _T("world") ) ) return FALSE;

	for ( POSITION pos = pRoot->GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = pRoot->GetNextElement( pos );
		if ( pElement->IsNamed( _T("country") ) ) m_nCountry++;
	}

	m_pCountry = new CWorldCountry[ m_nCountry ];
	m_nCountry = 0;

	for ( POSITION pos = pRoot->GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = pRoot->GetNextElement( pos );
		if ( pElement->IsNamed( _T("country") ) )
			m_pCountry[ m_nCountry++ ].LoadFrom( pElement );
	}

	return m_nCountry > 0;
}

//////////////////////////////////////////////////////////////////////
// CWorldGPS clear

void CWorldGPS::Clear()
{
	if ( m_pCountry ) delete [] m_pCountry;
	m_pCountry = NULL;
	m_nCountry = 0;
}


//////////////////////////////////////////////////////////////////////
// CWorldCountry construction

CWorldCountry::CWorldCountry()
{
	m_pCity = NULL;
	m_nCity = 0;
}

CWorldCountry::~CWorldCountry()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CWorldCountry serialize

void CWorldCountry::Serialize(CArchive& ar)
{
	if ( ar.IsStoring() )
	{
		ar << m_szID[0];
		ar << m_szID[1];
		ar << m_sName;

		ar.WriteCount( m_nCity );
	}
	else
	{
		ar >> m_szID[0];
		ar >> m_szID[1];
		ar >> m_sName;

		m_nCity = static_cast< DWORD >( ar.ReadCount() );
		m_pCity = new CWorldCity[ m_nCity ];
	}

	for ( DWORD nCity = 0 ; nCity < m_nCity ; nCity++ )
	{
		m_pCity[ nCity ].Serialize( ar );
	}
}

BOOL CWorldCountry::LoadFrom(CXMLElement* pRoot)
{
	if ( ! pRoot->IsNamed( _T("country") ) ) return FALSE;

	m_sName = pRoot->GetAttributeValue( _T("name") );
	CString strID = pRoot->GetAttributeValue( _T("code") );

	if ( strID.GetLength() != 2 ) return FALSE;
	m_szID[0] = (CHAR)strID.GetAt( 0 );
	m_szID[1] = (CHAR)strID.GetAt( 1 );

	for ( POSITION pos = pRoot->GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = pRoot->GetNextElement( pos );
		if ( pElement->IsNamed( _T("city") ) ) m_nCity++;
	}

	m_pCity = new CWorldCity[ m_nCity ];
	m_nCity = 0;

	for ( POSITION pos = pRoot->GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = pRoot->GetNextElement( pos );
		if ( pElement->IsNamed( _T("city") ) )
			m_pCity[ m_nCity++ ].LoadFrom( pElement );
	}

	return m_nCity > 0;
}

//////////////////////////////////////////////////////////////////////
// CWorldCountry clear

void CWorldCountry::Clear()
{
	if ( m_pCity ) delete [] m_pCity;
	m_pCity = NULL;
	m_nCity = 0;
}

//////////////////////////////////////////////////////////////////////
// CWorldCity load

void CWorldCity::Serialize(CArchive& ar)
{
	WORD nValue;

	if ( ar.IsStoring() )
	{
		ar << m_sName;
		ar << m_sState;

		nValue = (WORD)(signed short)( m_nLatitude / 90.0f * 32700.0f );
		ar << nValue;
		nValue = (WORD)(signed short)( m_nLongitude / 180.0f * 32700.0f );
		ar << nValue;
	}
	else
	{
		ar >> m_sName;
		ar >> m_sState;

		ar >> nValue;
		m_nLatitude = (float)(signed short)nValue / 32700.0f * 90.0f;

		ar >> nValue;
		m_nLongitude = (float)(signed short)nValue / 32700.0f * 180.0f;
	}
}

BOOL CWorldCity::LoadFrom(CXMLElement* pRoot)
{
	if ( ! pRoot->IsNamed( _T("city") ) ) return FALSE;

	m_sName		= pRoot->GetAttributeValue( _T("name") );
	m_sState	= pRoot->GetAttributeValue( _T("state") );

	CString strValue = pRoot->GetAttributeValue( _T("latitude") );
	if ( _stscanf( strValue, _T("%f"), &m_nLatitude ) != 1 ) return FALSE;

	strValue = pRoot->GetAttributeValue( _T("longitude") );
	if ( _stscanf( strValue, _T("%f"), &m_nLongitude ) != 1 ) return FALSE;

	return TRUE;
}

