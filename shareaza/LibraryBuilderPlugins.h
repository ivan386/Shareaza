//
// LibraryBuilderPlugins.h
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

#if !defined(AFX_LIBRARYBUILDERPLUGINS_H__9D5B1BBA_DED4_42C9_89FE_FC3244779663__INCLUDED_)
#define AFX_LIBRARYBUILDERPLUGINS_H__9D5B1BBA_DED4_42C9_89FE_FC3244779663__INCLUDED_

#pragma once

class CLibraryBuilder;
interface ILibraryBuilderPlugin;


class CLibraryBuilderPlugins  
{
// Construction
public:
	CLibraryBuilderPlugins(CLibraryBuilder* pBuilder);
	virtual ~CLibraryBuilderPlugins();
	
// Attributes
public:
	CMapStringToPtr		m_pMap;
	CLibraryBuilder*	m_pBuilder;
	BOOL				m_bCOM;

// Operations
public:
	BOOL	ExtractMetadata(CString& strPath, HANDLE hFile);
	void	Cleanup();
protected:
	ILibraryBuilderPlugin*	LoadPlugin(LPCTSTR pszType);

};

#endif // !defined(AFX_LIBRARYBUILDERPLUGINS_H__9D5B1BBA_DED4_42C9_89FE_FC3244779663__INCLUDED_)
