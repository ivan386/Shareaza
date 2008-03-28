//
// BENode.h
//
// Copyright (c) Shareaza Development Team, 2007.
// This file is part of Shareaza Torrent Wizard (shareaza.sourceforge.net).
//
// Shareaza Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Torrent Wizard is distributed in the hope that it will be useful,
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

class CHashSHA1;
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
	CHashSHA1	GetSHA1() const;
	void		Encode(CBuffer* pBuffer) const;
#ifdef _BE_DECODE_
public:
	static CBENode*	Decode(CBuffer* pBuffer);
private:
	void		Decode(LPBYTE& pInput, DWORD& nInput);
	static int	DecodeLen(LPBYTE& pInput, DWORD& nInput);
#endif
private:
	static int __cdecl SortDict(const void * pA, const void * pB);

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

		return str;
	}
	
	inline void SetString(LPCTSTR psz)
	{
		Clear();
		m_nType		= beString;	
		m_pValue	= MakeStr( psz, FALSE );
		m_nValue	= strlen( (LPCSTR)m_pValue );
	}
	
	inline void SetString(LPCVOID pString, DWORD nLength)
	{
		Clear();
		m_nType		= beString;
		m_nValue	= (QWORD)nLength;
		m_pValue	= new BYTE[ nLength ];
		CopyMemory( m_pValue, pString, nLength );
	}
	
	inline CBENode* Add(LPCSTR psz = NULL)
	{
		return Add( (LPBYTE)psz, psz ? (int)strlen(psz) : 0 );
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
	
	static inline LPVOID MakeStr(LPCTSTR psz, BOOL bNull)
	{
		int nLen = WideCharToMultiByte( CP_UTF8,  0, psz, -1, NULL, 0, NULL, NULL ) + ( bNull ? 1 : 0 );
		LPSTR pStr	= new CHAR[ nLen ];
		WideCharToMultiByte( CP_UTF8, 0, psz, -1, pStr, nLen, NULL, NULL );
		return pStr;
	}
};

#endif // !defined(AFX_BENODE_H__8E447816_68F5_461A_A032_09A0CB97F3CB__INCLUDED_)
