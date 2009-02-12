//
// EDPartImporter.h
//
// Copyright © Shareaza Development Team, 2002-2009.
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

#include "ShareazaThread.h"


class CEDPartImporter : public CRazaThread
{
public:
	CEDPartImporter();
	virtual ~CEDPartImporter();

	DECLARE_DYNAMIC(CEDPartImporter)

	void	AddFolder(LPCTSTR pszFolder);
	void	Start(CEdit* pCtrl);
	void	Stop();
	BOOL	IsRunning();

protected:
	CList< CString >	m_pFolders;
	CEdit	*	m_pTextCtrl;
	int			m_nCount;

	void	ImportFolder(LPCTSTR pszPath);
	BOOL	ImportFile(LPCTSTR pszPath, LPCTSTR pszFile);
	BOOL	CopyFile(LPCTSTR pszSource, LPCTSTR pszTarget);
	void	Message(UINT nMessageID, ...);

	virtual int Run();

	DECLARE_MESSAGE_MAP()
};
