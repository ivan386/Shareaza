//
// revision.js
//
// Copyright (c) Shareaza Development Team, 2009.
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

// This scipt compares current directory revision and revision saved in
// revision.h file using TortoiseSVN COM-interface.

var fso = WScript.CreateObject( "Scripting.FileSystemObject" );
var fpath = fso.GetAbsolutePathName( "." )
var fname = fso.GetAbsolutePathName( "revision.h" );

var revision, date, changed;
try
{
	var rev = new ActiveXObject( "SubWCRev.object" );
	rev.GetWCInfo( fpath, 0, 0 );
	revision = rev.Revision;
	changed = rev.HasModifications;
	revision += changed ? "M" : "";
	date = rev.Date;
	WScript.Echo( "Current revision \"" + revision + "\" at \"" + date + "\"");
}
catch(e)
{
	WScript.Echo( "TortoiseSVN COM-interface failed. (Re)Install it from: http://tortoisesvn.net/" );
	WScript.Quit( 1 );
}

var modified;
try
{
	var tsr = fso.OpenTextFile( fname, 1, false );
	modified = tsr.ReadLine().substr( 7 );	// Parsing "// rev.XXXXX"
	tsr.Close();
}
catch(e)
{
}
if ( revision != modified )
{
	WScript.Echo( "Updating from \"" + modified + "\" to \"" + revision + "\"...");
	try
	{
		var tsw = fso.OpenTextFile( fname, 2, true );
		tsw.WriteLine( "// rev." + revision );
		tsw.WriteLine( "" );
		tsw.WriteLine( "#pragma once" );
		tsw.WriteLine( "" );
		tsw.WriteLine( "#define __REVISION__\t\t\"" + revision + "\"" );
		tsw.WriteLine( "#define __REVISION_DATE__\t\"" + date + "\"" );
		tsw.Close();
	}
	catch(e)
	{
		WScript.Echo( "Update failed: \"" + fname + "\"" );
		WScript.Quit( 1 );
	}
}
else
	WScript.Echo( "Already up to date." );

WScript.Quit( 0 );
