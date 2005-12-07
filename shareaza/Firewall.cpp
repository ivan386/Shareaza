//
// Firewall.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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

// CFirewall wraps Windows COM components to change Windows Firewall settings, and talk UPnP to a NAT router
// http://wiki.shareaza.com/static/Developers.Code.CFirewall

// Include
#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Firewall.h"

// Make a new WindowsFirewall object
CFirewall::CFirewall()
{
	// Set the COM interface pointers to NULL so we'll know if we've initialized them
	Manager     = NULL;
	Policy      = NULL;
	Profile     = NULL;
	ServiceList = NULL;
	ProgramList = NULL;
	PortList    = NULL;
	Service     = NULL;
	Program     = NULL;
	Port        = NULL;
	Nat         = NULL;
	Collection  = NULL;
	Mapping     = NULL;
	m_bInitialized = FALSE;
}

// Delete the WindowsFirewall object
CFirewall::~CFirewall()
{
	// Release the COM interfaces that we got access to
	if ( Port )        { Port->Release();        Port        = NULL; }
	if ( Program )     { Program->Release();     Program     = NULL; }
	if ( Service )     { Service->Release();     Service     = NULL; }
	if ( PortList )    { PortList->Release();    PortList    = NULL; }
	if ( ProgramList ) { ProgramList->Release(); ProgramList = NULL; }
	if ( ServiceList ) { ServiceList->Release(); ServiceList = NULL; }
	if ( Profile )     { Profile->Release();     Profile     = NULL; }
	if ( Policy )      { Policy->Release();      Policy      = NULL; }
	if ( Manager )     { Manager->Release();     Manager     = NULL; }
	if ( Mapping )     { Mapping->Release();     Mapping     = NULL; }
	if ( Collection )  { Collection->Release();  Collection  = NULL; }
	if ( Nat )         { Nat->Release();         Nat         = NULL; }
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
BOOL CFirewall::SetupProgram( CString path, CString name, BOOL bRemove )
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
	else return FALSE;

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
	// Initialize COM itself so this thread can use it
	HRESULT result = CoInitialize( NULL ); // Must be NULL
	if ( FAILED( result ) ) return FALSE;

	// Create an instance of the firewall settings manager
	result = CoCreateInstance( __uuidof( NetFwMgr ), NULL, CLSCTX_INPROC_SERVER, __uuidof( INetFwMgr ), ( void** )&Manager );
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
BOOL CFirewall::IsProgramListed( CString path, BOOL* listed )
{
	// Look for the program in the list
	if ( Program ) { Program->Release(); Program = NULL; }
	CBstr p( path ); // Express the name as a BSTR
	HRESULT result = ProgramList->Item( p.B, &Program ); // Try to get the interface for the program with the given name
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
	if ( Service ) { Service->Release(); Service = NULL; }
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
BOOL CFirewall::IsProgramEnabled( CString path, BOOL* enabled )
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
BOOL CFirewall::AddProgram( CString path, CString name )
{
	// Create an instance of an authorized application, we'll use this to add our new application
	if ( Program ) { Program->Release(); Program = NULL; }
	HRESULT result = CoCreateInstance( __uuidof( NetFwAuthorizedApplication ), NULL, CLSCTX_INPROC_SERVER,
		__uuidof( INetFwAuthorizedApplication ), ( void** )&Program );
	if ( FAILED( result ) ) return FALSE;

	// Set the text
	CBstr p( path );                                // Express the text as BSTRs
	result = Program->put_ProcessImageFileName( p.B ); // Set the process image file name
	if ( FAILED( result ) ) return FALSE;
	CBstr n( name );
	result = Program->put_Name( n.B );                 // Set the program name
	if ( FAILED( result ) ) return FALSE;

	// Get the program on the Windows Firewall accept list
	result = ProgramList->Add( Program ); // Add the application to the collection
	if ( FAILED( result ) ) return FALSE;
	return TRUE;
}

BOOL CFirewall::RemoveProgram( CString path )
{
	if ( ! ProgramList || ! m_bInitialized ) return FALSE;

	CBstr p( path );

	HRESULT result = ProgramList->Remove( p.B ); // Remove the application to the collection
	if ( FAILED( result ) ) return FALSE;
	return TRUE;
}

// Takes a service type, like NET_FW_SERVICE_UPNP
// Checks the checkbox next to its listing in Windows Firewall
// Returns false on error
BOOL CFirewall::EnableService( NET_FW_SERVICE_TYPE service )
{
	// Look for the service in the list
	if ( Service ) { Service->Release(); Service = NULL; }
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
BOOL CFirewall::EnableProgram( CString path )
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

// Takes a protocol 't' for TCP or 'u' for UDP
// Takes the external port number the NAT router will listen for on the Internet
// Takes the internal port number this computer is listening for on the LAN, the external and internal ports are usually the same
// Takes the IP address of this computer on the LAN as a string, like "192.168.1.103"
// Takes the program name that will show up in the router's Web configuration page
// Removes any listing for the given protocol and port, and then sets up a new one with all the given information
// Returns false on error
BOOL CFirewall::SetupForward( char protocol, int externalport, int internalport, CString ipaddress, CString name )
{
	// Connect to the COM object, and have it begin talking UPnP to the router
	if ( ! Nat ) if ( ! AccessUPnP() ) return FALSE;

	// Remove any forward the router has for the given protocol and port
	Remove( protocol, externalport ); // Even if remove fails, we still want to try to add

	// Add a fresh port mapping with the right name and a checked box
	if ( ! Add( protocol, externalport, internalport, ipaddress, name ) ) return FALSE;

	// Everything worked
	return TRUE;
}

// Get access to the COM objects
// Returns true if it works, false if there was an error
BOOL CFirewall::AccessUPnP()
{
	// Initialize COM itself so this thread can use it
	HRESULT result = CoInitialize( NULL ); // Must be NULL
	if ( FAILED( result ) ) return FALSE;

	// Access the IUPnPNAT COM interface, has Windows send UPnP messages to the NAT router
	result = CoCreateInstance( __uuidof( UPnPNAT ), NULL, CLSCTX_ALL, __uuidof( IUPnPNAT ), ( void** )&Nat );
	if ( FAILED( result ) || ! Nat ) return FALSE;

	// Get the collection of forwarded ports from it, has Windows send UPnP messages to the NAT router
	result = Nat->get_StaticPortMappingCollection( &Collection ); // Won't work if the NAT has UPnP turned off
	if ( FAILED( result ) || ! Collection ) return FALSE; // Frequently, result is S_OK but Collection is null

	// Everything worked
	return TRUE;
}

// Takes a protocol 't' for TCP or 'u' for UDP, the port to forward, the computer's LAN IP address like "192.168.1.103", and the program name
// Talks UPnP to the router to setup port forwarding
// Returns false if there was an error
BOOL CFirewall::Add( char protocol, int externalport, int internalport, CString ipaddress, CString name )
{
	// Make a local BSTR with the protocol described in text
	CBstr p;
	if ( protocol == 't' ) p.Set( "TCP" );
	else                   p.Set( "UDP" );

	// Express the name and description as BSTRs
	CBstr i( ipaddress );
	CBstr n( name );

	// Have Windows send UPnP messages to the NAT router to get it to forward a port
	if ( Mapping ) { Mapping->Release(); Mapping = NULL; } // Reuse the mapping interface pointer
	HRESULT result = Collection->Add(  // Create a new port mapping, and add it to the collection
		externalport, // The port to forward
		p.B,          // The protocol as the text "TCP" or "UDP" in a BSTR
		internalport, // This computer's internal LAN port to forward to, like 192.168.1.100:internalport
		i.B,          // Internal IP address to forward to, like "192.168.1.100"
		true,         // True to start forwarding now
		n.B,          // Description text the router can show in its Web configuration interface
		&Mapping );   // Access to the IStaticPortMapping interface, if this works
	if ( FAILED( result ) || ! Mapping ) return FALSE;
	return TRUE;
}

// Takes a protocol 't' for TCP or 'u' for UDP, and a port being forwarded
// Talks UPnP to the router to remove the forwarding
// Returns false if there was an error
BOOL CFirewall::Remove( char protocol, int externalport )
{
	// Make a local BSTR with the protocol described in text
	CBstr b;
	if ( protocol == 't' ) b.Set( "TCP" );
	else                   b.Set( "UDP" );

	// Have Windows send UPnP messages to the NAT router to get it to stop forwarding a port
	HRESULT result = Collection->Remove(  // Remove the specified port mapping from the collection
		externalport,                     // The port being forwarded
		b.B );                            // The protocol as the text "TCP" or "UDP" in a BSTR
	if ( FAILED( result ) ) return FALSE; // Returns S_OK even if there was nothing there to remove
	return TRUE;
}
