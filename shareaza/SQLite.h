//
// SQLite.h
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

// Example:
//	auto_ptr< CDatabase* > db( theApp.GetDatabase() );
//	if ( db->Prepare( _T("SELECT Number FROM Table;") ) )
//	{
//		while( db->Step() || db->IsBusy() )
//		{
//			if ( db->IsBusy() )
//			{
//				Sleep( 1000 );
//				continue;
//			}
//			int n = db->GetInt32( _T("Number") );
//		}
//	}

#pragma once

struct sqlite3;
struct sqlite3_stmt;


class CDatabase
{
public:
	CDatabase(LPCWSTR szDatabase);
	~CDatabase();

	// Return true if database successfully opened
	operator bool() const throw();
	// Return database last error message
	CString			GetLastErrorMessage() const;
	// Execute multiple queries without parameters
	bool			Exec(LPCTSTR szQuery);
	// Prepare single query
	bool			Prepare(LPCTSTR szQuery);
	// Run one query iteration
	bool			Step();
	// Finalize query
	void			Finalize();
	// Return true if latest SQL call failed because of a locked table state
	bool			IsBusy() const throw();
	// Return the number of values in the current row of the result set
	int				GetCount() const throw();
	// Return column type in the current row of the result set
	int				GetType(LPCTSTR pszName) const;

	__int32			GetInt32(LPCTSTR pszName) const;
	__int64			GetInt64(LPCTSTR pszName) const;
	double			GetDouble(LPCTSTR pszName) const;
	CString			GetString(LPCTSTR pszName) const;
	LPCVOID			GetBlob(LPCTSTR pszName, int* pnLength) const;

	// Note: The first parameter has an index of 1
	bool			Bind(int nIndex, __int32 nData);
	bool			Bind(int nIndex, __int64 nData);
	bool			Bind(int nIndex, double dData);
	bool			Bind(int nIndex, LPCTSTR sData);
	bool			Bind(int nIndex, LPCVOID pData, int nLength);

protected:
	typedef CMap< CString, const CString&, int, int > CRaw;

	sqlite3*		m_db;			// Handle to SQL database
	sqlite3_stmt*	m_st;			// SQL statement handle
	bool			m_bBusy;		// Last SQL call returned with busy error
	CString			m_sQuery;		// SQL query
	CRaw			m_raw;			// Column name to column number map

#ifdef _DEBUG
	DWORD			m_nThread;		// Thread ID (debug only)
#endif

	// Prepare single query
	bool			PrepareHelper();
	// Return column index by column name
	int				GetColumn(LPCTSTR pszName) const;

private:
	CDatabase(const CDatabase&);
	CDatabase& operator=(const CDatabase&);
};
