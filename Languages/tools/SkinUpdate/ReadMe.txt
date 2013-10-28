SkinUpdate 1.0
Test resources for validity, extracts string table from RC-file and saves result as XML-file.

Usage:
	SkinUpdate.exe input.h input.rc output.xml

Example for Shareaza resources:
	SkinUpdate.exe ..\..\..\shareaza\resource.h "$(VCInstallDir)\atlmfc\include\afxres.h" ..\..\..\shareaza\shareaza.rc default-en.xml
