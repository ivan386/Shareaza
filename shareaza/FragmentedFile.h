//
// FragmentedFile.h
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

#if !defined(AFX_FRAGMENTEDFILE_H__03EF9CB6_10EC_43A5_B6E0_D74B54044B51__INCLUDED_)
#define AFX_FRAGMENTEDFILE_H__03EF9CB6_10EC_43A5_B6E0_D74B54044B51__INCLUDED_

#pragma once

#include "FileFragment.h"

class CTransferFile;
class CEDPartImporter;


class CFragmentedFile
{
// Construction
public:
	CFragmentedFile();
	virtual ~CFragmentedFile();

// Attributes
protected:
	CTransferFile*	m_pFile;
	QWORD			m_nUnflushed;
public:
	QWORD			m_nTotal;
	CFileFragmentList m_oFree;
	QWORD			m_nShift;
	
// Operations
public:
	BOOL			Create(LPCTSTR pszFile, QWORD nLength);
	BOOL			Open(LPCTSTR pszFile);
	BOOL			Flush();
	void			Close();
	void			Clear();
	BOOL			MakeComplete();
	void			Serialize(CArchive& ar, int nVersion);
public:
	BOOL			WriteRange(QWORD nOffset, LPCVOID pData, QWORD nLength);
	BOOL			ReadRange(QWORD nOffset, LPVOID pData, QWORD nLength);
	BOOL			ReadRangeUnlimited(QWORD nOffset, LPVOID pData, QWORD nLength);
	QWORD			InvalidateRange(const QWORD nOffset, const QWORD nLength);
	void			InvalidateRange(const CFileFragmentList& Corrupted);
// Operations
public:
	inline BOOL IsValid() const;
	inline BOOL IsOpen() const;
	inline QWORD GetTotal() const;
	inline QWORD GetRemaining() const;
	inline QWORD GetCompleted() const;
	inline QWORD GetEmptyFragmentCount() const;
	inline BOOL IsFlushNeeded() const;
	
};

inline BOOL CFragmentedFile::IsValid() const
{
	return ( this != NULL ) && ( m_nTotal > 0 );
}
	
inline BOOL CFragmentedFile::IsOpen() const
{
	return ( this != NULL ) && ( m_pFile != NULL );
}
	
inline QWORD CFragmentedFile::GetTotal() const
{
	return m_nTotal;
}
	
inline QWORD CFragmentedFile::GetRemaining() const
{
	return m_oFree.GetSize();
}
	
inline QWORD CFragmentedFile::GetCompleted() const
{
	return IsValid() ? m_nTotal - m_oFree.GetSize() : 0;
}
	
inline BOOL CFragmentedFile::IsFlushNeeded() const
{
	return ( m_pFile != NULL ) && ( m_nUnflushed > 0 );
}

#endif // !defined(AFX_FRAGMENTEDFILE_H__03EF9CB6_10EC_43A5_B6E0_D74B54044B51__INCLUDED_)