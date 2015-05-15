//
// SchemaMember.h
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

#pragma once

class CSchema;
class CXMLElement;


class CSchemaMember
{
public:
	CSchemaMember(CSchema* pSchema);

	enum Format
	{
		smfNone, smfTimeMMSS, smfBitrate, smfFrequency, smfTimeHHMMSSdec
	};

	CString		m_sName;
	CString		m_sType;
	CString		m_sTitle;
	BOOL		m_bElement;
	BOOL		m_bNumeric;
	BOOL		m_bGUID;
	BOOL		m_bYear;
	BOOL		m_bIndexed;
	BOOL		m_bSearched;
	BOOL		m_bHidden;
	BOOL		m_bBoolean;

	int			m_nMinOccurs;
	int			m_nMaxOccurs;
	int			m_nMaxLength;

	BOOL		m_bPrompt;
	Format		m_nFormat;
	int			m_nColumnWidth;
	int			m_nColumnAlign;

	CString		m_sLinkURI;
	CString		m_sLinkName;

	inline POSITION GetItemIterator() const { return m_pItems.GetHeadPosition(); }
	inline CString GetNextItem(POSITION& pos) const { return m_pItems.GetNext( pos ); }
	inline INT_PTR GetItemCount() const { return m_pItems.GetCount(); }
	inline bool IsEqual(CSchemaPtr pSchema) const { return ( pSchema == m_pSchema ); }

	BOOL		LoadSchema(const CXMLElement* pRoot, const CXMLElement* pElement);
	BOOL		LoadDescriptor(const CXMLElement* pXML);
	CString		GetValueFrom(const CXMLElement* pElement, LPCTSTR pszDefault = NULL, BOOL bFormat = FALSE, BOOL bNoValidation = FALSE) const;
	void		SetValueTo(CXMLElement* pBase, const CString& strValue = CString(), BOOL bFormat = FALSE) const;

protected:
	CSchema*			m_pSchema;
	CList< CString >	m_pItems;

	void		AddItem(const CString& strItem);
	BOOL		LoadType(const CXMLElement* pType);
	BOOL		LoadDisplay(const CXMLElement* pXML);

private:
	CSchemaMember(const CSchemaMember&);
	CSchemaMember& operator=(const CSchemaMember&);
};

typedef const CSchemaMember* CSchemaMemberPtr;

typedef CList< CSchemaMemberPtr > CSchemaMemberList;

#ifdef _WIN64

template<>
AFX_INLINE UINT AFXAPI HashKey( CSchemaMemberPtr key )
{
	return HashKey< __int64 >( (__int64)key );
}

#endif // _WIN64
