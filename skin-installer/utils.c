/*
This software is released into the public domain.  You are free to 
redistribute and modify without any restrictions with the exception of
the following:

The Zlib library is Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler.
The Unzip library is Copyright (C) 1998-2003 Gilles Vollant.
*/
#include "skin.h"

static char* GetManifestValue(char *manifest, char *searchKey);
static int CheckManifestForSkin(char *szFile);

// EXPORT BEGIN
void XMLDecode(char *str) {
	char *p, *q;

	if (str == NULL)
		return;
	for (p=q=str; *p!='\0'; p++,q++) {
		if (*p=='&') {
			     if (!strncmp(p, "&amp;", 5))  { *q = '&'; p += 4; }
			else if (!strncmp(p, "&apos;", 6)) { *q = '\''; p += 5; }
			else if (!strncmp(p, "&gt;", 4))   { *q = '>'; p += 3; }
			else if (!strncmp(p, "&lt;", 4))   { *q = '<'; p += 3; }
			else if (!strncmp(p, "&#161;", 6)) { *q = '¡'; p += 5; }
			else if (!strncmp(p, "&#162;", 6)) { *q = '¢'; p += 5; }
			else if (!strncmp(p, "&#163;", 6)) { *q = '£'; p += 5; }
			else if (!strncmp(p, "&#164;", 6)) { *q = '¤'; p += 5; }
			else if (!strncmp(p, "&#165;", 6)) { *q = '¥'; p += 5; }
			else if (!strncmp(p, "&#166;", 6)) { *q = '¦'; p += 5; }
			else if (!strncmp(p, "&#167;", 6)) { *q = '§'; p += 5; }
			else if (!strncmp(p, "&#168;", 6)) { *q = '¨'; p += 5; }
			else if (!strncmp(p, "&#169;", 6)) { *q = '©'; p += 5; }
			else if (!strncmp(p, "&#170;", 6)) { *q = 'ª'; p += 5; }
			else if (!strncmp(p, "&#171;", 6)) { *q = '«'; p += 5; }
			else if (!strncmp(p, "&#172;", 6)) { *q = '¬'; p += 5; }
			else if (!strncmp(p, "&#173;", 6)) { *q = '­'; p += 5; }
			else if (!strncmp(p, "&#174;", 6)) { *q = '®'; p += 5; }
			else if (!strncmp(p, "&#175;", 6)) { *q = '¯'; p += 5; }
			else if (!strncmp(p, "&#176;", 6)) { *q = '°'; p += 5; }
			else if (!strncmp(p, "&#177;", 6)) { *q = '±'; p += 5; }
			else if (!strncmp(p, "&#178;", 6)) { *q = '²'; p += 5; }
			else if (!strncmp(p, "&#179;", 6)) { *q = '³'; p += 5; }
			else if (!strncmp(p, "&#180;", 6)) { *q = '´'; p += 5; }
			else if (!strncmp(p, "&#181;", 6)) { *q = 'µ'; p += 5; }
			else if (!strncmp(p, "&#182;", 6)) { *q = '¶'; p += 5; }
			else if (!strncmp(p, "&#183;", 6)) { *q = '·'; p += 5; }
			else if (!strncmp(p, "&#184;", 6)) { *q = '¸'; p += 5; }
			else if (!strncmp(p, "&#185;", 6)) { *q = '¹'; p += 5; }
			else if (!strncmp(p, "&#186;", 6)) { *q = 'º'; p += 5; }
			else if (!strncmp(p, "&#187;", 6)) { *q = '»'; p += 5; }
			else if (!strncmp(p, "&#188;", 6)) { *q = '¼'; p += 5; }
			else if (!strncmp(p, "&#189;", 6)) { *q = '½'; p += 5; }
			else if (!strncmp(p, "&#190;", 6)) { *q = '¾'; p += 5; }
			else if (!strncmp(p, "&#191;", 6)) { *q = '¿'; p += 5; }
			else if (!strncmp(p, "&#192;", 6)) { *q = 'À'; p += 5; }
			else if (!strncmp(p, "&#193;", 6)) { *q = 'Á'; p += 5; }
			else if (!strncmp(p, "&#194;", 6)) { *q = 'Â'; p += 5; }
			else if (!strncmp(p, "&#195;", 6)) { *q = 'Ã'; p += 5; }
			else if (!strncmp(p, "&#196;", 6)) { *q = 'Ä'; p += 5; }
			else if (!strncmp(p, "&#197;", 6)) { *q = 'Å'; p += 5; }
			else if (!strncmp(p, "&#198;", 6)) { *q = 'Æ'; p += 5; }
			else if (!strncmp(p, "&#199;", 6)) { *q = 'Ç'; p += 5; }
			else if (!strncmp(p, "&#200;", 6)) { *q = 'È'; p += 5; }
			else if (!strncmp(p, "&#201;", 6)) { *q = 'É'; p += 5; }
			else if (!strncmp(p, "&#202;", 6)) { *q = 'Ê'; p += 5; }
			else if (!strncmp(p, "&#203;", 6)) { *q = 'Ë'; p += 5; }
			else if (!strncmp(p, "&#204;", 6)) { *q = 'Ì'; p += 5; }
			else if (!strncmp(p, "&#205;", 6)) { *q = 'Í'; p += 5; }
			else if (!strncmp(p, "&#206;", 6)) { *q = 'Î'; p += 5; }
			else if (!strncmp(p, "&#207;", 6)) { *q = 'Ï'; p += 5; }
			else if (!strncmp(p, "&#208;", 6)) { *q = 'Ð'; p += 5; }
			else if (!strncmp(p, "&#209;", 6)) { *q = 'Ñ'; p += 5; }
			else if (!strncmp(p, "&#210;", 6)) { *q = 'Ò'; p += 5; }
			else if (!strncmp(p, "&#211;", 6)) { *q = 'Ó'; p += 5; }
			else if (!strncmp(p, "&#212;", 6)) { *q = 'Ô'; p += 5; }
			else if (!strncmp(p, "&#213;", 6)) { *q = 'Õ'; p += 5; }
			else if (!strncmp(p, "&#214;", 6)) { *q = 'Ö'; p += 5; }
			else if (!strncmp(p, "&#215;", 6)) { *q = '×'; p += 5; }
			else if (!strncmp(p, "&#216;", 6)) { *q = 'Ø'; p += 5; }
			else if (!strncmp(p, "&#217;", 6)) { *q = 'Ù'; p += 5; }
			else if (!strncmp(p, "&#218;", 6)) { *q = 'Ú'; p += 5; }
			else if (!strncmp(p, "&#219;", 6)) { *q = 'Û'; p += 5; }
			else if (!strncmp(p, "&#220;", 6)) { *q = 'Ü'; p += 5; }
			else if (!strncmp(p, "&#221;", 6)) { *q = 'Ý'; p += 5; }
			else if (!strncmp(p, "&#222;", 6)) { *q = 'Þ'; p += 5; }
			else if (!strncmp(p, "&#223;", 6)) { *q = 'ß'; p += 5; }
			else if (!strncmp(p, "&#224;", 6)) { *q = 'à'; p += 5; }
			else if (!strncmp(p, "&#225;", 6)) { *q = 'á'; p += 5; }
			else if (!strncmp(p, "&#226;", 6)) { *q = 'â'; p += 5; }
			else if (!strncmp(p, "&#227;", 6)) { *q = 'ã'; p += 5; }
			else if (!strncmp(p, "&#228;", 6)) { *q = 'ä'; p += 5; }
			else if (!strncmp(p, "&#229;", 6)) { *q = 'å'; p += 5; }
			else if (!strncmp(p, "&#230;", 6)) { *q = 'æ'; p += 5; }
			else if (!strncmp(p, "&#231;", 6)) { *q = 'ç'; p += 5; }
			else if (!strncmp(p, "&#232;", 6)) { *q = 'è'; p += 5; }
			else if (!strncmp(p, "&#233;", 6)) { *q = 'é'; p += 5; }
			else if (!strncmp(p, "&#234;", 6)) { *q = 'ê'; p += 5; }
			else if (!strncmp(p, "&#235;", 6)) { *q = 'ë'; p += 5; }
			else if (!strncmp(p, "&#236;", 6)) { *q = 'ì'; p += 5; }
			else if (!strncmp(p, "&#237;", 6)) { *q = 'í'; p += 5; }
			else if (!strncmp(p, "&#238;", 6)) { *q = 'î'; p += 5; }
			else if (!strncmp(p, "&#239;", 6)) { *q = 'ï'; p += 5; }
			else if (!strncmp(p, "&#240;", 6)) { *q = 'ð'; p += 5; }
			else if (!strncmp(p, "&#241;", 6)) { *q = 'ñ'; p += 5; }
			else if (!strncmp(p, "&#242;", 6)) { *q = 'ò'; p += 5; }
			else if (!strncmp(p, "&#243;", 6)) { *q = 'ó'; p += 5; }
			else if (!strncmp(p, "&#244;", 6)) { *q = 'ô'; p += 5; }
			else if (!strncmp(p, "&#245;", 6)) { *q = 'õ'; p += 5; }
			else if (!strncmp(p, "&#246;", 6)) { *q = 'ö'; p += 5; }
			else if (!strncmp(p, "&#247;", 6)) { *q = '÷'; p += 5; }
			else if (!strncmp(p, "&#248;", 6)) { *q = 'ø'; p += 5; }
			else if (!strncmp(p, "&#249;", 6)) { *q = 'ù'; p += 5; }
			else if (!strncmp(p, "&#250;", 6)) { *q = 'ú'; p += 5; }
			else if (!strncmp(p, "&#251;", 6)) { *q = 'û'; p += 5; }
			else if (!strncmp(p, "&#252;", 6)) { *q = 'ü'; p += 5; }
			else if (!strncmp(p, "&#253;", 6)) { *q = 'ý'; p += 5; }
			else if (!strncmp(p, "&#254;", 6)) { *q = 'þ'; p += 5; }
			else if (!strncmp(p, "&#255;", 6)) { *q = 'ÿ'; p += 5; }
			else { *q = *p;	}
		}
		else {
			*q = *p;
		}
	}
	*q = '\0';
}

void LoadManifestInfo(char *buf) {
	char *p;

	if (p=GetManifestValue(buf, "type")) {
		if (!_strcmpi(p, "language")) {
			skinType = 1;
		}
		free(p);
	}
	if (p=GetManifestValue(buf, "name")) {
		szName = _strdup(p);
		XMLDecode(szName);
		free(p);
	}
	if (p=GetManifestValue(buf, "version")) {
		szVersion = _strdup(p);
		XMLDecode(szVersion);
		free(p);
	}
	if (p=GetManifestValue(buf, "author")) {
		szAuthor = _strdup(p);
		XMLDecode(szAuthor);
		free(p);
	}
}

int SetSkinAsDefault() {
	HKEY hkey;
	char szXMLNorm[MAX_PATH];

	if (szXML==NULL) return 0;
	strcpy(szXMLNorm,szName);
	strcat(szXMLNorm,"\\");
	strcat(szXMLNorm,szXML);

	if (ERROR_SUCCESS==RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Shareaza\\Shareaza\\Skins", 0, KEY_ALL_ACCESS, &hkey)) {
		int i = 0, needsKey = 1;
		char key[256];
		DWORD keys = sizeof(key);
		DWORD dval;

		for(i=0;; i++) {
			keys = sizeof(key);
			if (RegEnumValue(hkey, i, key, &keys, NULL, NULL, NULL, NULL)!=ERROR_SUCCESS) break;
			if (!_strcmpi(key, szXMLNorm)) {
				needsKey = 0;
				dval = 1;
				RegSetValueEx(hkey,key,0,REG_DWORD,(LPBYTE)&dval,sizeof(DWORD));
			}
			else if (strlen(key)) {
				if (CheckManifestForSkin(key)) {
					dval = 0;
					RegSetValueEx(hkey,key,0,REG_DWORD,(LPBYTE)&dval,sizeof(DWORD));
				}
			}
		}
		if (needsKey) {
			DWORD dval = 1;
			RegSetValueEx(hkey,szXMLNorm,0,REG_DWORD,(LPBYTE)&dval,sizeof(DWORD));
		}
		RegCloseKey(hkey);
	}
	else {
		free(szXMLNorm);
		return 0;
	}
	free(szXMLNorm);
	return 1;
}

int MakeDirectory(char *newdir) {
	char *buffer;
	char *p;
	int len = strlen(newdir);

	if (len<=0) return 0;
	buffer = (char*)malloc(len+1);
	strcpy(buffer,newdir);
	if (buffer[len-1]=='/') {
		buffer[len-1] = '\0';
	}
	if (_mkdir(buffer)==0) {
		free(buffer);
		return 1;
    }
	p = buffer+1;
	while(1) {
		char hold;
		while(*p&&*p!='\\'&&*p!='/') p++;
		hold = *p;
		*p = 0;
		if ((_mkdir(buffer)==-1) && (errno==ENOENT)) {
			free(buffer);
			return 0;
        }
		if (hold==0)
			break;
		*p++ = hold;
    }
	free(buffer);
	return 1;
}
// EXPORT END

static char* GetManifestValue(char *manifest, char *searchKey) {
	char *p;
	char *kstart, *vstart;
	int klen, vlen;
	char *key;
	char *val;
	char *info = _strdup(manifest);
	char *ret;

	if (p = strstr(info, "/>")) {
		info += 10;
		*p = '\0';
		for (p=info;;) {
			for (;*p!='\0' && (*p==' ' || *p=='\t'); p++);
			if (*p == '\0')
				break;
			kstart = p;
			for (;*p!='\0' && *p!='=' && *p!=' ' && *p!='\t'; p++);
			klen = p-kstart;
			for (;*p!='\0' && (*p==' ' || *p=='\t'); p++);
			if (*p == '\0') break;
			if (*p != '=') continue;
			p++;
			for (;*p!='\0' && (*p==' ' || *p=='\t'); p++);
			if (*p == '\0') {
				break;
			}
			if (*p=='\'' || *p=='"') {
				p++;
				vstart = p;
				for (;*p!='\0' && *p!=*(vstart-1); p++);
				vlen = p-vstart;
				if (*p != '\0') p++;
			}
			else {
				vstart = p;
				for (;*p!='\0' && *p!=' ' && *p!='\t'; p++);
				vlen = p-vstart;
			}
			key = (char *) malloc(klen+1);
			strncpy(key, kstart, klen);
			key[klen] = '\0';
			val = (char *) malloc(vlen+1);
			strncpy(val, vstart, vlen);
			val[vlen] = '\0';
			if (klen && !_strcmpi(key, searchKey)) {
				ret = _strdup(val);
				free(key);
				free(val);
				free(info);
				return ret;
			}
			free(key);
			free(val);
		}
	}
	free(info);
	return NULL;
}

static int CheckManifestForSkin(char *szFile) {
	char modDir[MAX_PATH], *tmp;
	FILE * pFile;
	long lSize;
	char *buffer,*tt, *val;
	int skin = 0;
	GetModuleFileName(NULL,modDir,sizeof(modDir));
	tmp=strrchr(modDir,'\\');
	if (tmp) *tmp=0;
	SetCurrentDirectory(modDir);

	pFile = fopen(szFile , "rb");
	if (pFile==NULL) return 0;

	fseek(pFile , 0 , SEEK_END);
	lSize = ftell(pFile);
	rewind(pFile);

	buffer = (char*) malloc(lSize);
	if (buffer == NULL) return 0;
	fread(buffer,1,lSize,pFile);
	if (!(tt=strstr(buffer, "<manifest"))) {
		fclose(pFile);
		free(buffer);
		return 1;
	}
	
	val = GetManifestValue(tt, "type");
	if (!val) skin = 1;
	else {
		if (!_strcmpi(val, "skin")) skin = 1;
		free(val);
	}
	fclose(pFile);
	free(buffer);
	return skin;
}
