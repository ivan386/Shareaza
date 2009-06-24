#include "stdafx.h"
#include "SkinTranslate.h"

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

	// Add to message ID index
	ATLVERIFY( m_IDIndex.SetAt( item.sID, &inserted ) );

	// Add to references index
	CString sRefLC( inserted.sRef );
	sRefLC.MakeLower();

	int curPos = 0;
	CString sTokenLC = sRefLC.Tokenize( _T(" "), curPos );
	while ( ! sTokenLC.IsEmpty() )
	{
		sTokenLC.Trim();
		if ( ! sTokenLC.IsEmpty() )
		{
			ATLVERIFY( m_RefIndex.SetAt( sTokenLC, &inserted ) );
		}
		sTokenLC = sRefLC.Tokenize( _T(" "), curPos );
	};

	return inserted;
}

bool CXMLLoader::LoadPO(LPCWSTR szFilename)
{
	_tprintf( _T("Loading PO file: '%s'\n"), szFilename );

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
						if ( mode != mode_start && mode != mode_msgstr )
						{
							_tprintf( _T("Error in line #%d: %s\n"), nLine, sOriginalLine );
							return false;
						}
						if ( sLine[ 1 ] == ':' )
						{
							// Ref
							if ( mode == mode_msgstr )
							{
								// Save previous string
								item.sTranslated = UTF8Decode( sString );
								UnMakeSafe( item.sTranslated );

								sString.Empty();

								// Save previous non-empty string
								if ( item.sRef != "" )
								{
									Add( item );
								}

								item.Clear();

								mode = mode_ref;
							}

							if ( ! sString.IsEmpty() )
								sString += " ";
							sString += sLine.Mid( 2 ).Trim();
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
								_tprintf( _T("Error in line #%d: %s\n"), nLine, sOriginalLine );
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
								_tprintf( _T("Error in line #%d: %s\n"), nLine, sOriginalLine );
								return false;
							}
							
							// Save previous string
							item.sID = UTF8Decode( sString );
							UnMakeSafe( item.sID );

							sString.Empty();

							sLine = sLine.Mid( 7, sLine.GetLength() - 7 );
							mode = mode_msgstr;
						}
						else
						{
							// Unknown string
							_tprintf( _T("Error in line #%d: %s\n"), nLine, sOriginalLine );
							return false;
						}

					case '\"':
						if( mode != mode_msgid && mode != mode_msgstr )
						{
							_tprintf( _T("Error in line #%d: %s\n"), nLine, sOriginalLine );
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
							_tprintf( _T("Error in line #%d: %s\n"), nLine, sOriginalLine );
							return false;
						}
						break;

					default:
						// Unknown string
						_tprintf( _T("Error in line #%d: %s\n"), nLine, sOriginalLine );
						return false;
					}

					// Get next line
					sLine = sFile.Tokenize( "\n", curPos );
					nLine++;
				} // while

				if ( mode != mode_msgstr )
				{
					// Unknown string
					_tprintf( _T("Error in line #%d: %s\n"), nLine, sOriginalLine );
					return false;
				}

				// Save last string
				item.sTranslated = UTF8Decode( sString );
				UnMakeSafe( item.sTranslated );
				if ( item.sRef != "" )
				{
					Add( item );
				}

				if ( ! m_Items.IsEmpty() )
					return true;

				_tprintf( _T("Error: PO file is empty\n") );
			}
			else
				_tprintf( _T("Error: Can't read PO file: %ls\n"), szFilename );
		}
		else
			_tprintf( _T("Error: Can't read PO file: %ls\n"), szFilename );
	}
	else
		_tprintf( _T("Error: Can't open PO file: %ls\n"), szFilename );

	return false;
}

bool CXMLLoader::LoadXML(LPCWSTR szFilename, const CXMLLoader* pTranslator)
{
	m_pXMLTranslator = pTranslator;

	_tprintf( _T("Loading XML file: '%s'\n"), szFilename );

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
							_tprintf( _T("Error: Can't find <skin> tag\n") );
					}
					else
						_tprintf( _T("Error: Can't find valid root element\n") );
				}
				else
					_tprintf( _T("Error: Can't find XML tags\n") );
			}
			else
				_tprintf( _T("Error: Can't open XML file: %ls\n"), szFilename );
		}
		else
			_tprintf( _T("Error: Can't put XML object in sync mode\n") );
	}
	else
		_tprintf( _T("Error: Can't create MS XML object\n") );

	m_pXMLTranslator = NULL;

	return false;
}

void CXMLLoader::Translate(const CXMLLoader& oTranslator)
{
	for ( POSITION pos1 = m_Items.GetHeadPosition(); pos1; )
	{
		CItem& item = m_Items.GetNext( pos1 );

		CString sRefLC( item.sRef );
		sRefLC.MakeLower();

		int curPos = 0;
		CString sTokenLC = sRefLC.Tokenize( _T(" "), curPos );
		while ( ! sTokenLC.IsEmpty() )
		{
			sTokenLC.Trim();
			if ( ! sTokenLC.IsEmpty() )
			{
				CItem* translated = NULL;
				if ( oTranslator.m_RefIndex.Lookup( sTokenLC, translated ) )
				{
					ATLASSERT( item.sTranslated.IsEmpty() );
					item.sTranslated = translated->sID;
					break;
				}
			}
			sTokenLC = sRefLC.Tokenize( _T(" "), curPos );
		};

		if ( item.sTranslated.IsEmpty() )
			_tprintf( _T("Untranslated message \"%s\": \"%s\"\n"), item.sRef, item.sID );
	}
}

bool CXMLLoader::SaveXML(LPCTSTR szFilename) const
{
	_tprintf( _T("Saving XML file: '%s'\n"), szFilename );

	ATLASSERT( m_pXMLDoc );

	CComBSTR pXML;
	m_pXMLDoc->get_xml( &pXML );

	CString sXML( pXML );
	sXML.Replace( _T("\x00a0"), _T("&#160;") ); // Non-break space

	CAtlFile oFile;
	if ( SUCCEEDED( oFile.Create( szFilename, GENERIC_WRITE, 0, CREATE_ALWAYS ) ) )
	{
		const BYTE Marker[3] = { 0xef, 0xbb, 0xbf };
		CStringA sOutput = UTF8Encode( sXML );
		return SUCCEEDED( oFile.Write( Marker, 3 ) ) &&
			SUCCEEDED( oFile.Write( (LPCSTR)sOutput, sOutput.GetLength() - 1 ) );
	}
	else
		_tprintf( _T("Error: Can't save .xml-file: %s\n"), szFilename );

	return false;
}

bool CXMLLoader::SavePO(LPCTSTR szFilename) const
{
	_tprintf( _T("Saving PO file: '%s'\n"), szFilename );

	CStringA sOutput;
	sOutput.Format(
		"msgid \"\"\n"
		"msgstr \"\"\n"
		"\"Project-Id-Version: Shareaza\\n\"\n"
		"\"Report-Msgid-Bugs-To: ryo-oh-ki <ryo-oh-ki@narod.ru>\\n\"\n"
		"\"POT-Creation-Date: %s\\n\"\n"
		"\"PO-Revision-Date: \\n\"\n"
		"\"Last-Translator: \\n\"\n"
		"\"Language-Team: Shareaza Development Team\\n\"\n"
		"\"MIME-Version: 1.0\\n\"\n"
		"\"Content-Type: text/plain; charset=utf-8\\n\"\n"
		"\"Content-Transfer-Encoding: 8bit\\n\"\n"
		"\"X-Poedit-Language: \\n\"\n"
		"\"X-Poedit-Country: \\n\"\n"
		"\"X-Poedit-SourceCharset: utf-8\\n\"\n",
		(LPCSTR)CT2A( CTime::GetCurrentTime().FormatGmt( _T("%Y-%m-%d %H:%M+0000") ) ) );

	for( POSITION pos = m_Items.GetHeadPosition(); pos; )
	{
		const CItem& item = m_Items.GetNext( pos );

		ATLASSERT( item.sID != "" );

		CString sSafeID( item.sID );
		MakeSafe( sSafeID );
		CString sSafeTranslated( item.sTranslated );
		MakeSafe( sSafeTranslated );

		CStringA sSafeUtf8ID( UTF8Encode( sSafeID ) );
		CStringA sSafeUtf8Translated( UTF8Encode( sSafeTranslated ) );

		sOutput += "\n#: ";
		sOutput += item.sRef;
		sOutput += "\nmsgid \"";
		sOutput += sSafeUtf8ID;
		sOutput += "\"\nmsgstr \"";
		sOutput += sSafeUtf8Translated;
		sOutput += "\"\n";
	}

	CAtlFile oFile;
	if ( SUCCEEDED( oFile.Create( szFilename, GENERIC_WRITE, 0, CREATE_ALWAYS ) ) )
	{
		if ( SUCCEEDED( oFile.Write( (LPCSTR)sOutput, sOutput.GetLength() ) ) )
		{
			oFile.Close();

#ifdef _DEBUG
			// Test it
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
			_tprintf( _T("Error: Can't save file: %s\n"), szFilename );
	}
	else
		_tprintf( _T("Error: Can't create file: %s\n"), szFilename );

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

CString& CXMLLoader::MakeSafe(CString& str)
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
	str = tmp;
	return str;
}

CString& CXMLLoader::UnMakeSafe(CString& str)
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
	str = tmp;
	return str;
}

bool CXMLLoader::Add(const CString& sRef, CString sID, bool bKeepUnderscores)
{
	// #:  file_name:line_number
	// msgid ""
	// msgstr ""

	if ( sID.IsEmpty() )
		return true;

	if( sRef.Find( _T(" \t\r\n") ) != -1 )
	{
		_tprintf( _T("Found invalid PO reference: \"%s\"\n"), sRef );
		return false;
	}

	CString sRefLC( sRef );
	sRefLC.MakeLower();

	const CItemMap::CPair* pair = m_RefIndex.Lookup( sRefLC );
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
				item.sRef = item.sRef + _T(" ") + sRef;
				ATLVERIFY( m_IDIndex.SetAt( item.sID, &item ) );
				ATLVERIFY( m_IDIndex.SetAt( sID, &item ) );
				ATLVERIFY( m_RefIndex.SetAt( sRefLC, &item ) );
				return true;
			}
		}
		else
		{
			if ( item.sID == sID )
			{
				// Already defined string
				item.sRef = item.sRef + _T(" ") + sRef;
				ATLVERIFY( m_IDIndex.SetAt( sID, &item ) );
				ATLVERIFY( m_RefIndex.SetAt( sRefLC, &item ) );
				return true;
			}
		}
	}

	if ( ! bKeepUnderscores )
		sID.Remove( _T('_') );

	// New string
	Add( CItem( sRef, sID, bKeepUnderscores ) );

	return true;
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

bool CXMLLoader::LoadManifest(IXMLDOMElement* /*pXMLElement*/)
{
	/*CComVariant vName;
	pXMLElement->getAttribute( CComBSTR( _T("name") ), &vName );
	CComVariant vAuthor;
	pXMLElement->getAttribute( CComBSTR( _T("author") ), &vAuthor );*/
	return true;
}

bool CXMLLoader::Load(LPCWSTR szParentName, LPCWSTR szRefName, LPCWSTR szTextName,
	IXMLDOMElement* pXMLElement, bool bKeepUnderscores)
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

	CString sRef( szParentName );
	if ( szRefName )
	{
		CComVariant vRef;
		if ( S_OK != pXMLElement->getAttribute( CComBSTR( szRefName ), &vRef ) ||
			vRef.vt != VT_BSTR )
		{
			_tprintf( _T("Missed required XML attribute \"%s\" at \"%s\"\n"),
				szRefName, szParentName );
			return false;
		}
		sRef += _T(';');
		sRef += (LPCWSTR)vRef.bstrVal;
	}

	CString sRefLC( sRef );
	sRefLC.MakeLower();

	if ( m_pXMLTranslator )
	{
		// On-line translation
		CItem* translated = NULL;
		if ( m_pXMLTranslator->m_RefIndex.Lookup( sRefLC, translated ) ||
			m_pXMLTranslator->m_IDIndex.Lookup( sID, translated ) )
		{
			if ( translated->sTranslated == _T("") )
			{
				_tprintf( _T("Untranslated message %s=\"%s\" %s=\"%s\"\n"),
					( szRefName ? szRefName : _T("") ), sRef,
					( szTextName ? szTextName : _T("inner_text") ),
					translated->sID );
				if ( szTextName )
				{
					pXMLElement->setAttribute(
						CComBSTR( szTextName ), CComVariant( translated->sID ) );
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
					pXMLElement->setAttribute(
						CComBSTR( szTextName ), CComVariant( translated->sTranslated ) );
				}
				else
				{
					pXMLElement->put_text( CComBSTR( translated->sTranslated ) );
				}
			}
		}
		else
			_tprintf( _T("Missed message %s=\"%s\"\n"),
				( szRefName ? szRefName : _T("") ), sRef );

		return true;
	}
	else
		return Add( sRef, sID, bKeepUnderscores );
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
			_tprintf( _T("Error: Unexpected tag inside <toolbar>: %ls\n"), (LPCWSTR)name );
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
			_tprintf( _T("Error: Unexpected tag inside <toolbars>: %ls\n"), (LPCWSTR)name );
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
			if ( ! Load( _T("menu"), _T("id"), _T("text"), pXMLElement, true  ) )
				return false;
		}
		else if ( name == _T("separator") )
		{
			// Menu separator
			continue;
		}
		else
		{
			_tprintf( _T("Error: Unexpected tag inside <menu>: %ls\n"), (LPCWSTR)name );
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
			_tprintf( _T("Error: Unexpected tag inside <menus>: %ls\n"), (LPCWSTR)name );
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
			_tprintf( _T("Error: Unexpected tag inside <document>: %ls\n"), (LPCWSTR)name );
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
			_tprintf( _T("Error: Unexpected tag inside <documents>: %ls\n"), (LPCWSTR)name );
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
			_tprintf( _T("Error: Unexpected tag inside <strings>: %ls\n"), (LPCWSTR)name );
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
			_tprintf( _T("Error: Unexpected tag inside <strings>: %ls\n"), (LPCWSTR)name );
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
			_tprintf( _T("Error: Unexpected tag inside <strings>: %ls\n"), (LPCWSTR)name );
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
			_tprintf( _T("Error: Unexpected tag inside <dialog>: %ls\n"), (LPCWSTR)name );
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
			_tprintf( _T("Error: Unexpected tag inside <dialogs>: %ls\n"), (LPCWSTR)name );
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
			if ( ! Load( _T("column"), _T("from"), _T("to"), pXMLElement ) )
				return false;
		}
		else
		{
			_tprintf( _T("Error: Unexpected tag inside <list>: %ls\n"), (LPCWSTR)name );
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
			_tprintf( _T("Error: Unexpected tag inside <listcolumns>: %ls\n"), (LPCWSTR)name );
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
		else
		{
			_tprintf( _T("Error: Unexpected tag inside <skin>: %ls\n"), (LPCWSTR)name );
			return false;
		}
	}

	XML_FOR_EACH_END()

	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
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
	LPCTSTR szTemplate = argv[ 1 ];
	LPCTSTR szOutput = argv[ argc - 1 ];
	LPCTSTR szExt = _tcsrchr( szOutput, _T('.') );
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
				bResult = oTranslated.LoadXML( argv[ 2 ] );
			}
			else if ( bXMLMode )
			{
				// Load translated text as PO
				bResult = oTranslated.LoadPO( argv[ 2 ] );
			}
			else
				bResult = true;

			if ( bResult )
			{
				// Load template
				if ( oTemplate.LoadXML( szTemplate, bXMLMode ? &oTranslated : NULL ) )
				{
					if ( bXMLMode )
					{
						// Generate .xml-file from template XML and translated .po-file
						bResult = oTemplate.SaveXML( szOutput );
					}
					else
					{
						// Generate .po-file from template XML
						if ( bPoMode )
							// ...  and translated XML
							oTemplate.Translate( oTranslated );

						bResult = oTemplate.SavePO( szOutput );
					}
				}
			}
		}
		CoUninitialize();
	}
	else
		_tprintf( _T("Error: Can't initialize COM\n") );

	return ( bResult ? 0 : 1 );
}
