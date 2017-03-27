//
// SkinTranslate.cpp
//
// Copyright (c) Shareaza Development Team, 2009-2017.
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
#include "SkinTranslate.h"


inline bool isspec( LPCTSTR szChar )
{
	if ( *szChar == _T('.') || *szChar == _T('+') || *szChar == _T('-') )
		return isspec( szChar + 1 );
	else
		return ( *szChar != 0 ) && ( _tcschr( _T( "aAcCdeEfgGhinoplsSIwxXZ0123456789#" ), *szChar ) != NULL );
}

CXMLLoader::CItem::CItem()
	: bKeepUnderscores	( false )
	, bFuzzy			( false )
	, bError			( false )
{
}

CXMLLoader::CItem::CItem( const CItem& it )
	: sRef				( it.sRef )
	, sID				( it.sID )
	, sTranslated		( it.sTranslated )
	, bKeepUnderscores	( it.bKeepUnderscores )
	, bFuzzy			( it.bFuzzy )
	, bError			( it.bError )
{
}

CXMLLoader::CItem::CItem( const CString& r, const CString& t, bool k )
	: sRef				( r )
	, sID				( t )
	, bKeepUnderscores	( k )
	, bFuzzy			( false )
	, bError			( false )
{
}

CXMLLoader::CItem& CXMLLoader::CItem::operator=( const CItem& it )
{
	sRef = it.sRef;
	sID = it.sID;
	sTranslated = it.sTranslated;
	bKeepUnderscores = it.bKeepUnderscores;
	bFuzzy = it.bFuzzy;
	bError = it.bError;
	return *this;
}

void CXMLLoader::CItem::Clear()
{
	sRef.Empty();
	sID.Empty();
	sTranslated.Empty();
	bKeepUnderscores = false;
	bFuzzy = false;
	bError = false;
}

void CXMLLoader::CItem::SetTranslate(LPCSTR szText)
{
	sTranslated = CXMLLoader::UnMakeSafe( CXMLLoader::UTF8Decode( szText ) );
	
	// Compare amount of "|" and "%" in original and translated texts

	int nVerticalBar = 0;//, nColumn = 0;
	CAtlList< TCHAR > oSpecs;
	for ( LPCTSTR ch = sTranslated; *ch; ++ch )
	{
		if ( *ch == _T('|') )
			++nVerticalBar;
		else if ( *ch == _T('%') && isspec( ch + 1 ) )
			oSpecs.AddTail( *(ch + 1) );
	//	else if ( *ch == _T(':') )
	//		++nColumn;
	}

	for ( LPCTSTR ch = sID; *ch; ++ch )
	{
		if ( *ch == _T('|') )
			--nVerticalBar;
		else if ( *ch == _T('%') && isspec( ch + 1 ) )
		{
			if ( oSpecs.IsEmpty() || oSpecs.RemoveHead() != *(ch + 1) )
			{
				bError = true;
				return;
			}
		}
	//	else if ( *ch == _T(':') )
	//		--nColumn;
	}

	if ( nVerticalBar != 0 || ! oSpecs.IsEmpty() /* || nColumn != 0 */)
	{
		bError = true;
	}
}

void CXMLLoader::CItem::SetID(LPCSTR szText)
{
	sID = CXMLLoader::UnMakeSafe( CXMLLoader::UTF8Decode( szText ) );
}

CXMLLoader::CXMLLoader() :
	 m_bRemoveComments( false ),
	 m_pXMLTranslator ( NULL )
{
	m_IDIndex.InitHashTable( 2039 );
	m_RefIndex.InitHashTable( 2039 );
}

CXMLLoader::CItem& CXMLLoader::Add(const CItem& item)
{
	// Add item list
	POSITION pos = m_Items.AddTail( item );
	ATLASSERT( pos );

	CItem& inserted = m_Items.GetAt( pos );
	inserted.sRef.MakeLower();

	// Add to message ID index
	ATLVERIFY( m_IDIndex.SetAt( item.sID, &inserted ) );

	// Add to references index
	int curPos = 0;
	CString sToken = inserted.sRef.Tokenize( _T(" "), curPos );
	while ( ! sToken.IsEmpty() )
	{
		sToken.Trim();
		if ( ! sToken.IsEmpty() )
		{
			ATLVERIFY( m_RefIndex.SetAt( sToken, &inserted ) );
		}
		sToken = inserted.sRef.Tokenize( _T(" "), curPos );
	};

	return inserted;
}

bool CXMLLoader::LoadPO(LPCWSTR szFilename)
{
	_tprintf( _T("Loading PO file: %s\n"), szFilename );

	m_Items.RemoveAll();

	CAtlFile oFile;
	if ( SUCCEEDED( oFile.Create( szFilename, GENERIC_READ, 0, OPEN_EXISTING ) ) )
	{
		ULONGLONG nLength = 0;
		if ( SUCCEEDED( oFile.GetSize( nLength ) ) )
		{
			CStringA sFile;
			if ( SUCCEEDED( oFile.Read(
				sFile.GetBuffer( (DWORD)nLength + 1 ), (DWORD)nLength ) ) )
			{
				sFile.ReleaseBuffer( (DWORD)nLength );

				CItem item;
				CStringA sString, sOriginalLine;
				enum
				{
					mode_start, mode_ref, mode_msgid, mode_msgstr
				}
				mode = mode_start;

				int nLine = 1;
				int curPos = 0;
				CStringA sLine = sFile.Tokenize( "\n", curPos );
				while ( ! sLine.IsEmpty() )
				{
					sOriginalLine = sLine;
					sLine.Trim();

					switch ( sLine[ 0 ] )
					{
					case '#':
						if ( mode == mode_msgid )
						{
							_tprintf( _T("ERROR: Invalid .po-file line #%d: %s\n"), nLine, (LPCTSTR)CA2T( sOriginalLine ) );
							return false;
						}
						if ( mode == mode_msgstr )
						{
							// Save previous string
							item.SetTranslate( sString );

							sString.Empty();

							// Save previous non-empty string
							if ( item.sRef != "" )
							{
								Add( item );
							}

							item.Clear();
						}
						if ( sLine[ 1 ] == ':' )
						{
							// Ref
							mode = mode_ref;
							if ( ! sString.IsEmpty() )
								sString += " ";
							sString += sLine.Mid( 2 ).Trim();
						}
						else if ( sLine[ 1 ] == ',' )
						{
							// Options
							if ( sLine.Find( "fuzzy" ) != -1 )
								item.bFuzzy = true;
						}
						// else Comments
						break;

					case 0:
						// Empty line
						break;

					case 'm':
						if ( sLine.Mid( 0, 7 ) == "msgid \"" )
						{
							if( mode != mode_start && mode != mode_ref )
							{
								_tprintf( _T("ERROR: Invalid .po-file line #%d: %s\n"), nLine,  (LPCTSTR)CA2T( sOriginalLine ) );
								return false;
							}
							
							// Save previous string
							item.sRef = sString.Trim();

							sString.Empty();

							sLine = sLine.Mid( 6, sLine.GetLength() - 6 );
							mode = mode_msgid;
						}
						else if ( sLine.Mid( 0, 8 ) == "msgstr \"" )
						{
							if ( mode != mode_msgid )
							{
								_tprintf( _T("ERROR: Invalid .po-file line #%d: %s\n"), nLine,  (LPCTSTR)CA2T( sOriginalLine ) );
								return false;
							}
							
							// Save previous string
							item.SetID( sString );

							sString.Empty();

							sLine = sLine.Mid( 7, sLine.GetLength() - 7 );
							mode = mode_msgstr;
						}
						else
						{
							// Unknown string
							_tprintf( _T("ERROR: Invalid .po-file line #%d: %s\n"), nLine, (LPCTSTR)CA2T( sOriginalLine ) );
							return false;
						}

					case '\"':
						if( mode != mode_msgid && mode != mode_msgstr )
						{
							_tprintf( _T("ERROR: Invalid .po-file line #%d: %s\n"), nLine, (LPCTSTR)CA2T( sOriginalLine ) );
							return false;
						}
						if ( sLine[ sLine.GetLength() - 1 ] == '\"' )
						{
							// String continue
							sString += sLine.Mid( 1, sLine.GetLength() - 2 );
						}
						else
						{
							// Unknown string
							_tprintf( _T("ERROR: Invalid .po-file line #%d: %s\n"), nLine, (LPCTSTR)CA2T( sOriginalLine ) );
							return false;
						}
						break;

					default:
						// Unknown string
						_tprintf( _T("ERROR: Invalid .po-file line #%d: %s\n"), nLine, (LPCTSTR)CA2T( sOriginalLine ) );
						return false;
					}

					// Get next line
					sLine = sFile.Tokenize( "\n", curPos );
					nLine++;
				} // while

				if ( mode != mode_msgstr )
				{
					// Unknown string
					_tprintf( _T("ERROR: Invalid .po-file line #%d: %s\n"), nLine, (LPCTSTR)CA2T( sOriginalLine ) );
					return false;
				}

				// Save last string
				item.SetTranslate( sString );
				if ( item.sRef != "" )
				{
					Add( item );
				}

				if ( ! m_Items.IsEmpty() )
					return true;

				_tprintf( _T("ERROR: PO file is empty: %s\n"), szFilename );
			}
			else
				_tprintf( _T("ERROR: Can't read PO file: %s\n"), szFilename );
		}
		else
			_tprintf( _T("ERROR: Can't read PO file: %s\n"), szFilename );
	}
	else
		_tprintf( _T("ERROR: Can't open PO file: %s\n"), szFilename );

	return false;
}

bool CXMLLoader::LoadXML(LPCWSTR szFilename, const CXMLLoader* pTranslator)
{
	m_pXMLTranslator = pTranslator;

	_tprintf( _T("Loading %sXML file: %s\n"), ( pTranslator ? _T("and translating ") : _T("") ), szFilename );

	ATLASSERT( ! m_pXMLDoc );
	HRESULT hr = m_pXMLDoc.CoCreateInstance( CLSID_DOMDocument );
	if ( SUCCEEDED( hr ) )
	{
		hr = m_pXMLDoc->put_async( 0 );
		if ( SUCCEEDED( hr ) )
		{
			VARIANT_BOOL ret = 0;
			hr = m_pXMLDoc->load( CComVariant( szFilename ), &ret );
			if ( ( hr == S_OK ) && ret )
			{
				CComPtr< IXMLDOMElement > pXMLRoot;
				hr = m_pXMLDoc->get_documentElement( &pXMLRoot );
				if ( hr == S_OK )
				{
					CComBSTR name;
					if ( pXMLRoot->get_nodeName( &name ) == S_OK )
					{
						name.ToLower();
						if ( name == _T("skin") )
						{
							if ( LoadSkin( pXMLRoot ) )
							{
								m_pXMLTranslator = NULL;

								return true;
							}
						}
						else
							_tprintf( _T("ERROR: Can't find <skin> tag\n") );
					}
					else
						_tprintf( _T("ERROR: Can't find valid root element\n") );
				}
				else
					_tprintf( _T("ERROR: Can't find XML tags\n") );
			}
			else
			{
				_tprintf( _T("ERROR: Can't read XML file: %s\n"), szFilename );
				CComPtr< IXMLDOMParseError > pError;
				if ( SUCCEEDED ( m_pXMLDoc->get_parseError( &pError ) ) )
				{
					long nCode = 0;
					pError->get_errorCode( &nCode );
					CComBSTR bstrReason;
					pError->get_reason( &bstrReason );
					CComBSTR bstrText;
					pError->get_srcText( &bstrText );
					long nLineNumber = 0;
					pError->get_line( &nLineNumber );
					_tprintf( _T("XML error 0x%08x: %sat line #%d: %s\n"), (DWORD)nCode, (LPCWSTR)bstrReason, nLineNumber, (LPCWSTR)bstrText );
				}
			}
		}
		else
			_tprintf( _T("ERROR: Can't put XML object in sync mode\n") );
	}
	else
		_tprintf( _T("ERROR: Can't create MS XML object\n") );

	m_pXMLTranslator = NULL;

	return false;
}

void CXMLLoader::Translate(const CXMLLoader& oTranslator)
{
	for ( POSITION pos1 = m_Items.GetHeadPosition(); pos1; )
	{
		CItem& item = m_Items.GetNext( pos1 );

		int curPos = 0;
		CString sToken = item.sRef.Tokenize( _T(" "), curPos );
		while ( ! sToken.IsEmpty() )
		{
			sToken.Trim();
			if ( ! sToken.IsEmpty() )
			{
				CItem* translated = NULL;
				if ( oTranslator.m_RefIndex.Lookup( sToken, translated ) )
				{
					ATLASSERT( item.sTranslated.IsEmpty() );

					if ( translated->sTranslated.IsEmpty() )
						item.sTranslated = translated->sID;
					else
						item.sTranslated = translated->sTranslated;

					break;
				}
			}
			sToken = item.sRef.Tokenize( _T(" "), curPos );
		};

		if ( item.sTranslated.IsEmpty() )
			_tprintf( _T("WARNING: Untranslated message \"%s\": \"%s\"\n"), (LPCTSTR)item.sRef, (LPCTSTR)item.sID );
	}
}

bool CXMLLoader::SaveXML(LPCTSTR szFilename) const
{
	_tprintf( _T("Saving XML file: %s\n"), szFilename );

	ATLASSERT( m_pXMLDoc );

	if ( SUCCEEDED( m_pXMLDoc->save( CComVariant( szFilename ) ) ) )
		return true;
	else
	{
		_tprintf( _T("ERROR: Can't save .xml-file: %s\n"), szFilename );
		return false;
	}
}

bool CXMLLoader::SavePO(LPCTSTR szFilename) const
{
	CString	sLanguage;		// PO language (full)
	CString	sAuthor;		// PO author

	_tprintf( _T("Saving PO file: %s\n"), szFilename );

	CItem* item;
	if ( m_RefIndex.Lookup(  _T("manifest-name"), item ) )
	{
		sLanguage = item->sTranslated.IsEmpty() ? item->sID : item->sTranslated;
		sLanguage = sLanguage.SpanExcluding( _T(" (") );
	}
	if ( m_RefIndex.Lookup(  _T("manifest-author"), item ) )
	{
		sAuthor = item->sTranslated.IsEmpty() ? item->sID : item->sTranslated;
	}

	CString sOutput;
	sOutput.Format(
		_T("msgid \"\"\n")
		_T("msgstr \"\"\n")
		_T("\"Project-Id-Version: Shareaza\\n\"\n")
		_T("\"Report-Msgid-Bugs-To: shareaza <shareaza@cherubicsoft.com>\\n\"\n")
		_T("\"POT-Creation-Date: %s\\n\"\n")
		_T("\"PO-Revision-Date: \\n\"\n")
		_T("\"Last-Translator: %s\\n\"\n")
		_T("\"Language-Team: Shareaza Development Team\\n\"\n")
		_T("\"MIME-Version: 1.0\\n\"\n")
		_T("\"Content-Type: text/plain; charset=utf-8\\n\"\n")
		_T("\"Content-Transfer-Encoding: 8bit\\n\"\n")
		_T("\"X-Poedit-SourceCharset: utf-8\\n\"\n")
		_T("\"X-Poedit-Language: %s\\n\"\n"),
		CTime::GetCurrentTime().FormatGmt( _T("%Y-%m-%d %H:%M+0000") ),
		sAuthor, sLanguage );

	for( POSITION pos = m_Items.GetHeadPosition(); pos; )
	{
		const CItem& it = m_Items.GetNext( pos );

		ATLASSERT( it.sID != "" );

		CString sSafeID( MakeSafe( it.sID ) );
		CString sSafeTranslated( MakeSafe( it.sTranslated ) );

		sOutput += "\n#: ";
		sOutput += it.sRef;
		sOutput += "\nmsgid \"";
		sOutput += sSafeID;
		sOutput += "\"\nmsgstr \"";
		sOutput += sSafeTranslated;
		sOutput += "\"\n";
	}

	CAtlFile oFile;
	if ( SUCCEEDED( oFile.Create( szFilename, GENERIC_WRITE, 0, CREATE_ALWAYS ) ) )
	{
		CStringA sBuf( UTF8Encode( sOutput ) );

		if ( SUCCEEDED( oFile.Write( (LPCSTR)sBuf, sBuf.GetLength() ) ) )
		{
			oFile.Close();

#ifdef _DEBUG
			// Test it
			_tprintf( _T("Testing...\n") );
			CXMLLoader oTest;
			ATLASSERT( oTest.LoadPO( szFilename ) );
			POSITION pos1 = m_Items.GetHeadPosition();
			POSITION pos2 = oTest.m_Items.GetHeadPosition();
			while ( pos1 && pos2 )
			{
				const CItem& item1 = m_Items.GetNext( pos1 );
				const CItem& item2 = oTest.m_Items.GetNext( pos2 );
				ATLASSERT( item1.sRef == item2.sRef );
				ATLASSERT( item1.sID == item2.sID );
				ATLASSERT( item1.sTranslated == item2.sTranslated );
			}
			while ( pos1 )
			{
				const CItem& item1 = m_Items.GetNext( pos1 );
				ATLTRACE( "Unsaved item: \"%s\" \"%s\" \"%s\"\n",
					(LPCSTR)CT2A( item1.sRef ), (LPCSTR)CT2A( item1.sID ),
					(LPCSTR)CT2A( item1.sTranslated ) );
			}
			while ( pos2 )
			{
				const CItem& item2 = m_Items.GetNext( pos2 );
				ATLTRACE( "Extra item: \"%s\" \"%s\" \"%s\"\n",
					(LPCSTR)CT2A( item2.sRef ), (LPCSTR)CT2A( item2.sID ),
					(LPCSTR)CT2A( item2.sTranslated ) );
			}
			ATLASSERT( m_Items.GetCount() == oTest.m_Items.GetCount() );
#endif
			return true;
		}
		else
			_tprintf( _T("ERROR: Can't save file: %s\n"), szFilename );
	}
	else
		_tprintf( _T("ERROR: Can't create file: %s\n"), szFilename );

	return false;
}

size_t CXMLLoader::GetUniqueCount() const
{
	return m_Items.GetCount();
}

size_t CXMLLoader::GetTotalCount() const
{
	return m_IDIndex.GetCount();
}

CStringA CXMLLoader::UTF8Encode(LPCWSTR szInput, int nInput)
{
	int nUTF8 = WideCharToMultiByte( CP_UTF8, 0, szInput, nInput, NULL, 0, NULL, NULL );
	CStringA sUTF8;
	if ( nUTF8 > 0 )
	{
		WideCharToMultiByte( CP_UTF8, 0, szInput, nInput, sUTF8.GetBuffer( nUTF8 ), nUTF8, NULL, NULL );
		sUTF8.ReleaseBuffer( nUTF8 - ( ( nInput == -1 ) ? 1 : 0 ) );
	}
	return sUTF8;
}

CStringW CXMLLoader::UTF8Decode(LPCSTR szInput, int nInput)
{
	int nWide = MultiByteToWideChar( CP_UTF8, 0, szInput, nInput, NULL, 0 );
	CStringW sWide;
	if ( nWide > 0 )
	{
		MultiByteToWideChar( CP_UTF8, 0, szInput, nInput, sWide.GetBuffer( nWide ), nWide );
		sWide.ReleaseBuffer( nWide - ( ( nInput == -1 ) ? 1 : 0 ) );
	}
	return sWide;
}

CString CXMLLoader::MakeSafe(const CString& str)
{
	CString tmp;
	LPTSTR dst = tmp.GetBuffer( str.GetLength() * 2 + 1 );
	for ( LPCTSTR src = str; *src; src++ )
	{
		switch ( *src )
		{
		case _T('\r'):
			*dst++ = _T('\\');
			*dst++ = _T('r');
			break;
		case _T('\n'):
			*dst++ = _T('\\');
			*dst++ = _T('n');
			break;
		case _T('\t'):
			*dst++ = _T('\\');
			*dst++ = _T('t');
			break;
		case _T('\\'):
			*dst++ = _T('\\');
			*dst++ = _T('\\');
			break;
		case _T('\"'):
			*dst++ = _T('\\');
			*dst++ = _T('\"');
			break;
		default:
			*dst++ = *src;
		}
	}
	*dst = 0;
	tmp.ReleaseBuffer();
	return tmp;
}

CString CXMLLoader::UnMakeSafe(const CString& str)
{
	CString tmp;
	LPTSTR dst = tmp.GetBuffer( str.GetLength() + 1 );
	for ( LPCTSTR src = str; *src; src++ )
	{
		if ( *src == _T('\\') )
		{
			switch ( *(src + 1) )
			{
			case _T('\\'):
				*dst++ = _T('\\');
				src++;
				break;
			case _T('r'):
				*dst++ = _T('\r');
				src++;
				break;
			case _T('n'):
				*dst++ = _T('\n');
				src++;
				break;
			case _T('t'):
				*dst++ = _T('\t');
				src++;
				break;
			case _T('\"'):
				*dst++ = _T('\"');
				src++;
				break;
			default:
				*dst++ = *src;
			}
		}
		else
			*dst++ = *src;
	}
	*dst = 0;
	tmp.ReleaseBuffer();
	return tmp;
}

bool CXMLLoader::IsEqual(const CString& _left, const CString& _right)
{
	LPCTSTR l = _left;
	LPCTSTR r = _right;
	for ( ; *l || *r; ++l, ++r )
	{
		if ( *l == _T('_') )
		{
			--r;
			continue;
		}
		if ( *r == _T('_') )
		{
			--l;
			continue;
		}
		if ( *l != *r )
			return false;
	}
	return ! *l && ! *r;
}

bool CXMLLoader::Load(LPCWSTR szParentName, LPCWSTR szRefName, LPCWSTR szTextName,
	IXMLDOMElement* pXMLElement, bool bKeepUnderscores, bool bSubstitute)
{
	CString sID;
	if ( szTextName )
	{
		// Attribute
		CComVariant vText;
		if ( S_OK != pXMLElement->getAttribute( CComBSTR( szTextName ), &vText ) ||
			vText.vt != VT_BSTR )
			// Skip empty tag
			return true;
		sID = (LPCWSTR)vText.bstrVal;
	}
	else
	{
		// Inner text
		CComBSTR bstrText;
		if ( S_OK != pXMLElement->get_text( &bstrText ) )
			// Skip empty tag
			return true;
		sID = bstrText;
	}

	CString sRef;
	if ( szRefName )
	{
		CComVariant vRef;
		if ( S_OK != pXMLElement->getAttribute( CComBSTR( szRefName ), &vRef ) ||
			vRef.vt != VT_BSTR )
		{
			_tprintf( _T("ERROR: Missed required XML attribute \"%s\" at \"%s\"\n"), szRefName, szParentName );
			return false;
		}
		sRef = (LPCWSTR)vRef.bstrVal;
		sRef.Trim();
		sRef.Remove( _T(' ') );
		sRef.Remove( _T('\t') );
		sRef.Remove( _T('\r') );
		sRef.Remove( _T('\n') );
	}

	CString sRefFull( szParentName );
	if ( ! sRef.IsEmpty() )
		sRefFull = sRefFull + _T(";") + sRef;
	sRefFull.MakeLower();

	if ( m_pXMLTranslator )
	{
		// On-line translation
		CItem* translated = NULL;
		if ( m_pXMLTranslator->m_RefIndex.Lookup( sRefFull, translated ) || m_pXMLTranslator->m_IDIndex.Lookup( sID, translated ) )
		{
			if ( translated->sTranslated.IsEmpty() || translated->bFuzzy || translated->bError )
			{
				_tprintf( _T("WARNING: %s message %s=\"%s\" %s=\"%s\" -> \"%s\"\n"),
					( translated->sTranslated.IsEmpty() ? _T("Untranslated") : ( translated->bFuzzy ? _T("Fuzzy") : _T("Ill-formated") ) ),
					( szRefName ? szRefName : _T("?") ),
					(LPCTSTR)sRefFull,
					( szTextName ? szTextName : _T("inner_text") ),
					(LPCTSTR)translated->sID,
					(LPCTSTR)translated->sTranslated );

				if ( szTextName )
				{
					pXMLElement->setAttribute( CComBSTR( szTextName ), CComVariant( translated->sID ) );
				}
				else
				{
					pXMLElement->put_text( CComBSTR( translated->sID ) );
				}
			}
			else
			{
				if ( szTextName )
				{
					pXMLElement->setAttribute( CComBSTR( szTextName ), CComVariant( translated->sTranslated ) );
				}
				else
				{
					pXMLElement->put_text( CComBSTR( translated->sTranslated ) );
				}
			}
		}
#ifdef _DEBUG
		else
			_tprintf( _T("WARNING: Missed message \"%s\"=\"%s\"\n"), ( szRefName ? szRefName : _T("?") ), sRefFull );
#endif // _DEBUG
		return true;
	}
	else
	{
		// #:  file_name:line_number
		// msgid ""
		// msgstr ""

		if ( sID.IsEmpty() )
		{
			if ( bSubstitute && ! sRef.IsEmpty() )
				sID = sRef;
			else
				return true;
		}

		if( sRefFull.Find( _T(" \t\r\n") ) != -1 )
		{
			_tprintf( _T("ERROR: Found invalid PO reference: \"%s\"\n"), (LPCTSTR)sRefFull );
			return false;
		}

		const CItemMap::CPair* pair = m_RefIndex.Lookup( sRefFull );
		if ( pair )
			// Skip duplicate
			return true;

		for( POSITION pos = m_Items.GetHeadPosition(); pos; )
		{
			CItem& item = m_Items.GetNext( pos );

			if ( bKeepUnderscores && item.bKeepUnderscores )
			{
				if ( IsEqual( item.sID, sID ) )
				{
					// Already defined string
					item.sRef = item.sRef + _T(" ") + sRefFull;
					ATLVERIFY( m_IDIndex.SetAt( item.sID, &item ) );
					ATLVERIFY( m_IDIndex.SetAt( sID, &item ) );
					ATLVERIFY( m_RefIndex.SetAt( sRefFull, &item ) );
					return true;
				}
			}
			else
			{
				if ( item.sID == sID )
				{
					// Already defined string
					item.sRef = item.sRef + _T(" ") + sRefFull;
					ATLVERIFY( m_IDIndex.SetAt( sID, &item ) );
					ATLVERIFY( m_RefIndex.SetAt( sRefFull, &item ) );
					return true;
				}
			}
		}

		//if ( ! bKeepUnderscores )
		//	sID.Remove( _T('_') );

		// New string
		Add( CItem( sRefFull, sID, bKeepUnderscores ) );

		return true;
	}
}

bool CXMLLoader::LoadManifest(IXMLDOMElement* pXMLElement)
{
	//<manifest	name="English"
	//	author=""
	//	updatedBy=""
	//	description="Shareaza Default English Skin File"
	//	link="http://shareaza.sourceforge.net/"
	//	email=""
	//	version="2.4.0.2"
	//	type="Language"
	//	language="en"
	//	prompt="Click here to select English as your natural language."
	//	dir="ltr"/>

	if ( ! Load( _T("manifest-name"), NULL, _T("name"), pXMLElement ) )
		return false;
	if ( ! Load( _T("manifest-author"), NULL, _T("author"), pXMLElement ) )
		return false;
	if ( ! Load( _T("manifest-updatedBy"), NULL, _T("updatedBy"), pXMLElement ) )
		return false;
	if ( ! Load( _T("manifest-description"), NULL, _T("description"), pXMLElement ) )
		return false;
	if ( ! Load( _T("manifest-link"), NULL, _T("link"), pXMLElement ) )
		return false;
	if ( ! Load( _T("manifest-email"), NULL, _T("email"), pXMLElement ) )
		return false;
	if ( ! Load( _T("manifest-version"), NULL, _T("version"), pXMLElement ) )
		return false;
	if ( ! Load( _T("manifest-language"), NULL, _T("language"), pXMLElement ) )
		return false;
	if ( ! Load( _T("manifest-prompt"), NULL, _T("prompt"), pXMLElement ) )
		return false;
	if ( ! Load( _T("manifest-dir"), NULL, _T("dir"), pXMLElement ) )
		return false;

	return true;
}

bool CXMLLoader::LoadToolbar(IXMLDOMElement* pXMLRoot)
{
	int i = 1;
	CString tmp;

	CComVariant vName;
	pXMLRoot->getAttribute( CComBSTR( _T("name") ), &vName );

	XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

	// <button id="" text="" tip=""/>
	// <control id="" text=""/>
	// <label text="" tip=""/>
	// <separator/>
	// <rightalign/>

	CComQIPtr< IXMLDOMElement > pXMLElement( pXMLNode );

	CComBSTR name;
	if ( pXMLNode->get_nodeName( &name ) == S_OK )
	{
		name.ToLower();
		if ( name == _T("#comment") )
		{
			if ( m_bRemoveComments )
			{
				CComPtr< IXMLDOMNode > pChild;
				pXMLRoot->removeChild( pXMLNode, &pChild );
			}
			continue;
		}
		else if ( name == _T("button") )
		{
			if ( ! Load( _T("button"), _T("id"), _T("text"), pXMLElement ) )
				return false;
			if ( ! Load( _T("tip"), _T("id"), _T("tip"), pXMLElement ) )
				return false;
		}
		else if ( name == _T("control") )
		{
			if ( ! Load( _T("control"), _T("id"), _T("text"), pXMLElement ) )
				return false;
		}
		else if ( name == _T("separator") )
		{
			continue;
		}
		else if ( name == _T("rightalign") )
		{
			continue;
		}
		else if ( name == _T("label") )
		{
			tmp.Format( _T("%s;label-text;%d"), (LPCWSTR)vName.bstrVal, i );
			if ( ! Load( tmp, NULL, _T("text"), pXMLElement ) )
				return false;
			tmp.Format( _T("%s;label-tip;%d"), (LPCWSTR)vName.bstrVal, i );
			if ( ! Load( tmp, NULL, _T("tip"), pXMLElement ) )
				return false;
			++i;
		}
		else
		{
			_tprintf( _T("ERROR: Unexpected tag inside <toolbar name=\"%s\">: %s\n"), (LPCWSTR)vName.bstrVal, (LPCWSTR)name );
			return false;
		}
	}

	XML_FOR_EACH_END()

	return true;
}

bool CXMLLoader::LoadToolbars(IXMLDOMElement* pXMLRoot)
{
	XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

	// <toolbar name=""></toolbar>

	CComQIPtr< IXMLDOMElement > pXMLElement( pXMLNode );

	CComBSTR name;
	if ( pXMLNode->get_nodeName( &name ) == S_OK )
	{
		name.ToLower();
		if ( name == _T("#comment") )
		{
			if ( m_bRemoveComments )
			{
				CComPtr< IXMLDOMNode > pChild;
				pXMLRoot->removeChild( pXMLNode, &pChild );
			}
			continue;
		}
		else if ( name == _T("toolbar") )
		{
			if ( ! LoadToolbar( pXMLElement ) )
				return false;
		}
		else
		{
			_tprintf( _T("ERROR: Unexpected tag inside <toolbars>: %s\n"), (LPCWSTR)name );
			return false;
		}
	}

	XML_FOR_EACH_END()

	return true;
}

bool CXMLLoader::LoadMenu(IXMLDOMElement* pXMLRoot, LPCTSTR szParentName, int& i)
{
	CComVariant vName;
	pXMLRoot->getAttribute( CComBSTR( _T("name") ), &vName );
	if ( vName.vt == VT_NULL )
		vName = szParentName;

	CString tmp;

	XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

	// <menu text=""></menu>
	// <item id="" text=""/>
	// <separator/>

	CComQIPtr< IXMLDOMElement > pXMLElement( pXMLNode );

	CComBSTR name;
	if ( pXMLNode->get_nodeName( &name ) == S_OK )
	{
		name.ToLower();
		if ( name == _T("#comment") )
		{
			// Comments
			if ( m_bRemoveComments )
			{
				CComPtr< IXMLDOMNode > pChild;
				pXMLRoot->removeChild( pXMLNode, &pChild );
			}
			continue;
		}
		else if ( name == _T("menu") )
		{
			// Sub-menu
			tmp.Format( _T("%s;%d"), (LPCWSTR)vName.bstrVal, i );
			if ( ! Load( tmp, NULL, _T("text"), pXMLElement, true ) )
				return false;
			++i;
			if ( ! LoadMenu( pXMLElement, tmp, i ) )
				return false;
		}
		else if ( name == _T("item") )
		{
			// Menu item
			if ( ! Load( CString( (LPCWSTR)vName.bstrVal ) + _T(";item"), _T("id"), _T("text"), pXMLElement, true  ) )
				return false;
		}
		else if ( name == _T("separator") )
		{
			// Menu separator
			continue;
		}
		else
		{
			_tprintf( _T("ERROR: Unexpected tag inside <menu name=\"%s\">: %s\n"), (LPCWSTR)vName.bstrVal, (LPCWSTR)name );
			return false;
		}
	}

	XML_FOR_EACH_END()

	return true;
}

bool CXMLLoader::LoadMenus(IXMLDOMElement* pXMLRoot)
{
	int i = 1;

	XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

	// <menu name=""></menu>

	CComQIPtr< IXMLDOMElement > pXMLElement( pXMLNode );

	CComBSTR name;
	if ( pXMLNode->get_nodeName( &name ) == S_OK )
	{
		name.ToLower();
		if ( name == _T("#comment") )
		{
			if ( m_bRemoveComments )
			{
				CComPtr< IXMLDOMNode > pChild;
				pXMLRoot->removeChild( pXMLNode, &pChild );
			}
			continue;
		}
		else if ( name == _T("menu") )
		{
			if ( ! LoadMenu( pXMLElement, _T(""), i ) )
				return false;
		}
		else
		{
			_tprintf( _T("ERROR: Unexpected tag inside <menus>: %s\n"), (LPCWSTR)name );
			return false;
		}
	}

	XML_FOR_EACH_END()

	return true;
}

bool CXMLLoader::LoadDocument(IXMLDOMElement* pXMLRoot, LPCTSTR szParentName, int& i_text, int& i_heading)
{
	CComVariant vName;
	pXMLRoot->getAttribute( CComBSTR( _T("name") ), &vName );
	if ( vName.vt == VT_NULL )
		vName = szParentName;
	
	CString tmp;

	XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

	// <styles></styles>
	// <anchor/>
	// <newline/>
	// <icon></icon>
	// <gap/>
	// <text></text>
	// <link target=""></link>
	// <group></group>
	// <para></para>

	CComQIPtr< IXMLDOMElement > pXMLElement( pXMLNode );

	CComBSTR name;
	if ( pXMLNode->get_nodeName( &name ) == S_OK )
	{
		name.ToLower();
		if ( name == _T("#comment") )
		{
			if ( m_bRemoveComments )
			{
				CComPtr< IXMLDOMNode > pChild;
				pXMLRoot->removeChild( pXMLNode, &pChild );
			}
			continue;
		}
		else if ( name == _T("styles") || name == _T("anchor") ||
			name == _T("newline") || name == _T("icon") || name == _T("gap") )
		{
			continue;
		}
		else if ( name == _T("group") || name == _T("para") )
		{
			if ( ! LoadDocument( pXMLElement, (LPCWSTR)vName.bstrVal, i_text, i_heading ) )
				return false;
		}
		else if ( name == _T("text") )
		{
			tmp.Format( _T("%s;text;%d"), (LPCWSTR)vName.bstrVal, i_text );
			if ( ! Load( tmp, NULL, NULL, pXMLElement ) )
				return false;
			++i_text;
		}
		else if ( name == _T("heading") )
		{
			tmp.Format( _T("%s;heading;%d"), (LPCWSTR)vName.bstrVal, i_heading * 1000 );
			if ( ! Load( tmp, NULL, NULL, pXMLElement ) )
				return false;
			i_text = i_heading * 1000 + 1;
			++i_heading;
		}
		else if ( name == _T("link") )
		{
			tmp.Format( _T("%s;link;%d"), (LPCWSTR)vName.bstrVal, i_text );
			CComVariant vTarget;
			pXMLElement->getAttribute( CComBSTR( _T("id") ), &vTarget );
			if ( ! Load( tmp, ( vTarget.vt == VT_NULL ) ? NULL : _T("id"),
				NULL, pXMLElement ) )
				return false;
			++i_text;
		}
		else
		{
			_tprintf( _T("ERROR: Unexpected tag inside <document name=\"%s\">: %s.\n"), (LPCWSTR)vName.bstrVal, (LPCWSTR)name );
			return false;
		}
	}

	XML_FOR_EACH_END()

	return true;
}

bool CXMLLoader::LoadDocuments(IXMLDOMElement* pXMLRoot)
{
	XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

	// <document name="" title=""></document>

	CComQIPtr< IXMLDOMElement > pXMLElement( pXMLNode );

	CComBSTR name;
	if ( pXMLNode->get_nodeName( &name ) == S_OK )
	{
		name.ToLower();
		if ( name == _T("#comment") )
		{
			if ( m_bRemoveComments )
			{
				CComPtr< IXMLDOMNode > pChild;
				pXMLRoot->removeChild( pXMLNode, &pChild );
			}
			continue;
		}
		else if ( name == _T("document") )
		{
			if ( ! Load( _T("document"), _T("name"), _T("title"), pXMLElement ) )
				return false;
	
			int i_text = 1, i_heading = 1;
			if ( ! LoadDocument( pXMLElement, _T("document"), i_text, i_heading ) )
				return false;
		}
		else
		{
			_tprintf( _T("ERROR: Unexpected tag inside <documents>: %s\n"), (LPCWSTR)name );
			return false;
		}
	}

	XML_FOR_EACH_END()

	return true;
}

bool CXMLLoader::LoadCommandTips(IXMLDOMElement* pXMLRoot)
{
	XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

	// <tip id="" message="" tip=""/>

	CComQIPtr< IXMLDOMElement > pXMLElement( pXMLNode );
	
	CComBSTR name;
	if ( pXMLNode->get_nodeName( &name ) == S_OK )
	{
		name.ToLower();
		if ( name == _T("#comment") )
		{
			if ( m_bRemoveComments )
			{
				CComPtr< IXMLDOMNode > pChild;
				pXMLRoot->removeChild( pXMLNode, &pChild );
			}
			continue;
		}
		else if ( name == _T("tip") )
		{
			if ( ! Load( _T("message"), _T("id"), _T("message"), pXMLElement ) )
				return false;
			if ( ! Load( _T("tip"), _T("id"), _T("tip"), pXMLElement ) )
				return false;
		}
		else
		{
			_tprintf( _T("ERROR: Unexpected tag inside <strings>: %s\n"), (LPCWSTR)name );
			return false;
		}
	}

	XML_FOR_EACH_END()

	return true;
}

bool CXMLLoader::LoadControlTips(IXMLDOMElement* pXMLRoot)
{
	XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

	// <tip id="" message=""/>

	CComQIPtr< IXMLDOMElement > pXMLElement( pXMLNode );

	CComBSTR name;
	if ( pXMLNode->get_nodeName( &name ) == S_OK )
	{
		name.ToLower();
		if ( name == _T("#comment") )
		{
			if ( m_bRemoveComments )
			{
				CComPtr< IXMLDOMNode > pChild;
				pXMLRoot->removeChild( pXMLNode, &pChild );
			}
			continue;
		}
		else if ( name == _T("tip") )
		{
			if ( ! Load( _T("tip"), _T("id"), _T("message"), pXMLElement ) )
				return false;
		}
		else
		{
			_tprintf( _T("ERROR: Unexpected tag inside <strings>: %s\n"), (LPCWSTR)name );
			return false;
		}
	}

	XML_FOR_EACH_END()

	return true;
}

bool CXMLLoader::LoadStrings(IXMLDOMElement* pXMLRoot)
{
	XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

	// <string id="" value=""/>

	CComQIPtr< IXMLDOMElement > pXMLElement( pXMLNode );

	CComBSTR name;
	if ( pXMLNode->get_nodeName( &name ) == S_OK )
	{
		name.ToLower();
		if ( name == _T("#comment") )
		{
			if ( m_bRemoveComments )
			{
				CComPtr< IXMLDOMNode > pChild;
				pXMLRoot->removeChild( pXMLNode, &pChild );
			}
			continue;
		}
		else if ( name == _T("string") )
		{
			if ( ! Load( _T("string"), _T("id"), _T("value"), pXMLElement ) )
				return false;
		}
		else
		{
			_tprintf( _T("ERROR: Unexpected tag inside <strings>: %s\n"), (LPCWSTR)name );
			return false;
		}
	}

	XML_FOR_EACH_END()

	return true;
}

bool CXMLLoader::LoadDialog(IXMLDOMElement* pXMLRoot)
{
	int i = 1;
	CString tmp;

	CComVariant vName;
	pXMLRoot->getAttribute( CComBSTR( _T("name") ), &vName );

	XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

	// <control name="" caption=""/>

	CComQIPtr< IXMLDOMElement > pXMLElement( pXMLNode );
	CComBSTR name;
	if ( pXMLNode->get_nodeName( &name ) == S_OK )
	{
		name.ToLower();
		if ( name == _T("#comment") )
		{
			if ( m_bRemoveComments )
			{
				CComPtr< IXMLDOMNode > pChild;
				pXMLRoot->removeChild( pXMLNode, &pChild );
			}
			continue;
		}
		else if ( name == _T("control") )
		{
			tmp.Format( _T("%s;control;%d"), (LPCWSTR)vName.bstrVal, i );
			if ( ! Load( tmp, NULL, _T("caption"), pXMLElement ) )
				return false;
			++i;
		}
		else
		{
			_tprintf( _T("ERROR: Unexpected tag inside <dialog name=\"%s\">: %s\n"), (LPCWSTR)vName.bstrVal, (LPCWSTR)name );
			return false;
		}
	}

	XML_FOR_EACH_END()

	return true;
}

bool CXMLLoader::LoadDialogs(IXMLDOMElement* pXMLRoot)
{
	XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

	// <dialog name="" cookie="" caption=""></dialog>

	CComQIPtr< IXMLDOMElement > pXMLElement( pXMLNode );

	CComBSTR name;
	if ( pXMLNode->get_nodeName( &name ) == S_OK )
	{
		name.ToLower();
		if ( name == _T("#comment") )
		{
			if ( m_bRemoveComments )
			{
				CComPtr< IXMLDOMNode > pChild;
				pXMLRoot->removeChild( pXMLNode, &pChild );
			}
			continue;
		}
		else if ( name == _T("dialog") )
		{
			if ( ! Load( _T("dialog"), _T("name"), _T("caption"), pXMLElement ) )
				return false;

			if ( ! LoadDialog( pXMLElement ) )
				return false;
		}
		else
		{
			_tprintf( _T("ERROR: Unexpected tag inside <dialogs>: %s\n"), (LPCWSTR)name );
			return false;
		}
	}

	XML_FOR_EACH_END()

	return true;
}

bool CXMLLoader::LoadListColumn(IXMLDOMElement* pXMLRoot)
{
	XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

	// <column from="" to=""/>

	CComQIPtr< IXMLDOMElement > pXMLElement( pXMLNode );

	CComBSTR name;
	if ( pXMLNode->get_nodeName( &name ) == S_OK )
	{
		name.ToLower();
		if ( name == _T("#comment") )
		{
			if ( m_bRemoveComments )
			{
				CComPtr< IXMLDOMNode > pChild;
				pXMLRoot->removeChild( pXMLNode, &pChild );
			}
			continue;
		}
		else if ( name == _T("column") )
		{
			if ( ! Load( _T("column"), _T("from"), _T("to"), pXMLElement, false, true ) )
				return false;
		}
		else
		{
			_tprintf( _T("ERROR: Unexpected tag inside <list>: %s\n"), (LPCWSTR)name );
			return false;
		}
	}

	XML_FOR_EACH_END()

	return true;
}

bool CXMLLoader::LoadListColumns(IXMLDOMElement* pXMLRoot)
{
	XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

	// <list name=""></list>

	CComQIPtr< IXMLDOMElement > pXMLElement( pXMLNode );

	CComBSTR name;
	if ( pXMLNode->get_nodeName( &name ) == S_OK )
	{
		name.ToLower();
		if ( name == _T("#comment") )
		{
			if ( m_bRemoveComments )
			{
				CComPtr< IXMLDOMNode > pChild;
				pXMLRoot->removeChild( pXMLNode, &pChild );
			}
			continue;
		}
		else if ( name == _T("list") )
		{
			if ( ! LoadListColumn( pXMLElement ) )
				return false;
		}
		else
		{
			_tprintf( _T("ERROR: Unexpected tag inside <listcolumns>: %s\n"), (LPCWSTR)name );
			return false;
		}
	}

	XML_FOR_EACH_END()

	return true;
}

bool CXMLLoader::LoadFonts(IXMLDOMElement* pXMLRoot)
{
	XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

	// <font name="" face=""/>

	CComQIPtr< IXMLDOMElement > pXMLElement( pXMLNode );

	CComBSTR name;
	if ( pXMLNode->get_nodeName( &name ) == S_OK )
	{
		name.ToLower();
		if ( name == _T("#comment") )
		{
			if ( m_bRemoveComments )
			{
				CComPtr< IXMLDOMNode > pChild;
				pXMLRoot->removeChild( pXMLNode, &pChild );
			}
			continue;
		}
		else if ( name == _T("font") )
		{
			if ( ! Load( _T("font"), _T("name"), _T("face"), pXMLElement ) )
				return false;
		}
		else
		{
			_tprintf( _T("ERROR: Unexpected tag inside <fonts>: %s\n"), (LPCWSTR)name );
			return false;
		}
	}

	XML_FOR_EACH_END()

	return true;
}

bool CXMLLoader::LoadSkin(IXMLDOMElement* pXMLRoot)
{
	XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

	CComQIPtr< IXMLDOMElement > pXMLElement( pXMLNode );

	CComBSTR name;
	if ( pXMLNode->get_nodeName( &name ) == S_OK )
	{
		name.ToLower();
		if ( name == _T("#comment") )
		{
			if ( m_bRemoveComments )
			{
				CComPtr< IXMLDOMNode > pChild;
				pXMLRoot->removeChild( pXMLNode, &pChild );
			}
			continue;
		}
		else if ( name == _T("manifest") )
		{
			if ( ! LoadManifest( pXMLElement ) )
				return false;
		}
		else if ( name == _T("toolbars") )
		{
			if ( ! LoadToolbars( pXMLElement ) )
				return false;
		}
		else if ( name == _T("menus") )
		{
			if ( ! LoadMenus( pXMLElement ) )
				return false;
		}
		else if ( name == _T("documents") )
		{
			if ( ! LoadDocuments( pXMLElement ) )
				return false;
		}
		else if ( name == _T("commandtips") )
		{
			if ( ! LoadCommandTips( pXMLElement ) )
				return false;
		}
		else if ( name == _T("controltips") )
		{
			if ( ! LoadControlTips( pXMLElement ) )
				return false;
		}
		else if ( name == _T("strings") )
		{
			if ( ! LoadStrings( pXMLElement ) )
				return false;
		}
		else if ( name == _T("dialogs") )
		{
			if ( ! LoadDialogs( pXMLElement ) )
				return false;
		}
		else if ( name == _T("listcolumns") )
		{
			if ( ! LoadListColumns( pXMLElement ) )
				return false;
		}
		else if ( name == _T("fonts") )
		{
			if ( ! LoadFonts( pXMLElement ) )
				return false;
		}
		else
		{
			_tprintf( _T("ERROR: Unexpected tag inside <skin>: %s\n"), (LPCWSTR)name );
			return false;
		}
	}

	XML_FOR_EACH_END()

	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	_setmode( _fileno(stdout), _O_U16TEXT );
	_tprintf( _T("\xfeff") );

	if ( argc != 3 && argc != 4 )
	{
		LPCTSTR szExe = _tcsrchr( argv[ 0 ], _T('\\') ) + 1;
		_tprintf( _T("Usage:\n\n")
			_T("Generating .POT-file from template .XML-file:\n\n")
			_T("\t%s <template.xml> <template.pot>\n\n")
			_T("Generating translated .PO-file from template and translated .XML-file:\n\n")
			_T("\t%s <template.xml> <translated.xml> <translated.po>\n\n")
			_T("Generating translated .XML-file from template and .PO-file:\n\n")
			_T("\t%s <template.xml> <translated.po> <translated.xml>\n"),
			 szExe, szExe, szExe );
		return 1;
	}

	bool bResult = false;
	CString sTemplate = argv[ 1 ];
	CString sFile = argv[ 2 ];
	CString sFilenameOnly = PathFindFileName( argv[ 2 ] );
	sFilenameOnly = sFilenameOnly.Mid( 0,
		( PathFindExtension( sFilenameOnly ) - (LPCTSTR)sFilenameOnly ) );
	CString sOutput = argv[ argc - 1 ];
	sOutput.Replace( _T("#"), sFilenameOnly );
	LPCTSTR szExt = _tcsrchr( sOutput, _T('.') );
	bool bPoMode = ( argc == 4 ) && szExt && ( _tcsicmp( szExt, _T(".po") ) == 0 );
	bool bXMLMode = ( argc == 4 ) && szExt && ( _tcsicmp( szExt, _T(".xml") ) == 0 );

	if ( SUCCEEDED( CoInitialize( NULL ) ) )
	{
		{
			CXMLLoader oTemplate;
			CXMLLoader oTranslated;

			if ( bPoMode )
			{
				// Load translated text as XML
				bResult = oTranslated.LoadXML( sFile );
			}
			else if ( bXMLMode )
			{
				// Load translated text as PO
				bResult = oTranslated.LoadPO( sFile );
			}
			else
				bResult = true;

			if ( bResult )
			{
				// Load template
				if ( oTemplate.LoadXML( sTemplate, bXMLMode ? &oTranslated : NULL ) )
				{
					if ( bXMLMode )
					{
						// Generate .xml-file from template XML and translated .po-file
						bResult = oTemplate.SaveXML( sOutput );
					}
					else
					{
						// Generate .po-file from template XML
						if ( bPoMode )
							// ...  and translated XML
							oTemplate.Translate( oTranslated );

						bResult = oTemplate.SavePO( sOutput );
					}
				}
			}
		}
		CoUninitialize();
	}
	else
		_tprintf( _T("ERROR: Can't initialize COM\n") );

	return ( bResult ? 0 : 1 );
}
