//
// XML.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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
#include "XML.h"

#ifdef DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CXMLNode construction

CXMLNode::CXMLNode(CXMLElement* pParent, LPCTSTR pszName) :
	m_nNode		( xmlNode )
,	m_pParent	( pParent )
{
	if ( pszName )
	{
		m_sName = pszName;
		ToLower( m_sName );
	}
}

CXMLNode::~CXMLNode()
{
}

void CXMLNode::Delete()
{
	if ( this == NULL ) return;

	if ( m_pParent != NULL )
	{
		if ( m_nNode == xmlElement ) m_pParent->RemoveElement( (CXMLElement*)this );
		else if ( m_nNode == xmlAttribute ) m_pParent->RemoveAttribute( (CXMLAttribute*)this );
	}

	delete this;
}

//////////////////////////////////////////////////////////////////////
// CXMLNode parsing

BOOL CXMLNode::ParseMatch(LPCTSTR& pszBase, LPCTSTR pszToken)
{
	LPCTSTR pszXML = pszBase;
	int nParse = 0;

	for ( ; *pszXML == ' ' || *pszXML == '\t' || *pszXML == '\r' || *pszXML == '\n' ; pszXML++, nParse++ );
	if ( ! *pszXML ) return FALSE;

	for ( ; *pszXML && *pszToken ; pszXML++, pszToken++, nParse++ )
	{
		if ( *pszXML != *pszToken ) return FALSE;
	}

	pszBase += nParse;

	return TRUE;
}

BOOL CXMLNode::ParseIdentifier(LPCTSTR& pszBase, CString& strIdentifier)
{
	LPCTSTR pszXML = pszBase;
	int nParse = 0;

	while ( *pszXML == ' ' || *pszXML == '\t' || *pszXML == '\r' || *pszXML == '\n' )
	{
		pszXML++;
		nParse++;
	}
	if ( !*pszXML )
		return FALSE;

	int nIdentifier = 0;
	while ( *pszXML && ( _istalnum( *pszXML ) || *pszXML == ':' || *pszXML == '_' || *pszXML == '-' ) )
	{
		pszXML++;
		nIdentifier++;
	}
	if ( !nIdentifier )
		return FALSE;

	pszBase += nParse;
	_tcsncpy( strIdentifier.GetBuffer( nIdentifier ), pszBase, nIdentifier );
	strIdentifier.ReleaseBuffer( nIdentifier );
	pszBase += nIdentifier;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CXMLNode string to value

CString CXMLNode::StringToValue(LPCTSTR& pszXML, int nLength)
{
	CString strValue;

	if ( ! nLength || ! *pszXML ) return strValue;

	LPTSTR pszValue = strValue.GetBuffer( nLength + 4 );
	LPTSTR pszOut = pszValue;

	LPTSTR pszNull = (LPTSTR)pszXML + nLength;
	TCHAR cNull = *pszNull;
	*pszNull = 0;

	while ( *pszXML && pszXML < pszNull )
	{
		if ( _istspace( *pszXML ) )
		{
			if ( pszValue != pszOut ) *pszOut++ = ' ';
			pszXML++;
			while ( *pszXML && _istspace( *pszXML ) ) pszXML++;
			if ( ! *pszXML || pszXML >= pszNull ) break;
		}

		if ( *pszXML == '&' )
		{
			pszXML++;
			if ( ! *pszXML || pszXML >= pszNull ) break;

			if ( _tcsnicmp( pszXML, _T("amp;"), 4 ) == 0 )
			{
				*pszOut++ = '&';
				pszXML += 4;
			}
			else if ( _tcsnicmp( pszXML, _T("lt;"), 3 ) == 0 )
			{
				*pszOut++ = '<';
				pszXML += 3;
			}
			else if ( _tcsnicmp( pszXML, _T("gt;"), 3 ) == 0 )
			{
				*pszOut++ = '>';
				pszXML += 3;
			}
			else if ( _tcsnicmp( pszXML, _T("quot;"), 5 ) == 0 )
			{
				*pszOut++ = '\"';
				pszXML += 5;
			}
			else if ( _tcsnicmp( pszXML, _T("apos;"), 5 ) == 0 )
			{
				*pszOut++ = '\'';
				pszXML += 5;
			}
			else if ( _tcsnicmp( pszXML, _T("nbsp;"), 5 ) == 0 )
			{
				*pszOut++ = ' ';
				pszXML += 5;
			}
			else if ( *pszXML == '#' )
			{
				int nChar;
				pszXML++;
				if ( ! *pszXML || pszXML >= pszNull || ! _istdigit( *pszXML ) ) break;

				if ( _stscanf( pszXML, _T("%lu;"), &nChar ) == 1 )
				{
					*pszOut++ = (TCHAR)nChar;
					while ( *pszXML && *pszXML != ';' ) pszXML++;
					if ( ! *pszXML || pszXML >= pszNull ) break;
					pszXML++;
				}
			}
			else
			{
				*pszOut++ = '&';
			}
		}
		else
		{
			*pszOut++ = *pszXML++;
		}
	}

	ASSERT( pszNull == pszXML );
	*pszNull = cNull;

	ASSERT( pszOut - pszValue <= nLength );
	strValue.ReleaseBuffer( (int)( pszOut - pszValue ) );

	return strValue;
}

//////////////////////////////////////////////////////////////////////
// CXMLNode value to string

#define V2S_APPEND(x,y)	\
	if ( (x) > nOut ) \
	{ \
		strXML.ReleaseBuffer( nLen + nOut ); \
		nOut += (x) + 16; \
		pszOut = strXML.GetBuffer( nLen + nOut ) + nLen; \
	} \
	{ for ( LPCTSTR pszIn = (y) ; *pszIn ; nOut--, nLen++ ) *pszOut++ = *pszIn++; }

void CXMLNode::ValueToString(const CString& strValue, CString& strXML)
{
	int nLen = strXML.GetLength();
	int nOut = strValue.GetLength();
	LPTSTR pszOut = strXML.GetBuffer( nLen + nOut ) + nLen;
	LPCTSTR pszValue = strValue;
	for ( int i = nOut ; i != 0 ; --i, ++pszValue )
	{
		int nChar = (int)(unsigned short)*pszValue;

		switch ( nChar )
		{
		case '&':
			V2S_APPEND( 5, _T("&amp;") );
			break;
		case '<':
			V2S_APPEND( 4, _T("&lt;") );
			break;
		case '>':
			V2S_APPEND( 4, _T("&gt;") );
			break;
		case '\"':
			V2S_APPEND( 6, _T("&quot;") );
			break;
		case '\'':
			V2S_APPEND( 6, _T("&apos;") );
			break;
		default:
			if ( nChar > 127 )
			{
				CString strItem;
				strItem.Format( _T("&#%lu;"), nChar );
				V2S_APPEND( strItem.GetLength(), strItem );
			}
			else if ( nOut > 0 )
			{
				*pszOut++ = WCHAR( nChar );
				nOut--;
				nLen++;
			}
			else
			{
				strXML.ReleaseBuffer( nLen + nOut );
				nOut += 16;
				pszOut = strXML.GetBuffer( nLen + nOut ) + nLen;
				*pszOut++ = WCHAR( nChar );
				nOut--;
				nLen++;
			}
			break;
		}
	}

	strXML.ReleaseBuffer( nLen );
}

//////////////////////////////////////////////////////////////////////
// CXMLNode serialize

void CXMLNode::Serialize(CArchive& ar)
{
	if ( ar.IsStoring() )
	{
		ar << m_sName;
		ar << m_sValue;
	}
	else
	{
		ar >> m_sName;
		ar >> m_sValue;
	}
}

//////////////////////////////////////////////////////////////////////
// CXMLNode string helper

void CXMLNode::UniformString(CString& str)
{
	// non-alphanumeric characters which will not be ignored
	static LPCTSTR pszOK = _T("'-&/,;#()");

	str.Trim();
	BOOL bSpace = TRUE;

	for ( int nPos = 0 ; nPos < str.GetLength() ; nPos++ )
	{
		int nChar = (int)(unsigned short)str.GetAt( nPos );

		if ( nChar <= 32 )
		{
			if ( bSpace )
			{
				str = str.Left( nPos ) + str.Mid( nPos + 1 );
				nPos--;
			}
			else
			{
				if ( nChar != 32 ) str.SetAt( nPos, 32 );
				bSpace = TRUE;
			}
		}
		else if ( ! _istalnum( TCHAR( nChar ) ) && nChar < 0xC0 && _tcschr( pszOK, TCHAR( nChar ) ) == NULL )
		{
			if ( nPos == 0 || str.GetAt( nPos - 1 ) == ' ' )
				str = str.Left( nPos ) + str.Mid( nPos + 1 );
			else
			{
				LPTSTR pszTemp = _tcsninc( str, nPos );
				pszTemp[ 0 ] = ' ';
			}
		}
		else
		{
			bSpace = FALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////
// CXMLElement construction

CXMLElement::CXMLElement(CXMLElement* pParent, LPCTSTR pszName) : CXMLNode( pParent, pszName )
{
	m_nNode = xmlElement;
}

CXMLElement::~CXMLElement()
{
	DeleteAllElements();
	DeleteAllAttributes();
}

CXMLElement* CXMLElement::AddElement(LPCTSTR pszName)
{
	CXMLElement* pElement = new CXMLElement( this, pszName );
	m_pElements.AddTail( pElement );
	return pElement;
}

//////////////////////////////////////////////////////////////////////
// CXMLElement clone

CXMLElement* CXMLElement::Clone(CXMLElement* pParent) const
{
	CXMLElement* pClone = new CXMLElement( pParent, m_sName );

	for ( POSITION pos = GetAttributeIterator() ; pos ; )
	{
		CXMLAttribute* pAttribute = GetNextAttribute( pos )->Clone( pClone );
		CString strName( pAttribute->m_sName );

		// Convert to lowercase with CLowerCaseTable
		ToLower( strName );

		// Delete the old attribute if one exists
		CXMLAttribute* pExisting;
		if ( pClone->m_pAttributes.Lookup( strName, pExisting ) )
			delete pExisting;

		pClone->m_pAttributes.SetAt( strName, pAttribute );
	}

	for ( POSITION pos = GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = GetNextElement( pos );
		pClone->m_pElements.AddTail( pElement->Clone( pClone ) );
	}

	pClone->m_sValue = m_sValue;

	return pClone;
}

//////////////////////////////////////////////////////////////////////
// CXMLElement delete

void CXMLElement::DeleteAllElements()
{
	for ( POSITION pos = m_pElements.GetHeadPosition() ; pos ; )
	{
		delete m_pElements.GetNext( pos );
	}
	m_pElements.RemoveAll();
}

void CXMLElement::DeleteAllAttributes()
{
	for ( POSITION pos = m_pAttributes.GetStartPosition() ; pos ; )
	{
		CXMLAttribute* pAttribute = NULL;
		CString strName;

		m_pAttributes.GetNextAssoc( pos, strName, pAttribute );
		delete pAttribute;
	}
	m_pAttributes.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CXMLElement to string

CString CXMLElement::ToString(BOOL bHeader, BOOL bNewline) const
{
	CString strXML;
	if ( bHeader )
		strXML = _T("<?xml version=\"1.0\"?>");
	if ( bNewline )
		strXML += _T("\r\n");
	ToString( strXML, bNewline );
	ASSERT( strXML.GetLength() == int( _tcslen(strXML) ) );
	return strXML;
}

void CXMLElement::ToString(CString& strXML, BOOL bNewline) const
{
	strXML += '<' + m_sName;

	POSITION pos = GetAttributeIterator();
	for ( ; pos ; )
	{
		strXML += ' ';
		CXMLAttribute* pAttribute = GetNextAttribute( pos );
		pAttribute->ToString( strXML );
	}

	pos = GetElementIterator();

	if ( pos == NULL && m_sValue.IsEmpty() )
	{
		strXML += _T("/>");
		if ( bNewline ) strXML += _T("\r\n");
		return;
	}

	strXML += '>';
	if ( bNewline && pos ) strXML += _T("\r\n");

	while ( pos )
	{
		CXMLElement* pElement = GetNextElement( pos );
		pElement->ToString( strXML, bNewline );
	}

	ValueToString( m_sValue, strXML );

	strXML += _T("</") + m_sName + '>';
	if ( bNewline ) strXML += _T("\r\n");
}

//////////////////////////////////////////////////////////////////////
// CXMLElement from string

CXMLElement* CXMLElement::FromString(LPCTSTR pszXML, BOOL bHeader)
{
	CXMLElement* pElement	= NULL;
	LPCTSTR pszElement		= NULL;

	try
	{
		if ( ParseMatch( pszXML, _T("<?xml version=\"") ) )
		{
			pszElement = _tcsstr( pszXML, _T("?>") );
			if ( !pszElement )
				return FALSE;
			pszXML = pszElement + 2;
		}
		else if ( bHeader )
			return NULL;

		while ( ParseMatch( pszXML, _T("<!--") ) )
		{
			pszElement = _tcsstr( pszXML, _T("-->") );
			if ( !pszElement || *pszElement != '-' )
				return FALSE;
			pszXML = pszElement + 3;
		}

		while ( ParseMatch( pszXML, _T("<?xml") ) )
		{
			pszElement = _tcsstr( pszXML, _T("?>") );
			if ( !pszElement )
				return FALSE;
			pszXML = pszElement + 2;
		}

		if ( ParseMatch( pszXML, _T("<!DOCTYPE") ) )
		{
			pszElement = _tcsstr( pszXML, _T(">") );
			if ( !pszElement )
				return FALSE;
			pszXML = pszElement + 1;
		}

		while ( ParseMatch( pszXML, _T("<!--") ) )
		{
			pszElement = _tcsstr( pszXML, _T("-->") );
			if ( !pszElement || *pszElement != '-' )
				return FALSE;
			pszXML = pszElement + 3;
		}

		pElement = new CXMLElement();

		if ( !pElement->ParseString( pszXML ) )
		{
			delete pElement;
			pElement = NULL;
		}
	}
	catch ( CException* pException )
	{
		pException->Delete();
		delete pElement;
		pElement = NULL;
	}

	return pElement;
}

BOOL CXMLElement::ParseString(LPCTSTR& strXML)
{
	if ( !ParseMatch( strXML, _T("<") ) )
		return FALSE;

	if ( !ParseIdentifier( strXML, m_sName ) )
		return FALSE;

	while ( ! ParseMatch( strXML, _T(">") ) )
	{
		if ( ParseMatch( strXML, _T("/") ) )
		{
			return ParseMatch( strXML, _T(">") );
		}

		if ( !*strXML )
			return FALSE;

		CXMLAttribute* pAttribute = new CXMLAttribute( this );

		if ( pAttribute->ParseString( strXML ) )
		{
			CString strName( pAttribute->m_sName );

			// Convert to lowercase with CLowerCaseTable
			ToLower( strName );

			// Delete the old attribute if one exists
			CXMLAttribute* pExisting;
			if ( m_pAttributes.Lookup( strName, pExisting ) )
				delete pExisting;

			m_pAttributes.SetAt( strName, pAttribute );
		}
		else
		{
			delete pAttribute;
			return FALSE;
		}
	}

	CString strClose = _T("</");
	strClose += m_sName + '>';

	for (;;)
	{
		if ( !*strXML )
			return FALSE;

		LPCTSTR pszElement = _tcschr( strXML, '<' );
		if ( !pszElement || *pszElement != '<' )
			return FALSE;

		if ( ParseMatch( strXML, _T("<![CDATA[") ) )
		{
			pszElement = _tcsstr( strXML, _T("]]>") );
			if ( !pszElement || *pszElement != ']' )
				return FALSE;
			if ( m_sValue.GetLength() && m_sValue.Right( 1 ) != ' ' )
				m_sValue += ' ';
			m_sValue += StringToValue( strXML, (int)( pszElement - strXML ) );
			pszElement += 3;
			strXML = pszElement;
		}

		if ( pszElement > strXML )
		{
			if ( m_sValue.GetLength() && m_sValue.Right( 1 ) != ' ' )
				m_sValue += ' ';
			m_sValue += StringToValue( strXML, (int)( pszElement - strXML ) );
			ASSERT( strXML == pszElement );
			if ( strXML != pszElement )
				return FALSE;
		}

		if ( ParseMatch( strXML, strClose ) )
		{
			break;
		}
		else if ( ParseMatch( strXML, _T("<!--") ) )
		{
			pszElement = _tcsstr( strXML, _T("-->") );
			if ( !pszElement || *pszElement != '-' )
				return FALSE;
			strXML = pszElement + 3;
		}
		else
		{
			CXMLElement* pElement = new CXMLElement( this );

			if ( pElement->ParseString( strXML ) )
			{
				m_pElements.AddTail( pElement );
			}
			else
			{
				delete pElement;
				return FALSE;
			}
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CXMLElement from bytes

CXMLElement* CXMLElement::FromBytes(BYTE* pByte, DWORD nByte, BOOL bHeader)
{
	CString strXML;

	if ( nByte >= 2 && ( ( pByte[0] == 0xFE && pByte[1] == 0xFF ) || ( pByte[0] == 0xFF && pByte[1] == 0xFE ) ) )
	{
		nByte = nByte / 2 - 1;

		if ( pByte[0] == 0xFE && pByte[1] == 0xFF )
		{
			pByte += 2;

			for ( DWORD nSwap = 0 ; nSwap < nByte ; nSwap ++ )
			{
				register CHAR nTemp = pByte[ ( nSwap << 1 ) + 0 ];
				pByte[ ( nSwap << 1 ) + 0 ] = pByte[ ( nSwap << 1 ) + 1 ];
				pByte[ ( nSwap << 1 ) + 1 ] = nTemp;
			}
		}
		else
		{
			pByte += 2;
		}

		CopyMemory( strXML.GetBuffer( nByte ), pByte, nByte * sizeof(TCHAR) );
		strXML.ReleaseBuffer( nByte );
	}
	else
	{
		if ( nByte >= 3 && pByte[0] == 0xEF && pByte[1] == 0xBB && pByte[2] == 0xBF )
		{
			pByte += 3; nByte -= 3;
		}

		strXML = UTF8Decode( (LPCSTR)pByte, nByte );
	}

	return FromString( strXML, bHeader );
}

//////////////////////////////////////////////////////////////////////
// CXMLElement from file

CXMLElement* CXMLElement::FromFile(LPCTSTR pszPath, BOOL bHeader)
{
	HANDLE hFile = CreateFile(	pszPath, GENERIC_READ, FILE_SHARE_READ, NULL,
								OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

	if ( hFile == INVALID_HANDLE_VALUE ) return NULL;

	CXMLElement* pXML = FromFile( hFile, bHeader );

	CloseHandle( hFile );

	return pXML;
}

CXMLElement* CXMLElement::FromFile(HANDLE hFile, BOOL bHeader)
{
	DWORD nByte = GetFileSize( hFile, NULL );
	if ( nByte > 4096*1024 ) return FALSE;

	auto_array< BYTE > pByte( new BYTE[ nByte ] );

	if ( ! ReadFile( hFile, pByte.get(), nByte, &nByte, NULL ) ) return FALSE;

	return FromBytes( pByte.get(), nByte, bHeader );
}

//////////////////////////////////////////////////////////////////////
// CXMLElement equality

BOOL CXMLElement::Equals(CXMLElement* pXML) const
{
	if ( this == NULL || pXML == NULL ) return FALSE;
	if ( pXML == this ) return TRUE;

	if ( m_sName != pXML->m_sName ) return FALSE;
	if ( m_sValue != pXML->m_sValue ) return FALSE;

	if ( GetAttributeCount() != pXML->GetAttributeCount() ) return FALSE;
	if ( GetElementCount() != pXML->GetElementCount() ) return FALSE;

	for ( POSITION pos = GetAttributeIterator() ; pos ; )
	{
		CXMLAttribute* pAttribute1 = GetNextAttribute( pos );
		CXMLAttribute* pAttribute2 = pXML->GetAttribute( pAttribute1->m_sName );
		if ( pAttribute2 == NULL ) return FALSE;
		if ( ! pAttribute1->Equals( pAttribute2 ) ) return FALSE;
	}

	POSITION pos1 = GetElementIterator();
	POSITION pos2 = pXML->GetElementIterator();

	for ( ; pos1 && pos2 ; )
	{
		CXMLElement* pElement1 = GetNextElement( pos1 );
		CXMLElement* pElement2 = pXML->GetNextElement( pos2 );
		if ( pElement1 == NULL || pElement2 == NULL ) return FALSE;
		if ( ! pElement1->Equals( pElement2 ) ) return FALSE;
	}

	if ( pos1 != NULL || pos2 != NULL ) return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CXMLElement Metadata merge

BOOL CXMLElement::Merge(const CXMLElement* pInput, BOOL bOverwrite)
{
	if ( ! this || ! pInput ) return FALSE;
	if ( this == pInput ) return TRUE;
	if ( m_sName != pInput->m_sName ) return FALSE;

	TRACE( _T("Merging XML:%sand XML:%s"),
		ToString( FALSE, TRUE ), pInput->ToString( FALSE, TRUE ) );

	BOOL bChanged = FALSE;

	for ( POSITION pos = pInput->GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement	= pInput->GetNextElement( pos );
		CXMLElement* pTarget	= GetElementByName( pElement->m_sName );

		if ( pTarget == NULL )
		{
			AddElement( pElement->Clone() );
			bChanged = TRUE;
		}
		else if ( pTarget->Merge( pElement, bOverwrite ) )
		{
			bChanged = TRUE;
		}
	}

	for ( POSITION pos = pInput->GetAttributeIterator() ; pos ; )
	{
		CXMLAttribute* pAttribute	= pInput->GetNextAttribute( pos );
		CXMLAttribute* pTarget		= GetAttribute( pAttribute->m_sName );

		if ( pTarget == NULL )
		{
			AddAttribute( pAttribute->Clone() );
			bChanged = TRUE;
		}
		else if ( bOverwrite && ! pTarget->Equals( pAttribute ) )
		{
			pTarget->SetValue( pAttribute->GetValue() );
			bChanged = TRUE;
		}
	}

	if ( bChanged )
	{
		TRACE( _T("resulting XML:%s\n"), ToString( FALSE, TRUE ) );
	}

	return bChanged;
}

//////////////////////////////////////////////////////////////////////
// CXMLElement recursive word accumulation

CString CXMLElement::GetRecursiveWords() const
{
	CString strWords;

	AddRecursiveWords( strWords );
	strWords.TrimLeft();
	strWords.TrimRight();

	return strWords;
}

void CXMLElement::AddRecursiveWords(CString& strWords) const
{
	for ( POSITION pos = GetAttributeIterator() ; pos ; )
	{
		CXMLAttribute* pAttribute = GetNextAttribute( pos );
		CString strText = pAttribute->GetName();

		if ( strText.Find( ':' ) >= 0 ) continue;
		if ( strText.CompareNoCase( _T("SHA1") ) == 0 ) continue;	// NOTE: Shareaza Specific

		if ( strWords.GetLength() ) strWords += ' ';
		strWords += pAttribute->GetValue();
	}

	for ( POSITION pos = GetElementIterator() ; pos ; )
	{
		GetNextElement( pos )->AddRecursiveWords( strWords );
	}

	if ( m_sValue.GetLength() )
	{
		if ( strWords.GetLength() ) strWords += ' ';
		strWords += m_sValue;
	}
}

//////////////////////////////////////////////////////////////////////
// CXMLElement serialize

void CXMLElement::Serialize(CArchive& ar)
{
	CXMLNode::Serialize( ar );

	if ( ar.IsStoring() )
	{
		ar.WriteCount( GetAttributeCount() );

		for ( POSITION pos = GetAttributeIterator() ; pos ; )
		{
			GetNextAttribute( pos )->Serialize( ar );
		}

		ar.WriteCount( GetElementCount() );

		for ( POSITION pos = GetElementIterator() ; pos ; )
		{
			GetNextElement( pos )->Serialize( ar );
		}
	}
	else
	{
		for ( int nCount = (int)ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			CXMLAttribute* pAttribute = new CXMLAttribute( this );
			pAttribute->Serialize( ar );

			// Skip attribute if name is missing
			if ( pAttribute->m_sName.IsEmpty() )
			{
				delete pAttribute;
				continue;
			}

			CString strName( pAttribute->m_sName );

			// Convert to lowercase with CLowerCaseTable
			ToLower( strName );

			// Delete the old attribute if one exists
			CXMLAttribute* pExisting;
			if ( m_pAttributes.Lookup( strName, pExisting ) )
				delete pExisting;

			m_pAttributes.SetAt( strName, pAttribute );
		}

		for ( int nCount = (int)ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			CXMLElement* pElement = new CXMLElement( this );
			pElement->Serialize( ar );
			m_pElements.AddTail( pElement );
		}
	}
}


//////////////////////////////////////////////////////////////////////
// CXMLAttribute construction

LPCTSTR CXMLAttribute::xmlnsSchema		= _T("http://www.w3.org/2001/XMLSchema");
LPCTSTR CXMLAttribute::xmlnsInstance	= _T("http://www.w3.org/2001/XMLSchema-instance");
LPCTSTR CXMLAttribute::schemaName		= _T("xsi:noNamespaceSchemaLocation");

CXMLAttribute::CXMLAttribute(CXMLElement* pParent, LPCTSTR pszName) : CXMLNode( pParent, pszName )
{
	m_nNode = xmlAttribute;
}

CXMLAttribute::~CXMLAttribute()
{
}

CXMLAttribute* CXMLElement::AddAttribute(LPCTSTR pszName, LPCTSTR pszValue)
{
	CXMLAttribute* pAttribute = GetAttribute( pszName );

	if ( ! pAttribute )
	{
		pAttribute = new CXMLAttribute( this, pszName );
		CString strName( pszName );

		// Convert to lowercase with CLowerCaseTable
		ToLower( strName );

		// Delete the old attribute if one exists
		CXMLAttribute* pExisting;
		if ( m_pAttributes.Lookup( strName, pExisting ) )
			delete pExisting;

		m_pAttributes.SetAt( strName, pAttribute );
	}

	if ( pszValue ) pAttribute->SetValue( pszValue );

	return pAttribute;
}

CXMLAttribute* CXMLElement::AddAttribute(CXMLAttribute* pAttribute)
{
	if ( pAttribute->m_pParent ) return NULL;
	CString strName( pAttribute->m_sName );

	// Convert to lowercase with CLowerCaseTable
	ToLower( strName );

	// Delete the old attribute if one exists
	CXMLAttribute* pExisting;
	if ( m_pAttributes.Lookup( strName, pExisting ) )
		delete pExisting;

	m_pAttributes.SetAt( pAttribute->m_sName, pAttribute );
	pAttribute->m_pParent = this;
	return pAttribute;
}

//////////////////////////////////////////////////////////////////////
// CXMLAttribute clone

CXMLAttribute* CXMLAttribute::Clone(CXMLElement* pParent) const
{
	CXMLAttribute* pClone = new CXMLAttribute( pParent, m_sName );
	pClone->m_sValue = m_sValue;
	return pClone;
}

//////////////////////////////////////////////////////////////////////
// CXMLAttribute to string

void CXMLAttribute::ToString(CString& strXML) const
{
	strXML += m_sName + _T("=\"");
	ValueToString( m_sValue, strXML );
	strXML += '\"';
}

//////////////////////////////////////////////////////////////////////
// CXMLAttribute from string

BOOL CXMLAttribute::ParseString(LPCTSTR& strXML)
{
	if ( !ParseIdentifier( strXML, m_sName ) )
		return FALSE;
	if ( !ParseMatch( strXML, _T("=") ) )
		return FALSE;

	if ( ParseMatch( strXML, _T("\"") ) )
	{
		LPCTSTR pszQuote = _tcschr( strXML,  '\"' );
		if ( !pszQuote || *pszQuote != '\"' )
			return FALSE;

		m_sValue = StringToValue( strXML, (int)( pszQuote - strXML ) );

		return ParseMatch( strXML, _T("\"") );
	}
	else if ( ParseMatch( strXML, _T("'") ) )
	{
		LPCTSTR pszQuote = _tcschr( strXML,  '\'' );
		if ( !pszQuote || *pszQuote != '\'' )
			return FALSE;

		m_sValue = StringToValue( strXML, (int)( pszQuote - strXML ) );

		return ParseMatch( strXML, _T("\'") );
	}
	else
	{
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CXMLAttribute equality

BOOL CXMLAttribute::Equals(CXMLAttribute* pXML) const
{
	if ( this == NULL || pXML == NULL ) return FALSE;
	if ( pXML == this ) return TRUE;

	if ( m_sName != pXML->m_sName ) return FALSE;
	if ( m_sValue != pXML->m_sValue ) return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CXMLAttribute serialize

void CXMLAttribute::Serialize(CArchive& ar)
{
	CXMLNode::Serialize( ar );
}
