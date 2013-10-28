//
// ImageViewerPlugin.cpp
//
// This software is released into the public domain. You are free to
// redistribute and modify without any restrictions.
// This file is part of SHAREAZA (shareaza.sourceforge.net), original author Michael Stokes. 
//
// This file contains the CImageViewerPlugin class, which is the "plugin object".
// It is created by Shareaza when the plugin is loaded or enabled by the user, and
// destroyed when the application is closed or the plugin is disabled.
//
// This is a "general plugin", so it implements the IGeneralPlugin interface.  General
// plugins are always invoked from the GUI thread.
//
// The image viewer needs to capture the "open file" event so that it can open the
// applicable image file in a viewer window.  This is achieved by implementing the
// IExecutePlugin interface, which has OnExecute() and OnEnqueue() methods that can
// override Shareaza's default file-opening behaviour.
//
// The ICommandPlugin interface is also implemented, which allows the image viewer to
// register its own user interface commands, and respond to them when the user
// invokes them.
//

#include "StdAfx.h"
#include "ImageViewerPlugin.h"
#include "ImageWindow.h"


/////////////////////////////////////////////////////////////////////////////
// CImageViewerPlugin construction

CImageViewerPlugin::CImageViewerPlugin()
{
	// We will maintain a list of open CImageWindow objects as a linked list
	m_pWindow	= NULL;
	
	// Load the "move / grab" cursor from the DLL's resources
	m_hcMove	= LoadCursor( _AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDC_GRABMOVE) );
}

/////////////////////////////////////////////////////////////////////////////
// CImageViewerPlugin destruction

CImageViewerPlugin::~CImageViewerPlugin()
{
	// If there are CImageWindow windows open, we must close them now as the plugin is being
	// destroyed.  If this isn't done, the windows will be left behind and will become
	// unstable.  Simply walk through the linked list and call DestroyWindow()
	
	while ( m_pWindow != NULL )
	{
		CImageWindow* pNext = m_pWindow->m_pNext;
		m_pWindow->m_pPlugin = NULL;
		m_pWindow->DestroyWindow();
		m_pWindow = pNext;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CImageViewerPlugin IGeneralPlugin implementation

HRESULT STDMETHODCALLTYPE CImageViewerPlugin::SetApplication(IApplication __RPC_FAR *pApplication)
{
	// This method is invoked as soon as the plugin object has been created, to pass a reference
	// to the core Application object
	
	// Save the IApplication interface in a member variable (it will be AddRef'ed)
	m_pApplication = pApplication;
	
	// Get the user interface manager (IUserInterface) and store it in a member variable
	m_pApplication->get_UserInterface( &m_pInterface );
	
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CImageViewerPlugin::QueryCapabilities(DWORD __RPC_FAR* /*pnCaps*/)
{
	// This method is not currently used, please return S_OK and do not modify pnCaps
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CImageViewerPlugin::Configure()
{
	// This method is invoked if the user selects the "Setup" command from the Plugin Settings
	// page.
	
	// Here we simply load a string from the string table and display it in a MessageBox
	TCHAR szMessage[1024];
	LoadString( _AtlBaseModule.GetResourceInstance(), IDS_ABOUT, szMessage, 1024 );
	MessageBox( GetActiveWindow(), szMessage, _T("Image Viewer Plugin"), MB_ICONINFORMATION );
	
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CImageViewerPlugin::OnSkinChanged()
{
	// This method is invoked to allow the plugin to handle a "skin changed" event.  The plugin
	// should destroy any skin-based resources it has acquired, and re-acquire them from the
	// user interface manager.  It is also invoked when a new langauge is selected (languages
	// are special skins), and after plugins have been loaded/unloaded.
	
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CImageViewerPlugin IExecutePlugin implementation

HRESULT STDMETHODCALLTYPE CImageViewerPlugin::OnExecute(BSTR sFilePath)
{
	// The OnExecute method is invoked whenever Shareaza needs to execute (open) a file.  This can
	// be from the library, from the downloads list, the uploads list, etc.  The path of the file is
	// provided as an argument.  Return S_OK if you have handled the execution, or S_FALSE if Shareaza
	// should keep looking for someone to handle it (and potentially fall back to its internal handler).
	// Failure codes (E_*) are interpreted as an inability to open the file, and an error message will
	// be displayed.  So don't return failures (E_*) unless you should be able to open the file, but
	// can't.  S_FALSE is the correct code if you don't want to open this kind of file.
	
	// String conversion macros
	
	USES_CONVERSION;
	
	// Convert the BSTR to a LPCTSTR, and locate the file extension
	
	LPCTSTR pszFilePath = OLE2T( sFilePath );
	LPCTSTR pszFileType = _tcsrchr( pszFilePath, '.' );
	
	// If there was no file extension, this file is not for us
	
	if ( pszFileType == NULL ) return S_FALSE;
	
	// This image viewer plugin attempts to decide whether or not it should open the file
	// based on whether or not there is an ImageService plugin available for the image
	// file type.  HOWEVER -- there are ImageServices available for some video types, too
	// (they produce a thumbnail) -- but we DON'T want to open videos here -- they should
	// be left for the media player.
	//
	// SO, we check some common video file types here, and return S_FALSE if we get a match.
	
	if ( lstrcmpi( pszFileType, _T(".avi") ) == 0 ) return S_FALSE;
	if ( lstrcmpi( pszFileType, _T(".asf") ) == 0 ) return S_FALSE;
	if ( lstrcmpi( pszFileType, _T(".div") ) == 0 ) return S_FALSE;
	if ( lstrcmpi( pszFileType, _T(".divx") ) == 0 ) return S_FALSE;
	if ( lstrcmpi( pszFileType, _T(".mpg") ) == 0 ) return S_FALSE;
	if ( lstrcmpi( pszFileType, _T(".mpeg") ) == 0 ) return S_FALSE;
	if ( lstrcmpi( pszFileType, _T(".nsv") ) == 0 ) return S_FALSE;
	if ( lstrcmpi( pszFileType, _T(".mov") ) == 0 ) return S_FALSE;
	if ( lstrcmpi( pszFileType, _T(".ogm") ) == 0 ) return S_FALSE;
	if ( lstrcmpi( pszFileType, _T(".wmv") ) == 0 ) return S_FALSE;
	
	// Okay, so we're assuming now that it's not a video file.  The next (and primary) step
	// is to check if there is an ImageService plugin available for this file type.  This is
	// done by checking a registry key:
	
	if ( lstrcmpi( pszFileType, _T(".partial") ) != 0 )
	{
		DWORD dwCount = 128;
		TCHAR szValue[128];
		CRegKey pReg;

		if ( pReg.Open( HKEY_CURRENT_USER,
			_T("SOFTWARE\\Shareaza\\Shareaza\\Plugins\\ImageService") ) != ERROR_SUCCESS )
			return S_FALSE;

		if ( pReg.QueryValue( pszFileType, NULL, szValue, &dwCount ) != ERROR_SUCCESS )
			return S_FALSE;

		pReg.Close();
	}

	// If we made it to this point, there was indeed an ImageService plugin for the file type,
	// so we should have a go at opening it.  Delegate to our OpenNewWindow() function to
	// select or create the image window.
	
	OpenNewWindow( pszFilePath );
	
	// Return S_OK, because we have successfully opened this file (we hope).
	
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CImageViewerPlugin::OnEnqueue(BSTR /*sFilePath*/)
{
	// The OnEnqueue method is invoked whenever Shareaza needs to enqueue a file ("add to playlist").
	// The path of the file is provided as an argument.  Return S_OK if you have handled the execution,
	// or S_FALSE if Shareaza should keep looking for someone to handle it (and potentially fall back
	// to its internal handler).  Failure codes (E_*) are interpreted as an inability to open the file,
	// and an error message will be displayed.  So don't return failures (E_*) unless you should be able
	// to open the file, but can't.  S_FALSE is the correct code if you don't want to open this kind of
	// file.
	
	// The image viewer does not enqueue files, so we return S_FALSE (keep looking).
	
	return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CImageViewerPlugin ICommandPlugin implementation

HRESULT STDMETHODCALLTYPE CImageViewerPlugin::RegisterCommands()
{
	// The RegisterCommands() method is invoked when the user interface manager is building its list
	// of available commands.  Shareaza uses a unified command architecture in which every command in
	// the UI is assigned a friendly name and an ID number.  Shareaza has already registered its
	// internal commands, and is now providing an opportunity for plugins to register their own.
	
	// The friendly name is assigned by you, and Shareaza will provide you with a command ID number.
	// Command ID numbers are not fixed, so they should be stored in a variable for later use!
	
	// By convention, you should name your commands with the name of the plugin, followed by an
	// underscore, followed by your command name.  There should be no spaces (normal C++ identifier
	// rules apply).
	
	// Note that this method may be called more than once in the lifetime of the plugin, in which
	// case you must re-register your commands and receive new command IDs.  This will happen when
	// other skins and plugins are loaded or unloaded.
	
	// (The second argument, although NULL here, can optionally provide a 16x16 icon handle,
	// but that is not the neatest way of doing it)
	
	m_pInterface->RegisterCommand( L"ImageViewer_BestFit", NULL, &m_nCmdBestFit );
	m_pInterface->RegisterCommand( L"ImageViewer_ActualSize", NULL, &m_nCmdActualSize );
	m_pInterface->RegisterCommand( L"ImageViewer_Refresh", NULL, &m_nCmdRefresh );
	m_pInterface->RegisterCommand( L"ImageViewer_Close", NULL, &m_nCmdClose );
	
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CImageViewerPlugin::InsertCommands()
{
	// The InsertCommands() method is invoked when the user interface manager is building the user
	// interface objects such as menus, toolbars, etc.  At this point it has created its internal
	// objects, and parsed all of the applicable skin files to create their objects.
	
	// The plugin should use this opportunity to either create its own user interface objects,
	// and/or modify existing objects to add new commands, etc.  Most of this work is achieved
	// through the IUserInterface interface.
	
	// Via IUserInterface you can access or create menus and toolbars by name (eg "CMainWnd.Tabbed"),
	// and then view their content, adding, modifying or deleting commands as desired.
	
	// If you are not modifying existing user interface objects, but rather creating your own (as
	// in the case of this image viewer), it is a lot easier to actually use the XML skin file
	// system.  This is a lot better than having to do it all here programatically.  The
	// IUserInterface interface provides three methods for loading and incorporating a chunk of
	// skin XML.
	
	// The best choice is often to include the XML as a resource in your DLL, which is what we have
	// done here.  IUserInterface::AddFromResource allows you to load a skin XML resource directly!
	
	m_pInterface->AddFromResource( _AtlBaseModule.GetResourceInstance(), IDR_SKIN );
	
	// Note that the resource type should be 23 decimal.  See the Skin.xml file for further detail.
	
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CImageViewerPlugin::OnUpdate(UINT /*nCommandID*/, TRISTATE __RPC_FAR* /*pbVisible*/, TRISTATE __RPC_FAR* /*pbEnabled*/, TRISTATE __RPC_FAR* /*pbChecked*/)
{
	// The OnUpdate() method is invoked when Shareaza needs to update the state of a command in its
	// user interface.  This provides an opportunity to show or hide, enable or disable, and check or
	// uncheck user interface commands.  Because of the unified command architecture, it does not matter
	// if the command is in a menu or a toolbar or something else entirely.
	
	// The nCommandID argument is the ID of the command being updated.  You should check this against
	// a list of command IDs your plugin has registered.  If you don't get a match, return S_FALSE.
	// Unless you have a really good reason, you don't want to mess with commands that you didn't
	// register (for one thing, you probably won't know what their ID number is).  The S_FALSE code
	// tells Shareaza to keep looking.
	
	// If you do find a match, you should modify pbVisible, pbEnabled and pbChecked.  Each is a "tri-state"
	// enumeration, defaulting to TSUNKNOWN.  Set TSTRUE to activate, or TSFALSE to deactivate.  Then,
	// return S_OK to indicate that you are responsible for this command, and have updated it.
	
	// You must check whether pbVisible, pbEnabled and pbChecked are NULL before reading or writing to
	// them, as one or more of them may be NULL if it is not required.
	
	// Here we are not interested in updating any commands, so we return S_FALSE.
	
	return S_FALSE;
}

HRESULT STDMETHODCALLTYPE CImageViewerPlugin::OnCommand(UINT /*nCommandID*/)
{
	// The OnCommand() method is invoked whenever the user invokes a command.  This applies to
	// ANY command in the unified architecture, which could be a built-in command, a command you
	// registered, or a command registered by another plugin.
	
	// Return S_OK if you are handling the command, or S_FALSE if Shareaza should keep looking.
	// Failure codes (E_*) will also cause Shareaza to stop looking for a handler.
	
	// Typically you would check the nCommandID argument against a list of command IDs you have
	// registered, and only return S_OK if you get a match.  If the command is not currently
	// available, you'd return E_UNEXPECTED.
	
	// However, if its for a good cause, you could also check for internal commands from the
	// base Shareaza UI, i.e. those which did not come from plugins.  These have fixed command IDs,
	// so they can be safely detected.  If you return S_OK for one of these, Shareaza won't take
	// its default action.
	
	// Here we are not interested in handling any commands, so we return S_FALSE.
	
	return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CImageViewerPlugin open a new window

BOOL CImageViewerPlugin::OpenNewWindow(LPCTSTR pszFilePath)
{
	// This is a helper function which opens a new window, or activates an existing window, for the
	// file name it is passed.

	// First, we check through the linked list of CImageWindow windows, to see if the file is
	// already open.

	CImageWindow* pWindow;
	for ( pWindow = m_pWindow ; pWindow ; pWindow = pWindow->m_pNext )
	{
		if ( pWindow->m_sFile.CompareNoCase( pszFilePath ) == 0 )
		{
			// Got a match, break out of the loop.
			break;
		}
	}
	
	// If we did not find the window..
	
	if ( pWindow == NULL )
	{
		// Create a new one, and add it to the linked list.
		
		pWindow = new CComObject<CImageWindow>;
		pWindow->m_pNext = m_pWindow;
		m_pWindow = pWindow;
		
		// Invoke the Create() method, to pass a reference to this plugin object, and the filename.
		
		pWindow->Create( this, pszFilePath );
	}
	
	// Tell the window (old or new) to refresh its image
	
	pWindow->Refresh();
	
	// Show and activate the window
	
	pWindow->ShowWindow( SW_SHOWNORMAL );
	pWindow->BringWindowToTop();
	pWindow->Invalidate();
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CImageViewerPlugin remove an existing window from the list

void CImageViewerPlugin::RemoveWindow(CImageWindow* pWindow)
{
	CImageWindow** ppPrev = &m_pWindow;
	
	// Search through the linked list of CImageWindow objects, and remove the one
	// which is being closed.
	
	for ( CImageWindow* pSeek = *ppPrev ; pSeek ; pSeek = pSeek->m_pNext )
	{
		if ( pWindow == pSeek )
		{
			*ppPrev = pWindow->m_pNext;
			return;
		}
		
		ppPrev = &pSeek->m_pNext;
	}
}

