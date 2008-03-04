/*
This software is released into the public domain.  You are free to 
redistribute and modify without any restrictions with the exception of
the following:

The Zlib library is Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler.
The Unzip library is Copyright (C) 1998-2003 Gilles Vollant.
*/
#include "skin.h"

// EXPORT BEGIN
void ExtractSkinFile(LPCTSTR szFile) {
	TCHAR* szRealFile = _wcsdup( szFile );
	TCHAR* tmp;

	if ( *szRealFile=='\"') szRealFile++;
	tmp = szRealFile;
	while ( tmp && *tmp )
	{
		if ( *tmp=='\"' ) 
		{
			*tmp = '\0';
			break;
		}
		tmp++;
	}

	DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN), NULL, ExtractProc, (LPARAM)szRealFile);
	//free(szRealFile);
}

int GetInstallDirectory() {
    //Registry Variables
	HKEY hKey;
    DWORD dwBufLen;
    LONG lRet;
    
    //Get the Shareaza install directory from the registry
    //Check for "Path hack" first : http://shareazawiki.anenga.com/tiki-index.php?page=FAQ%3AMiscellaneous   
    lRet = RegOpenKeyEx( HKEY_CURRENT_USER,
           L"Software\\Shareaza\\Shareaza",
           0, KEY_QUERY_VALUE, &hKey );
    if( lRet != ERROR_SUCCESS )
        return 0;
     
    dwBufLen=MAX_PATH;
    lRet = RegQueryValueEx( hKey, L"Path", NULL, NULL,
           (LPBYTE) prefix, &dwBufLen);
    if (dwBufLen > MAX_PATH) {
        return 0;
        }
    RegCloseKey( hKey );
    if (lRet != ERROR_SUCCESS) { 
        lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
               L"SOFTWARE\\Shareaza",
               0, KEY_QUERY_VALUE, &hKey );
        if( lRet != ERROR_SUCCESS ) {
            return 0;
            }
        lRet = RegQueryValueEx( hKey, NULL, NULL, NULL,
               (LPBYTE) prefix, &dwBufLen);
        if( (lRet != ERROR_SUCCESS) || (dwBufLen > MAX_PATH) ) {
            return 0;
            }
        RegCloseKey( hKey ); 
    }
    wcscat(prefix, L"\\Skins\\");
    return 1;
}

int GetSkinFileCount(LPTSTR pszFile) 
{
	unz_global_info gi;
	int err;
	unzFile ufile;

	char* pszDest = (char*)malloc( wcslen( pszFile ) + 1 );
	char* tmp = pszDest;
	LPTSTR pszScanName = pszFile;

	for ( ; *pszScanName ; pszScanName++, pszDest++ ) *pszDest = (char)*pszScanName;
	*pszDest = '\0';

	ufile = unzOpen( tmp );

	if ( !ufile )
	{
		pszScanName = pszFile;
		pszDest = tmp;
		if ( GetShortPathNameW( pszFile, pszScanName, (DWORD)wcslen( pszFile ) + 1 ) )
		{
			for ( ; *pszScanName ; pszScanName++, pszDest++ ) *pszDest = (char)*pszScanName;
			*pszDest = '\0';
			ufile = unzOpen( tmp );
		}
	}
	
	free(tmp);
	if ( !ufile ) return 0;
	err = unzGetGlobalInfo(ufile, &gi);
	if (err!=UNZ_OK) return 0;
	return gi.number_entry;
}

int ValidateSkin(LPTSTR pszFile, HWND hwndDlg) {
	unz_global_info gi;
	UINT i = 0;
	int err, xmlFile = 0;
	unzFile ufile;
	char *buf,*tmp;

	char* pszDest = (char*)malloc( wcslen( pszFile ) + 1 );
	char* tmpName = pszDest;
	LPTSTR pszScanName = pszFile;

	for ( ; *pszScanName ; pszScanName++, pszDest++ ) *pszDest = (char)*pszScanName;
	*pszDest = '\0';

	ufile = unzOpen( tmpName );

	if ( !ufile )
	{
		pszScanName = pszFile;
		pszDest = tmpName;
		if ( GetShortPathNameW( pszFile, pszScanName, (DWORD)wcslen( pszFile ) + 1 ) )
		{
			for ( ; *pszScanName ; pszScanName++, pszDest++ ) *pszDest = (char)*pszScanName;
			*pszDest = '\0';
			ufile = unzOpen( tmpName );
		}
	}
	
	free(tmpName);
	if ( !ufile ) return 0;

	err = unzGetGlobalInfo(ufile, &gi);
	if (err!=UNZ_OK) return 0;
	for (i=0;i<gi.number_entry;i++) {
		unz_file_info fi;
		char fn_zip[MAX_PATH];
		char *p, *filename_withoutpath;

		err = unzGetCurrentFileInfo(ufile, &fi, fn_zip, sizeof(fn_zip), NULL, 0, NULL, 0);
		if (err!=UNZ_OK) {
			unzClose(ufile);
			return 0;
		}
		if (strstr(fn_zip, "../") || strstr(fn_zip, "..\\") || fn_zip[0]=='.') {
			unzClose(ufile);
			return 0;
		}
		// Only allow .xml and .XML (who would say .Xml anyway?)
		if (strstr(fn_zip, ".xml") || strstr(fn_zip, ".XML") && !xmlFile) {
			xmlFile = 1;
			{
				buf = (char*)malloc(fi.uncompressed_size+1);
				err = unzOpenCurrentFile(ufile);
				if (err!=UNZ_OK) return 0;
				do {
					err = unzReadCurrentFile(ufile, buf, fi.uncompressed_size);
					if (err<0) {
						free(buf);
						unzCloseCurrentFile(ufile);
						unzClose(ufile);
						return 0;
					}
					else {
						// Make sure the string is NULL terminated
						buf[err] = '\0';

						if ((tmp=strstr(buf, "<manifest"))!=NULL) {
							LoadManifestInfo(tmp);
						}
					}
				} while (err);
				free(buf);
				unzCloseCurrentFile(ufile);

				p = filename_withoutpath = fn_zip;
				while ((*p) != '\0')
        		{
            		if (((*p)=='/') || ((*p)=='\\'))
                	filename_withoutpath = p+1;
            		p++;
        		}
				if ( szXML ) free(szXML);
				szXML = (TCHAR*)GetUnicodeString(filename_withoutpath);
			}
		}
		if ((i+1)<gi.number_entry) {
			err = unzGoToNextFile(ufile);
			if (err!=UNZ_OK) {
				unzClose(ufile);
				return 0;
			}
		}
	}
	unzClose(ufile);
	if (!xmlFile) return 0;
	return 1;
}

int ExtractSkin(LPTSTR pszFile, HWND hwndDlg) {
	unz_global_info gi;
	UINT i = 0;
	int err; /* xmlFile = 0; */
	unzFile ufile;
	DWORD nBytesWritten = 0;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	unz_file_info fi;
	char fn_zip[MAX_PATH], buf[256];
	TCHAR *p, *filename_withoutpath, *zippedName;
	TCHAR fn_fs[MAX_PATH];

	char* pszDest = (char*)malloc( wcslen( pszFile ) + 1 );
	char* tmp = pszDest;
	LPTSTR pszScanName = pszFile;

	for ( ; *pszScanName ; pszScanName++, pszDest++ ) *pszDest = (char)*pszScanName;
	*pszDest = '\0';
		
	ufile = unzOpen( tmp );

	if ( !ufile )
	{
		pszScanName = pszFile;
		pszDest = tmp;
		if ( GetShortPathNameW( pszFile, pszScanName, (DWORD)wcslen( pszFile ) + 1 ) )
		{
			for ( ; *pszScanName ; pszScanName++, pszDest++ ) *pszDest = (char)*pszScanName;
			*pszDest = '\0';
			ufile = unzOpen( tmp );
		}
	}
	
	free(tmp);

	err = unzGetGlobalInfo(ufile, &gi);
	if (err!=UNZ_OK) return 0;
    
    if (skinType == 0) {
        wcscat(prefix, szName);
        //Create Directory for the new skin  
        if (!MakeDirectory((LPCTSTR)prefix)) {
    		unzClose(ufile);
			return 0;
		}
	}
    else {
    	wcscat(prefix, L"Languages\\");
    }
    
	for (i=0;i<gi.number_entry;i++) {
		err = unzGetCurrentFileInfo(ufile, &fi, fn_zip, sizeof(fn_zip), NULL, 0, NULL, 0);
		
		zippedName = p = filename_withoutpath = (TCHAR*)GetUnicodeString(fn_zip);
        while ((*p) != '\0')
        {
            if (((*p)=='/') || ((*p)=='\\'))
                filename_withoutpath = p+1;
            p++;
        }
		SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_STEPIT, 0, 0);
		{
			TCHAR pb[512];

			_snwprintf(pb, sizeof(pb), L"Installing (%s)...", (TCHAR*)filename_withoutpath);
			SetWindowText(GetDlgItem(hwndDlg, IDC_STATUS), pb);
		}
		if (err!=UNZ_OK) {
			unzClose(ufile);
			free(zippedName);
			return 0;
		}

        if ((*filename_withoutpath) != '\0') {
            if (skinType == 1) {
            	wcscpy(fn_fs, (LPCTSTR)prefix);
                wcscat(fn_fs, filename_withoutpath);
            }           
            else {
            	wcscpy(fn_fs, (LPCTSTR)prefix);
                wcscat(fn_fs, L"\\");
                wcscat(fn_fs, filename_withoutpath);
            }
            
    		err = unzOpenCurrentFile(ufile);
			if (err!=UNZ_OK) {
				free(zippedName);
				return 0;
			}
			hFile = CreateFile( fn_fs, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
				FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE, NULL );

			if (!hFile) {
    			unzCloseCurrentFile(ufile);
    			unzClose(ufile);
				free(zippedName);
    			return 0;
    		}
    		do {
    			err = unzReadCurrentFile(ufile, buf, sizeof(buf));
    			if ( err < 0 ) {
    				unzCloseCurrentFile(ufile);
    				unzClose(ufile);
					free(zippedName);
					CloseHandle( hFile );
    				return 0;
    			}
    			else if ( err > 0 ) {
    				if ( WriteFile( hFile, (LPCVOID)buf, err, &nBytesWritten, NULL ) != TRUE ||
						err != (int)nBytesWritten ) {
    					unzCloseCurrentFile(ufile);
    					unzClose(ufile);
						free(zippedName);
						CloseHandle( hFile );
    					return 0;
    				}
    			}
    		} while (err);
    	}
    	unzCloseCurrentFile(ufile);
		CloseHandle( hFile );

   		if ((i+1)<gi.number_entry) {
   			err = unzGoToNextFile(ufile);
   			if (err!=UNZ_OK) {
   				unzClose(ufile);
				free(zippedName);
   				return 0;
   			}
   		}
        free(zippedName);
	}
	unzClose(ufile);
	return 1;
}

LPCTSTR GetUnicodeString(char* pszString)
{
	TCHAR* ret;
	int nLen = 0;
	nLen = MultiByteToWideChar( CP_UTF8, 0, (LPCSTR)pszString, (DWORD)strlen(pszString), NULL, 0 );
	if ( nLen == 0 ) return NULL;
	ret = (TCHAR*)malloc( ( nLen + 1) * sizeof(TCHAR) );
	MultiByteToWideChar( CP_UTF8, 0, (LPCSTR)pszString, (DWORD)strlen(pszString), ret, nLen * sizeof(TCHAR) );
	ret[nLen] = '\0';
	return ret;
}
// EXPORT END
