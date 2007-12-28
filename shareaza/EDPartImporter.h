//
// EDPartImporter.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#if !defined(AFX_EDPARTIMPORTER_H__C40852E2_4CA4_458D_9B00_3301A02AF029__INCLUDED_)
#define AFX_EDPARTIMPORTER_H__C40852E2_4CA4_458D_9B00_3301A02AF029__INCLUDED_

#pragma once


class CEDPartImporter : public CWinThread
{
// Construction
public:
	CEDPartImporter();
	virtual ~CEDPartImporter();

	DECLARE_DYNAMIC(CEDPartImporter)

// Attributes
protected:
	CList< CString >	m_pFolders;
	CEdit	*	m_pTextCtrl;
	int			m_nCount;

// Operations
public:
	void	AddFolder(LPCTSTR pszFolder);
	void	Start(CEdit* pCtrl);
	void	Stop();
	BOOL	IsRunning();
protected:
	void	ImportFolder(LPCTSTR pszPath);
	BOOL	ImportFile(LPCTSTR pszPath, LPCTSTR pszFile);
	BOOL	CopyFile(LPCTSTR pszSource, LPCTSTR pszTarget);
	void	Message(UINT nMessageID, ...);

// Overrides
public:
	//{{AFX_VIRTUAL(CEDPartImporter)
	public:
	virtual BOOL InitInstance();
	virtual int Run();
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CEDPartImporter)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_EDPARTIMPORTER_H__C40852E2_4CA4_458D_9B00_3301A02AF029__INCLUDED_)
