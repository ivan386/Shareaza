#include "stdafx.h"


#define XML_FOR_EACH_BEGIN(x,y) \
{ \
	CComPtr< IXMLDOMNodeList > pList##x; \
	HRESULT hr = (x)->get_childNodes( &(pList##x) ); \
	long nLength##x = 0; \
	if ( hr == S_OK && ( hr = (pList##x)->get_length( &(nLength##x) ) ) == S_OK && (nLength##x) ) \
	{ \
		(pList##x)->reset(); \
		while ( hr == S_OK ) \
		{ \
			CComPtr< IXMLDOMNode > y; \
			hr = (pList##x)->nextNode( &(y) ); \
			if ( hr == S_OK ) \
			{

#define XML_FOR_EACH_END() \
			} \
		} \
	} \
}


class CXMLLoader
{
public:
	CXMLLoader() :
		 m_bRemoveComments( false ),
		 m_pXMLTranslator ( NULL )
	{
		m_Index.InitHashTable( 1021 );
	}

	void SetTranslator(const CXMLLoader* pTranslator)
	{
		m_pXMLTranslator = pTranslator;
	}

	bool LoadPO(LPCWSTR szFilename)
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
					CStringA sString;
					enum
					{
						mode_start, mode_ref, mode_msgid, mode_msgstr
					}
					mode = mode_start;

					int nLine = 0;
					int curPos = 0;
					CStringA sLine = sFile.Tokenize( "\n", curPos );
					while ( ! sLine.IsEmpty() )
					{
						CStringA sOriginalLine = sLine;
						sLine.Trim();
						nLine++;

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
									item.sTranslated = UTF8Decode( UnMakeSafe( sString ) );
									sString.Empty();

									// Save previous item
									if ( ! item.sRef.IsEmpty() )
									{
										m_Items.AddTail( item );
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
								item.sRef = UTF8Decode( sString.Trim() );
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
								item.sID = UTF8Decode( UnMakeSafe( sString ) );
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
						sLine = sFile.Tokenize( "\n", curPos );
					} // while

					if ( ! m_Items.IsEmpty() )
					{
						ReIndex();

						return true;
					}

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

	bool LoadXML(LPCWSTR szFilename)
	{
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
								return LoadSkin( pXMLRoot );
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

		return false;
	}

	// Re-index strings
	void ReIndex()
	{
		m_Index.RemoveAll();

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
					ATLASSERT( const_cast< CItemMap::CPair* >( m_Index.Lookup( sTokenLC) ) == NULL );
					m_Index.SetAt( sTokenLC, &item );
				}
				sTokenLC = sRefLC.Tokenize( _T(" "), curPos );
			};
		}
	}

	// Add translated strings from specified translator object
	void Translate(const CXMLLoader& oTranslator)
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
				CItem* translated = NULL;
				if ( oTranslator.m_Index.Lookup( sTokenLC, translated ) )
				{
					item.sTranslated = translated->sID;
					break;
				}
				sTokenLC = sRefLC.Tokenize( _T(" "), curPos );
			};
		}
	}

	// Save translator object data as .xml-file
	bool SaveXML(LPCTSTR szFilename) const
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

	// Extract translator object data as .po-file
	bool SavePO(LPCTSTR szFilename) const
	{
		_tprintf( _T("Saving PO file: '%s'\n"), szFilename );

		CStringA sOutput;
		sOutput.Format(
			"# Messages: %d\n"
			"msgid \"\"\n"
			"msgstr \"\"\n"
			"\"Project-Id-Version: Shareaza\\n\"\n"
			"\"Report-Msgid-Bugs-To: ryo-oh-ki <ryo-oh-ki@narod.ru>\\n\"\n"
			"\"POT-Creation-Date: %s\\n\"\n"
			"\"PO-Revision-Date: \\n\"\n"
			"\"Last-Translator: ryo-oh-ki <ryo-oh-ki@narod.ru>\\n\"\n"
			"\"Language-Team: Shareaza Development Team\\n\"\n"
			"\"MIME-Version: 1.0\\n\"\n"
			"\"Content-Type: text/plain; charset=utf-8\\n\"\n"
			"\"Content-Transfer-Encoding: 8bit\\n\"\n"
			"\"X-Poedit-Language: English\\n\"\n"
			"\"X-Poedit-Country: UNITED STATES\\n\"\n"
			"\"X-Poedit-SourceCharset: utf-8\\n\"\n",
			m_Items.GetCount(), (LPCSTR)CT2A(
			CTime::GetCurrentTime().FormatGmt( _T("%Y-%m-%d %H:%M+0000") ) ) );

		for( POSITION pos = m_Items.GetHeadPosition(); pos; )
		{
			const CItem& item = m_Items.GetNext( pos );

			if ( item.sID == "" )
				continue;

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
			return SUCCEEDED( oFile.Write( (LPCSTR)sOutput, sOutput.GetLength() ) );
		}
		else
			_tprintf( _T("Error: Can't save file: %s\n"), szFilename );

		return false;
	}

protected:
	class CItem
	{
	public:
		CItem() :
			bKeepUnderscores( false )
		{
		}
		CItem(const CString& r, const CString& t, bool k) :
			sRef( r ), sID( t ), bKeepUnderscores( k )
		{
		}
		void Clear()
		{
			sRef.Empty();
			sID.Empty();
			sTranslated.Empty();
		}
		CString sRef;
		CString sID;
		CString sTranslated;
		bool	bKeepUnderscores;
	} ;

	typedef CAtlList< CItem > CItemList;
	typedef CAtlMap< CString, CItem* > CItemMap;

	CComPtr< IXMLDOMDocument > m_pXMLDoc;	// XML data
	bool				m_bRemoveComments;	// Remove comments from XML data
	const CXMLLoader*	m_pXMLTranslator;	// XML load translator
	CItemList			m_Items;			// PO data
	CItemMap			m_Index;			// PO data index

	static CStringA UTF8Encode(LPCWSTR szInput, int nInput = -1)
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

	static CStringW UTF8Decode(LPCSTR szInput, int nInput = -1)
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

	static CString& MakeSafe(CString& str)
	{
		CString tmp;
		LPTSTR dst = tmp.GetBuffer( str.GetLength() * 2 + 1 );
		for ( LPCTSTR src = str; *src; src++ )
		{
			if ( *src >= _T(' ') )
			{
				switch ( *src )
				{
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
		}
		*dst = 0;
		tmp.ReleaseBuffer();
		str = tmp;
		return str;
	}

	static CStringA& UnMakeSafe(CStringA& str)
	{
		str.Replace( "\\\\", "\\" );
		str.Replace( "\\\"", "\"" );
		return str;
	}

	bool Add(const CString& sRef, CString sID, bool bKeepUnderscores = false)
	{
		// #:  file_name:line_number
		// msgid ""
		// msgstr ""

		CString sRefLC( sRef );
		sRefLC.MakeLower();

		if( sRef.Find( _T(" \t\r\n#:") ) != -1 )
		{
			_tprintf( _T("Found invalid PO reference: %s\n"), sRef );
			return false;
		}

		const CItemMap::CPair* pair = m_Index.Lookup( sRefLC );
		if ( pair )
		{
			// Skip duplicate
			return true;
		}

		for( POSITION pos = m_Items.GetHeadPosition(); pos; )
		{
			CItem& item = m_Items.GetNext( pos );
			if ( ( bKeepUnderscores && item.bKeepUnderscores ) ?
				IsEqual( item.sID, sID ) : ( item.sID == sID ) )
			{
				// Already defined string
				item.sRef = item.sRef + _T(" ") + sRef;
				m_Index.SetAt( sRefLC, &item );
				return true;
			}
		}

		if ( ! bKeepUnderscores )
			sID.Remove( _T('_') );

		// New string
		POSITION pos = m_Items.AddTail( CItem( sRef, sID, bKeepUnderscores ) );
		CItem& item = m_Items.GetAt( pos );
		m_Index.SetAt( sRefLC, &item );
		return true;
	}

	static bool IsEqual(const CString& _left, const CString& _right)
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

	bool LoadManifest(IXMLDOMElement* /*pXMLElement*/)
	{
		/*CComVariant vName;
		pXMLElement->getAttribute( CComBSTR( _T("name") ), &vName );
		CComVariant vAuthor;
		pXMLElement->getAttribute( CComBSTR( _T("author") ), &vAuthor );*/
		return true;
	}

	bool Load(LPCWSTR szParentName, LPCWSTR szRefName, LPCWSTR szTextName,
		IXMLDOMElement* pXMLElement, bool bKeepUnderscores = false)
	{
		CComVariant vRef;
		if ( szRefName )
		{
			if ( S_OK != pXMLElement->getAttribute( CComBSTR( szRefName ), &vRef ) )
			{
				_tprintf( _T("Missed required XML attribute \"%s\" at \"%s;?\"\n"),
					szRefName, szParentName );
				return false;
			}
		}

		CComVariant vText;
		if ( S_OK != pXMLElement->getAttribute( CComBSTR( szTextName ), &vText ) )
			// Skip empty tag
			return true;

		CString sRef( szRefName ?
			( CString( szParentName ) + _T(";") + (LPCWSTR)vRef.bstrVal ) : szParentName );
		CString sText( vText.bstrVal );

		if ( m_pXMLTranslator )
		{
			// On-line translation
			CString sRefLC( sRef );
			sRefLC.MakeLower();

			CItem* translated = NULL;
			if ( m_pXMLTranslator->m_Index.Lookup( sRefLC, translated ) )
			{
				if ( translated->sTranslated == _T("") )
					pXMLElement->setAttribute(
						CComBSTR( szTextName ), CComVariant( translated->sID ) );
				else
					pXMLElement->setAttribute(
						CComBSTR( szTextName ), CComVariant( translated->sTranslated ) );
			}
			else
				_tprintf( _T("Missed translation for %s=\"%s\" %s=\"%s\"\n"),
					( szRefName ? szRefName : _T("") ), sRef, szTextName,
					(LPCWSTR)vText.bstrVal );

			return true;
		}
		else
			return Add( sRef, sText, bKeepUnderscores );
	}

	bool LoadToolbar(IXMLDOMElement* pXMLRoot)
	{
		CComVariant vName;
		pXMLRoot->getAttribute( CComBSTR( _T("name") ), &vName );

		CString tmp;
		int i = 0;

		XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

		// <button id="" text=""/>
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
				if ( ! Load( CString( vName.bstrVal ) + _T(";button-text"),
					_T("id"), _T("text"), pXMLElement ) )
					return false;
				if ( ! Load( CString( vName.bstrVal ) + _T(";button-tip"),
					_T("id"), _T("tip"), pXMLElement ) )
					return false;
			}
			else if ( name == _T("control") )
			{
				if ( ! Load( CString( vName.bstrVal ) + _T(";control"),
					_T("id"), _T("text"), pXMLElement ) )
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
				i++;
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

	bool LoadToolbars(IXMLDOMElement* pXMLRoot)
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

	bool LoadMenu(IXMLDOMElement* pXMLRoot, LPCTSTR szParentName)
	{
		CComVariant vName;
		pXMLRoot->getAttribute( CComBSTR( _T("name") ), &vName );
		if ( vName.vt == VT_NULL )
			vName = szParentName;

		CString tmp;
		int i = 0;

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
				tmp.Format( _T("%s;%d"), (LPCWSTR)vName.bstrVal, ++i );
				if ( ! Load( tmp, NULL, _T("text"), pXMLElement, true ) )
					return false;
				if ( ! LoadMenu( pXMLElement, tmp ) )
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

	bool LoadMenus(IXMLDOMElement* pXMLRoot)
	{
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
				if ( ! LoadMenu( pXMLElement, _T("") ) )
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

	bool LoadDocuments(IXMLDOMElement* pXMLRoot)
	{
		XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

		// <document name="" title=""></document>

		CComQIPtr< IXMLDOMElement > pXMLElement( pXMLNode );

		XML_FOR_EACH_END()

		return true;
	}

	bool LoadCommandTips(IXMLDOMElement* pXMLRoot)
	{
		XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

		// <tip id="" message=""/>

		CComQIPtr< IXMLDOMElement > pXMLElement( pXMLNode );

		XML_FOR_EACH_END()

		return true;
	}

	bool LoadControlTips(IXMLDOMElement* pXMLRoot)
	{
		XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

		// <tip id="" message=""/>

		CComQIPtr< IXMLDOMElement > pXMLElement( pXMLNode );

		XML_FOR_EACH_END()

		return true;
	}

	bool LoadStrings(IXMLDOMElement* pXMLRoot)
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

	bool LoadDialogs(IXMLDOMElement* pXMLRoot)
	{
		XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

		// <dialog name="" cookie="" caption=""></dialog>

		CComQIPtr< IXMLDOMElement > pXMLElement( pXMLNode );

		XML_FOR_EACH_END()

		return true;
	}

	bool LoadListColumns(IXMLDOMElement* pXMLRoot)
	{
		XML_FOR_EACH_BEGIN( pXMLRoot, pXMLNode );

		// <list name=""></list>

		CComQIPtr< IXMLDOMElement > pXMLElement( pXMLNode );

		XML_FOR_EACH_END()

		return true;
	}

	bool LoadSkin(IXMLDOMElement* pXMLRoot)
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
};

int _tmain(int argc, _TCHAR* argv[])
{
	int ret = 1;

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
		return ret;
	}

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
				if ( oTranslated.LoadXML( argv[ 2 ] ) )
				{
					ret = 0;
				}
			}
			else if ( bXMLMode )
			{
				// Load translated text as PO
				if ( oTranslated.LoadPO( argv[ 2 ] ) )
				{
					oTemplate.SetTranslator( &oTranslated );
					ret = 0;
				}
			}
			else
				ret = 0;

			if ( ret == 0 )
			{
				ret = 2;

				// Load template
				if ( oTemplate.LoadXML( szTemplate ) )
				{
					if ( bPoMode )
					{
						// Generate .po-file from template XML and translated XML
						oTemplate.Translate( oTranslated );
						ret = oTemplate.SavePO( szOutput ) ? 0 : 2;
					}
					else if ( bXMLMode )
					{
						// Generate .xml-file from template XML and translated .po-file
						ret = oTemplate.SaveXML( szOutput ) ? 0 : 2;
					}
					else
					{
						// Generate .pot-file from template XML
						ret = oTemplate.SavePO( szOutput ) ? 0 : 2;
					}
				}
			}
		}
		CoUninitialize();
	}
	else
		_tprintf( _T("Error: Can't initialize COM\n") );

	return ret;
}
