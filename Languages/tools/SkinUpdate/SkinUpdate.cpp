#include "stdafx.h"

enum States
{
	stFile,
	stGudelines, stGudelinesContent, stGudelinesDialog, stGudelinesDialogContent,
	stStringTable, stContent, stString,
	stError
};

typedef CAtlMap< CStringA, UINT > CSUMap;
typedef CAtlMap< UINT, CStringA > CUSMap;

CSUMap g_oGudelines;
CSUMap g_oDialogs;
CSUMap g_oIDs;
CUSMap g_oStrings;

BOOL ProcessString(CStringA sID, CStringA sString)
{
	int len = sString.GetLength();
	if ( sString.GetAt( 0 ) != '\"' || sString.GetAt( len - 1 ) != '\"' )
	{
		_tprintf( _T("Error: Invalid string format \"%hs\"\n"), sString );
		return FALSE;
	}
	sString = sString.Mid( 1, len - 2 );

	UINT nID = 0;
	if ( ! g_oIDs.Lookup( sID, nID ) )
	{
		_tprintf( _T("Warning: Unknown ID %hs \"%hs\"\n"), sID, sString );
		return FALSE;
	}

	CStringA sFoo;
	if ( g_oStrings.Lookup( nID, sFoo ) )
	{
		_tprintf( _T("Error: Duplicate ID %hs \"%hs\"\n"), sID, sString );
		return FALSE;
	}

	g_oStrings.SetAt( nID, sString );

	return TRUE;
}

BOOL LoadIDs(LPCTSTR szFilename)
{
	BOOL bSuccess = FALSE;

	FILE* pFile = NULL;
	if ( _tfopen_s( &pFile, szFilename, _T("rb") ) == 0 )
	{
		for (;;)
		{
			CStringA sLine;
			CHAR* res = fgets( sLine.GetBuffer( 4096 ), 4096, pFile );
			sLine.ReleaseBuffer();
			if ( ! res )
				// End of file
				break;
			sLine.Trim( " \t\r\n" );
			if ( sLine.IsEmpty() || sLine.GetAt( 0 ) != '#' )
				// Skip empty lines
				continue;
			int nPos = sLine.FindOneOf( " \t" );
			if ( nPos == -1 || sLine.Left( nPos ) != "#define" )
				// Skip unknown line
				continue;
			sLine = sLine.Mid( nPos + 1 ).TrimLeft( " \t" );
			nPos = sLine.FindOneOf( " \t" );
			if ( nPos == -1 )
				// Skip unknown line
				continue;
			CStringA sID = sLine.Left( nPos );
			sLine = sLine.Mid( nPos + 1 ).TrimLeft( " \t" );
			int nID = 0;
			if ( sLine.Left( 2 ).CompareNoCase( "0x" ) == 0 )
			{
				if ( sscanf_s( sLine, "%x", &nID ) != 1 )
					// Skip unknown line
					continue;
			}
			else
			{
				if ( sscanf_s( sLine, "%u", &nID ) != 1 )
					// Skip unknown line
					continue;
			}
			g_oIDs.SetAt( sID, nID );
			bSuccess = TRUE;
		}			
		fclose( pFile );
	}

	return bSuccess;
}

BOOL LoadResources(LPCTSTR szFilename)
{
	FILE* pFile = NULL;
	if ( _tfopen_s( &pFile, szFilename, _T("rb") ) == 0 )
	{
		CStringA sID;
		for ( States nState = stFile; nState != stError; )
		{
			CStringA sLine;
			CHAR* res = fgets( sLine.GetBuffer( 4096 ), 4096, pFile );
			sLine.ReleaseBuffer();
			if ( ! res )
			{
				if ( nState != stFile )
				{
					_tprintf( _T("Error: Unexpected end of file\n") );
				}
				// End of file
				break;
			}
			sLine.Trim( ", \t\r\n" );
			if ( sLine.IsEmpty() ||
				 sLine.GetAt( 0 ) == '/' ||
				 sLine.GetAt( 0 ) == '#' )
				// Skip empty line, comment and pragma
				continue;

			switch ( nState )
			{
			case stFile:
				if ( sLine == "STRINGTABLE" )
					nState = stStringTable;
				else if ( sLine == "GUIDELINES DESIGNINFO" )
					nState = stGudelines;
				else
				{
					int nPos = sLine.Find( " DIALOG" ); // DIALOG or DIALOGEX
					if ( nPos != -1 )
					{
						CStringA sID = sLine.SpanExcluding( " " );
						UINT nID;
						if ( ! g_oIDs.Lookup( sID, nID ) )
						{
							_tprintf( _T("Error: Unknown ID \"%hs\" inside DIALOG\n"), sID );
							return 2;
						}
						if ( g_oDialogs.Lookup( sID, nID ) )
						{
							_tprintf( _T("Error: Duplicate ID \"%hs\" inside DIALOG\n"), sID );
							return 2;
						}
						g_oDialogs.SetAt( sID, nID );
					}
				}
				break;

			case stGudelines:
				if ( sLine == "BEGIN" )
					nState = stGudelinesContent;
				else
				{
					_tprintf( _T("Error: BEGIN not found after GUIDELINES DESIGNINFO\n") );
					return 2;
				}
				break;

			case stGudelinesContent:
				if ( sLine == "END" )
					nState = stFile;
				else if ( sLine.Right( 6 ) == "DIALOG" )
				{
					nState = stGudelinesDialog;
					CStringA sID = sLine.SpanExcluding( "," );
					UINT nID;
					if ( ! g_oIDs.Lookup( sID, nID ) )
					{
						_tprintf( _T("Error: Unknown dialog ID \"%hs\" inside GUIDELINES\n"), sID );
						return 2;
					}
					if ( g_oGudelines.Lookup( sID, nID ) )
					{
						_tprintf( _T("Error: Duplicate dialog ID \"%hs\" inside GUIDELINES\n"), sID );
						return 2;
					}
					if ( ! g_oDialogs.Lookup( sID, nID ) )
					{
						_tprintf( _T("Error: Orphan dialog ID \"%hs\" inside GUIDELINES\n"), sID );
						return 2;
					}
					g_oGudelines.SetAt( sID, nID );
				}
				else 
				{
					_tprintf( _T("Error: Unknown line \"%hs\" inside GUIDELINES\n"), sLine );
					return 2;
				}
				break;

			case stGudelinesDialog:
				if ( sLine == "END" )
					nState = stGudelinesContent;
				break;

			case stStringTable:
				if ( sLine == "BEGIN" )
					nState = stContent;
				else
				{
					_tprintf( _T("Error: BEGIN not found after STRINGTABLE\n") );
					return 2;
				}
				break;

			case stContent:
				if ( sLine == "END" )
					nState = stFile;
				else
				{
					int nPos = sLine.FindOneOf( ", \t" );
					if ( nPos != -1 )
					{
						if ( ProcessString( sLine.Left( nPos ),
							sLine.Mid( nPos + 1 ).TrimLeft( ", \t" ) ) )
							nState = stContent;
						else
							nState = stError;
					}
					else
					{
						sID = sLine;
						nState =  stString;
					}
				}
				break;

			case stString:
				if ( ProcessString( sID, sLine ) )
					nState = stContent;
				else
					nState = stError;
				sID.Empty();
				break;
			}
		}

		fclose( pFile );
	}

	return TRUE;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if ( argc < 4 )
	{
		_tprintf( _T("SkinUpdate 1.0\n")
			_T("Usage: SkinUpdate.exe input.h input.rc output.xml\n") );
		return 1;
	}

	LPCTSTR szOutput = NULL;

	for ( int i = 1; i < argc; i++ )
	{
		LPCTSTR szFilename = PathFindFileName( argv[ i ] );
		LPCTSTR szExt = PathFindExtension( szFilename );
		if ( _tcscmp( szExt, _T(".h") ) == 0 )
		{
			if ( ! LoadIDs( argv[ i ] ) )
			{
				_tprintf( _T("Error: Filed to load IDs from: %s\n"), szFilename );
				return 1;
			}
			_tprintf( _T("Loaded %d IDs from: %s\n"), g_oIDs.GetCount(), szFilename);
		}
		else if ( _tcscmp( szExt, _T(".rc") ) == 0 )
		{
			if ( ! LoadResources( argv[ i ] ) )
			{
				_tprintf( _T("Error: Filed to load strings from: %s\n"), szFilename );
				return 1;
			}
			_tprintf( _T("Loaded %d strings from: %s\n"), g_oStrings.GetCount(), szFilename );
			_tprintf( _T("Loaded %d gudelines from: %s\n"), g_oGudelines.GetCount(), szFilename );
			_tprintf( _T("Loaded %d dialogs from: %s\n"), g_oDialogs.GetCount(), szFilename );
		}
		else if ( _tcscmp( szExt, _T(".xml") ) == 0 )
		{
			szOutput = argv[ i ];
		}
		else
		{
			_tprintf( _T("Error: Unknown file extension: %s\n"), szExt );
			return 1;
		}
	}

	for ( POSITION pos = g_oDialogs.GetStartPosition(); pos; )
	{
		CStringA sID;
		UINT nID;
		g_oDialogs.GetNextAssoc( pos, sID, nID );
		if ( ! g_oGudelines.Lookup( sID, nID ) )
		{
			_tprintf( _T("Warning: Found dialog \"%hs\" without gudeline\n"), sID );			
		}
	}

	// Sort by ID
	std::list< UINT > index;
	for ( POSITION pos = g_oStrings.GetStartPosition(); pos; )
	{
		index.push_back( g_oStrings.GetNextKey( pos ) );
	}
	index.sort();

	if ( ! szOutput )
	{
		szOutput = _T("default-en.xml");
	}

	FILE* pFile = NULL;
	if ( _tfopen_s( &pFile, szOutput, _T("wt") ) != 0 )
	{
		_tprintf( _T("Error: Can't create output XML-file: %s\n"), szOutput );
		return 1;
	}

	_ftprintf( pFile, _T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n")
		_T("<skin xmlns=\"http://www.shareaza.com/schemas/Skin.xsd\" version=\"1.0\">\n")
		_T("\t<!-- Localised Strings -->\n")
		_T("\t<strings>\n") );

	for ( std::list< UINT >::iterator i = index.begin(); i != index.end(); ++i )
	{
		CStringA sString;
		g_oStrings.Lookup( (*i), sString );
		sString.Replace( "&", "&amp;" );
		sString.Replace( " ", "&#160;" ); // it's not a space
		sString.Replace( "\"\"", "&quot;" );
		sString.Replace( "\\r\\n", "\\n" );
		_ftprintf( pFile, _T("\t\t<string id=\"%u\" value=\"%hs\"/>\n"), (*i), sString ); 
	}

	_ftprintf( pFile, _T("\t</strings>\n")
		_T("</skin>\n") );

	fclose( pFile );

	_tprintf( _T("Saved output XML-file: %s\n"), szOutput );

	return 0;
}

