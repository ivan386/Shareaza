//
// Class.cpp : Implementation of CClass
//
// Copyright (c) Shareaza Development Team, 2007-2013.
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
#include "Class.h"

#define MAX_SIZE_FILES		128
#define MAX_SIZE_FOLDERS	128
#define MAX_SIZE_COMMENTS	256

class ATL_NO_VTABLE CZipHandler
{
public:
	inline CZipHandler( LPCTSTR szFilename ) throw()
	{
		hArchive = unzOpen( CT2CA( szFilename ) );
		if ( ! hArchive )
		{
			TCHAR szFileShort[ MAX_PATH ];
			if ( GetShortPathName( szFilename, szFileShort, MAX_PATH ) )
				hArchive = unzOpen( CT2CA( szFileShort ) );
		}
	}

	inline ~CZipHandler() throw()
	{
		if ( hArchive )
		{
			unzClose( hArchive );
			hArchive = NULL;
		}
	}

	inline operator unzFile() const throw()
	{
		return hArchive;
	}

protected:
	unzFile hArchive;
};

STDMETHODIMP CZIPBuilder::Process (
	/* [in] */ BSTR sFile,
	/* [in] */ ISXMLElement* pXML)
{
	if ( ! pXML )
		return E_POINTER;

	CComPtr <ISXMLElements> pISXMLRootElements;
	HRESULT hr = pXML->get_Elements(&pISXMLRootElements);
	if ( FAILED( hr ) )
		return hr;
	CComPtr <ISXMLElement> pXMLRootElement;
	hr = pISXMLRootElements->Create (CComBSTR ("archives"), &pXMLRootElement);
	if ( FAILED( hr ) )
		return hr;
	CComPtr <ISXMLAttributes> pISXMLRootAttributes;
	hr = pXMLRootElement->get_Attributes(&pISXMLRootAttributes);
	if ( FAILED( hr ) )
		return hr;
	pISXMLRootAttributes->Add( CComBSTR( L"xmlns:xsi"), CComBSTR( L"http://www.w3.org/2001/XMLSchema-instance" ) );
	pISXMLRootAttributes->Add( CComBSTR( L"xsi:noNamespaceSchemaLocation" ), CComBSTR( L"http://www.shareaza.com/schemas/archive.xsd" ) );

	CComPtr <ISXMLElements> pISXMLElements;
	hr = pXMLRootElement->get_Elements(&pISXMLElements);
	if ( FAILED( hr ) )
		return hr;
	CComPtr <ISXMLElement> pXMLElement;
	hr = pISXMLElements->Create (CComBSTR( L"archive" ), &pXMLElement);
	if ( FAILED( hr ) )
		return hr;
	CComPtr <ISXMLAttributes> pISXMLAttributes;
	hr = pXMLElement->get_Attributes(&pISXMLAttributes);
	if ( FAILED( hr ) )
		return hr;

	CString strFiles;				// Plain list of archive files
	bool bMoreFiles = false;		// More files than listed in sFiles
	CString strFolders;				// Plain list of archive folders
	bool bMoreFolders = false;		// More folders than listed in sFolders
	CString strComment;				// Archive comments
	bool bEncrypted = false;		// Archive itself or selective files are encrypted
	ULONGLONG nUnpackedSize = 0;	// Total size of unpacked files

	USES_CONVERSION;
	CZipHandler pFile( OLE2CT( sFile ) );
	if ( ! pFile )
		return E_FAIL;

	unz_global_info pDir = {};
	if ( unzGetGlobalInfo( pFile, &pDir ) != UNZ_OK )
	{
		// Bad format. Call CLibraryBuilder::SubmitCorrupted()
		return E_UNEXPECTED;
	}

	if ( pDir.size_comment )
	{
		char szCmtBuf[ MAX_SIZE_COMMENTS ];
		int nResult = unzGetGlobalComment( pFile, szCmtBuf, MAX_SIZE_COMMENTS );
		if ( nResult < 0 )
		{
			// Bad format. Call CLibraryBuilder::SubmitCorrupted()
			return E_UNEXPECTED;
		}
		else
		{
			szCmtBuf[ MAX_SIZE_COMMENTS - 1 ] = _T('\0');
			strComment = szCmtBuf;
			strComment.Replace( _T('\r'), _T(' ') );
			strComment.Replace( _T('\n'), _T(' ') );
			strComment.Replace( _T("  "), _T(" ") );
		}
	}

	CAtlMap< CString, CString > oFolderList;

	for ( UINT nFile = 0; nFile < pDir.number_entry; nFile++ )
	{
		if ( nFile && unzGoToNextFile( pFile ) != UNZ_OK )
		{
			// Bad format. Call CLibraryBuilder::SubmitCorrupted()
			return E_UNEXPECTED;
		}

		unz_file_info pInfo = {};
		CHAR szFile[ MAX_PATH ] = {};
		if ( unzGetCurrentFileInfo( pFile, &pInfo, szFile,
			MAX_PATH, NULL, 0, NULL, 0 ) != UNZ_OK )
		{
			// Bad format. Call CLibraryBuilder::SubmitCorrupted()
			return E_UNEXPECTED;
		}
		OemToCharA( szFile, szFile );

		if ( ( pInfo.flag & 0x01 ) )
			bEncrypted = true;

		bool bFolder = false;
		CString strName( szFile );

		// Get golder names from paths
		for ( int i = 0;; )
		{
			CString strPart = strName.Tokenize( _T("/"), i );
			if ( strPart.IsEmpty() )
				break;
			if ( i + 1 >= strName.GetLength() )
			{
				// Last part
				break;
			}
			else
			{
				CString strPartLC = strPart;
				strPartLC.MakeLower();
				oFolderList.SetAt( strPartLC, strPart );
			}
		}
		int nBackSlashPos = strName.ReverseFind( _T('/') );
		if ( nBackSlashPos == strName.GetLength() - 1 )
		{
			bFolder = true;
			strName = strName.Left( nBackSlashPos );
			nBackSlashPos = strName.ReverseFind( _T('/') );
		}
		if ( nBackSlashPos >= 0 )
			strName = strName.Mid( nBackSlashPos + 1 );

		if ( ( pInfo.external_fa & FILE_ATTRIBUTE_DIRECTORY ) )
			bFolder = true;

		if ( bFolder )
		{
			CString strPartLC = strName;
			strPartLC.MakeLower();
			oFolderList.SetAt( strPartLC, strName );
		}
		else
		{
			if ( strFiles.GetLength() + strName.GetLength() <= MAX_SIZE_FILES - 5 )
			{
				if ( strFiles.GetLength() )
					strFiles += _T(", ");
				strFiles += strName;
			}
			else
				bMoreFiles = true;

			nUnpackedSize += pInfo.uncompressed_size;
		}
	}

	for ( POSITION pos = oFolderList.GetStartPosition(); pos; )
	{
		CString strName = oFolderList.GetNextValue( pos );
		if ( strFolders.GetLength() + strName.GetLength() <= MAX_SIZE_FOLDERS - 5 )
		{
			if ( strFolders.GetLength() )
				strFolders += _T(", ");
			strFolders += strName;
		}
		else
		{
			bMoreFolders = true;
			break;
		}
	}

	if ( strFiles.GetLength() )
	{
		if ( bMoreFiles )
			strFiles += _T(", ...");
		pISXMLAttributes->Add( CComBSTR( L"files" ), CComBSTR( strFiles ) );
	}

	if ( strFolders.GetLength() )
	{
		if ( bMoreFolders )
			strFolders += _T(", ...");
		pISXMLAttributes->Add( CComBSTR( L"folders" ), CComBSTR( strFolders ) );
	}

	if ( strComment.GetLength() )
		pISXMLAttributes->Add( CComBSTR( L"comments" ), CComBSTR( strComment ) );

	if ( bEncrypted )
		pISXMLAttributes->Add( CComBSTR( L"encrypted" ), CComBSTR( L"true" ) );

	if ( nUnpackedSize )
	{
		CString strTmp;
		strTmp.Format( _T("%I64u"), nUnpackedSize );
		pISXMLAttributes->Add( CComBSTR( L"unpackedsize" ), CComBSTR( strTmp ) );
	}

	return S_OK;
}
