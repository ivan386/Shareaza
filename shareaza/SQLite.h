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
//
//	SQLite::CDatabase db( _T("C:\\Database.db3") );
//	SQLite::CStatement st( db, _T("SELECT Number FROM Table;") );
//	while( st.Step() || st.IsBusy() )
//	{
//		if ( st.IsBusy() )
//		{
//			Sleep( 1000 );
//			continue;
//		}
//		int n = st.GetInt( _T("Number") );
//	}

#pragma once

namespace SQLite {

#include "../sqlite3/sqlite3.h"

class CStatement;
class CDatabase;


class CDatabase : private boost::noncopyable
{
public:
	CDatabase(LPCWSTR szDatabasePath = NULL);
	virtual ~CDatabase();

	operator bool() const;

	class CSQLitePtr : private boost::noncopyable
	{
	public:
		CSQLitePtr(LPCWSTR szDatabasePath);
		virtual ~CSQLitePtr();

		sqlite3*	m_db;
	};
	typedef std::tr1::shared_ptr< CSQLitePtr > CSQLiteSharedPtr;

	CSQLiteSharedPtr	GetHandle() const;
	LPCWSTR				GetLastErrorMessage() const;
	bool				Open(LPCWSTR szDatabasePath);
	bool				Exec(LPCWSTR szQuery);

protected:
	CSQLiteSharedPtr	m_db;
};


class CStatement : private boost::noncopyable
{
public:
	CStatement(const CDatabase& db, LPCWSTR szQuery);
	virtual ~CStatement();

	operator bool() const;

	bool			Step();
	bool			Prepare();
	void			Finalize();
	void			Reset();

	bool			IsPending() const;

	// Return true if latest SQL call failed because of a locked table state
	bool			IsBusy() const;

	// Return the number of values in the current row of the result set
	int				GetCount() const;

	// Return column type in the current row of the result set
	int				GetType(LPCWSTR pszName) const;

	int				GetInt(LPCWSTR pszName) const;
	__int64			GetInt64(LPCWSTR pszName) const;
	double			GetDouble(LPCWSTR pszName) const;
	LPCWSTR			GetString(LPCWSTR pszName) const;
	LPCVOID			GetBlob(LPCWSTR pszName, int* pnLength) const;

	bool			Bind(int nNumber, int nData);
	bool			Bind(int nNumber, __int64 nData);
	bool			Bind(int nNumber, double dData);
	bool			Bind(int nNumber, LPCWSTR sData);
	bool			Bind(int nNumber, LPCVOID pData, int nLength);

protected:
	typedef std::map< std::wstring, int > CRaw;

	CDatabase::CSQLiteSharedPtr	m_db;			// Handle to database
	std::wstring				m_query;		// SQL query (UTF16)
	sqlite3_stmt*				m_st;			// SQL statement handle
	bool						m_prepared;		// Prepare was called successfully
	bool						m_busy;			// Last SQL call returned with busy error
	CRaw						m_raw;			// Column name to column number map

	// Return column index by column name
	int				GetColumn(LPCWSTR pszName) const;
	// Check column index for validity
	bool			IsValidIndex(int nIndex) const;
};

}	// namespace SQLite
