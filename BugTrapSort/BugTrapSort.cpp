// BugTrapSort.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

typedef struct 
{
	CString sIf;
	CString sIs;
	CString sThen;
} CRule;

typedef CAtlList< CRule > CRuleList;
typedef CAtlMap< CString, bool > CStringMap;

CString		g_sConfig;
CRuleList	g_oRules;
CString		g_sOutput;
CString		g_sInputDir;
CStringMap	g_oModules;

#define REPLACE(x) \
	x.Replace( _T("{VER_FULL}"), sVersion ); \
	x.Replace( _T("{VER_NUMBER}"), sVersionNumber ); \
	x.Replace( _T("{VER_CONFIG}"), sVersionConfig ); \
	x.Replace( _T("{VER_PLATFORM}"), sVersionPlatform ); \
	x.Replace( _T("{VER_REVISION}"), sVersionRevision ); \
	x.Replace( _T("{VER_DATE}"), sVersionDate ); \
	x.Replace( _T("{REPORT}"), sReport ); \
	x.Replace( _T("{USER}"), sUser ); \
	x.Replace( _T("{COMPUTER}"), sComputer ); \
	x.Replace( _T("{CPU}"), sCPU ); \
	x.Replace( _T("{WHAT}"), sWhat ); \
	x.Replace( _T("{MODULE}"), sModule ); \
	x.Replace( _T("{ADDRESS}"), sAddress );

CString GetValue(IXMLDOMElement* pRoot, LPCTSTR szXPath)
{
	CComPtr< IXMLDOMNode > pNode;
	HRESULT hr = pRoot->selectSingleNode( CComBSTR( szXPath ), &pNode );
	if ( hr != S_OK )
		return CString();

	CComBSTR bstrVersion;
	hr = pNode->get_text( &bstrVersion );
	if ( hr != S_OK )
		return CString();

	return (LPCWSTR)bstrVersion;
}

bool ProcessReport(const CString& sInput)
{
	CString sReport = PathFindFileName( sInput );
	sReport.TrimRight( _T("\\") );
	sReport = sReport.Right( 13 );
	_tprintf( _T("Processing %s ...\n"), sReport );

	CComPtr< IXMLDOMDocument > pFile;
	HRESULT hr = pFile.CoCreateInstance( CLSID_DOMDocument );
	if ( FAILED( hr ) )
		return false;

	hr = pFile->put_async( VARIANT_FALSE );
	if ( hr != S_OK )
		return false;

	VARIANT_BOOL ret = VARIANT_FALSE;
	hr = pFile->load( CComVariant( sInput + _T("errorlog.xml") ), &ret );
	if ( hr != S_OK || ! ret )
		return false;

	CComPtr< IXMLDOMElement > pRoot;
	hr = pFile->get_documentElement( &pRoot );
	if ( hr != S_OK )
		return false;

	CString sVersion = GetValue( pRoot, _T("/report/version") );

	CString sComputer = GetValue( pRoot, _T("/report/computer") );
	if ( sComputer.IsEmpty() ) sComputer = _T("UNKNOWN");

	CString sUser = GetValue( pRoot, _T("/report/user") );
	if ( sUser.IsEmpty() ) sUser = _T("UNKNOWN");

	CString sCPU = GetValue( pRoot, _T("/report/cpus/cpu/description") );
	if ( sCPU.IsEmpty() )
	{
		sCPU = GetValue( pRoot, _T("/report/cpus/cpu/id") );
		if ( sCPU.IsEmpty() )
			sCPU = _T("UNKNOWN");
	}

	CString sWhat = GetValue( pRoot, _T("/report/error/what") );
	if ( sWhat.IsEmpty() ) sWhat = _T("UNKNOWN");

	CString sAddress = GetValue( pRoot, _T("/report/error/address") );
	CString sModule = PathFindFileName( GetValue( pRoot, _T("/report/error/module") ) );
	sModule.MakeLower();
	bool bGood;
	if ( sModule.IsEmpty() || ( g_oModules.Lookup( sModule, bGood ) && ! bGood ) )
	{
		CString sOrigAddress = sAddress;
		CString sOrigModule = sModule;

		CComPtr< IXMLDOMNode > pStack;
		hr = pRoot->selectSingleNode( CComBSTR( _T("/report/threads/thread/stack") ), &pStack );
		if ( hr == S_OK )
		{
			CComPtr< IXMLDOMNodeList > pFrames;
			hr = pStack->get_childNodes( &pFrames );
			if ( hr == S_OK )
			{
				long n = 0;
				hr = pFrames->get_length( &n );
				for ( long i = 0; hr == S_OK && i < n; ++i )
				{
					CComPtr< IXMLDOMNode > pFrame;
					hr = pFrames->get_item( i, &pFrame );
					if ( hr == S_OK )
					{
						CComQIPtr< IXMLDOMElement > pFrameE( pFrame );
						sAddress = GetValue( pFrameE, _T("address") );
						sModule = PathFindFileName( GetValue( pFrameE, _T("module") ) );
						if ( ! sModule.IsEmpty() )
						{
							sModule.MakeLower();
							if ( ! g_oModules.Lookup( sModule, bGood ) || bGood )
								// Неизвестный модуль или подходящий модуль
								break;
						}
						sAddress.Empty();
						sModule.Empty();
					}
				}
			}
		}
		if ( sModule.IsEmpty() )
		{
			sAddress = sOrigAddress;
			sModule = sOrigModule;
		}
		if ( sModule.IsEmpty() ) sModule = _T("UNKNOWN");
	}
	sAddress.Remove( _T(':') );
	if ( sAddress.IsEmpty() ) sAddress = _T("UNKNOWN");

	int i = 0;
	CString sVersionNumber = sVersion.Tokenize( _T(" "), i );
	if ( i == -1 )
		return false;
	CString sVersionConfig = sVersion.Tokenize( _T(" "), i );
	if ( i == -1 )
		return false;
	if ( sVersionConfig.CompareNoCase( _T("release") ) == 0 ) sVersionConfig = _T("r");
	else if ( sVersionConfig.CompareNoCase( _T("debug") ) == 0 ) sVersionConfig = _T("d");
	CString sVersionPlatform = sVersion.Tokenize( _T(" "), i );
	if ( i == -1 )
		return false;
	if ( sVersionPlatform.Find( _T("32") ) != -1 ) sVersionPlatform = _T("32");
	else if ( sVersionPlatform.Find( _T("64") ) != -1 ) sVersionPlatform = _T("64");
	CString sVersionRevision = sVersion.Tokenize( _T(" "), i );
	if ( i == -1 )
		return false;
	sVersionRevision.Trim( _T(" ()") );
	sVersionRevision.MakeLower();
	CString sVersionDate = sVersion.Tokenize( _T(" "), i );
	if ( i == -1 )
		return false;
	sVersionDate.Trim( _T(" ()") );

	_tprintf( _T("Version: %ls\n"), sVersion );

	CString sOutput = g_sOutput;
	for ( POSITION pos = g_oRules.GetHeadPosition(); pos ; )
	{
		CRule r = g_oRules.GetNext( pos );
		REPLACE( r.sIf );
		if ( StrStrI( r.sIf, r.sIs ) )
		{
			sOutput = r.sThen;
			break;
		}
	}

	if ( sOutput.IsEmpty() )
		// Нет действия
		return true;

	REPLACE( sOutput );

	if ( sOutput.IsEmpty() )
		// Нет действия
		return true;

	sOutput.TrimRight( _T("\\") ) += _T("\\");

	ret = SHCreateDirectory( GetDesktopWindow(), sOutput );
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_EXISTS && ret != ERROR_ALREADY_EXISTS )
		return false;

	if ( ! CopyFile( sInput + _T("errorlog.xml"), sOutput + _T("errorlog.xml"), FALSE ) )
		return false;

	// Опциональные файлы
	CopyFile( sInput + _T("crashdump.dmp"), sOutput + _T("crashdump.dmp"), FALSE );
	CopyFile( sInput + _T("settings.reg"), sOutput + _T("settings.reg"), FALSE );

	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if ( argc != 2 )
		return 1;

	CoInitializeEx( NULL, COINIT_MULTITHREADED );

	g_sConfig = argv[ 1 ];

	GetPrivateProfileString( _T("options"), _T("output"), NULL,
		g_sOutput.GetBuffer( 4096 ), 4096, g_sConfig );
	g_sOutput.ReleaseBuffer();
	if ( ! g_sOutput.IsEmpty() )
		g_sOutput.TrimRight( _T("\\") ) += _T("\\");

	GetPrivateProfileString( _T("options"), _T("input"), NULL,
		g_sInputDir.GetBuffer( 4096 ), 4096, g_sConfig );
	g_sInputDir.ReleaseBuffer();
	if ( g_sInputDir.IsEmpty() )
		g_sInputDir = _T(".");
	g_sInputDir.TrimRight( _T("\\") ) += _T("\\");

	// Принять
	CString sModules;
	GetPrivateProfileString( _T("options"), _T("accept"), NULL,
		sModules.GetBuffer( 4096 ), 4096, g_sConfig );
	sModules.ReleaseBuffer();
	for ( int i = 0; ; )
	{
		CString sModule = sModules.Tokenize( _T("|"), i );
		if ( sModule.IsEmpty() || i == -1 )
			break;
		sModule.MakeLower();
		g_oModules.SetAt( sModule, true );
	}

	// Не принимать
	GetPrivateProfileString( _T("options"), _T("ignore"), NULL,
		sModules.GetBuffer( 4096 ), 4096, g_sConfig );
	sModules.ReleaseBuffer();
	for ( int i = 0; ; )
	{
		CString sModule = sModules.Tokenize( _T("|"), i );
		if ( sModule.IsEmpty() || i == -1 )
			break;
		sModule.MakeLower();
		g_oModules.SetAt( sModule, false );
	}

	// Загрузка списка секций
	CAutoVectorPtr< TCHAR > pSections( new TCHAR[ 16384 ] );
	GetPrivateProfileString( NULL, NULL, NULL, pSections, 16384, g_sConfig );

	// Загрузка правил из всех секций
	for ( LPCTSTR szSect = pSections ; szSect && *szSect ; szSect += lstrlen( szSect ) + 1 )
	{
		if  ( lstrcmpi( szSect, _T("options") ) == 0 )
			// Пропуск служебной секции
			continue;

		CRule r;
		GetPrivateProfileString( szSect, _T("if"), NULL,
			r.sIf.GetBuffer( 4096 ), 4096, g_sConfig );
		r.sIf.ReleaseBuffer();
		GetPrivateProfileString( szSect, _T("is"), NULL,
			r.sIs.GetBuffer( 4096 ), 4096, g_sConfig );
		r.sIs.ReleaseBuffer();
		GetPrivateProfileString( szSect, _T("output"), NULL,
			r.sThen.GetBuffer( 4096 ), 4096, g_sConfig );
		r.sThen.ReleaseBuffer();
		g_oRules.AddTail( r );
	}

	pSections.Free();

	CAtlList< CString > oDirs;
	WIN32_FIND_DATA wfa = {};
	HANDLE hFF = FindFirstFile( g_sInputDir + _T("Shareaza_*.*"), &wfa );
	if ( hFF != INVALID_HANDLE_VALUE )
	{
		do 
		{
			if ( ( wfa.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
			{
				oDirs.AddTail( g_sInputDir + wfa.cFileName + _T("\\") );
			}
		}
		while ( FindNextFile( hFF, &wfa ) );
		FindClose( hFF );
	}

	_tprintf( _T("Processing %d reports from folder %s ...\n"),
		oDirs.GetCount(), g_sInputDir );
	
	for ( POSITION pos = oDirs.GetHeadPosition(); pos; )
	{
		if ( ! ProcessReport( oDirs.GetNext( pos ) ) )
		{
			_tprintf( _T("Report load error!\n") );
		}
	}

	_tprintf( _T("Done.\n") );

	CoUninitialize();

	return 0;
}
