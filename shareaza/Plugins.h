//
// Plugins.h
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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

#include "ThreadImpl.h"

class CChildWnd;
class CLibraryFile;
class CPlugin;


class CPlugins : public CThreadImpl
{
public:
	CPlugins();

	CMutex		m_pSection;

	// Register all plugins in this folder
	BOOL		Register(const CString& sGeneralPath);

	void		Enumerate();
	void		Clear();
	BOOL		LookupCLSID(LPCTSTR pszGroup, LPCTSTR pszKey, REFCLSID pCLSID) const;
	BOOL		LookupEnable(REFCLSID pCLSID, LPCTSTR pszExt = NULL) const;

	// Load non-generic plugin and save it to cache
	IUnknown*	GetPlugin(LPCTSTR pszGroup, LPCTSTR pszType);

	// Reload non-generic plugin within cache
	BOOL		ReloadPlugin(LPCTSTR pszGroup, LPCTSTR pszType);

	// Unload plugin (from cache and generic plugin list)
	void		UnloadPlugin(REFCLSID pCLSID);

	// Retrieve next free command ID
	UINT		GetCommandID();

	// IGeneralPlugin mirroring
	void		OnSkinChanged();

	// ICommandPlugin mirroring
	void		RegisterCommands();
	void		InsertCommands();
	BOOL		OnUpdate(CChildWnd* pActiveWnd, CCmdUI* pCmdUI);
	BOOL		OnCommand(CChildWnd* pActiveWnd, UINT nCommandID);

	// IExecutePlugin mirroring
	BOOL		OnExecuteFile(LPCTSTR pszFile, BOOL bUseImageViewer = FALSE);
	BOOL		OnEnqueueFile(LPCTSTR pszFile);

	// IChatPlugin mirroring
	BOOL		OnChatMessage(LPCTSTR pszChatID, BOOL bOutgoing, LPCTSTR pszFrom, LPCTSTR pszTo, LPCTSTR pszMessage);

	// ILibraryPlugin mirroring
	BOOL		OnNewFile(CLibraryFile* pFile);

	inline POSITION GetIterator() const
	{
		return m_pList.GetHeadPosition();
	}

	inline CPlugin* GetNext(POSITION& pos) const
	{
		return m_pList.GetNext( pos );
	}

private:
	typedef struct
	{
		CComGITPtr< IUnknown >	m_pGIT;
		CComPtr< IUnknown >		m_pIUnknown;
	} CPluginPtr;
	typedef CMap< CLSID, const CLSID&, CPluginPtr*, CPluginPtr* > CPluginMap;

	CPluginMap			m_pCache;		// Non-generic plugin cache
	CList< CPlugin* >	m_pList;		// Generic plugins
	UINT				m_nCommandID;	// First free command ID
	CLSID				m_inCLSID;		// [in] Create this interface
	CEvent				m_pReady;		// Interface creation completed

	virtual void OnRun();

	CPlugins(const CPlugins&);
	CPlugins& operator=(const CPlugins&);
};


class CPlugin
{
public:
	CPlugin(REFCLSID pCLSID, LPCTSTR pszName);
	~CPlugin();

	CLSID						m_pCLSID;
	CString						m_sName;
	DWORD						m_nCapabilities;
	CComPtr< IGeneralPlugin >	m_pPlugin;
	CComPtr< ICommandPlugin >	m_pCommand;
	CComPtr< IExecutePlugin >	m_pExecute;
	CComPtr< IChatPlugin >		m_pChat;
	CComPtr< ILibraryPlugin >	m_pLibrary;

	BOOL		Start();
	void		Stop();
	CString		GetStringCLSID() const;
	HICON		LookupIcon() const;

private:
	CPlugin(const CPlugin&);
	CPlugin& operator=(const CPlugin&);
};

template<> AFX_INLINE UINT AFXAPI HashKey(const CLSID& key)
{
	return ( key.Data1 + MAKEDWORD( key.Data2, key.Data3 ) +
		*(UINT*)&key.Data4[0] + *(UINT*)&key.Data4[4] ) & 0xffffffff;
}

extern CPlugins Plugins;
