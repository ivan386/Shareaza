//
// Class.cpp : Implementation of CClass
//
// Copyright (c) Shareaza Development Team, 2007.
// This file is part of SHAREAZA (www.shareaza.com)
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

#define MAX_SIZE_FILES		80
#define MAX_SIZE_FOLDERS	80
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
	pISXMLRootAttributes->Add (CComBSTR ("xmlns:xsi"),
		CComBSTR ("http://www.w3.org/2001/XMLSchema-instance"));
	pISXMLRootAttributes->Add (CComBSTR ("xsi:noNamespaceSchemaLocation"),
		CComBSTR ("http://www.shareaza.com/schemas/archive.xsd"));

	CComPtr <ISXMLElements> pISXMLElements;
	hr = pXMLRootElement->get_Elements(&pISXMLElements);
	if ( FAILED( hr ) )
		return hr;
	CComPtr <ISXMLElement> pXMLElement;
	hr = pISXMLElements->Create (CComBSTR ("archive"), &pXMLElement);
	if ( FAILED( hr ) )
		return hr;
	CComPtr <ISXMLAttributes> pISXMLAttributes;
	hr = pXMLElement->get_Attributes(&pISXMLAttributes);
	if ( FAILED( hr ) )
		return hr;

	CString sFiles;					// Plain list of archive files
	bool bMoreFiles = false;		// More files than listed in sFiles
	CString sFolders;				// Plain list of archive folders
	bool bMoreFolders = false;		// More folders than listed in sFolders
	CString sComment;				// Archive comments
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
			szCmtBuf[ MAX_SIZE_COMMENTS - 1 ] = '\0';
			sComment = szCmtBuf;
			sComment.Replace( _T('\r'), _T(' ') );
			sComment.Replace( _T('\n'), _T(' ') );
			sComment.Replace( _T("  "), _T(" ") );
		}
	}

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

		CString sName( szFile );
		int n = sName.ReverseFind( _T('/') );
		if ( n == sName.GetLength() - 1 )
		{
			bFolder = true;
			sName = sName.Left( n );
			n = sName.ReverseFind( _T('/') );
		}
		if ( n >= 0 )
			sName = sName.Mid( n + 1 );

		if ( ( pInfo.external_fa & FILE_ATTRIBUTE_DIRECTORY ) )
			bFolder = true;

		if ( bFolder )
		{
			if ( sFolders.GetLength() + sName.GetLength() <= MAX_SIZE_FOLDERS - 5 )
			{
				if ( sFolders.GetLength() )
					sFolders += _T(", ");
				sFolders += sName;
			}
			else
				bMoreFolders = true;
		}
		else
		{
			if ( sFiles.GetLength() + sName.GetLength() <= MAX_SIZE_FILES - 5 )
			{
				if ( sFiles.GetLength() )
					sFiles += _T(", ");
				sFiles += sName;
			}
			else
				bMoreFiles = true;

			nUnpackedSize += pInfo.uncompressed_size;
		}
	}

	if ( sFiles.GetLength() )
	{
		if ( bMoreFiles )
			sFiles += _T(", ...");
		pISXMLAttributes->Add( CComBSTR( "files" ), CComBSTR( sFiles ) );
	}

	if ( sFolders.GetLength() )
	{
		if ( bMoreFolders )
			sFolders += _T(", ...");
		pISXMLAttributes->Add( CComBSTR( "folders" ), CComBSTR( sFolders ) );
	}

	if ( sComment.GetLength() )
		pISXMLAttributes->Add( CComBSTR( "comments" ), CComBSTR( sComment ) );

	if ( bEncrypted )
		pISXMLAttributes->Add( CComBSTR( "encrypted" ), CComBSTR( "true" ) );

	if ( nUnpackedSize )
	{
		CString sTmp;
		sTmp.Format( _T("%I64u"), nUnpackedSize );
		pISXMLAttributes->Add( CComBSTR( "unpackedsize" ), CComBSTR( sTmp ) );
	}

	return S_OK;
}
