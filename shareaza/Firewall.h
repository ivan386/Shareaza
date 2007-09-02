//
// Firewall.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#include <upnp.h>

// Control the Windows Firewall, and talk UPnP to the NAT router to setup port forwarding
class CFirewall
{
public:

	// Windows Firewall COM interfaces accessed with the object
    CComPtr< INetFwMgr >					Manager;
    CComPtr< INetFwPolicy >					Policy;
	CComPtr< INetFwProfile >				Profile;
	CComPtr< INetFwServices >				ServiceList;
	CComPtr< INetFwAuthorizedApplications >	ProgramList;
    CComPtr< INetFwOpenPorts >				PortList;

	// Windows Firewall COM interfaces accessed in methods
	CComPtr< INetFwService >				Service;
	CComPtr< INetFwAuthorizedApplication >	Program;
    CComPtr< INetFwOpenPort >				Port;

	// UPnP COM interfaces
	CComPtr< IUPnPNAT >						Nat;
	CComPtr< IStaticPortMappingCollection >	Collection;
	CComPtr< IStaticPortMapping >			Mapping;
	BOOL									m_bInitialized;

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
	BOOL SetupProgram( const CString& path, const CString& name, BOOL bRemove = FALSE);// List a program and check its box
	BOOL AccessWindowsFirewall();                                        // Access the Windows Firewall COM objects, call before calling the methods below
	BOOL IsProgramListed( const CString& path, BOOL* listed );           // Determine if a program is on the exceptions list
	BOOL IsServiceEnabled( NET_FW_SERVICE_TYPE service, BOOL* enabled ); // Determine if a service is checked
	BOOL IsProgramEnabled( const CString& path, BOOL* enabled );         // Determine if a listed program is checked
	BOOL AreExceptionsAllowed();										 // Find out if the system is in no-exceptions mode
	BOOL AddProgram( const CString& path, const CString& name );         // Add a program to the list with a checked box
	BOOL RemoveProgram( const CString& path );							 // Add a program to the list with a checked box
	BOOL EnableService( NET_FW_SERVICE_TYPE service );                   // Check the box for a service
	BOOL EnableProgram( const CString& path );                           // Check the box for a program
};

#endif //#ifndef FIREWALL_H_INCLUDED
