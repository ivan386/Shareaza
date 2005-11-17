//
// Firewall.h
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

// If Visual Studio can't find these files, check out http://wiki.shareaza.com/static/Zootella.SP2Platform
// Include headers from the Windows XP SP2 version of the Platform SDK, or later

#ifndef FIREWALL_H_INCLUDED
#define FIREWALL_H_INCLUDED

// Wraps a COM string BSTR so the destructor can call SysFreeString
class CBstr
{
public:

	// The BSTR
	BSTR B;

	// Make a new CBstr object
	CBstr()
	{
		// No memory allocated, just the BSTR pointed at NULL
		B = NULL;
	}

	// Takes a string
	// Make a new CBstr object with the text
	CBstr( CString s )
	{
		// Initialize the BSTR to null, and then set it with the text
		B = NULL;
		Set( s );
	}

	// Gets called when a local CBstr object goes out of scope
	// Calls SysFreeString automatically
	~CBstr()
	{
		// Clear the BSTR, freeing its memory
		Clear();
	}

	// Takes a string
	// Sets the BSTR in the object with the text
	void Set( CString s )
	{
		// Clear the BSTR, freeing its memory
		Clear();

		// Allocate a BSTR of wide characters with the ASCII or Unicode text in the given CString s
		B = s.AllocSysString();
	}

	// Clears the BSTR, freeing its memory
	void Clear()
	{
		// If the BSTR points to allocated memory, free it and set it back to NULL
		if ( B ) { SysFreeString( B ); B = NULL; }
	}
};

// Control the Windows Firewall, and talk UPnP to the NAT router to setup port forwarding
class CFirewall
{
public:

	// Windows Firewall COM interfaces accessed with the object
    INetFwMgr*                    Manager;
    INetFwPolicy*                 Policy;
	INetFwProfile*                Profile;
	INetFwServices*               ServiceList;
	INetFwAuthorizedApplications* ProgramList;
    INetFwOpenPorts*              PortList;

	// Windows Firewall COM interfaces accessed in methods
	INetFwService*                Service;
	INetFwAuthorizedApplication*  Program;
    INetFwOpenPort*               Port;

	// UPnP COM interfaces
	IUPnPNAT*                     Nat;
	IStaticPortMappingCollection* Collection;
	IStaticPortMapping*           Mapping;

	// Constructor and destructor
	CFirewall();
	~CFirewall();

	// Examples controlling Windows Firewall
	//
	//	// Let a program listen on a socket without Windows Firewall poping up a warning
	//	CFirewall firewall;
	//	firewall.SetupProgram( "C:\\Program Files\\Outlook Express\\msimn.exe", "Outlook Express" );
	//
	//	// Enable the UPnP service in Windows Firewall so we can talk UPnP through it
	//	firewall.SetupService( NET_FW_SERVICE_UPNP );

	// Windows Firewall Methods
	BOOL SetupService( NET_FW_SERVICE_TYPE service );                    // Check a box for a service on the Windows Firewall exceptions list
	BOOL SetupProgram( CString path, CString name );                     // List a program and check its box
	BOOL AccessWindowsFirewall();                                        // Access the Windows Firewall COM objects, call before calling the methods below
	BOOL IsProgramListed( CString path, BOOL* listed );                  // Determine if a program is on the exceptions list
	BOOL IsServiceEnabled( NET_FW_SERVICE_TYPE service, BOOL* enabled ); // Determine if a service is checked
	BOOL IsProgramEnabled( CString path, BOOL* enabled );                // Determine if a listed program is checked
	BOOL AddProgram( CString path, CString name );                       // Add a program to the list with a checked box
	BOOL EnableService( NET_FW_SERVICE_TYPE service );                   // Check the box for a service
	BOOL EnableProgram( CString path );                                  // Check the box for a program

	// Example talking UPnP to the router to setup port forwarding
	//
	//	// Have the NAT router forward TCP port 6346 to this computer
	//	CFirewall firewall;
	//	firewall.SetupForward( 't', 6346, 6346, "192.168.1.103", "Shareaza" );

	// UPnP Methods
	BOOL SetupForward( char protocol, int externalport, int internalport, CString ipaddress, CString name ); // Remove and then add a port forwarding
	BOOL AccessUPnP();                                                                                       // Access the UPnP COM objects, call before below
	BOOL Add( char protocol, int externalport, int internalport, CString ipaddress, CString name );          // Add a port forwarding
	BOOL Remove( char protocol, int externalport );                                                          // Remove a port forwarding
};

#endif //#ifndef FIREWALL_H_INCLUDED
