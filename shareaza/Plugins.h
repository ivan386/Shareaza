//
// Plugins.h
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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

class CPlugin;
class CChildWnd;


class CPlugins
{
public:
	CPlugins();
	virtual ~CPlugins();

public:
	CList< CPlugin* >	m_pList;
	UINT				m_nCommandID;

public:
	void		Enumerate();
	void		Clear();
	BOOL		LookupCLSID(LPCTSTR pszGroup, LPCTSTR pszKey, CLSID& pCLSID, BOOL bEnableDefault = TRUE) const;
	BOOL		LookupEnable(REFCLSID pCLSID, BOOL bDefault, LPCTSTR pszExt = NULL) const;
	CPlugin*	Find(REFCLSID pCLSID) const;
	void		OnSkinChanged();
	void		InsertCommands();
	void		RegisterCommands();
	UINT		GetCommandID();
	BOOL		OnUpdate(CChildWnd* pActiveWnd, CCmdUI* pCmdUI);
	BOOL		OnCommand(CChildWnd* pActiveWnd, UINT nCommandID);
	BOOL		OnExecuteFile(LPCTSTR pszFile, BOOL bUseImageViewer = FALSE);
	BOOL		OnEnqueueFile(LPCTSTR pszFile);

	inline POSITION GetIterator() const
	{
		return m_pList.GetHeadPosition();
	}

	inline CPlugin* GetNext(POSITION& pos) const
	{
		return m_pList.GetNext( pos );
	}

	inline INT_PTR GetCount() const
	{
		return m_pList.GetCount();
	}
};


class CPlugin
{
public:
	CPlugin(REFCLSID pCLSID, LPCTSTR pszName);
	virtual ~CPlugin();

public:
	CLSID			m_pCLSID;
	CString			m_sName;
	DWORD			m_nCapabilities;
	IGeneralPlugin*	m_pPlugin;
	ICommandPlugin*	m_pCommand;
	IExecutePlugin*	m_pExecute;

public:
	BOOL		Start();
	void		Stop();
	BOOL		StartIfEnabled();
	CString		GetStringCLSID() const;
	HICON		LookupIcon() const;
};

extern CPlugins Plugins;
