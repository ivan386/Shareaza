/*
This software is released into the public domain.  You are free to 
redistribute and modify without any restrictions with the exception of
the following:

The Zlib library is Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler.
The Unzip library is Copyright (C) 1998-2003 Gilles Vollant.
*/
#include "skin.h"

// EXPORT BEGIN
void ExtractSkinFile(char *szFile) {
	char *szRealFile = _strdup(szFile), *tmp;

	if (*szRealFile=='\"') szRealFile++;
	tmp = szRealFile;
	while (tmp && *tmp) {
		if (*tmp=='\"') {
			*tmp = '\0';
			break;
		}
		tmp++;
	}	
	DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN), NULL, ExtractProc, (LPARAM)szRealFile);
	free(szRealFile);
}

int GetInstallDirectory() {
    //Registry Variables
	HKEY hKey;
    DWORD dwBufLen;
    LONG lRet;
    
    //Get the Shareaza install directory from the registry
    //Check for "Path hack" first : http://shareazawiki.anenga.com/tiki-index.php?page=FAQ%3AMiscellaneous   
    lRet = RegOpenKeyEx( HKEY_CURRENT_USER,
           "Software\\Shareaza\\Shareaza",
           0, KEY_QUERY_VALUE, &hKey );
    if( lRet != ERROR_SUCCESS )
        return 0;
     
    dwBufLen=MAX_PATH;
    lRet = RegQueryValueEx( hKey, "Path", NULL, NULL,
           (LPBYTE) prefix, &dwBufLen);
    if (dwBufLen > MAX_PATH) {
        return 0;
        }
    RegCloseKey( hKey );
    if (lRet != ERROR_SUCCESS) { 
        lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
               "SOFTWARE\\Shareaza",
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
    strcat(prefix,"\\Skins\\");
    return 1;
}

int GetSkinFileCount(char *szFile) {
	unz_global_info gi;
	int i, err;
	unzFile ufile;
	
	ufile = unzOpen(szFile);
	if (!ufile) return 0;
	err = unzGetGlobalInfo(ufile, &gi);
	if (err!=UNZ_OK) return 0;
	return gi.number_entry;
}

int ValidateSkin(char *szFile, HWND hwndDlg) {
	unz_global_info gi;
	int i, err, xmlFile = 0;
	unzFile ufile;
	char *buf,*tmp;

	ufile = unzOpen(szFile);
	if (!ufile) return 0;
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
						if (tmp=strstr(buf, "<manifest")) {
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
				szXML = _strdup(filename_withoutpath);
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

int ExtractSkin(char *szFile, HWND hwndDlg) {
	unz_global_info gi;
	int i, err, xmlFile = 0;
	unzFile ufile;
	FILE *wfile;
	unz_file_info fi;
	char fn_zip[MAX_PATH], fn_fs[MAX_PATH], buf[256];
	char *p, *filename_withoutpath;
		
	ufile = unzOpen(szFile);
	if (!ufile) return 0;
	err = unzGetGlobalInfo(ufile, &gi);
	if (err!=UNZ_OK) return 0;
             
    if (skinType == 0) {
        strcat(prefix,szName);
        //Create Directory for the new skin    
        if (!MakeDirectory(prefix)) {
    		unzClose(ufile);
    		return 0;
		}
	}
    else {
    	strcat(prefix,"Languages\\");
    }
    
	for (i=0;i<gi.number_entry;i++) {
		err = unzGetCurrentFileInfo(ufile, &fi, fn_zip, sizeof(fn_zip), NULL, 0, NULL, 0);
		p = filename_withoutpath = fn_zip;
        while ((*p) != '\0')
        {
            if (((*p)=='/') || ((*p)=='\\'))
                filename_withoutpath = p+1;
            p++;
        }
		SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_STEPIT, 0, 0);
		{
			char pb[512];

			_snprintf(pb, sizeof(pb), "Installing (%s)...", filename_withoutpath);
			SetWindowText(GetDlgItem(hwndDlg, IDC_STATUS), pb);
		}
		if (err!=UNZ_OK) {
			unzClose(ufile);
			return 0;
		}

        if ((*filename_withoutpath) != '\0') {
            if (skinType == 1) {
            	strcpy(fn_fs,prefix);
                strcat(fn_fs,filename_withoutpath);
            }           
            else {
            	strcpy(fn_fs,prefix);
                strcat(fn_fs,"\\");
                strcat(fn_fs,filename_withoutpath);
            }
            
    		err = unzOpenCurrentFile(ufile);
    		if (err!=UNZ_OK) return 0;
    		wfile = fopen(fn_fs, "wb");
    		if (!wfile) {
    			unzCloseCurrentFile(ufile);
    			unzClose(ufile);
    			return 0;
    		}
    		do {
    			err = unzReadCurrentFile(ufile, buf, sizeof(buf));
    			if (err<0) {
    				unzCloseCurrentFile(ufile);
    				unzClose(ufile);
    				return 0;
    			}
    			else if (err>0) {
    				if (fwrite(buf,err,1,wfile)!=1) {
    					unzCloseCurrentFile(ufile);
    					unzClose(ufile);
    					return 0;
    				}
    			}
    		} while (err);
    	}
    	unzCloseCurrentFile(ufile);

   		if ((i+1)<gi.number_entry) {
   			err = unzGoToNextFile(ufile);
   			if (err!=UNZ_OK) {
   				unzClose(ufile);
   				return 0;
   			}
   		}
        
	}
	unzClose(ufile);
	return 1;
}
// EXPORT END
