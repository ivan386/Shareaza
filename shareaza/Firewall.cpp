//
// Firewall.cpp
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

// CFirewall wraps Windows COM components to change Windows Firewall settings, and talk UPnP to a NAT router
// http://wiki.shareaza.com/static/Developers.Code.CFirewall

// Include
#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Firewall.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Make a new WindowsFirewall object
CFirewall::CFirewall() :
	m_bInitialized( FALSE )
{
}

// Delete the WindowsFirewall object
CFirewall::~CFirewall()
{
}

// Takes a service type, like NET_FW_SERVICE_UPNP, which is listed in Windows Firewall and can't be removed
// Makes sure it is checked in Windows Firewall, checking it if necessary
// Returns true if the service is listed and checked, false if we weren't able to check it
BOOL CFirewall::SetupService( NET_FW_SERVICE_TYPE service )
{
	// Make sure the COM interfaces have been accessed
	if ( ! Manager ) if ( ! m_bInitialized ) return FALSE;

	// If the service isn't enabled on the Windows Firewall exceptions list
	BOOL enabled;
	if ( ! IsServiceEnabled( service, &enabled ) ) return FALSE;
	if ( ! enabled )
	{
		// Check its checkbox
		if ( ! EnableService( service ) ) return FALSE;
	}

	// The service is listed and checked
	return TRUE;
}

// Takes a path and file name like "C:\Folder\Program.exe" and a name like "My Program"
// Makes sure the program is listed in Windows Firewall and its listing is checked, adding and checking it as necessary
// Returns true if the program is listed and checked, false if we weren't able to do it
// When bRemove is TRUE, it removes the application from the exception list
BOOL CFirewall::SetupProgram( const CString& path, const CString& name, BOOL bRemove )
{
	// Make sure the COM interfaces have been accessed
	if ( ! Manager ) if ( ! m_bInitialized ) return FALSE;

	// If the program isn't on the Windows Firewall exceptions list
	BOOL listed, enabled;
	if ( ! IsProgramListed( path, &listed ) ) return FALSE;
	if ( ! listed && ! bRemove )
	{
		// Add it to the list with a checked checkbox
		if ( ! AddProgram( path, name ) ) return FALSE;
	}
	else if ( listed && bRemove )
	{
		if ( ! RemoveProgram( path ) ) return FALSE;
		return TRUE;
	}

	// If the program is on the list, but its checkbox isn't checked
	if ( ! IsProgramEnabled( path, &enabled ) ) return FALSE;
	if ( ! enabled )
	{
		// Check the checkbox
		if ( ! EnableProgram( path ) ) return FALSE;
	}

	// The program is listed and checked
	return TRUE;
}

// Get access to the COM objects
// Returns true if it works, false if there was an error
BOOL CFirewall::AccessWindowsFirewall()
{
	HRESULT result;

	// Create an instance of the firewall settings manager
	result = Manager.CoCreateInstance( __uuidof( NetFwMgr ) );
	if ( FAILED( result ) || ! Manager ) return FALSE;

	// Retrieve the local firewall policy
	result = Manager->get_LocalPolicy( &Policy );
	if ( FAILED( result ) || ! Policy ) return FALSE;

	// Retrieve the firewall profile currently in effect
	result = Policy->get_CurrentProfile( &Profile );
	if ( FAILED( result ) || ! Profile ) return FALSE;

	// Retrieve the allowed services collection
	result = Profile->get_Services( &ServiceList );
	if ( FAILED( result ) || ! ServiceList ) return FALSE;

	// Retrieve the authorized application collection
	result = Profile->get_AuthorizedApplications( &ProgramList );
	if ( FAILED( result ) || ! ProgramList ) return FALSE;

	// Retrieve the globally open ports collection
	result = Profile->get_GloballyOpenPorts( &PortList );
	if ( FAILED( result ) || ! PortList ) return FALSE;

	// Everything worked
	m_bInitialized = TRUE;
	return TRUE;
}

// Takes a program path and file name, like "C:\Folder\Program.exe"
// Determines if it's listed in Windows Firewall
// Returns true if it works, and writes the answer in listed
BOOL CFirewall::IsProgramListed( const CString& path, BOOL* listed )
{
	// Look for the program in the list
	Program.Release();

	// Try to get the interface for the program with the given name
	HRESULT result = ProgramList->Item( CComBSTR( path ), &Program );
	if ( SUCCEEDED( result ) )
	{
		// The program is in the list
		*listed = TRUE;
		return TRUE;

	} // The ProgramList->Item call failed
	else
	{
		// The error is not found
		if ( result == HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) )
		{
			// The program is not in the list
			*listed = FALSE;
			return TRUE;

		} // Some other error occurred
		else
		{
			// Report it
			return FALSE;
		}
	}
}

// Takes a service type, like NET_FW_SERVICE_UPNP
// Determines if the listing for that service in Windows Firewall is checked or unchecked
// Returns true if it works, and writes the answer in enabled
BOOL CFirewall::IsServiceEnabled( NET_FW_SERVICE_TYPE service, BOOL* enabled )
{
	// Look for the service in the list
	Service.Release();
	HRESULT result = ServiceList->Item( service, &Service );
	if ( FAILED( result ) ) return FALSE; // Services can't be removed from the list

	// Find out if the service is enabled
	VARIANT_BOOL v;
	result = Service->get_Enabled( &v );
	if ( FAILED( result ) ) return FALSE;
	if ( v == VARIANT_FALSE )
	{
		// The service is on the list, but the checkbox next to it is cleared
		*enabled = FALSE;
		return TRUE;
	}
	else
	{
		// The service is on the list and the checkbox is checked
		*enabled = TRUE;
		return TRUE;
	}
}

// Takes a program path and file name like "C:\Folder\Program.exe"
// Determines if the listing for that program in Windows Firewall is checked or unchecked
// Returns true if it works, and writes the answer in enabled
BOOL CFirewall::IsProgramEnabled( const CString& path, BOOL* enabled )
{
	// First, make sure the program is listed
	BOOL listed;
	if ( ! IsProgramListed( path, &listed ) ) return FALSE; // This sets the Program interface we can use here
	if ( ! listed ) return FALSE; // The program isn't in the list at all

	// Find out if the program is enabled
	VARIANT_BOOL v;
	HRESULT result = Program->get_Enabled( &v );
	if ( FAILED( result ) ) return FALSE;
	if ( v == VARIANT_FALSE )
	{
		// The program is on the list, but the checkbox next to it is cleared
		*enabled = FALSE;
		return TRUE;
	}
	else
	{
		// The program is on the list and the checkbox is checked
		*enabled = TRUE;
		return TRUE;
	}
}

// This means that all the exceptions such as GloballyOpenPorts, Applications, or Services, 
// which are specified in the profile, are ignored and only locally initiated traffic is allowed

BOOL CFirewall::AreExceptionsAllowed()
{
    VARIANT_BOOL	vbNotAllowed = VARIANT_FALSE;
    HRESULT			hr = S_OK;

    hr = Profile->get_ExceptionsNotAllowed( &vbNotAllowed );
    if ( SUCCEEDED(hr) && vbNotAllowed != VARIANT_FALSE ) return FALSE;
    
    return TRUE;
}

// Takes a path and file name like "C:\Folder\Program.exe" and a name like "My Program"
// Lists and checks the program on Windows Firewall, so now it can listed on a socket without a warning popping up
// Returns false on error
BOOL CFirewall::AddProgram( const CString& path, const CString& name )
{
	// Create an instance of an authorized application, we'll use this to add our new application
	Program.Release();
	HRESULT result = Program.CoCreateInstance( __uuidof( NetFwAuthorizedApplication ) );
	if ( FAILED( result ) ) return FALSE;

	result = Program->put_ProcessImageFileName( CComBSTR( path ) ); // Set the process image file name
	if ( FAILED( result ) ) return FALSE;

	result = Program->put_Name( CComBSTR( name ) );					// Set the program name
	if ( FAILED( result ) ) return FALSE;

	// Get the program on the Windows Firewall accept list
	result = ProgramList->Add( Program ); // Add the application to the collection
	if ( FAILED( result ) ) return FALSE;
	return TRUE;
}

BOOL CFirewall::RemoveProgram( const CString& path )
{
	if ( ! ProgramList || ! m_bInitialized ) return FALSE;

	HRESULT result = ProgramList->Remove( CComBSTR( path ) ); // Remove the application to the collection
	if ( FAILED( result ) ) return FALSE;
	return TRUE;
}

// Takes a service type, like NET_FW_SERVICE_UPNP
// Checks the checkbox next to its listing in Windows Firewall
// Returns false on error
BOOL CFirewall::EnableService( NET_FW_SERVICE_TYPE service )
{
	// Look for the service in the list
	Service.Release();
	HRESULT result = ServiceList->Item( service, &Service );
	if ( FAILED( result ) ) return FALSE; // Services can't be removed from the list

	// Check the box next to the service
	VARIANT_BOOL v = TRUE;
	result = Service->put_Enabled( v );
	if ( FAILED( result ) ) return FALSE;
	return TRUE;
}

// Takes a program path and file name like "C:\Folder\Program.exe"
// Checks the checkbox next to its listing in Windows Firewall
// Returns false on error
BOOL CFirewall::EnableProgram( const CString& path )
{
	// First, make sure the program is listed
	BOOL listed;
	if ( ! IsProgramListed( path, &listed ) ) return FALSE; // This sets the Program interface we can use here
	if ( ! listed ) return FALSE; // The program isn't on the list at all

	// Check the box next to the program
	VARIANT_BOOL v = TRUE;
	HRESULT result = Program->put_Enabled( v );
	if ( FAILED( result ) ) return FALSE;
	return TRUE;
}
