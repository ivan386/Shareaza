//
// LibraryList.h
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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

#if !defined(AFX_LIBRARYLIST_H__C045FFC3_F813_4962_94D0_FC5E9E4A7BA3__INCLUDED_)
#define AFX_LIBRARYLIST_H__C045FFC3_F813_4962_94D0_FC5E9E4A7BA3__INCLUDED_

#pragma once

class CLibraryFile;


class CLibraryList : public CComObject
{
// Construction
public:
	CLibraryList(int nBlockSize = 16);
	virtual ~CLibraryList();

// Attributes
protected:
	DWORD*	m_pList;
	int		m_nCount;
	int		m_nBuffer;
	int		m_nBlock;

// Operations
public:
	inline int GetCount() const
	{
		return m_nCount;
	}
	
	inline BOOL IsEmpty() const
	{
		return m_nCount == 0;
	}
	
	inline DWORD GetHead() const
	{
		ASSERT( m_nCount > 0 );
		return m_pList[ 0 ];
	}
	
	inline DWORD GetTail() const
	{
		ASSERT( m_nCount > 0 );
		return m_pList[ m_nCount - 1 ];
	}

	inline POSITION GetIterator() const
	{
		return m_nCount ? (POSITION)1 : NULL;
	}

	inline POSITION GetHeadPosition() const
	{
		return m_nCount ? (POSITION)1 : NULL;
	}
	
	inline POSITION GetTailPosition() const
	{
		return m_nCount ? (POSITION)m_nCount : NULL;
	}
	
	inline DWORD GetPrev(POSITION& pos) const
	{
		ASSERT( (int)pos > 0 && (int)pos <= m_nCount );
		DWORD nItem = m_pList[ (int)pos-- - 1 ];
		if ( (int)pos < 1 || (int)pos > m_nCount ) pos = NULL;
		return nItem;
	}
	
	inline DWORD GetNext(POSITION& pos) const
	{
		ASSERT( (int)pos > 0 && (int)pos <= m_nCount );
		DWORD nItem = m_pList[ (int)pos++ - 1 ];
		if ( (int)pos < 1 || (int)pos > m_nCount ) pos = NULL;
		return nItem;
	}
	
	inline POSITION AddHead(DWORD nItem)
	{
		if ( m_nCount == m_nBuffer )
		{
			m_nBuffer += m_nBlock;
			m_pList = (DWORD*)realloc( m_pList, sizeof(DWORD) * m_nBuffer );
		}
		MoveMemory( m_pList + 1, m_pList, sizeof(DWORD) * m_nCount );
		m_pList[ 0 ] = nItem;
		m_nCount++;
		return (POSITION)1;
	}

	inline POSITION AddTail(DWORD nItem)
	{
		if ( m_nCount == m_nBuffer )
		{
			m_nBuffer += m_nBlock;
			m_pList = (DWORD*)realloc( m_pList, sizeof(DWORD) * m_nBuffer );
		}
		m_pList[ m_nCount++ ] = nItem;
		return (POSITION)m_nCount;
	}
	
	inline POSITION CheckAndAdd(DWORD nItem)
	{
		return ( Find( nItem ) == NULL ) ? AddTail( nItem ) : NULL;
	}

	inline DWORD RemoveHead()
	{
		ASSERT( m_nCount > 0 );
		DWORD nItem = m_pList[ 0 ];
		m_nCount--;
		MoveMemory( m_pList, m_pList + 1, sizeof(DWORD) * m_nCount );
		return nItem;
	}

	inline DWORD RemoveTail()
	{
		ASSERT( m_nCount > 0 );
		return m_pList[ --m_nCount ];
	}
	
	inline void RemoveAt(POSITION pos)
	{
		int nPos = (int)pos;
		ASSERT( nPos > 0 && nPos <= m_nCount );
		MoveMemory( m_pList + nPos - 1, m_pList + nPos, sizeof(DWORD) * ( m_nCount - nPos ) );
		m_nCount--;
	}
	
	inline void RemoveAll()
	{
		m_nCount = 0;
	}

	inline POSITION Find(DWORD nItem) const
	{
		DWORD* pSeek = m_pList;
		for ( int nCount = m_nCount ; nCount ; nCount--, pSeek++ )
		{
			if ( *pSeek == nItem ) return (POSITION)( m_nCount - nCount + 1 );
		}
		return NULL;
	}

// Operations
public:
	CLibraryFile*	GetNextFile(POSITION& pos) const;
	int				Merge(CLibraryList* pList);
	

// Automation
protected:
	BEGIN_INTERFACE_PART(GenericView, IGenericView)
		DECLARE_DISPATCH()
		STDMETHOD(get_Name)(BSTR FAR* psName);
		STDMETHOD(get_Unknown)(IUnknown FAR* FAR* ppUnknown);
		STDMETHOD(get_Param)(LONG FAR* pnParam);
		STDMETHOD(get_Count)(LONG FAR* pnCount);
		STDMETHOD(get_Item)(VARIANT vIndex, VARIANT FAR* pvItem);
		STDMETHOD(get__NewEnum)(IUnknown FAR* FAR* ppEnum);
	END_INTERFACE_PART(GenericView)

	BEGIN_INTERFACE_PART(EnumVARIANT, IEnumVARIANT)
		STDMETHOD(Next)(THIS_ DWORD celt, VARIANT FAR* rgvar, DWORD FAR* pceltFetched);
		STDMETHOD(Skip)(THIS_ DWORD celt);
		STDMETHOD(Reset)(THIS);
		STDMETHOD(Clone)(THIS_ IEnumVARIANT FAR* FAR* ppenum);
		POSITION m_pos;
	END_INTERFACE_PART(EnumVARIANT)
	
	DECLARE_INTERFACE_MAP()

};

#endif // !defined(AFX_LIBRARYLIST_H__C045FFC3_F813_4962_94D0_FC5E9E4A7BA3__INCLUDED_)
