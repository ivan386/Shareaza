//
// Plugins.h
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

#if !defined(AFX_PLUGINS_H__3EDF2C68_FCEB_438E_A5CD_CF5DC1114FC4__INCLUDED_)
#define AFX_PLUGINS_H__3EDF2C68_FCEB_438E_A5CD_CF5DC1114FC4__INCLUDED_

#pragma once

class CPlugin;
class CChildWnd;


class CPlugins  
{
// Construction
public:
	CPlugins();
	virtual ~CPlugins();
	
// Attributes
public:
	CPtrList	m_pList;
	UINT		m_nCommandID;

// Operations
public:
	void		Enumerate();
	void		Clear();
	BOOL		LookupCLSID(LPCTSTR pszGroup, LPCTSTR pszKey, CLSID& pCLSID, BOOL bEnableDefault = TRUE);
	BOOL		LookupEnable(REFCLSID pCLSID, BOOL bDefault);
public:
	void		OnSkinChanged();
	void		RegisterCommands();
	UINT		GetCommandID();
	BOOL		OnUpdate(CChildWnd* pActiveWnd, CCmdUI* pCmdUI);
	BOOL		OnCommand(CChildWnd* pActiveWnd, UINT nCommandID);
	BOOL		OnExecuteFile(LPCTSTR pszFile);
	BOOL		OnEnqueueFile(LPCTSTR pszFile);

// Inlines
public:
	inline POSITION GetIterator() const
	{
		return m_pList.GetHeadPosition();
	}

	inline CPlugin* GetNext(POSITION& pos) const
	{
		return (CPlugin*)m_pList.GetNext( pos );
	}

	inline int GetCount() const
	{
		return m_pList.GetCount();
	}

};


class CPlugin
{
// Construction
public:
	CPlugin(REFCLSID pCLSID, LPCTSTR pszName);
	virtual ~CPlugin();
	
// Attributes
public:
	CLSID		m_pCLSID;
	CString		m_sName;
	HICON		m_hIcon;
	DWORD		m_nCapabilities;
public:
	IGeneralPlugin*	m_pPlugin;
	ICommandPlugin*	m_pCommand;
	IExecutePlugin*	m_pExecute;
	
// Operations
public:
	BOOL		Start();
	void		Stop();
	BOOL		StartIfEnabled();
	CString		GetStringCLSID() const;
protected:
	HICON		LookupIcon();

};

extern CPlugins Plugins;

#endif // !defined(AFX_PLUGINS_H__3EDF2C68_FCEB_438E_A5CD_CF5DC1114FC4__INCLUDED_)
