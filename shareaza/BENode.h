//
// BENode.h
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

#if !defined(AFX_BENODE_H__8E447816_68F5_461A_A032_09A0CB97F3CB__INCLUDED_)
#define AFX_BENODE_H__8E447816_68F5_461A_A032_09A0CB97F3CB__INCLUDED_

#pragma once

class CBuffer;


class CBENode  
{
// Construction
public:
	CBENode();
	~CBENode();

// Attributes
public:
	int			m_nType;
	LPVOID		m_pValue;
	QWORD		m_nValue;
	
	enum { beNull, beString, beInt, beList, beDict };
	
// Operations
public:
	void		Clear();
	CBENode*	Add(const LPBYTE pKey, int nKey);
	CBENode*	GetNode(LPCSTR pszKey) const;
	CBENode*	GetNode(const LPBYTE pKey, int nKey) const;
	void		GetSHA1(SHA1* pSHA1) const;
	void		Encode(CBuffer* pBuffer) const;
public:
	static CBENode*	Decode(CBuffer* pBuffer);
private:
	void		Decode(LPBYTE& pInput, DWORD& nInput);
	static int	DecodeLen(LPBYTE& pInput, DWORD& nInput);


// Inline
public:
	inline bool IsType(int nType) const
	{
		if ( this == NULL ) return false;
		return m_nType == nType;
	}
	
	inline QWORD GetInt() const
	{
		if ( m_nType != beInt ) return 0;
		return m_nValue;
	}
	
	inline void SetInt(QWORD nValue)
	{
		Clear();
		m_nType		= beInt;
		m_nValue	= nValue;
	}
	
	inline CString GetString() const
	{
		CString str;
		if ( m_nType != beString ) return str;
		str = (LPCSTR)m_pValue;
		int nSource = str.GetLength();
		int nLength = MultiByteToWideChar( CP_UTF8, 0, (LPCSTR)m_pValue, nSource, NULL, 0 );
#ifdef _UNICODE
		MultiByteToWideChar( CP_UTF8, 0, (LPCSTR)m_pValue, nSource, str.GetBuffer( nLength ), nLength );
		str.ReleaseBuffer( nLength );
#else
		LPWSTR pszWide = new WCHAR[ nLength + 1 ];
		MultiByteToWideChar( CP_UTF8, 0, (LPCSTR)m_pValue, nSource, pszWide, nLength );
		pszWide[ nLength ] = 0;
		str = pszWide;
		delete [] pszWide;
#endif

		return str;
	}

	inline void SetString(LPCSTR psz)
	{
		SetString( psz, strlen(psz), TRUE );
	}
	
	void SetString(LPCWSTR psz)
	{
		USES_CONVERSION;
		LPCSTR pszASCII = W2CA(psz);
		SetString( pszASCII, strlen(pszASCII), TRUE );
	}
	
	inline void SetString(LPCVOID pString, DWORD nLength, BOOL bNull = FALSE)
	{
		Clear();
		m_nType		= beString;
		m_nValue	= (QWORD)nLength;
		m_pValue	= new BYTE[ nLength + ( bNull ? 1 : 0 ) ];
		CopyMemory( m_pValue, pString, nLength + ( bNull ? 1 : 0 ) );
	}
	
	inline CBENode* Add(LPCSTR pszKey = NULL)
	{
		return Add( (LPBYTE)pszKey, pszKey != NULL ? strlen( pszKey ) : 0 );
	}
	
	inline int GetCount() const
	{
		if ( m_nType != beList && m_nType != beDict ) return 0;
		return (int)m_nValue;
	}
	
	inline CBENode* GetNode(int nItem) const
	{
		if ( m_nType != beList && m_nType != beDict ) return NULL;
		if ( m_nType == beDict ) nItem *= 2;
		if ( nItem < 0 || nItem >= (int)m_nValue ) return NULL;
		return *( (CBENode**)m_pValue + nItem );
	}
};

#endif // !defined(AFX_BENODE_H__8E447816_68F5_461A_A032_09A0CB97F3CB__INCLUDED_)
