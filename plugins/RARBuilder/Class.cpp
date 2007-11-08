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
	/* [in] */ HANDLE /* hFile */,
	/* [in] */ BSTR sFile,
	/* [in] */ ISXMLElement* pXML)
{
	if ( ! pXML )
		return E_POINTER;

	if ( ! fnRAROpenArchiveEx || ! fnRARCloseArchive || ! fnRARReadHeaderEx ||
		! fnRARProcessFileW )
		// Unrar.dll not loaded
		return E_NOTIMPL;

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

	char szCmtBuf[ MAX_SIZE_COMMENTS ] = {};
	RAROpenArchiveDataEx oad = {};
	oad.ArcNameW = sFile;
	oad.CmtBuf = szCmtBuf;
	oad.CmtBufSize = sizeof( szCmtBuf );
	oad.OpenMode = RAR_OM_LIST;

	CRarHandler oArchive( &oad );

	switch( oad.OpenResult )
	{
	// Success
	case 0:
		switch( oad.CmtState )
		{
		// Comments not present
		case 0:
			break;

		// Comments read completely
		case 1:

		// Buffer too small, comments not completely read
		case ERAR_SMALL_BUF:
			szCmtBuf[ MAX_SIZE_COMMENTS - 1 ] = '\0';
			sComment = szCmtBuf;
			sComment.Replace( _T('\r'), _T(' ') );
			sComment.Replace( _T('\n'), _T(' ') );
			sComment.Replace( _T("  "), _T(" ") );
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

		if ( ( oad.Flags & 0x0080 ) )
			// Block headers are encrypted
			bEncrypted = true;

		// List all files
		for( int nResult = 0; nResult == 0; )
		{
			RARHeaderDataEx hd = {};  
			nResult = fnRARReadHeaderEx( oArchive, &hd );
			switch ( nResult )
			{
			// Success
			case 0:
			{
				CString sName( hd.FileNameW );
				int n = sName.ReverseFind( _T('\\') );
				if ( n >= 0 )
					sName = sName.Mid( n + 1 );

				if ( ( hd.Flags & 0xe0 ) == 0xe0 )
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

					nUnpackedSize += hd.UnpSize;

					if ( ( hd.Flags & 0x04 ) )
						// File encrypted with password
						bEncrypted = true;
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

			// Decryption errors
			case ERAR_UNKNOWN:
				break;

			// Other errors
			default:
				return E_FAIL;
			}

			if ( nResult != 0 )
				break;

			nResult = fnRARProcessFileW( oArchive, RAR_SKIP, NULL, NULL );
			switch ( nResult )
			{
			// Success
			case 0:
				break;

			// File CRC error
			case ERAR_BAD_DATA:
			// Volume is not valid RAR archive
			case ERAR_BAD_ARCHIVE:			
			// Unknown archive format
			case ERAR_UNKNOWN_FORMAT:
				// Bad format. Call CLibraryBuilder::SubmitCorrupted()
				return E_UNEXPECTED;
			
			// Volume open error
			case ERAR_EOPEN:
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
