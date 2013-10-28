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

class ATL_NO_VTABLE CRarHandler
{
public:
	inline CRarHandler( RAROpenArchiveDataEx* oad ) throw()
	{
		hArchive = fnRAROpenArchiveEx( oad );
	}

	inline ~CRarHandler() throw()
	{
		if ( hArchive )
		{
			fnRARCloseArchive( hArchive );
			hArchive = NULL;
		}
	}

	inline operator HANDLE() const throw()
	{
		return hArchive;
	}

protected:
	HANDLE hArchive;
};

STDMETHODIMP CRARBuilder::Process (
	/* [in] */ BSTR sFile,
	/* [in] */ ISXMLElement* pXML)
{
	if ( ! pXML )
		return E_POINTER;

	if ( ! fnRAROpenArchiveEx || ! fnRARCloseArchive || ! fnRARReadHeaderEx || ! fnRARProcessFileW )
		// Unrar.dll not loaded
		return E_NOTIMPL;

	CComPtr <ISXMLElements> pISXMLRootElements;
	HRESULT hr = pXML->get_Elements(&pISXMLRootElements);
	if ( FAILED( hr ) )
		return hr;
	CComPtr <ISXMLElement> pXMLRootElement;
	hr = pISXMLRootElements->Create (CComBSTR( L"archives" ), &pXMLRootElement);
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

	char szCmtBuf[ MAX_SIZE_COMMENTS ] = {};
	RAROpenArchiveDataEx oad = {};
	oad.ArcNameW = sFile;
	oad.CmtBuf = szCmtBuf;
	oad.CmtBufSize = sizeof( szCmtBuf );
	oad.OpenMode = RAR_OM_LIST;

	CRarHandler oArchive( &oad );
	CAtlMap< CString, CString > oFolderList;

	switch( oad.OpenResult )
	{
	// Success
	case ERAR_SUCCESS:
		switch( oad.CmtState )
		{
		// Comments not present
		case ERAR_SUCCESS:
			break;

		// Comments read completely
		case 1:

		// Buffer too small, comments not completely read
		case ERAR_SMALL_BUF:
			szCmtBuf[ MAX_SIZE_COMMENTS - 1 ] = _T('\0');
			strComment = szCmtBuf;
			strComment.Replace( _T('\r'), _T(' ') );
			strComment.Replace( _T('\n'), _T(' ') );
			strComment.Replace( _T("  "), _T(" ") );
			break;

		// Not enough memory to extract comments
		case ERAR_NO_MEMORY:
			return E_OUTOFMEMORY;

		// Broken comment
		case ERAR_BAD_DATA:
		// Unknown comment format
		case ERAR_UNKNOWN_FORMAT:
			// Bad format. Call CLibraryBuilder::SubmitCorrupted()
			return E_UNEXPECTED;

		// Other errors
		default:
			return E_FAIL;
		}

		if ( ( oad.Flags & RAR_HEAD_ENCRYPTED ) )
			// Block headers are encrypted
			bEncrypted = true;

		// List all files
		for( int nResult = ERAR_SUCCESS; nResult == ERAR_SUCCESS; )
		{
			RARHeaderDataEx hd = {};  
			nResult = fnRARReadHeaderEx( oArchive, &hd );
			switch ( nResult )
			{
			// Success
			case ERAR_SUCCESS:
			{
				if ( ( hd.Flags & RAR_FILE_ENCRYPTED ) )
					// File is encrypted
					bEncrypted = true;

				bool bFolder = false;
				CString strName( hd.FileNameW );

				// Get golder names from paths
				for ( int i = 0;; )
				{
					CString strPart = strName.Tokenize( _T("\\"), i );
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
				int nBackSlashPos = strName.ReverseFind( _T('\\') );
				if ( nBackSlashPos == strName.GetLength() - 1 )
				{
					bFolder = true;
					strName = strName.Left( nBackSlashPos );
					nBackSlashPos = strName.ReverseFind( _T('\\') );
				}
				if ( nBackSlashPos >= 0 )
					strName = strName.Mid( nBackSlashPos + 1 );

				if ( ( hd.Flags & RAR_FILE_DIRECTORY ) == RAR_FILE_DIRECTORY )
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

					nUnpackedSize += hd.UnpSize;
				}
				break;
			}

			// End of archive
			case ERAR_END_ARCHIVE:
				break;

			// File header broken
			case ERAR_BAD_DATA:
				// Bad format. Call CLibraryBuilder::SubmitCorrupted()
				return E_UNEXPECTED;

			// Decryption errors (wrong password)
			case ERAR_UNKNOWN:
				break;

			// Other errors
			default:
				return E_FAIL;
			}

			if ( nResult != ERAR_SUCCESS )
				break;

			nResult = fnRARProcessFileW( oArchive, RAR_SKIP, NULL, NULL );
			switch ( nResult )
			{
			// Success
			case ERAR_SUCCESS:
				break;

			// File CRC error
			case ERAR_BAD_DATA:
			// Volume is not valid RAR archive
			case ERAR_BAD_ARCHIVE:			
			// Unknown archive format
			case ERAR_UNKNOWN_FORMAT:
				// Bad format. Call CLibraryBuilder::SubmitCorrupted()
				return E_UNEXPECTED;
			
			// Volume open error (volume missing)
			case ERAR_EOPEN:
				break;

			// File create error
			case ERAR_ECREATE:
			// File close error
			case ERAR_ECLOSE:
			// Read error
			case ERAR_EREAD:
			// Write error
			case ERAR_EWRITE:
			// Other errors
			default:
				return E_FAIL;
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
			CString strUnpackedSize;
			strUnpackedSize.Format( _T("%I64u"), nUnpackedSize );
			pISXMLAttributes->Add( CComBSTR( L"unpackedsize" ), CComBSTR( strUnpackedSize ) );
		}

		return S_OK;

	// Not enough memory to initialize data structures
	case ERAR_NO_MEMORY:
		return E_OUTOFMEMORY;

	// Archive header broken
	case ERAR_BAD_DATA:
	// File is not valid RAR archive
	case ERAR_BAD_ARCHIVE:
	// Unknown encryption used for archive headers
	case ERAR_UNKNOWN_FORMAT:
		// Bad format. Call CLibraryBuilder::SubmitCorrupted()
		return E_UNEXPECTED;

	// File open error
	case ERAR_EOPEN:
	// Other errors
	default:
		return E_FAIL;
	}
}
