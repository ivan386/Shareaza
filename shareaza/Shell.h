//
// Shell.h
//
// Copyright (c) Shareaza Development Team, 2008-2010.
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

#pragma once

class CShellItem
{
public:
	CShellItem(LPCTSTR szFullPath, void** ppFolder = NULL, HWND hWnd = NULL) :
		m_pidl( NULL ),
		m_pLastId( NULL )
	{
		CComPtr< IShellFolder > pDesktop;
		HRESULT hr = SHGetDesktopFolder( &pDesktop );
		if ( FAILED( hr ) )
			return;

		hr = pDesktop->ParseDisplayName( hWnd, 0, CT2OLE( szFullPath ), NULL,
			&m_pidl, NULL );
		if ( FAILED( hr ) )
			return;

		m_pLastId = m_pidl;
		USHORT temp;
		for (;;)
		{
			USHORT offset = m_pLastId->mkid.cb;
			temp = *(USHORT*)( (BYTE*)m_pLastId + offset );
			if ( temp == 0 )
				break;
			m_pLastId = (LPITEMIDLIST)( (BYTE*)m_pLastId + offset );
		}
		if ( ppFolder )
		{
			temp = m_pLastId->mkid.cb;
			m_pLastId->mkid.cb = 0;
			hr = pDesktop->BindToObject( m_pidl, NULL, IID_IShellFolder, ppFolder );
			m_pLastId->mkid.cb = temp;
		}
	}

	virtual ~CShellItem()
	{
		if ( m_pidl )
			CoTaskMemFree( m_pidl );
	}

	inline operator LPITEMIDLIST() const throw()
	{
		return m_pidl;
	}

public:
	LPITEMIDLIST	m_pidl;		// Full path
	LPITEMIDLIST	m_pLastId;	// Filename only
};

class CShellList :
	public CList< CShellItem* >
{
public:
	CShellList() :
		m_pID( NULL )
	{
	}
	CShellList(const CStringList& oFiles) :
		m_pID( NULL )
	{
		*this = oFiles;
	}

	virtual ~CShellList()
	{
		Clear();
	}

	CShellList& operator=(const CStringList& oFiles)
	{
		Clear();

		for ( POSITION pos = oFiles.GetHeadPosition(); pos; )
		{
			CString strPath = oFiles.GetNext( pos );
			CShellItem* pItemIDList = new CShellItem( strPath,
				( m_pFolder ? NULL : (void**)&m_pFolder ) );	// Get only one
			if ( pItemIDList->m_pidl )
				AddTail( pItemIDList );
			else
				// Bad path
				delete pItemIDList;
		}

		if ( GetCount() )
		{
			m_pID.reset( new LPCITEMIDLIST [ GetCount() ] );
			if ( m_pID.get() )
			{
				int i = 0;
				for ( POSITION pos = GetHeadPosition(); pos; i++)
					m_pID[ i ] = GetNext( pos )->m_pLastId;
			}
		}

		return *this;
	}

	// Creates menu from file paths list
	bool GetMenu(HWND hWnd, void** ppContextMenu) const
	{
		return GetCount() && m_pFolder && SUCCEEDED( m_pFolder->GetUIObjectOf( hWnd,
			(UINT)GetCount(), m_pID.get(), IID_IContextMenu, NULL, ppContextMenu ) );
	}

protected:
	CComPtr< IShellFolder >		m_pFolder;	// First file folder
	auto_array< LPCITEMIDLIST >	m_pID;		// File ItemID array
	
	void Clear()
	{
		for ( POSITION pos = GetHeadPosition(); pos; )
			delete GetNext( pos );
		RemoveAll();

		m_pID.reset();
	}
};
