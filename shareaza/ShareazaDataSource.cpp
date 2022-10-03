//
// ShareazaDataSource.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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
#include "Shareaza.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "AlbumFolder.h"
#include "Library.h"
#include "LibraryList.h"
#include "LibraryFolders.h"
#include "CtrlLibraryTreeView.h"
#include "ShareazaDataSource.h"
#include "SchemaCache.h"

#include "HGlobal.h"
#include "StreamArchive.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG

static LPCTSTR GetFORMATLIST(UINT id)
{
	const struct {
		UINT id;
		LPCTSTR name;
	} FORMATLIST [] = {
		{ CF_NULL,            _T("CF_NULL") },
		{ CF_TEXT,            _T("CF_TEXT") },
		{ CF_BITMAP,          _T("CF_BITMAP") },
		{ CF_METAFILEPICT,    _T("CF_METAFILEPICT") },
		{ CF_SYLK,            _T("CF_SYLK") },
		{ CF_DIF,             _T("CF_DIF") },
		{ CF_TIFF,            _T("CF_TIFF") },
		{ CF_OEMTEXT,         _T("CF_OEMTEXT") },
		{ CF_DIB,             _T("CF_DIB") },
		{ CF_PALETTE,         _T("CF_PALETTE") },
		{ CF_PENDATA,         _T("CF_PENDATA") },
		{ CF_RIFF,            _T("CF_RIFF") },
		{ CF_WAVE,            _T("CF_WAVE") },
		{ CF_UNICODETEXT,     _T("CF_UNICODETEXT") },
		{ CF_ENHMETAFILE,     _T("CF_ENHMETAFILE") },
		{ CF_HDROP,           _T("CF_HDROP") },
		{ CF_LOCALE,          _T("CF_LOCALE") },
		{ CF_DIBV5,           _T("CF_DIBV5") },
		{ CF_OWNERDISPLAY,    _T("CF_OWNERDISPLAY") },
		{ CF_DSPTEXT,         _T("CF_DSPTEXT") },
		{ CF_DSPBITMAP,       _T("CF_DSPBITMAP") },
		{ CF_DSPMETAFILEPICT, _T("CF_DSPMETAFILEPICT") },
		{ CF_DSPENHMETAFILE,  _T("CF_DSPENHMETAFILE") },
		{ 0, NULL }
	};
	static TCHAR buf [256] = { 0 };
	for ( int i = 0; FORMATLIST [i].name; i++ )
	{
		if ( FORMATLIST [i].id == id )
			return FORMATLIST [i].name;
	}
	if ( ! GetClipboardFormatName( id, buf, _countof( buf ) ) )
		_stprintf_s( buf, _countof( buf ), _T("0x%x"), id );
	return buf;
}

void DumpIDataObject(IDataObject* pIDataObject)
{
	CComPtr< IEnumFORMATETC > pIEnumFORMATETC;
	if ( SUCCEEDED( pIDataObject->EnumFormatEtc( DATADIR_GET, &pIEnumFORMATETC ) ) )
	{
		TRACE( _T("IDataObject = {\n") );
		pIEnumFORMATETC->Reset();
		for (;;)
		{
			FORMATETC formatetc = {};
			ULONG celtFetched = 0;
			if ( pIEnumFORMATETC->Next( 1, &formatetc, &celtFetched ) != S_OK )
				break;
			TRACE( _T("\t{%s, %d, %d, 0x%08x, %d}\n"),
				GetFORMATLIST( formatetc.cfFormat ), formatetc.dwAspect, formatetc.lindex,
				formatetc.ptd, formatetc.tymed );
		}
		TRACE( _T("}\n") );
	}
}

#endif	// _DEBUG

// STGMEDIUM wrapper
class __declspec(novtable) CStgMedium : public STGMEDIUM
{
public:
	CStgMedium() : STGMEDIUM()
	{
	}
	~CStgMedium()
	{
		ReleaseStgMedium( static_cast< STGMEDIUM* >( this ) );
	}
};

typedef struct sAsyncFileOperationParams {
	HWND			hWnd;		// Main window handle
	DWORD			dwEffect;	// Drop effect
	CArray<WCHAR>	sFrom;		// Sources list (double NULL terminated)
	CArray<WCHAR>	sTo;		// Destination path (double NULL terminated)
} AsyncFileOperationParams;

UINT AsyncFileOperationThread(LPVOID param)
{
	ASSERT( param != NULL );

	AsyncFileOperationParams* pAFOP = (AsyncFileOperationParams*)param;

	bool bCopy = (pAFOP->dwEffect == DROPEFFECT_COPY);

	// Shell file operations
	SHFILEOPSTRUCT sFileOp = {
		pAFOP->hWnd,
		(UINT)( bCopy ? FO_COPY : FO_MOVE ),
		pAFOP->sFrom.GetData(),
		pAFOP->sTo.GetData(),
		FOF_ALLOWUNDO,
		FALSE,
		NULL,
		NULL
	};
	SHFileOperation( &sFileOp );

	// Notify Shell about changes (first file/folder only)
	if ( ! bCopy )
	{
		CString sFromDir = pAFOP->sFrom.GetData();
		int nSlash = sFromDir.ReverseFind( _T('\\') );
		if ( nSlash != -1 )
			sFromDir = sFromDir.Left( nSlash );
		SHChangeNotify( SHCNE_UPDATEDIR, SHCNF_PATH, (LPCVOID)(LPCTSTR)sFromDir, 0 );
	}
	SHChangeNotify( SHCNE_UPDATEDIR, SHCNF_PATH, (LPCVOID)(LPCTSTR)pAFOP->sTo.GetData(), 0 );

	Library.Update( true );

	delete pAFOP;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Helper for implementing OLE enumerators
//
// Note: Implementation of this class resides inside MFC library
//

#undef  INTERFACE
#define INTERFACE   IEnumVOID

DECLARE_INTERFACE_(IEnumVOID, IUnknown)
{
	STDMETHOD(QueryInterface)(REFIID, LPVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)()  PURE;
	STDMETHOD_(ULONG,Release)() PURE;
	STDMETHOD(Next)(ULONG, void*, ULONG*) PURE;
	STDMETHOD(Skip)(ULONG) PURE;
	STDMETHOD(Reset)() PURE;
	STDMETHOD(Clone)(IEnumVOID**) PURE;
};

class CEnumArray : public CCmdTarget
{
public:
	CEnumArray(size_t nSize, const void* pvEnum, UINT nCount, BOOL bNeedFree = FALSE);
	virtual ~CEnumArray();

protected:
	size_t m_nSizeElem;			// size of each item in the array
	CCmdTarget* m_pClonedFrom;	// used to keep original alive for clones

	BYTE* m_pvEnum;				// pointer data to enumerate
	UINT m_nCurPos;				// current position in m_pvEnum
	UINT m_nSize;				// total number of items in m_pvEnum
	BOOL m_bNeedFree;			// free on release?

	virtual BOOL OnNext(void* pv);
	virtual BOOL OnSkip();
	virtual void OnReset();
	virtual CEnumArray* OnClone();

public:
	BEGIN_INTERFACE_PART(EnumVOID, IEnumVOID)
		INIT_INTERFACE_PART(CEnumArray, EnumVOID)
		STDMETHOD(Next)(ULONG, void*, ULONG*);
		STDMETHOD(Skip)(ULONG);
		STDMETHOD(Reset)();
		STDMETHOD(Clone)(IEnumVOID**);
	END_INTERFACE_PART(EnumVOID)
};

/////////////////////////////////////////////////////////////////////////////
// CEnumFormatEtc - enumerator for array for FORMATETC structures
//
// Note: Implementation of this class resides inside MFC library
//

class CEnumFormatEtc : public CEnumArray
{
public:
	CEnumFormatEtc();
	virtual ~CEnumFormatEtc();

	void AddFormat(const FORMATETC* lpFormatEtc);

protected:
	UINT m_nMaxSize;    // number of items allocated (>= m_nSize)

	virtual BOOL OnNext(void* pv);

	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CShareazaDataSource

IMPLEMENT_DYNAMIC(CShareazaDataSource, CComObject)

// {34791E02-51DC-4CF4-9E34-018166D91D0E}
IMPLEMENT_OLECREATE_FLAGS(CShareazaDataSource, CLIENT_NAME _T(".DataSource"),
	afxRegFreeThreading|afxRegApartmentThreading,
	0x34791e02, 0x51dc, 0x4cf4, 0x9e, 0x34, 0x1, 0x81, 0x66, 0xd9, 0x1d, 0xe)

CShareazaDataSource::CShareazaDataSource() :
	m_rgde (NULL ),
	m_cde ( 0 )
{
	CoCreateInstance( CLSID_DragDropHelper, NULL, CLSCTX_ALL,
		IID_IDragSourceHelper, (LPVOID*) &m_pdsh );
}

CShareazaDataSource::~CShareazaDataSource()
{
	Clean();

	m_pdsh.Release();
}

void CShareazaDataSource::Clean()
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( m_rgde )
	{
		for ( int ide = 0; ide < m_cde; ide++ )
		{
			if ( m_rgde[ide].fe.ptd )
			{
				CoTaskMemFree( m_rgde[ide].fe.ptd );
				m_rgde[ide].fe.ptd = NULL;
			}
			ReleaseStgMedium( &m_rgde[ide].stgm );
		}
		CoTaskMemFree( m_rgde );
		m_rgde = NULL;
	}
	m_cde = 0;
}

template < typename T >
UINT CShareazaDataSource::DragDropThread(LPVOID param)
{
	DWORD dwCurrentThreadID = GetCurrentThreadId();

	// Get thread ID's
	HWND hwndAttach	= AfxGetMainWnd()->GetSafeHwnd();
	DWORD dwAttachThreadID = GetWindowThreadProcessId( hwndAttach, NULL );

	// Attach input queues if necessary
	if ( dwAttachThreadID != dwCurrentThreadID )
		AttachThreadInput( dwAttachThreadID, dwCurrentThreadID, TRUE );

	{
		CComPtr< IDataObject > pIDataObject;
		HRESULT hr = CoGetInterfaceAndReleaseStream( (IStream*)param, IID_IDataObject,
			(LPVOID*)&pIDataObject );
		if ( SUCCEEDED( hr ) )
		{
			// Create drag-n-drop source object							
			// TODO: next line returns E_NOINTERFACE for unknown reason
			// CComQIPtr< IDropSource > pIDropSource( pIDataObject );
			// therefore we used some hack since IDropSource object is
			// not IDataObject dependent:
			CShareazaDataSource foo;
			IDropSource* pIDropSource = &(foo.m_xDropSource);

			DWORD dwEffect = DROPEFFECT_NONE;
			hr = ::DoDragDrop( pIDataObject, pIDropSource,
				DROPEFFECT_MOVE | DROPEFFECT_COPY, &dwEffect );
			ASSERT ( SUCCEEDED( hr ) );

			// TODO: need to detect unoptimized move and
			// delete dragged items
			ASSERT ( dwEffect != DROPEFFECT_MOVE );
		}
	}

	// Detach input queues
	if ( dwAttachThreadID != dwCurrentThreadID )
		AttachThreadInput( dwAttachThreadID, dwCurrentThreadID, FALSE );

	return 0;
}

// Perform CLibraryList drag operation

HRESULT CShareazaDataSource::DoDragDrop(const CLibraryList* pList, HBITMAP pImage, const Hashes::Guid& oGUID, const CPoint& ptOffset)
{
	return DoDragDropHelper < CLibraryList > (pList, pImage, oGUID, ptOffset);
}

// Perform CLibraryTreeItem drag operation

HRESULT CShareazaDataSource::DoDragDrop(const CLibraryTreeItem* pList, HBITMAP pImage, const Hashes::Guid& oGUID, const CPoint& ptOffset)
{
	return DoDragDropHelper < CLibraryTreeItem > (pList, pImage, oGUID, ptOffset);
}

// Perform universal drag operation

template < typename T >
HRESULT CShareazaDataSource::DoDragDropHelper(const T* pList, HBITMAP pImage, const Hashes::Guid& oGUID, const CPoint& ptOffset)
{
	HRESULT hr = E_FAIL;

	// Create drag-n-drop data object
	CShareazaDataSource* pSrc = new CShareazaDataSource;
	if ( ! pSrc )
		return E_OUTOFMEMORY;
	CComPtr< IDataObject > pIDataObject;
	pIDataObject.Attach( 
		 static_cast< IDataObject* >( pSrc->GetInterface( IID_IDataObject ) ) );
	if ( pIDataObject )
	{
		// Set raza flag to detect self drag-n-drop
		hr = Add( pIDataObject );
		if ( SUCCEEDED( hr ) )
		{
			// Pack file/folder/album list
			hr = AddFiles < T > ( pIDataObject, pList, oGUID );
			if ( SUCCEEDED( hr ) )
			{
				// Prepare IDragSourceHelper handler
				Add( pIDataObject, pImage, ptOffset );

				// Send data object to thread
				IStream* pStream = NULL;
				hr = CoMarshalInterThreadInterfaceInStream( IID_IDataObject,
					pIDataObject, &pStream);
				if ( SUCCEEDED( hr ) )
				{
					// Begin async drag-n-drop operation
					HANDLE hThread = CRazaThread::BeginThread( "DragDrop", DragDropThread<T>, (LPVOID)pStream );
					hr = ( hThread != NULL ) ? S_OK : E_FAIL;
				}
			}
		}
	}
	if ( pImage )
	{
		DeleteObject( pImage );
	}
	return hr;
}

// Get CFSTR_SHELLURL as string from data object

HRESULT CShareazaDataSource::ObjectToURL(IDataObject* pIDataObject, CString& str)
{
	ASSERT( pIDataObject != NULL );

	FORMATETC fmtc = { (CLIPFORMAT) RegisterClipboardFormat( CFSTR_SHELLURL ), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	CStgMedium medium;

	HRESULT hr = pIDataObject->GetData( &fmtc, &medium );
	if ( SUCCEEDED( hr ) )
	{
		hr = E_INVALIDARG;
		if ( medium.tymed == TYMED_HGLOBAL && medium.hGlobal != NULL )
		{
			LPCSTR psz = (LPCSTR)GlobalLock( medium.hGlobal );
			if ( psz )
			{
				str = psz;
				hr = str.IsEmpty() ? S_FALSE : S_OK;
				GlobalUnlock( medium.hGlobal );
			}
		}
	}

	return hr;
}

// Get CF_HDROP as string list from data object

HRESULT CShareazaDataSource::ObjectToFiles(IDataObject* pIDataObject, CList < CString >& oFiles)
{
	ASSERT( pIDataObject != NULL );

	FORMATETC fmtc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	CStgMedium medium;

	oFiles.RemoveAll();

	HRESULT hr = pIDataObject->GetData( &fmtc, &medium );
	if ( SUCCEEDED( hr ) )
	{
		hr = E_INVALIDARG;
		if ( medium.tymed == TYMED_HGLOBAL && medium.hGlobal != NULL )
		{
			HDROP hDropInfo = (HDROP) GlobalLock( medium.hGlobal );
			if ( hDropInfo )
			{
				UINT nCount = DragQueryFile( hDropInfo, 0xFFFFFFFF, NULL, 0 );
				for ( UINT nFile = 0 ; nFile < nCount ; nFile++ )
				{
					CString sFile;
					DragQueryFile( hDropInfo, nFile, sFile.GetBuffer( MAX_PATH ), MAX_PATH );
					sFile.ReleaseBuffer();

					if ( sFile.GetLength() > 4 && ! lstrcmpi( (LPCTSTR)sFile +
						sFile.GetLength() - 4, _T(".lnk") ) )
					{
						 sFile = ResolveShortcut( sFile );
					}

					if ( sFile.GetLength() )
					{
						oFiles.AddTail( sFile );
					}
				}
				hr = ( oFiles.GetCount() > 0 ) ? S_OK : S_FALSE;
				GlobalUnlock( medium.hGlobal );
			}
		}
	}
	return hr;
}

// Add CFSTR_PERFORMEDDROPEFFECT value to data object

HRESULT CShareazaDataSource::SetDropEffect(IDataObject* pIDataObject, DWORD dwEffect)
{
	ASSERT( pIDataObject != NULL );

	FORMATETC formatetc = { (CLIPFORMAT) RegisterClipboardFormat( CFSTR_PERFORMEDDROPEFFECT ), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM medium = { TYMED_HGLOBAL, NULL, NULL };

	CHGlobal < DWORD > oHGlobal;
	if ( ! oHGlobal.IsValid() )
	{
		return E_OUTOFMEMORY;
	}
	DWORD* pdwEffect = oHGlobal;

	*pdwEffect = dwEffect;

	medium.hGlobal = oHGlobal.Detach();

	return pIDataObject->SetData( &formatetc, &medium, TRUE );
}

// Perform basic file operation (copy or move)

BOOL CShareazaDataSource::DropToFolder(IDataObject* pIDataObject, DWORD grfKeyState, DWORD* pdwEffect, BOOL bDrop, LPCTSTR pszDest)
{
	if( ! pdwEffect )
		return FALSE;

	*pdwEffect = DROPEFFECT_NONE;
	
	if ( ( grfKeyState & MK_CONTROL ) && ( grfKeyState & MK_SHIFT ) )
		return FALSE;

	if( ! pIDataObject )
		return FALSE;

	FORMATETC fmtc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	if ( FAILED ( pIDataObject->QueryGetData( &fmtc ) ) )
		return FALSE;

	// Drop files to folder
	CStgMedium medium;
	if ( FAILED ( pIDataObject->GetData( &fmtc, &medium ) ) )
		return FALSE;

	SIZE_T size = GlobalSize( medium.hGlobal );
	if ( medium.tymed != TYMED_HGLOBAL || medium.hGlobal == NULL ||
		size < sizeof( DROPFILES ) || size > 10000000 )
		return FALSE;

	auto_ptr<AsyncFileOperationParams> pAFOP( new AsyncFileOperationParams );
	if ( ! pAFOP.get() )
		return FALSE;

	DROPFILES* pdf = (DROPFILES*)GlobalLock( medium.hGlobal );
	if ( ! pdf )
		return FALSE;

	int offset = 0, len;
	if ( ! pdf->fWide )
	{
		// ANSI
		for ( LPCSTR pFrom = (LPCSTR)( (char*)pdf + pdf->pFiles ); *pFrom; offset += len + 1, pFrom += len + 1 )
		{
			len = lstrlenA( pFrom );
			CStringW sFile( pFrom );	// ANSI -> UNICODE
			if ( len > 4 && ! lstrcmpiA( pFrom + len - 4, ".lnk" ) )
			{
				sFile = ResolveShortcut( sFile );
			}
			if ( sFile.GetLength() )
			{
				pAFOP->sFrom.SetSize( pAFOP->sFrom.GetSize() + sFile.GetLength() + 1 );
				CopyMemory( pAFOP->sFrom.GetData() + offset,
					(LPCWSTR)sFile, sizeof(WCHAR) * ( sFile.GetLength() + 1 ) );
			}
			if ( ! bDrop )
				// Test only one
				break;
		}
	}
	else
	{
		// UNICODE
		for ( LPCWSTR pFrom = (LPCWSTR)( (char*)pdf + pdf->pFiles ); *pFrom; offset += len + 1, pFrom += len + 1 )
		{ 
			len = lstrlenW( pFrom );
			if ( len > 4 && ! lstrcmpiW( pFrom + len - 4, L".lnk" ) )
			{
				CStringW sFile = ResolveShortcut( pFrom );
				if ( sFile.GetLength() )
				{
					pAFOP->sFrom.SetSize( pAFOP->sFrom.GetSize() + sFile.GetLength() + 1 );
					CopyMemory( pAFOP->sFrom.GetData() + offset,
						(LPCTSTR)sFile, sizeof(WCHAR) * ( sFile.GetLength() + 1 ) );
				}
			}
			else
			{
				pAFOP->sFrom.SetSize( pAFOP->sFrom.GetSize() + len + 1 );
				CopyMemory( pAFOP->sFrom.GetData() + offset,
					pFrom, sizeof(WCHAR) * ( len + 1 ) );
			}
			if ( ! bDrop )
				// Test only one
				break;
		}
	}
	GlobalUnlock( medium.hGlobal );

	// Check for "source == destination"
	{
		bool bFolder = ( GetFileAttributes( pAFOP->sFrom.GetData() ) & FILE_ATTRIBUTE_DIRECTORY ) != 0;
		LPCTSTR szPath2 = PathFindFileName( pAFOP->sFrom.GetData() );
		if ( szPath2 )
		{
			int nPath1Length = lstrlen( pszDest );
			if ( nPath1Length > 0 && pszDest[ nPath1Length - 1 ] == _T('\\') )
				nPath1Length--;
			int nPath2Length = bFolder ? lstrlen( pAFOP->sFrom.GetData() ) :
				(int)( szPath2 - pAFOP->sFrom.GetData() - 1 );
			if ( nPath1Length == nPath2Length &&
				_tcsncicmp( pszDest, pAFOP->sFrom.GetData(), nPath1Length ) == 0 )
				// source == destination
				return TRUE;
		}
		// else "virtual" drop
	}

	*pdwEffect = PathIsSameRoot( pszDest, pAFOP->sFrom.GetData() ) ?
		( ( grfKeyState & MK_CONTROL ) ? DROPEFFECT_COPY : DROPEFFECT_MOVE ) :
		( ( grfKeyState & MK_SHIFT   ) ? DROPEFFECT_MOVE : DROPEFFECT_COPY );

	if ( ! bDrop )
		// Test only
		return TRUE;

	len = (int)pAFOP->sFrom.GetSize();
	pAFOP->sFrom.SetSize( len + 1 );	// Double NULL terminated

	len = lstrlen( pszDest ) + 1;
	pAFOP->sTo.SetSize( len + 1 );		// Double NULL terminated
	CopyMemory( pAFOP->sTo.GetData(), pszDest, sizeof(WCHAR) * len );

	pAFOP->hWnd = AfxGetMainWnd()->GetSafeHwnd();

	pAFOP->dwEffect = *pdwEffect;

	HANDLE hThread = CRazaThread::BeginThread( "SHFileOperation", AsyncFileOperationThread, (LPVOID)pAFOP.release() );
	if ( hThread == NULL )
		return FALSE;

	if ( *pdwEffect == DROPEFFECT_MOVE )
	{
		// Optimized move used
		*pdwEffect = DROPEFFECT_NONE;
		SetDropEffect( pIDataObject, DROPEFFECT_NONE );
	}

	return TRUE;
}

// Perform basic album operations (copy or move)

BOOL CShareazaDataSource::DropToAlbum(IDataObject* pIDataObject, DWORD grfKeyState, DWORD* pdwEffect, BOOL bDrop, CAlbumFolder* pAlbumFolder)
{
	if( ! pdwEffect )
		return FALSE;

	*pdwEffect = DROPEFFECT_NONE;

	if ( ( grfKeyState & MK_CONTROL ) && ( grfKeyState & MK_SHIFT ) )
		return FALSE;

	if( ! pIDataObject )
		return FALSE;

	if ( ! pAlbumFolder ||
		! LibraryFolders.CheckAlbum( pAlbumFolder ) ||
		CheckURI( pAlbumFolder->m_sSchemaURI, CSchema::uriGhostFolder ) ||
		CheckURI( pAlbumFolder->m_sSchemaURI, CSchema::uriSearchFolder ) )
	{
		// Drop disabled to temporary/invalid, ghost or search albums
		return FALSE;
	}

	if ( ! IsShareazaObject( pIDataObject ) )
	{
		// This is not a Shareaza's drop
		return FALSE;
	}

	CAlbumFolder* pRoot = Library.GetAlbumRoot();
	if ( ! pRoot )
		return FALSE;

	BOOL bRet = FALSE;

	// Drop files to album
	FORMATETC fmtc1 = { (CLIPFORMAT) RegisterClipboardFormat( CF_SHAREAZA_FILES ), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	if ( SUCCEEDED ( pIDataObject->QueryGetData( &fmtc1 ) ) )
	{
		*pdwEffect = (grfKeyState & MK_CONTROL) ? DROPEFFECT_COPY :
			( (grfKeyState & MK_SHIFT ) ? DROPEFFECT_MOVE : DROPEFFECT_COPY );

		CStgMedium medium;
		if ( SUCCEEDED ( pIDataObject->GetData( &fmtc1, &medium ) ) )
		{
			SIZE_T size = GlobalSize( medium.hGlobal ) / 20;
			if ( medium.tymed == TYMED_HGLOBAL && medium.hGlobal != NULL &&
				size > 0 && size < 10000000 )
			{
				LPBYTE p = (LPBYTE)GlobalLock( medium.hGlobal );
				if ( p )
				{
					while ( size-- )
					{
						DWORD index = *(DWORD*)p;
						CLibraryFile* pFile = Library.LookupFile( index, FALSE, TRUE );
						if ( pFile )
						{
							Hashes::Guid oGUID;
							CopyMemory( &*oGUID.begin(), p + sizeof( DWORD ), 16 );
							CAlbumFolder* pFolder = pRoot->FindFolder( oGUID );
							if ( pFolder && *pAlbumFolder == *pFolder )
							{
								// Drop disabled to same album
							}
							else
							{
								// if pFolder == NULL than file is not from album

								bRet = TRUE;
								if ( bDrop )
								{
									// Add new file
									pAlbumFolder->AddFile( pFile );

									// Remove old file
									if ( pFolder && *pdwEffect == DROPEFFECT_MOVE )
									{
										pFolder->RemoveFile( pFile );
									}

									pAlbumFolder->m_nUpdateCookie++;
								}
							}
						}
						p += 20;	// DWORD + GUID
					}
					GlobalUnlock( medium.hGlobal );
				}
			}
		}
	}

	// Drop album to album
	FORMATETC fmtc2 = { (CLIPFORMAT) RegisterClipboardFormat( CF_SHAREAZA_ALBUMS ), NULL, DVASPECT_CONTENT, -1, TYMED_ISTREAM };
	if ( SUCCEEDED ( pIDataObject->QueryGetData( &fmtc2 ) ) )
	{
		*pdwEffect = (grfKeyState & MK_CONTROL) ? DROPEFFECT_COPY :
			( (grfKeyState & MK_SHIFT ) ? DROPEFFECT_MOVE : DROPEFFECT_COPY );

		CStgMedium medium;
		if ( SUCCEEDED ( pIDataObject->GetData( &fmtc2, &medium ) ) )
		{
			LARGE_INTEGER zero = { 0 };
			medium.pstm->Seek( zero, STREAM_SEEK_SET, NULL );

			CStreamArchive ar ( medium.pstm, CArchive::load );
			DWORD size_Archive = 0;
			ar >> (DWORD) size_Archive;
			while( size_Archive-- )
			{
				CAlbumFolder* pFolder =  new CAlbumFolder( pAlbumFolder );
				if ( pFolder )
				{
					try {
						pFolder->Serialize( ar, LIBRARY_SER_VERSION );

						if ( *pAlbumFolder == *pFolder ||
							pAlbumFolder->CheckFolder( pFolder, FALSE ) ||
							pFolder->CheckFolder( pAlbumFolder, TRUE ) )
						{
							// Drop disabled to same, parent or child album
						}
						else
						{
							bRet = TRUE;
							if ( bDrop )
							{
								if ( *pdwEffect == DROPEFFECT_MOVE )
								{
									// Delete old album (by GUID)
									if ( CAlbumFolder* pRealFodler = pRoot->FindFolder( pFolder->m_oGUID ) )
									{
										if ( pRealFodler->GetParent() )
											pRealFodler->GetParent()->OnFolderDelete( pRealFodler );
										else
											pRoot->OnFolderDelete( pRealFodler );
									}
								}

								// Add new album
								pAlbumFolder->AddFolder( pFolder );

								// Keep album
								pFolder = NULL;								
							}
						}
					}
					catch (...)
					{						
					}
					delete pFolder;
				}
			}
			ar.Detach();
		}
	}

	if ( bRet )
	{
		if ( bDrop )
		{
			Library.Update( true );

			if ( *pdwEffect == DROPEFFECT_MOVE )
			{
				// Optimized move used
				*pdwEffect = DROPEFFECT_NONE;
				SetDropEffect( pIDataObject, DROPEFFECT_NONE );
			}
		}
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}
	return bRet;
}

// Check if this is a Shareazas drag-n-drop object

BOOL CShareazaDataSource::IsShareazaObject(IDataObject* pIDataObject)
{
	ASSERT( pIDataObject != NULL );
	FORMATETC formatetc = { (CLIPFORMAT) RegisterClipboardFormat( CF_SHAREAZA ), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	return pIDataObject->QueryGetData( &formatetc ) == S_OK;
}

// Add CF_SHAREAZA

HRESULT CShareazaDataSource::Add(IDataObject* pIDataObject)
{
	FORMATETC formatetc = { (CLIPFORMAT) RegisterClipboardFormat( CF_SHAREAZA ), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM medium = { TYMED_HGLOBAL, NULL, NULL };
	
	CHGlobal < BOOL > oHGlobal;
	if ( ! oHGlobal.IsValid() )
	{
		return E_OUTOFMEMORY;
	}
	medium.hGlobal = oHGlobal;
	HRESULT hr = pIDataObject->SetData( &formatetc, &medium, FALSE );
	return hr;
}

// Add medias by IDragSourceHelper

HRESULT CShareazaDataSource::Add(IDataObject* pIDataObject, HBITMAP pImage, const CPoint& ptOffset)
{
	CComQIPtr< IDragSourceHelper, &IID_IDragSourceHelper > pIDragSourceHelper( pIDataObject );
	HRESULT hr = S_FALSE;
	if ( pIDragSourceHelper.p && pImage )
	{
		SHDRAGIMAGE shdi = { 0 };
		BITMAP bmpInfo = { 0 };
		GetObject( pImage, sizeof( BITMAP ), &bmpInfo );
		shdi.sizeDragImage.cx = bmpInfo.bmWidth;
		shdi.sizeDragImage.cy = bmpInfo.bmHeight;
		shdi.ptOffset.x = ptOffset.x;
		shdi.ptOffset.y = ptOffset.y;
		shdi.hbmpDragImage = pImage;
		shdi.crColorKey = DRAG_COLOR_KEY;
		hr = pIDragSourceHelper->InitializeFromBitmap( &shdi, pIDataObject );
	}
	return hr;
}

// Add CF_HDROP/CF_SHAREAZA_ALBUMS/CF_SHAREAZA_FILES

template < typename T >
HRESULT CShareazaDataSource::AddFiles(IDataObject* pIDataObject, const T* pSelFirst, const Hashes::Guid& oGUID )
{
	// Precalculate sizes
	size_t size_HDROP = 0;
	size_t size_Archive = 0;
	size_t size_Files = 0;

	// Precalculate size of structures
	GetTotalLength( pSelFirst, size_HDROP, size_Archive, size_Files, TRUE );

	// Initialize CF_HDROP
	CHGlobal < DROPFILES > oHDROP( size_HDROP + sizeof (DROPFILES) + sizeof( TCHAR ) );
	if ( ! oHDROP.IsValid() )
	{
		return E_OUTOFMEMORY;
	}
	LPTSTR buf_HDROP = (LPTSTR)( (BYTE*)( (DROPFILES*)oHDROP ) + sizeof( DROPFILES ) );
	oHDROP->pFiles = sizeof( DROPFILES );
	GetCursorPos( &oHDROP->pt );
	oHDROP->fNC = TRUE;
	oHDROP->fWide = ( sizeof( TCHAR ) != sizeof( char ) );	

	// Initialize CF_SHAREAZA_ALBUMS
	CStreamArchive buf_Archive ( CArchive::store );
	if ( ! buf_Archive.IsValid() )
	{
		return E_OUTOFMEMORY;
	}
	if ( size_Archive )
	{
		buf_Archive << (DWORD) size_Archive;
	}

	// Initialize CF_SHAREAZA_FILES
	CHGlobal < BYTE > oFiles( size_Files * 20 );	// [DWORD 1][GUID 1]...[DWORD N][GUID N]
	if ( ! oFiles.IsValid() )
	{
		return E_OUTOFMEMORY;
	}
	LPBYTE buf_Files = oFiles;

	CString buf_Text;

	// Fill structures
	FillBuffer( pSelFirst, buf_HDROP, buf_Archive, buf_Files, buf_Text, TRUE, oGUID );

	// Finalize CF_TEXT
	if ( buf_Text.GetLength() )
	{
		STGMEDIUM medium_Text = { TYMED_HGLOBAL, NULL, NULL };
		FORMATETC formatetc_Text = { CF_TEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		size_t size_Text = ( buf_Text.GetLength() + 1 ) * sizeof( CHAR );
		CHGlobal < TCHAR > oText( size_Text );
		if ( ! oText.IsValid() )
		{
			return E_OUTOFMEMORY;
		}
		CopyMemory( (LPTSTR)oText, (LPCSTR)CT2CA( buf_Text ), size_Text );
		medium_Text.hGlobal = oText;
		HRESULT hr = pIDataObject->SetData( &formatetc_Text, &medium_Text, FALSE );
		if ( FAILED ( hr ) )
		{
			return hr;
		}
	}

	// Finalize CF_HDROP and optional CFSTR_PREFERREDDROPEFFECT
	if ( size_HDROP ) 
	{
		STGMEDIUM medium_HDROP = { TYMED_HGLOBAL, NULL, NULL };
		FORMATETC formatetc_HDROP = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		medium_HDROP.hGlobal = oHDROP;
		HRESULT hr = pIDataObject->SetData( &formatetc_HDROP, &medium_HDROP, FALSE );
		if ( FAILED ( hr ) )
		{
			return hr;
		}

		STGMEDIUM medium_PDS = { TYMED_HGLOBAL, NULL, NULL };
		FORMATETC formatetc_PDS = { (CLIPFORMAT) RegisterClipboardFormat( CFSTR_PREFERREDDROPEFFECT ), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		CHGlobal < DWORD > oPDS;
		if ( ! oPDS.IsValid() )
		{
			return E_OUTOFMEMORY;
		}
		*(DWORD*)oPDS = DROPEFFECT_NONE; // Let's Explorer choose
		medium_PDS.hGlobal = oPDS;
		pIDataObject->SetData( &formatetc_PDS, &medium_PDS, FALSE );
	}

	// Finalize CF_SHAREAZA_ALBUMS
	if ( size_Archive )
	{
		STGMEDIUM medium_Archive = { TYMED_ISTREAM, NULL, NULL };
		FORMATETC formatetc_Archive = { (CLIPFORMAT) RegisterClipboardFormat( CF_SHAREAZA_ALBUMS ), NULL, DVASPECT_CONTENT, -1, TYMED_ISTREAM };
		buf_Archive.Close();
		medium_Archive.pstm = buf_Archive;
		HRESULT hr = pIDataObject->SetData( &formatetc_Archive, &medium_Archive, FALSE );
		if ( FAILED ( hr ) )
		{
			return hr;
		}
	}

	// Finalize CF_SHAREAZA_FILES
	if ( size_Files ) 
	{
		STGMEDIUM medium_Files = { TYMED_HGLOBAL, NULL, NULL };
		FORMATETC formatetc_Files = { (CLIPFORMAT) RegisterClipboardFormat( CF_SHAREAZA_FILES ), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		medium_Files.hGlobal = oFiles;
		HRESULT hr = pIDataObject->SetData( &formatetc_Files, &medium_Files, FALSE );
		if ( FAILED ( hr ) )
		{
			return hr;
		}
	}

	return S_OK;
}

// Find data in data cahe (and allocate new if not found but need)
//	Returns:
//	S_OK - added new entry			DV_E_DVTARGETDEVICE - not supported format
//	S_FALSE - found old entry		DV_E_FORMATETC - not found
//	E_OUTOFMEMORY - out of memory	DV_E_TYMED - found but different storage type

HRESULT CShareazaDataSource::FindFORMATETC(FORMATETC *pfe, LPDATAENTRY *ppde, BOOL fAdd)
{
	ASSERT( pfe != NULL );
	ASSERT( ppde != NULL );

	*ppde = NULL;

	// Comparing two DVTARGETDEVICE structures is hard, so we don't even try
	if (pfe->ptd != NULL)
		return DV_E_DVTARGETDEVICE;

	CSingleLock pLock( &m_pSection, TRUE );

	// See if it's in our list
	for (int ide = 0; ide < m_cde; ide++)
	{
		if (m_rgde[ide].fe.cfFormat == pfe->cfFormat &&
			m_rgde[ide].fe.dwAspect == pfe->dwAspect &&
			m_rgde[ide].fe.lindex == pfe->lindex)
		{
			if (fAdd || (m_rgde[ide].fe.tymed & pfe->tymed))
			{
				*ppde = &m_rgde[ide];
				return S_FALSE;
			} else {
				return DV_E_TYMED;
			}
		}
	}

	if ( ! fAdd )
		return DV_E_FORMATETC;

	LPDATAENTRY pdeT = (LPDATAENTRY) CoTaskMemRealloc(
		m_rgde, sizeof( DATAENTRY ) * ( m_cde + 1 ) );
	if ( ! pdeT )
		return E_OUTOFMEMORY;

	m_rgde = pdeT;
	m_rgde[m_cde].fe = *pfe;
	ZeroMemory( &pdeT[m_cde].stgm, sizeof( STGMEDIUM ) );
	*ppde = &m_rgde[m_cde];
	m_cde++;
	return S_OK;
}

// Add data to data cache

HRESULT CShareazaDataSource::AddRefStgMedium(STGMEDIUM *pstgmIn, STGMEDIUM *pstgmOut, BOOL fCopyIn)
{
	ASSERT( pstgmIn != NULL );
	ASSERT( pstgmOut != NULL );

	HRESULT hr = S_OK;
	STGMEDIUM stgmOut = *pstgmIn;
	if (  ( pstgmIn->pUnkForRelease == NULL ) &&
		! ( pstgmIn->tymed & ( TYMED_ISTREAM | TYMED_ISTORAGE ) ) )
	{
		if ( fCopyIn ) {
			// Object needs to be cloned
			if ( pstgmIn->tymed == TYMED_HGLOBAL )
			{
				CHGlobal < BYTE > oHGlobal( pstgmIn->hGlobal );
				if ( oHGlobal.IsValid() )
				{
					stgmOut.hGlobal = oHGlobal.Detach();
				}
				else
					hr = E_OUTOFMEMORY;
			}
			else
				hr = DV_E_TYMED;      // Don't know how to clone GDI objects
		}
		else
			stgmOut.pUnkForRelease = &m_xDataObject;
	}
	if ( SUCCEEDED( hr ) )
	{
		switch ( stgmOut.tymed )
		{
			case TYMED_ISTREAM:
				stgmOut.pstm->AddRef();
				break;

			case TYMED_ISTORAGE:
				stgmOut.pstg->AddRef();
				break;
		}
		if ( stgmOut.pUnkForRelease )
			stgmOut.pUnkForRelease->AddRef();

		*pstgmOut = stgmOut;
	}
	return hr;
}

BEGIN_INTERFACE_MAP(CShareazaDataSource, CComObject)
	INTERFACE_PART(CShareazaDataSource, IID_IDropSource, DropSource)
	INTERFACE_PART(CShareazaDataSource, IID_IDataObject, DataObject)
	INTERFACE_PART(CShareazaDataSource, IID_IDragSourceHelper, DragSourceHelper)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShareazaDataSource data source interface

IMPLEMENT_UNKNOWN(CShareazaDataSource, DataObject)

STDMETHODIMP CShareazaDataSource::XDataObject::GetData(FORMATETC *pformatetc, STGMEDIUM *pmedium)
{
	METHOD_PROLOGUE( CShareazaDataSource, DataObject )

	if ( pformatetc == NULL )
		return E_POINTER;

	if ( pmedium == NULL )
		return E_POINTER;

	LPDATAENTRY pde = NULL;
    HRESULT hr = pThis->FindFORMATETC( pformatetc, &pde, FALSE );
    if ( SUCCEEDED ( hr ) )
	{
        hr = pThis->AddRefStgMedium( &pde->stgm, pmedium, FALSE );
    }

#ifdef _DEBUG
	if ( FAILED ( hr ) )
		TRACE("0x%08x : GetData( {%ls, %d, %d, 0x%08x, %d}, 0x%08x ) : 0x%08x\n",
			this, GetFORMATLIST( pformatetc->cfFormat ), pformatetc->dwAspect,
			pformatetc->lindex, pformatetc->ptd, pformatetc->tymed, pmedium, hr);
#endif
	return hr;
}

STDMETHODIMP CShareazaDataSource::XDataObject::GetDataHere(FORMATETC* /* pformatetc */, STGMEDIUM* /* pmedium */)
{
	METHOD_PROLOGUE( CShareazaDataSource, DataObject )

	TRACE("0x%08x : GetDataHere() : E_NOTIMPL\n", this);

	return E_NOTIMPL;
}

STDMETHODIMP CShareazaDataSource::XDataObject::QueryGetData (FORMATETC* pformatetc)
{
	METHOD_PROLOGUE( CShareazaDataSource, DataObject )

	if ( pformatetc == NULL )
		return E_POINTER;

    LPDATAENTRY pde = NULL;
	HRESULT hr = pThis->FindFORMATETC( pformatetc, &pde, FALSE );
	if ( SUCCEEDED( hr ) ) hr = S_OK;

#ifdef _DEBUG
	if ( FAILED ( hr ) )
		TRACE("0x%08x : QueryGetData( {%ls, %d, %d, 0x%08x, %d} ) : 0x%08x\n",
			this, GetFORMATLIST( pformatetc->cfFormat ), pformatetc->dwAspect,
			pformatetc->lindex, pformatetc->ptd, pformatetc->tymed, hr);
#endif

	return hr;
}

STDMETHODIMP CShareazaDataSource::XDataObject::GetCanonicalFormatEtc(FORMATETC* /* pformatectIn */,FORMATETC* /* pformatetcOut */)
{
	METHOD_PROLOGUE( CShareazaDataSource, DataObject )

	TRACE("0x%08x : GetCanonicalFormatEtc() : E_NOTIMPL\n", this);

	return E_NOTIMPL;
}

STDMETHODIMP CShareazaDataSource::XDataObject::SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease)
{
	METHOD_PROLOGUE( CShareazaDataSource, DataObject )

	if ( pformatetc == NULL )
		return E_POINTER;

	if ( pmedium == NULL )
		return E_POINTER;

#ifdef _DEBUG
	TRACE("0x%08x : SetData( {%ls, %d, %d, 0x%08x, %d}, { %d, 0x%08x, 0x%08x }, %d ) : ", this,
		GetFORMATLIST( pformatetc->cfFormat ), pformatetc->dwAspect, pformatetc->lindex, pformatetc->ptd, pformatetc->tymed,
		pmedium->tymed, pmedium->hGlobal, pmedium->pUnkForRelease, fRelease);
#endif

	LPDATAENTRY pde = NULL;
	HRESULT hr = pThis->FindFORMATETC( pformatetc, &pde, TRUE );
	if ( hr == S_FALSE )
		// Release old data
		ReleaseStgMedium( &pde->stgm );
	if ( SUCCEEDED( hr ) )
	{
		if (fRelease) {
			pde->stgm = *pmedium;
			hr = S_OK;
		} else {
			hr = pThis->AddRefStgMedium( pmedium, &pde->stgm, TRUE );
		}
		pde->fe.tymed = pde->stgm.tymed;    // Keep in sync

		// Subtlety!  Break circular reference loop
		if (GetCanonicalIUnknown( pde->stgm.pUnkForRelease ) == GetCanonicalIUnknown( this ) )
		{
			pde->stgm.pUnkForRelease->Release();
			pde->stgm.pUnkForRelease = NULL;
		}
	}

	TRACE("0x%08x\n", hr);

	return hr;
}

STDMETHODIMP CShareazaDataSource::XDataObject::EnumFormatEtc(DWORD /*dwDirection*/, IEnumFORMATETC** ppenumFormatEtc)
{
	METHOD_PROLOGUE( CShareazaDataSource, DataObject )

	if ( ppenumFormatEtc == NULL )
		return E_POINTER;
	*ppenumFormatEtc = NULL;

	CEnumFormatEtc* pFormatList = new CEnumFormatEtc;
	if ( ! pFormatList )
		return E_OUTOFMEMORY;

	CSingleLock pLock( &pThis->m_pSection, TRUE );

	for ( int nIndex = 0; nIndex < pThis->m_cde; nIndex++ )
	{
		pFormatList->AddFormat( &pThis->m_rgde[nIndex].fe );
	}

	// give it away to OLE (ref count is already 1)
	*ppenumFormatEtc = (LPENUMFORMATETC) &pFormatList->m_xEnumVOID;

	return S_OK;
}

STDMETHODIMP CShareazaDataSource::XDataObject::DAdvise(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
	METHOD_PROLOGUE( CShareazaDataSource, DataObject )

	HRESULT hr = S_OK;
	if ( m_spDataAdviseHolder == NULL )
		hr = CreateDataAdviseHolder( &m_spDataAdviseHolder );

	if ( hr == S_OK )
		hr = m_spDataAdviseHolder->Advise( this, pformatetc, advf, pAdvSink, pdwConnection );

	return hr;
}

STDMETHODIMP CShareazaDataSource::XDataObject::DUnadvise(DWORD dwConnection)
{
	METHOD_PROLOGUE( CShareazaDataSource, DataObject )

	HRESULT hr = S_OK;
	if ( m_spDataAdviseHolder == NULL )
		hr = OLE_E_NOCONNECTION;
	else
		hr = m_spDataAdviseHolder->Unadvise(dwConnection);

	return hr;
}

STDMETHODIMP CShareazaDataSource::XDataObject::EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
{
	METHOD_PROLOGUE( CShareazaDataSource, DataObject )

	if ( ppenumAdvise == NULL )
		return E_POINTER;
	*ppenumAdvise = NULL;

	if ( m_spDataAdviseHolder != NULL )
		return m_spDataAdviseHolder->EnumAdvise( ppenumAdvise );

	return E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaDataSource drop source interface

IMPLEMENT_UNKNOWN(CShareazaDataSource, DropSource)

STDMETHODIMP CShareazaDataSource::XDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
	METHOD_PROLOGUE( CShareazaDataSource, DropSource )

	// check escape key or right button and cancel
	if ( fEscapePressed || ( grfKeyState & MK_RBUTTON ) || (grfKeyState & MK_MBUTTON ) )
		return DRAGDROP_S_CANCEL;

	// check left-button up and do the drop
	if ( ! ( grfKeyState & MK_LBUTTON ) )
		return DRAGDROP_S_DROP;

	// otherwise, keep polling...
	return S_OK;
}

STDMETHODIMP CShareazaDataSource::XDropSource::GiveFeedback(DWORD /* dwEffect */)
{
	METHOD_PROLOGUE( CShareazaDataSource, DropSource )

	return DRAGDROP_S_USEDEFAULTCURSORS;
}

/////////////////////////////////////////////////////////////////////////////
// CShareazaDataSource data source interface

IMPLEMENT_UNKNOWN(CShareazaDataSource, DragSourceHelper)

STDMETHODIMP CShareazaDataSource::XDragSourceHelper::InitializeFromBitmap(LPSHDRAGIMAGE pshdi, IDataObject* pDataObject)
{
	METHOD_PROLOGUE( CShareazaDataSource, DragSourceHelper )

	return ( pThis->m_pdsh.p ) ? pThis->m_pdsh->InitializeFromBitmap( pshdi, pDataObject ) : E_NOTIMPL;
}

STDMETHODIMP CShareazaDataSource::XDragSourceHelper::InitializeFromWindow(HWND hwnd, POINT* ppt, IDataObject* pDataObject)
{
	METHOD_PROLOGUE( CShareazaDataSource, DragSourceHelper )

	return ( pThis->m_pdsh.p ) ? pThis->m_pdsh->InitializeFromWindow( hwnd, ppt, pDataObject ) : E_NOTIMPL;
}

// Service methods

IUnknown* CShareazaDataSource::GetCanonicalIUnknown(IUnknown *punk)
{
	IUnknown* punkCanonical = NULL;
	if (punk && SUCCEEDED( punk->QueryInterface( IID_IUnknown, (LPVOID*) &punkCanonical ) ) )
		punkCanonical->Release();
	else
		punkCanonical = punk;
	return punkCanonical;
}

// Calculate total length of files names and albums

void CShareazaDataSource::GetTotalLength(const CLibraryList* pList, size_t& size_HDROP, size_t& size_Archive, size_t& size_Files, BOOL bRoot)
{
	ASSERT_VALID( pList );

	POSITION pos = pList->GetHeadPosition();
	while( pos )
	{
		CLibraryListItem Item = pList->GetNext( pos );
		switch ( Item.Type )
		{
		case CLibraryListItem::LibraryFile:
			{
				CLibraryFile* pFile = Item;
				ASSERT( pFile != NULL );
				if ( pFile )
				{
					int len = pFile->GetPath().GetLength();
					if ( len )
					{
						size_HDROP += ( len + 1 ) * sizeof( TCHAR );
					}

					if ( bRoot )
					{
						size_Files++;
					}
				}
			}
			break;

		case CLibraryListItem::AlbumFolder:
			{
				CAlbumFolder* pAlbum = Item;
				ASSERT( pAlbum != NULL );
				if ( pAlbum && bRoot &&
					! CheckURI( pAlbum->m_sSchemaURI, CSchema::uriGhostFolder ) )
				{
					CLibraryListPtr pNewList( new CLibraryList() );
					pAlbum->GetFileList( pNewList, TRUE );
					GetTotalLength( pNewList, size_HDROP, size_Archive, size_Files, FALSE );

					size_Archive++;
				}
			}
			break;

		case CLibraryListItem::LibraryFolder:
			{
				CLibraryFolder* pFolder = Item;
				ASSERT( pFolder != NULL );
				if ( pFolder )
				{
					int len = pFolder->m_sPath.GetLength();
					if ( len )
					{
						size_HDROP += ( len + 1 ) * sizeof( TCHAR );
					}
				}
			}
			break;

		case CLibraryListItem::Empty:
		default:
			break;
		}
	}
}

// Calculate total length of files names and albums

void CShareazaDataSource::GetTotalLength(const CLibraryTreeItem* pSelFirst, size_t& size_HDROP, size_t& size_Archive, size_t& size_Files, BOOL bRoot)
{
	ASSERT_VALID( pSelFirst );

	for ( const CLibraryTreeItem* pItem = pSelFirst ; pItem ; pItem = pItem->m_pSelNext )
	{
		if ( pItem->m_pVirtual && bRoot &&
			! CheckURI( pItem->m_pVirtual->m_sSchemaURI, CSchema::uriGhostFolder ) )
		{
			// Add all files within virtual folder (recursively)
			CLibraryListPtr pList( new CLibraryList() );
			pItem->m_pVirtual->GetFileList( pList, TRUE );
			GetTotalLength( pList, size_HDROP, size_Archive, size_Files, FALSE );

			// Add virtual folder
			size_Archive++;
		}
		if ( pItem->m_pPhysical )
		{
			// Add physical folder
			if ( ! pItem->m_pPhysical->m_sPath.IsEmpty() )
			{
				size_HDROP += ( pItem->m_pPhysical->m_sPath.GetLength() + 1 ) * sizeof( TCHAR );
			}
		}
	}
}

// Fill buffer by files names and albums

void CShareazaDataSource::FillBuffer(const CLibraryList* pList, LPTSTR& buf_HDROP, CArchive& buf_Archive, LPBYTE& buf_Files, CString& buf_Text, BOOL bRoot, const Hashes::Guid& oGUID)
{
	ASSERT_VALID( pList );

	POSITION pos = pList->GetHeadPosition();
	while( pos )
	{
		CLibraryListItem Item = pList->GetNext( pos );
		switch ( Item.Type )
		{
		case CLibraryListItem::LibraryFile:
			{
				CLibraryFile* pFile = Item;
				ASSERT( pFile != NULL );
				if ( pFile )
				{
					if ( pFile->m_oSHA1 && pFile->m_oTiger && pFile->m_oED2K &&
						pFile->m_nSize != 0 && pFile->m_nSize != SIZE_UNKNOWN &&
						pFile->m_sName )
					{
						CString sTemp;
						sTemp.Format(
							_T("magnet:?xt=urn:bitprint:%s.%s&xt=%s&xl=%I64u&dn=%s"),
							(LPCTSTR)pFile->m_oSHA1.toString(),
							(LPCTSTR)pFile->m_oTiger.toString(),
							(LPCTSTR)pFile->m_oED2K.toUrn(),
							pFile->m_nSize,
							(LPCTSTR)URLEncode( pFile->m_sName ) );
						if ( buf_Text.GetLength() )
							buf_Text += _T("\r\n\r\n");
						buf_Text += sTemp;
					}

					int len = pFile->GetPath().GetLength();
					if ( len )
					{
						_tcscpy_s( buf_HDROP, len + 1, pFile->GetPath() );
						buf_HDROP += len + 1;
					}

					if ( bRoot )
					{
						*(DWORD*)buf_Files = Item;
						CopyMemory( buf_Files + sizeof( DWORD ), &*oGUID.begin(), 16 );
						buf_Files += 20;
					}
				}
			}
			break;

		case CLibraryListItem::AlbumFolder:
			{
				CAlbumFolder* pAlbum = Item;
				ASSERT( pAlbum != NULL );
				if ( pAlbum && bRoot &&
					! CheckURI( pAlbum->m_sSchemaURI, CSchema::uriGhostFolder ) )
				{
					CLibraryListPtr pNewList( new CLibraryList() );
					pAlbum->GetFileList( pNewList, TRUE );
					FillBuffer( pNewList, buf_HDROP, buf_Archive, buf_Files, buf_Text, FALSE, pAlbum->m_oGUID );

					pAlbum->Serialize( buf_Archive, LIBRARY_SER_VERSION );
				}
			}
			break;

		case CLibraryListItem::LibraryFolder:
			{
				CLibraryFolder* pFolder = Item;
				ASSERT( pFolder != NULL );
				if ( pFolder )
				{
					int len = pFolder->m_sPath.GetLength();
					if ( len )
					{
						_tcscpy_s( buf_HDROP, len + 1, pFolder->m_sPath );
						buf_HDROP += len + 1;
					}
				}
			}
			break;

		case CLibraryListItem::Empty:
		default:
			break;
		}
	}
}

// Fill buffer by files names and albums

void CShareazaDataSource::FillBuffer(const CLibraryTreeItem* pSelFirst, LPTSTR& buf_HDROP, CArchive& buf_Archive, LPBYTE& buf_Files, CString& buf_Text, BOOL bRoot, const Hashes::Guid& /*oGUID*/)
{
	ASSERT_VALID( pSelFirst );

	for ( const CLibraryTreeItem* pItem = pSelFirst ; pItem ; pItem = pItem->m_pSelNext )
	{
		if ( pItem->m_pVirtual && bRoot &&
			! CheckURI( pItem->m_pVirtual->m_sSchemaURI, CSchema::uriGhostFolder ) )
		{
			// Add all files within virtual folder (recursively)
			CLibraryListPtr pList( new CLibraryList() );
			pItem->GetFileList( pList, TRUE );
			FillBuffer( pList, buf_HDROP, buf_Archive, buf_Files, buf_Text, FALSE, pItem->m_pVirtual->m_oGUID );

			// Add virtual folder
			pItem->m_pVirtual->Serialize( buf_Archive, LIBRARY_SER_VERSION );
		}
		if ( pItem->m_pPhysical )
		{
			// Add physical folder
			if ( ! pItem->m_pPhysical->m_sPath.IsEmpty() )
			{
				int len = pItem->m_pPhysical->m_sPath.GetLength();
				if ( len )
				{
					_tcscpy_s( buf_HDROP, len + 1, pItem->m_pPhysical->m_sPath );
					buf_HDROP += len + 1;
				}
			}
		}
	}
}
